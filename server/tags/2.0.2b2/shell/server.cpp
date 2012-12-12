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

#include "server.h"

#include <common/env.h>
#include <common/exception/exceptions.h>
#include <common/utility/string.h>

#include <core/mixer/gpu/ogl_device.h>
#include <core/video_channel.h>
#include <core/producer/stage.h>
#include <core/consumer/output.h>

#include <modules/bluefish/bluefish.h>
#include <modules/decklink/decklink.h>
#include <modules/ffmpeg/ffmpeg.h>
#include <modules/flash/flash.h>
#include <modules/oal/oal.h>
#include <modules/ogl/ogl.h>
#include <modules/silverlight/silverlight.h>
#include <modules/image/image.h>

#include <modules/oal/consumer/oal_consumer.h>
#include <modules/bluefish/consumer/bluefish_consumer.h>
#include <modules/decklink/consumer/decklink_consumer.h>
#include <modules/ogl/consumer/ogl_consumer.h>
#include <modules/ffmpeg/consumer/ffmpeg_consumer.h>

#include <protocol/amcp/AMCPProtocolStrategy.h>
#include <protocol/cii/CIIProtocolStrategy.h>
#include <protocol/CLK/CLKProtocolStrategy.h>
#include <protocol/util/AsyncEventServer.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace caspar {

using namespace core;
using namespace protocol;

struct server::implementation : boost::noncopyable
{
	safe_ptr<ogl_device>						ogl_;
	std::vector<safe_ptr<IO::AsyncEventServer>> async_servers_;	
	std::vector<safe_ptr<video_channel>>		channels_;

	implementation()		
		: ogl_(ogl_device::create())
	{			
		ffmpeg::init();
		CASPAR_LOG(info) << L"Initialized ffmpeg module.";
							  
		bluefish::init();	  
		CASPAR_LOG(info) << L"Initialized bluefish module.";
							  
		decklink::init();	  
		CASPAR_LOG(info) << L"Initialized decklink module.";
							  
		flash::init();		  
		CASPAR_LOG(info) << L"Initialized flash module.";
							  
		oal::init();		  
		CASPAR_LOG(info) << L"Initialized oal module.";
							  
		ogl::init();		  
		CASPAR_LOG(info) << L"Initialized ogl module.";

		image::init();		  
		CASPAR_LOG(info) << L"Initialized image module.";

		setup_channels(env::properties());
		CASPAR_LOG(info) << L"Initialized channels.";

		setup_controllers(env::properties());
		CASPAR_LOG(info) << L"Initialized controllers.";
	}

	~implementation()
	{		
		ffmpeg::uninit();

		async_servers_.clear();
		channels_.clear();
	}
				
	void setup_channels(const boost::property_tree::ptree& pt)
	{   
		using boost::property_tree::ptree;
		BOOST_FOREACH(auto& xml_channel, pt.get_child("configuration.channels"))
		{		
			auto format_desc = video_format_desc::get(widen(xml_channel.second.get("video-mode", "PAL")));		
			if(format_desc.format == video_format::invalid)
				BOOST_THROW_EXCEPTION(caspar_exception() << msg_info("Invalid video-mode."));
			
			channels_.push_back(make_safe<video_channel>(channels_.size(), format_desc, ogl_));
			
			int index = 0;
			BOOST_FOREACH(auto& xml_consumer, xml_channel.second.get_child("consumers"))
			{
				try
				{
					const std::string name = xml_consumer.first;
					if(name == "screen")
						channels_.back()->output()->add(index++, ogl::create_consumer(xml_consumer.second));					
					else if(name == "bluefish")					
						channels_.back()->output()->add(index++, bluefish::create_consumer(xml_consumer.second));					
					else if(name == "decklink")					
						channels_.back()->output()->add(index++, decklink::create_consumer(xml_consumer.second));				
					//else if(name == "file")					
					//	channels_.back()->output()->add(index++, create_ffmpeg_consumer(xml_consumer.second));						
					else if(name == "system-audio")
						channels_.back()->output()->add(index++, oal::create_consumer());		
					else if(name != "<xmlcomment>")
						CASPAR_LOG(warning) << "Invalid consumer: " << widen(name);	
				}
				catch(...)
				{
					CASPAR_LOG_CURRENT_EXCEPTION();
				}
			}							
		}
	}
		
	void setup_controllers(const boost::property_tree::ptree& pt)
	{		
		using boost::property_tree::ptree;
		BOOST_FOREACH(auto& xml_controller, pt.get_child("configuration.controllers"))
		{
			try
			{
				std::string name = xml_controller.first;
				std::string protocol = xml_controller.second.get<std::string>("protocol");	

				if(name == "tcp")
				{					
					unsigned int port = xml_controller.second.get("port", 5250);
					auto asyncbootstrapper = make_safe<IO::AsyncEventServer>(create_protocol(protocol), port);
					asyncbootstrapper->Start();
					async_servers_.push_back(asyncbootstrapper);
				}
				else
					CASPAR_LOG(warning) << "Invalid controller: " << widen(name);	
			}
			catch(...)
			{
				CASPAR_LOG_CURRENT_EXCEPTION();
			}
		}
	}

	safe_ptr<IO::IProtocolStrategy> create_protocol(const std::string& name) const
	{
		if(name == "AMCP")
			return make_safe<amcp::AMCPProtocolStrategy>(channels_);
		else if(name == "CII")
			return make_safe<cii::CIIProtocolStrategy>(channels_);
		else if(name == "CLOCK")
			return make_safe<CLK::CLKProtocolStrategy>(channels_);
		
		BOOST_THROW_EXCEPTION(caspar_exception() << arg_name_info("name") << arg_value_info(name) << msg_info("Invalid protocol"));
	}
};

server::server() : impl_(new implementation()){}

const std::vector<safe_ptr<video_channel>> server::get_channels() const
{
	return impl_->channels_;
}

}