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
#include "NstWindowParam.hpp"
#include "NstManager.hpp"
#include "NstManagerInput.hpp"

namespace Nestopia
{
	namespace Managers
	{
		Input::Cursor::Cursor(Window::Custom& w,const Screening& i,const Screening& o)
		:
		hCursor           ( Resource::Cursor::GetArrow() ),
		hCurrent          ( Resource::Cursor::GetArrow() ),
		deadline          ( NO_DEADLINE ),
		window            ( w ),
		wheel             ( WHEEL_MAX ),
		getInputRect      ( i ),
		getOutputRect     ( o ),
		gun               ( IDC_CURSOR_GUN ),
		primaryButtonId   ( ::GetSystemMetrics( SM_SWAPBUTTON ) ? VK_RBUTTON : VK_LBUTTON ),
		secondaryButtonId ( primaryButtonId == VK_LBUTTON ? VK_RBUTTON : VK_LBUTTON )
		{
			static const Window::MsgHandler::Entry<Input::Cursor> messages[] =
			{
				{ WM_SETCURSOR,   &Cursor::OnSetCursor      },
				{ WM_MOUSEMOVE,   &Cursor::OnNop            },
				{ WM_LBUTTONDOWN, &Cursor::OnNop            },
				{ WM_RBUTTONDOWN, &Cursor::OnRButtonDownNop },
				{ WM_LBUTTONUP,   &Cursor::OnNop            },
				{ WM_RBUTTONUP,   &Cursor::OnNop            },
				{ WM_MOUSEWHEEL,  &Cursor::OnWheel          }
			};

			window.Messages().Add( this, messages );
		}

		Input::Cursor::~Cursor()
		{
			window.Messages().Remove( this );
		}

		void Input::Cursor::Acquire(Emulator& emulator)
		{
			bool autoHide = false;
			bool usesRButton = false;

			if
			(
				Nes::Input(emulator).IsControllerConnected( Nes::Input::ZAPPER ) ||
				Nes::Input(emulator).IsControllerConnected( Nes::Input::BANDAIHYPERSHOT )
			)
			{
				hCursor = gun;
				usesRButton = true;
			}
			else if (Nes::Input(emulator).IsControllerConnected( Nes::Input::POWERGLOVE ))
			{
				hCursor = NULL;
				usesRButton = true;
			}
			else if
			(
				Nes::Input(emulator).IsControllerConnected( Nes::Input::PADDLE ) ||
				Nes::Input(emulator).IsControllerConnected( Nes::Input::HORITRACK ) ||
				Nes::Input(emulator).IsControllerConnected( Nes::Input::PACHINKO ) ||
				Nes::Input(emulator).IsControllerConnected( Nes::Input::OEKAKIDSTABLET ) ||
				Nes::Input(emulator).IsControllerConnected( Nes::Input::MOUSE )
			)
			{
				hCursor = NULL;
			}
			else
			{
				hCursor = Resource::Cursor::GetArrow();
				autoHide = true;
			}

			Window::MsgHandler& router = window.Messages();

			router[ WM_MOUSEMOVE   ].Set( this, autoHide ? &Cursor::OnMouseMove   : &Cursor::OnNop );
			router[ WM_LBUTTONDOWN ].Set( this, autoHide ? &Cursor::OnLButtonDown : &Cursor::OnNop );
			router[ WM_LBUTTONUP   ].Set( this, autoHide ? &Cursor::OnButtonUp    : &Cursor::OnNop );
			router[ WM_RBUTTONDOWN ].Set( this, autoHide ? &Cursor::OnRButtonDown : usesRButton ? &Cursor::OnNop : &Cursor::OnRButtonDownNop );
			router[ WM_RBUTTONUP   ].Set( this, autoHide ? &Cursor::OnButtonUp    : &Cursor::OnNop );

			hCurrent = hCursor;
			deadline = autoHide ? ::GetTickCount() + TIME_OUT : NO_DEADLINE;
		}

		void Input::Cursor::Unacquire()
		{
			deadline = NO_DEADLINE;
			hCurrent = hCursor = Resource::Cursor::GetArrow();

			Window::MsgHandler& router = window.Messages();

			router[ WM_MOUSEMOVE   ].Set( this, &Cursor::OnNop            );
			router[ WM_LBUTTONDOWN ].Set( this, &Cursor::OnNop            );
			router[ WM_LBUTTONUP   ].Set( this, &Cursor::OnNop            );
			router[ WM_RBUTTONDOWN ].Set( this, &Cursor::OnRButtonDownNop );
			router[ WM_RBUTTONUP   ].Set( this, &Cursor::OnNop            );
		}

		void Input::Cursor::AutoHide()
		{
			deadline = NO_DEADLINE;

			if (window.Focused())
			{
				Window::Point pos;
				::GetCursorPos( &pos );

				if (Window::Rect(window.PictureCoordinates()).ScreenTransform(window).Inside( pos ))
					::SetCursor( hCurrent=NULL );
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		bool Input::Cursor::Poll(uint& x,uint& y,uint& lbutton,uint* const rbutton) const
		{
			POINT mouse;
			::GetCursorPos( &mouse );

			Window::Rect output;
			getOutputRect( output );

			if (output.Inside( mouse ))
			{
				Window::Rect input;
				getInputRect( input );

				x = input.left + (mouse.x - output.left) * (uint(input.Width())-1) / output.Width();
				y = input.top + (mouse.y - output.top) * (uint(input.Height())-1) / output.Height();

				if (window.Active())
				{
					lbutton = ::GetAsyncKeyState( primaryButtonId ) & 0x8000U;

					if (rbutton)
						*rbutton = ::GetAsyncKeyState( secondaryButtonId ) & 0x8000U;

					return true;
				}
			}

			lbutton = 0;

			if (rbutton)
				*rbutton = 0;

			return false;
		}

		ibool Input::Cursor::OnNop(Window::Param&)
		{
			return true;
		}

		ibool Input::Cursor::OnSetCursor(Window::Param& param)
		{
			if (LOWORD(param.lParam) == HTCLIENT)
			{
				param.lResult = true;
				::SetCursor( hCurrent );
				return true;
			}

			return false;
		}

		ibool Input::Cursor::OnMouseMove(Window::Param&)
		{
			hCurrent = hCursor;
			deadline = ::GetTickCount() + TIME_OUT;
			return true;
		}

		ibool Input::Cursor::OnLButtonDown(Window::Param&)
		{
			deadline = NO_DEADLINE;
			::SetCursor( hCurrent=hCursor );
			return true;
		}

		ibool Input::Cursor::OnRButtonDown(Window::Param&)
		{
			deadline = NO_DEADLINE;
			::SetCursor( hCurrent=hCursor );
			window.SendCommand( IDM_VIEW_MENU );
			return true;
		}

		ibool Input::Cursor::OnRButtonDownNop(Window::Param&)
		{
			window.SendCommand( IDM_VIEW_MENU );
			return true;
		}

		ibool Input::Cursor::OnButtonUp(Window::Param&)
		{
			::SetCursor( hCurrent=hCursor );
			deadline = ::GetTickCount() + TIME_OUT;
			return true;
		}

		ibool Input::Cursor::OnWheel(Window::Param& param)
		{
			wheel += GET_WHEEL_DELTA_WPARAM(param.wParam);
			wheel  = NST_CLAMP(wheel,WHEEL_MIN,WHEEL_MAX);

			return true;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
