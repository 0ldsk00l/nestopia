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
#include "NstCtrlComboBox.hpp"
#include <WindowsX.h>

namespace Nestopia
{
	namespace Window
	{
		namespace Control
		{
			NST_COMPILE_ASSERT( CB_ERR < 0 && CB_ERRSPACE < 0 );

			ComboBox::Item::DataProxy::operator ComboBox::Value () const
			{
				return ComboBox_GetItemData( item.control, item.index );
			}

			void ComboBox::Item::DataProxy::operator = (Value data) const
			{
				ComboBox_SetItemData( item.control, item.index, data );
			}

			void ComboBox::Item::Select() const
			{
				ComboBox_SetCurSel( control, index );
			}

			void ComboBox::Item::Erase() const
			{
				ComboBox_DeleteString( control, index );
			}

			ComboBox::Item ComboBox::Add(wcstring name) const
			{
				NST_ASSERT( name );
				return Item( control, ComboBox_AddString( control, name ) );
			}

			void ComboBox::Reserve(uint items,uint lengths) const
			{
				control.Send( CB_INITSTORAGE, items, sizeof(wchar_t) * lengths );
			}

			uint ComboBox::Size() const
			{
				const int size = ComboBox_GetCount( control );
				return NST_MAX(size,0);
			}

			void ComboBox::Clear() const
			{
				ComboBox_ResetContent( control );
			}

			ComboBox::Item ComboBox::Selection() const
			{
				return Item( control, ComboBox_GetCurSel( control ) );
			}
		}
	}
}
