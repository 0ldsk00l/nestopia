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
#include "NstResourceBitmap.hpp"
#include "NstWindowCustom.hpp"
#include "NstCtrlTreeView.hpp"
#include <CommCtrl.h>

namespace Nestopia
{
	namespace Window
	{
		namespace Control
		{
			TreeView::ImageList::ImageList(const uint x,const uint y,const uint selected,const uint unselected)
			: handle(ImageList_Create(x,y,ILC_COLOR16|ILC_MASK,0,2))
			{
				if (handle)
				{
					try
					{
						if
						(
							ImageList_AddMasked( static_cast<HIMAGELIST>(handle), Resource::Bitmap( selected   ), 0 ) == -1 ||
							ImageList_AddMasked( static_cast<HIMAGELIST>(handle), Resource::Bitmap( unselected ), 0 ) == -1
						)
							throw Application::Exception( IDS_ERR_FAILED, L"ImageList_Add()" );
					}
					catch (const Application::Exception& exception)
					{
						ImageList_Destroy( static_cast<HIMAGELIST>(handle) );
						throw exception;
					}
				}
				else
				{
					throw Application::Exception( IDS_ERR_FAILED, L"ImageList_Create()" );
				}
			}

			TreeView::ImageList::~ImageList()
			{
				if (handle)
					ImageList_Destroy( static_cast<HIMAGELIST>(handle) );
			}

			void TreeView::Item::Select() const
			{
				NST_VERIFY( hItem );
				TreeView_SelectItem( root, static_cast<HTREEITEM>(hItem) );
			}

			void TreeView::SetImageList(const ImageList& imageList) const
			{
				NST_VERIFY( imageList.handle );
				TreeView_SetImageList( control, static_cast<HIMAGELIST>(imageList.handle), TVSIL_NORMAL );
			}

			void TreeView::Add(const GenericString text) const
			{
				TVINSERTSTRUCT tv;

				tv.hParent = TVI_ROOT;
				tv.hInsertAfter = TVI_LAST;
				tv.item.mask = TVIF_TEXT;
				tv.item.pszText = const_cast<wchar_t*>( text.Ptr() );
				tv.item.cchTextMax = text.Length();

				if (TreeView_GetImageList( control, TVSIL_NORMAL ))
				{
					tv.item.mask = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
					tv.item.iImage = ImageList::UNSELECTED;
					tv.item.iSelectedImage = ImageList::SELECTED;
				}

				TreeView_InsertItem( control, &tv );
			}

			TreeView::Item TreeView::operator [] (const uint index) const
			{
				HTREEITEM hItem = TreeView_GetRoot( control );

				for (uint i=index; i; --i)
					hItem = TreeView_GetNextSibling( control, hItem );

				return Item( control, hItem, hItem ? int(index) : -1 );
			}

			int TreeView::GetIndex(void* const handle) const
			{
				int index = -1;

				if (handle)
				{
					index = 0;

					for (HTREEITEM hItem = TreeView_GetRoot( control ); hItem && hItem != static_cast<HTREEITEM>(handle); ++index)
						hItem = TreeView_GetNextSibling( control, hItem );
				}

				return index;
			}

			TreeView::Item TreeView::Selection() const
			{
				HTREEITEM const hSelection = TreeView_GetSelection( control );
				return Item( control, hSelection, GetIndex( hSelection ) );
			}

			void TreeView::SetBackgroundColor(const uint color) const
			{
				TreeView_SetBkColor( control, color );
			}

			void TreeView::SetTextColor(const uint color) const
			{
				TreeView_SetTextColor( control, color );
			}
		}
	}
}
