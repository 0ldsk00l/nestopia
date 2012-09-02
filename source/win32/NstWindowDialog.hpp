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

#ifndef NST_WINDOW_DIALOG_H
#define NST_WINDOW_DIALOG_H

#pragma once

#include "language/resource.h"
#include "NstWindowCustom.hpp"
#include "NstCtrlCheckBox.hpp"
#include "NstCtrlRadioButton.hpp"
#include "NstCtrlListBox.hpp"
#include "NstCtrlListView.hpp"
#include "NstCtrlTreeView.hpp"
#include "NstCtrlComboBox.hpp"
#include "NstCtrlSlider.hpp"
#include "NstCtrlEdit.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Dialog : public Custom
		{
		public:

			explicit Dialog(uint);

			template<typename Owner,typename MsgArray>
			Dialog(uint,Owner*,MsgArray&);

			template<typename Owner,typename MsgArray,typename CmdArray>
			Dialog(uint,Owner*,MsgArray&,CmdArray&);

			template<typename Owner,typename Msg>
			Dialog(uint,uint,Owner*,Msg);

			~Dialog();

			enum Type
			{
				MODAL,
				MODELESS_CHILD,
				MODELESS_FREE
			};

			INT_PTR Open(Type=MODAL);
			void Close(int=0);
			void SetItemIcon(uint,uint) const;

		private:

			class NoTaskbarWindow;

			typedef Collection::Vector<Dialog*> Instances;

			void Init();

			ibool OnCommand (Param&);
			ibool OnClose   (Param&);

			void OnIdle       (Param&);
			void OnSysCommand (Param&);
			void OnNclButton  (Param&);
			void OnNcrButton  (Param&);

			NST_NO_INLINE void Fetch(HWND);
			NST_NO_INLINE void Ditch(Instances::Iterator);

			static BOOL CALLBACK DlgProc(HWND,uint,WPARAM,LPARAM);

			MsgHandler cmdHandler;
			const uint id;
			const NoTaskbarWindow* noTaskbarWindow;

			class ModelessDialogs
			{
			public:

				static void Add(HWND);
				static bool Remove(HWND);

			private:

				typedef bool (*Processor)(MSG&);
				typedef Collection::Vector<HWND> Instances;

				static void Update();
				static bool ProcessNone (MSG&);
				static bool ProcessSingle (MSG&);
				static bool ProcessMultiple (MSG&);

				static Processor processor;
				static Instances instances;

			public:

				static bool ProcessMessage(MSG& msg)
				{
					return processor( msg );
				}
			};

			static Instances instances;

		public:

			Control::Generic     Control     (uint id) const { return Control::Generic     ( hWnd, id ); }
			Control::CheckBox    CheckBox    (uint id) const { return Control::CheckBox    ( hWnd, id ); }
			Control::ComboBox    ComboBox    (uint id) const { return Control::ComboBox    ( hWnd, id ); }
			Control::RadioButton RadioButton (uint id) const { return Control::RadioButton ( hWnd, id ); }
			Control::Edit        Edit        (uint id) const { return Control::Edit        ( hWnd, id ); }
			Control::ListBox     ListBox     (uint id) const { return Control::ListBox     ( hWnd, id ); }
			Control::ListView    ListView    (uint id) const { return Control::ListView    ( hWnd, id ); }
			Control::TreeView    TreeView    (uint id) const { return Control::TreeView    ( hWnd, id ); }
			Control::Slider      Slider      (uint id) const { return Control::Slider      ( hWnd, id ); }

			uint GetId() const
			{
				return id;
			}

			bool IsOpen() const
			{
				return hWnd;
			}

			MsgHandler& Commands()
			{
				return cmdHandler;
			}

			static bool ProcessMessage(MSG& msg)
			{
				return ModelessDialogs::ProcessMessage( msg );
			}

			static void RegisterModeless(HWND hWnd)
			{
				NST_ASSERT( hWnd );
				ModelessDialogs::Add( hWnd );
			}

			static bool UnregisterModeless(HWND hWnd)
			{
				NST_ASSERT( hWnd );
				return ModelessDialogs::Remove( hWnd );
			}
		};
	}
}

#include "NstWindowDialog.inl"

#endif
