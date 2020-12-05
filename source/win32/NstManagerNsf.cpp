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
#include "NstManagerNsf.hpp"
#include "../core/api/NstApiNsf.hpp"

namespace Nestopia
{
	namespace Managers
	{
		Nsf::Nsf(Emulator& e,const Configuration& cfg,Window::Menu& m)
		: Manager(e,m,this,&Nsf::OnEmuEvent)
		{
			static const Window::Menu::CmdHandler::Entry<Nsf> commands[] =
			{
				{ IDM_MACHINE_NSF_PLAY,                     &Nsf::OnCmd          },
				{ IDM_MACHINE_NSF_STOP,                     &Nsf::OnCmd          },
				{ IDM_MACHINE_NSF_NEXT,                     &Nsf::OnCmd          },
				{ IDM_MACHINE_NSF_PREV,                     &Nsf::OnCmd          },
				{ IDM_MACHINE_NSF_OPTIONS_PLAYINBACKGROUND, &Nsf::OnCmdPlayInBkg }
			};

			menu.Commands().Add( this, commands );

			static const Window::Menu::PopupHandler::Entry<Nsf> popups[] =
			{
				{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_NSF>::ID, &Nsf::OnMenu }
			};

			menu.Popups().Add( this, popups );

			menu[IDM_MACHINE_NSF_OPTIONS_PLAYINBACKGROUND].Check( !cfg["nsf"]["play-in-background"].No() );
		}

		void Nsf::Save(Configuration& cfg) const
		{
			cfg["nsf"]["play-in-background"].YesNo() = menu[IDM_MACHINE_NSF_OPTIONS_PLAYINBACKGROUND].Checked();
		}

		void Nsf::OnMenu(const Window::Menu::PopupHandler::Param& param)
		{
			const bool on = emulator.IsNsfOn();
			const Nes::Nsf nsf( emulator );

			menu[IDM_MACHINE_NSF_PLAY].Enable( !param.show || (on && !nsf.IsPlaying()) );
			menu[IDM_MACHINE_NSF_STOP].Enable( !param.show || (on && nsf.IsPlaying()) );
			menu[IDM_MACHINE_NSF_NEXT].Enable( !param.show || (on && nsf.GetCurrentSong() + 1 < nsf.GetNumSongs()) );
			menu[IDM_MACHINE_NSF_PREV].Enable( !param.show || (on && nsf.GetCurrentSong() > 0) );
		}

		void Nsf::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_POWER_ON:

					if (emulator.IsNsf())
						Nes::Nsf(emulator).PlaySong();

					break;

				case Emulator::EVENT_NETPLAY_MODE:

					menu[IDM_MACHINE_NSF_OPTIONS_PLAYINBACKGROUND].Enable( !data );
					menu[IDM_POS_MACHINE][IDM_POS_MACHINE_NSF].Enable( !data );
					break;
			}
		}

		void Nsf::OnCmd(uint cmd)
		{
			Nes::Nsf nsf(emulator);

			switch (cmd)
			{
				case IDM_MACHINE_NSF_PLAY: nsf.PlaySong(); break;
				case IDM_MACHINE_NSF_STOP: nsf.StopSong(); break;
				case IDM_MACHINE_NSF_NEXT: nsf.SelectNextSong(); break;
				case IDM_MACHINE_NSF_PREV: nsf.SelectPrevSong(); break;
			}

			Resume();
		}

		void Nsf::OnCmdPlayInBkg(uint)
		{
			menu[IDM_MACHINE_NSF_OPTIONS_PLAYINBACKGROUND].ToggleCheck();
		}
	}
}
