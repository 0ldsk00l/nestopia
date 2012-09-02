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

#include "NstIoFile.hpp"
#include "NstIoLog.hpp"
#include "NstIoArchive.hpp"
#include "NstResourceString.hpp"
#include "NstManagerPaths.hpp"
#include "NstWindowUser.hpp"
#include "NstDialogSound.hpp"
#include "NstManagerSound.hpp"
#include "NstManagerSoundRecorder.hpp"
#include "NstManagerPreferences.hpp"

namespace Nestopia
{
	namespace Managers
	{
		struct Sound::Callbacks
		{
			static bool NST_CALLBACK Lock(Nes::Sound::UserData,Nes::Sound::Output&);
			static void NST_CALLBACK Unlock(Nes::Sound::UserData,Nes::Sound::Output&);
		};

		Sound::Sound
		(
			Window::Custom& window,
			Window::Menu& m,
			Emulator& e,
			const Paths& p,
			const Preferences& r,
			const Configuration& cfg
		)
		:
		Manager     ( e, m, this, &Sound::OnEmuEvent, IDM_OPTIONS_SOUND, &Sound::OnMenuOptionsSound ),
		paths       ( p ),
		preferences ( r ),
		directSound ( window ),
		dialog      ( new Window::Sound(e,directSound.GetAdapters(),p,cfg) ),
		recorder    ( new Recorder(m,dialog->GetRecorder(),e) )
		{
			Nes::Sound::Output::lockCallback.Set( &Callbacks::Lock, this );
			Nes::Sound::Output::unlockCallback.Set( &Callbacks::Unlock, this );

			UpdateSettings();
		}

		Sound::~Sound()
		{
			Nes::Sound::Output::lockCallback.Unset();
			Nes::Sound::Output::unlockCallback.Unset();
		}

		bool Sound::CanRunInBackground() const
		{
			if (emulator.IsNsf())
				return menu[IDM_MACHINE_NSF_OPTIONS_PLAYINBACKGROUND].Checked();
			else
				return preferences[Managers::Preferences::RUN_IN_BACKGROUND];
		}

		void Sound::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_POWER_ON:
				case Emulator::EVENT_SPEED:

					if (emuOutput && emulator.NetPlayers() == 0)
					{
						Nes::Sound( emulator ).SetSpeed( emulator.GetSpeed() );
						Nes::Sound( emulator ).EmptyBuffer();
					}
					break;

				case Emulator::EVENT_REWINDING_START:
				case Emulator::EVENT_REWINDING_STOP:

					Nes::Sound(emulator).SetVolume
					(
						Nes::Sound::CHANNEL_DPCM,
						event == Emulator::EVENT_REWINDING_START ? 0 : dialog->GetVolume(Nes::Sound::CHANNEL_DPCM)
					);

				case Emulator::EVENT_REWINDING_PREPARE:

					directSound.StopStream();
					break;

				case Emulator::EVENT_NETPLAY_MODE:

					menu[IDM_OPTIONS_SOUND].Enable( !data );
					break;
			}
		}

		void Sound::OnMenuOptionsSound(uint)
		{
			dialog->Open();
			UpdateSettings();
		}

		void Sound::Save(Configuration& cfg) const
		{
			dialog->Save( cfg );
		}

		void Sound::UpdateSettings()
		{
			cstring errMsg = NULL;

			if (dialog->SoundEnabled())
			{
				Nes::Sound nesSound( emulator );

				errMsg = directSound.Update
				(
					dialog->GetAdapter(),
					nesSound.GetSampleRate(),
					nesSound.GetSampleBits(),
					nesSound.GetSpeaker() == Nes::Sound::SPEAKER_STEREO ? DirectX::DirectSound::STEREO : DirectX::DirectSound::MONO,
					emulator.IsNsf() ? 500 : (dialog->GetLatency() + 2) * 21,
					dialog->GetPool(),
					CanRunInBackground()
				);

				if (errMsg == NULL)
				{
					emuOutput = &output;
					nesSound.SetSpeed( emulator.GetSpeed() );
					recorder->Enable( &directSound.GetWaveFormat() );
					return;
				}
			}
			else
			{
				directSound.Destroy();
			}

			Disable( errMsg );
		}

		void Sound::Disable(cstring const errMsg)
		{
			emuOutput = NULL;
			Nes::Sound( emulator ).SetVolume( Nes::Sound::ALL_CHANNELS, 0 );
			recorder->Enable( NULL );

			if (errMsg)
				Window::User::Fail( Resource::String( IDS_ERR_SOUND_FAILED ).Invoke( errMsg ) );
		}

		void Sound::StartEmulation() const
		{
			Nes::Sound( emulator ).EmptyBuffer();
		}

		void Sound::StopEmulation()
		{
			directSound.StopStream();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		bool NST_CALLBACK Sound::Callbacks::Lock(Nes::Sound::UserData data,Nes::Sound::Output&)
		{
			Sound& sound = *static_cast<Sound*>(data);

			if (sound.directSound.Streaming())
			{
				return sound.directSound.LockStream( sound.output.samples, sound.output.length );
			}
			else
			{
				if (sound.CanRunInBackground() != sound.directSound.GlobalFocus())
					sound.UpdateSettings();

				sound.directSound.StartStream();
				return false;
			}
		}

		void NST_CALLBACK Sound::Callbacks::Unlock(Nes::Sound::UserData data,Nes::Sound::Output&)
		{
			Sound& sound = *static_cast<Sound*>(data);

			if (sound.recorder->IsRecording())
				sound.recorder->Flush( sound.output );

			sound.directSound.UnlockStream( sound.output.samples, sound.output.length );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
