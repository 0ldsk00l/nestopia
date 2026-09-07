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

#ifndef NST_BOARD_BATLAB_SRRX_H
#define NST_BOARD_BATLAB_SRRX_H

#include "../NstTimer.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Batlab
			{
				class Srrx : public Board
				{
				public:

					explicit Srrx(const Context&);

				private:

					void SubReset(bool);
					void SubLoad(State::Loader&,dword);
					void SubSave(State::Saver&) const;
					void Sync(Event,Input::Controllers*);

					void UpdateBanks();

					NES_DECL_PEEK( 5000 );
					NES_DECL_PEEK( 6000 );
					NES_DECL_PEEK( Pcm  );

					NES_DECL_POKE( 8000 );
					NES_DECL_POKE( 9000 );
					NES_DECL_POKE( A000 );
					NES_DECL_POKE( C000 );
					NES_DECL_POKE( D000 );
					NES_DECL_POKE( E000 );

					enum
					{
						READ_FILTER = 6
					};

					struct Irq
					{
						void Reset(bool);
						bool Clock();

						enum
						{
							CLOCK_FILTER = 16
						};

						uint count;
						uint latch;
						ibool enabled;
					};

					uint regs[4];
					dword pcmAddress;
					uint pcmControl;
					Cycle pcmLastRead;
					Timer::A12<Irq,Irq::CLOCK_FILTER> irq;
				};
			}
		}
	}
}

#endif
