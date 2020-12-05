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

#include "resource/resource.h"
#include "NstObjectHeap.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogImageDatabase.hpp"
#include "NstManagerImageDatabase.hpp"

namespace Nestopia
{
	namespace Managers
	{
		ImageDatabase::ImageDatabase(Emulator& e,const Configuration& cfg,Window::Menu& m,const Paths& paths)
		:
		Manager ( e, m, this, &ImageDatabase::OnEmuEvent ),
		dialog  ( new Window::ImageDatabase(e,cfg,paths) )
		{
			menu.Commands().Add( IDM_OPTIONS_IMAGEDATABASE, this, &ImageDatabase::OnCmdImagedatabase );
		}

		ImageDatabase::~ImageDatabase()
		{
		}

		void ImageDatabase::Save(Configuration& cfg) const
		{
			dialog->Save( cfg );
		}

		void ImageDatabase::OnCmdImagedatabase(uint)
		{
			dialog->Open();
		}

		void ImageDatabase::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_NETPLAY_MODE:

					menu[IDM_OPTIONS_IMAGEDATABASE].Enable( !data );
					break;
			}
		}
	}
}
