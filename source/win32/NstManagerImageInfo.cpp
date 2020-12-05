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
#include "NstDialogImageInfo.hpp"
#include "NstManagerImageInfo.hpp"

namespace Nestopia
{
	namespace Managers
	{
		ImageInfo::ImageInfo(Emulator& e,Window::Menu& m)
		: Manager(e,m,this,&ImageInfo::OnEmuEvent,IDM_VIEW_IMAGE_INFO,&ImageInfo::OnCmd)
		{
			UpdateMenu();
		}

		void ImageInfo::OnCmd(uint)
		{
			Window::ImageInfo( emulator ).Open();
		}

		bool ImageInfo::Available() const
		{
			return emulator.NetPlayers() == 0 && emulator.IsImage();
		}

		void ImageInfo::UpdateMenu() const
		{
			menu[IDM_VIEW_IMAGE_INFO].Enable( Available() );
		}

		void ImageInfo::OnEmuEvent(const Emulator::Event event,Emulator::Data)
		{
			switch (event)
			{
				case Emulator::EVENT_LOAD:
				case Emulator::EVENT_UNLOAD:

					UpdateMenu();
					break;
			}
		}
	}
}
