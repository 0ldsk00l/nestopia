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

#include "language/resource.h"
#include "NstApplicationException.hpp"
#include "NstWindowCustom.hpp"

namespace Nestopia
{
	namespace Window
	{
		Custom::Timers Custom::timers;

		Custom::Timer::Timer(const TimerCallback& t)
		: TimerCallback(t) {}

		void Custom::StartTimer(const TimerCallback timer,const uint duration) const
		{
			NST_ASSERT( hWnd );

			uint id = 0;

			for (Timers::ConstIterator it(timers.Begin()), end(timers.End()); ; ++it, ++id)
			{
				if (it == end)
				{
					timers.PushBack( timer );
					break;
				}
				else if (*it == timer)
				{
					break;
				}
			}

			timers[id].active = true;

			if (!::SetTimer( hWnd, id, duration, &TimerProc ))
				throw Application::Exception( IDS_ERR_FAILED, L"SetTimer()" );
		}

		bool Custom::StopTimer(const TimerCallback timer) const
		{
			for (Timers::Iterator it(timers.Begin()), end(timers.End()); it != end; ++it)
			{
				if (*it == timer && it->active)
				{
					it->active = false;
					::KillTimer( hWnd, it - timers.Begin() );
					return true;
				}
			}

			return false;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		void CALLBACK Custom::TimerProc(HWND const hWnd,uint,const UINT_PTR id,DWORD)
		{
			NST_ASSERT( id < timers.Size() );

			const uint next = timers[id]();

			if (timers[id].active)
			{
				if (next == 0)
				{
					timers[id].active = false;
					::KillTimer( hWnd, id );
				}
				else if (next != 1)
				{
					::SetTimer( hWnd, id, next, &TimerProc );
				}
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif
	}
}
