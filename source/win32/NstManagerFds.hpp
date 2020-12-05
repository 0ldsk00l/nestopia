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

#ifndef NST_MANAGER_FDS_H
#define NST_MANAGER_FDS_H

#pragma once

#include "../core/api/NstApiFds.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Fds;
	}

	namespace Managers
	{
		class Paths;

		class Fds : Manager
		{
		public:

			Fds(Emulator&,const Configuration&,Window::Menu&,const Paths&);
			~Fds();

			void Save(Configuration&) const;

		private:

			struct Callbacks;

			enum
			{
				MAX_SIDES = IDM_MACHINE_EXT_FDS_INSERT_DISK_8_SIDE_B+1 - IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A
			};

			bool CanInsertDisk() const;
			bool CanEjectDisk() const;
			bool CanChangeSide() const;

			void UpdateSettings() const;
			void UpdateMenuDisks() const;

			void OnMenuExtFds       (const Window::Menu::PopupHandler::Param&);
			void OnMenuExtFdsInsert (const Window::Menu::PopupHandler::Param&);
			void OnEmuEvent         (Emulator::Event,Emulator::Data);
			void OnCmdInsertDisk    (uint);
			void OnCmdChangeSide    (uint);
			void OnCmdEjectDisk     (uint);
			void OnCmdOptions       (uint);

			Object::Heap<Window::Fds> dialog;
		};
	}
}

#endif
