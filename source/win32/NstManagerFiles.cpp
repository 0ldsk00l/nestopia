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

#include "NstIoScreen.hpp"
#include "NstIoLog.hpp"
#include "NstResourceString.hpp"
#include "NstWindowUser.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDropFiles.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerPreferences.hpp"
#include "NstManagerMovie.hpp"
#include "NstManagerTapeRecorder.hpp"
#include "NstManagerSaveStates.hpp"
#include "NstManagerCheats.hpp"
#include "NstManagerFiles.hpp"
#include "NstWindowMain.hpp"
#include "../core/api/NstApiMachine.hpp"
#include "../core/api/NstApiCartridge.hpp"

namespace Nestopia
{
	namespace Managers
	{
		Files::Files
		(
			Emulator& e,
			Window::Menu& m,
			const Paths& p,
			const Preferences& r,
			Movie& o,
			const TapeRecorder& t,
			const Cheats& c,
			const SaveStates& s,
			Window::Main& w
		)
		:
		Manager      ( e, m, this, &Files::OnEmuEvent ),
		paths        ( p ),
		preferences  ( r ),
		movie        ( o ),
		tapeRecorder ( t ),
		cheats       ( c ),
		saveStates   ( s ),
		window       ( w )
		{
			static const Window::MsgHandler::Entry<Files> messages[] =
			{
				{ WM_DROPFILES,                         &Files::OnMsgDropFiles },
				{ WM_COPYDATA,                          &Files::OnMsgCopyData  },
				{ Application::Instance::WM_NST_LAUNCH, &Files::OnMsgLaunch    }
			};

			window.Get().Messages().Add( this, messages );

			static const Window::Menu::CmdHandler::Entry<Files> commands[] =
			{
				{ IDM_FILE_OPEN,  &Files::OnCmdOpen  },
				{ IDM_FILE_CLOSE, &Files::OnCmdClose }
			};

			menu.Commands().Add( this, commands );

			UpdateMenu();
		}

		Files::~Files()
		{
			window.Get().Messages().Remove( this );
		}

		void Files::UpdateMenu() const
		{
			const bool available = (emulator.NetPlayers() == 0 && emulator.IsGame());

			menu[IDM_FILE_CLOSE].Enable( emulator.IsImage() );
			menu[IDM_FILE_SAVE_NST].Enable( available );
		}

		void Files::Open(wcstring const name,uint types) const
		{
			Application::Instance::Events::Signal( Application::Instance::EVENT_SYSTEM_BUSY );

			if (!window.Get().Activate())
				return;

			if (!types)
			{
				types =
				(
					Paths::File::IMAGE |
					Paths::File::STATE |
					Paths::File::MOVIE |
					Paths::File::PATCH |
					Paths::File::BATTERY |
					Paths::File::ARCHIVE
				);
			}

			Path path;

			if (name)
			{
				path = name;

				if (path.File().Empty())
					path = paths.BrowseLoad( types, name );
			}
			else
			{
				path = paths.BrowseLoad( types );
			}

			if (path.Empty())
				return;

			if (path == emulator.GetStartPath())
			{
				AutoStart();
				return;
			}

			Paths::File file;
			Paths::File patch;
			Emulator::Context context;

			switch (paths.Load( file, types, path ))
			{
				case Paths::File::NONE:
					return;

				case Paths::File::STATE:

					if (emulator.IsGame())
					{
						saveStates.Load( file.data, file.name );
						return;
					}

					types = Paths::File::GAME|Paths::File::ARCHIVE;
					context.state = file.name;
					break;

				case Paths::File::MOVIE:

					if (emulator.IsGame())
					{
						if (movie.Load( file.name, Movie::NOISY ) && preferences[Preferences::AUTOSTART_EMULATION])
						{
							AutoStart();
							window.Get().PostCommand( IDM_FILE_MOVIE_PLAY );
						}

						return;
					}

					types = Paths::File::GAME|Paths::File::ARCHIVE;
					context.movie = file.name;
					break;

				case Paths::File::BATTERY:

					if (file.name.FileInArchive().Length())
					{
						Window::User::Fail( IDS_FILE_ERR_CANT_USE_IN_ARCHIVE );
						return;
					}

					if (emulator.IsCart() && Nes::Cartridge(emulator).GetProfile()->board.HasBattery())
					{
						if (Window::User::Confirm( IDS_LOAD_APPLY_CURRENT_GAME ))
							context.image = emulator.GetImagePath();
					}

					types = Paths::File::CARTRIDGE|Paths::File::ARCHIVE;
					context.save = file.name;
					break;

				case Paths::File::IPS:
				case Paths::File::UPS:

					if (emulator.IsCart())
					{
						if (Window::User::Confirm( IDS_LOAD_APPLY_CURRENT_GAME ))
							context.image = emulator.GetImagePath();
					}

					types = Paths::File::CARTRIDGE|Paths::File::ARCHIVE;
					patch.name = file.name;
					patch.data.Import( file.data );
					break;

				default:

					types = Paths::File::NONE;
					context.image = file.name;
					break;
			}

			if (types)
			{
				if (context.image.Empty())
				{
					context.image = file.name;

					if (!paths.LocateFile( context.image, types ))
					{
						if (Window::User::Confirm( IDS_LOAD_SPECIFY_FILE ))
							context.image = paths.BrowseLoad( types );
						else
							context.image.Clear();
					}
				}

				if (context.image.Empty())
					return;

				path = file.name;

				if (!paths.Load( file, types, context.image ))
					return;

				file.name = path;
			}

			NST_ASSERT( file.type & Paths::File::IMAGE );

			if (context.save.Empty())
				context.save = paths.GetSavePath( context.image, file.type );

			if (patch.data.Empty())
			{
				const Path path(paths.GetPatchPath( context.image, file.type ));

				if (path.FileExists() && !paths.Load( patch, Paths::File::PATCH|Paths::File::ARCHIVE, path, Paths::QUIETLY ))
					Window::User::Warn( Resource::String( IDS_EMU_WARN_PATCH_LOAD_ERR ).Invoke( path ).Ptr() );
			}

			if (context.tape.Empty())
				context.tape = tapeRecorder.GetFile( context.save );

			context.samples = paths.GetSamplesPath();

			if (!emulator.Load( file.data, file.name, patch.data, paths.BypassPatchValidation(), context, preferences.GetFavoredSystem(), preferences.GetAlwaysAskProfile(), !preferences[Preferences::SUPPRESS_WARNINGS] ))
				return;

			if (context.mode == Emulator::Context::UNKNOWN && menu[IDM_MACHINE_SYSTEM_AUTO].Checked())
				Nes::Machine(emulator).SetMode( Nes::Machine(emulator).GetDesiredMode() );

			if (context.state.Length())
			{
				if (paths.Load( file, Paths::File::STATE|Paths::File::ARCHIVE, context.state ))
					saveStates.Load( file.data, file.name );
			}

			if (context.movie.Length())
				movie.Load( context.movie, Movie::QUIET );

			AutoStart();
		}

		bool Files::Close() const
		{
			if
			(
				!emulator.IsOn() ||
				!preferences[Preferences::CONFIRM_RESET] ||
				Window::User::Confirm( IDS_ARE_YOU_SURE, IDS_MACHINE_POWER_OFF_TITLE )
			)
			{
				emulator.Unload();
				return true;
			}

			return false;
		}

		void Files::AutoStart() const
		{
			if
			(
				preferences[Preferences::AUTOSTART_EMULATION] &&
				emulator.IsImage() && !emulator.IsOn()
			)
				emulator.Power( true );
		}

		void Files::DisplayLoadMessage(const bool loaded) const
		{
			const uint length = window.GetMaxMessageLength();
			const uint threshold = (loaded ? 10 : 12);

			if (length > threshold)
				Io::Screen() << Resource::String( loaded ? IDS_SCREEN_LOADED : IDS_SCREEN_UNLOADED ).Invoke( Path::Compact( emulator.GetImagePath().Target(), length - (threshold - 1) ) );
		}

		ibool Files::OnMsgDropFiles(Window::Param& param)
		{
			Window::DropFiles dropFiles( param );

			if (dropFiles.Size() && menu[IDM_FILE_OPEN].Enabled())
				Open( dropFiles[0].Ptr() );

			return true;
		}

		ibool Files::OnMsgCopyData(Window::Param& param)
		{
			NST_VERIFY( param.lParam );

			if (menu[IDM_FILE_OPEN].Enabled() && param.lParam)
			{
				const COPYDATASTRUCT& copyData = *reinterpret_cast<COPYDATASTRUCT*>(param.lParam);
				NST_VERIFY( copyData.dwData == Application::Instance::COPYDATA_OPENFILE_ID );

				if (copyData.dwData == Application::Instance::COPYDATA_OPENFILE_ID)
				{
					NST_VERIFY( copyData.lpData && copyData.cbData >= sizeof(wchar_t) && static_cast<wcstring>(copyData.lpData)[copyData.cbData/sizeof(wchar_t)-1] == '\0' );

					if (copyData.lpData && copyData.cbData >= sizeof(wchar_t) && static_cast<wcstring>(copyData.lpData)[copyData.cbData/sizeof(wchar_t)-1] == '\0')
						Open( static_cast<wcstring>(copyData.lpData) );
				}
			}

			param.lResult = true;
			return true;
		}

		ibool Files::OnMsgLaunch(Window::Param& param)
		{
			NST_ASSERT( param.lParam );
			Open( reinterpret_cast<wcstring>(param.lParam), param.wParam );
			return true;
		}

		void Files::OnCmdOpen(uint)
		{
			Open();
		}

		void Files::OnCmdClose(uint)
		{
			Close();
		}

		void Files::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_LOAD:
				case Emulator::EVENT_UNLOAD:

					DisplayLoadMessage( event == Emulator::EVENT_LOAD );
					UpdateMenu();
					break;

				case Emulator::EVENT_NETPLAY_MODE:

					menu[IDM_FILE_OPEN].Enable( !data );
					menu[IDM_FILE_LOAD_NST].Enable( !data );
					break;
			}
		}
	}
}
