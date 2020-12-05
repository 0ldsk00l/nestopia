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

#include "NstWindowParam.hpp"
#include "NstManagerEmulator.hpp"
#include "NstDialogDipSwitches.hpp"

namespace Nestopia
{
	namespace Window
	{
		NST_COMPILE_ASSERT
		(
			IDC_DIPSWITCHES_2 == IDC_DIPSWITCHES_1 + 1 &&
			IDC_DIPSWITCHES_3 == IDC_DIPSWITCHES_1 + 2 &&
			IDC_DIPSWITCHES_4 == IDC_DIPSWITCHES_1 + 3 &&
			IDC_DIPSWITCHES_5 == IDC_DIPSWITCHES_1 + 4 &&
			IDC_DIPSWITCHES_6 == IDC_DIPSWITCHES_1 + 5 &&
			IDC_DIPSWITCHES_7 == IDC_DIPSWITCHES_1 + 6 &&
			IDC_DIPSWITCHES_8 == IDC_DIPSWITCHES_1 + 7 &&

			IDC_DIPSWITCHES_2_TEXT == IDC_DIPSWITCHES_1_TEXT + 1 &&
			IDC_DIPSWITCHES_3_TEXT == IDC_DIPSWITCHES_1_TEXT + 2 &&
			IDC_DIPSWITCHES_4_TEXT == IDC_DIPSWITCHES_1_TEXT + 3 &&
			IDC_DIPSWITCHES_5_TEXT == IDC_DIPSWITCHES_1_TEXT + 4 &&
			IDC_DIPSWITCHES_6_TEXT == IDC_DIPSWITCHES_1_TEXT + 5 &&
			IDC_DIPSWITCHES_7_TEXT == IDC_DIPSWITCHES_1_TEXT + 6 &&
			IDC_DIPSWITCHES_8_TEXT == IDC_DIPSWITCHES_1_TEXT + 7
		);

		struct DipSwitches::Handlers
		{
			static const MsgHandler::Entry<DipSwitches> messages[];
			static const MsgHandler::Entry<DipSwitches> commands[];
		};

		const MsgHandler::Entry<DipSwitches> DipSwitches::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &DipSwitches::OnInitDialog }
		};

		const MsgHandler::Entry<DipSwitches> DipSwitches::Handlers::commands[] =
		{
			{ IDOK, &DipSwitches::OnCmdOk }
		};

		DipSwitches::DipSwitches(Managers::Emulator& emulator,bool u)
		:
		dialog      (IDD_DIPSWITCHES,this,Handlers::messages,Handlers::commands),
		dipSwitches (emulator),
		userOpened  (u)
		{
		}

		ibool DipSwitches::OnInitDialog(Param&)
		{
			NST_ASSERT( dipSwitches.NumDips() <= MAX_DIPS );

			Point delta;

			for (uint i=0, canModify=dipSwitches.CanModify(); i < MAX_DIPS; ++i)
			{
				Control::ComboBox valueField( dialog.ComboBox(IDC_DIPSWITCHES_1 + i) );
				Control::Generic textField( dialog.Control(IDC_DIPSWITCHES_1_TEXT + i) );

				if (i < dipSwitches.NumDips())
				{
					textField.Text() << (HeapString() << dipSwitches.GetDipName(i) << ':').Ptr();

					for (uint j=0, n=dipSwitches.NumValues(i); j < n; ++j)
						valueField.Add( HeapString(dipSwitches.GetValueName(i,j)).Ptr() );

					valueField[dipSwitches.GetValue(i)].Select();

					if (!canModify)
					{
						valueField.Disable();
						textField.Disable();
					}
				}
				else
				{
					if (i == dipSwitches.NumDips())
						delta.y = valueField.GetWindow().Coordinates().top;

					if (i == MAX_DIPS-1)
						delta.y = valueField.GetWindow().Coordinates().bottom - delta.y;

					textField.GetWindow().Destroy();
					valueField.GetWindow().Destroy();
				}
			}

			for (uint i=IDC_DIPSWITCHES_1_TEXT, n=IDC_DIPSWITCHES_1_TEXT+dipSwitches.NumDips(); i < n; ++i)
			{
				Control::Generic textField( dialog.Control(i) );

				Rect rect( textField.GetWindow().Coordinates() );
				rect.right = rect.left + textField.GetMaxTextSize().x;

				if (delta.x < rect.right)
					delta.x = rect.right;

				textField.GetWindow().Size() = rect.Size();
			}

			delta.x = dialog.ComboBox(IDC_DIPSWITCHES_1).GetWindow().Coordinates().left - delta.x + 12;

			for (uint i=IDC_DIPSWITCHES_1, n=IDC_DIPSWITCHES_1+dipSwitches.NumDips(); i < n; ++i)
				dialog.Control(i).GetWindow().Position() -= Point(delta.x,0);

			dialog.Control(IDC_DIPSWITCHES_GROUP).GetWindow().Size() -= delta;

			if (userOpened)
				dialog.Control(IDC_DIPSWITCHES_DONTSHOWAGAIN).GetWindow().Destroy();
			else
				dialog.Control(IDC_DIPSWITCHES_DONTSHOWAGAIN).GetWindow().Position() -= Point(0,delta.y);

			dialog.Control(IDOK).GetWindow().Position() -= delta;
			dialog.Control(IDCANCEL).GetWindow().Position() -= delta;

			dialog.Size() -= delta;

			return true;
		}

		ibool DipSwitches::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				if (dipSwitches.CanModify())
				{
					for (uint i=0, n=dipSwitches.NumDips(); i < n; ++i)
						dipSwitches.SetValue( i, dialog.ComboBox( IDC_DIPSWITCHES_1 + i ).Selection().GetIndex() );
				}

				dialog.Close( userOpened ? 0 : dialog.CheckBox(IDC_DIPSWITCHES_DONTSHOWAGAIN).Checked() );
			}

			return true;
		}
	}
}
