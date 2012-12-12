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

#pragma once

#include <common/memory/safe_ptr.h>

#include <boost/noncopyable.hpp>

#include <boost/property_tree/ptree_fwd.hpp>

namespace caspar { namespace core {
	
class stage;
class mixer;
class output;
class ogl_device;
struct video_format_desc;

class video_channel : boost::noncopyable
{
public:
	explicit video_channel(int index, const video_format_desc& format_desc, const safe_ptr<ogl_device>& ogl);

	safe_ptr<stage> stage();
	safe_ptr<mixer>	mixer();
	safe_ptr<output> output();
	
	video_format_desc get_video_format_desc() const;
	void set_video_format_desc(const video_format_desc& format_desc);
	
	boost::property_tree::wptree info() const;

	int index() const;

private:
	struct implementation;
	safe_ptr<implementation> impl_;
};

}}