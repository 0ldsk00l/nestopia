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
#include "NstBoardMmc2.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void Mmc2::SubReset(const bool hard)
			{
				if (hard)
				{
					selector[0] = 0;
					selector[1] = 2;

					banks[0] = 0;
					banks[1] = 0;
					banks[2] = 0;
					banks[3] = 0;

					prg.SwapBank<SIZE_32K,0x0000>(~0U);
				}

				chr.SetAccessor( this, &Mmc2::Access_Chr );

				Map( 0xA000U, 0xAFFFU, PRG_SWAP_8K_0    );
				Map( 0xB000U, 0xEFFFU, &Mmc2::Poke_B000 );
				Map( 0xF000U, 0xFFFFU, NMT_SWAP_HV      );
			}

			void Mmc2::SubLoad(State::Loader& state,const dword baseChunk)
			{
				NST_VERIFY( baseChunk == (AsciiId<'M','M','2'>::V) );

				if (baseChunk == AsciiId<'M','M','2'>::V)
				{
					while (const dword subId = state.Begin())
					{
						if (subId == AsciiId<'R','E','G'>::V)
						{
							State::Loader::Data<4+1> data( state );

							banks[0] = data[0];
							banks[1] = data[1];
							banks[2] = data[2];
							banks[3] = data[3];

							selector[0] = 0 + (data[4] >> 0 & 0x1);
							selector[1] = 2 + (data[4] >> 1 & 0x1);
						}

						state.End();
					}
				}
			}

			void Mmc2::SubSave(State::Saver& state) const
			{
				const byte data[4+1] =
				{
					banks[0],
					banks[1],
					banks[2],
					banks[3],
					selector[0] | (selector[1] - 2) << 1
				};

				state.Begin( AsciiId<'M','M','2'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_ACCESSOR(Mmc2,Chr)
			{
				const uint data = chr.Peek( address );
				uint bank;

				switch (address & 0xFF8)
				{
					case 0xFD8: bank = (address >> 11 & 0x2) | 0x0; break;
					case 0xFE8: bank = (address >> 11 & 0x2) | 0x1; break;
					default: return data;
				}

				selector[address >> 12] = bank;
				chr.SwapBank<SIZE_4K>( address & 0x1000, banks[bank] );

				return data;
			}

			NES_POKE_AD(Mmc2,B000)
			{
				ppu.Update();
				banks[(address - 0xB000) >> 12] = data;
				chr.SwapBanks<SIZE_4K,0x0000>( banks[selector[0]], banks[selector[1]] );
			}
		}
	}
}
