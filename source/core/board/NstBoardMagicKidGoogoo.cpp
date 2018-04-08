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
#include "NstBoardMagicKidGoogoo.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void MagicKidGoogoo::SubReset(const bool hard)
			{
				Map( 0x8000U, 0x9FFFU, &MagicKidGoogoo::Poke_8000 );
				Map( 0xC000U, 0xDFFFU, &MagicKidGoogoo::Poke_8000 );

				for (uint i=0x0000; i < 0x2000; i += 0x04)
				{
					Map( 0xA000U + i, CHR_SWAP_2K_0 );
					Map( 0xA001U + i, CHR_SWAP_2K_1 );
					Map( 0xA002U + i, CHR_SWAP_2K_2 );
					Map( 0xA003U + i, CHR_SWAP_2K_3 );
				}

				if (hard)
					prg.SwapBank<SIZE_16K, 0x4000>(0);
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_POKE_AD(MagicKidGoogoo, 8000)
			{
				prg.SwapBank<SIZE_16K, 0x0000>( (address >> 11) | (data & 0x07) );
			}
		}
	}
}
