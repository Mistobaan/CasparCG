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

#include "decklink_producer.h"

#include "../interop/DeckLinkAPI_h.h"
#include "../util/util.h"

#include "../../ffmpeg/producer/filter/filter.h"
#include "../../ffmpeg/producer/util/util.h"
#include "../../ffmpeg/producer/muxer/frame_muxer.h"

#include <common/log/log.h>
#include <common/diagnostics/graph.h>
#include <common/concurrency/com_context.h>
#include <common/exception/exceptions.h>
#include <common/memory/memclr.h>

#include <core/mixer/write_frame.h>
#include <core/producer/frame/frame_transform.h>
#include <core/producer/frame/frame_factory.h>

#include <tbb/concurrent_queue.h>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/timer.hpp>

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable : 4244)
#endif
extern "C" 
{
	#define __STDC_CONSTANT_MACROS
	#define __STDC_LIMIT_MACROS
	#include <libavcodec/avcodec.h>
}
#if defined(_MSC_VER)
#pragma warning (pop)
#endif

#pragma warning(push)
#pragma warning(disable : 4996)

	#include <atlbase.h>

	#include <atlcom.h>
	#include <atlhost.h>

#pragma warning(push)

#include <functional>

namespace caspar { namespace decklink {
		
class decklink_producer : boost::noncopyable, public IDeckLinkInputCallback
{	
	CComPtr<IDeckLink>											decklink_;
	CComQIPtr<IDeckLinkInput>									input_;
	
	const std::wstring											model_name_;
	const core::video_format_desc								format_desc_;
	const size_t												device_index_;

	safe_ptr<diagnostics::graph>								graph_;
	boost::timer												tick_timer_;
	boost::timer												frame_timer_;
		
	safe_ptr<core::frame_factory>								frame_factory_;

	tbb::concurrent_bounded_queue<safe_ptr<core::basic_frame>>	frame_buffer_;

	std::exception_ptr											exception_;
		
	ffmpeg::frame_muxer											muxer_;

public:
	decklink_producer(const core::video_format_desc& format_desc, size_t device_index, const safe_ptr<core::frame_factory>& frame_factory, const std::wstring& filter)
		: decklink_(get_device(device_index))
		, input_(decklink_)
		, model_name_(get_model_name(decklink_))
		, format_desc_(format_desc)
		, device_index_(device_index)
		, frame_factory_(frame_factory)
		, muxer_(format_desc.fps, frame_factory, filter)
	{
		frame_buffer_.set_capacity(2);
		
		graph_->add_guide("tick-time", 0.5);
		graph_->set_color("tick-time", diagnostics::color(0.0f, 0.6f, 0.9f));	
		graph_->set_color("late-frame", diagnostics::color(0.6f, 0.3f, 0.3f));
		graph_->set_color("frame-time", diagnostics::color(1.0f, 0.0f, 0.0f));
		graph_->set_color("dropped-frame", diagnostics::color(0.3f, 0.6f, 0.3f));
		graph_->set_color("output-buffer", diagnostics::color(0.0f, 1.0f, 0.0f));
		graph_->set_text(print());
		diagnostics::register_graph(graph_);
		
		auto display_mode = get_display_mode(input_, format_desc_.format, bmdFormat8BitYUV, bmdVideoInputFlagDefault);
		
		// NOTE: bmdFormat8BitARGB is currently not supported by any decklink card. (2011-05-08)
		if(FAILED(input_->EnableVideoInput(display_mode, bmdFormat8BitYUV, 0))) 
			BOOST_THROW_EXCEPTION(caspar_exception() 
									<< msg_info(narrow(print()) + " Could not enable video input.")
									<< boost::errinfo_api_function("EnableVideoInput"));

		if(FAILED(input_->EnableAudioInput(bmdAudioSampleRate48kHz, bmdAudioSampleType32bitInteger, format_desc_.audio_channels))) 
			BOOST_THROW_EXCEPTION(caspar_exception() 
									<< msg_info(narrow(print()) + " Could not enable audio input.")
									<< boost::errinfo_api_function("EnableAudioInput"));
			
		if (FAILED(input_->SetCallback(this)) != S_OK)
			BOOST_THROW_EXCEPTION(caspar_exception() 
									<< msg_info(narrow(print()) + " Failed to set input callback.")
									<< boost::errinfo_api_function("SetCallback"));
			
		if(FAILED(input_->StartStreams()))
			BOOST_THROW_EXCEPTION(caspar_exception() 
									<< msg_info(narrow(print()) + " Failed to start input stream.")
									<< boost::errinfo_api_function("StartStreams"));

		CASPAR_LOG(info) << print() << L" Successfully Initialized.";
	}

	~decklink_producer()
	{
		if(input_ != nullptr) 
		{
			input_->StopStreams();
			input_->DisableVideoInput();
		}
	}

	virtual HRESULT STDMETHODCALLTYPE	QueryInterface (REFIID, LPVOID*)	{return E_NOINTERFACE;}
	virtual ULONG STDMETHODCALLTYPE		AddRef ()							{return 1;}
	virtual ULONG STDMETHODCALLTYPE		Release ()							{return 1;}
		
	virtual HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(BMDVideoInputFormatChangedEvents /*notificationEvents*/, IDeckLinkDisplayMode* newDisplayMode, BMDDetectedVideoInputFormatFlags /*detectedSignalFlags*/)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame* video, IDeckLinkAudioInputPacket* audio)
	{	
		if(!video)
			return S_OK;

		try
		{
			graph_->update_value("tick-time", tick_timer_.elapsed()*format_desc_.fps*0.5);
			tick_timer_.restart();

			frame_timer_.restart();

			void* bytes = nullptr;
			if(FAILED(video->GetBytes(&bytes)) || !bytes)
				return S_OK;
			
			safe_ptr<AVFrame> av_frame(avcodec_alloc_frame(), av_free);	
			avcodec_get_frame_defaults(av_frame.get());
						
			av_frame->data[0]			= reinterpret_cast<uint8_t*>(bytes);
			av_frame->linesize[0]		= video->GetRowBytes();			
			av_frame->format			= PIX_FMT_UYVY422;
			av_frame->width				= video->GetWidth();
			av_frame->height			= video->GetHeight();
			av_frame->interlaced_frame	= format_desc_.field_mode != core::field_mode::progressive;
			av_frame->top_field_first	= format_desc_.field_mode == core::field_mode::upper ? 1 : 0;
					
			muxer_.push(av_frame);		
									
			// It is assumed that audio is always equal or ahead of video.
			if(audio && SUCCEEDED(audio->GetBytes(&bytes)))
			{
				auto sample_frame_count = audio->GetSampleFrameCount();
				auto audio_data = reinterpret_cast<int32_t*>(bytes);
				muxer_.push(std::make_shared<core::audio_buffer>(audio_data, audio_data + sample_frame_count*format_desc_.audio_channels));
			}
			else
				muxer_.push(std::make_shared<core::audio_buffer>(frame_factory_->get_video_format_desc().audio_samples_per_frame, 0));
					
			for(auto frame = muxer_.poll(); frame; frame = muxer_.poll())
			{
				if(!frame_buffer_.try_push(make_safe_ptr(frame)))
					graph_->add_tag("dropped-frame");
			}

			graph_->update_value("frame-time", frame_timer_.elapsed()*format_desc_.fps*0.5);

			graph_->set_value("output-buffer", static_cast<float>(frame_buffer_.size())/static_cast<float>(frame_buffer_.capacity()));	
		}
		catch(...)
		{
			exception_ = std::current_exception();
			return E_FAIL;
		}

		return S_OK;
	}
	
	safe_ptr<core::basic_frame> get_frame()
	{
		if(exception_ != nullptr)
			std::rethrow_exception(exception_);

		safe_ptr<core::basic_frame> frame = core::basic_frame::late();
		if(!frame_buffer_.try_pop(frame))
			graph_->add_tag("late-frame");
		graph_->set_value("output-buffer", static_cast<float>(frame_buffer_.size())/static_cast<float>(frame_buffer_.capacity()));	
		return frame;
	}
	
	std::wstring print() const
	{
		return model_name_ + L" [" + boost::lexical_cast<std::wstring>(device_index_) + L"]";
	}
};
	
class decklink_producer_proxy : public core::frame_producer
{		
	safe_ptr<core::basic_frame>		last_frame_;
	com_context<decklink_producer>	context_;
	const int64_t					length_;
public:

	explicit decklink_producer_proxy(const safe_ptr<core::frame_factory>& frame_factory, const core::video_format_desc& format_desc, size_t device_index, const std::wstring& filter_str, int64_t length)
		: context_(L"decklink_producer[" + boost::lexical_cast<std::wstring>(device_index) + L"]")
		, last_frame_(core::basic_frame::empty())
		, length_(length)
	{
		context_.reset([&]{return new decklink_producer(format_desc, device_index, frame_factory, filter_str);}); 
	}

	~decklink_producer_proxy()
	{
		auto str = print();
		context_.reset();
		CASPAR_LOG(info) << str << L" Successfully Uninitialized.";	
	}
				
	virtual safe_ptr<core::basic_frame> receive(int)
	{
		auto frame = context_->get_frame();
		if(frame != core::basic_frame::late())
			last_frame_ = frame;
		return frame;
	}

	virtual safe_ptr<core::basic_frame> last_frame() const
	{
		return disable_audio(last_frame_);
	}
	
	virtual int64_t nb_frames() const 
	{
		return length_;
	}
	
	std::wstring print() const
	{
		return context_->print();
	}
};

safe_ptr<core::frame_producer> create_producer(const safe_ptr<core::frame_factory>& frame_factory, const std::vector<std::wstring>& params)
{
	if(params.empty() || !boost::iequals(params[0], "decklink"))
		return core::frame_producer::empty();

	auto device_index	= core::get_param(L"DEVICE", params, 1);
	auto filter_str		= core::get_param<std::wstring>(L"FILTER", params, L""); 	
	auto length			= core::get_param(L"LENGTH", params, std::numeric_limits<int64_t>::max()); 	
	
	boost::replace_all(filter_str, L"DEINTERLACE", L"YADIF=0:-1");
	boost::replace_all(filter_str, L"DEINTERLACE_BOB", L"YADIF=1:-1");

	auto format_desc	= core::video_format_desc::get(core::get_param<std::wstring>(L"FORMAT", params, L"INVALID"));

	if(format_desc.format == core::video_format::invalid)
		format_desc = frame_factory->get_video_format_desc();
			
	return create_destroy_proxy(make_safe<decklink_producer_proxy>(frame_factory, format_desc, device_index, filter_str, length));
}

}}