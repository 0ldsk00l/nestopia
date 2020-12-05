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
#include "NstDialogLanguage.hpp"

namespace Nestopia
{
	namespace Window
	{
		struct Language::Handlers
		{
			static const MsgHandler::Entry<Language> messages[];
			static const MsgHandler::Entry<Language> commands[];
		};

		const MsgHandler::Entry<Language> Language::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &Language::OnInitDialog }
		};

		const MsgHandler::Entry<Language> Language::Handlers::commands[] =
		{
			{ IDOK, &Language::OnCmdOk },
			{ IDC_LANGUAGE_LIST, &Language::OnDblClk }
		};

		Language::Language()
		: dialog(IDD_LANGUAGE,this,Handlers::messages,Handlers::commands) {}

		Language::~Language()
		{
		}

		ibool Language::OnInitDialog(Param&)
		{
			paths.clear();
			Application::Instance::GetLanguage().EnumerateResources( paths );

			const Control::ListBox listBox( dialog.ListBox(IDC_LANGUAGE_LIST) );

			Path name;

			for (Paths::const_iterator it(paths.begin()), end(paths.end()); it != end; ++it)
			{
				name = it->File();
				name.Extension().Clear();
				::CharUpperBuff( name.Ptr(), 1 );

				const uint index = listBox.Add( name.Ptr() ).GetIndex();
				listBox[index].Data() = (it - paths.begin());

				if (*it == Application::Instance::GetLanguage().GetResourcePath())
					listBox[index].Select();
			}

			return true;
		}

		void Language::CloseOk()
		{
			const Control::ListBox listBox( dialog.ListBox(IDC_LANGUAGE_LIST) );

			if (listBox.AnySelection())
			{
				const uint index = listBox.Selection().Data();

				if (paths[index] != Application::Instance::GetLanguage().GetResourcePath())
					newPath = paths[index];
			}

			dialog.Close();
		}

		ibool Language::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
				CloseOk();

			return true;
		}

		ibool Language::OnDblClk(Param& param)
		{
			if (HIWORD(param.wParam) == LBN_DBLCLK)
			{
				CloseOk();
				return true;
			}

			return false;
		}
	}
}
