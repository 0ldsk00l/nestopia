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

#include "NstManager.hpp"
#include "NstDialogDipSwitches.hpp"
#include "NstManagerDipSwitches.hpp"

namespace Nestopia
{
	namespace Managers
	{
		DipSwitches::DipSwitches(Emulator& e,const Configuration& cfg,Window::Menu& m)
		: Manager(e,m,this,&DipSwitches::OnEmuEvent)
		{
			static const Window::Menu::CmdHandler::Entry<DipSwitches> commands[] =
			{
				{ IDM_MACHINE_EXT_DIPSWITCHES,           &DipSwitches::OnCmdDipSwitches       },
				{ IDM_MACHINE_OPTIONS_DIPSWITCHESONLOAD, &DipSwitches::OnCmdDipSwitchesOnLoad }
			};

			menu.Commands().Add( this, commands );

			static const Window::Menu::PopupHandler::Entry<DipSwitches> popups[] =
			{
				{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_EXT>::ID, &DipSwitches::OnMenuExt }
			};

			menu.Popups().Add( this, popups );

			menu[IDM_MACHINE_OPTIONS_DIPSWITCHESONLOAD].Check( !cfg["machine"]["dip-switches-on-load"].No() );
		}

		void DipSwitches::Save(Configuration& cfg) const
		{
			cfg["machine"]["dip-switches-on-load"].YesNo() = menu[IDM_MACHINE_OPTIONS_DIPSWITCHESONLOAD].Checked();
		}

		bool DipSwitches::Available() const
		{
			return
			(
				Nes::DipSwitches(emulator).NumDips() &&
				!emulator.NetPlayers() &&
				!emulator.IsLocked()
			);
		}

		bool DipSwitches::OpenDialog(bool userOpen) const
		{
			return Available() ? Window::DipSwitches( emulator, userOpen ).Open() : false;
		}

		void DipSwitches::OnMenuExt(const Window::Menu::PopupHandler::Param& param)
		{
			param.menu[IDM_MACHINE_EXT_DIPSWITCHES].Enable( !param.show || Available() );
		}

		void DipSwitches::OnCmdDipSwitches(uint)
		{
			OpenDialog( true );
		}

		void DipSwitches::OnCmdDipSwitchesOnLoad(uint)
		{
			menu[IDM_MACHINE_OPTIONS_DIPSWITCHESONLOAD].ToggleCheck();
		}

		void DipSwitches::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_LOAD:

					if (menu[IDM_MACHINE_OPTIONS_DIPSWITCHESONLOAD].Checked())
					{
						if (OpenDialog( false ))
							menu[IDM_MACHINE_OPTIONS_DIPSWITCHESONLOAD].Uncheck();
					}

					break;

				case Emulator::EVENT_NETPLAY_MODE:

					menu[IDM_MACHINE_OPTIONS_DIPSWITCHESONLOAD].Enable( !data );
					break;
			}
		}
	}
}
