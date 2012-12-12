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
#include "../../stdafx.h"

#include "video_decoder.h"

#include "../util/util.h"

#include "../../ffmpeg_error.h"

#include <core/producer/frame/frame_transform.h>
#include <core/producer/frame/frame_factory.h>

#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/filesystem.hpp>

#include <queue>

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable : 4244)
#endif
extern "C" 
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}
#if defined(_MSC_VER)
#pragma warning (pop)
#endif

namespace caspar { namespace ffmpeg {
	
struct video_decoder::implementation : boost::noncopyable
{
	int										index_;
	const safe_ptr<AVCodecContext>			codec_context_;

	std::queue<safe_ptr<AVPacket>>			packets_;
	
	const double							fps_;
	const int64_t							nb_frames_;

	const size_t							width_;
	const size_t							height_;
	bool									is_progressive_;

public:
	explicit implementation(const safe_ptr<AVFormatContext>& context) 
		: codec_context_(open_codec(*context, AVMEDIA_TYPE_VIDEO, index_))
		, fps_(static_cast<double>(codec_context_->time_base.den) / static_cast<double>(codec_context_->time_base.num))
		, nb_frames_(context->streams[index_]->nb_frames)
		, width_(codec_context_->width)
		, height_(codec_context_->height)
	{
		CASPAR_LOG(debug) << "[video_decoder] " << context->streams[index_]->codec->codec->long_name;
	}

	void push(const std::shared_ptr<AVPacket>& packet)
	{
		if(!packet)
			return;

		if(packet->stream_index == index_ || packet == flush_packet())
			packets_.push(make_safe_ptr(packet));
	}

	std::shared_ptr<AVFrame> poll()
	{		
		if(packets_.empty())
			return nullptr;
		
		auto packet = packets_.front();
					
		if(packet == flush_packet())
		{			
			if(codec_context_->codec->capabilities & CODEC_CAP_DELAY)
			{
				AVPacket pkt;
				av_init_packet(&pkt);
				pkt.data = nullptr;
				pkt.size = 0;

				auto video = decode(pkt);
				if(video)
					return video;
			}
		
			packets_.pop();
			avcodec_flush_buffers(codec_context_.get());
			return flush_video();	
		}
			
		packets_.pop();
		return decode(*packet);
	}

	std::shared_ptr<AVFrame> decode(AVPacket& pkt)
	{
		std::shared_ptr<AVFrame> decoded_frame(avcodec_alloc_frame(), av_free);

		int frame_finished = 0;
		THROW_ON_ERROR2(avcodec_decode_video2(codec_context_.get(), decoded_frame.get(), &frame_finished, &pkt), "[video_decocer]");
		
		// If a decoder consumes less then the whole packet then something is wrong
		// that might be just harmless padding at the end, or a problem with the
		// AVParser or demuxer which puted more then one frame in a AVPacket.

		if(frame_finished == 0)	
			return nullptr;

		is_progressive_ = !decoded_frame->interlaced_frame;

		if(decoded_frame->repeat_pict > 0)
			CASPAR_LOG(warning) << "[video_decoder] Field repeat_pict not implemented.";
		
		return decoded_frame;
	}
	
	bool ready() const
	{
		return packets_.size() > 10;
	}
	
	double fps() const
	{
		return fps_;
	}
};

video_decoder::video_decoder(const safe_ptr<AVFormatContext>& context) : impl_(new implementation(context)){}
void video_decoder::push(const std::shared_ptr<AVPacket>& packet){impl_->push(packet);}
std::shared_ptr<AVFrame> video_decoder::poll(){return impl_->poll();}
bool video_decoder::ready() const{return impl_->ready();}
double video_decoder::fps() const{return impl_->fps();}
int64_t video_decoder::nb_frames() const{return impl_->nb_frames_;}
size_t video_decoder::width() const{return impl_->width_;}
size_t video_decoder::height() const{return impl_->height_;}
bool	video_decoder::is_progressive() const{return impl_->is_progressive_;}

}}