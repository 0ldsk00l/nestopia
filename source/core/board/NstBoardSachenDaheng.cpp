////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2026 Rupert Carmichael
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
#include "NstBoardSachenDaheng.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Sachen
			{
				void Daheng::SubReset(const bool hard)
				{
					if (hard)
					{
						mode = 0;
						mirroring = 0;
					}

					Mmc3::SubReset( hard );

					// Mask is undocumented; $C100 matches the observed decode and
					// keeps clear of the WRAM window.
					for (uint i=0x4100; i < 0x6000; i += 0x200)
						Map( i, i + 0xFF, &Daheng::Poke_4100 );

					// The board type is four-screen, so Mmc3 leaves $A000 alone.
					for (uint i=0x0000; i < 0x2000; i += 0x2)
						Map( 0xA000 + i, &Daheng::Poke_A000 );

					// WRAM cannot be disabled on this clone; the game never enables it.
					for (uint i=0x0000; i < 0x2000; i += 0x2)
						Map( 0xA001 + i, NOP_POKE );

					wrk.Source().SetSecurity( true, true );

					UpdateNmt();
				}

				void Daheng::SubSave(State::Saver& state) const
				{
					const byte data[2] =
					{
						static_cast<byte>(mode),
						static_cast<byte>(mirroring)
					};

					state.Begin( AsciiId<'D','H','G'>::V ).Write( data ).End();
					Mmc3::SubSave( state );
				}

				void Daheng::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'D','H','G'>::V)
					{
						State::Loader::Data<2> data( state );

						mode = data[0] & 0x3;
						mirroring = data[1];

						Mmc3::UpdateChr();
						UpdateNmt();
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void Daheng::UpdateNmt() const
				{
					if ((mode & 0x3) == 0x1)
						nmt.Source(1).SwapBank<SIZE_4K,0x0000>(0);
					else
						ppu.SetMirroring( (mirroring & 0x1) ? Ppu::NMT_H : Ppu::NMT_V );
				}

				void NST_FASTCALL Daheng::UpdateChr(uint address,uint bank) const
				{
					chr.Source( (mode & 0x2) ? 1 : 0 ).SwapBank<SIZE_1K>( address, bank );
				}

				NES_POKE_D(Daheng,4100)
				{
					if (mode != (data & 0x3))
					{
						ppu.Update();

						mode = data & 0x3;

						Mmc3::UpdateChr();
						UpdateNmt();
					}
				}

				NES_POKE_D(Daheng,A000)
				{
					if (mirroring != data)
					{
						mirroring = data;
						UpdateNmt();
					}
				}
			}
		}
	}
}
