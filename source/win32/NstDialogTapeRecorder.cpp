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
#include "NstManagerPaths.hpp"
#include "NstDialogTapeRecorder.hpp"
#include "NstApplicationInstance.hpp"

namespace Nestopia
{
	namespace Window
	{
		struct TapeRecorder::Handlers
		{
			static const MsgHandler::Entry<TapeRecorder> messages[];
			static const MsgHandler::Entry<TapeRecorder> commands[];
		};

		const MsgHandler::Entry<TapeRecorder> TapeRecorder::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &TapeRecorder::OnInitDialog }
		};

		const MsgHandler::Entry<TapeRecorder> TapeRecorder::Handlers::commands[] =
		{
			{ IDC_TAPE_RECORDER_USE_IMAGENAME, &TapeRecorder::OnCmdUseImageName },
			{ IDC_TAPE_RECORDER_CLEAR,         &TapeRecorder::OnCmdClear        },
			{ IDC_TAPE_RECORDER_BROWSE,        &TapeRecorder::OnCmdBrowse       },
			{ IDOK,                            &TapeRecorder::OnCmdOk           }
		};

		TapeRecorder::TapeRecorder(const Configuration& cfg,const Managers::Paths& p)
		: dialog(IDD_TAPE_RECORDER,this,Handlers::messages,Handlers::commands), paths(p)
		{
			Configuration::ConstSection tapes( cfg["paths"]["tapes"] );

			settings.useImageNaming = !tapes["use-image-name"].No();
			settings.customFile = tapes["file"].Str();
			paths.FixFile( Managers::Paths::File::TAPE, settings.customFile );
		}

		TapeRecorder::~TapeRecorder()
		{
		}

		void TapeRecorder::Save(Configuration& cfg) const
		{
			Configuration::Section tapes( cfg["paths"]["tapes"] );

			tapes["use-image-name"].YesNo() = settings.useImageNaming;
			tapes["file"].Str() = settings.customFile;
		}

		const Path TapeRecorder::GetCustomFile() const
		{
			return Application::Instance::GetFullPath( settings.customFile );
		}

		ibool TapeRecorder::OnInitDialog(Param&)
		{
			dialog.CheckBox(IDC_TAPE_RECORDER_USE_IMAGENAME).Check( settings.useImageNaming );
			dialog.Edit(IDC_TAPE_RECORDER_FILE) << settings.customFile.Ptr();

			Update();

			return true;
		}

		void TapeRecorder::Update() const
		{
			const bool unchecked = !dialog.CheckBox(IDC_TAPE_RECORDER_USE_IMAGENAME).Checked();

			dialog.Control( IDC_TAPE_RECORDER_FILE   ).Enable( unchecked );
			dialog.Control( IDC_TAPE_RECORDER_BROWSE ).Enable( unchecked );
			dialog.Control( IDC_TAPE_RECORDER_CLEAR  ).Enable( unchecked );
		}

		ibool TapeRecorder::OnCmdUseImageName(Param& param)
		{
			if (param.Button().Clicked())
				Update();

			return true;
		}

		ibool TapeRecorder::OnCmdClear(Param& param)
		{
			if (param.Button().Clicked())
				dialog.Edit(IDC_TAPE_RECORDER_FILE).Clear();

			return true;
		}

		ibool TapeRecorder::OnCmdBrowse(Param& param)
		{
			if (param.Button().Clicked())
			{
				Path tmp;
				dialog.Edit(IDC_TAPE_RECORDER_FILE) >> tmp;
				dialog.Edit(IDC_TAPE_RECORDER_FILE).Try() << paths.BrowseSave( Managers::Paths::File::TAPE, Managers::Paths::SUGGEST, Application::Instance::GetFullPath(tmp) ).Ptr();
			}

			return true;
		}

		ibool TapeRecorder::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				settings.useImageNaming = dialog.CheckBox(IDC_TAPE_RECORDER_USE_IMAGENAME).Checked();
				dialog.Edit(IDC_TAPE_RECORDER_FILE) >> settings.customFile;
				paths.FixFile( Managers::Paths::File::TAPE, settings.customFile );
				dialog.Close();
			}

			return true;
		}
	}
}
