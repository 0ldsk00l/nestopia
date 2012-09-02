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
#include "NstBoardBandaiOekaKids.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bandai
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void OekaKids::SubReset(const bool hard)
				{
					ppu.SetAddressLineHook( Core::Io::Line(this,&OekaKids::Line_Nmt) );

					Map( 0x8000U, 0xFFFFU, &OekaKids::Poke_8000 );

					if (hard)
						NES_DO_POKE(8000,0x8000,0x00);
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_LINE(OekaKids,Nmt)
				{
					if (address >= 0x2000 && (address & 0x3FF) < 0x3C0)
						chr.SwapBank<SIZE_4K,0x0000>( (chr.GetBank<SIZE_4K,0x0000>() & 0x4) | (address >> 8 & 0x3) );
				}

				NES_POKE_AD(OekaKids,8000)
				{
					ppu.Update();
					data = GetBusData(address,data);
					prg.SwapBank<SIZE_32K,0x0000>( data );
					chr.SwapBanks<SIZE_4K,0x0000>( (data & 0x4) | (chr.GetBank<SIZE_4K,0x0000>() & 0x3), (data & 0x4) | 0x3 );
				}
			}
		}
	}
}
