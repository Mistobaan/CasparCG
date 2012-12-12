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
* Author: Nicklas P Andersson
*/

 
#pragma once

#include "CLKCommand.h"
#include "../util/ProtocolStrategy.h"
#include <core/video_channel.h>

namespace caspar { namespace protocol { namespace CLK {

class CLKProtocolStrategy : public IO::IProtocolStrategy
{
public:
	CLKProtocolStrategy(const std::vector<safe_ptr<core::video_channel>>& channels);

	void Parse(const TCHAR* pData, int charCount, IO::ClientInfoPtr pClientInfo);
	UINT GetCodepage() { return 28591; }	//ISO 8859-1
	
private:
	enum ParserState
	{
		ExpectingNewCommand,
		ExpectingCommand,
		ExpectingClockID,
		ExpectingTime,
		ExpectingParameter
	};

	ParserState	currentState_;
	CLKCommand currentCommand_;
	std::wstringstream currentCommandString_;

	safe_ptr<core::video_channel> pChannel_;

	bool bClockLoaded_;
};

}}}
