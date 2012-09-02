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

#include "NstResourceMenu.hpp"
#include "NstApplicationLanguage.hpp"

namespace Nestopia
{
	namespace Resource
	{
		Menu::Menu(const uint id)
		: handle(::LoadMenu(Application::Instance::GetLanguage().GetResourceHandle(),MAKEINTRESOURCE(id)))
		{
			if (handle)
				Instance::Events::Add( this, &Menu::OnAppEvent );
		}

		void Menu::RemoveFrom(HWND const hWnd) const
		{
			NST_ASSERT( handle );

			if (hWnd && handle == ::GetMenu( hWnd ))
				::SetMenu( hWnd, NULL );
		}

		Menu::~Menu()
		{
			if (handle)
			{
				Instance::Events::Remove( this );

				RemoveFrom( Instance::GetMainWindow() );

				for (uint i=Instance::NumChildWindows(); i--; )
					RemoveFrom( Instance::GetChildWindow( i ) );

				::DestroyMenu( handle );
			}
		}

		void Menu::OnAppEvent(Instance::Event event,const void* param)
		{
			if (event == Instance::EVENT_WINDOW_DESTROY && handle)
				RemoveFrom( static_cast<const Instance::Events::WindowDestroyParam*>(param)->hWnd );
		}
	}
}
