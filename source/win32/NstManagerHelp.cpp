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

#include "NstDialogAbout.hpp"
#include "NstDialogLicense.hpp"
#include "NstManager.hpp"
#include "NstManagerHelp.hpp"
#include <ShellAPI.h>

namespace Nestopia
{
	namespace Managers
	{
		Help::Help(Emulator& e,Window::Menu& m)
		: Manager(e,m,this,&Help::OnEmuEvent)
		{
			static const Window::Menu::CmdHandler::Entry<Help> commands[] =
			{
				{ IDM_HELP_HELP,    &Help::OnCmdHelp    },
				{ IDM_HELP_ABOUT,   &Help::OnCmdAbout   },
				{ IDM_HELP_LICENSE, &Help::OnCmdLicense }
			};

			menu.Commands().Add( this, commands );
			menu[IDM_HELP_HELP].Enable( Application::Instance::GetExePath(L"readme.html").FileExists() );
		}

		void Help::OnCmdHelp(uint)
		{
			::ShellExecute
			(
				NULL,
				L"open",
				Application::Instance::GetExePath(L"readme.html").Ptr(),
				NULL,
				NULL,
				SW_SHOWNORMAL
			);
		}

		void Help::OnCmdAbout(uint)
		{
			Window::About().Open();
		}

		void Help::OnCmdLicense(uint)
		{
			Window::License().Open();
		}

		void Help::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_NETPLAY_MODE:

					menu[IDM_HELP_HELP].Enable( !data );
					menu[IDM_HELP_ABOUT].Enable( !data );
					menu[IDM_HELP_LICENSE].Enable( !data );
					break;
			}
		}
	}
}
