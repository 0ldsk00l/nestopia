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

#ifndef NST_BOARD_ACCLAIM_MCACC_H
#define NST_BOARD_ACCLAIM_MCACC_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "../NstTimer.hpp"
#include "NstBoardMmc3.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Acclaim
			{
				class McAcc : public Board
				{
				public:

					explicit McAcc(const Context&);

				protected:

					void SubReset(bool);
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);
					void Sync(Event,Input::Controllers*);

					void UpdatePrg();
					void UpdateChr() const;

					virtual void NST_FASTCALL UpdatePrg(uint,uint);
					virtual void NST_FASTCALL UpdateChr(uint,uint) const;

					NES_DECL_POKE( 8000 );
					NES_DECL_POKE( 8001 );
					NES_DECL_POKE( A001 );
					NES_DECL_POKE( C000 );
					NES_DECL_POKE( C001 );
					NES_DECL_POKE( E000 );
					NES_DECL_POKE( E001 );

					enum
					{
						CTRL1_WRAM_READONLY = 0x40,
						CTRL1_WRAM_ENABLED  = 0x80,
						CTRL1_WRAM          = CTRL1_WRAM_ENABLED|CTRL1_WRAM_READONLY
					};

					struct Regs
					{
						uint ctrl0;
						uint ctrl1;
					}   regs;

					struct
					{
						byte prg[4];
						byte chr[8];
					}   banks;

				private:
					enum {
						A12_FILTER = 39,
						IRQ_DELAY = 0,
					};

					// FIXME: IRQ_DELAY should be 4 ppu clocks
					Mmc3::Irq<IRQ_DELAY, A12_FILTER> irq;
				};
			}
		}
	}
}

#endif
