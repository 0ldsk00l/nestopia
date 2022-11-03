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

/* References:
   https://github.com/NovaSquirrel/Mesen-X/blob/master/Core/Mapper400.h
*/

#include "NstBoard.hpp"
#include "NstBoardUnlRetX7Gbl.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Unlicensed
			{
				void RetX7Gbl::SubReset(bool)
				{
					Map( 0x7800U, 0x7FFFU, &RetX7Gbl::Poke_7800 );
					Map( 0x8000U, 0xBFFFU, &RetX7Gbl::Poke_8000 );
					Map( 0xC000U, 0xFFFFU, &RetX7Gbl::Poke_C000 );

					regs[0] = 0x80;
					regs[1] = 0x00;

					UpdatePrg();
				}

				void RetX7Gbl::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'R','X','G'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<3> data( state );

								regs[0] = data[0];
								regs[1] = data[1];
								led = data[2];
							}

							state.End();
						}
					}

					UpdatePrg();
				}

				void RetX7Gbl::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'R','X','G'>::V );

					const byte data[3] =
					{
						regs[0], regs[1], led
					};

					state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
					state.End();
				}

				void RetX7Gbl::UpdatePrg()
				{
					prg.SwapBank<SIZE_16K,0x0000>( (regs[0] & 0x78) | (regs[1] & 0x7) );
					prg.SwapBank<SIZE_16K,0x4000>( (regs[0] & 0x78) | 0x7 );

					if (regs[0] != 0x80)
					{
						ppu.SetMirroring( regs[0] & 0x20 ? Ppu::NMT_H : Ppu::NMT_V );
					}
				}

				NES_POKE_D(RetX7Gbl,7800)
				{
					regs[0] = data;
					UpdatePrg();
				}

				NES_POKE_D(RetX7Gbl,8000)
				{
					led = data;
				}

				NES_POKE_D(RetX7Gbl,C000)
				{
					regs[1] = data;
					UpdatePrg();

					chr.SwapBank<SIZE_8K,0x0000>( (data >> 5) & 0x3 );
				}
			}
		}
	}
}
