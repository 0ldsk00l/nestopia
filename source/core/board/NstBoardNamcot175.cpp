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
#include "NstBoardNamcot175.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Namcot
			{
				N175::N175(const Context& c)
				:
				Board (c)
				{
				}

				void N175::SubReset(const bool hard)
				{
					Map( 0x6000U, 0x7FFFU, &N175::Peek_6000 );
					Map( 0x6000U, 0x7FFFU, &N175::Poke_6000 );
					Map( 0x8000U, 0x87FFU, CHR_SWAP_1K_0 );
					Map( 0x8800U, 0x8FFFU, CHR_SWAP_1K_1 );
					Map( 0x9000U, 0x97FFU, CHR_SWAP_1K_2 );
					Map( 0x9800U, 0x9FFFU, CHR_SWAP_1K_3 );
					Map( 0xA000U, 0xA7FFU, CHR_SWAP_1K_4 );
					Map( 0xA800U, 0xAFFFU, CHR_SWAP_1K_5 );
					Map( 0xB000U, 0xB7FFU, CHR_SWAP_1K_6 );
					Map( 0xB800U, 0xBFFFU, CHR_SWAP_1K_7 );
					Map( 0xC000U, 0xC7FFU, &N175::Poke_C000 );
					Map( 0xE000U, 0xE7FFU, PRG_SWAP_8K_0 );
					Map( 0xE800U, 0xEFFFU, PRG_SWAP_8K_1 );
					Map( 0xF000U, 0xF7FFU, PRG_SWAP_8K_2 );
				}


				NES_PEEK_A(N175,6000)
				{
					NST_VERIFY( wrk.Readable(0) );
					return wrk.Readable(0) ? wrk[0][address & 0x7FFU] : (address >> 8);
				}

				NES_POKE_AD(N175,6000)
				{
					NST_VERIFY( wrk.Writable(0) );

					if (wrk.Writable(0))
						wrk[0][address & 0x7FFU] = data;
				}

				NES_POKE_D(N175,C000)
				{
					bool enable = data & 0x1;
					wrk.Source().SetSecurity(enable, enable);
				}
			}
		}
	}
}
