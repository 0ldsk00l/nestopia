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

#ifndef NST_MANAGER_EMULATOR_H
#define NST_MANAGER_EMULATOR_H

#pragma once

#include "NstCollectionVector.hpp"
#include "NstObjectDelegate.hpp"
#include "NstString.hpp"
#include "../core/api/NstApiEmulator.hpp"
#include "../core/api/NstApiVideo.hpp"
#include "../core/api/NstApiSound.hpp"
#include "../core/api/NstApiInput.hpp"
#include "../core/api/NstApiMachine.hpp"
#include "../core/api/NstApiUser.hpp"

namespace Nes
{
	using namespace Api;
}

namespace Nestopia
{
	namespace Managers
	{
		class Emulator : public Nes::Emulator
		{
		public:

			Emulator();
			~Emulator();

			typedef int Data;

			enum Alert
			{
				NOISY,
				QUIETLY,
				STICKY
			};

			enum DiskImageSaveMethod
			{
				DISKIMAGE_SAVE_DISABLED,
				DISKIMAGE_SAVE_TO_IMAGE,
				DISKIMAGE_SAVE_TO_PATCH
			};

			enum Event
			{
				EVENT_LOAD = 1,
				EVENT_UNLOAD,
				EVENT_POWER_ON,
				EVENT_POWER_OFF,
				EVENT_RESET_SOFT,
				EVENT_RESET_HARD,
				EVENT_MODE_NTSC,
				EVENT_MODE_PAL,
				EVENT_PAUSE,
				EVENT_RESUME,
				EVENT_NETPLAY_MODE,
				EVENT_SPEED,
				EVENT_BASE_SPEED,
				EVENT_SPEEDING_ON,
				EVENT_SPEEDING_OFF,
				EVENT_DISK_INSERT,
				EVENT_DISK_EJECT,
				EVENT_DISK_NONSTANDARD,
				EVENT_DISK_QUERY_BIOS,
				EVENT_TAPE_PLAYING,
				EVENT_TAPE_RECORDING,
				EVENT_TAPE_STOPPED,
				EVENT_MOVIE_PLAYING,
				EVENT_MOVIE_PLAYING_STOPPED,
				EVENT_MOVIE_RECORDING,
				EVENT_MOVIE_RECORDING_STOPPED,
				EVENT_REWINDING_ON,
				EVENT_REWINDING_OFF,
				EVENT_REWINDING_PREPARE,
				EVENT_REWINDING_START,
				EVENT_REWINDING_STOP,
				EVENT_NSF_PLAY,
				EVENT_NSF_STOP,
				EVENT_NSF_SELECT,
				EVENT_PORT1_CONTROLLER,
				EVENT_PORT2_CONTROLLER,
				EVENT_PORT3_CONTROLLER,
				EVENT_PORT4_CONTROLLER,
				EVENT_PORT5_CONTROLLER,
				EVENT_PORT_ADAPTER
			};

			enum Command
			{
				COMMAND_RESET = 1,
				COMMAND_DISK_INSERT,
				COMMAND_DISK_EJECT
			};

			enum
			{
				NUM_COMMANDS = 3,
				DEFAULT_SPEED = 0,
				MASTER = 0
			};

			struct Context
			{
				Context();
				~Context();

				enum
				{
					UNKNOWN = INT_MAX
				};

				Path image;
				Path save;
				Path state;
				Path tape;
				Path movie;
				Path samples;
				uint controllers[Nes::Input::NUM_PORTS];
				uint mode;
			};

			void Stop();
			void Pause(bool);
			void ResetSpeed(uint,bool,bool);
			void SetSpeed(uint);
			uint GetDefaultSpeed();
			uint GetBaseSpeed();
			uint GetSpeed();
			void ToggleSpeed(bool);
			void ToggleRewind(bool);

			bool Load
			(
				const Collection::Buffer&,
				const Path&,
				const Collection::Buffer&,
				bool,
				const Context&,
				Nes::Machine::FavoredSystem,
				Nes::Machine::AskProfile,
				bool
			);

			void Unload();
			void SendCommand(Command,Data=0);
			bool SaveState(Collection::Buffer&,bool,Alert=NOISY);
			bool LoadState(Collection::Buffer&,Alert=NOISY);
			bool Power(bool);
			void Execute(Nes::Video::Output*,Nes::Sound::Output*,Nes::Input::Controllers*);
			void BeginNetplayMode();
			void EndNetplayMode();
			void StopNetplay();
			void Unhook();

			bool IsImage();
			bool IsGame();
			bool IsCart();
			bool IsFds();
			bool IsNsf();
			bool IsOn();
			bool IsImageOn();
			bool IsGameOn();
			bool IsCartOn();
			bool IsFdsOn();
			bool IsNsfOn();
			bool IsLocked();

			static uint ResultToString(Nes::Result);

		private:

			struct Callbacks;

			class EventHandler
			{
				friend class Emulator;
				friend struct Callbacks;

				typedef Object::Delegate<void,Event,Data> Callback;
				typedef Collection::Vector<Callback> Callbacks;

				Callbacks callbacks;

				~EventHandler();

				void operator () (Event,Data=0) const;
				void Add(const Callback&);

			public:

				void Remove(const void*);

				template<typename Data,typename Code>
				void Add(Data* data,Code code)
				{
					Add( Callback(data,code) );
				}
			};

			struct Settings
			{
				Settings();
				~Settings();

				void Reset();

				struct Timing
				{
					inline Timing();

					uint speed;
					uint baseSpeed;
					bool speeding;
					bool sync;
					bool tripleBuffering;
					bool rewinding;
				};

				struct Fds
				{
					inline Fds();

					DiskImageSaveMethod save;
				};

				struct Cartridge
				{
					inline Cartridge();

					bool writeProtect;
				};

				Timing timing;

				struct
				{
					Path start;
					Path image;
					Path save;
					Path tape;
					Path samples;
				}   paths;

				Cartridge cartridge;
				Fds fds;
				bool askSave;
			};

			struct State
			{
				typedef Object::Delegate<bool> Activator;
				typedef Object::Delegate<void> Inactivator;

				inline State();

				bool NoActivator();
				void NoInactivator();

				bool running;
				bool paused;
				Activator activator;
				Inactivator inactivator;
			};

			struct Netplay : ImplicitBool<Netplay>
			{
				Netplay();

				typedef Object::Delegate<void,Command,Data> Commander;
				typedef Object::Delegate<void,Nes::Input::Controllers&> Executor;

				Executor executor;
				Commander commander;
				uint player;
				uint players;

				bool operator ! () const
				{
					return !executor;
				}
			};

			bool UsesBaseSpeed() const;
			void StartNetplay(const Netplay::Executor&,const Netplay::Commander&,uint,uint);
			bool Start();

			Nes::Result Load
			(
				const Collection::Buffer&,
				const Collection::Buffer&,
				bool,
				Nes::Machine::FavoredSystem,
				Nes::Machine::AskProfile,
				bool
			);

			void LoadFileData(Nes::User::File&,bool,Path,wcstring) const;
			void LoadImageData(Nes::User::File&) const;
			void SaveImageData(Nes::User::File&) const;
			void SaveDiskData(Nes::User::File&) const;
			void LoadDiskData(Nes::User::File&) const;
			void LoadSampleData(wcstring,Nes::User::File&) const;

			State state;
			EventHandler events;
			Netplay netplay;
			Settings settings;

		public:

			bool Resume()
			{
				return state.running || Start();
			}

			bool Running() const
			{
				return state.running;
			}

			bool Paused() const
			{
				return state.paused;
			}

			bool Speeding() const
			{
				return settings.timing.speeding;
			}

			bool Rewinding() const
			{
				return settings.timing.rewinding;
			}

			void AskBeforeSaving()
			{
				settings.askSave = true;
			}

			DiskImageSaveMethod GetDiskImageSaveMethod() const
			{
				return settings.fds.save;
			}

			void SetDiskImageSaveMethod(DiskImageSaveMethod method)
			{
				settings.fds.save = method;
			}

			void WriteProtectCartridge(bool state)
			{
				settings.cartridge.writeProtect = state;
			}

			bool CartridgeWriteProtected() const
			{
				return settings.cartridge.writeProtect;
			}

			const Path& GetStartPath() const
			{
				return settings.paths.start;
			}

			const Path& GetImagePath() const
			{
				return settings.paths.image;
			}

			const Path& GetSavePath() const
			{
				return settings.paths.save;
			}

			bool SyncFrameRate() const
			{
				return settings.timing.sync;
			}

			bool UseTripleBuffering() const
			{
				return settings.timing.tripleBuffering;
			}

			uint GetPlayer() const
			{
				return netplay.player;
			}

			uint NetPlayers() const
			{
				return netplay.players;
			}

			EventHandler& Events()
			{
				return events;
			}

			template<typename Data,typename Activator,typename Inactivator>
			void Hook(Data data,Activator activator,Inactivator inactivator)
			{
				state.activator.Set( data, activator );
				state.inactivator.Set( data, inactivator );
			}

			template<typename Data,typename Executor,typename Commander>
			void StartNetplay(Data data,Executor executor,Commander commander,uint player,uint players)
			{
				StartNetplay( Netplay::Executor(data,executor), Netplay::Commander(data,commander), player, players );
			}
		};
	}
}

#endif
