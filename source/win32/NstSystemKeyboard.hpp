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

#ifndef NST_SYSTEM_KEYBOARD_H
#define NST_SYSTEM_KEYBOARD_H

#pragma once

#include "NstString.hpp"

namespace Nestopia
{
	namespace System
	{
		class Keyboard
		{
		public:

			enum
			{
				NUM_KEYS = 0x100,
				NONE = 0
			};

			static uint GetKey(const GenericString);
			static const GenericString GetName(uint);

			enum Indicator
			{
				CAPS_LOCK = 0x14,
				NUM_LOCK = 0x90,
				SCROLL_LOCK = 0x91
			};

			static bool ToggleIndicator(Indicator,bool);

		private:

			struct KeyName : HeapString
			{
				void Set(uint);
			};

			static KeyName keyNames[NUM_KEYS];
		};
	}
}

#endif
