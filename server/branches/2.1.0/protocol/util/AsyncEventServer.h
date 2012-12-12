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
 
// AsyncEventServer.h: interface for the AsyncServer class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "protocol_strategy.h"

namespace caspar { namespace IO {

class AsyncEventServer
{
public:
	explicit AsyncEventServer(const protocol_strategy_factory<char>::ptr& protocol, unsigned short port);
	~AsyncEventServer();
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}	

