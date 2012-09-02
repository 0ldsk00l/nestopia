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

#ifndef NST_DIALOG_PREFERENCES_H
#define NST_DIALOG_PREFERENCES_H

#pragma once

#include "NstCollectionBitSet.hpp"
#include "NstWindowDialog.hpp"
#include "../core/api/NstApiMachine.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Preferences
		{
		public:

			Preferences(Managers::Emulator&,const Configuration&);

			void Save(Configuration&) const;

			enum Type
			{
				START_IN_FULLSCREEN,
				SUPPRESS_WARNINGS,
				FIRST_UNLOAD_ON_EXIT,
				CONFIRM_EXIT,
				RUN_IN_BACKGROUND,
				AUTOSTART_EMULATION,
				SAVE_LOGFILE,
				ALLOW_MULTIPLE_INSTANCES,
				SAVE_LAUNCHER,
				CONFIRM_RESET,
				SAVE_CHEATS,
				SAVE_NETPLAY_GAMELIST,
				SAVE_WINDOWPOS,
				SAVE_LAUNCHERSIZE,
				SAVE_SETTINGS,
				NUM_SETTINGS
			};

			enum Priority
			{
				PRIORITY_NORMAL,
				PRIORITY_ABOVE_NORMAL,
				PRIORITY_HIGH
			};

			struct MenuLook
			{
				COLORREF color;
				bool enabled;
			};

		private:

			enum
			{
				DEFAULT_FULLSCREEN_MENU_COLOR = RGB(0x18,0xCA,0xEF),
				DEFAULT_DESKTOP_MENU_COLOR    = RGB(0x18,0xCA,0xEF)
			};

			struct Handlers;
			struct MenuColorWindow;
			class Association;

			ibool OnInitDialog          (Param&);
			ibool OnPaint               (Param&);
			ibool OnCmdDefault          (Param&);
			ibool OnCmdStyle            (Param&);
			ibool OnCmdMenuColorDefault (Param&);
			ibool OnCmdMenuColorChange  (Param&);
			ibool OnCmdOk               (Param&);

			void UpdateIconStyle() const;
			void UpdateColors() const;

			struct Settings : Collection::BitSet
			{
				Priority priority;
				Nes::Machine::FavoredSystem favoredSystem;
				bool alwaysAskSystem;
				MenuLook menuLookDesktop;
				MenuLook menuLookFullscreen;
			};

			Settings settings;
			Dialog dialog;
			Managers::Emulator& emulator;

			static MenuColorWindow menuColorWindows[2];
			static const ushort icons[4][5];

		public:

			void Open()
			{
				dialog.Open();
			}

			const Settings& GetSettings() const
			{
				return settings;
			}
		};
	}
}

#endif
