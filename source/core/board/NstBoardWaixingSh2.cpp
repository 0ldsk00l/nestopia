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

				void Sh2::SubReset(const bool hard)
				{
					selector[1] = 0;
					selector[0] = 0;

					chr.SetAccessor( this, &Sh2::Access_Chr );

					Mmc3::SubReset( hard );
				}

				void Sh2::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );
					state.Begin( AsciiId<'W','S','2'>::V ).Begin( AsciiId<'L','T','C'>::V ).Write8( (selector[0] >> 1) | (selector[1] & 0x2) ).End().End();
				}

				void Sh2::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'W','S','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'L','T','C'>::V)
							{
								const uint data = state.Read8();
								selector[0] = data << 1 & 0x2;
								selector[1] = data & 0x2 | 0x4;
							}

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				inline void Sh2::SwapChr(uint address) const
				{
					chr.Source( banks.chr[selector[address >> 12]] == 0 ).SwapBank<SIZE_4K>( address, banks.chr[selector[address >> 12]] >> 2 );
				}

				void NST_FASTCALL Sh2::UpdateChr(uint,uint) const
				{
					SwapChr( 0x0000 );
					SwapChr( 0x1000 );
				}

				NES_ACCESSOR(Sh2,Chr)
				{
					const uint data = chr.Peek( address );
					uint bank;

					switch (address & 0xFF8)
					{
						case 0xFD0: bank = (address >> 10 & 0x4) | 0x0; break;
						case 0xFE8: bank = (address >> 10 & 0x4) | 0x2; break;
						default: return data;
					}

					selector[address >> 12] = bank;
					SwapChr( address & 0x1000 );

					return data;
				}
			}
		}
	}
}
