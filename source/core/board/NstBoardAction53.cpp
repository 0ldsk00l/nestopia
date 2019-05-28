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
#include "NstBoardAction53.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void Action53::SubReset(const bool hard)
			{
				Map( 0x5000U, 0x5FFFU, &Action53::Poke_5000 );
				Map( 0x8000U, 0xFFFFU, &Action53::Poke_8000 );

				if (hard)
				{
					preg[1] = 0xf;
					preg[3] = 0x3f;
				}
			}

			void Action53::SubLoad(State::Loader& state,const dword baseChunk)
			{
				NST_VERIFY( baseChunk == (AsciiId<'A','5','3'>::V) );

				if (baseChunk == AsciiId<'A','5','3'>::V)
				{
					while (const dword chunk = state.Begin())
					{
						if (chunk == AsciiId<'R','E','G'>::V)
						{
							State::Loader::Data<6> data( state );

							preg[0] = data[0];
							preg[1] = data[1];
							preg[2] = data[2];
							preg[3] = data[3];
							mirroring = data[4];
							index = data[5];
						}

						state.End();
					}
				}
			}

			void Action53::SubSave(State::Saver& state) const
			{
				const byte data[] =
				{
					preg[0], preg[1], preg[2], preg[3],
					mirroring, index
				};

				state.Begin( AsciiId<'A','5','3'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_POKE_D(Action53, 5000)
			{
				index = ((data >> 6) & 0x2) | (data & 0x1);
			}

			NES_POKE_D(Action53, 8000)
			{
				switch (index) {
				case 0:
					if (!(mirroring & 2)) {
						mirroring = (mirroring & 0x2) | (data >> 4) & 0x1;
						set_nmt_mirroring();
					}
					chr.SwapBank<SIZE_8K, 0x0000>( data & 0x3 );
					break;
				case 1:
					if (!(mirroring & 2)) {
						mirroring = (mirroring & 0x2) | (data >> 4) & 0x1;
						set_nmt_mirroring();
					}
					preg[1] = data & 0xf;
					set_prg();
					break;
				case 2:
					preg[2] = data & 0x3c;
					mirroring = (data & 3);
					set_prg();
					set_nmt_mirroring();
					break;
				case 3:
					preg[3] = data & 0x3f;
					set_prg();
					break;
				}
			}

			void Action53::set_prg(void)
			{
				byte prglo, prghi;
				byte prg_inner_b = preg[1];
				byte prg_outer_b = (preg[3] << 1);

				/* this can probably be rolled up, but i have no motivation to do so
				 * until it's been tested */
				switch (preg[2] & 0x3c)
				{
				/* 32K modes */
				case 0x00:
				case 0x04:
					prglo = prg_outer_b;
					prghi = prg_outer_b | 0x1;
					break;
				case 0x10:
				case 0x14:
					prglo = (prg_outer_b & ~0x2) | ((prg_inner_b << 1) & 0x2);
					prghi = (prg_outer_b & ~0x2) | ((prg_inner_b << 1) & 0x2) | 0x1;
					break;
				case 0x20:
				case 0x24:
					prglo = (prg_outer_b & ~0x6) | ((prg_inner_b << 1) & 0x6);
					prghi = (prg_outer_b & ~0x6) | ((prg_inner_b << 1) & 0x6) | 0x1;
					break;
				case 0x30:
				case 0x34:
					prglo = (prg_outer_b & ~0xe) | ((prg_inner_b << 1) & 0xe);
					prghi = (prg_outer_b & ~0xe) | ((prg_inner_b << 1) & 0xe) | 0x1;
					break;
				/* bottom fixed modes */
				case 0x08:
					prglo = prg_outer_b;
					prghi = prg_outer_b | (prg_inner_b & 0x1);
					break;
				case 0x18:
					prglo = prg_outer_b;
					prghi = (prg_outer_b & ~0x2) | (prg_inner_b & 0x3);
					break;
				case 0x28:
					prglo = prg_outer_b;
					prghi = (prg_outer_b & ~0x6) | (prg_inner_b & 0x7);
					break;
				case 0x38:
					prglo = prg_outer_b;
					prghi = (prg_outer_b & ~0xe) | (prg_inner_b & 0xf);
					break;
				/* top fixed modes */
				case 0x0c:
					prglo = prg_outer_b | (prg_inner_b & 0x1);
					prghi = prg_outer_b | 0x1;
					break;
				case 0x1c:
					prglo = (prg_outer_b & ~0x2) | (prg_inner_b & 0x3);
					prghi = prg_outer_b | 0x1;
					break;
				case 0x2c:
					prglo = (prg_outer_b & ~0x6) | (prg_inner_b & 0x7);
					prghi = prg_outer_b | 0x1;
					break;
				case 0x3c:
					prglo = (prg_outer_b & ~0xe) | (prg_inner_b & 0xf);
					prghi = prg_outer_b | 0x1;
					break;
				}

				prg.SwapBank<SIZE_16K, 0x0000>( prglo );
				prg.SwapBank<SIZE_16K, 0x4000>( prghi );
			}

			void Action53::set_nmt_mirroring(void)
			{
				switch (mirroring)
				{
				case 0: ppu.SetMirroring( Ppu::NMT_0 ); break;
				case 1: ppu.SetMirroring( Ppu::NMT_1 ); break;
				case 2: ppu.SetMirroring( Ppu::NMT_V ); break;
				case 3: ppu.SetMirroring( Ppu::NMT_H ); break;
				}
			}
		}
	}
}
