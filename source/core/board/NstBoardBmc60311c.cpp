////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2025 Rupert Carmichael
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
#include "NstBoardBmc60311c.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bmc
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void B60311c::SubReset(const bool hard)
				{
					Map( 0x6000U, &B60311c::Poke_6000 );
					Map( 0x6001U, &B60311c::Poke_6001 );
					Map( 0x8000U, 0xFFFFU, &B60311c::Poke_8000 );

					regs[0] = 0;
					regs[1] = 0;
					latch = 0;
					UpdatePrg();
				}

				void B60311c::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( (baseChunk == AsciiId<'B','6','3'>::V) );

					if (baseChunk == AsciiId<'B','6','3'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<3> data( state );

								latch = data[0];
								regs[0] = data[1];
								regs[1] = data[2];
							}

							state.End();
						}

						UpdatePrg();
					}
				}

				void B60311c::SubSave(State::Saver& state) const
				{
					const byte data[3] =
					{
						static_cast<byte>(latch),
						static_cast<byte>(regs[0]),
						static_cast<byte>(regs[1])
					};

					state.Begin( AsciiId<'B','6','3'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				void B60311c::UpdatePrg()
				{
					if (regs[0] & 0x02)
					{
						prg.SwapBank<SIZE_16K,0x0000>( (latch & 0x07) | (regs[1] & ~0x07) );
						prg.SwapBank<SIZE_16K,0x4000>( 0x07 | (regs[1] & ~0x07) );
					}
					else if (regs[0] & 0x01)
					{
						prg.SwapBank<SIZE_32K,0x0000>( regs[1] >> 1 );
					}
					else
					{
						prg.SwapBank<SIZE_16K,0x0000>( regs[1] );
						prg.SwapBank<SIZE_16K,0x4000>( regs[1] );
					}

					chr.Source().SetSecurity(true, regs[0] & 0x04 ? false : true);
					ppu.SetMirroring( (regs[0] & 0x08) ? Ppu::NMT_H : Ppu::NMT_V );
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(B60311c,6000)
				{
					regs[0] = data;
					UpdatePrg();
				}

				NES_POKE_D(B60311c,6001)
				{
					regs[1] = data;
					UpdatePrg();
				}

				NES_POKE_D(B60311c,8000)
				{
					latch = data;
					UpdatePrg();
				}
			}
		}
	}
}
