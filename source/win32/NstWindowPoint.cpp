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

#include "NstWindowPoint.hpp"

namespace Nestopia
{
	namespace Window
	{
		uint Point::ScaleToFit(const uint px,const uint py,const Scaling type,const uint limit)
		{
			if (x <= 0)
				x = 1;

			if (y <= 0)
				y = 1;

			const int ox = x;
			const int oy = y;
			uint s = 0;

			while (x + ox <= int(px) && y + oy <= int(py) && s < limit)
				x += ox, y += oy, ++s;

			if
			(
				s < limit &&
				(
					(type == SCALE_ABOVE && x < int(px) && y < int(py)) ||
					(type == SCALE_NEAREST && (px-x) * (py-y) > (x+ox-px) * (y+oy-py))
				)
			)
				x += ox, y += oy, ++s;

			return s;
		}

		Point::Client::Client(HWND const hWnd)
		{
			RECT tmp;
			::GetClientRect( hWnd, &tmp );
			NST_VERIFY( !tmp.left && !tmp.top );

			x = tmp.right;
			y = tmp.bottom;
		}

		Point::Picture::Picture(HWND const hWnd)
		{
			RECT tmp;
			::GetClientRect( hWnd, &tmp );
			NST_VERIFY( !tmp.left && !tmp.top );

			for (HWND hChild = ::GetTopWindow(hWnd); hChild; hChild = ::GetNextWindow(hChild,GW_HWNDNEXT))
			{
				RECT rChild;
				::GetClientRect( hChild, &rChild );
				tmp.bottom -= rChild.bottom - rChild.top;
			}

			if (tmp.bottom < 0)
				tmp.bottom = 0;

			x = tmp.right;
			y = tmp.bottom;
		}

		Point::NonClient::NonClient(HWND const hWnd)
		{
			RECT rect = {0,0,0,0};

			::AdjustWindowRectEx
			(
				&rect,
				::GetWindowLongPtr(hWnd,GWL_STYLE),
				false,
				::GetWindowLongPtr(hWnd,GWL_EXSTYLE)
			);

			x = rect.right - rect.left;
			y = rect.bottom - rect.top;

			if (::GetMenu( hWnd ))
			{
				MENUBARINFO mbi;
				mbi.cbSize = sizeof(mbi);

				if (::GetMenuBarInfo( hWnd, OBJID_MENU, 0, &mbi ) && mbi.rcBar.bottom >= mbi.rcBar.top)
					y += (mbi.rcBar.bottom - mbi.rcBar.top) + 1;
				else
					y += ::GetSystemMetrics( SM_CYMENU );
			}

			for (HWND hChild = ::GetTopWindow(hWnd); hChild; hChild = ::GetNextWindow(hChild,GW_HWNDNEXT))
			{
				::GetClientRect( hChild, &rect );
				y += rect.bottom - rect.top;
			}
		}
	}
}
