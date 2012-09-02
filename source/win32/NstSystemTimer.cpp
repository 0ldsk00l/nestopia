////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
//
// This file is part of Nestopia.
//
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

#include "NstSystemTimer.hpp"
#include <windows.h>

#if NST_MSVC
#pragma comment(lib,"winmm")
#endif

#if NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4201 )
#endif

#include <MMSystem.h>

#if NST_MSVC >= 1200
#pragma warning( pop )
#endif

namespace Nestopia
{
	namespace System
	{
		const Timer::Settings Timer::settings;

		Timer::Settings::Settings()
		: period(0)
		{
			LARGE_INTEGER li;

			if (::QueryPerformanceFrequency( &li ))
			{
				pfFrequency = li.HighPart;
				pfFrequency = pfFrequency << 32 | li.LowPart;
			}
			else
			{
				pfFrequency = 0;
			}

			if (::timeBeginPeriod( 1 ) != TIMERR_NOCANDO)
			{
				period = 1;
			}
			else
			{
				TIMECAPS caps;

				if
				(
					::timeGetDevCaps( &caps, sizeof(caps) ) == TIMERR_NOERROR &&
					caps.wPeriodMin &&
					::timeBeginPeriod( caps.wPeriodMin ) != TIMERR_NOCANDO
				)
					period = caps.wPeriodMin;
			}
		}

		Timer::Settings::~Settings()
		{
			if (period)
				::timeEndPeriod( period );
		}

		Timer::Timer(const Type desired)
		{
			Reset( desired );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		inline bool Timer::QueryPf(Value& value)
		{
			LARGE_INTEGER li;

			if (::QueryPerformanceCounter( &li ))
			{
				value = li.HighPart;
				value = value << 32 | li.LowPart;
				return true;
			}
			else
			{
				return false;
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		bool Timer::Reset(const Type desired)
		{
			threshold = THRESHOLD;
			giveup = 0;

			if (desired == PERFORMANCE && settings.pfFrequency - uint(MIN_PF_FRQ) <= uint(MAX_PF_FRQ) && QueryPf( start ))
			{
				frequency = settings.pfFrequency;
				checkPoint = settings.pfFrequency * uint(CHECKPOINT);
				type = PERFORMANCE;
			}
			else
			{
				start = ::timeGetTime();
				frequency = 1000;
				checkPoint = 1000 * CHECKPOINT;
				type = MULTIMEDIA;
			}

			return type == desired;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		Timer::Value Timer::Elapsed() const
		{
			if (type == PERFORMANCE)
			{
				Value current;
				return QueryPf( current ) ? current - start : Value(0);
			}
			else
			{
				return ::timeGetTime() - uint(start);
			}
		}

		bool Timer::Wait(Value current,const Value target)
		{
			NST_ASSERT( target >= current );

			if (target - current >= frequency * uint(SUSPICIOUS))
			{
				NST_DEBUG_MSG("timer clock jump!");
				return false;
			}

			const uint milliSecs( (target - current) * 1000 / frequency );

			if (milliSecs > settings.period + threshold)
			{
				::Sleep( milliSecs - threshold );
				current = Elapsed();

				if (current > target)
				{
					threshold += giveup;
					giveup ^= 1;
				}
			}

			if (threshold > THRESHOLD && checkPoint < target)
			{
				--threshold;
				checkPoint = target + frequency * uint(CHECKPOINT);
			}

			if (type == PERFORMANCE)
			{
				for (const Value s(start); current < target && QueryPf( current ); current -= s);
			}
			else
			{
				for (uint c=current, t=target, s=start; c < t; c = ::timeGetTime() - s);
			}

			return true;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
