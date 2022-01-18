////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
// Copyright (C) 2020-2022 Rupert Carmichael
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

/* Note: At the time this was written, MMC3 boards with larger than 512K PRG
   are fantasy hardware. This may change in the future if it is implemented in
   a product along the lines of the PowerPak or Everdrive.
*/

#include "NstBoard.hpp"
#include "NstBoardMmc3.hpp"
#include "NstBoardUnlMmc3BigPrgRom.hpp"
#include "../NstFile.hpp"

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

				void Mmc3BigPrgRom::SubReset(const bool hard)
				{
					Mmc3::SubReset( hard );

					for (uint i=0x0000; i < 0x2000; i += 0x2)
					{
						Map( 0x8001 + i, &Mmc3BigPrgRom::Poke_8001 );
					}
				}

				void Mmc3BigPrgRom::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'M','M','3'>::V) );

					if (baseChunk == AsciiId<'M','M','3'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'R','E','G'>::V:
								{
									State::Loader::Data<12> data( state );

									regs.ctrl0   = data[0];
									regs.ctrl1   = data[1];
									banks.prg[0] = data[2] & 0x7FU;
									banks.prg[1] = data[3] & 0x7FU;
									banks.chr[0] = (data[6] & 0x7FU) << 1;
									banks.chr[1] = banks.chr[0] | 0x01U;
									banks.chr[2] = (data[7] & 0x7FU) << 1;
									banks.chr[3] = banks.chr[2] | 0x01U;
									banks.chr[4] = data[8];
									banks.chr[5] = data[9];
									banks.chr[6] = data[10];
									banks.chr[7] = data[11];
									break;
								}

								case AsciiId<'I','R','Q'>::V:

									irq.unit.LoadState( state );
									break;
							}

							state.End();
						}
					}
				}

				void Mmc3BigPrgRom::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'M','M','3'>::V );

					const byte data[12] =
					{
						regs.ctrl0,
						regs.ctrl1,
						banks.prg[0],
						banks.prg[1],
						0x7E,
						0x7F,
						banks.chr[0] >> 1,
						banks.chr[2] >> 1,
						banks.chr[4],
						banks.chr[5],
						banks.chr[6],
						banks.chr[7]
					};

					state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();

					irq.unit.SaveState( state, AsciiId<'I','R','Q'>::V );

					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Mmc3BigPrgRom,8001)
				{
					uint address = regs.ctrl0 & 0x7;

					if (address < 6)
					{
						ppu.Update();

						uint base = regs.ctrl0 << 5 & 0x1000;

						if (address < 2)
						{
							address <<= 1;
							base |= address << 10;
							UpdateChr( base | 0x0000, (banks.chr[address+0] = data & 0xFE) );
							UpdateChr( base | 0x0400, (banks.chr[address+1] = data | 0x01) );
						}
						else
						{
							UpdateChr( (base ^ 0x1000) | (address-2) << 10, (banks.chr[address+2] = data) );
						}
					}
					else
					{
						UpdatePrg( (address == 6) ? (regs.ctrl0 << 8 & 0x4000) : 0x2000, (banks.prg[address-6] = data & 0x7F) );
					}
				}
			}
		}
	}
}
