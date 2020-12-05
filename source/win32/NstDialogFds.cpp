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
#include "NstWindowUser.hpp"
#include "NstIoStream.hpp"
#include "NstWindowParam.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogFds.hpp"
#include "../core/api/NstApiFds.hpp"

namespace Nestopia
{
	namespace Window
	{
		struct Fds::Handlers
		{
			static const MsgHandler::Entry<Fds> messages[];
			static const MsgHandler::Entry<Fds> commands[];
		};

		const MsgHandler::Entry<Fds> Fds::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &Fds::OnInitDialog }
		};

		const MsgHandler::Entry<Fds> Fds::Handlers::commands[] =
		{
			{ IDC_FDS_CLEAR,   &Fds::OnCmdClear   },
			{ IDC_FDS_BROWSE,  &Fds::OnCmdBrowse  },
			{ IDC_FDS_DEFAULT, &Fds::OnCmdDefault },
			{ IDOK,            &Fds::OnCmdOk      }
		};

		enum
		{
			BIOS_FILE_TYPES =
			(
				Managers::Paths::File::ROM |
				Managers::Paths::File::INES |
				Managers::Paths::File::ARCHIVE
			)
		};

		Fds::Fds(Managers::Emulator& e,const Configuration& cfg,const Managers::Paths& p)
		: dialog(IDD_FDS,this,Handlers::messages,Handlers::commands), emulator(e), paths(p)
		{
			Configuration::ConstSection fds( cfg["paths"]["fds"] );

			const GenericString method( fds["save-method"].Str() );

			emulator.SetDiskImageSaveMethod
			(
				( method == L"disabled" ) ? Managers::Emulator::DISKIMAGE_SAVE_DISABLED :
				( method == L"image"    ) ? Managers::Emulator::DISKIMAGE_SAVE_TO_IMAGE :
											Managers::Emulator::DISKIMAGE_SAVE_TO_PATCH
			);

			const GenericString led( fds["led"].Str() );

			settings.led =
			(
				( led == L"numlock"    ) ? LED_NUM_LOCK :
				( led == L"capslock"   ) ? LED_CAPS_LOCK :
				( led == L"scrolllock" ) ? LED_SCROLL_LOCK :
				( led == L"disabled"   ) ? LED_DISABLED :
                                           LED_SCREEN
			);

			settings.bios = fds["bios"].Str();

			if (settings.bios.Length())
			{
				settings.bios.MakePretty();
			}
			else
			{
				settings.bios = "disksys.rom";

				if (!paths.FindFile( settings.bios ))
					settings.bios.Clear();
			}

			if (settings.bios.Length())
				SubmitBios();
		}

		Fds::~Fds()
		{
		}

		void Fds::Save(Configuration& cfg) const
		{
			Configuration::Section fds( cfg["paths"]["fds"] );

			fds["bios"].Str() = settings.bios;

			fds["save-method"].Str() =
			(
				( emulator.GetDiskImageSaveMethod() == Managers::Emulator::DISKIMAGE_SAVE_TO_IMAGE ) ? "image" :
				( emulator.GetDiskImageSaveMethod() == Managers::Emulator::DISKIMAGE_SAVE_TO_PATCH ) ? "patch" :
                                                                                                       "disabled"
			);

			fds["led"].Str() =
			(
				( settings.led == LED_SCREEN      ) ? "screen" :
				( settings.led == LED_NUM_LOCK    ) ? "numlock" :
				( settings.led == LED_CAPS_LOCK   ) ? "capslock" :
				( settings.led == LED_SCROLL_LOCK ) ? "scrolllock" :
                                                      "disabled"
			);
		}

		void Fds::QueryBiosFile()
		{
			if (settings.bios.Empty())
			{
				settings.bios = paths.BrowseLoad( BIOS_FILE_TYPES );

				if (settings.bios.Length())
				{
					settings.bios.MakePretty();
					SubmitBios();
				}
			}
		}

		void Fds::SubmitBios()
		{
			NST_ASSERT( settings.bios.Length() );

			Managers::Paths::File file;

			if (paths.Load( file, BIOS_FILE_TYPES, Path(Application::Instance::GetFullPath(settings.bios.Directory()),settings.bios.File()), Managers::Paths::QUIETLY ))
			{
				Io::Stream::In stream( file.data );

				if (NES_SUCCEEDED(Nes::Fds(emulator).SetBIOS( &static_cast<std::istream&>(stream) )))
					return;
			}

			settings.bios.Clear();
			User::Warn( IDS_FDS_ERR_INVALIDBIOS );
		}

		ibool Fds::OnInitDialog(Param&)
		{
			NST_COMPILE_ASSERT
			(
				Managers::Emulator::DISKIMAGE_SAVE_DISABLED == IDC_FDS_SAVEDISABLE - IDC_FDS_SAVEDISABLE &&
				Managers::Emulator::DISKIMAGE_SAVE_TO_IMAGE == IDC_FDS_SAVETOIMAGE - IDC_FDS_SAVEDISABLE &&
				Managers::Emulator::DISKIMAGE_SAVE_TO_PATCH == IDC_FDS_SAVETOPATCH - IDC_FDS_SAVEDISABLE
			);

			if (emulator.IsFdsOn())
			{
				dialog.Control( IDC_FDS_BIOS   ).Disable();
				dialog.Control( IDC_FDS_CLEAR  ).Disable();
				dialog.Control( IDC_FDS_BROWSE ).Disable();
			}

			dialog.Edit( IDC_FDS_BIOS ) << settings.bios.Ptr();
			dialog.RadioButton( IDC_FDS_SAVEDISABLE + emulator.GetDiskImageSaveMethod() ).Check();

			const uint id =
			(
				( settings.led == LED_SCREEN      ) ? IDC_FDS_LED_SCREEN :
				( settings.led == LED_NUM_LOCK    ) ? IDC_FDS_LED_NUMLOCK :
				( settings.led == LED_CAPS_LOCK   ) ? IDC_FDS_LED_CAPSLOCK :
				( settings.led == LED_SCROLL_LOCK ) ? IDC_FDS_LED_SCROLLLOCK :
                                                      IDC_FDS_LED_DISABLE
			);

			dialog.RadioButton( id ).Check();

			return true;
		}

		ibool Fds::OnCmdDefault(Param& param)
		{
			if (param.Button().Clicked())
			{
				dialog.RadioButton( IDC_FDS_LED_SCREEN     ).Check();
				dialog.RadioButton( IDC_FDS_LED_NUMLOCK    ).Uncheck();
				dialog.RadioButton( IDC_FDS_LED_CAPSLOCK   ).Uncheck();
				dialog.RadioButton( IDC_FDS_LED_SCROLLLOCK ).Uncheck();
				dialog.RadioButton( IDC_FDS_LED_DISABLE    ).Uncheck();

				dialog.RadioButton( IDC_FDS_SAVETOPATCH ).Check();
				dialog.RadioButton( IDC_FDS_SAVETOIMAGE ).Uncheck();
				dialog.RadioButton( IDC_FDS_SAVEDISABLE ).Uncheck();
			}

			return true;
		}

		ibool Fds::OnCmdClear(Param& param)
		{
			if (param.Button().Clicked())
				dialog.Edit(IDC_FDS_BIOS).Clear();

			return true;
		}

		ibool Fds::OnCmdBrowse(Param& param)
		{
			if (param.Button().Clicked())
				dialog.Edit(IDC_FDS_BIOS).Try() << paths.BrowseLoad( BIOS_FILE_TYPES ).Ptr();

			return true;
		}

		ibool Fds::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				emulator.SetDiskImageSaveMethod
				(
					dialog.RadioButton( IDC_FDS_SAVEDISABLE ).Checked() ? Managers::Emulator::DISKIMAGE_SAVE_DISABLED :
					dialog.RadioButton( IDC_FDS_SAVETOPATCH ).Checked() ? Managers::Emulator::DISKIMAGE_SAVE_TO_PATCH :
                                                                          Managers::Emulator::DISKIMAGE_SAVE_TO_IMAGE
				);

				settings.led =
				(
					dialog.RadioButton( IDC_FDS_LED_SCREEN     ).Checked() ? LED_SCREEN :
					dialog.RadioButton( IDC_FDS_LED_NUMLOCK    ).Checked() ? LED_NUM_LOCK :
					dialog.RadioButton( IDC_FDS_LED_CAPSLOCK   ).Checked() ? LED_CAPS_LOCK :
					dialog.RadioButton( IDC_FDS_LED_SCROLLLOCK ).Checked() ? LED_SCROLL_LOCK :
                                                                             LED_DISABLED
				);

				Path path;
				dialog.Edit( IDC_FDS_BIOS ) >> path;
				path.MakePretty();

				if (settings.bios != path)
				{
					settings.bios = path;

					if (settings.bios.Empty())
						Nes::Fds(emulator).SetBIOS( NULL );
					else
						SubmitBios();
				}

				dialog.Close();
			}

			return true;
		}
	}
}
