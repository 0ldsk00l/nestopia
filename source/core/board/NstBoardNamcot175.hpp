////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
// Copyright (C) 2014 R. Danbrook
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

#ifndef NST_BOARD_NAMCOT_175_H
#define NST_BOARD_NAMCOT_175_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Namcot
			{
				class N175 : public Board
				{
				public:

					explicit N175(const Context&);

				private:

					void SubReset(bool);
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);
					void Load(File&);
					void Save(File&) const;
					void Sync(Event,Input::Controllers*);
					void SwapChr(uint,uint,uint) const;
					void SwapNmt(uint,uint) const;

					struct Irq
					{
						void Reset(bool);
						bool Clock();

						uint count;
					};

					NES_DECL_PEEK( 4800 );
					NES_DECL_POKE( 4800 );
					NES_DECL_PEEK( 5000 );
					NES_DECL_POKE( 5000 );
					NES_DECL_PEEK( 5800 );
					NES_DECL_POKE( 5800 );
					NES_DECL_POKE( C000 );
					NES_DECL_POKE( C800 );
					NES_DECL_POKE( D000 );
					NES_DECL_POKE( D800 );
					NES_DECL_POKE( F800 );

					Timer::M2<Irq> irq;
				};
			}
		}
	}
}

#endif
