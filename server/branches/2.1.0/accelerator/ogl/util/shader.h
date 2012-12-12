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

#include <common/memory.h>

#include <string>

namespace caspar { namespace accelerator { namespace ogl {
		
class shader sealed
{
	shader(const shader&);
	shader& operator=(const shader&);
public:

	// Static Members

	// Constructors

	shader(const std::string& vertex_source_str, const std::string& fragment_source_str);
	~shader();

	// Methods

	void set(const std::string& name, bool value);
	void set(const std::string& name, int value);
	void set(const std::string& name, float value);
	void set(const std::string& name, float value0, float value1);
	void set(const std::string& name, double value);
	void use() const;

	// Properties
	
	int id() const;

private:
	struct impl;
	spl::unique_ptr<impl> impl_;
};

}}}