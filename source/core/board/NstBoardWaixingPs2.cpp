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

#include "NstBoard.hpp"
#include "NstBoardWaixing.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Waixing
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Ps2::SubReset(bool)
				{
					Map( 0x8000U, 0xFFFFU, &Ps2::Poke_8000 );

					prg.SwapBank<SIZE_32K,0x0000>(0);
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_AD(Ps2,8000)
				{
					ppu.SetMirroring( (data & 0x40) ? Ppu::NMT_H : Ppu::NMT_V );

					uint preg[4];
					uint bank = (data & 0x3F) << 1;
					switch (address & 0x03)
					{
						case 0x00:

							for (uint i = 0; i < 4; ++i)
								preg[i] = bank + i;

							break;

						case 0x02:

							bank = bank | (data >> 7);
							for (uint i = 0; i < 4; ++i)
								preg[i] = bank;

							break;

						case 0x01:
						case 0x03:

							preg[0] = bank + 0;
							preg[1] = bank + 1;
							preg[2] = ((address & 0x02) ? bank : (bank | 0x0E)) + 0;
							preg[3] = ((address & 0x02) ? bank : (bank | 0x0E)) + 1;
							break;
					}

					prg.SwapBanks<SIZE_8K,0x0000>( preg[0], preg[1], preg[2], preg[3] );
				}
			}
		}
	}
}
