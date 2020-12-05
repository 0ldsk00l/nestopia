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
#include "NstWindowParam.hpp"
#include "NstWindowUser.hpp"
#include "NstIoStream.hpp"
#include "NstResourceFile.hpp"
#include "NstManagerPaths.hpp"
#include "NstApplicationInstance.hpp"
#include "NstDialogImageDatabase.hpp"
#include "../core/api/NstApiCartridge.hpp"

namespace Nestopia
{
	namespace Window
	{
		struct ImageDatabase::Handlers
		{
			static const MsgHandler::Entry<ImageDatabase> messages[];
			static const MsgHandler::Entry<ImageDatabase> commands[];
		};

		const MsgHandler::Entry<ImageDatabase> ImageDatabase::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &ImageDatabase::OnInitDialog }
		};

		const MsgHandler::Entry<ImageDatabase> ImageDatabase::Handlers::commands[] =
		{
			{ IDC_IMAGEDATABASE_EXTERNAL, &ImageDatabase::OnCmdExternal },
			{ IDC_IMAGEDATABASE_CLEAR,    &ImageDatabase::OnCmdClear    },
			{ IDC_IMAGEDATABASE_BROWSE,   &ImageDatabase::OnCmdBrowse   },
			{ IDC_IMAGEDATABASE_DEFAULT,  &ImageDatabase::OnCmdDefault  },
			{ IDOK,                       &ImageDatabase::OnCmdOk       }
		};

		ImageDatabase::ImageDatabase(Managers::Emulator& e,const Configuration& cfg,const Managers::Paths& p)
		:
		dialog   (IDD_IMAGE_DATABASE,this,Handlers::messages,Handlers::commands),
		emulator (e),
		paths    (p)
		{
			Configuration::ConstSection database( cfg["image-database"] );

			settings.internal = !database["internal"].No();
			settings.external = database["external"].Yes();
			settings.file = database["file"].Str();

			Nes::Cartridge(emulator).GetDatabase().Enable();

			Update(false);
		}

		ImageDatabase::~ImageDatabase()
		{
		}

		void ImageDatabase::Save(Configuration& cfg) const
		{
			Configuration::Section database( cfg["image-database"] );

			database["internal"].YesNo() = settings.internal;
			database["external"].YesNo() = settings.external;
			database["file"].Str() = settings.file;
		}

		void ImageDatabase::Update(const bool reportError)
		{
			try
			{
				Application::Instance::Waiter wait;

				Nes::Cartridge(emulator).GetDatabase().Unload();

				if (settings.internal || settings.external)
				{
					Collection::Buffer internal;

					if (settings.internal)
						Resource::File( IDR_IMAGEDATABASE, L"ImageDatabase" ).Uncompress( internal );

					if (settings.external && settings.file.Empty())
						settings.external = false;

					if (settings.external && internal.Size())
					{
						Io::Stream::In stream0( Application::Instance::GetFullPath(settings.file) );
						Io::Stream::In stream1( internal );

						if (NES_FAILED(Nes::Cartridge(emulator).GetDatabase().Load( stream0, stream1 )))
							throw 1;
					}
					else if (settings.external)
					{
						Io::Stream::In stream( Application::Instance::GetFullPath(settings.file) );

						if (NES_FAILED(Nes::Cartridge(emulator).GetDatabase().Load( stream )))
							throw 1;
					}
					else if (internal.Size())
					{
						Io::Stream::In stream( internal );

						if (NES_FAILED(Nes::Cartridge(emulator).GetDatabase().Load( stream )))
							throw 1;
					}
				}
			}
			catch (...)
			{
				settings.external = false;

				if (reportError)
					Window::User::Fail( IDS_EXT_DATABASE_LOAD_FAILED );
				else
					Window::User::Warn( IDS_EXT_DATABASE_LOAD_FAILED );
			}
		}

		ibool ImageDatabase::OnInitDialog(Param&)
		{
			dialog.CheckBox( IDC_IMAGEDATABASE_INTERNAL ).Check( settings.internal );
			dialog.CheckBox( IDC_IMAGEDATABASE_EXTERNAL ).Check( settings.external );
			dialog.Edit( IDC_IMAGEDATABASE_FILE ) << settings.file.Ptr();

			UpdateAvailibility();

			return true;
		}

		void ImageDatabase::UpdateAvailibility() const
		{
			const bool external = dialog.CheckBox( IDC_IMAGEDATABASE_EXTERNAL ).Checked();

			dialog.Edit( IDC_IMAGEDATABASE_FILE   ).Enable( external );
			dialog.Edit( IDC_IMAGEDATABASE_BROWSE ).Enable( external );
			dialog.Edit( IDC_IMAGEDATABASE_CLEAR  ).Enable( external );
		}

		ibool ImageDatabase::OnCmdExternal(Param& param)
		{
			if (param.Button().Clicked())
				UpdateAvailibility();

			return true;
		}

		ibool ImageDatabase::OnCmdClear(Param& param)
		{
			if (param.Button().Clicked())
				dialog.Edit( IDC_IMAGEDATABASE_FILE ).Clear();

			return true;
		}

		ibool ImageDatabase::OnCmdBrowse(Param& param)
		{
			if (param.Button().Clicked())
			{
				Path tmp;
				dialog.Edit( IDC_IMAGEDATABASE_FILE ).Text() >> tmp;
				dialog.Edit( IDC_IMAGEDATABASE_FILE ).Try() << paths.BrowseLoad( Managers::Paths::File::XML ).Ptr();
			}

			return true;
		}

		ibool ImageDatabase::OnCmdDefault(Param& param)
		{
			if (param.Button().Clicked())
			{
				dialog.CheckBox( IDC_IMAGEDATABASE_INTERNAL ).Check( true );
				dialog.CheckBox( IDC_IMAGEDATABASE_EXTERNAL ).Check( false );

				UpdateAvailibility();
			}

			return true;
		}

		ibool ImageDatabase::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				const bool oldInternal = settings.internal;
				const bool oldExternal = settings.external;

				settings.internal = dialog.CheckBox( IDC_IMAGEDATABASE_INTERNAL ).Checked();
				settings.external = dialog.CheckBox( IDC_IMAGEDATABASE_EXTERNAL ).Checked();

				const Path oldFile(settings.file);

				dialog.Edit(IDC_IMAGEDATABASE_FILE) >> settings.file;

				if ((settings.internal != oldInternal || settings.external != oldExternal) || (settings.external && settings.file != oldFile))
					Update(false);

				dialog.Close();
			}

			return true;
		}
	}
}
