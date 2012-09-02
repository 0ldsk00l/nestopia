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

#ifndef NST_WINDOW_PARAM_H
#define NST_WINDOW_PARAM_H

#pragma once

#include <cstdlib>
#include "NstMain.hpp"
#include <windows.h>

namespace Nestopia
{
	namespace Window
	{
		struct Param
		{
			WPARAM wParam;
			LPARAM lParam;
			LRESULT lResult;
			HWND hWnd;

			class ButtonParam
			{
				const uint wParam;

			public:

				explicit ButtonParam(uint w)
				: wParam(w) {}

				bool Clicked() const
				{
					return HIWORD(wParam) == BN_CLICKED;
				}

				uint GetId() const
				{
					return LOWORD(wParam);
				}
			};

			class ListBoxParam
			{
				const uint wParam;

			public:

				explicit ListBoxParam(uint w)
				: wParam(w) {}

				bool SelectionChanged() const
				{
					return HIWORD(wParam) == LBN_SELCHANGE;
				}

				uint GetId() const
				{
					return LOWORD(wParam);
				}
			};

			class ComboBoxParam
			{
				const uint wParam;

			public:

				explicit ComboBoxParam(uint w)
				: wParam(w) {}

				bool SelectionChanged() const
				{
					return HIWORD(wParam) == CBN_SELCHANGE;
				}

				bool DropingDown() const
				{
					return HIWORD(wParam) == CBN_DROPDOWN;
				}

				uint GetId() const
				{
					return LOWORD(wParam);
				}
			};

			class EditParam
			{
				const uint wParam;

			public:

				explicit EditParam(uint w)
				: wParam(w) {}

				bool SetFocus() const
				{
					return HIWORD(wParam) == EN_SETFOCUS;
				}

				bool KillFocus() const
				{
					return HIWORD(wParam) == EN_KILLFOCUS;
				}

				bool Changed() const
				{
					return HIWORD(wParam) == EN_CHANGE;
				}

				uint GetId() const
				{
					return LOWORD(wParam);
				}
			};

			class SliderParam
			{
				const Param& param;

			public:

				explicit SliderParam(const Param& p)
				: param(p) {}

				uint Scroll() const;

				uint GetId() const
				{
					return ::GetDlgCtrlID( reinterpret_cast<HWND>(param.lParam) );
				}

				bool Released() const
				{
					return LOWORD(param.wParam) == SB_ENDSCROLL;
				}
			};

			class CursorParam
			{
				const Param& param;

			public:

				explicit CursorParam(const Param& p)
				: param(p) {}

				bool Inside(uint id) const
				{
					return reinterpret_cast<HWND>(param.wParam) == ::GetDlgItem( param.hWnd, id );
				}
			};

			class ActivatorParam
			{
				const Param& param;

			public:

				explicit ActivatorParam(const Param& p)
				: param(p) {}

				bool Leaving() const
				{
					return LOWORD(param.wParam) == WA_INACTIVE;
				}

				bool Entering() const
				{
					return !Leaving();
				}

				bool InsideApplication() const
				{
					return param.lParam && ::IsChild( param.hWnd, reinterpret_cast<HWND>(param.lParam) );
				}

				bool OutsideApplication() const
				{
					return !InsideApplication();
				}

				bool Minimized() const
				{
					return HIWORD(param.wParam);
				}
			};

			ButtonParam    Button()    const { return ButtonParam    ( wParam ); }
			ListBoxParam   ListBox()   const { return ListBoxParam   ( wParam ); }
			ComboBoxParam  ComboBox()  const { return ComboBoxParam  ( wParam ); }
			EditParam      Edit()      const { return EditParam      ( wParam ); }
			SliderParam    Slider()    const { return SliderParam    ( *this  ); }
			CursorParam    Cursor()    const { return CursorParam    ( *this  ); }
			ActivatorParam Activator() const { return ActivatorParam ( *this  ); }
		};
	}
}

#endif
