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

#ifndef NST_MANAGER_FILES_H
#define NST_MANAGER_FILES_H

#pragma once

namespace Nestopia
{
	namespace Window
	{
		class Main;
	}

	namespace Managers
	{
		class Paths;

		class Files : Manager
		{
		public:

			Files
			(
				Emulator&,
				Window::Menu&,
				const Paths&,
				const Preferences&,
				Movie&,
				const TapeRecorder&,
				const Cheats&,
				const SaveStates&,
				Window::Main&
			);

			~Files();

		private:

			void Open(wcstring=NULL,uint=0) const;
			bool Close() const;
			void DisplayLoadMessage(bool) const;
			void UpdateMenu() const;
			void AutoStart() const;

			ibool OnMsgDropFiles (Window::Param&);
			ibool OnMsgCopyData  (Window::Param&);
			ibool OnMsgLaunch    (Window::Param&);

			void OnCmdOpen  (uint);
			void OnCmdClose (uint);

			void OnEmuEvent (Emulator::Event,Emulator::Data);

			const Paths& paths;
			const Preferences& preferences;
			Movie& movie;
			const TapeRecorder& tapeRecorder;
			const Cheats& cheats;
			const SaveStates& saveStates;
			Window::Main& window;
		};
	}
}

#endif
