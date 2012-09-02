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
#include "NstDialogPreferences.hpp"
#include "NstManagerPreferences.hpp"

namespace Nestopia
{
	namespace Managers
	{
		NST_COMPILE_ASSERT
		(
			Preferences::START_IN_FULLSCREEN      - Window::Preferences::START_IN_FULLSCREEN      == 0 &&
			Preferences::SUPPRESS_WARNINGS        - Window::Preferences::SUPPRESS_WARNINGS        == 0 &&
			Preferences::FIRST_UNLOAD_ON_EXIT     - Window::Preferences::FIRST_UNLOAD_ON_EXIT     == 0 &&
			Preferences::CONFIRM_EXIT             - Window::Preferences::CONFIRM_EXIT             == 0 &&
			Preferences::RUN_IN_BACKGROUND        - Window::Preferences::RUN_IN_BACKGROUND        == 0 &&
			Preferences::AUTOSTART_EMULATION      - Window::Preferences::AUTOSTART_EMULATION      == 0 &&
			Preferences::SAVE_LOGFILE             - Window::Preferences::SAVE_LOGFILE             == 0 &&
			Preferences::ALLOW_MULTIPLE_INSTANCES - Window::Preferences::ALLOW_MULTIPLE_INSTANCES == 0 &&
			Preferences::SAVE_LAUNCHER            - Window::Preferences::SAVE_LAUNCHER            == 0 &&
			Preferences::CONFIRM_RESET            - Window::Preferences::CONFIRM_RESET            == 0 &&
			Preferences::SAVE_CHEATS              - Window::Preferences::SAVE_CHEATS              == 0 &&
			Preferences::SAVE_NETPLAY_GAMELIST    - Window::Preferences::SAVE_NETPLAY_GAMELIST    == 0 &&
			Preferences::SAVE_WINDOWPOS           - Window::Preferences::SAVE_WINDOWPOS           == 0 &&
			Preferences::SAVE_LAUNCHERSIZE        - Window::Preferences::SAVE_LAUNCHERSIZE        == 0 &&
			Preferences::SAVE_SETTINGS            - Window::Preferences::SAVE_SETTINGS            == 0 &&
			Preferences::NUM_SETTINGS             - Window::Preferences::NUM_SETTINGS             == 0
		);

		NST_COMPILE_ASSERT
		(
			Preferences::PRIORITY_NORMAL       - Window::Preferences::PRIORITY_NORMAL       == 0 &&
			Preferences::PRIORITY_ABOVE_NORMAL - Window::Preferences::PRIORITY_ABOVE_NORMAL == 0 &&
			Preferences::PRIORITY_HIGH         - Window::Preferences::PRIORITY_HIGH         == 0
		);

		Preferences::Preferences(Emulator& e,const Configuration& cfg,Window::Menu& m)
		:
		Manager      ( e, m, this, &Preferences::OnEmuEvent, IDM_OPTIONS_PREFERENCES, &Preferences::OnCmdOptions, &Preferences::OnAppEvent ),
		dialog       ( new Window::Preferences(e,cfg) ),
		inFullscreen ( false )
		{
			UpdateSettings();
		}

		Preferences::~Preferences()
		{
		}

		void Preferences::Save(Configuration& cfg) const
		{
			dialog->Save( cfg );
		}

		void Preferences::UpdateMenuColor() const
		{
			const Window::Preferences::MenuLook& look =
			(
				inFullscreen ? dialog->GetSettings().menuLookFullscreen :
                               dialog->GetSettings().menuLookDesktop
			);

			if (look.enabled)
				menu.SetColor( look.color );
			else
				menu.ResetColor();
		}

		void Preferences::UpdateSettings()
		{
			settings.flags = dialog->GetSettings();
			settings.priority = static_cast<Priority>(dialog->GetSettings().priority);
			settings.favoredSystem = dialog->GetSettings().favoredSystem;
			settings.alwaysAskSystem = dialog->GetSettings().alwaysAskSystem;

			UpdateMenuColor();
		}

		void Preferences::OnCmdOptions(uint)
		{
			dialog->Open();
			UpdateSettings();
		}

		void Preferences::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_NETPLAY_MODE:

					if (data)
					{
						settings.flags[RUN_IN_BACKGROUND] = true;
						settings.flags[AUTOSTART_EMULATION] = true;
						settings.flags[CONFIRM_RESET] = false;
						settings.flags[SUPPRESS_WARNINGS] = true;

						if (settings.priority == PRIORITY_NORMAL)
							settings.priority = PRIORITY_ABOVE_NORMAL;
					}
					else
					{
						UpdateSettings();
					}

					menu[IDM_OPTIONS_PREFERENCES].Enable( !data );
					break;
			}
		}

		void Preferences::OnAppEvent(Instance::Event event,const void*)
		{
			switch (event)
			{
				case Instance::EVENT_FULLSCREEN:
				case Instance::EVENT_DESKTOP:

					inFullscreen = (event == Instance::EVENT_FULLSCREEN);
					UpdateMenuColor();
					break;
			}
		}
	}
}
