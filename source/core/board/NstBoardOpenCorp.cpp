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

// Reference: https://github.com/TASVideos/fceux/blob/master/src/boards/156.cpp

#include "NstBoard.hpp"
#include "NstBoardOpenCorp.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace OpenCorp
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Daou306::RemapChr()
				{
					chr.SwapBank<SIZE_1K>( 0x0000, (chrHigh[0] << 8) | chrLow[0] );
					chr.SwapBank<SIZE_1K>( 0x0400, (chrHigh[1] << 8) | chrLow[1] );
					chr.SwapBank<SIZE_1K>( 0x0800, (chrHigh[2] << 8) | chrLow[2] );
					chr.SwapBank<SIZE_1K>( 0x0C00, (chrHigh[3] << 8) | chrLow[3] );
					chr.SwapBank<SIZE_1K>( 0x1000, (chrHigh[4] << 8) | chrLow[4] );
					chr.SwapBank<SIZE_1K>( 0x1400, (chrHigh[5] << 8) | chrLow[5] );
					chr.SwapBank<SIZE_1K>( 0x1800, (chrHigh[6] << 8) | chrLow[6] );
					chr.SwapBank<SIZE_1K>( 0x1C00, (chrHigh[7] << 8) | chrLow[7] );

					if (mirrorUsed)
					{
						ppu.SetMirroring( mirror ^ 0x1 ? Ppu::NMT_V : Ppu::NMT_H );
					}
					else
					{
						ppu.SetMirroring( Ppu::NMT_0 );
					}
				}

				void Daou306::SubReset(bool)
				{
					for (uint i = 0; i < 8; i++)
					{
						chrLow[i] = chrHigh[i] = 0;
					}

					mirror = 0;
					mirrorUsed = 0;

					Map( 0xC000U, 0xC00FU, &Daou306::Poke_C000 );
					Map( 0xC010U, PRG_SWAP_16K_0 );
					Map( 0xC014U, &Daou306::Poke_C014 );
				}

				NES_POKE_AD(Daou306,C000)
				{
					switch (address)
					{
						case 0xC000:
						case 0xC001:
						case 0xC002:
						case 0xC003:
							chrLow[address & 0x03] = data;
							break;
						case 0xC004:
						case 0xC005:
						case 0xC006:
						case 0xC007:
							chrHigh[address & 0x03] = data;
							break;
						case 0xC008:
						case 0xC009:
						case 0xC00A:
						case 0xC00B:
							chrLow[4 + (address & 0x03)] = data;
							break;
						case 0xC00C:
						case 0xC00D:
						case 0xC00E:
						case 0xC00F:
							chrHigh[4 + (address & 0x03)] = data;
							break;
					}
					RemapChr();
				}

				NES_POKE_D(Daou306,C014)
				{
					mirror = data;
					mirrorUsed = 1;
				}

				void Daou306::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'O','P','C'>::V) );

					if (baseChunk == AsciiId<'O','P','C'>::V)
					{
						state.Begin();

						State::Loader::Data<18> data( state );
						chrLow[0] = data[0];
						chrLow[1] = data[1];
						chrLow[2] = data[2];
						chrLow[3] = data[3];
						chrLow[4] = data[4];
						chrLow[5] = data[5];
						chrLow[6] = data[6];
						chrLow[7] = data[7];
						chrHigh[0] = data[8];
						chrHigh[1] = data[9];
						chrHigh[2] = data[10];
						chrHigh[3] = data[11];
						chrHigh[4] = data[12];
						chrHigh[5] = data[13];
						chrHigh[6] = data[14];
						chrHigh[7] = data[15];
						mirror = data[16];
						mirrorUsed = data[17];

						state.End();

						RemapChr();
					}
				}

				void Daou306::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'O','P','C'>::V );

					const byte data[18] =
						{
							chrLow[0],
							chrLow[1],
							chrLow[2],
							chrLow[3],
							chrLow[4],
							chrLow[5],
							chrLow[6],
							chrLow[7],
							chrHigh[0],
							chrHigh[1],
							chrHigh[2],
							chrHigh[3],
							chrHigh[4],
							chrHigh[5],
							chrHigh[6],
							chrHigh[7],
							mirror,
							mirrorUsed,
						};

					state.Begin( AsciiId<'C','H','R'>::V ).Write( data ).End();

					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif
			}
		}
	}
}
