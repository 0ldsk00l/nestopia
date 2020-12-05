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
#include "NstDialogBarcodeReader.hpp"
#include "NstManagerBarcodeReader.hpp"

namespace Nestopia
{
	namespace Managers
	{
		BarcodeReader::BarcodeReader(Emulator& e,Window::Menu& m)
		: Manager(e,m)
		{
			static const Window::Menu::CmdHandler::Entry<BarcodeReader> commands[] =
			{
				{ IDM_MACHINE_EXT_BARCODE, &BarcodeReader::OnMenuCmd }
			};

			menu.Commands().Add( this, commands );

			static const Window::Menu::PopupHandler::Entry<BarcodeReader> popups[] =
			{
				{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_EXT>::ID, &BarcodeReader::OnMenuExt }
			};

			menu.Popups().Add( this, popups );
		}

		BarcodeReader::~BarcodeReader()
		{
		}

		bool BarcodeReader::Available() const
		{
			return
			(
				Nes::BarcodeReader(emulator).IsConnected() &&
				!emulator.NetPlayers() &&
				emulator.IsOn() &&
				!emulator.IsLocked()
			);
		}

		void BarcodeReader::OnMenuExt(const Window::Menu::PopupHandler::Param& param)
		{
			param.menu[IDM_MACHINE_EXT_BARCODE].Enable( !param.show || Available() );
		}

		void BarcodeReader::OnMenuCmd(uint)
		{
			if (Available())
				Window::BarcodeReader( emulator, lastCode ).Open();
		}
	}
}
