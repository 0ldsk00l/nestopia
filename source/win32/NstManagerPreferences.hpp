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

#ifndef NST_MANAGER_PREFERENCES_H
#define NST_MANAGER_PREFERENCES_H

#pragma once

#include "NstApplicationInstance.hpp"
#include "NstCollectionBitSet.hpp"
#include "NstObjectHeap.hpp"
#include "../core/api/NstApiMachine.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Preferences;
	}

	namespace Managers
	{
		class Preferences : Manager
		{
		public:

			Preferences(Emulator&,const Configuration&,Window::Menu&);
			~Preferences();

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
				NUM_SETTINGS,
				DISABLE_STATUSMSG
			};

			enum Priority
			{
				PRIORITY_NORMAL,
				PRIORITY_ABOVE_NORMAL,
				PRIORITY_HIGH
			};

			void Save(Configuration&) const;

		private:

			typedef Application::Instance Instance;

			void UpdateSettings();
			void UpdateMenuColor() const;
			void OnCmdOptions(uint);
			void OnEmuEvent(Emulator::Event,Emulator::Data);
			void OnAppEvent(Instance::Event,const void*);

			struct
			{
				Collection::BitSet flags;
				Priority priority;
				Nes::Machine::FavoredSystem favoredSystem;
				bool alwaysAskSystem;
				bool disableStatusMsg;
			}   settings;

			Object::Heap<Window::Preferences> dialog;
			bool inFullscreen;

		public:

			bool operator [] (Type type) const
			{
				return settings.flags[type];
			}

			Priority GetPriority() const
			{
				return settings.priority;
			}

			Nes::Machine::FavoredSystem GetFavoredSystem() const
			{
				return settings.favoredSystem;
			}

			Nes::Machine::AskProfile GetAlwaysAskProfile() const
			{
				return settings.alwaysAskSystem ? Nes::Machine::ASK_PROFILE : Nes::Machine::DONT_ASK_PROFILE;
			}

			bool GetDisableStatusMsg() const
			{
				return settings.disableStatusMsg;
			}
		};
	}
}

#endif
