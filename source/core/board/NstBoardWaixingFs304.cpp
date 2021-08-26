////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
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
#include "NstBoardWaixing.hpp"

// Reference: https://github.com/TASVideos/fceux/blob/master/src/boards/164.cpp

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Waixing
			{

				void Fs304::SubReset(bool)
				{
					Map( 0x5000U, 0x5FFFU, &Fs304::Poke_5000 );

					regs[0] = 0x3;
					regs[1] = 0x0;
					regs[2] = 0x0;
					regs[3] = 0x7;

					UpdatePrg();
				}

				void Fs304::SubSave(State::Saver& state) const
				{
					const byte data[4] = { regs[0], regs[1], regs[2], regs[3] };
					state.Begin( AsciiId<'3','0','4'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				void Fs304::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'3','0','4'>::V) );

					if (baseChunk == AsciiId<'3','0','4'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<4> data( state );

								regs[0] = data[0];
								regs[1] = data[1];
								regs[2] = data[2];
								regs[3] = data[3];
							}

							state.End();
						}
					}
				}

				void Fs304::UpdatePrg()
				{
					switch (regs[3] & 0x5) {
						case 0:
							prg.SwapBank<SIZE_32K>( 0x0000, ((regs[0] & 0xC) | (regs[1] & 0x2) | ((regs[2] & 0xF) << 4)) );
							break;
						case 1:
							prg.SwapBank<SIZE_32K>( 0x0000, ((regs[0] & 0xC) | (regs[2] & 0xF) << 4) );
							break;
						case 4:
							prg.SwapBank<SIZE_32K>( 0x0000, ((regs[0] & 0xE) | ((regs[1] >> 1) & 0x1) | ((regs[2] & 0xF) << 4)) );
							break;
						case 5:
							prg.SwapBank<SIZE_32K>( 0x0000, ((regs[0] & 0xF) | ((regs[2] & 0xF) << 4)) );
							break;
					}
				}

				NES_POKE_AD(Fs304,5000)
				{
					regs[(address >> 8) & 0x3] = data;
					UpdatePrg();
				}
			}
		}
	}
}
