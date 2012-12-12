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
 
// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#define NOMINMAX

#include <Windows.h>

#include <memory>
#include <array>
#include <functional>
#include <algorithm>
#include <vector>
#include <deque>
#include <queue>
#include <string>
#include <math.h>

#include <common/utility/string.h>
#include <common/memory/safe_ptr.h>
//#include "../common/concurrency/executor.h" // Can't include this due to MSVC lambda bug

#include <common/log/Log.h>
#include <common/exception/exceptions.h>

#include <assert.h>
