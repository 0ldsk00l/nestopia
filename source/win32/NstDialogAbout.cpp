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

#include "resource/resource.h"
#include "NstResourceCursor.hpp"
#include "NstWindowParam.hpp"
#include "NstDialogAbout.hpp"
#include "NstApplicationInstance.hpp"
#include <ShellAPI.h>

namespace Nestopia
{
	namespace Window
	{
		struct About::Handlers
		{
			static const MsgHandler::Entry<About> messages[];
			static const MsgHandler::Entry<About> commands[];
		};

		const MsgHandler::Entry<About> About::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &About::OnInitDialog },
			{ WM_SETCURSOR,  &About::OnSetCursor  }
		};

		const MsgHandler::Entry<About> About::Handlers::commands[] =
		{
			{ IDC_ABOUT_URL,  &About::OnCmdClick },
			{ IDC_ABOUT_MAIL, &About::OnCmdClick }
		};

		About::About()
		: dialog(IDD_ABOUT,this,Handlers::messages,Handlers::commands) {}

		ibool About::OnInitDialog(Param&)
		{
			dialog.SetItemIcon( IDC_ABOUT_ICON, Application::Instance::GetIconStyle() == Application::Instance::ICONSTYLE_NES ? IDI_APP : IDI_APP_J );
			//dialog.Control( IDC_ABOUT_NAMEVERSION ).Text() << (String::Heap<char>() << "Nestopia UE v" << Application::Instance::GetVersion()).Ptr();
			dialog.Control( IDC_ABOUT_NAMEVERSION ).Text() << "Nestopia UE vx.xx";
			return true;
		}

		ibool About::OnSetCursor(Param& param)
		{
			HCURSOR hCursor;

			if (param.Cursor().Inside( IDC_ABOUT_URL ) || param.Cursor().Inside( IDC_ABOUT_MAIL ))
				hCursor = Resource::Cursor::GetHand();
			else
				hCursor = Resource::Cursor::GetArrow();

			::SetCursor( hCursor );
			::SetWindowLongPtr( dialog, DWLP_MSGRESULT, true );

			return true;
		}

		ibool About::OnCmdClick(Param& param)
		{
			if (param.Button().Clicked())
			{
				HeapString cmd;

				if (dialog.Control( param.Button().GetId() ).Text() >> cmd)
				{
					cmd.Insert( 0, param.Button().GetId() == IDC_ABOUT_MAIL ? L"mailto:" : L"http://" );
					::ShellExecute( NULL, L"open", cmd.Ptr(), NULL, NULL, SW_SHOWNORMAL );
				}
			}

			return true;
		}
	}
}
