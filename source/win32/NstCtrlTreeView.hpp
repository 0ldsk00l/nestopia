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

#ifndef NST_CTRL_TREEVIEW_H
#define NST_CTRL_TREEVIEW_H

#pragma once

#include "NstCtrlStandard.hpp"
#include "NstString.hpp"

namespace Nestopia
{
	namespace Window
	{
		namespace Control
		{
			class TreeView : public Generic
			{
				class Item
				{
					const Window::Generic root;
					void* const hItem;
					const int index;

				public:

					Item(Window::Generic w,void* h,int i)
					: root(w), hItem(h), index(i) {}

					void Select() const;

					int GetIndex() const
					{
						return index;
					}
				};

			public:

				TreeView(HWND hWnd=NULL)
				: Generic( hWnd ) {}

				TreeView(HWND hWnd,uint id)
				: Generic( hWnd, id ) {}

				class ImageList
				{
					friend class TreeView;

					void* const handle;

					enum {SELECTED,UNSELECTED};

				public:

					ImageList(uint,uint,uint,uint);
					~ImageList();
				};

				void SetImageList(const ImageList&) const;
				void Add(GenericString) const;
				Item Selection() const;
				void SetBackgroundColor(uint) const;
				void SetTextColor(uint) const;
				int  GetIndex(void*) const;

				Item operator [] (uint) const;
			};
		}
	}
}

#endif
