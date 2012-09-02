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
#include "NstApplicationInstance.hpp"
#include "NstDialogMovie.hpp"

namespace Nestopia
{
	namespace Window
	{
		struct Movie::Handlers
		{
			static const MsgHandler::Entry<Movie> messages[];
			static const MsgHandler::Entry<Movie> commands[];
		};

		const MsgHandler::Entry<Movie> Movie::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &Movie::OnInitDialog }
		};

		const MsgHandler::Entry<Movie> Movie::Handlers::commands[] =
		{
			{ IDC_MOVIE_CLEAR,  &Movie::OnCmdClear  },
			{ IDC_MOVIE_BROWSE, &Movie::OnCmdBrowse },
			{ IDOK,             &Movie::OnCmdOk     }
		};

		Movie::Movie(const Managers::Paths& p)
		: dialog(IDD_MOVIE,this,Handlers::messages,Handlers::commands), paths(p) {}

		Movie::~Movie()
		{
		}

		const Path Movie::GetMovieFile() const
		{
			return Application::Instance::GetFullPath( movieFile );
		}

		void Movie::SetMovieFile(const Path& file)
		{
			movieFile = file;
			paths.FixFile( Managers::Paths::File::MOVIE, movieFile );
		}

		ibool Movie::OnInitDialog(Param&)
		{
			dialog.Edit(IDC_MOVIE_FILE) << movieFile.Ptr();

			return true;
		}

		ibool Movie::OnCmdClear(Param& param)
		{
			if (param.Button().Clicked())
				dialog.Edit(IDC_MOVIE_FILE).Clear();

			return true;
		}

		ibool Movie::OnCmdBrowse(Param& param)
		{
			if (param.Button().Clicked())
			{
				Path tmp;
				dialog.Edit(IDC_MOVIE_FILE).Text() >> tmp;
				dialog.Edit(IDC_MOVIE_FILE).Try() << paths.BrowseSave( Managers::Paths::File::MOVIE, Managers::Paths::SUGGEST, Application::Instance::GetFullPath(tmp) ).Ptr();
			}

			return true;
		}

		ibool Movie::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				dialog.Edit(IDC_MOVIE_FILE) >> movieFile;
				paths.FixFile( Managers::Paths::File::MOVIE, movieFile );
				dialog.Close();
			}

			return true;
		}
	}
}
