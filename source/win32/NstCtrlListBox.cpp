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

#include "NstWindowCustom.hpp"
#include "NstCtrlListBox.hpp"
#include <WindowsX.h>

namespace Nestopia
{
	namespace Window
	{
		namespace Control
		{
			NST_COMPILE_ASSERT( LB_ERR < 0 && LB_ERRSPACE < 0 );

			ListBox::Item::DataProxy::operator ULONG_PTR () const
			{
				return ListBox_GetItemData( item.control, item.index );
			}

			void ListBox::Item::DataProxy::operator = (const ULONG_PTR data) const
			{
				ListBox_SetItemData( item.control, item.index, data );
			}

			void ListBox::Item::TextProxy::operator << (wcstring const string) const
			{
				NST_ASSERT( string );

				const int selection = ListBox_GetCurSel( item.control );

				ListBox_DeleteString( item.control, item.index );
				ListBox_InsertString( item.control, item.index, string );

				if (selection != LB_ERR)
					ListBox_SetCurSel( item.control, selection );
			}

			void ListBox::Item::Select() const
			{
				ListBox_SetCurSel( control, index );
			}

			void ListBox::Item::Remove() const
			{
				ListBox_DeleteString( control, index );
			}

			ListBox::Item ListBox::Selection() const
			{
				return Item( control, ListBox_GetCurSel( control ) );
			}

			bool ListBox::AnySelection() const
			{
				return ListBox_GetCurSel( control ) != LB_ERR;
			}

			ListBox::Item ListBox::Add(wcstring const text) const
			{
				NST_ASSERT( text );
				return Item( control, ListBox_AddString( control, text ) );
			}

			ListBox::Item ListBox::Insert(const uint index,wcstring const text) const
			{
				NST_ASSERT( text );
				return Item( control, ListBox_InsertString( control, index, text ) );
			}

			uint ListBox::Size() const
			{
				const int size = ListBox_GetCount( control );
				return NST_MAX(size,0);
			}

			void ListBox::Reserve(const uint capacity) const
			{
				if (capacity < Size())
					control.Send( LB_INITSTORAGE, capacity, 0 );
			}

			void ListBox::Clear() const
			{
				ListBox_ResetContent( control );
			}

			ListBox::HScrollBar::HScrollBar(HWND h)
			: width(0), hWnd(h), hDC(h ? ::GetDC(h) : NULL)
			{
				NST_VERIFY( hWnd );
			}

			void ListBox::HScrollBar::Update(wcstring string,uint length)
			{
				NST_ASSERT( string || !length );

				SIZE size;

				if (hDC && ::GetTextExtentPoint32( hDC, string, length, &size ) && width < size.cx)
					width = size.cx;
			}

			ListBox::HScrollBar::~HScrollBar()
			{
				if (hDC)
				{
					::ReleaseDC( hWnd, hDC );

					if (width)
					{
						RECT rect;
						::GetClientRect( hWnd, &rect );

						if (width > rect.right)
							ListBox_SetHorizontalExtent( hWnd, width );
					}
				}
			}
		}
	}
}
