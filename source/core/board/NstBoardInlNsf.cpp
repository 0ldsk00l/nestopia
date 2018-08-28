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
#include "NstBoardInlNsf.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif
			
			void InlNsf::SubReset(const bool hard)
			{
				Map ( 0x5000U, 0x5FFFU, &InlNsf::Poke_5000 );
				Map ( 0x8000U, 0xFFFFU, &InlNsf::Peek_8000 );

				if (hard)
				{
					for (int i=0; i<8; ++i) regs[i] = 0;
					Bank( 7, 0xFF );
				}
			}

			void InlNsf::SubSave(State::Saver& state) const
			{
				state.Begin( AsciiId<'I','N','L'>::V );
				state.Write( regs );
				state.End();
			}

			void InlNsf::SubLoad(State::Loader& state,const dword baseChunk)
			{
				NST_VERIFY( baseChunk == (AsciiId<'I','N','L'>::V) );
				if (baseChunk == AsciiId<'I','N','L'>::V)
				{
					state.Begin();
					state.Read(regs);
					state.End();
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void InlNsf::Bank(uint slot, Data data)
			{
				slot = slot & 7;
				regs[slot] = data;
			}

			NES_POKE_AD(InlNsf,5000)
			{
				Bank(address & 7, data);
			}
			
			NES_PEEK_A(InlNsf,8000)
			{
				// Not an ideal way to do this, but Nestopia does not seem to support 4K banks directly?
				uint slot = (address >> 12) & 7;
				byte b = regs[slot];
				prg.SwapBanks<SIZE_8K>( address & 0x6000, b >> 1 ); // 2 banks per 8k page.
				return prg.Peek( ((b & 1) << 12) | (address & 0x6FFF) ); // Read from 1/2 of the banks.
			}
		}
	}
}
