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

#include "NstString.hpp"
#include "NstWindowGeneric.hpp"
#include <WindowsX.h>

namespace Nestopia
{
	namespace Window
	{
		Rect Generic::GetPlacement() const
		{
			WINDOWPLACEMENT placement;

			placement.length = sizeof(placement);

			if (::GetWindowPlacement( hWnd, &placement ))
				return placement.rcNormalPosition;

			return 0;
		}

		void Generic::SetPlacement(const Rect& rect) const
		{
			WINDOWPLACEMENT placement;

			placement.length = sizeof(placement);

			if (::GetWindowPlacement( hWnd, &placement ))
			{
				placement.length = sizeof(placement);
				placement.rcNormalPosition = rect;
				::SetWindowPlacement( hWnd, &placement );
			}
		}

		Point Generic::SizeProxy::Coordinates() const
		{
			return Rect::Window(hWnd).Size();
		}

		void Generic::SizeProxy::Set(const Point& p) const
		{
			::SetWindowPos
			(
				hWnd,
				NULL,
				0,
				0,
				p.x,
				p.y,
				SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE
			);
		}

		void Generic::SizeProxy::operator = (const Point& p) const
		{
			Set( p );
		}

		void Generic::SizeProxy::operator += (const Point& p) const
		{
			Set( Coordinates() + p );
		}

		void Generic::SizeProxy::operator -= (const Point& p) const
		{
			Set( Coordinates() - p );
		}

		Point Generic::PositionProxy::Coordinates() const
		{
			return Rect::Window(hWnd).Position();
		}

		void Generic::PositionProxy::Set(Point p) const
		{
			if (HWND const hParent = ::GetParent( hWnd ))
				p.ClientTransform( hParent );

			::SetWindowPos
			(
				hWnd,
				NULL,
				p.x,
				p.y,
				0,
				0,
				SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE
			);
		}

		void Generic::PositionProxy::operator = (const Point& p) const
		{
			Set( p );
		}

		void Generic::PositionProxy::operator += (const Point& p) const
		{
			Set( Coordinates() + p );
		}

		void Generic::PositionProxy::operator -= (const Point& p) const
		{
			Set( Coordinates() - p );
		}

		void Generic::PositionProxy::BringBehind(HWND hOther) const
		{
			::SetWindowPos( hWnd, hOther, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE );
		}

		Generic::StyleProxy::operator long () const
		{
			return ::GetWindowLong( hWnd, ex ? GWL_EXSTYLE : GWL_STYLE ) & flags;
		}

		void Generic::StyleProxy::operator = (bool on) const
		{
			const long style = ::GetWindowLong( hWnd, ex ? GWL_EXSTYLE : GWL_STYLE );

			if ((style & flags) != (on ? flags : 0))
			{
				::SetWindowLong( hWnd, ex ? GWL_EXSTYLE : GWL_STYLE, (style & ~flags) | (on ? flags : 0) );
				::SetWindowPos( hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED );
			}
		}

		void Generic::Post(uint msg) const
		{
			Post( msg, 0, 0 );
		}

		LONG_PTR Generic::Send(uint msg) const
		{
			return Send( msg, 0, 0 );
		}

		void Generic::SendCommand(uint id) const
		{
			Send( WM_COMMAND, id, 0 );
		}

		void Generic::PostCommand(uint id) const
		{
			Post( WM_COMMAND, id, 0 );
		}

		bool Generic::Enabled() const
		{
			return ::IsWindowEnabled( hWnd );
		}

		bool Generic::Active() const
		{
			return hWnd && hWnd == ::GetActiveWindow();
		}

		bool Generic::Focused() const
		{
			return hWnd && hWnd == ::GetFocus();
		}

		bool Generic::Visible() const
		{
			return ::IsWindowVisible( hWnd );
		}

		bool Generic::Enable(bool enable) const
		{
			bool focused = (!enable && ::GetFocus() == hWnd);
			bool prevDisabled = ::EnableWindow( hWnd, enable );

			if (!prevDisabled && focused)
			{
				if (HWND parent = ::GetParent( hWnd ))
					::SetFocus( parent );
			}

			return prevDisabled;
		}

		void Generic::Show(bool show) const
		{
			::ShowWindow( hWnd, show ? SW_SHOW : SW_HIDE );
		}

		void Generic::Maximize() const
		{
			if (SameThread())
				Send( WM_SYSCOMMAND, SC_MAXIMIZE, 0 );
			else
				Post( WM_SYSCOMMAND, SC_MAXIMIZE, 0 );
		}

		void Generic::Minimize() const
		{
			if (SameThread())
				Send( WM_SYSCOMMAND, SC_MINIMIZE, 0 );
			else
				Post( WM_SYSCOMMAND, SC_MINIMIZE, 0 );
		}

		void Generic::Restore() const
		{
			if (SameThread())
				Send( WM_SYSCOMMAND, SC_RESTORE, 0 );
			else
				Post( WM_SYSCOMMAND, SC_RESTORE, 0 );
		}

		bool Generic::Maximized() const
		{
			return ::IsZoomed( hWnd );
		}

		bool Generic::Minimized() const
		{
			return ::IsIconic( hWnd );
		}

		bool Generic::Restored() const
		{
			return !::IsZoomed( hWnd ) && !::IsIconic( hWnd );
		}

		bool Generic::SameThread() const
		{
			return ::GetCurrentThreadId() == ::GetWindowThreadProcessId( hWnd, NULL );
		}

		void Generic::MakeTopMost(bool topMost) const
		{
			::SetWindowPos
			(
				hWnd,
				topMost ? HWND_TOPMOST : HWND_NOTOPMOST,
				0,
				0,
				0,
				0,
				SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE
			);
		}

		void Generic::MakeWindowed(long style,long exStyle) const
		{
			::SetWindowLongPtr( hWnd, GWL_STYLE, style );
			::SetWindowLongPtr( hWnd, GWL_EXSTYLE, exStyle );
			::SetWindowPos( hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED );
		}

		void Generic::MakeFullscreen() const
		{
			::SetWindowLongPtr( hWnd, GWL_STYLE, WS_POPUP|WS_VISIBLE );
			::SetWindowLongPtr( hWnd, GWL_EXSTYLE, 0 );

			::SetWindowPos
			(
				hWnd,
				NULL,
				0,
				0,
				::GetSystemMetrics( SM_CXSCREEN ),
				::GetSystemMetrics( SM_CYSCREEN ),
				SWP_NOZORDER|SWP_FRAMECHANGED
			);
		}

		bool Generic::Activate() const
		{
			if (::IsWindowEnabled( hWnd ))
			{
				if (::IsIconic( hWnd ))
					Restore();
				else
					::SetForegroundWindow( hWnd );

				return true;
			}

			return false;
		}

		void Generic::Redraw(bool redraw) const
		{
			if (redraw)
				::InvalidateRect( hWnd, NULL, true );
			else
				::ValidateRect( hWnd, NULL );
		}

		Generic Generic::Parent() const
		{
			return ::GetParent( hWnd );
		}

		void Generic::Close() const
		{
			if (hWnd)
			{
				if (SameThread())
					Send( WM_SYSCOMMAND, SC_CLOSE, 0 );
				else
					Post( WM_SYSCOMMAND, SC_CLOSE, 0 );
			}
		}

		bool Generic::Destroy()
		{
			NST_VERIFY( !hWnd || ::IsWindow( hWnd ) );

			if (hWnd)
			{
				bool result = ::DestroyWindow( hWnd );
				hWnd = NULL;
				return result;
			}

			return false;
		}

		Generic Generic::Find(wcstring className)
		{
			return className ? ::FindWindow( className, NULL ) : NULL;
		}

		int Generic::Stream::GetTextLength(const char*) const
		{
			return ::GetWindowTextLengthA( hWnd );
		}

		int Generic::Stream::GetTextLength(const wchar_t*) const
		{
			return ::GetWindowTextLengthW( hWnd );
		}

		int Generic::Stream::GetText(char* string,uint maxLength) const
		{
			return ::GetWindowTextA( hWnd, string, maxLength + 1 );
		}

		int Generic::Stream::GetText(wchar_t* string,uint maxLength) const
		{
			return ::GetWindowTextW( hWnd, string, maxLength + 1 );
		}

		void Generic::Stream::operator << (const char* string) const
		{
			NST_ASSERT( string );
			::SetWindowTextA( hWnd, string );
		}

		void Generic::Stream::operator << (const wchar_t* string) const
		{
			NST_ASSERT( string );
			::SetWindowTextW( hWnd, string );
		}

		void Generic::Stream::operator << (int value) const
		{
			operator << (ValueString(value).Ptr());
		}

		void Generic::Stream::operator << (uint value) const
		{
			operator << (ValueString(value).Ptr());
		}

		uint Generic::Stream::operator >> (ulong& value) const
		{
			String::Stack<16,wchar_t> string;
			string.ShrinkTo( ::GetWindowText( hWnd, string.Ptr(), 16+1 ) );
			string >> value;
			return string.Length();
		}

		uint Generic::Stream::operator >> (long& value) const
		{
			String::Stack<16,wchar_t> string;
			string.ShrinkTo( ::GetWindowText( hWnd, string.Ptr(), 16+1 ) );
			string >> value;
			return string.Length();
		}

		void Generic::Stream::Clear() const
		{
			::SetWindowText( hWnd, L"" );
		}

		Generic::LockDraw::LockDraw(HWND h)
		: hWnd(h)
		{
			NST_ASSERT( hWnd );
			SetWindowRedraw( hWnd, false );
		}

		Generic::LockDraw::~LockDraw()
		{
			SetWindowRedraw( hWnd, true );
			::InvalidateRect( hWnd, NULL, false );
		}
	}
}
