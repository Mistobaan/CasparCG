﻿/*
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

#include "ogl_consumer.h"

#include <GL/glew.h>
#include <SFML/Window.hpp>

#include <common/diagnostics/graph.h>
#include <common/gl/gl_check.h>
#include <common/log/log.h>
#include <common/memory/safe_ptr.h>
#include <common/memory/memcpy.h>
#include <common/memory/memshfl.h>
#include <common/utility/timer.h>
#include <common/utility/string.h>

#include <ffmpeg/producer/filter/filter.h>

#include <core/video_format.h>
#include <core/mixer/read_frame.h>
#include <core/consumer/frame_consumer.h>

#include <boost/timer.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include <tbb/atomic.h>
#include <tbb/concurrent_queue.h>

#include <boost/assign.hpp>

#include <algorithm>
#include <vector>

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable : 4244)
#endif
extern "C" 
{
	#define __STDC_CONSTANT_MACROS
	#define __STDC_LIMIT_MACROS
	#include <libavcodec/avcodec.h>
	#include <libavutil/imgutils.h>
}
#if defined(_MSC_VER)
#pragma warning (pop)
#endif

namespace caspar { namespace ogl {
		
enum stretch
{
	none,
	uniform,
	fill,
	uniform_to_fill
};

struct configuration
{
	size_t		screen_index;
	stretch		stretch;
	bool		windowed;
	bool		auto_deinterlace;
	bool		key_only;

	configuration()
		: screen_index(0)
		, stretch(fill)
		, windowed(true)
		, auto_deinterlace(true)
		, key_only(false)
	{
	}
};

struct ogl_consumer : boost::noncopyable
{		
	const configuration		config_;
	core::video_format_desc format_desc_;
	
	GLuint					texture_;
	std::vector<GLuint>		pbos_;
	
	float					width_;
	float					height_;	
	unsigned int			screen_x_;
	unsigned int			screen_y_;
	unsigned int			screen_width_;
	unsigned int			screen_height_;
	size_t					square_width_;
	size_t					square_height_;				
	
	sf::Window				window_;
	
	safe_ptr<diagnostics::graph>	graph_;
	boost::timer					perf_timer_;

	boost::circular_buffer<safe_ptr<core::read_frame>>			input_buffer_;
	tbb::concurrent_bounded_queue<safe_ptr<core::read_frame>>	frame_buffer_;

	boost::thread			thread_;
	tbb::atomic<bool>		is_running_;

	
	ffmpeg::filter			filter_;
public:
	ogl_consumer(const configuration& config, const core::video_format_desc& format_desc) 
		: config_(config)
		, format_desc_(format_desc)
		, texture_(0)
		, pbos_(2, 0)	
		, screen_width_(format_desc.width)
		, screen_height_(format_desc.height)
		, square_width_(format_desc.square_width)
		, square_height_(format_desc.square_height)
		, input_buffer_(core::consumer_buffer_depth()-1)
		, filter_(format_desc.field_mode == core::field_mode::progressive || !config.auto_deinterlace ? L"" : L"YADIF=0:-1", boost::assign::list_of(PIX_FMT_BGRA))
	{		
		frame_buffer_.set_capacity(2);

		graph_->add_guide("frame-time", 0.5);
		graph_->set_color("frame-time", diagnostics::color(1.0f, 0.0f, 0.0f));
		graph_->set_color("dropped-frame", diagnostics::color(0.3f, 0.6f, 0.3f));
		graph_->set_text(print());
		diagnostics::register_graph(graph_);
									
		DISPLAY_DEVICE d_device = {sizeof(d_device), 0};			
		std::vector<DISPLAY_DEVICE> displayDevices;
		for(int n = 0; EnumDisplayDevices(NULL, n, &d_device, NULL); ++n)
			displayDevices.push_back(d_device);

		if(config_.screen_index >= displayDevices.size())
			BOOST_THROW_EXCEPTION(out_of_range() << arg_name_info("screen_index_") << msg_info(narrow(print())));
		
		DEVMODE devmode = {};
		if(!EnumDisplaySettings(displayDevices[config_.screen_index].DeviceName, ENUM_CURRENT_SETTINGS, &devmode))
			BOOST_THROW_EXCEPTION(invalid_operation() << arg_name_info("screen_index") << msg_info(narrow(print()) + " EnumDisplaySettings"));
		
		screen_x_		= devmode.dmPosition.x;
		screen_y_		= devmode.dmPosition.y;
		screen_width_	= config_.windowed ? square_width_ : devmode.dmPelsWidth;
		screen_height_	= config_.windowed ? square_height_ : devmode.dmPelsHeight;
		
		is_running_ = true;
		thread_ = boost::thread([this]{run();});
	}
	
	~ogl_consumer()
	{
		is_running_ = false;
		frame_buffer_.try_push(make_safe<core::read_frame>());
		thread_.join();
	}

	void init()
	{
		if(!GLEW_VERSION_2_1)
			BOOST_THROW_EXCEPTION(not_supported() << msg_info("Missing OpenGL 2.1 support."));

		window_.Create(sf::VideoMode(screen_width_, screen_height_, 32), narrow(print()), config_.windowed ? sf::Style::Resize : sf::Style::Fullscreen);
		window_.ShowMouseCursor(false);
		window_.SetPosition(screen_x_, screen_y_);
		window_.SetSize(screen_width_, screen_height_);
		window_.SetActive();
		GL(glEnable(GL_TEXTURE_2D));
		GL(glDisable(GL_DEPTH_TEST));		
		GL(glClearColor(0.0, 0.0, 0.0, 0.0));
		GL(glViewport(0, 0, format_desc_.width, format_desc_.height));
		GL(glLoadIdentity());
				
		calculate_aspect();
			
		GL(glGenTextures(1, &texture_));
		GL(glBindTexture(GL_TEXTURE_2D, texture_));
		GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP));
		GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP));
		GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, format_desc_.width, format_desc_.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0));
		GL(glBindTexture(GL_TEXTURE_2D, 0));
					
		GL(glGenBuffers(2, pbos_.data()));
			
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbos_[0]);
		glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, format_desc_.size, 0, GL_STREAM_DRAW_ARB);
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbos_[1]);
		glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, format_desc_.size, 0, GL_STREAM_DRAW_ARB);
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

		CASPAR_LOG(info) << print() << " Sucessfully Initialized.";
	}

	void uninit()
	{		
		if(texture_)
			glDeleteTextures(1, &texture_);

		BOOST_FOREACH(auto& pbo, pbos_)
		{
			if(pbo)
				glDeleteBuffers(1, &pbo);
		}

		CASPAR_LOG(info) << print() << " Sucessfully Uninitialized.";
	}

	void run()
	{
		try
		{
			init();

			while(is_running_)
			{			
				try
				{
					perf_timer_.restart();

					sf::Event e;		
					while(window_.GetEvent(e))
					{
						if(e.Type == sf::Event::Resized)
							calculate_aspect();
					}
			
					safe_ptr<core::read_frame> frame;
					frame_buffer_.pop(frame);
					render(frame);

					window_.Display();

					graph_->update_value("frame-time", static_cast<float>(perf_timer_.elapsed()*format_desc_.fps*0.5));	
				}
				catch(...)
				{
					CASPAR_LOG_CURRENT_EXCEPTION();
					is_running_ = false;
				}
			}

			uninit();
		}
		catch(...)
		{
			CASPAR_LOG_CURRENT_EXCEPTION();
		}
	}
	
	const core::video_format_desc& get_video_format_desc() const
	{
		return format_desc_;
	}

	safe_ptr<AVFrame> get_av_frame()
	{		
		safe_ptr<AVFrame> av_frame(avcodec_alloc_frame(), av_free);	
		avcodec_get_frame_defaults(av_frame.get());
						
		av_frame->linesize[0]		= format_desc_.width*4;			
		av_frame->format			= PIX_FMT_BGRA;
		av_frame->width				= format_desc_.width;
		av_frame->height			= format_desc_.height;
		av_frame->interlaced_frame	= format_desc_.field_mode != core::field_mode::progressive;
		av_frame->top_field_first	= format_desc_.field_mode == core::field_mode::upper ? 1 : 0;

		return av_frame;
	}

	void render(const safe_ptr<core::read_frame>& frame)
	{			
		if(frame->image_data().empty())
			return;
					
		auto av_frame = get_av_frame();
		av_frame->data[0] = const_cast<uint8_t*>(frame->image_data().begin());

		filter_.push(av_frame);
		auto frames = filter_.poll_all();

		if(frames.empty())
			return;

		av_frame = frames[0];

		if(av_frame->linesize[0] != static_cast<int>(format_desc_.width*4))
		{
			const uint8_t *src_data[4] = {0};
			memcpy(const_cast<uint8_t**>(&src_data[0]), av_frame->data, 4);
			const int src_linesizes[4] = {0};
			memcpy(const_cast<int*>(&src_linesizes[0]), av_frame->linesize, 4);

			auto av_frame2 = get_av_frame();
			av_image_alloc(av_frame2->data, av_frame2->linesize, av_frame2->width, av_frame2->height, PIX_FMT_BGRA, 16);
			av_frame = safe_ptr<AVFrame>(av_frame2.get(), [=](AVFrame*)
			{
				av_freep(&av_frame2->data[0]);
			});

			av_image_copy(av_frame2->data, av_frame2->linesize, src_data, src_linesizes, PIX_FMT_BGRA, av_frame2->width, av_frame2->height);
		}

		glBindTexture(GL_TEXTURE_2D, texture_);

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[0]);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, format_desc_.width, format_desc_.height, GL_BGRA, GL_UNSIGNED_BYTE, 0);

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos_[1]);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, format_desc_.size, 0, GL_STREAM_DRAW);

		auto ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
		if(ptr)
		{
			if(config_.key_only)
				fast_memshfl(reinterpret_cast<char*>(ptr), av_frame->data[0], frame->image_data().size(), 0x0F0F0F0F, 0x0B0B0B0B, 0x07070707, 0x03030303);
			else
				fast_memcpy(reinterpret_cast<char*>(ptr), av_frame->data[0], frame->image_data().size());

			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); // release the mapped buffer
		}

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
				
		GL(glClear(GL_COLOR_BUFFER_BIT));			
		glBegin(GL_QUADS);
				glTexCoord2f(0.0f,	  1.0f);	glVertex2f(-width_, -height_);
				glTexCoord2f(1.0f,	  1.0f);	glVertex2f( width_, -height_);
				glTexCoord2f(1.0f,	  0.0f);	glVertex2f( width_,  height_);
				glTexCoord2f(0.0f,	  0.0f);	glVertex2f(-width_,  height_);
		glEnd();
		
		glBindTexture(GL_TEXTURE_2D, 0);

		std::rotate(pbos_.begin(), pbos_.begin() + 1, pbos_.end());
	}

	void send(const safe_ptr<core::read_frame>& frame)
	{
		input_buffer_.push_back(frame);

		if(input_buffer_.full())
		{
			if(!frame_buffer_.try_push(input_buffer_.front()))
				graph_->add_tag("dropped-frame");
		}
	}
		
	std::wstring print() const
	{	
		return  L"ogl[" + boost::lexical_cast<std::wstring>(config_.screen_index) + L"|" + format_desc_.name + L"]";
	}
	
	void calculate_aspect()
	{
		if(config_.windowed)
		{
			screen_height_ = window_.GetHeight();
			screen_width_ = window_.GetWidth();
		}
		
		GL(glViewport(0, 0, screen_width_, screen_height_));

		std::pair<float, float> target_ratio = None();
		if(config_.stretch == fill)
			target_ratio = Fill();
		else if(config_.stretch == uniform)
			target_ratio = Uniform();
		else if(config_.stretch == uniform_to_fill)
			target_ratio = UniformToFill();

		width_ = target_ratio.first;
		height_ = target_ratio.second;
	}
		
	std::pair<float, float> None()
	{
		float width = static_cast<float>(square_width_)/static_cast<float>(screen_width_);
		float height = static_cast<float>(square_height_)/static_cast<float>(screen_height_);

		return std::make_pair(width, height);
	}

	std::pair<float, float> Uniform()
	{
		float aspect = static_cast<float>(square_width_)/static_cast<float>(square_height_);
		float width = std::min(1.0f, static_cast<float>(screen_height_)*aspect/static_cast<float>(screen_width_));
		float height = static_cast<float>(screen_width_*width)/static_cast<float>(screen_height_*aspect);

		return std::make_pair(width, height);
	}

	std::pair<float, float> Fill()
	{
		return std::make_pair(1.0f, 1.0f);
	}

	std::pair<float, float> UniformToFill()
	{
		float wr = static_cast<float>(square_width_)/static_cast<float>(screen_width_);
		float hr = static_cast<float>(square_height_)/static_cast<float>(screen_height_);
		float r_inv = 1.0f/std::min(wr, hr);

		float width = wr*r_inv;
		float height = hr*r_inv;

		return std::make_pair(width, height);
	}
};


struct ogl_consumer_proxy : public core::frame_consumer
{
	const configuration config_;
	std::unique_ptr<ogl_consumer> consumer_;

public:

	ogl_consumer_proxy(const configuration& config)
		: config_(config){}
	
	virtual void initialize(const core::video_format_desc& format_desc)
	{
		consumer_.reset(new ogl_consumer(config_, format_desc));
	}
	
	virtual bool send(const safe_ptr<core::read_frame>& frame)
	{
		consumer_->send(frame);
		return true;
	}
	
	virtual std::wstring print() const
	{
		return consumer_->print();
	}

	virtual bool has_synchronization_clock() const 
	{
		return false;
	}

	virtual const core::video_format_desc& get_video_format_desc() const
	{
		return consumer_->get_video_format_desc();
	}
};	

safe_ptr<core::frame_consumer> create_consumer(const std::vector<std::wstring>& params)
{
	if(params.size() < 1 || params[0] != L"SCREEN")
		return core::frame_consumer::empty();
	
	configuration config;
		
	if(params.size() > 1) 
		config.screen_index = lexical_cast_or_default<int>(params[2], config.screen_index);

	if(params.size() > 2) 
		config.windowed = lexical_cast_or_default<bool>(params[3], config.windowed);

	config.key_only = std::find(params.begin(), params.end(), L"KEY_ONLY") != params.end();

	return make_safe<ogl_consumer_proxy>(config);
}

safe_ptr<core::frame_consumer> create_consumer(const boost::property_tree::ptree& ptree) 
{
	configuration config;
	config.screen_index		= ptree.get("device",   config.screen_index);
	config.windowed			= ptree.get("windowed", config.windowed);
	config.key_only			= ptree.get("key-only", config.key_only);
	config.auto_deinterlace	= ptree.get("auto-deinterlace", config.auto_deinterlace);
	
	auto stretch_str = ptree.get("stretch", "default");
	if(stretch_str == "uniform")
		config.stretch = stretch::uniform;
	else if(stretch_str == "uniform_to_fill")
		config.stretch = stretch::uniform_to_fill;
	
	return make_safe<ogl_consumer_proxy>(config);
}

}}