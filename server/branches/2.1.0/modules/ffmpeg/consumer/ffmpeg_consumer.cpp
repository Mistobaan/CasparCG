/*
* Copyright (c) 2011 Sveriges Television AB <info@casparcg.com>
*
* This file is part of CasparCG (www.casparcg.com).
*
* CasparCG is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CasparCG is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CasparCG. If not, see <http://www.gnu.org/licenses/>.
*
* Author: Robert Nagy, ronag89@gmail.com
*/
 
#include "../StdAfx.h"

#include "../ffmpeg_error.h"

#include "ffmpeg_consumer.h"

#include "../producer/tbb_avcodec.h"

#include <core/frame/frame.h>
#include <core/mixer/audio/audio_util.h>
#include <core/consumer/frame_consumer.h>
#include <core/video_format.h>

#include <common/array.h>
#include <common/env.h>
#include <common/except.h>
#include <common/executor.h>
#include <common/diagnostics/graph.h>
#include <common/lock.h>
#include <common/memory.h>
#include <common/param.h>
#include <common/utf.h>
#include <common/assert.h>

#include <boost/algorithm/string.hpp>
#include <boost/timer.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/lexical_cast.hpp>

#include <tbb/spin_mutex.h>

#include <numeric>

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable : 4244)
#endif
extern "C" 
{
	#define __STDC_CONSTANT_MACROS
	#define __STDC_LIMIT_MACROS
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <libavutil/opt.h>
	#include <libavutil/pixdesc.h>
	#include <libavutil/parseutils.h>
	#include <libavutil/samplefmt.h>
	#include <libswresample/swresample.h>
}
#if defined(_MSC_VER)
#pragma warning (pop)
#endif

namespace caspar { namespace ffmpeg {
	
int av_opt_set(void *obj, const char *name, const char *val, int search_flags)
{
	AVClass* av_class = *(AVClass**)obj;

	if((strcmp(name, "pix_fmt") == 0 || strcmp(name, "pixel_format") == 0) && strcmp(av_class->class_name, "AVCodecContext") == 0)
	{
		AVCodecContext* c = (AVCodecContext*)obj;		
		auto pix_fmt = av_get_pix_fmt(val);
		if(pix_fmt == PIX_FMT_NONE)
			return -1;		
		c->pix_fmt = pix_fmt;
		return 0;
	}
	//if((strcmp(name, "r") == 0 || strcmp(name, "frame_rate") == 0) && strcmp(av_class->class_name, "AVCodecContext") == 0)
	//{
	//	AVCodecContext* c = (AVCodecContext*)obj;	

	//	if(c->codec_type != AVMEDIA_TYPE_VIDEO)
	//		return -1;

	//	AVRational rate;
	//	int ret = av_parse_video_rate(&rate, val);
	//	if(ret < 0)
	//		return ret;

	//	c->time_base.num = rate.den;
	//	c->time_base.den = rate.num;
	//	return 0;
	//}

	return ::av_opt_set(obj, name, val, search_flags);
}

struct option
{
	std::string name;
	std::string value;

	option(std::string name, std::string value)
		: name(std::move(name))
		, value(std::move(value))
	{
	}
};
	
struct output_format
{
	AVOutputFormat* format;
	int				width;
	int				height;
	CodecID			vcodec;
	CodecID			acodec;
	int				croptop;
	int				cropbot;

	output_format(const core::video_format_desc& format_desc, const std::string& filename, std::vector<option>& options)
		: format(av_guess_format(nullptr, filename.c_str(), nullptr))
		, width(format_desc.width)
		, height(format_desc.height)
		, vcodec(CODEC_ID_NONE)
		, acodec(CODEC_ID_NONE)
		, croptop(0)
		, cropbot(0)
	{
		if(boost::iequals(boost::filesystem::path(filename).extension().string(), ".dv"))
			set_opt("f", "dv");

		boost::range::remove_erase_if(options, [&](const option& o)
		{
			return set_opt(o.name, o.value);
		});
		
		if(vcodec == CODEC_ID_NONE && format)
			vcodec = format->video_codec;

		if(acodec == CODEC_ID_NONE && format)
			acodec = format->audio_codec;
		
		if(vcodec == CODEC_ID_NONE)
			vcodec = CODEC_ID_H264;
		
		if(acodec == CODEC_ID_NONE)
			acodec = CODEC_ID_PCM_S16LE;
	}
	
	bool set_opt(const std::string& name, const std::string& value)
	{
		//if(name == "target")
		//{ 
		//	enum { PAL, NTSC, FILM, UNKNOWN } norm = UNKNOWN;
		//	
		//	if(name.find("pal-") != std::string::npos)
		//		norm = PAL;
		//	else if(name.find("ntsc-") != std::string::npos)
		//		norm = NTSC;

		//	if(norm == UNKNOWN)
		//		CASPAR_THROW_EXCEPTION(invalid_argument() << arg_name_info("target"));
		//	
		//	if (name.find("-dv") != std::string::npos) 
		//	{
		//		set_opt("f", "dv");
		//		if(norm == PAL)
		//		{
		//			set_opt("s", "720x576");
		//		}
		//		else
		//		{
		//			set_opt("s", "720x480");
		//			if(height == 486)
		//			{
		//				set_opt("croptop", "2");
		//				set_opt("cropbot", "4");
		//			}
		//		}
		//		set_opt("s", norm == PAL ? "720x576" : "720x480");
		//	} 

		//	return true;
		//}
		//else 
		if(name == "f")
		{
			format = av_guess_format(value.c_str(), nullptr, nullptr);

			if(format == nullptr)
				CASPAR_THROW_EXCEPTION(invalid_argument() << arg_name_info("f"));

			return true;
		}
		else if(name == "vcodec" || name == "v:codec")
		{
			auto c = avcodec_find_encoder_by_name(value.c_str());
			if(c == nullptr)
				CASPAR_THROW_EXCEPTION(invalid_argument() << arg_name_info("vcodec"));

			vcodec = avcodec_find_encoder_by_name(value.c_str())->id;
			return true;

		}
		else if(name == "acodec" || name == "a:codec")
		{
			auto c = avcodec_find_encoder_by_name(value.c_str());
			if(c == nullptr)
				CASPAR_THROW_EXCEPTION(invalid_argument() << arg_name_info("acodec"));

			acodec = avcodec_find_encoder_by_name(value.c_str())->id;

			return true;
		}
		else if(name == "s")
		{
			if(av_parse_video_size(&width, &height, value.c_str()) < 0)
				CASPAR_THROW_EXCEPTION(invalid_argument() << arg_name_info("s"));
			
			return true;
		}
		else if(name == "croptop")
		{
			croptop = boost::lexical_cast<int>(value);

			return true;
		}
		else if(name == "cropbot")
		{
			cropbot = boost::lexical_cast<int>(value);

			return true;
		}
		
		return false;
	}
};

typedef std::vector<uint8_t, tbb::cache_aligned_allocator<uint8_t>>	byte_vector;

struct ffmpeg_consumer : boost::noncopyable
{		
	const spl::shared_ptr<diagnostics::graph>	graph_;
	const std::string							filename_;		
	const std::shared_ptr<AVFormatContext>		oc_;
	const core::video_format_desc				format_desc_;	

	monitor::basic_subject						event_subject_;
	
	tbb::spin_mutex								exception_mutex_;
	std::exception_ptr							exception_;
	
	std::shared_ptr<AVStream>					audio_st_;
	std::shared_ptr<AVStream>					video_st_;
	
	byte_vector									picture_buffer_;
	byte_vector									audio_buffer_;
	std::shared_ptr<SwrContext>					swr_;
	std::shared_ptr<SwsContext>					sws_;

	int64_t										frame_number_;

	output_format								output_format_;
	
	executor									executor_;
public:
	ffmpeg_consumer(const std::string& filename, const core::video_format_desc& format_desc, std::vector<option> options)
		: filename_(filename)
		, oc_(avformat_alloc_context(), av_free)
		, format_desc_(format_desc)
		, frame_number_(0)
		, output_format_(format_desc, filename, options)
		, executor_(print())
	{
		check_space();

		// TODO: Ask stakeholders about case where file already exists.
		boost::filesystem::remove(boost::filesystem::path(env::media_folder() + u16(filename))); // Delete the file if it exists

		graph_->set_color("frame-time", diagnostics::color(0.1f, 1.0f, 0.1f));
		graph_->set_text(print());
		diagnostics::register_graph(graph_);

		executor_.set_capacity(8);

		oc_->oformat = output_format_.format;
				
		strcpy_s(oc_->filename, filename_.c_str());
		
		//  Add the audio and video streams using the default format codecs	and initialize the codecs.
		video_st_ = add_video_stream(options);
		audio_st_ = add_audio_stream(options);
				
		av_dump_format(oc_.get(), 0, filename_.c_str(), 1);
		 
		// Open the output ffmpeg, if needed.
		if (!(oc_->oformat->flags & AVFMT_NOFILE)) 
			THROW_ON_ERROR2(avio_open(&oc_->pb, filename.c_str(), AVIO_FLAG_WRITE), "[ffmpeg_consumer]");
				
		THROW_ON_ERROR2(avformat_write_header(oc_.get(), nullptr), "[ffmpeg_consumer]");

		if(options.size() > 0)
		{
			BOOST_FOREACH(auto& option, options)
				CASPAR_LOG(warning) << L"Invalid option: -" << u16(option.name) << L" " << u16(option.value);
		}

		CASPAR_LOG(info) << print() << L" Successfully Initialized.";	
	}

	~ffmpeg_consumer()
	{    
		try
		{
			executor_.wait();
		}
		catch(...)
		{
			CASPAR_LOG_CURRENT_EXCEPTION();
		}

		LOG_ON_ERROR2(av_write_trailer(oc_.get()), "[ffmpeg_consumer]");
		
		audio_st_.reset();
		video_st_.reset();
			  
		if (!(oc_->oformat->flags & AVFMT_NOFILE)) 
			LOG_ON_ERROR2(avio_close(oc_->pb), "[ffmpeg_consumer]");

		CASPAR_LOG(info) << print() << L" Successfully Uninitialized.";	
	}
	
	// frame_consumer

	boost::unique_future<bool> send(core::const_frame& frame)
	{
		auto exception = lock(exception_mutex_, [&]
		{
			return exception_;
		});

		if(exception != nullptr)
			std::rethrow_exception(exception);
			
		return executor_.begin_invoke([=]() -> bool
		{		
			encode(frame);

			return true;
		});
	}

	std::wstring print() const
	{
		return L"ffmpeg[" + u16(filename_) + L"]";
	}
	
	void subscribe(const monitor::observable::observer_ptr& o)
	{
		event_subject_.subscribe(o);
	}

	void unsubscribe(const monitor::observable::observer_ptr& o)
	{
		event_subject_.unsubscribe(o);
	}		

private:
	std::shared_ptr<AVStream> add_video_stream(std::vector<option>& options)
	{ 
		if(output_format_.vcodec == CODEC_ID_NONE)
			return nullptr;

		auto st = av_new_stream(oc_.get(), 0);
		if (!st) 		
			CASPAR_THROW_EXCEPTION(caspar_exception() << msg_info("Could not allocate video-stream.") << boost::errinfo_api_function("av_new_stream"));		

		auto encoder = avcodec_find_encoder(output_format_.vcodec);
		if (!encoder)
			CASPAR_THROW_EXCEPTION(caspar_exception() << msg_info("Codec not found."));

		auto c = st->codec;

		avcodec_get_context_defaults3(c, encoder);
				
		c->codec_id			= output_format_.vcodec;
		c->codec_type		= AVMEDIA_TYPE_VIDEO;
		c->width			= output_format_.width;
		c->height			= output_format_.height - output_format_.croptop - output_format_.cropbot;
		c->time_base.den	= format_desc_.time_scale;
		c->time_base.num	= format_desc_.duration;
		c->gop_size			= 25;
		c->flags		   |= format_desc_.field_mode == core::field_mode::progressive ? 0 : (CODEC_FLAG_INTERLACED_ME | CODEC_FLAG_INTERLACED_DCT);
		c->pix_fmt			= c->pix_fmt != PIX_FMT_NONE ? c->pix_fmt : PIX_FMT_YUV420P;

		if(c->codec_id == CODEC_ID_PRORES)
		{			
			c->bit_rate	= output_format_.width < 1280 ? 63*1000000 : 220*1000000;
			c->pix_fmt	= PIX_FMT_YUV422P10;
		}
		else if(c->codec_id == CODEC_ID_DNXHD)
		{
			if(c->width < 1280 || c->height < 720)
				CASPAR_THROW_EXCEPTION(caspar_exception() << msg_info("Unsupported video dimensions."));

			c->bit_rate	= 220*1000000;
			c->pix_fmt	= PIX_FMT_YUV422P;
		}
		else if(c->codec_id == CODEC_ID_DVVIDEO)
		{
			c->width = c->height == 1280 ? 960  : c->width;
			
			if(format_desc_.format == core::video_format::ntsc)
			{
				c->pix_fmt = PIX_FMT_YUV411P;
				output_format_.croptop = 2;
				output_format_.cropbot = 4;
				c->height			   = output_format_.height - output_format_.croptop - output_format_.cropbot;
			}
			else if(format_desc_.format == core::video_format::pal)
				c->pix_fmt = PIX_FMT_YUV420P;
			else // dv50
				c->pix_fmt = PIX_FMT_YUV422P;
			
			if(format_desc_.duration == 1001)			
				c->width = c->height == 1080 ? 1280 : c->width;			
			else
				c->width = c->height == 1080 ? 1440 : c->width;			
		}
		else if(c->codec_id == CODEC_ID_H264)
		{			   
			c->pix_fmt = PIX_FMT_YUV420P;    
			av_opt_set(c->priv_data, "preset", "ultrafast", 0);
			av_opt_set(c->priv_data, "tune",   "fastdecode",   0);
			av_opt_set(c->priv_data, "crf",    "5",     0);
		}
		else if(c->codec_id == CODEC_ID_QTRLE)
		{
			c->pix_fmt = PIX_FMT_ARGB;
		}
								
		boost::range::remove_erase_if(options, [&](const option& o)
		{
			return o.name.at(0) != 'a' && ffmpeg::av_opt_set(c, o.name.c_str(), o.value.c_str(), AV_OPT_SEARCH_CHILDREN) > -1;
		});
				
		if(output_format_.format->flags & AVFMT_GLOBALHEADER)
			c->flags |= CODEC_FLAG_GLOBAL_HEADER;
		
		THROW_ON_ERROR2(tbb_avcodec_open(c, encoder), "[ffmpeg_consumer]");

		return std::shared_ptr<AVStream>(st, [](AVStream* st)
		{
			LOG_ON_ERROR2(tbb_avcodec_close(st->codec), "[ffmpeg_consumer]");
			av_freep(&st->codec);
			av_freep(&st);
		});
	}
		
	std::shared_ptr<AVStream> add_audio_stream(std::vector<option>& options)
	{
		if(output_format_.acodec == CODEC_ID_NONE)
			return nullptr;

		auto st = av_new_stream(oc_.get(), 1);
		if(!st)
			CASPAR_THROW_EXCEPTION(caspar_exception() << msg_info("Could not allocate audio-stream") << boost::errinfo_api_function("av_new_stream"));		
		
		auto encoder = avcodec_find_encoder(output_format_.acodec);
		if (!encoder)
			CASPAR_THROW_EXCEPTION(caspar_exception() << msg_info("codec not found"));
		
		auto c = st->codec;

		avcodec_get_context_defaults3(c, encoder);

		c->codec_id			= output_format_.acodec;
		c->codec_type		= AVMEDIA_TYPE_AUDIO;
		c->sample_rate		= 48000;
		c->channels			= 2;
		c->sample_fmt		= AV_SAMPLE_FMT_S16;
		c->time_base.num	= 1;
		c->time_base.den	= c->sample_rate;

		if(output_format_.vcodec == CODEC_ID_FLV1)		
			c->sample_rate	= 44100;		

		if(output_format_.format->flags & AVFMT_GLOBALHEADER)
			c->flags |= CODEC_FLAG_GLOBAL_HEADER;
				
		boost::range::remove_erase_if(options, [&](const option& o)
		{
			return ffmpeg::av_opt_set(c, o.name.c_str(), o.value.c_str(), AV_OPT_SEARCH_CHILDREN) > -1;
		});

		THROW_ON_ERROR2(avcodec_open(c, encoder), "[ffmpeg_consumer]");

		return std::shared_ptr<AVStream>(st, [](AVStream* st)
		{
			LOG_ON_ERROR2(avcodec_close(st->codec), "[ffmpeg_consumer]");;
			av_freep(&st->codec);
			av_freep(&st);
		});
	}
  
	void encode_video_frame(core::const_frame frame)
	{ 
		if(!video_st_)
			return;
		
		auto enc = video_st_->codec;
	 
		auto av_frame				= convert_video(frame, enc);
		av_frame->interlaced_frame	= format_desc_.field_mode != core::field_mode::progressive;
		av_frame->top_field_first	= format_desc_.field_mode == core::field_mode::upper;
		av_frame->pts				= frame_number_++;

		event_subject_ << monitor::event("frame")	% static_cast<int64_t>(frame_number_)
													% static_cast<int64_t>(std::numeric_limits<int64_t>::max());

		AVPacket pkt;
		av_init_packet(&pkt);
		pkt.data = nullptr;
		pkt.size = 0;

		int got_packet = 0;
		THROW_ON_ERROR2(avcodec_encode_video2(enc, &pkt, av_frame.get(), &got_packet), "[ffmpeg_consumer]");
		std::shared_ptr<AVPacket> guard(&pkt, av_free_packet);

		if(!got_packet)
			return;
		 
		if (pkt.pts != AV_NOPTS_VALUE)
			pkt.pts = av_rescale_q(pkt.pts, enc->time_base, video_st_->time_base);
		if (pkt.dts != AV_NOPTS_VALUE)
			pkt.dts = av_rescale_q(pkt.dts, enc->time_base, video_st_->time_base);
		 
		pkt.stream_index = video_st_->index;
			
		THROW_ON_ERROR2(av_interleaved_write_frame(oc_.get(), &pkt), "[ffmpeg_consumer]");
	}
		
	uint64_t get_channel_layout(AVCodecContext* dec)
	{
		auto layout = (dec->channel_layout && dec->channels == av_get_channel_layout_nb_channels(dec->channel_layout)) ? dec->channel_layout : av_get_default_channel_layout(dec->channels);
		return layout;
	}
		
	void encode_audio_frame(core::const_frame frame)
	{		
		if(!audio_st_)
			return;
		
		auto enc = audio_st_->codec;

		boost::push_back(audio_buffer_, convert_audio(frame, enc));
			
		auto frame_size = enc->frame_size != 0 ? enc->frame_size * enc->channels * av_get_bytes_per_sample(enc->sample_fmt) : static_cast<int>(audio_buffer_.size());
			
		while(audio_buffer_.size() >= frame_size)
		{			
			std::shared_ptr<AVFrame> av_frame(avcodec_alloc_frame(), av_free);
			avcodec_get_frame_defaults(av_frame.get());		
			av_frame->nb_samples = frame_size / (enc->channels * av_get_bytes_per_sample(enc->sample_fmt));

			AVPacket pkt;
			av_init_packet(&pkt);
			pkt.data = nullptr;
			pkt.size = 0;				
			
			THROW_ON_ERROR2(avcodec_fill_audio_frame(av_frame.get(), enc->channels, enc->sample_fmt, audio_buffer_.data(), frame_size, 1), "[ffmpeg_consumer]");

			int got_packet = 0;
			THROW_ON_ERROR2(avcodec_encode_audio2(enc, &pkt, av_frame.get(), &got_packet), "[ffmpeg_consumer]");
			std::shared_ptr<AVPacket> guard(&pkt, av_free_packet);
				
			audio_buffer_.erase(audio_buffer_.begin(), audio_buffer_.begin() + frame_size);

			if(!got_packet)
				return;
		
			if (pkt.pts != AV_NOPTS_VALUE)
				pkt.pts      = av_rescale_q(pkt.pts, enc->time_base, audio_st_->time_base);
			if (pkt.dts != AV_NOPTS_VALUE)
				pkt.dts      = av_rescale_q(pkt.dts, enc->time_base, audio_st_->time_base);
			if (pkt.duration > 0)
				pkt.duration = static_cast<int>(av_rescale_q(pkt.duration, enc->time_base, audio_st_->time_base));
		
			pkt.stream_index = audio_st_->index;
						
			THROW_ON_ERROR2(av_interleaved_write_frame(oc_.get(), &pkt), "[ffmpeg_consumer]");
		}
	}		 
	
	std::shared_ptr<AVFrame> convert_video(core::const_frame frame, AVCodecContext* c)
	{
		if(!sws_) 
		{
			sws_.reset(sws_getContext(format_desc_.width, 
									  format_desc_.height - output_format_.croptop  - output_format_.cropbot, 
									  PIX_FMT_BGRA,
									  c->width,
									  c->height, 
									  c->pix_fmt, 
									  SWS_BICUBIC, nullptr, nullptr, nullptr), 
						sws_freeContext);
			if (sws_ == nullptr) 
				CASPAR_THROW_EXCEPTION(caspar_exception() << msg_info("Cannot initialize the conversion context"));
		}

		// #in_frame

		std::shared_ptr<AVFrame> in_frame(avcodec_alloc_frame(), av_free);

		avpicture_fill(reinterpret_cast<AVPicture*>(in_frame.get()), 
					   const_cast<uint8_t*>(frame.image_data().begin()),
					   PIX_FMT_BGRA, 
					   format_desc_.width,
					   format_desc_.height - output_format_.croptop  - output_format_.cropbot);
		// crop-top

		for(int n = 0; n < 4; ++n)		
			in_frame->data[n] += in_frame->linesize[n] * output_format_.croptop;		
		
		// #out_frame

		std::shared_ptr<AVFrame> out_frame(avcodec_alloc_frame(), av_free);
		
		av_image_fill_linesizes(out_frame->linesize, c->pix_fmt, c->width);
		for(int n = 0; n < 4; ++n)
			out_frame->linesize[n] += 32 - (out_frame->linesize[n] % 32); // align

		picture_buffer_.resize(av_image_fill_pointers(out_frame->data, c->pix_fmt, c->height, nullptr, out_frame->linesize));
		av_image_fill_pointers(out_frame->data, c->pix_fmt, c->height, picture_buffer_.data(), out_frame->linesize);
		
		// #scale

		sws_scale(sws_.get(), 
				  in_frame->data, 
				  in_frame->linesize,
				  0, 
				  format_desc_.height - output_format_.cropbot - output_format_.croptop, 
				  out_frame->data, 
				  out_frame->linesize);

		return out_frame;
	}
	
	byte_vector convert_audio(core::const_frame& frame, AVCodecContext* c)
	{
		if(!swr_) 
		{
			swr_ = std::shared_ptr<SwrContext>(swr_alloc_set_opts(nullptr,
										get_channel_layout(c), c->sample_fmt, c->sample_rate,
										av_get_default_channel_layout(format_desc_.audio_channels), AV_SAMPLE_FMT_S32, format_desc_.audio_sample_rate,
										0, nullptr), [](SwrContext* p){swr_free(&p);});

			if(!swr_)
				CASPAR_THROW_EXCEPTION(bad_alloc());

			THROW_ON_ERROR2(swr_init(swr_.get()), "[audio_decoder]");
		}
				
		byte_vector buffer(48000);

		const uint8_t* in[]  = {reinterpret_cast<const uint8_t*>(frame.audio_data().data())};
		uint8_t*       out[] = {buffer.data()};

		auto channel_samples = swr_convert(swr_.get(), 
										   out, static_cast<int>(buffer.size()) / c->channels / av_get_bytes_per_sample(c->sample_fmt), 
										   in, static_cast<int>(frame.audio_data().size()/format_desc_.audio_channels));

		buffer.resize(channel_samples * c->channels * av_get_bytes_per_sample(c->sample_fmt));	

		return buffer;
	}

	void check_space()
	{
		auto space = boost::filesystem::space(boost::filesystem::path(filename_).parent_path());
		if(space.available < 512*1000000)
			BOOST_THROW_EXCEPTION(file_write_error() << msg_info("out of space"));
	}

	void encode(const core::const_frame& frame)
	{
		try
		{
			if(frame_number_ % 25 == 0)
				check_space();

			boost::timer frame_timer;

			encode_video_frame(frame);
			encode_audio_frame(frame);

			graph_->set_value("frame-time", frame_timer.elapsed()*format_desc_.fps*0.5);
		}
		catch(...)
		{			
			lock(exception_mutex_, [&]
			{
				exception_ = std::current_exception();
			});
		}
	}
};

struct ffmpeg_consumer_proxy : public core::frame_consumer
{
	const std::wstring				filename_;
	const std::vector<option>		options_;

	std::unique_ptr<ffmpeg_consumer> consumer_;

public:

	ffmpeg_consumer_proxy(const std::wstring& filename, const std::vector<option>& options)
		: filename_(filename)
		, options_(options)
	{
	}
	
	virtual void initialize(const core::video_format_desc& format_desc, int)
	{
		if(consumer_)
			BOOST_THROW_EXCEPTION(invalid_operation() << msg_info("Cannot reinitialize ffmpeg-consumer."));

		consumer_.reset(new ffmpeg_consumer(u8(filename_), format_desc, options_));
	}
	
	boost::unique_future<bool> send(core::const_frame frame) override
	{
		return consumer_->send(frame);
	}
	
	std::wstring print() const override
	{
		return consumer_ ? consumer_->print() : L"[ffmpeg_consumer]";
	}

	std::wstring name() const override
	{
		return L"file";
	}

	boost::property_tree::wptree info() const override
	{
		boost::property_tree::wptree info;
		info.add(L"type", L"file");
		info.add(L"filename", filename_);
		return info;
	}
		
	bool has_synchronization_clock() const override
	{
		return false;
	}

	int buffer_depth() const override
	{
		return 1;
	}

	int index() const override
	{
		return 200;
	}

	void subscribe(const monitor::observable::observer_ptr& o) override
	{
		consumer_->subscribe(o);
	}

	void unsubscribe(const monitor::observable::observer_ptr& o) override
	{
		consumer_->unsubscribe(o);
	}		
};	
spl::shared_ptr<core::frame_consumer> create_consumer(const std::vector<std::wstring>& params)
{
	auto str = std::accumulate(params.begin(), params.end(), std::wstring(), [](const std::wstring& lhs, const std::wstring& rhs) {return lhs + L" " + rhs;});
	
	boost::wregex path_exp(L"\\s*FILE(\\s(?<PATH>.+\\.[^\\s]+))?.*", boost::regex::icase);

	boost::wsmatch path;
	if(!boost::regex_match(str, path, path_exp))
		return core::frame_consumer::empty();
	
	boost::wregex opt_exp(L"-((?<NAME>[^\\s]+)\\s+(?<VALUE>[^\\s]+))");	
	
	std::vector<option> options;
	for(boost::wsregex_iterator it(str.begin(), str.end(), opt_exp); it != boost::wsregex_iterator(); ++it)
	{
		auto name  = u8(boost::trim_copy(boost::to_lower_copy((*it)["NAME"].str())));
		auto value = u8(boost::trim_copy(boost::to_lower_copy((*it)["VALUE"].str())));
		
		if(value == "h264")
			value = "libx264";
		else if(value == "dvcpro")
			value = "dvvideo";

		options.push_back(option(name, value));
	}
				
	return spl::make_shared<ffmpeg_consumer_proxy>(env::media_folder() + path["PATH"].str(), options);
}

spl::shared_ptr<core::frame_consumer> create_consumer(const boost::property_tree::wptree& ptree)
{
	auto filename	= ptree.get<std::wstring>(L"path");
	auto codec		= ptree.get(L"vcodec", L"libx264");

	std::vector<option> options;
	options.push_back(option("vcodec", u8(codec)));
	
	return spl::make_shared<ffmpeg_consumer_proxy>(env::media_folder() + filename, options);
}

}}
