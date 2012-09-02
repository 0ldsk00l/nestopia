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
#include "NstBoardUnlN625092.hpp"

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

				void N625092::SubReset(const bool hard)
				{
					Map( 0x8000U, 0xBFFFU, &N625092::Poke_8000 );
					Map( 0xC000U, 0xFFFFU, &N625092::Poke_C000 );

					if (hard)
					{
						regs[0] = 0;
						regs[1] = 0;

						UpdatePrg();
					}
				}

				void N625092::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'N','6','2'>::V) );

					if (baseChunk == AsciiId<'N','6','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<2> data( state );

								regs[0] = data[0];
								regs[1] = data[1];
							}

							state.End();
						}
					}
				}

				void N625092::SubSave(State::Saver& state) const
				{
					const byte data[] =
					{
						regs[0], regs[1]
					};

					state.Begin( AsciiId<'N','6','2'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void N625092::UpdatePrg()
				{
					prg.SwapBanks<SIZE_16K,0x0000>
					(
						(regs[0] >> 1 & 0x38) | ((regs[0] & 0x1) ? (regs[0] & 0x80) ? regs[1] : (regs[1] & 0x6) | 0x0 : regs[1]),
						(regs[0] >> 1 & 0x38) | ((regs[0] & 0x1) ? (regs[0] & 0x80) ?     0x7 : (regs[1] & 0x6) | 0x1 : regs[1])
					);
				}

				NES_POKE_A(N625092,8000)
				{
					ppu.SetMirroring( (address & 0x1) ? Ppu::NMT_H : Ppu::NMT_V );
					address = address >> 1 & 0xFF;

					if (regs[0] != address)
					{
						regs[0] = address;
						UpdatePrg();
					}
				}

				NES_POKE_A(N625092,C000)
				{
					address &= 0x7;

					if (regs[1] != address)
					{
						regs[1] = address;
						UpdatePrg();
					}
				}
			}
		}
	}
}
