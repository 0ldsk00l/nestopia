////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2022 Rupert Carmichael
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

#ifndef NST_BOARD_UNL_RETX7GBL_H
#define NST_BOARD_UNL_RETX7GBL_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Unlicensed
			{
				class RetX7Gbl : public Board
				{
				public:

					explicit RetX7Gbl(const Context& c)
					: Board(c) {}

				private:

					void SubReset(bool);
					void SubLoad(State::Loader&,dword);
					void SubSave(State::Saver&) const;

					void UpdatePrg();

					NES_DECL_POKE( 7800 );
					NES_DECL_POKE( 8000 );
					NES_DECL_POKE( C000 );

					uint regs[2];
					uint led;
				};
			}
		}
	}
}

#endif
