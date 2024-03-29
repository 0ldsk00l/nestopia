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
#include "NstBoardTxcPoliceman.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Txc
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Policeman::SubReset(const bool hard)
				{
					Map( 0x8400U, 0xFFFEU, &Policeman::Poke_8400 );

					if (hard)
						prg.SwapBank<SIZE_32K,0x0000>(0);
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_AD(Policeman,8400)
				{
					ppu.Update();
					//data = GetBusData(address,data); // Unnecessary
					prg.SwapBank<SIZE_32K,0x0000>( data >> 4 );
					chr.SwapBank<SIZE_8K,0x0000>( data & 0x0F );
				}
			}
		}
	}
}
