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

#include "NstManagerPaths.hpp"
#include "../core/api/NstApiCartridge.hpp"
#include "NstDialogInesHeader.hpp"
#include "NstManagerInesHeader.hpp"

namespace Nestopia
{
	namespace Managers
	{
		InesHeader::InesHeader(Emulator& e,Window::Menu& m,const Paths& p)
		:
		Manager ( e, m, this, &InesHeader::OnEmuEvent, IDM_FILE_EDIT_INES_HEADER, &InesHeader::OnCmdEditInesHeader ),
		paths   ( p )
		{
		}

		void InesHeader::OnCmdEditInesHeader(uint)
		{
			Window::InesHeader( paths ).Open( paths.BrowseLoad( Paths::File::INES ) );
		}

		void InesHeader::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_NETPLAY_MODE:

					menu[IDM_FILE_EDIT_INES_HEADER].Enable( !data );
					break;
			}
		}
	}
}
