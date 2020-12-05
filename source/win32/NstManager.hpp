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

#ifndef NST_MANAGER_H
#define NST_MANAGER_H

#pragma once

#include "NstWindowMenu.hpp"
#include "NstManagerEmulator.hpp"

namespace Nestopia
{
	namespace Managers
	{
		using Application::Configuration;

		class Manager
		{
		protected:

			Emulator& emulator;
			Window::Menu& menu;

			Manager(Emulator& e,Window::Menu& m)
			: emulator(e), menu(m)
			{
			}

			template<typename Data,typename EmuCode>
			Manager(Emulator& e,Window::Menu& m,Data* data,EmuCode emuCode)
			: emulator(e), menu(m)
			{
				emulator.Events().Add( data, emuCode );
			}

			template<typename Data,typename EmuCode,typename MenuCode>
			Manager(Emulator& e,Window::Menu& m,Data* data,EmuCode emuCode,uint id,MenuCode menuCode)
			: emulator(e), menu(m)
			{
				emulator.Events().Add( data, emuCode );
				menu.Commands().Add( id, data, menuCode );
			}

			template<typename Data,typename EmuCode,typename AppCode>
			Manager(Emulator& e,Window::Menu& m,Data* data,EmuCode emuCode,AppCode appCode)
			: emulator(e), menu(m)
			{
				emulator.Events().Add( data, emuCode );
				Application::Instance::Events::Add( data, appCode );
			}

			template<typename Data,typename EmuCode,typename MenuCode,typename AppCode>
			Manager(Emulator& e,Window::Menu& m,Data* data,EmuCode emuCode,uint id,MenuCode menuCode,AppCode appCode)
			: emulator(e), menu(m)
			{
				emulator.Events().Add( data, emuCode );
				menu.Commands().Add( id, data, menuCode );
				Application::Instance::Events::Add( data, appCode );
			}

			~Manager();

		public:

			static void Resume();
		};
	}
}

#endif
