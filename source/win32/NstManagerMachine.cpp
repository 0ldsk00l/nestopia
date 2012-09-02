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

#include "NstResourceString.hpp"
#include "NstWindowUser.hpp"
#include "NstIoScreen.hpp"
#include "NstManager.hpp"
#include "NstManagerPreferences.hpp"
#include "NstManagerMachine.hpp"
#include "../core/api/NstApiMachine.hpp"

namespace Nestopia
{
	namespace Managers
	{
		NST_COMPILE_ASSERT
		(
			IDM_MACHINE_SYSTEM_NTSC == IDM_MACHINE_SYSTEM_AUTO + 1 &&
			IDM_MACHINE_SYSTEM_PAL  == IDM_MACHINE_SYSTEM_AUTO + 2
		);

		Machine::Machine(Emulator& e,const Configuration& cfg,Window::Menu& m,const Preferences& p)
		:
		Manager     ( e, m, this, &Machine::OnEmuEvent ),
		preferences ( p )
		{
			{
				static const Window::Menu::CmdHandler::Entry<Machine> commands[] =
				{
					{ IDM_MACHINE_POWER,       &Machine::OnCmdPower   },
					{ IDM_MACHINE_RESET_SOFT,  &Machine::OnCmdReset   },
					{ IDM_MACHINE_RESET_HARD,  &Machine::OnCmdReset   },
					{ IDM_MACHINE_PAUSE,       &Machine::OnCmdPause   },
					{ IDM_MACHINE_SYSTEM_AUTO, &Machine::OnCmdSystem  },
					{ IDM_MACHINE_SYSTEM_NTSC, &Machine::OnCmdSystem  },
					{ IDM_MACHINE_SYSTEM_PAL,  &Machine::OnCmdSystem  }
				};

				menu.Commands().Add( this, commands );
			}

			{
				const GenericString type( cfg["machine"]["region"].Str() );

				SetRegion
				(
					type == L"ntsc" ? IDM_MACHINE_SYSTEM_NTSC :
					type == L"pal"  ? IDM_MACHINE_SYSTEM_PAL  :
                                      IDM_MACHINE_SYSTEM_AUTO
				);
			}

			UpdateMenu();
			UpdateMenuPowerState();
		}

		void Machine::Save(Configuration& cfg) const
		{
			cfg["machine"]["region"].Str() =
			(
				menu[IDM_MACHINE_SYSTEM_AUTO].Checked()       ? "auto" :
				Nes::Machine(emulator).Is(Nes::Machine::NTSC) ? "ntsc" :
																"pal"
			);
		}

		void Machine::UpdateMenu() const
		{
			const bool on = emulator.IsOn();
			const bool reset = (on && emulator.GetPlayer() == Emulator::MASTER);
			const bool pause = (on && emulator.NetPlayers() == 0);

			menu[IDM_MACHINE_POWER].Text() << Resource::String( on ? IDS_MENU_POWER_OFF : IDS_MENU_POWER_ON );
			menu[IDM_MACHINE_RESET_SOFT].Enable( reset );
			menu[IDM_MACHINE_RESET_HARD].Enable( reset );
			menu[IDM_POS_MACHINE][IDM_POS_MACHINE_RESET].Enable( reset );
			menu[IDM_MACHINE_PAUSE].Enable( pause );
		}

		void Machine::UpdateMenuPowerState() const
		{
			menu[IDM_MACHINE_POWER].Enable( emulator.IsImage() && emulator.NetPlayers() == 0 );
		}

		bool Machine::SetRegion(const uint id) const
		{
			const Nes::Result result = Nes::Machine(emulator).SetMode
			(
				id == IDM_MACHINE_SYSTEM_AUTO ? Nes::Machine(emulator).GetDesiredMode() :
				id == IDM_MACHINE_SYSTEM_NTSC ? Nes::Machine::NTSC : Nes::Machine::PAL
			);

			if (NES_SUCCEEDED(result))
			{
				menu[id].Check( IDM_MACHINE_SYSTEM_AUTO, IDM_MACHINE_SYSTEM_PAL );
				return result != Nes::RESULT_NOP;
			}

			return false;
		}

		void Machine::OnCmdPower(uint)
		{
			if (emulator.IsOn())
			{
				if
				(
					!preferences[Preferences::CONFIRM_RESET] ||
					Window::User::Confirm( IDS_ARE_YOU_SURE, IDS_MACHINE_POWER_OFF_TITLE )
				)
				{
					if (emulator.Power( false ))
						Io::Screen() << Resource::String(IDS_SCREEN_POWER_OFF);
				}
			}
			else if (emulator.Power( true ))
			{
				Io::Screen() << Resource::String(IDS_SCREEN_POWER_ON);
			}
		}

		void Machine::OnCmdReset(uint hard)
		{
			if
			(
				!preferences[Preferences::CONFIRM_RESET] ||
				Window::User::Confirm( IDS_ARE_YOU_SURE, IDS_MACHINE_RESET_TITLE )
			)
				emulator.SendCommand( Emulator::COMMAND_RESET, hard == IDM_MACHINE_RESET_HARD );

			Resume();
		}

		void Machine::OnCmdPause(uint)
		{
			const bool pause = !emulator.Paused();

			emulator.Pause( pause );
			Io::Screen() << Resource::String(pause ? IDS_SCREEN_PAUSE : IDS_SCREEN_RESUME);

			if (!pause)
				Resume();
		}

		void Machine::OnCmdSystem(uint id)
		{
			if (SetRegion( id ))
				Io::Screen() << Resource::String(Nes::Machine(emulator).Is(Nes::Machine::NTSC) ? IDS_SCREEN_NTSC : IDS_SCREEN_PAL);

			Resume();
		}

		void Machine::OnEmuEvent(const Emulator::Event event,Emulator::Data)
		{
			switch (event)
			{
				case Emulator::EVENT_RESET_SOFT:
				case Emulator::EVENT_RESET_HARD:

					Io::Screen() << Resource::String(event == Emulator::EVENT_RESET_HARD ? IDS_SCREEN_RESET_HARD : IDS_SCREEN_RESET_SOFT);
					break;

				case Emulator::EVENT_MODE_NTSC:
				case Emulator::EVENT_MODE_PAL:

					if (menu[IDM_MACHINE_SYSTEM_AUTO].Unchecked())
						menu[event == Emulator::EVENT_MODE_NTSC ? IDM_MACHINE_SYSTEM_NTSC : IDM_MACHINE_SYSTEM_PAL].Check( IDM_MACHINE_SYSTEM_AUTO, IDM_MACHINE_SYSTEM_PAL );

					break;

				case Emulator::EVENT_PAUSE:
				case Emulator::EVENT_RESUME:

					menu[IDM_MACHINE_PAUSE].Check( event == Emulator::EVENT_PAUSE );
					break;

				case Emulator::EVENT_MOVIE_PLAYING:
				case Emulator::EVENT_MOVIE_PLAYING_STOPPED:
				case Emulator::EVENT_MOVIE_RECORDING:
				case Emulator::EVENT_MOVIE_RECORDING_STOPPED:
				{
					const bool noplay = (event != Emulator::EVENT_MOVIE_PLAYING);
					const bool stopped = (event == Emulator::EVENT_MOVIE_PLAYING_STOPPED || event == Emulator::EVENT_MOVIE_RECORDING_STOPPED);

					menu[IDM_MACHINE_RESET_SOFT].Enable( noplay );
					menu[IDM_MACHINE_RESET_HARD].Enable( noplay );
					menu[IDM_POS_MACHINE][IDM_POS_MACHINE_RESET].Enable( noplay );

					for (uint i=IDM_MACHINE_SYSTEM_AUTO; i <= IDM_MACHINE_SYSTEM_PAL; ++i)
						menu[i].Enable( stopped );

					break;
				}

				case Emulator::EVENT_POWER_ON:
				case Emulator::EVENT_POWER_OFF:

					UpdateMenu();
					break;

				case Emulator::EVENT_LOAD:
				case Emulator::EVENT_UNLOAD:

					UpdateMenuPowerState();

					if (emulator.NetPlayers())
					{
						const bool enable = (event == Emulator::EVENT_UNLOAD);

						menu[IDM_POS_MACHINE][IDM_POS_MACHINE_REGION].Enable( enable );

						for (uint i=IDM_MACHINE_SYSTEM_AUTO; i <= IDM_MACHINE_SYSTEM_PAL; ++i)
							menu[i].Enable( enable );
					}
					break;
			}
		}
	}
}
