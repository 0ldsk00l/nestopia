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

/* References:
   https://www.nesdev.org/wiki/NES_2.0_Mapper_372
*/

#include "NstBoard.hpp"
#include "NstBoardMmc3.hpp"
#include "NstBoardBmcHero.hpp"
#include "NstBoardBmcSfc12.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bmc
			{
				/* Bit 5 of the third outer bank register swaps the pattern
				 * space over to CHR-RAM, which is not banked - the MMC3
				 * registers stop reaching it and each 1k slot maps to its
				 * own 1k of RAM.
				*/
				void NST_FASTCALL Sfc12::UpdateChr(uint address,uint bank) const
				{
					if (exRegs[2] & 0x20)
						chr.Source(1).SwapBank<SIZE_1K>( address, address >> 10 );
					else
						Hero::UpdateChr( address, bank );
				}
			}
		}
	}
}
