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
#include "NstCtrlListView.hpp"
#include <CommCtrl.h>

namespace Nestopia
{
	namespace Window
	{
		namespace Control
		{
			void ListView::ColumnsProxy::Insert(const uint index,wcstring const text) const
			{
				NST_ASSERT( text );

				LVCOLUMN lvColumn;

				lvColumn.mask = LVCF_FMT|LVCF_TEXT;
				lvColumn.fmt = LVCFMT_LEFT;
				lvColumn.pszText = const_cast<wchar_t*>( text );

				ListView_InsertColumn( control, index, &lvColumn );
			}

			void ListView::ColumnsProxy::Align() const
			{
				for (int i=0; ListView_SetColumnWidth( control, i, LVSCW_AUTOSIZE_USEHEADER ); ++i)
				{
					const int header = ListView_GetColumnWidth( control, i );

					ListView_SetColumnWidth( control, i, LVSCW_AUTOSIZE );

					const int width = ListView_GetColumnWidth( control, i );

					if (width < header)
						ListView_SetColumnWidth( control, i, header );
				}
			}

			void ListView::ColumnsProxy::GetOrder(int* const array,const uint count) const
			{
				ListView_GetColumnOrderArray( control, count, array );
			}

			uint ListView::ColumnsProxy::GetIndex(const uint index) const
			{
				LVCOLUMN lvColumn;
				lvColumn.mask = LVCF_ORDER;

				ListView_GetColumn( control, index, &lvColumn );

				return NST_MAX(lvColumn.iOrder,0);
			}

			void ListView::ColumnsProxy::Clear() const
			{
				while (ListView_DeleteColumn( control, 0 ));
			}

			ListView::StyleExProxy::operator uint () const
			{
				return ListView_GetExtendedListViewStyle( control );
			}

			void ListView::StyleExProxy::operator = (const uint style) const
			{
				ListView_SetExtendedListViewStyle( control, style );
			}

			void ListView::Item::TextProxy::operator << (wcstring const text) const
			{
				ListView_SetItemText( item.control, item.index, index, const_cast<wchar_t*>(text) );
			}

			void ListView::Item::TextProxy::GetText(Buffer& buffer) const
			{
				NST_ASSERT( buffer.Empty() );

				LVITEM lvItem;
				lvItem.iSubItem = index;

				do
				{
					buffer.Resize( buffer.Length() + BLOCK_SIZE );

					lvItem.cchTextMax = buffer.Length() + 1;
					lvItem.pszText = buffer.Ptr();

					lvItem.cchTextMax = item.control.Send( LVM_GETITEMTEXT, item.index, &lvItem );
				}
				while (*(buffer.Ptr() + buffer.Length()));

				buffer.ShrinkTo( lvItem.cchTextMax );
			}

			void ListView::Item::DataProxy::operator = (const LPARAM data) const
			{
				LVITEM lvItem;

				lvItem.mask = LVIF_PARAM;
				lvItem.iItem = item.index;
				lvItem.iSubItem = 0;
				lvItem.lParam = data;

				ListView_SetItem( item.control, &lvItem );
			}

			ListView::Item::DataProxy::operator LPARAM () const
			{
				LVITEM lvItem;

				lvItem.mask = LVIF_PARAM;
				lvItem.iItem = item.index;
				lvItem.iSubItem = 0;
				lvItem.lParam = 0;

				ListView_GetItem( item.control, &lvItem );

				return lvItem.lParam;
			}

			bool ListView::Item::Delete() const
			{
				return index != ~0U ? ListView_DeleteItem( control, index ) : false;
			}

			void ListView::Item::Select(const bool state) const
			{
				ListView_SetItemState( control, index, state ? LVIS_SELECTED : 0, LVIS_SELECTED );
			}

			void ListView::Item::Check(const bool state) const
			{
				ListView_SetCheckState( control, index, state );
			}

			bool ListView::Item::Checked() const
			{
				return ListView_GetCheckState( control, index );
			}

			void ListView::Item::Show() const
			{
				ListView_EnsureVisible( control, index, false );
			}

			ListView::Item ListView::Selection() const
			{
				return Item( control, ListView_GetNextItem( control, -1, LVNI_SELECTED ) );
			}

			int ListView::Add(GenericString text,const LPARAM data,const bool checked) const
			{
				HeapString tmp;

				LVITEM lvItem;

				if (text.Length())
				{
					if (text.NullTerminated())
					{
						lvItem.pszText = const_cast<wchar_t*>(text.Ptr());
					}
					else
					{
						tmp = text;
						lvItem.pszText = tmp.Ptr();
					}
				}
				else
				{
					lvItem.pszText = LPSTR_TEXTCALLBACK;
				}

				lvItem.mask = LVIF_TEXT;

				if (data)
					lvItem.mask |= LVIF_PARAM;

				lvItem.iItem = INT_MAX;
				lvItem.iSubItem = 0;
				lvItem.lParam = data;

				const int index = ListView_InsertItem( control, &lvItem );

				if (checked)
					ListView_SetCheckState( control, index, true );

				return index;
			}

			uint ListView::Size() const
			{
				const int count = ListView_GetItemCount( control );
				return NST_MAX(count,0);
			}

			void ListView::Clear() const
			{
				ListView_DeleteAllItems( control );
			}

			void ListView::Reserve(const uint capacity) const
			{
				if (capacity > Size())
					ListView_SetItemCount( control, capacity );
			}

			void ListView::SetBkColor(const uint color) const
			{
				ListView_SetBkColor( control, color );
			}

			void ListView::SetTextColor(const uint color) const
			{
				ListView_SetTextColor( control, color );
			}

			void ListView::SetTextBkColor(const uint color) const
			{
				ListView_SetTextBkColor( control, color );
			}

			bool ListView::HitTest(const uint x,const uint y) const
			{
				LVHITTESTINFO lvHitTestInfo;

				lvHitTestInfo.pt.x = x;
				lvHitTestInfo.pt.y = y;

				::ScreenToClient( control, &lvHitTestInfo.pt );

				return ListView_HitTest( control, &lvHitTestInfo ) != -1;
			}

			void ListView::Sort(const SortFunction& sortFunction) const
			{
				if (Size() > 1)
					ListView_SortItems( control, SortProc, reinterpret_cast<WPARAM>(&sortFunction) );
			}

			int CALLBACK ListView::SortProc(LPARAM obj1,LPARAM obj2,LPARAM ref)
			{
				return (*reinterpret_cast<const SortFunction*>(ref))
				(
					reinterpret_cast<const void*>( obj1 ),
					reinterpret_cast<const void*>( obj2 )
				);
			}
		}
	}
}
