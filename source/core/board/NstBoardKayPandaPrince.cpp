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
#include "NstBoardKay.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Kay
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void PandaPrince::SubReset(const bool hard)
				{
					exRegs[0] = 0;
					exRegs[1] = 0;
					exRegs[2] = 0;

					Mmc3::SubReset( hard );

					Map( 0x5000U, 0x5FFFU, &PandaPrince::Peek_5000, &PandaPrince::Poke_5000 );
					Map( 0x8000U, 0x9FFFU, &PandaPrince::Poke_8000 );
				}

				void PandaPrince::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'K','P','P'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<3> data( state );

								exRegs[0] = data[0];
								exRegs[1] = data[1];
								exRegs[2] = data[2];
							}

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void PandaPrince::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );

					const byte data[] =
					{
						exRegs[0], exRegs[1], exRegs[2]
					};

					state.Begin( AsciiId<'K','P','P'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void NST_FASTCALL PandaPrince::UpdatePrg(uint address,uint bank)
				{
					if (address == 0x4000)
					{
						if (exRegs[0])
							bank = exRegs[0];
					}
					else if (address == 0x6000)
					{
						if (exRegs[1])
							bank = exRegs[1];
					}

					prg.SwapBank<SIZE_8K>( address, bank );
				}

				NES_PEEK(PandaPrince,5000)
				{
					return exRegs[2];
				}

				NES_POKE_D(PandaPrince,5000)
				{
					static const byte lut[] =
					{
						0x00, 0x83, 0x42, 0x00
					};

					exRegs[2] = lut[data & 0x3];
				}

				NES_POKE_AD(PandaPrince,8000)
				{
					if ((address & 0x3) == 0x3)
					{
						switch (data)
						{
							case 0x28: exRegs[0] = 0x0C; break;
							case 0x26: exRegs[1] = 0x08; break;
							case 0xAB: exRegs[1] = 0x07; break;
							case 0xEC: exRegs[1] = 0x0D; break;
							case 0xEF: exRegs[1] = 0x0D; break;
							case 0xFF: exRegs[1] = 0x09; break;

							case 0x20: exRegs[1] = 0x13; break;
							case 0x29: exRegs[1] = 0x1B; break;

							default:   exRegs[0] = 0x0; exRegs[1] = 0x0; break;
						}
					}
					else if (address & 0x1)
					{
						Mmc3::NES_DO_POKE(8001,address,data);
					}
					else
					{
						Mmc3::NES_DO_POKE(8000,address,data);
					}

					Mmc3::UpdatePrg();
				}
			}
		}
	}
}
