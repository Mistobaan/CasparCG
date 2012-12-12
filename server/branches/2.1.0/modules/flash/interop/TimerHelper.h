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
 
#ifndef _TIMER_HELPER_H__
#define _TIMER_HELPER_H__

#include "FlashAxContainer.h"

namespace caspar {
namespace flash {

	class TimerHelper
	{
		TimerHelper(const TimerHelper&);
		const TimerHelper& operator=(const TimerHelper&);

	public:
		TimerHelper()
		{}
		TimerHelper(DWORD first, DWORD interv, ITimerSink* pTS) : firstTime(first), interval(interv), currentTime(first), pTimerSink(pTS)
		{
			ID = first;
		}
		~TimerHelper()
		{
		}
		void Setup(DWORD first, DWORD interv, ITimerSink* pTS)
		{
			firstTime = first;
			interval = interv;
			currentTime = first;
			pTimerSink = pTS;
			ID = first;
		}

		DWORD Invoke()
		{
			if(pTimerSink != 0)
			{
				VARIANT value;
				value.vt = VT_UI4;
				value.ulVal = currentTime;

				pTimerSink->OnTimer(value);
				currentTime += interval;
			}
			return currentTime;
		}

		DWORD firstTime;
		DWORD interval;
		DWORD currentTime;
		ATL::CComPtr<ITimerSink> pTimerSink;
		DWORD ID;
	};

}	//namespace flash
}	//namespace caspar

#endif	//_TIMER_HELPER_H__