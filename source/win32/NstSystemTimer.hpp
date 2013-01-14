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

#ifndef NST_SYSTEM_TIMER_H
#define NST_SYSTEM_TIMER_H

#pragma once

#include "NstMain.hpp"

namespace Nestopia
{
	namespace System
	{
		class Timer
		{
		public:

			typedef qaword Value;

			enum Type
			{
				MULTIMEDIA,
				PERFORMANCE
			};

			explicit Timer(Type=MULTIMEDIA);

			bool  Reset(Type);
			Value Elapsed() const;
			bool  Wait(Value,Value);

		private:

			static inline bool QueryPf(Value&);

			enum
			{
				THRESHOLD = 1,
				CHECKPOINT = 60,
				SUSPICIOUS = 3,
				MIN_PF_FRQ = 1000,
				MAX_PF_FRQ = 0x10000000
			};

			struct Settings
			{
				Settings();
				~Settings();

				Value pfFrequency;
				uint period;
			};

			Value start;
			Value frequency;
			Value checkPoint;
			Type type;
			uint threshold;
			uint giveup;

			static const Settings settings;

		public:

			static bool HasPerformanceCounter()
			{
				return settings.pfFrequency != 0;
			}

			static Value GetPerformanceCounterFrequency()
			{
				return settings.pfFrequency;
			}

			Type GetType() const
			{
				return type;
			}

			Value GetFrequency() const
			{
				return frequency;
			}
		};
	}
}

#endif
