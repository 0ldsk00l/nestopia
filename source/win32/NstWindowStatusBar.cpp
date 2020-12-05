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

#include "language/resource.h"
#include "NstApplicationException.hpp"
#include "NstWindowCustom.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowStatusBar.hpp"
#include <CommCtrl.h>

namespace Nestopia
{
	namespace Window
	{
		inline StatusBar::Width::Width(const uint n)
		: numChars(n) {}

		void StatusBar::Width::Calculate(HWND const hWnd)
		{
			NST_ASSERT( hWnd );

			character = DEF_CHAR_WIDTH;
			first = DEF_FIRST_WIDTH;

			if (HDC const hDC = ::GetDC( hWnd ))
			{
				TEXTMETRIC tm;

				if (::GetTextMetrics( hDC, &tm ))
					character = tm.tmAveCharWidth;

				first = character * numChars;

				::ReleaseDC( hWnd, hDC );
			}
		}

		StatusBar::StatusBar(Custom& p,const uint numChars)
		:
		width  ( numChars ),
		parent ( p )
		{
			static const MsgHandler::HookEntry<StatusBar> hooks[] =
			{
				{ WM_SIZE,    &StatusBar::OnSize    },
				{ WM_DESTROY, &StatusBar::OnDestroy }
			};

			parent.Messages().Hooks().Add( this, hooks );
		}

		StatusBar::~StatusBar()
		{
			window.Destroy();
			parent.Messages().Hooks().Remove( this );
		}

		void StatusBar::Enable(const bool enable,const bool show)
		{
			NST_ASSERT( bool(parent) );

			if (enable)
			{
				if (window == NULL)
				{
					window = CreateWindowEx
					(
						0,
						STATUSCLASSNAME,
						NULL,
						WS_CHILD|SBARS_SIZEGRIP|(show ? WS_VISIBLE : 0),
						0,0,0,0,
						parent,
						reinterpret_cast<HMENU>(CHILD_ID),
						::GetModuleHandle(NULL),
						NULL
					);

					if (window == NULL)
						throw Application::Exception( IDS_ERR_FAILED, L"CreateWindowEx()" );

					width.Calculate( window );
					Update();
				}
			}
			else
			{
				window.Destroy();
			}
		}

		void StatusBar::OnDestroy(Param&)
		{
			window.Destroy();
		}

		void StatusBar::Show() const
		{
			window.Show();
		}

		void StatusBar::Stream::operator << (wcstring text) const
		{
			if (window)
				window.Send( SB_SETTEXT, field|0, text );
		}

		void StatusBar::Stream::Clear() const
		{
			operator << (L"");
		}

		uint StatusBar::GetMaxMessageLength() const
		{
			if (window)
			{
				int list[2] = {0,0};
				window.Send( SB_GETPARTS, 2, list );

				if (list[0] > int(width.character))
					return uint(list[0]) / (width.character - 1);
			}

			return 0;
		}

		void StatusBar::Update() const
		{
			NST_ASSERT( window && parent );

			int list[2] = { parent.ClientCoordinates().x - int(width.first), -1 };
			window.Send( SB_SETPARTS, 2, list );
		}

		void StatusBar::OnSize(Param& param)
		{
			if (window)
			{
				window.Send( WM_SIZE, param.wParam, param.lParam );
				Update();
			}
		}

		uint StatusBar::Height() const
		{
			if (window)
			{
				const int height = window.Coordinates().Height();

				if (height > 0)
					return height;
			}

			return 0;
		}
	}
}

