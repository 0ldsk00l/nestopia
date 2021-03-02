////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
// Copyright (C) 2021 Rupert Carmichael
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

#include "NstBoard.hpp"
#include "NstBoardNamcot340.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Namcot
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				N340::N340(const Context& c)
				:
				Board (c)
				{
				}

				void N340::SubReset(const bool hard)
				{
					Map( 0x8000U, 0x87FFU, CHR_SWAP_1K_0 );
					Map( 0x8800U, 0x8FFFU, CHR_SWAP_1K_1 );
					Map( 0x9000U, 0x97FFU, CHR_SWAP_1K_2 );
					Map( 0x9800U, 0x9FFFU, CHR_SWAP_1K_3 );
					Map( 0xA000U, 0xA7FFU, CHR_SWAP_1K_4 );
					Map( 0xA800U, 0xAFFFU, CHR_SWAP_1K_5 );
					Map( 0xB000U, 0xB7FFU, CHR_SWAP_1K_6 );
					Map( 0xB800U, 0xBFFFU, CHR_SWAP_1K_7 );
					Map( 0xE000U, 0xE7FFU, &N340::Poke_E000 );
					Map( 0xE800U, 0xEFFFU, PRG_SWAP_8K_1 );
					Map( 0xF000U, 0xF7FFU, PRG_SWAP_8K_2 );
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(N340,E000)
				{
					prg.SwapBank<SIZE_8K>( 0, data & 0x3FU );

					switch ((data >> 6) & 0x3U)
					{
						case 0x0U: // One-screen A
							ppu.SetMirroring( Ppu::NMT_0 );
							break;
						case 0x1U: // Vertical
							ppu.SetMirroring( Ppu::NMT_V );
							break;
						case 0x2U: // One-screen B
							ppu.SetMirroring( Ppu::NMT_1 );
							break;
						case 0x3U: // Horizontal
							ppu.SetMirroring( Ppu::NMT_H );
							break;
					}
				}
			}
		}
	}
}
