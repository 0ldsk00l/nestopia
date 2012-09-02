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
#include "NstBoardMmc3.hpp"
#include "NstBoardUnlA9746.hpp"

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

				void A9746::SubReset(const bool hard)
				{
					exRegs[0] = 0;
					exRegs[1] = 0;
					exRegs[2] = 0;

					Mmc3::SubReset( hard );

					for (uint i=0x8000; i < 0xA000; i += 0x4)
					{
						Map( i + 0x0, &A9746::Poke_8000 );
						Map( i + 0x1, &A9746::Poke_8001 );
						Map( i + 0x2, &A9746::Poke_8002 );
						Map( i + 0x3, NOP_POKE          );
					}
				}

				void A9746::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'A','9','7'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<3> data( state );

								exRegs[0] = data[0];
								exRegs[1] = data[1];
								exRegs[2] = data[2] << 4;
							}

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void A9746::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );

					const byte data[] =
					{
						exRegs[0], exRegs[1], exRegs[2] >> 4
					};

					state.Begin( AsciiId<'A','9','7'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_AD(A9746,8000)
				{
					exRegs[0] = 0;
					exRegs[1] = data;
				}

				NES_POKE_D(A9746,8001)
				{
					ppu.Update();

					if (exRegs[0] - 0x23 < 4)
					{
						prg.SwapBank<SIZE_8K>
						(
							(exRegs[0] - 0x23) << 13 ^ 0x6000,
							(data >> 5 & 0x1) |
							(data >> 3 & 0x2) |
							(data >> 1 & 0x4) |
							(data << 1 & 0x8)
						);
					}

					switch (exRegs[1])
					{
						case 0x08:
						case 0x0A:
						case 0x0E:
						case 0x12:
						case 0x16:
						case 0x1A:
						case 0x1E: exRegs[2] = data << 4; break;
						case 0x09: chr.SwapBank<SIZE_1K,0x0000>( exRegs[2] | (data >> 1 & 0xE) ); break;
						case 0x0B: chr.SwapBank<SIZE_1K,0x0400>( exRegs[2] | (data >> 1 | 0x1) ); break;
						case 0x0C:
						case 0x0D: chr.SwapBank<SIZE_1K,0x0800>( exRegs[2] | (data >> 1 & 0xE) ); break;
						case 0x0F: chr.SwapBank<SIZE_1K,0x0C00>( exRegs[2] | (data >> 1 | 0x1) ); break;
						case 0x10:
						case 0x11: chr.SwapBank<SIZE_1K,0x1000>( exRegs[2] | (data >> 1 & 0xF) ); break;
						case 0x14:
						case 0x15: chr.SwapBank<SIZE_1K,0x1400>( exRegs[2] | (data >> 1 & 0xF) ); break;
						case 0x18:
						case 0x19: chr.SwapBank<SIZE_1K,0x1800>( exRegs[2] | (data >> 1 & 0xF) ); break;
						case 0x1C:
						case 0x1D: chr.SwapBank<SIZE_1K,0x1C00>( exRegs[2] | (data >> 1 & 0xF) ); break;
					}
				}

				NES_POKE_AD(A9746,8002)
				{
					exRegs[0] = data;
					exRegs[1] = 0;
				}
			}
		}
	}
}
