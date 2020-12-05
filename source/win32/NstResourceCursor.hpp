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

#ifndef NST_RESOURCE_CURSOR_H
#define NST_RESOURCE_CURSOR_H

#pragma once

#include "NstMain.hpp"
#include <windows.h>

namespace Nestopia
{
	namespace Resource
	{
		class Cursor
		{
			HCURSOR const handle;

			static HCURSOR const hArrow;
			static HCURSOR const hWait;
			static HCURSOR const hHand;

			static HCURSOR LoadHand();

		public:

			explicit Cursor(uint);
			~Cursor ();

			operator HCURSOR () const
			{
				return handle;
			}

			static HCURSOR GetArrow()
			{
				return hArrow;
			}

			static HCURSOR GetWait()
			{
				return hWait;
			}

			static HCURSOR GetHand()
			{
				return hHand;
			}
		};
	}
}

#endif
