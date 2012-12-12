/*
* copyright (c) 2010 Sveriges Television AB <info@casparcg.com>
*
*  This file is part of CasparCG.
*
*    CasparCG is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    CasparCG is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.

*    You should have received a copy of the GNU General Public License
*    along with CasparCG.  If not, see <http://www.gnu.org/licenses/>.
*
*/
#include "../stdafx.h"

#include "ffmpeg_producer.h"

#include "../ffmpeg_error.h"
#include "muxer/frame_muxer.h"
#include "input/input.h"
#include "util/util.h"
#include "audio/audio_decoder.h"
#include "video/video_decoder.h"

#include <common/env.h>
#include <common/utility/assert.h>
#include <common/diagnostics/graph.h>

#include <core/video_format.h>
#include <core/producer/frame_producer.h>
#include <core/producer/frame/frame_factory.h>
#include <core/producer/frame/basic_frame.h>
#include <core/producer/frame/frame_transform.h>

#include <boost/algorithm/string.hpp>
#include <boost/assign.hpp>
#include <boost/timer.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/find.hpp>

#include <tbb/parallel_invoke.h>

#include <limits>
#include <memory>
#include <queue>

namespace caspar { namespace ffmpeg {
				
struct ffmpeg_producer : public core::frame_producer
{
	const std::wstring								filename_;
	
	const safe_ptr<diagnostics::graph>				graph_;
	boost::timer									frame_timer_;
	boost::timer									video_timer_;
	boost::timer									audio_timer_;
					
	const safe_ptr<core::frame_factory>				frame_factory_;
	const core::video_format_desc					format_desc_;

	input											input_;	
	std::unique_ptr<video_decoder>					video_decoder_;
	std::unique_ptr<audio_decoder>					audio_decoder_;	
	std::unique_ptr<frame_muxer>					muxer_;

	const int										start_;
	const bool										loop_;
	const size_t									length_;

	safe_ptr<core::basic_frame>						last_frame_;
	
	std::queue<safe_ptr<core::basic_frame>>			frame_buffer_;
	
public:
	explicit ffmpeg_producer(const safe_ptr<core::frame_factory>& frame_factory, const std::wstring& filename, const std::wstring& filter, bool loop, int start, size_t length) 
		: filename_(filename)
		, frame_factory_(frame_factory)		
		, format_desc_(frame_factory->get_video_format_desc())
		, input_(graph_, filename_, loop, start, length)
		, start_(start)
		, loop_(loop)
		, length_(length)
		, last_frame_(core::basic_frame::empty())
	{
		graph_->add_guide("frame-time", 0.5);
		graph_->set_color("frame-time", diagnostics::color(0.1f, 1.0f, 0.1f));
		graph_->set_color("underflow", diagnostics::color(0.6f, 0.3f, 0.9f));	
		diagnostics::register_graph(graph_);
		
		try
		{
			video_decoder_.reset(new video_decoder(input_.context()));
		}
		catch(averror_stream_not_found&)
		{
			CASPAR_LOG(warning) << "No video-stream found. Running without video.";	
		}
		catch(...)
		{
			CASPAR_LOG_CURRENT_EXCEPTION();
			CASPAR_LOG(warning) << "Failed to open video-stream. Running without video.";	
		}

		try
		{
			audio_decoder_.reset(new audio_decoder(input_.context(), frame_factory->get_video_format_desc()));
		}
		catch(averror_stream_not_found&)
		{
			CASPAR_LOG(warning) << "No audio-stream found. Running without audio.";	
		}
		catch(...)
		{
			CASPAR_LOG_CURRENT_EXCEPTION();
			CASPAR_LOG(warning) << "Failed to open audio-stream. Running without audio.";		
		}	

		muxer_.reset(new frame_muxer(video_decoder_ ? video_decoder_->fps() : frame_factory->get_video_format_desc().fps, frame_factory, filter));
	}
	
	virtual safe_ptr<core::basic_frame> receive(int hints)
	{		
		frame_timer_.restart();
		
		for(int n = 0; n < 32 && frame_buffer_.size() < 2; ++n)
			try_decode_frame(hints);
		
		graph_->update_value("frame-time", frame_timer_.elapsed()*format_desc_.fps*0.5);
				
		if(frame_buffer_.empty() && input_.eof())
			return core::basic_frame::eof();

		if(frame_buffer_.empty())
		{
			graph_->add_tag("underflow");	
			return core::basic_frame::late();			
		}
		
		last_frame_ = frame_buffer_.front();	
		frame_buffer_.pop();

		graph_->set_text(print());

		return last_frame_;
	}

	virtual safe_ptr<core::basic_frame> last_frame() const
	{
		return disable_audio(last_frame_);
	}
	
	void try_decode_frame(int hints)
	{
		std::shared_ptr<AVPacket> pkt;

		for(int n = 0; n < 32 && ((video_decoder_ && !video_decoder_->ready()) || (audio_decoder_ && !audio_decoder_->ready())) && input_.try_pop(pkt); ++n)
		{
			if(video_decoder_)
				video_decoder_->push(pkt);
			if(audio_decoder_)
				audio_decoder_->push(pkt);
		}
		
		std::shared_ptr<AVFrame>			video;
		std::shared_ptr<core::audio_buffer> audio;

		tbb::parallel_invoke(
		[&]
		{
			if(!muxer_->video_ready() && video_decoder_)			
				video = video_decoder_->poll();			
		},
		[&]
		{		
			if(!muxer_->audio_ready() && audio_decoder_)			
				audio = audio_decoder_->poll();			
		});

		muxer_->push(video, hints);
		muxer_->push(audio);

		if(!audio_decoder_)
		{
			if(video == flush_video())
				muxer_->push(flush_audio());
			else if(!muxer_->audio_ready())
				muxer_->push(empty_audio());
		}

		if(!video_decoder_)
		{
			if(audio == flush_audio())
				muxer_->push(flush_video(), 0);
			else if(!muxer_->video_ready())
				muxer_->push(empty_video(), 0);
		}
		
		for(auto frame = muxer_->poll(); frame; frame = muxer_->poll())
			frame_buffer_.push(make_safe_ptr(frame));
	}

	virtual int64_t nb_frames() const 
	{
		if(loop_)
			return std::numeric_limits<int64_t>::max();

		// This function estimates nb_frames until input has read all packets for one loop, at which point the count should be accurate.

		int64_t nb_frames = input_.nb_frames();
		if(input_.nb_loops() < 1) // input still hasn't counted all frames
		{
			auto video_nb_frames = video_decoder_ ? video_decoder_->nb_frames() : std::numeric_limits<int64_t>::max();
			auto audio_nb_frames = audio_decoder_ ? audio_decoder_->nb_frames() : std::numeric_limits<int64_t>::max();

			nb_frames = std::min(static_cast<int64_t>(length_), std::max(nb_frames, std::max(video_nb_frames, audio_nb_frames)));
		}

		nb_frames = muxer_->calc_nb_frames(nb_frames);

		// TODO: Might need to scale nb_frames av frame_muxer transformations.

		return nb_frames - start_;
	}
				
	virtual std::wstring print() const
	{
		if(video_decoder_)
		{
			return L"ffmpeg[" + boost::filesystem::wpath(filename_).filename() + L"|" 
							  + boost::lexical_cast<std::wstring>(video_decoder_->width()) + L"x" + boost::lexical_cast<std::wstring>(video_decoder_->height())
							  + (video_decoder_->is_progressive() ? L"p" : L"i")  + boost::lexical_cast<std::wstring>(video_decoder_->is_progressive() ? video_decoder_->fps() : 2.0 * video_decoder_->fps())
							  + L"]";
		}
		
		return L"ffmpeg[" + boost::filesystem::wpath(filename_).filename() + L"]";
	}
};

safe_ptr<core::frame_producer> create_producer(const safe_ptr<core::frame_factory>& frame_factory, const std::vector<std::wstring>& params)
{		
	static const std::vector<std::wstring> extensions = boost::assign::list_of
		(L"mpg")(L"mpeg")(L"m2v")(L"m4v")(L"mp3")(L"mp4")(L"mpga")
		(L"avi")
		(L"mov")
		(L"qt")
		(L"webm")
		(L"dv")		
		(L"f4v")(L"flv")
		(L"mkv")(L"mka")
		(L"wmv")(L"wma")(L"wav")
		(L"rm")(L"ram")
		(L"ogg")(L"ogv")(L"oga")(L"ogx")
		(L"divx")(L"xvid");

	std::wstring filename = env::media_folder() + L"\\" + params[0];
	
	auto ext = boost::find_if(extensions, [&](const std::wstring& ex)
	{					
		return boost::filesystem::is_regular_file(boost::filesystem::wpath(filename + L"." + ex));
	});

	if(ext == extensions.end())
		return core::frame_producer::empty();

	auto path		= filename + L"." + *ext;
	auto loop		= boost::range::find(params, L"LOOP") != params.end();
	auto start		= core::get_param(L"SEEK", params, 0);
	auto length		= core::get_param(L"LENGTH", params, std::numeric_limits<size_t>::max());
	auto filter_str = core::get_param<std::wstring>(L"FILTER", params, L""); 	
		
	boost::replace_all(filter_str, L"DEINTERLACE", L"YADIF=0:-1");
	boost::replace_all(filter_str, L"DEINTERLACE_BOB", L"YADIF=1:-1");
	
	return create_destroy_proxy(make_safe<ffmpeg_producer>(frame_factory, path, filter_str, loop, start, length));
}

}}