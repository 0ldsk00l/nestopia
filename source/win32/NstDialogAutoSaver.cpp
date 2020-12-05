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

#include "NstApplicationInstance.hpp"
#include "NstWindowParam.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogAutoSaver.hpp"

namespace Nestopia
{
	namespace Window
	{
		AutoSaver::Settings::Settings()
		: interval(1), notify(true) {}

		struct AutoSaver::Handlers
		{
			static const MsgHandler::Entry<AutoSaver> messages[];
			static const MsgHandler::Entry<AutoSaver> commands[];
		};

		const MsgHandler::Entry<AutoSaver> AutoSaver::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &AutoSaver::OnInitDialog }
		};

		const MsgHandler::Entry<AutoSaver> AutoSaver::Handlers::commands[] =
		{
			{ IDC_AUTOSAVE_CLEAR,  &AutoSaver::OnCmdClear  },
			{ IDC_AUTOSAVE_BROWSE, &AutoSaver::OnCmdBrowse },
			{ IDOK,                &AutoSaver::OnCmdOk     }
		};

		AutoSaver::AutoSaver(const Managers::Paths& p)
		: dialog(IDD_AUTOSAVER,this,Handlers::messages,Handlers::commands), paths(p) {}

		AutoSaver::~AutoSaver()
		{
		}

		const Path AutoSaver::GetStateFile() const
		{
			return Application::Instance::GetFullPath( settings.stateFile );
		}

		ibool AutoSaver::OnInitDialog(Param&)
		{
			dialog.Edit(IDC_AUTOSAVE_FILE) << settings.stateFile.Ptr();
			dialog.Edit(IDC_AUTOSAVE_TIME).Limit(2);
			dialog.Edit(IDC_AUTOSAVE_TIME) << settings.interval;
			dialog.CheckBox(IDC_AUTOSAVE_ENABLE_MSG).Check( settings.notify );

			return true;
		}

		ibool AutoSaver::OnCmdClear(Param& param)
		{
			if (param.Button().Clicked())
				dialog.Edit(IDC_AUTOSAVE_FILE).Clear();

			return true;
		}

		ibool AutoSaver::OnCmdBrowse(Param& param)
		{
			if (param.Button().Clicked())
			{
				Path tmp;
				dialog.Edit(IDC_AUTOSAVE_FILE) >> tmp;
				dialog.Edit(IDC_AUTOSAVE_FILE).Try() << paths.BrowseSave( Managers::Paths::File::STATE, Managers::Paths::SUGGEST, Application::Instance::GetFullPath(tmp) ).Ptr();
			}

			return true;
		}

		ibool AutoSaver::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				dialog.Edit(IDC_AUTOSAVE_FILE) >> settings.stateFile;
				dialog.Edit(IDC_AUTOSAVE_TIME) >> settings.interval;
				settings.notify = dialog.CheckBox(IDC_AUTOSAVE_ENABLE_MSG).Checked();
				paths.FixFile( Managers::Paths::File::STATE, settings.stateFile );
				dialog.Close();
			}

			return true;
		}
	}
}
