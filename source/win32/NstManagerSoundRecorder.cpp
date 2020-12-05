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

#include "NstResourceString.hpp"
#include "NstIoScreen.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowUser.hpp"
#include "NstManager.hpp"
#include "NstDialogSound.hpp"
#include "NstManagerSound.hpp"
#include "NstManagerSoundRecorder.hpp"

namespace Nestopia
{
	namespace Managers
	{
		Sound::Recorder::Recorder
		(
			Window::Menu& m,
			Window::Sound::Recorder& d,
			Emulator& e
		)
		:
		Manager   ( e, m, this, &Recorder::OnEmuEvent ),
		recording ( false ),
		file      ( Io::Wave::MODE_WRITE ),
		dialog    ( d )
		{
			static const Window::Menu::CmdHandler::Entry<Recorder> commands[] =
			{
				{ IDM_FILE_SOUND_RECORDER_FILE,   &Recorder::OnCmdFile   },
				{ IDM_FILE_SOUND_RECORDER_START,  &Recorder::OnCmdRecord },
				{ IDM_FILE_SOUND_RECORDER_STOP,   &Recorder::OnCmdStop   },
				{ IDM_FILE_SOUND_RECORDER_REWIND, &Recorder::OnCmdRewind }
			};

			menu.Commands().Add( this, commands );

			static const Window::Menu::PopupHandler::Entry<Recorder> popups[] =
			{
				{ Window::Menu::PopupHandler::Pos<IDM_POS_FILE,IDM_POS_FILE_SOUNDRECORDER>::ID, &Recorder::OnMenu }
			};

			menu.Popups().Add( this, popups );
		}

		Sound::Recorder::~Recorder()
		{
		}

		bool Sound::Recorder::CanRecord() const
		{
			return
			(
				!recording &&
				dialog.WaveFile().Length() &&
				waveFormat.nSamplesPerSec &&
				waveFormat.wBitsPerSample &&
				!emulator.NetPlayers() &&
				emulator.IsOn()
			);
		}

		bool Sound::Recorder::CanRewind() const
		{
			return !recording && file.IsOpen();
		}

		bool Sound::Recorder::CanStop() const
		{
			return recording;
		}

		void Sound::Recorder::Close()
		{
			recording = false;

			try
			{
				file.Close();
			}
			catch (Io::Wave::Exception id)
			{
				Window::User::Fail( id );
			}
		}

		void Sound::Recorder::Enable(const WAVEFORMATEX* newWaveFormat)
		{
			if (newWaveFormat)
			{
				if (waveFormat == *newWaveFormat)
					return;

				waveFormat = *newWaveFormat;
			}
			else
			{
				waveFormat.Clear();
			}

			Close();
		}

		void Sound::Recorder::OnMenu(const Window::Menu::PopupHandler::Param& param)
		{
			menu[ IDM_FILE_SOUND_RECORDER_START  ].Enable( !param.show || CanRecord() );
			menu[ IDM_FILE_SOUND_RECORDER_STOP   ].Enable( !param.show || CanStop()   );
			menu[ IDM_FILE_SOUND_RECORDER_REWIND ].Enable( !param.show || CanRewind() );
		}

		void Sound::Recorder::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_POWER_OFF:

					recording = false;
					break;

				case Emulator::EVENT_NETPLAY_MODE:

					menu[IDM_POS_FILE][IDM_POS_FILE_SOUNDRECORDER].Enable( !data );
					break;
			}
		}

		void Sound::Recorder::OnCmdFile(uint)
		{
			dialog.Open();

			if (file.IsOpen() && file.GetName() != dialog.WaveFile())
				Close();
		}

		void Sound::Recorder::OnCmdRecord(uint)
		{
			if (CanRecord())
			{
				if (!file.IsOpen())
				{
					try
					{
						file.Open( dialog.WaveFile(), waveFormat );
					}
					catch (Io::Wave::Exception id)
					{
						Window::User::Fail( id );
						return;
					}

					size = 0;
					nextSmallSizeNotification = SMALL_SIZE;
					nextBigSizeNotification = BIG_SIZE;
				}

				Io::Screen() << Resource::String(size ? IDS_SCREEN_SOUND_RECORDER_RESUME : IDS_SCREEN_SOUND_RECORDER_START);

				recording = true;
				Resume();
			}
		}

		void Sound::Recorder::OnCmdStop(uint)
		{
			if (CanStop())
			{
				Io::Screen() << Resource::String(IDS_SCREEN_SOUND_RECORDER_STOP);

				recording = false;
				Resume();
			}
		}

		void Sound::Recorder::OnCmdRewind(uint)
		{
			if (CanRewind())
			{
				try
				{
					file.Close();
				}
				catch (Io::Wave::Exception id)
				{
					Window::User::Fail( id );
				}

				Resume();
			}
		}

		void Sound::Recorder::Flush(const Nes::Sound::Output& output)
		{
			if (recording)
			{
				NST_VERIFY( file.IsOpen() );

				try
				{
					for (uint i=0; i < 2; ++i)
						file.Write( output.samples[i], output.length[i] * waveFormat.nBlockAlign );
				}
				catch (Io::Wave::Exception ids)
				{
					recording = false;
					Window::User::Fail( ids );
					return;
				}

				size += output.length[0] + output.length[1];

				if (size >= nextBigSizeNotification)
				{
					nextBigSizeNotification += BIG_SIZE;
					Window::User::Inform( IDS_WAVE_WARN_FILE_BIG );
				}
				else if (size >= nextSmallSizeNotification)
				{
					Io::Screen() << Resource::String(IDS_SCREEN_SOUND_RECORDER_WRITTEN).Invoke( HeapString() << (nextSmallSizeNotification / ONE_MB) );
					nextSmallSizeNotification += SMALL_SIZE;
				}
			}
		}
	}
}
