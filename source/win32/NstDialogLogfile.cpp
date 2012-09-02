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
#include "NstDialogLogfile.hpp"

namespace Nestopia
{
	namespace Window
	{
		struct Logfile::Handlers
		{
			static const MsgHandler::Entry<Logfile> messages[];
			static const MsgHandler::Entry<Logfile> commands[];
		};

		const MsgHandler::Entry<Logfile> Logfile::Handlers::messages[] =
		{
			{ WM_INITDIALOG,     &Logfile::OnInitDialog     },
			{ WM_CTLCOLORSTATIC, &Logfile::OnCtlColorStatic }
		};

		const MsgHandler::Entry<Logfile> Logfile::Handlers::commands[] =
		{
			{ IDC_LOGFILE_CLEAR, &Logfile::OnCmdClear }
		};

		Logfile::Logfile()
		: dialog(IDD_LOGFILE,this,Handlers::messages,Handlers::commands) {}

		bool Logfile::Open(wcstring const string)
		{
			clear = false;

			if (*string)
			{
				text = string;
				dialog.Open();
			}

			return clear;
		}

		ibool Logfile::OnInitDialog(Param&)
		{
			dialog.Edit( IDC_LOGFILE_EDIT ) << text;
			return true;
		}

		ibool Logfile::OnCtlColorStatic(Param& param)
		{
			NST_COMPILE_ASSERT( sizeof(ibool) == sizeof(BOOL) );

			if (reinterpret_cast<HWND>(param.lParam) == ::GetDlgItem( param.hWnd, IDC_LOGFILE_EDIT ))
				return reinterpret_cast<ibool>(::GetSysColorBrush( COLOR_WINDOW ));
			else
				return false;
		}

		ibool Logfile::OnCmdClear(Param& param)
		{
			if (param.Button().Clicked())
			{
				dialog.Edit( IDC_LOGFILE_EDIT ).Clear();
				clear = true;
			}

			return true;
		}
	}
}
