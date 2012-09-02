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

#ifndef NST_MANAGER_FRAMECLOCK_H
#define NST_MANAGER_FRAMECLOCK_H

#pragma once

#include "NstSystemTimer.hpp"

namespace Nestopia
{
	namespace Window
	{
		class FrameClock;
	}

	namespace Managers
	{
		class FrameClock : Manager
		{
		public:

			FrameClock(Window::Menu&,Emulator&,const Configuration&,bool);
			~FrameClock();

			void Save(Configuration&) const;

		private:

			void OnEmuEvent(Emulator::Event,Emulator::Data);
			void OnMenuOptionsTiming(uint);
			uint Synchronize(bool,uint);
			void UpdateRewinderState(bool=true) const;
			void UpdateSettings();
			void ResetTimer();

			System::Timer timer;
			System::Timer::Value clkMul;
			uint counter;
			uint clkDiv;

			struct
			{
				uchar autoFrameSkip;
				uchar maxFrameSkips;
				ushort refreshRate;
			}   settings;

			Object::Heap<Window::FrameClock> dialog;

		public:

			uint GameSynchronize(bool throttle)
			{
				return (uint(throttle) | settings.autoFrameSkip) ? Synchronize( throttle, ~0U ) : 0;
			}

			void SoundSynchronize()
			{
				Synchronize( true, 0 );
			}

			void StartEmulation()
			{
				ResetTimer();
			}

			void StopEmulation()
			{
				ResetTimer();
			}

			uint GetRefreshRate() const
			{
				return settings.refreshRate;
			}
		};
	}
}

#endif
