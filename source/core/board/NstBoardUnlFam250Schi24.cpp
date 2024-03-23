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

// Reference: https://github.com/TASEmulators/fceux/blob/master/src/boards/354.cpp

#include "NstBoard.hpp"
#include "NstBoardUnlFam250Schi24.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Unlicensed
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Fam250Schi24::SubReset(bool)
				{
					bankreg = 0;
					Map( 0x6000U, 0x7FFFU, &Fam250Schi24::Peek_6000 );
					Map( submapper == 1 ? 0xE000U : 0xF000U, 0xFFFFU, &Fam250Schi24::Poke_F000 );

					prg.SwapBank<SIZE_32K,0x0000>(0);
				}

				void Fam250Schi24::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( (baseChunk == AsciiId<'F','S','C'>::V) );

					if (baseChunk == AsciiId<'F','S','C'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								bankreg = state.Read8();

							state.End();
						}
					}
				}

				void Fam250Schi24::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'F','S','C'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( bankreg ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK_A(Fam250Schi24,6000)
				{
					return *(prg.Source().Mem(bankreg * SIZE_8K) + (address & 0x1FFF));
				}

				NES_POKE_AD(Fam250Schi24,F000)
				{
					const uint bank = data & 0x3F | address << 2 & 0x40 | address >> 5 & 0x80;

					ppu.SetMirroring( (data & 0x40) ? Ppu::NMT_H : Ppu::NMT_V );
					chr.Source().WriteEnable( !(address & 0x08) );

					switch (address & 0x07)
					{
						case 0: case 4:
							prg.SwapBank<SIZE_32K,0x0000>(bank >> 1);
							break;

						case 1:
							prg.SwapBank<SIZE_16K,0x0000>(bank);
							prg.SwapBank<SIZE_16K,0x4000>(bank | 0x07);
							break;

						case 2: case 6:
							prg.SwapBank<SIZE_8K,0x0000>(bank << 1 | data >> 7);
							prg.SwapBank<SIZE_8K,0x2000>(bank << 1 | data >> 7);
							prg.SwapBank<SIZE_8K,0x4000>(bank << 1 | data >> 7);
							prg.SwapBank<SIZE_8K,0x6000>(bank << 1 | data >> 7);
							break;

						case 3: case 7:
							prg.SwapBank<SIZE_16K,0x0000>(bank);
							prg.SwapBank<SIZE_16K,0x4000>(bank);
							break;

						case 5:
							bankreg = bank << 1 | data >> 7;
							prg.SwapBank<SIZE_32K,0x0000>(bank >> 1 | 0x03);
							break;
					}
				}
			}
		}
	}
}
