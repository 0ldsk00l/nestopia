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

#include "NstIoLog.hpp"
#include "NstIoStream.hpp"
#include "NstIoScreen.hpp"
#include "NstResourceString.hpp"
#include "NstWindowUser.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogMovie.hpp"
#include "NstManagerAviConverter.hpp"
#include "NstManagerMovie.hpp"

namespace Nestopia
{
	namespace Managers
	{
		Movie::Movie(Emulator& e,Window::Menu& m,const Paths& p)
		:
		Manager ( e, m, this, &Movie::OnEmuEvent ),
		file    ( e ),
		dialog  ( new Window::Movie(p) ),
		paths   ( p )
		{
			static const Window::Menu::CmdHandler::Entry<Movie> commands[] =
			{
				{ IDM_FILE_MOVIE_FILE,       &Movie::OnCmdFile      },
				{ IDM_FILE_MOVIE_RECORD,     &Movie::OnCmdRecord    },
				{ IDM_FILE_MOVIE_PLAY,       &Movie::OnCmdPlay      },
				{ IDM_FILE_MOVIE_STOP,       &Movie::OnCmdStop      },
				{ IDM_FILE_MOVIE_REWIND,     &Movie::OnCmdRewind    },
				{ IDM_FILE_MOVIE_FORWARD,    &Movie::OnCmdForward   },
				{ IDM_FILE_MOVIE_EXPORT_AVI, &Movie::OnCmdExportAvi }
			};

			menu.Commands().Add( this, commands );

			static const Window::Menu::PopupHandler::Entry<Movie> popups[] =
			{
				{ Window::Menu::PopupHandler::Pos<IDM_POS_FILE,IDM_POS_FILE_MOVIE>::ID, &Movie::OnMenu }
			};

			menu.Popups().Add( this, popups );
		}

		Movie::~Movie()
		{
		}

		bool Movie::Load(const Path& path,const Alert alert)
		{
			if (path.Archive().Length())
			{
				dialog->ClearMovieFile();

				if (alert == NOISY)
					Window::User::Fail( IDS_FILE_ERR_CANT_USE_IN_ARCHIVE );
				else
					Io::Log() << "Movie: ignoring file, can't use it while it's archived..\r\n";
			}
			else
			{
				dialog->SetMovieFile( path );
			}

			file.Update( dialog->GetMovieFile() );

			return !file.GetPath().Empty();
		}

		void Movie::OnMenu(const Window::Menu::PopupHandler::Param& param)
		{
			param.menu[ IDM_FILE_MOVIE_PLAY       ].Enable( !param.show || file.CanPlay()    );
			param.menu[ IDM_FILE_MOVIE_RECORD     ].Enable( !param.show || file.CanRecord()  );
			param.menu[ IDM_FILE_MOVIE_STOP       ].Enable( !param.show || file.CanStop()    );
			param.menu[ IDM_FILE_MOVIE_REWIND     ].Enable( !param.show || file.CanRewind()  );
			param.menu[ IDM_FILE_MOVIE_FORWARD    ].Enable( !param.show || file.CanForward() );
			param.menu[ IDM_FILE_MOVIE_EXPORT_AVI ].Enable( !param.show || file.CanPlay()    );
		}

		void Movie::OnCmdFile(uint)
		{
			dialog->Open();
			file.Update( dialog->GetMovieFile() );
		}

		void Movie::OnCmdRecord(uint)
		{
			if (file.Start( File::MODE_RECORD ) && !emulator.Power( true ))
				Nes::Movie(emulator).Stop();

			Resume();
		}

		void Movie::OnCmdPlay(uint)
		{
			if (file.Start( File::MODE_PLAY ))
			{
				if (emulator.Power( true ))
					emulator.AskBeforeSaving();
				else
					Nes::Movie(emulator).Stop();
			}

			Resume();
		}

		void Movie::OnCmdExportAvi(uint)
		{
			if (file.CanPlay())
			{
				if (const uint ids = AviConverter( emulator ).Record( file.GetPath(), paths.BrowseSave( Paths::File::AVI, Paths::SUGGEST, file.GetPath() ) ))
				{
					if (ids == IDS_AVI_WRITE_ABORT || ids == IDS_AVI_WRITE_FINISHED)
						Window::User::Inform( ids );
					else
						Window::User::Fail( ids );
				}
			}
		}

		void Movie::OnCmdStop(uint)
		{
			Nes::Movie(emulator).Stop();
			Resume();
		}

		void Movie::OnCmdRewind(uint)
		{
			if (file.Rewind())
				Io::Screen() << Resource::String( IDS_SCREEN_MOVIE_REWOUND );

			Resume();
		}

		void Movie::OnCmdForward(uint)
		{
			if (file.Forward())
				Io::Screen() << Resource::String( IDS_SCREEN_MOVIE_FORWARDED );

			Resume();
		}

		void Movie::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_MOVIE_PLAYING:
				case Emulator::EVENT_MOVIE_PLAYING_STOPPED:
				case Emulator::EVENT_MOVIE_RECORDING:
				case Emulator::EVENT_MOVIE_RECORDING_STOPPED:

					if (event == Emulator::EVENT_MOVIE_PLAYING || event == Emulator::EVENT_MOVIE_RECORDING)
					{
						Io::Screen() << Resource::String
						(
							event == Emulator::EVENT_MOVIE_PLAYING ? IDS_SCREEN_MOVIE_PLAY_STARTED :
                                                                     IDS_SCREEN_MOVIE_REC_STARTED
						);
					}
					else
					{
						const uint op =
						(
							event == Emulator::EVENT_MOVIE_PLAYING_STOPPED ? IDS_SCREEN_MOVIE_PLAY_STOPPED :
                                                                             IDS_SCREEN_MOVIE_REC_STOPPED
						);

						const Nes::Result result = static_cast<Nes::Result>(data);

						if (NES_SUCCEEDED(result))
						{
							file.Stop( event == Emulator::EVENT_MOVIE_PLAYING_STOPPED ? File::POS_BEG : File::POS_END );

							Io::Screen() << Resource::String( op );
						}
						else
						{
							file.Stop( File::POS_BEG );

							const uint msg = Emulator::ResultToString( result );

							Io::Screen() << Resource::String( op )
                                         << ' '
                                         << Resource::String( msg ? msg : IDS_ERR_GENERIC );
						}
					}
					break;

				case Emulator::EVENT_POWER_ON:
				case Emulator::EVENT_POWER_OFF:

					if (emulator.NetPlayers())
						menu[IDM_FILE_MOVIE_FILE].Enable( event == Emulator::EVENT_POWER_OFF );

					break;
			}
		}

		Movie::File::File(Emulator& e)
		: stream(NULL), pos(POS_BEG), emulator(e)
		{
		}

		Movie::File::~File()
		{
			Nes::Movie(emulator).Stop();
			Stop();
		}

		const Path& Movie::File::GetPath() const
		{
			return path;
		}

		void Movie::File::Update(const Path& newPath)
		{
			if (path != newPath)
			{
				Nes::Movie(emulator).Stop();
				path = newPath;
				pos = POS_BEG;
			}
		}

		bool Movie::File::Start(const Mode mode)
		{
			if (!(mode == MODE_PLAY ? CanPlay() : CanRecord()))
				return false;

			uint msg;

			if (mode == MODE_PLAY || !path.FileProtected())
			{
				try
				{
					stream = new Io::Stream::InOut
					(
						path,
						mode == MODE_PLAY ? Io::Stream::InOut::READ :
						pos  == POS_BEG   ? Io::Stream::InOut::READ|Io::Stream::InOut::WRITE|Io::Stream::InOut::TRUNCATE :
											Io::Stream::InOut::READ|Io::Stream::InOut::WRITE
					);

					const Nes::Result result =
					(
						mode == MODE_RECORD ? Nes::Movie(emulator).Record( *stream, pos == POS_BEG ? Nes::Movie::CLEAN : Nes::Movie::APPEND ) :
                                              Nes::Movie(emulator).Play( *stream )
					);

					if (NES_SUCCEEDED(result))
						return true;

					msg = Emulator::ResultToString( result );
				}
				catch (...)
				{
					msg = IDS_FILE_ERR_OPEN;
				}
			}
			else
			{
				msg = IDS_FILE_ERR_WRITE_PROTECTED;
			}

			if (msg)
			{
				Io::Screen() << Resource::String( mode == MODE_RECORD ? IDS_EMU_ERR_MOVIE_REC : IDS_EMU_ERR_MOVIE_PLAY )
                             << ' '
                             << Resource::String( msg );
			}

			Stop();

			return false;
		}

		void Movie::File::Stop(Pos p)
		{
			pos = p;
			delete stream;
			stream = NULL;
		}

		bool Movie::File::Rewind()
		{
			if (CanRewind())
			{
				pos = POS_BEG;
				return true;
			}

			return false;
		}

		bool Movie::File::Forward()
		{
			if (CanForward())
			{
				pos = POS_END;
				return true;
			}

			return false;
		}

		bool Movie::File::CanPlay() const
		{
			return IsReady() && !emulator.NetPlayers() && Nes::Movie(emulator).IsStopped() && path.FileExists();
		}

		bool Movie::File::CanRecord() const
		{
			return IsReady() && Nes::Movie(emulator).IsStopped();
		}

		bool Movie::File::CanStop() const
		{
			return stream;
		}

		bool Movie::File::CanRewind() const
		{
			return pos != POS_BEG && CanSetPos();
		}

		bool Movie::File::CanForward() const
		{
			return pos != POS_END && CanSetPos();
		}

		bool Movie::File::IsReady() const
		{
			return path.Length() && emulator.IsGame();
		}

		bool Movie::File::CanSetPos() const
		{
			return IsReady() && Nes::Movie(emulator).IsStopped() && path.FileExists();
		}
	}
}
