////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2026 Rupert Carmichael
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

#ifndef NST_BOARD_BMC_SFC12_H
#define NST_BOARD_BMC_SFC12_H

#include "NstBoardBmcHero.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bmc
			{
				/* The SFC-12 revision of the Rockman I-VI multicart. Mapper 45
				 * with one bit of the third outer bank register turning the
				 * pattern space into unbanked CHR-RAM.
				*/
				class Sfc12 : public Hero
				{
				public:

					explicit Sfc12(const Context& c)
					: Hero(c) {}

				private:

					void NST_FASTCALL UpdateChr(uint,uint) const;
				};
			}
		}
	}
}

#endif
