////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
// Copyright (C) 2020-2024 Rupert Carmichael
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

#ifndef NST_BOARD_UNL_FAM250SCHI24_H
#define NST_BOARD_UNL_FAM250SCHI24_H

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
				class Fam250Schi24 : public Board
				{
				public:

					explicit Fam250Schi24(const Context& c)
					: Board(c) { submapper = c.chips.Has(L"SCHI-24") ? 1 : 0; }

				private:

					void SubReset(bool);
					void SubLoad(State::Loader&,dword);
					void SubSave(State::Saver&) const;

					NES_DECL_PEEK( 6000 );
					NES_DECL_POKE( F000 );

					uint bankreg;
					uint submapper;
				};
			}
		}
	}
}

#endif
