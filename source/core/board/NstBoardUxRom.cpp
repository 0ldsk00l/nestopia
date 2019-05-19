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
#include "NstBoardUxRom.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void UxRom::SubReset(bool)
			{
				switch (board.GetId())
				{
					case Type::STD_UNROM:
					case Type::STD_UOROM:

						Map( PRG_SWAP_16K_0_BC );
						break;

					case Type::STD_UN1ROM:

						Map( 0x8000U, 0xFFFFU, &UxRom::Poke_8000_D2 );
						break;

					case Type::STD_UNROM512:

						Map( 0x8000U, 0xFFFFU, &UxRom::Poke_8000_0 );
						hasbattery = board.HasBattery();
						mirr = board.GetNmt();
						switch (mirr) {
						case 0: ppu.SetMirroring( Ppu::NMT_H ); break;
						case 1: ppu.SetMirroring( Ppu::NMT_V ); break;
						}
						break;

					default:

						Map( 0x8000U, 0xFFFFU, PRG_SWAP_16K_0 );
						break;
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_POKE_AD(UxRom,8000_D2)
			{
				prg.SwapBank<SIZE_16K,0x0000>( GetBusData(address,data) >> 2 );
			}

			NES_POKE_AD(UxRom,8000_0)
			{
				if (!hasbattery)
					data = GetBusData(address,data);
				chr.SwapBank<SIZE_8K, 0x0000>( ( data >> 5) & 0x3 );
				prg.SwapBank<SIZE_16K, 0x0000>( ( data ) & 0x1f );
				if ( mirr == Type::NMT_FOURSCREEN )
					ppu.SetMirroring( ( data & 0x80 ) ? Ppu::NMT_1 : Ppu::NMT_0 );
			}
		}
	}
}
