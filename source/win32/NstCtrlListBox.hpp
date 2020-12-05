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

#ifndef NST_CTRL_LISTBOX_H
#define NST_CTRL_LISTBOX_H

#pragma once

#include "NstCtrlStandard.hpp"

namespace Nestopia
{
	namespace Window
	{
		namespace Control
		{
			class ListBox : public Generic
			{
				class Item : public ImplicitBool<Item>
				{
					class DataProxy
					{
						const Item& item;

					public:

						DataProxy(const Item& i)
						: item(i) {}

						operator ULONG_PTR () const;
						void operator = (ULONG_PTR) const;
					};

					class TextProxy
					{
						const Item& item;

					public:

						TextProxy(const Item& i)
						: item(i) {}

						void operator << (wcstring) const;

						template<typename T>
						uint operator >> (T&) const;
					};

					const Window::Generic control;
					const int index;

				public:

					Item(Window::Generic w,uint i)
					: control(w), index(i) {}

					void Remove() const;
					void Select() const;

					int GetIndex() const
					{
						return index;
					}

					bool operator ! () const
					{
						return index < 0;
					}

					DataProxy Data() const
					{
						return *this;
					}

					TextProxy Text() const
					{
						return *this;
					}
				};

			public:

				ListBox(HWND hWnd=NULL)
				: Generic( hWnd ) {}

				ListBox(HWND hWnd,uint id)
				: Generic( hWnd, id ) {}

				Item Add(wcstring) const;
				Item Insert(uint,wcstring) const;
				Item Selection() const;
				bool AnySelection() const;
				void Reserve(uint) const;
				uint Size() const;
				void Clear() const;

				template<typename T>
				void Add(T* list,uint count) const
				{
					Reserve( count );

					for (uint i=0; i < count; ++i)
						Add( list[i] );
				}

				Item operator [] (uint i) const
				{
					return Item( control, i );
				}

				class HScrollBar
				{
					long width;
					HWND const hWnd;
					HDC const hDC;

				public:

					HScrollBar(HWND);
					~HScrollBar();

					void Update(wcstring,uint);
				};
			};

			template<typename T>
			uint ListBox::Item::TextProxy::operator >> (T& string) const
			{
				NST_VERIFY( item.control );

				const int size = item.control.Send( LB_GETTEXTLEN, item.index, 0 );

				if (size > 0)
				{
					string.Resize( size );

					if (item.control.Send( LB_GETTEXT, item.index, static_cast<wchar_t*>(string.Ptr()) ) > 0)
						return string.Validate();
				}

				string.Clear();
				return 0;
			}
		}
	}
}

#endif
