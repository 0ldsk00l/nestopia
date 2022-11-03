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

#include "NstBoard.hpp"
#include "NstBoardUnl158b.hpp"

/* References:
   https://github.com/SourMesen/Mesen/blob/master/Core/Unl158B.h
   https://github.com/TASVideos/fceux/blob/master/src/boards/158B.cpp
*/

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Unlicensed
			{
				Gd98158b::Gd98158b(const Context& c)
				: Mmc3(c) {}

				void Gd98158b::SubReset(const bool hard)
				{
					lut[0] = lut[1] = lut[2] = lut[7] = 0x00;
					lut[3] = 0x01;
					lut[4] = 0x02;
					lut[5] = 0x04;
					lut[6] = 0x0F;

					if (hard)
						reg = 0;

					banks.prg[0] = 0x00;
					banks.prg[1] = 0x01;
					banks.prg[2] = 0x3E;
					banks.prg[3] = 0x3F;

					Mmc3::SubReset( hard );

					Map( 0x5000U, 0x5FFFU, &Gd98158b::Peek_5000 );
					Map( 0x5000U, 0x5FFFU, &Gd98158b::Poke_5000 );
				}

				void Gd98158b::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'1','5','8'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								reg = state.Read8();

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void Gd98158b::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );
					state.Begin( AsciiId<'1','5','8'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( reg ).End().End();
				}

				void NST_FASTCALL Gd98158b::UpdatePrg(uint address, uint data)
				{
					if (reg & 0x80)
					{
						const uint bank = reg & 0x7;

						if (reg & 0x20)
						{
							prg.SwapBank<SIZE_32K>( 0x0000, bank >> 1 );
						}
						else
						{
							prg.SwapBank<SIZE_16K>( 0x0000, bank );
							prg.SwapBank<SIZE_16K>( 0x4000, bank );
						}
					}
					else
					{
						prg.SwapBank<SIZE_8K>( (address & 0x6000), data & 0xF );
					}
				}

				NES_PEEK_A(Gd98158b,5000)
				{
					// Fake Open Bus read by always using 0x50
					return 0x50 | lut[address & 0x7];
				}

				NES_POKE_AD(Gd98158b,5000)
				{
					if ((address & 0x7) == 0)
					{
						reg = data;
						Mmc3::UpdatePrg();
					}
				}
			}
		}
	}
}
