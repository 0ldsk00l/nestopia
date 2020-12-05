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

#ifndef NST_CTRL_COMBOBOX_H
#define NST_CTRL_COMBOBOX_H

#pragma once

#include "NstCtrlStandard.hpp"

namespace Nestopia
{
	namespace Window
	{
		namespace Control
		{
			class ComboBox : public Generic
			{
			public:

				typedef ULONG_PTR Value;

			private:

				class Item : public ImplicitBool<Item>
				{
					class DataProxy
					{
						const Item& item;

					public:

						DataProxy(const Item& i)
						: item(i) {}

						operator Value () const;
						void operator = (const Value) const;
					};

					const Window::Generic control;
					const int index;

				public:

					Item(Window::Generic w,int i)
					: control(w), index(i) {}

					void Erase() const;
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
				};

			public:

				ComboBox(HWND hWnd=NULL)
				: Generic( hWnd ) {}

				ComboBox(HWND hWnd,uint id)
				: Generic( hWnd, id ) {}

				void Reserve(uint,uint) const;
				Item Add(wcstring) const;
				void Clear() const;
				Item Selection() const;
				uint Size() const;

				Item operator [] (uint i) const
				{
					return Item( control, i );
				}

				Item Back() const
				{
					return Item( control, Size() - 1 );
				}

				template<typename T>
				void Add(T* list,uint count) const
				{
					for (uint i=0; i < count; ++i)
						Add( list[i] );
				}
			};
		}
	}
}

#endif
