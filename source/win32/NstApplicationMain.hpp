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

#ifndef NST_APPLICATION_MAIN_H
#define NST_APPLICATION_MAIN_H

#pragma once

#include "NstApplicationInstance.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerLogfile.hpp"
#include "NstManagerPreferences.hpp"
#include "NstManagerLauncher.hpp"
#include "NstManagerMachine.hpp"
#include "NstManagerNetplay.hpp"
#include "NstManagerFds.hpp"
#include "NstManagerTapeRecorder.hpp"
#include "NstManagerDipSwitches.hpp"
#include "NstManagerBarcodeReader.hpp"
#include "NstManagerNsf.hpp"
#include "NstManagerMovie.hpp"
#include "NstManagerSaveStates.hpp"
#include "NstManagerCheats.hpp"
#include "NstManagerRecentFiles.hpp"
#include "NstManagerRecentDirs.hpp"
#include "NstManagerImageInfo.hpp"
#include "NstManagerImageDatabase.hpp"
#include "NstManagerLanguage.hpp"
#include "NstManagerHelp.hpp"
#include "NstManagerFiles.hpp"
#include "NstManagerInesHeader.hpp"
#include "NstWindowMain.hpp"

namespace Nestopia
{
	namespace Application
	{
		class Main
		{
		public:

			explicit Main(int);
			~Main();

			int Run();

		private:

			void Save();
			void Exit();

			bool FirstUnloadOnExit();
			bool OkToExit() const;

			ibool OnWinClose           (Window::Param&);
			ibool OnWinQueryEndSession (Window::Param&);

			void OnCmdFileExit(uint);

			Managers::Emulator emulator;
			Instance instance;
			Window::Menu menu;
			Managers::Preferences preferences;
			Managers::Logfile logfile;
			Managers::Language language;
			Managers::Paths paths;
			Window::Main window;
			Managers::RecentFiles recentFiles;
			Managers::RecentDirs recentDirs;
			Managers::Machine machine;
			Managers::Netplay netplay;
			Managers::Launcher launcher;
			Managers::Fds fds;
			Managers::TapeRecorder tapeRecorder;
			Managers::DipSwitches dipSwitches;
			Managers::BarcodeReader barcodeReader;
			Managers::Nsf nsf;
			Managers::Movie movie;
			Managers::Cheats cheats;
			Managers::SaveStates saveStates;
			Managers::ImageDatabase imageDatabase;
			Managers::ImageInfo imageInfo;
			Managers::Help help;
			Managers::InesHeader inesHeader;
			Managers::Files files;
		};
	}
}

#endif
