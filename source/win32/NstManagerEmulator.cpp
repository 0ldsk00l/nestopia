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

#include <new>
#include "NstIoFile.hpp"
#include "NstIoLog.hpp"
#include "NstIoStream.hpp"
#include "NstIoArchive.hpp"
#include "NstIoScreen.hpp"
#include "NstIoWave.hpp"
#include "NstResourceString.hpp"
#include "NstWindowUser.hpp"
#include "NstApplicationInstance.hpp"
#include "NstManagerEmulator.hpp"
#include "../core/api/NstApiMachine.hpp"
#include "../core/api/NstApiCartridge.hpp"
#include "../core/api/NstApiFds.hpp"
#include "../core/api/NstApiNsf.hpp"
#include "../core/api/NstApiMovie.hpp"
#include "../core/api/NstApiTapeRecorder.hpp"
#include "../core/api/NstApiRewinder.hpp"
#include "../core/api/NstApiUser.hpp"

namespace Nestopia
{
	namespace Managers
	{
		struct Emulator::Callbacks
		{
			static Nes::User::Answer NST_CALLBACK Confirm(Nes::User::UserData,Nes::User::Question question)
			{
				NST_COMPILE_ASSERT( Nes::User::NUM_QUESTION_CALLBACKS == 2 );

				switch (question)
				{
					case Nes::User::QUESTION_NST_PRG_CRC_FAIL_CONTINUE:
					case Nes::User::QUESTION_NSV_PRG_CRC_FAIL_CONTINUE:

						return Window::User::Confirm( IDS_EMU_CRC_MISSMATCH_CONTINUE, IDS_EMU_CRC_MISSMATCH ) ? Nes::User::ANSWER_YES : Nes::User::ANSWER_NO;

					default:

						return Nes::User::ANSWER_DEFAULT;
				}
			}

			static uint NST_CALLBACK ChooseProfile(Nes::User::UserData,const Nes::Cartridge::Profile*,const std::wstring* names,const uint count)
			{
				std::vector<wcstring> strings( count );

				for (uint i=0; i < count; ++i)
					strings[i] = names[i].c_str();

				const uint choice = Window::User::Choose( IDS_CHOOSE_GAME, IDS_TEXT_DEFAULT, &strings.front(), count );

				return choice ? choice - 1 : Nes::Cartridge::CHOOSE_DEFAULT_PROFILE;
			}

			static void NST_CALLBACK DoFileIO(Nes::User::UserData user,Nes::User::File& context)
			{
				NST_COMPILE_ASSERT( Nes::User::NUM_FILE_CALLBACKS == 17 );
				NST_ASSERT( user );

				Emulator& emulator = *static_cast<Emulator*>(user);

				switch (context.GetAction())
				{
					case Nes::User::File::LOAD_ROM:
					case Nes::User::File::LOAD_SAMPLE:

						emulator.LoadFileData
						(
							context,
							context.GetAction() == Nes::User::File::LOAD_SAMPLE,
							emulator.settings.paths.image.Archive(),
							context.GetName()
						);
						break;

					case Nes::User::File::LOAD_BATTERY:
					case Nes::User::File::LOAD_EEPROM:
					case Nes::User::File::LOAD_TAPE:
					case Nes::User::File::LOAD_TURBOFILE:

						emulator.LoadImageData( context );
						break;

					case Nes::User::File::SAVE_BATTERY:
					case Nes::User::File::SAVE_EEPROM:
					case Nes::User::File::SAVE_TAPE:
					case Nes::User::File::SAVE_TURBOFILE:

						emulator.SaveImageData( context );
						break;

					case Nes::User::File::LOAD_FDS:

						emulator.LoadDiskData( context );
						break;

					case Nes::User::File::SAVE_FDS:

						emulator.SaveDiskData( context );
						break;

					case Nes::User::File::LOAD_SAMPLE_MOERO_PRO_YAKYUU:

						emulator.LoadSampleData( L"moepro", context );
						break;

					case Nes::User::File::LOAD_SAMPLE_MOERO_PRO_YAKYUU_88:

						emulator.LoadSampleData( L"moepro88", context );
						break;

					case Nes::User::File::LOAD_SAMPLE_MOERO_PRO_TENNIS:

						emulator.LoadSampleData( L"mptennis", context );
						break;

					case Nes::User::File::LOAD_SAMPLE_TERAO_NO_DOSUKOI_OOZUMOU:

						emulator.LoadSampleData( L"terao", context );
						break;

					case Nes::User::File::LOAD_SAMPLE_AEROBICS_STUDIO:

						emulator.LoadSampleData( L"ftaerobi", context );
						break;
				}
			}

			static void NST_CALLBACK OnMachine(Nes::User::UserData user,Nes::Machine::Event event,Nes::Result result)
			{
				NST_COMPILE_ASSERT( Nes::Machine::NUM_EVENT_CALLBACKS == 8 );
				NST_ASSERT( user );

				Emulator& emulator = *static_cast<Emulator*>(user);

				switch (event)
				{
					case Nes::Machine::EVENT_LOAD:

						emulator.events( EVENT_LOAD );
						break;

					case Nes::Machine::EVENT_UNLOAD:

						emulator.events( EVENT_UNLOAD );
						Io::Log() << "Emulator: unloaded \"" << emulator.settings.paths.image << "\"\r\n";
						emulator.settings.Reset();
						break;

					case Nes::Machine::EVENT_POWER_ON:

						emulator.events( EVENT_POWER_ON );
						break;

					case Nes::Machine::EVENT_POWER_OFF:

						if (NES_FAILED(result))
						{
							if (const uint msg = ResultToString( result ))
								Window::User::Fail( msg );
						}

					case Nes::Machine::EVENT_RESET_SOFT:
					case Nes::Machine::EVENT_RESET_HARD:

						emulator.Stop();

						if (emulator.state.paused)
						{
							emulator.state.paused = false;
							emulator.events( EVENT_RESUME );
						}

						emulator.events
						(
							event == Nes::Machine::EVENT_RESET_HARD ? EVENT_RESET_HARD :
							event == Nes::Machine::EVENT_RESET_SOFT ? EVENT_RESET_SOFT :
                                                                      EVENT_POWER_OFF
						);
						break;

					case Nes::Machine::EVENT_MODE_NTSC:
					case Nes::Machine::EVENT_MODE_PAL:

						emulator.events( event == Nes::Machine::EVENT_MODE_NTSC ? EVENT_MODE_NTSC : EVENT_MODE_PAL );

						if (emulator.settings.timing.baseSpeed == DEFAULT_SPEED)
						{
							emulator.events( EVENT_BASE_SPEED );
							emulator.events( EVENT_SPEED );
						}
						break;
				}
			}

			static void NST_CALLBACK OnEvent(Nes::User::UserData,Nes::User::Event event,const void* data)
			{
				NST_COMPILE_ASSERT( Nes::User::NUM_EVENT_CALLBACKS == 3 );

				switch (event)
				{
					case Nes::User::EVENT_CPU_UNOFFICIAL_OPCODE:

						NST_ASSERT( data );
						Io::Log() << "Cpu: warning, " << static_cast<cstring>(data) << " opcode executed\r\n";
						break;

					case Nes::User::EVENT_CPU_JAM:

						Io::Screen() << Resource::String(IDS_SCREEN_CPU_JAM);
						Io::Log() << "Cpu: jammed!\r\n";
						break;

					case Nes::User::EVENT_DISPLAY_TIMER:

						NST_ASSERT( data );
						Io::Screen() << static_cast<cstring>( data );
						break;
				}
			}

			static void NST_CALLBACK OnControllerPort(Nes::Input::UserData user,uint port,Nes::Input::Type type)
			{
				NST_ASSERT( user && port < Nes::Input::NUM_PORTS );

				static_cast<Emulator*>(user)->events( static_cast<Event>(EVENT_PORT1_CONTROLLER + port), type );
			}

			static void NST_CALLBACK OnAdapterPort(Nes::Input::UserData user,Nes::Input::Adapter adapter)
			{
				NST_ASSERT( user );

				static_cast<Emulator*>(user)->events( EVENT_PORT_ADAPTER, adapter );
			}

			static void NST_CALLBACK OnDisk(Nes::Nsf::UserData user,Nes::Fds::Event event,uint disk,uint side)
			{
				NST_COMPILE_ASSERT( Nes::Fds::NUM_DISK_CALLBACKS == 3 );
				NST_ASSERT( user && side <= 1 );

				static_cast<Emulator*>(user)->events
				(
					event == Nes::Fds::DISK_INSERT ? EVENT_DISK_INSERT :
					event == Nes::Fds::DISK_EJECT  ? EVENT_DISK_EJECT :
                                                     EVENT_DISK_NONSTANDARD,
					disk * 2 + side
				);
			}

			static void NST_CALLBACK OnTape(Nes::TapeRecorder::UserData user,Nes::TapeRecorder::Event event)
			{
				NST_COMPILE_ASSERT( Nes::TapeRecorder::NUM_EVENT_CALLBACKS == 3 );
				NST_ASSERT( user );

				static_cast<Emulator*>(user)->events
				(
					event == Nes::TapeRecorder::EVENT_PLAYING   ? EVENT_TAPE_PLAYING :
					event == Nes::TapeRecorder::EVENT_RECORDING ? EVENT_TAPE_RECORDING :
                                                                  EVENT_TAPE_STOPPED
				);
			}

			static void NST_CALLBACK OnNsf(Nes::Nsf::UserData user,Nes::Nsf::Event event)
			{
				NST_COMPILE_ASSERT( Nes::Nsf::NUM_EVENT_CALLBACKS == 3 );
				NST_ASSERT( user );

				if (event == Nes::Nsf::EVENT_STOP_SONG)
					static_cast<Emulator*>(user)->Stop();

				static_cast<Emulator*>(user)->events
				(
					event == Nes::Nsf::EVENT_SELECT_SONG ? EVENT_NSF_SELECT :
					event == Nes::Nsf::EVENT_PLAY_SONG   ? EVENT_NSF_PLAY :
                                                           EVENT_NSF_STOP
				);
			}

			static void NST_CALLBACK OnMovie(Nes::Nsf::UserData user,Nes::Movie::Event event,Nes::Result result)
			{
				NST_COMPILE_ASSERT( Nes::Movie::NUM_EVENT_CALLBACKS == 4 );
				NST_ASSERT( user );

				static_cast<Emulator*>(user)->events
				(
					event == Nes::Movie::EVENT_PLAYING         ? EVENT_MOVIE_PLAYING :
					event == Nes::Movie::EVENT_PLAYING_STOPPED ? EVENT_MOVIE_PLAYING_STOPPED :
					event == Nes::Movie::EVENT_RECORDING       ? EVENT_MOVIE_RECORDING :
                                                                 EVENT_MOVIE_RECORDING_STOPPED,
					result
				);
			}

			static void NST_CALLBACK OnRewind(Nes::Rewinder::UserData user,Nes::Rewinder::State state)
			{
				NST_COMPILE_ASSERT( Nes::Rewinder::NUM_STATE_CALLBACKS == 3 );
				NST_ASSERT( user );

				static_cast<Emulator*>(user)->events
				(
					state == Nes::Rewinder::PREPARING ? EVENT_REWINDING_PREPARE :
					state == Nes::Rewinder::REWINDING ? EVENT_REWINDING_START :
														EVENT_REWINDING_STOP
				);
			}
		};

		Emulator::EventHandler::~EventHandler()
		{
			NST_VERIFY( callbacks.Empty() );
		}

		void Emulator::EventHandler::Add(const Callback& callback)
		{
			NST_ASSERT( bool(callback) && !callbacks.Find( callback ) );
			callbacks.PushBack( callback );
		}

		void Emulator::EventHandler::Remove(const void* const instance)
		{
			for (Callbacks::Iterator it(callbacks.Begin()), end(callbacks.End()); it != end; ++it)
			{
				if (it->VoidPtr() == instance)
				{
					callbacks.Erase( it );
					break;
				}
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		void Emulator::EventHandler::operator () (const Event event,const Data data) const
		{
			for (Callbacks::ConstIterator it(callbacks.Begin()), end(callbacks.End()); it != end; ++it)
				(*it)( event, data );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		Emulator::Context::Context()
		: mode(UNKNOWN)
		{
			for (uint i=0; i < Nes::Input::NUM_PORTS; ++i)
				controllers[i] = UNKNOWN;
		}

		Emulator::Context::~Context()
		{
		}

		inline Emulator::Settings::Cartridge::Cartridge()
		: writeProtect(false) {}

		inline Emulator::Settings::Fds::Fds()
		: save(DISKIMAGE_SAVE_TO_IMAGE) {}

		inline Emulator::Settings::Timing::Timing()
		:
		speed           (DEFAULT_SPEED),
		baseSpeed       (DEFAULT_SPEED),
		speeding        (false),
		sync            (false),
		tripleBuffering (false),
		rewinding       (false)
		{}

		Emulator::Settings::Settings()
		: askSave(false) {}

		Emulator::Settings::~Settings()
		{
		}

		void Emulator::Settings::Reset()
		{
			paths.start.Clear();
			paths.image.Clear();
			paths.save.Clear();
			paths.tape.Clear();
			askSave = false;
		}

		inline Emulator::State::State()
		:
		running     (false),
		paused      (false),
		activator   (this,&State::NoActivator),
		inactivator (this,&State::NoInactivator)
		{}

		bool Emulator::State::NoActivator()
		{
			return false;
		}

		void Emulator::State::NoInactivator()
		{
		}

		Emulator::Netplay::Netplay()
		: player(0), players(0) {}

		Emulator::Emulator()
		{
			Nes::Machine::eventCallback.Set( &Callbacks::OnMachine, this );
			Nes::User::eventCallback.Set( &Callbacks::OnEvent, NULL );
			Nes::User::questionCallback.Set( &Callbacks::Confirm, NULL );
			Nes::User::fileIoCallback.Set( &Callbacks::DoFileIO, this );
			Nes::Cartridge::chooseProfileCallback.Set( &Callbacks::ChooseProfile, NULL );
			Nes::Input::adapterCallback.Set( &Callbacks::OnAdapterPort, this );
			Nes::Input::controllerCallback.Set( &Callbacks::OnControllerPort, this );
			Nes::Fds::diskCallback.Set( &Callbacks::OnDisk, this );
			Nes::TapeRecorder::eventCallback.Set( &Callbacks::OnTape, this );
			Nes::Movie::eventCallback.Set( &Callbacks::OnMovie, this );
			Nes::Nsf::eventCallback.Set( &Callbacks::OnNsf, this );
			Nes::Rewinder::stateCallback.Set( &Callbacks::OnRewind, this );
		}

		Emulator::~Emulator()
		{
			Unload();

			Nes::Machine::eventCallback.Unset();
			Nes::User::eventCallback.Unset();
			Nes::User::questionCallback.Unset();
			Nes::User::fileIoCallback.Unset();
			Nes::Cartridge::chooseProfileCallback.Unset();
			Nes::Input::adapterCallback.Unset();
			Nes::Input::controllerCallback.Unset();
			Nes::Fds::diskCallback.Unset();
			Nes::TapeRecorder::eventCallback.Unset();
			Nes::Movie::eventCallback.Unset();
			Nes::Nsf::eventCallback.Unset();
			Nes::Rewinder::stateCallback.Unset();
		}

		bool Emulator::IsImage()
		{
			return Nes::Machine(*this).Is(Nes::Machine::IMAGE);
		}

		bool Emulator::IsGame()
		{
			return Nes::Machine(*this).Is(Nes::Machine::GAME);
		}

		bool Emulator::IsCart()
		{
			return Nes::Machine(*this).Is(Nes::Machine::CARTRIDGE);
		}

		bool Emulator::IsFds()
		{
			return Nes::Machine(*this).Is(Nes::Machine::DISK);
		}

		bool Emulator::IsNsf()
		{
			return Nes::Machine(*this).Is(Nes::Machine::SOUND);
		}

		bool Emulator::IsOn()
		{
			return Nes::Machine(*this).Is(Nes::Machine::ON);
		}

		bool Emulator::IsImageOn()
		{
			return Nes::Machine(*this).Is(Nes::Machine::IMAGE,Nes::Machine::ON);
		}

		bool Emulator::IsGameOn()
		{
			return Nes::Machine(*this).Is(Nes::Machine::GAME,Nes::Machine::ON);
		}

		bool Emulator::IsCartOn()
		{
			return Nes::Machine(*this).Is(Nes::Machine::CARTRIDGE,Nes::Machine::ON);
		}

		bool Emulator::IsFdsOn()
		{
			return Nes::Machine(*this).Is(Nes::Machine::DISK,Nes::Machine::ON);
		}

		bool Emulator::IsNsfOn()
		{
			return Nes::Machine(*this).Is(Nes::Machine::SOUND,Nes::Machine::ON);
		}

		bool Emulator::IsLocked()
		{
			return Nes::Machine(*this).IsLocked();
		}

		void Emulator::Unhook()
		{
			state.activator.Set( &state, &State::NoActivator );
			state.inactivator.Set( &state, &State::NoInactivator );
		}

		bool Emulator::Start()
		{
			if (!IsOn() || state.paused || (IsNsf() && !Nes::Nsf(*this).IsPlaying()))
				return false;

			state.running = state.activator();

			return state.running;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		void Emulator::Execute
		(
			Nes::Video::Output* const video,
			Nes::Sound::Output* const sound,
			Nes::Input::Controllers* const input
		)
		{
			if (state.running)
			{
				if (netplay)
					netplay.executor( *input );

				Nes::Emulator::Execute( video, sound, input );
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Emulator::Stop()
		{
			if (state.running)
			{
				state.running = false;
				state.inactivator();
			}
		}

		void Emulator::Pause(const bool pause)
		{
			NST_VERIFY( IsOn() );

			if (IsOn())
			{
				if (state.paused != pause)
				{
					Stop();
					state.paused = pause;
					events( pause ? EVENT_PAUSE : EVENT_RESUME );
				}
			}
		}

		bool Emulator::UsesBaseSpeed() const
		{
			return
			(
				(settings.timing.speeding|settings.timing.rewinding) == 0 ||
				(settings.timing.speed == DEFAULT_SPEED)
			);
		}

		uint Emulator::GetDefaultSpeed()
		{
			if (Nes::Machine(*this).GetMode() == Nes::Machine::NTSC)
				return Nes::Machine::CLK_NTSC_DOT / Nes::Machine::CLK_NTSC_VSYNC;
			else
				return Nes::Machine::CLK_PAL_DOT / Nes::Machine::CLK_PAL_VSYNC;
		}

		uint Emulator::GetBaseSpeed()
		{
			if (settings.timing.baseSpeed != DEFAULT_SPEED)
				return settings.timing.baseSpeed;
			else
				return GetDefaultSpeed();
		}

		uint Emulator::GetSpeed()
		{
			return UsesBaseSpeed() ? GetBaseSpeed() : settings.timing.speed;
		}

		void Emulator::ToggleSpeed(const bool speeding)
		{
			if (settings.timing.speeding != speeding && !netplay)
			{
				settings.timing.speeding = speeding;
				events( speeding ? EVENT_SPEEDING_ON : EVENT_SPEEDING_OFF );
			}
		}

		void Emulator::ToggleRewind(const bool rewinding)
		{
			if (settings.timing.rewinding != rewinding && !netplay)
			{
				settings.timing.rewinding = rewinding;
				events( rewinding ? EVENT_REWINDING_ON : EVENT_REWINDING_OFF );
			}
		}

		void Emulator::ResetSpeed(const uint baseSpeed,const bool sync,const bool tripleBuffering)
		{
			settings.timing.speed = DEFAULT_SPEED;
			settings.timing.baseSpeed = baseSpeed;
			settings.timing.speeding = false;
			settings.timing.sync = sync;
			settings.timing.tripleBuffering = tripleBuffering;
			settings.timing.rewinding = false;

			events( EVENT_BASE_SPEED );
			events( EVENT_SPEED );
		}

		void Emulator::SetSpeed(const uint speed)
		{
			if (settings.timing.speed != speed)
			{
				settings.timing.speed = speed;
				events( EVENT_SPEED );
			}
		}

		void Emulator::LoadFileData(Nes::User::File& context,const bool loadSamples,Path path,wcstring const filename) const
		{
			NST_VERIFY( filename && *filename );

			if (!filename || !*filename)
				return;

			cstring errReason = NULL;

			try
			{
				if (path.Length())
				{
					Io::File file( path, Io::File::COLLECT );
					Io::Archive archive( file );

					uint index = archive.Find( filename );

					if (index && index-- != Io::Archive::NO_FILES)
					{
						Collection::Buffer buffer( archive[index].Size() );

						if (buffer.Size())
						{
							archive[index].Uncompress( buffer.Ptr() );

							archive.Close();
							file.Close();

							if (loadSamples)
							{
								WAVEFORMATEX format;
								Io::Wave wave( Io::Wave::MODE_READ );

								if (const uint size = wave.Open( buffer.Ptr(), buffer.Size(), format ))
								{
									buffer.Resize( size );
									wave.Read( buffer.Ptr() );
									wave.Close();

									const Nes::Result result = context.SetSampleContent
									(
										buffer.Ptr(),
										buffer.Size() / format.nBlockAlign,
										format.nChannels == 2,
										format.wBitsPerSample,
										format.nSamplesPerSec
									);

									if (NES_FAILED(result))
										throw result;
								}
								else
								{
									throw Nes::RESULT_ERR_INVALID_FILE;
								}
							}
							else
							{
								const Nes::Result result = context.SetContent( buffer.Ptr(), buffer.Size() );

								if (NES_FAILED(result))
									throw result;
							}
						}
						else
						{
							throw Nes::RESULT_ERR_INVALID_FILE;
						}
					}
					else
					{
						throw Io::File::ERR_NOT_FOUND;
					}
				}
				else
				{
					path.Set( settings.paths.image.Directory(), filename );

					if (path.FileExists())
					{
						if (loadSamples)
						{
							WAVEFORMATEX format;
							Io::Wave wave( Io::Wave::MODE_READ );

							if (const uint size = wave.Open( path, format ))
							{
								Collection::Buffer buffer( size );
								wave.Read( buffer.Ptr() );
								wave.Close();

								const Nes::Result result = context.SetSampleContent
								(
									buffer.Ptr(),
									buffer.Size() / format.nBlockAlign,
									format.nChannels == 2,
									format.wBitsPerSample,
									format.nSamplesPerSec
								);

								if (NES_FAILED(result))
									throw result;
							}
							else
							{
								throw Nes::RESULT_ERR_INVALID_FILE;
							}
						}
						else
						{
							Io::Stream::In stream( path );
							const Nes::Result result = context.SetContent( stream );

							if (NES_FAILED(result))
								throw result;
						}
					}
					else
					{
						throw Io::File::ERR_NOT_FOUND;
					}
				}
			}
			catch (Nes::Result r)
			{
				if (r == Nes::RESULT_ERR_UNSUPPORTED)
					errReason = "unsupported format";
				else
					errReason = "invalid data";
			}
			catch (Io::File::Exception e)
			{
				if (e == Io::File::ERR_NOT_FOUND)
					errReason = "file not found";
				else
					errReason = "file I/O error";
			}
			catch (Io::Wave::Exception)
			{
				errReason = "invalid data";
			}
			catch (...)
			{
				NST_DEBUG_MSG("Emulator::LoadFileData() generic error!");
				errReason = "generic error";
			}

			if (errReason)
				Io::Log() << "Emulator: error loading \"" << filename << "\" - " << errReason << "!\r\n";
			else
				Io::Log() << "Emulator: loading \"" << filename << "\"\r\n";
		}

		void Emulator::LoadImageData(Nes::User::File& context) const
		{
			Path path( settings.paths.save );

			cstring desc = "image save";

			switch (context.GetAction())
			{
				case Nes::User::File::LOAD_BATTERY:

					desc = "battery";
					break;

				case Nes::User::File::LOAD_EEPROM:

					desc = "EEPROM";
					break;

				case Nes::User::File::LOAD_TAPE:

					desc = "tape";
					path = settings.paths.tape;
					break;

				case Nes::User::File::LOAD_TURBOFILE:

					desc = "turbo file";

					if (path.Length())
						path.Extension() = L"tf";

					break;
			}

			if (path.FileExists())
			{
				try
				{
					Io::Stream::In stream( path );
					const Nes::Result result = context.SetContent( stream );

					if (NES_SUCCEEDED(result))
					{
						Io::Log() << "Emulator: loaded " << desc << " data from \"" << path << "\"\r\n";
					}
					else
					{
						HeapString msg( Resource::String( IDS_CARTRIDGE_LOAD_FAILED ).Invoke( path ) );

						if (const uint id = ResultToString( result ))
							msg << ' ' << Resource::String( id );

						Window::User::Warn( msg.Ptr() );
					}
				}
				catch (Io::File::Exception id)
				{
					HeapString msg;

					msg << Resource::String( IDS_CARTRIDGE_LOAD_FAILED ).Invoke( path )
						<< ' '
						<< Resource::String( id );

					Window::User::Warn( msg.Ptr() );
				}
			}
			else if (context.GetAction() != Nes::User::File::LOAD_TAPE)
			{
				if (path.Length())
					Io::Log() << "Emulator: " << desc << " data file \"" << path << "\" not found\r\n";
				else
					Io::Log() << "Emulator: " << desc << " data was not loaded!\r\n";
			}
		}

		void Emulator::SaveImageData(Nes::User::File& context) const
		{
			Path path( settings.paths.save );

			cstring desc = "image save";
			uint ids = IDS_EMU_MOVIE_SAVE_IMAGEDATA;

			switch (context.GetAction())
			{
				case Nes::User::File::SAVE_BATTERY:

					desc = "battery";
					break;

				case Nes::User::File::SAVE_EEPROM:

					desc = "EEPROM";
					ids = IDS_EMU_MOVIE_SAVE_EEPROM;
					break;

				case Nes::User::File::SAVE_TAPE:

					desc = "cassette tape";
					ids = IDS_EMU_MOVIE_SAVE_TAPE;
					path = settings.paths.tape;
					break;

				case Nes::User::File::SAVE_TURBOFILE:

					desc = "turbo file";
					ids = IDS_EMU_MOVIE_SAVE_TURBOFILE;

					if (path.Length())
						path.Extension() = L"tf";

					break;
			}

			if (settings.cartridge.writeProtect)
			{
				Io::Log() << "Emulator: write-protection enabled, discarding " << desc << " data..\r\n";
			}
			else if (path.Length())
			{
				if (!settings.askSave || Window::User::Confirm( ids ))
				{
					try
					{
						Io::Stream::Out stream( path );
						const Nes::Result result = context.GetContent( stream );

						if (NES_SUCCEEDED(result))
						{
							Io::Log() << "Emulator: " << desc << " data was saved to \"" << path << "\"\r\n";
						}
						else
						{
							HeapString msg( Resource::String( IDS_CARTRIDGE_SAVE_FAILED ).Invoke( path ) );

							if (const uint id = ResultToString( result ))
								msg << ' ' << Resource::String( id );

							Window::User::Warn( msg.Ptr() );
						}
					}
					catch (Io::File::Exception id)
					{
						HeapString msg;

						msg << Resource::String( IDS_CARTRIDGE_SAVE_FAILED ).Invoke( path )
							<< ' '
							<< Resource::String( id );

						Window::User::Warn( msg.Ptr() );
					}
				}
			}
			else
			{
				Io::Log() << "Emulator: warning, " << desc << " data was not saved!\r\n";
			}
		}

		void Emulator::LoadDiskData(Nes::User::File& context) const
		{
			switch (settings.fds.save)
			{
				case DISKIMAGE_SAVE_TO_PATCH:

					if (settings.paths.save.FileExists())
					{
						try
						{
							Io::Stream::In stream( settings.paths.save );
							const Nes::Result result = context.SetPatchContent( stream );

							if (NES_SUCCEEDED(result))
							{
								Io::Log() << "Emulator: patched disk image with \"" << settings.paths.save << "\"\r\n";
							}
							else
							{
								HeapString msg( Resource::String( IDS_FDS_PATCHDATALOAD_FAILED ).Invoke( settings.paths.save ) );

								if (const uint id = ResultToString( result ))
									msg << ' ' << Resource::String( id );

								Window::User::Warn( msg.Ptr() );
							}
						}
						catch (Io::File::Exception id)
						{
							HeapString msg;

							msg << Resource::String( IDS_FDS_PATCHDATALOAD_FAILED ).Invoke( settings.paths.save )
								<< ' '
								<< Resource::String( id );

							Window::User::Warn( msg.Ptr() );
						}
					}
					else if (settings.paths.save.Length())
					{
						Io::Log() << "Emulator: patch file \"" << settings.paths.save << "\" not found\r\n";
					}
					break;

				case DISKIMAGE_SAVE_TO_IMAGE:

					if (!settings.paths.image.FileExists())
						Window::User::Warn( IDS_EMU_FDS_NO_FILE );

					break;
			}
		}

		void Emulator::SaveDiskData(Nes::User::File& context) const
		{
			if (!settings.askSave || Window::User::Confirm( IDS_EMU_MOVIE_SAVE_FDS ))
			{
				switch (settings.fds.save)
				{
					case DISKIMAGE_SAVE_TO_IMAGE:

						if (settings.paths.image.FileExists())
						{
							try
							{
								Io::Stream::Out stream( settings.paths.image );
								const Nes::Result result = context.GetContent( stream );

								if (NES_SUCCEEDED(result))
								{
									Io::Log() << "Emulator: saved disk image to \"" << settings.paths.image << "\"\r\n";
								}
								else
								{
									HeapString msg( Resource::String( IDS_FDS_SAVE_FAILED ) );

									if (const uint id = ResultToString( result ))
										msg << ' ' << Resource::String( id );

									Window::User::Warn( msg.Ptr() );
								}
							}
							catch (Io::File::Exception id)
							{
								HeapString msg;

								msg << Resource::String( IDS_FDS_SAVE_FAILED )
									<< ' '
									<< Resource::String( id );

								Window::User::Warn( msg.Ptr() );
							}
						}
						else
						{
							Io::Log() << "Emulator: warning, disk image file \"" << settings.paths.image << "\" not found, discarding changes!\r\n";
						}
						break;

					case DISKIMAGE_SAVE_TO_PATCH:

						if (settings.paths.save.Length())
						{
							try
							{
								Io::Stream::Out stream( settings.paths.save );

								const Nes::Result result = context.GetPatchContent
								(
									settings.paths.save.Extension() == L"ips" ? Nes::User::File::PATCH_IPS : Nes::User::File::PATCH_UPS,
									stream
								);

								if (NES_SUCCEEDED(result))
								{
									Io::Log() << "Emulator: saved disk image patch to \"" << settings.paths.save << "\"\r\n";
								}
								else
								{
									HeapString msg( Resource::String( IDS_FDS_PATCHDATASAVE_FAILED ).Invoke( settings.paths.save ) );

									if (const uint id = ResultToString( result ))
										msg << ' ' << Resource::String( id );

									Window::User::Warn( msg.Ptr() );
								}
							}
							catch (Io::File::Exception id)
							{
								HeapString msg;

								msg << Resource::String( IDS_FDS_PATCHDATASAVE_FAILED ).Invoke( settings.paths.save )
									<< ' '
									<< Resource::String( id );

								Window::User::Warn( msg.Ptr() );
							}
						}
						break;

					case DISKIMAGE_SAVE_DISABLED:

						Io::Log() << "Emulator: disk image changes were not saved\r\n";
						break;
				}
			}
		}

		void Emulator::LoadSampleData(wcstring const filename,Nes::User::File& context) const
		{
			NST_ASSERT( filename && *filename );

			Path path( settings.paths.samples, filename );

			for (uint i=0; ; ++i)
			{
				static const wchar_t types[][4] =
				{
					L"zip",
					L"rar",
					L"7z\0"
				};

				path.Extension() = types[i];

				if (path.FileExists())
					break;

				if (i == 2)
				{
					Io::Log() << "Emulator: warning, sound sample pack not found!";
					return;
				}
			}

			wchar_t wavefile[] = L"xx.wav";

			uint id = context.GetId();

			if (id > 99)
				return;

			wavefile[0] = '0' + id / 10;
			wavefile[1] = '0' + id % 10;

			LoadFileData( context, true, path, wavefile );
		}

		bool Emulator::Load
		(
			const Collection::Buffer& imageBuffer,
			const Path& start,
			const Collection::Buffer& patchBuffer,
			const bool bypassPatchValidation,
			const Context& context,
			const Nes::Machine::FavoredSystem favoredSystem,
			const Nes::Machine::AskProfile ask,
			const bool warn
		)
		{
			Application::Instance::Waiter wait;

			Unload();

			Io::Log() << "Emulator: loading \"" << context.image << "\"\r\n";

			settings.paths.start = start;
			settings.paths.image = context.image;
			settings.paths.save = context.save;
			settings.paths.tape = context.tape;
			settings.paths.samples = context.samples;

			Nes::Result result = Load( imageBuffer, patchBuffer, bypassPatchValidation, favoredSystem, ask, warn );

			if (result == Nes::RESULT_ERR_MISSING_BIOS && !netplay)
			{
				if (Window::User::Confirm( IDS_EMU_FDS_SUPPLY_BIOS ))
				{
					events( EVENT_DISK_QUERY_BIOS );

					if (Nes::Fds(*this).HasBIOS())
						result = Load( imageBuffer, patchBuffer, bypassPatchValidation, favoredSystem, ask, warn );
				}
				else
				{
					settings.Reset();
					return false;
				}
			}

			if (const uint msg = ResultToString( result ))
			{
				if (NES_FAILED(result))
				{
					Window::User::Fail( msg );
				}
				else if (warn)
				{
					Window::User::Warn( msg );
				}
				else
				{
					Io::Log() << "Emulator: warning, " << Resource::String( msg ) << "\r\n";
				}
			}

			if (NES_SUCCEEDED(result))
			{
				if (context.mode != Context::UNKNOWN)
					Nes::Machine(*this).SetMode( static_cast<Nes::Machine::Mode>(context.mode) );

				if (IsGame())
				{
					for (uint port=0; port < Nes::Input::NUM_PORTS; ++port)
					{
						if (context.controllers[port] != Context::UNKNOWN)
							Nes::Input(*this).ConnectController( port, static_cast<Nes::Input::Type>(context.controllers[port]) );
					}
				}

				return true;
			}
			else
			{
				settings.Reset();
				return false;
			}
		}

		Nes::Result Emulator::Load
		(
			const Collection::Buffer& imageBuffer,
			const Collection::Buffer& patchBuffer,
			const bool bypassPatchValidation,
			const Nes::Machine::FavoredSystem favoredSystem,
			const Nes::Machine::AskProfile ask,
			const bool warn
		)
		{
			Io::Stream::In imageStream( imageBuffer );

			if (patchBuffer.Size())
			{
				Io::Stream::In patchStream( patchBuffer );
				Nes::Machine::Patch patch( patchStream, bypassPatchValidation );
				const Nes::Result result = Nes::Machine(*this).Load( imageStream, favoredSystem, patch, ask );

				if (NES_FAILED(patch.result))
				{
					HeapString msg( Resource::String(IDS_EMU_WARN_PATCHING_FAILED) );

					if (const uint id = ResultToString( patch.result ))
						msg << ' ' << Resource::String( id );

					if (warn)
						Window::User::Warn( msg.Ptr() );
					else
						Io::Log() << "Emulator: warning, " << msg << "\r\n";
				}

				return result;
			}
			else
			{
				return Nes::Machine(*this).Load( imageStream, favoredSystem, ask );
			}
		}

		void Emulator::Unload()
		{
			Nes::Machine(*this).Unload();
		}

		bool Emulator::SaveState(Collection::Buffer& buffer,const bool compress,const Alert alert)
		{
			buffer.Clear();

			Nes::Result result;

			{
				Io::Stream::Out stream( buffer );
				result = Nes::Machine(*this).SaveState( stream, compress ? Nes::Machine::USE_COMPRESSION : Nes::Machine::NO_COMPRESSION );
			}

			if (NES_SUCCEEDED(result))
				return true;

			if (const uint msg = ResultToString( result ))
			{
				if (alert == NOISY)
				{
					Window::User::Fail( msg, IDS_EMU_ERR_SAVE_STATE );
				}
				else if (alert == STICKY)
				{
					Io::Screen() << Resource::String( IDS_EMU_ERR_SAVE_STATE )
                                 << ' '
                                 << Resource::String( msg );
				}
				else
				{
					Io::Log() << "Emulator: "
                              << Resource::String( IDS_EMU_ERR_SAVE_STATE )
                              << ' '
                              << Resource::String( msg )
                              << "\r\n";
				}

			}

			return false;
		}

		bool Emulator::LoadState(Collection::Buffer& buffer,const Alert alert)
		{
			const bool on = IsOn();

			if (!Power( true ))
				return false;

			Nes::Result result;

			{
				Io::Stream::In stream( buffer );
				result = Nes::Machine(*this).LoadState( stream );
			}

			if (NES_SUCCEEDED(result))
				return true;

			if (!on)
				Power( false );

			if (const uint msg = ResultToString( result ))
			{
				if (alert == NOISY)
				{
					Window::User::Fail( msg, IDS_EMU_ERR_LOAD_STATE );
				}
				else if (alert == STICKY)
				{
					Io::Screen() << Resource::String( IDS_EMU_ERR_LOAD_STATE )
                                 << ' '
                                 << Resource::String( msg );
				}
				else
				{
					Io::Log() << "Emulator: "
                              << Resource::String( IDS_EMU_ERR_LOAD_STATE )
                              << ' '
                              << Resource::String( msg )
                              << "\r\n";
				}

			}

			return false;
		}

		bool Emulator::Power(const bool state)
		{
			const Nes::Result result = Nes::Machine(*this).Power( state );

			if (NES_SUCCEEDED(result))
			{
				return true;
			}
			else
			{
				if (const uint msg = ResultToString( result ))
					Window::User::Fail( msg );

				return false;
			}
		}

		void Emulator::SendCommand(Command command,const Data data)
		{
			if (netplay)
			{
				netplay.commander( command, data );
			}
			else switch (command)
			{
				case COMMAND_RESET:
				{
					const Nes::Result result = Nes::Machine(*this).Reset( data != 0 );

					if (NES_FAILED(result))
					{
						if (const uint msg = ResultToString( result ))
							Window::User::Fail( msg );
					}
					break;
				}

				case COMMAND_DISK_INSERT:

					Nes::Fds(*this).InsertDisk( data / 2, data % 2 );
					break;

				case COMMAND_DISK_EJECT:

					Nes::Fds(*this).EjectDisk();
					break;
			}
		}

		void Emulator::BeginNetplayMode()
		{
			NST_ASSERT( !netplay && !IsImage() );

			if (netplay.players == 0)
			{
				netplay.players = 1;
				events( EVENT_NETPLAY_MODE, true );
			}
		}

		void Emulator::StartNetplay
		(
			const Netplay::Executor& executor,
			const Netplay::Commander& commander,
			const uint player,
			const uint players
		)
		{
			NST_ASSERT( netplay.players && players && player-1 < players );

			netplay.executor = executor;
			netplay.commander = commander;
			netplay.player = player - 1;
			netplay.players = players;
		}

		void Emulator::StopNetplay()
		{
			NST_VERIFY( netplay.players );

			netplay.executor.Unset();
			netplay.commander.Unset();
			netplay.player = 0;
			netplay.players = 1;
		}

		void Emulator::EndNetplayMode()
		{
			NST_ASSERT( !netplay && !IsImage() );

			if (netplay.players)
			{
				netplay.players = 0;
				events( EVENT_NETPLAY_MODE, false );
			}
		}

		uint Emulator::ResultToString(const Nes::Result result)
		{
			if (NES_SUCCEEDED(result))
			{
				if (result != Nes::RESULT_OK && result != Nes::RESULT_NOP)
				{
					switch (result)
					{
						case Nes::RESULT_WARN_BAD_DUMP:        return IDS_EMU_WARN_BAD_DUMP;
						case Nes::RESULT_WARN_BAD_PROM:        return IDS_EMU_WARN_BAD_PROM;
						case Nes::RESULT_WARN_BAD_CROM:        return IDS_EMU_WARN_BAD_CROM;
						case Nes::RESULT_WARN_BAD_FILE_HEADER: return IDS_EMU_WARN_BAD_INES;
					}

					NST_DEBUG_MSG("warning result not handled");
				}

				return 0;
			}
			else
			{
				switch (result)
				{
					case Nes::RESULT_ERR_WRONG_MODE:               return IDS_EMU_ERR_WRONG_MODE;
					case Nes::RESULT_ERR_MISSING_BIOS:             return IDS_EMU_ERR_BIOS_FILE_MISSING;
					case Nes::RESULT_ERR_UNSUPPORTED_MAPPER:       return IDS_EMU_ERR_UNSUPPORTED_MAPPER;
					case Nes::RESULT_ERR_UNSUPPORTED_VSSYSTEM:     return IDS_EMU_ERR_UNSUPPORTED_VSSYSTEM;
					case Nes::RESULT_ERR_UNSUPPORTED_FILE_VERSION: return IDS_EMU_ERR_UNSUPPORTED_FILE_VERSION;
					case Nes::RESULT_ERR_CORRUPT_FILE:             return IDS_FILE_ERR_CORRUPT;
					case Nes::RESULT_ERR_INVALID_FILE:             return IDS_FILE_ERR_INVALID;
					case Nes::RESULT_ERR_OUT_OF_MEMORY:            return IDS_ERR_OUT_OF_MEMORY;
					case Nes::RESULT_ERR_GENERIC:                  return IDS_ERR_GENERIC;
					case Nes::RESULT_ERR_INVALID_CRC:              return IDS_ERR_INVALID_CHECKSUM;
					case Nes::RESULT_ERR_NOT_READY:                return 0;
				}

				NST_DEBUG_MSG("error result not handled");

				return IDS_ERR_GENERIC;
			}
		}
	}
}
