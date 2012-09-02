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

#ifndef NST_BOARD_CONY_H
#define NST_BOARD_CONY_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Cony
			{
				class Standard : public Board
				{
				public:

					explicit Standard(const Context&);

				private:

					~Standard();

					class CartSwitches;

					void SubReset(bool);
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);
					Device QueryDevice(DeviceType);
					void UpdatePrg();
					void Sync(Event,Input::Controllers*);

					NES_DECL_PEEK( 5000   );
					NES_DECL_PEEK( 5100   );
					NES_DECL_POKE( 5100   );
					NES_DECL_PEEK( 6000   );
					NES_DECL_POKE( 8000   );
					NES_DECL_POKE( 8100   );
					NES_DECL_POKE( 8200   );
					NES_DECL_POKE( 8201   );
					NES_DECL_POKE( 8300   );
					NES_DECL_POKE( 8310_0 );
					NES_DECL_POKE( 8310_1 );

					struct Irq
					{
						void Reset(bool);
						bool Clock();

						ibool enabled;
						uint count;
						uint step;
					};

					struct
					{
						word ctrl;
						byte prg[5];
						byte pr8;
					}   regs;

					Timer::M2<Irq> irq;
					CartSwitches* const cartSwitches;
				};
			}
		}
	}
}

#endif
