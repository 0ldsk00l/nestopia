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

#include <algorithm>
#include "NstResourceString.hpp"
#include "NstIoScreen.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogAutoSaver.hpp"
#include "NstManagerSaveStates.hpp"
#include "NstWindowMain.hpp"

namespace Nestopia
{
	namespace Managers
	{
		NST_COMPILE_ASSERT
		(
			IDM_FILE_QUICK_SAVE_STATE_SLOT_1 == IDM_FILE_QUICK_SAVE_STATE_SLOT_OLDEST + 1 &&
			IDM_FILE_QUICK_SAVE_STATE_SLOT_2 == IDM_FILE_QUICK_SAVE_STATE_SLOT_OLDEST + 2 &&
			IDM_FILE_QUICK_SAVE_STATE_SLOT_3 == IDM_FILE_QUICK_SAVE_STATE_SLOT_OLDEST + 3 &&
			IDM_FILE_QUICK_SAVE_STATE_SLOT_4 == IDM_FILE_QUICK_SAVE_STATE_SLOT_OLDEST + 4 &&
			IDM_FILE_QUICK_SAVE_STATE_SLOT_5 == IDM_FILE_QUICK_SAVE_STATE_SLOT_OLDEST + 5 &&
			IDM_FILE_QUICK_SAVE_STATE_SLOT_6 == IDM_FILE_QUICK_SAVE_STATE_SLOT_OLDEST + 6 &&
			IDM_FILE_QUICK_SAVE_STATE_SLOT_7 == IDM_FILE_QUICK_SAVE_STATE_SLOT_OLDEST + 7 &&
			IDM_FILE_QUICK_SAVE_STATE_SLOT_8 == IDM_FILE_QUICK_SAVE_STATE_SLOT_OLDEST + 8 &&
			IDM_FILE_QUICK_SAVE_STATE_SLOT_9 == IDM_FILE_QUICK_SAVE_STATE_SLOT_OLDEST + 9 &&

			IDM_FILE_QUICK_LOAD_STATE_SLOT_1 == IDM_FILE_QUICK_LOAD_STATE_SLOT_NEWEST + 1 &&
			IDM_FILE_QUICK_LOAD_STATE_SLOT_2 == IDM_FILE_QUICK_LOAD_STATE_SLOT_NEWEST + 2 &&
			IDM_FILE_QUICK_LOAD_STATE_SLOT_3 == IDM_FILE_QUICK_LOAD_STATE_SLOT_NEWEST + 3 &&
			IDM_FILE_QUICK_LOAD_STATE_SLOT_4 == IDM_FILE_QUICK_LOAD_STATE_SLOT_NEWEST + 4 &&
			IDM_FILE_QUICK_LOAD_STATE_SLOT_5 == IDM_FILE_QUICK_LOAD_STATE_SLOT_NEWEST + 5 &&
			IDM_FILE_QUICK_LOAD_STATE_SLOT_6 == IDM_FILE_QUICK_LOAD_STATE_SLOT_NEWEST + 6 &&
			IDM_FILE_QUICK_LOAD_STATE_SLOT_7 == IDM_FILE_QUICK_LOAD_STATE_SLOT_NEWEST + 7 &&
			IDM_FILE_QUICK_LOAD_STATE_SLOT_8 == IDM_FILE_QUICK_LOAD_STATE_SLOT_NEWEST + 8 &&
			IDM_FILE_QUICK_LOAD_STATE_SLOT_9 == IDM_FILE_QUICK_LOAD_STATE_SLOT_NEWEST + 9
		);

		struct SaveStates::Slot::Compare
		{
			bool operator () (const Slot& a,const Slot& b) const
			{
				return a.time == b.time ? &a < &b : a.time < b.time;
			}
		};

		SaveStates::SaveStates(Emulator& e,Window::Menu& m,const Paths& p,const Window::Main& w)
		:
		Manager         ( e, m, this, &SaveStates::OnEmuEvent ),
		window          ( w ),
		paths           ( p ),
		autoSaveEnabled ( false ),
		autoSaver       ( new Window::AutoSaver(paths) )
		{
			static const Window::Menu::CmdHandler::Entry<SaveStates> commands[] =
			{
				{ IDM_FILE_LOAD_NST,                     &SaveStates::OnCmdStateLoad        },
				{ IDM_FILE_SAVE_NST,                     &SaveStates::OnCmdStateSave        },
				{ IDM_FILE_QUICK_LOAD_STATE_SLOT_NEWEST, &SaveStates::OnCmdSlotLoadNewest   },
				{ IDM_FILE_QUICK_LOAD_STATE_SLOT_1,      &SaveStates::OnCmdSlotLoad         },
				{ IDM_FILE_QUICK_LOAD_STATE_SLOT_2,      &SaveStates::OnCmdSlotLoad         },
				{ IDM_FILE_QUICK_LOAD_STATE_SLOT_3,      &SaveStates::OnCmdSlotLoad         },
				{ IDM_FILE_QUICK_LOAD_STATE_SLOT_4,      &SaveStates::OnCmdSlotLoad         },
				{ IDM_FILE_QUICK_LOAD_STATE_SLOT_5,      &SaveStates::OnCmdSlotLoad         },
				{ IDM_FILE_QUICK_LOAD_STATE_SLOT_6,      &SaveStates::OnCmdSlotLoad         },
				{ IDM_FILE_QUICK_LOAD_STATE_SLOT_7,      &SaveStates::OnCmdSlotLoad         },
				{ IDM_FILE_QUICK_LOAD_STATE_SLOT_8,      &SaveStates::OnCmdSlotLoad         },
				{ IDM_FILE_QUICK_LOAD_STATE_SLOT_9,      &SaveStates::OnCmdSlotLoad         },
				{ IDM_FILE_QUICK_SAVE_STATE_SLOT_OLDEST, &SaveStates::OnCmdSlotSaveOldest   },
				{ IDM_FILE_QUICK_SAVE_STATE_SLOT_1,      &SaveStates::OnCmdSlotSave         },
				{ IDM_FILE_QUICK_SAVE_STATE_SLOT_2,      &SaveStates::OnCmdSlotSave         },
				{ IDM_FILE_QUICK_SAVE_STATE_SLOT_3,      &SaveStates::OnCmdSlotSave         },
				{ IDM_FILE_QUICK_SAVE_STATE_SLOT_4,      &SaveStates::OnCmdSlotSave         },
				{ IDM_FILE_QUICK_SAVE_STATE_SLOT_5,      &SaveStates::OnCmdSlotSave         },
				{ IDM_FILE_QUICK_SAVE_STATE_SLOT_6,      &SaveStates::OnCmdSlotSave         },
				{ IDM_FILE_QUICK_SAVE_STATE_SLOT_7,      &SaveStates::OnCmdSlotSave         },
				{ IDM_FILE_QUICK_SAVE_STATE_SLOT_8,      &SaveStates::OnCmdSlotSave         },
				{ IDM_FILE_QUICK_SAVE_STATE_SLOT_9,      &SaveStates::OnCmdSlotSave         },
				{ IDM_OPTIONS_AUTOSAVER,                 &SaveStates::OnCmdAutoSaverOptions },
				{ IDM_OPTIONS_AUTOSAVER_START,           &SaveStates::OnCmdAutoSaverStart   }
			};

			menu.Commands().Add( this, commands );

			menu[IDM_FILE_LOAD_NST].Disable();
			menu[IDM_FILE_SAVE_NST].Disable();
			menu[IDM_FILE_QUICK_LOAD_STATE_SLOT_NEWEST].Disable();
			menu[IDM_FILE_QUICK_SAVE_STATE_SLOT_OLDEST].Disable();

			for (uint i=0; i < NUM_SLOTS; ++i)
			{
				menu[IDM_FILE_QUICK_LOAD_STATE_SLOT_1 + i].Disable();
				menu[IDM_FILE_QUICK_SAVE_STATE_SLOT_1 + i].Disable();
			}

			menu[IDM_POS_FILE][IDM_POS_FILE_QUICKLOADSTATE].Disable();
			menu[IDM_POS_FILE][IDM_POS_FILE_QUICKSAVESTATE].Disable();
			menu[IDM_OPTIONS_AUTOSAVER_START].Disable();

			ToggleAutoSaver( false );

			UpdateMenuTexts();
		}

		SaveStates::~SaveStates()
		{
			ToggleAutoSaver( false );
		}

		void SaveStates::UpdateMenuTexts() const
		{
			bool useSeconds = false;

			for (const Slot* a=slots; a != slots+NUM_SLOTS-1; ++a)
			{
				if (a->data.Size())
				{
					for (const Slot* b=a+1; b != slots+NUM_SLOTS; ++b)
					{
						if (b->data.Size() && a->time.Almost( b->time ))
						{
							useSeconds = true;
							a = slots+NUM_SLOTS-2;
							break;
						}
					}
				}
			}

			HeapString string( "&1  ..." );

			for (uint i=0; i < NUM_SLOTS; ++i)
			{
				string[1] = '1' + i;

				if (slots[i].data.Size())
				{
					string.ShrinkTo( 4 );
					string << slots[i].time.ToString( useSeconds );
				}
				else
				{
					string.ShrinkTo( 7 );
					string[4] = '.';
					string[5] = '.';
					string[6] = '.';
				}

				menu[ IDM_FILE_QUICK_LOAD_STATE_SLOT_1 + i ].Text() << string;
				menu[ IDM_FILE_QUICK_SAVE_STATE_SLOT_1 + i ].Text() << string;
			}
		}

		void SaveStates::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_POWER_ON:
				case Emulator::EVENT_POWER_OFF:

					if (emulator.IsGame())
					{
						const bool on = (event == Emulator::EVENT_POWER_ON);
						const bool single = (on && emulator.NetPlayers() == 0);

						menu[ IDM_FILE_SAVE_NST ].Enable( single );
						menu[ IDM_POS_FILE ][ IDM_POS_FILE_QUICKSAVESTATE ].Enable( on );
						menu[ IDM_OPTIONS_AUTOSAVER_START ].Enable( single );

						for (uint i=IDM_FILE_QUICK_SAVE_STATE_SLOT_OLDEST; i <= IDM_FILE_QUICK_SAVE_STATE_SLOT_9; ++i)
							menu[i].Enable( on );

						ToggleAutoSaver( false );
					}
					break;

				case Emulator::EVENT_LOAD:

					if (emulator.IsGame())
					{
						const bool single = (emulator.NetPlayers() == 0);

						menu[ IDM_FILE_LOAD_NST ].Enable( single );
						menu[ IDM_POS_FILE ][ IDM_POS_FILE_QUICKLOADSTATE ].Enable( single );

						if (paths.SaveSlotImportingEnabled())
							ImportSlots( single );
					}
					break;

				case Emulator::EVENT_UNLOAD:

					menu[ IDM_FILE_LOAD_NST ].Disable();
					menu[ IDM_POS_FILE ][ IDM_POS_FILE_QUICKLOADSTATE ].Disable();
					menu[ IDM_FILE_QUICK_LOAD_STATE_SLOT_NEWEST ].Disable();

					for (uint i=0; i < NUM_SLOTS; ++i)
					{
						menu[ IDM_FILE_QUICK_LOAD_STATE_SLOT_1 + i ].Disable();
						slots[i].time.Clear();
						slots[i].data.Destroy();
					}

					UpdateMenuTexts();
					break;

				case Emulator::EVENT_MOVIE_PLAYING:
				case Emulator::EVENT_MOVIE_PLAYING_STOPPED:
				{
					const bool notPlaying = (event != Emulator::EVENT_MOVIE_PLAYING);

					menu[ IDM_FILE_LOAD_NST ].Enable( notPlaying );
					menu[ IDM_POS_FILE ][ IDM_POS_FILE_QUICKLOADSTATE ].Enable( notPlaying );
					break;
				}

				case Emulator::EVENT_NETPLAY_MODE:

					menu[IDM_POS_OPTIONS][IDM_POS_OPTIONS_AUTOSAVER].Enable( !data );
					break;
			}
		}

		void SaveStates::SaveToSlot(const uint index,const bool notify)
		{
			if (emulator.SaveState( slots[index].data, paths.UseStateCompression(), Emulator::STICKY ))
			{
				if (emulator.NetPlayers() == 0)
				{
					menu[ IDM_FILE_QUICK_LOAD_STATE_SLOT_1 + index ].Enable();
					menu[ IDM_FILE_QUICK_LOAD_STATE_SLOT_NEWEST ].Enable();
				}

				slots[index].time.Set();

				if (paths.SaveSlotExportingEnabled())
					ExportSlot( index );

				if (notify)
					Io::Screen() << Resource::String( IDS_SCREEN_SAVE_STATE_TO_SLOT ).Invoke( wchar_t('1'+index) );
			}
			else
			{
				slots[index].time.Clear();
				slots[index].data.Destroy();
			}

			UpdateMenuTexts();
		}

		void SaveStates::LoadFromSlot(const uint index,const bool notify)
		{
			if (emulator.LoadState( slots[index].data, Emulator::STICKY ))
			{
				if (notify)
					Io::Screen() << Resource::String( IDS_SCREEN_LOAD_STATE_FROM_SLOT ).Invoke( wchar_t('1'+index) );
			}
		}

		void SaveStates::Load(Collection::Buffer& data,const GenericString name) const
		{
			if (emulator.LoadState( data ))
			{
				const uint length = window.GetMaxMessageLength();

				if (name.Length() && length > 20)
					Io::Screen() << Resource::String( IDS_SCREEN_LOAD_STATE_FROM ).Invoke( Path::Compact( name, length - 18 ) );
			}
		}

		void SaveStates::ExportSlot(const uint index)
		{
			Path path( emulator.GetImagePath().Target().File() );
			NST_ASSERT( slots[index].data.Size() && path.Length() );

			path.Extension() = L"ns1";
			path.Back() = '1' + index;

			paths.Save( slots[index].data.Ptr(), slots[index].data.Size(), Paths::File::SLOTS, path );
		}

		void SaveStates::ImportSlots(const bool canLoad)
		{
			bool anyLoaded = false;

			Path path( emulator.GetImagePath().Target().File() );
			NST_ASSERT( path.Length() );

			path.Extension() = L"ns1";

			Paths::File file;

			for (uint i=0; i < NUM_SLOTS; ++i)
			{
				path.Back() = '1' + i;

				if (paths.Load( file, Paths::File::SLOTS, path, Paths::QUIETLY ))
				{
					anyLoaded = true;

					slots[i].data.Import( file.data );
					slots[i].time.Set( file.name.Ptr() );

					if (canLoad)
					{
						menu[ IDM_FILE_QUICK_LOAD_STATE_SLOT_1 + i ].Enable();
						menu[ IDM_FILE_QUICK_LOAD_STATE_SLOT_NEWEST ].Enable();
					}
				}
			}

			if (anyLoaded)
				UpdateMenuTexts();
		}

		void SaveStates::OnCmdStateLoad(uint)
		{
			Paths::File file;

			if (paths.Load( file, Paths::File::STATE|Paths::File::SLOTS|Paths::File::ARCHIVE ) && emulator.LoadState( file.data ))
			{
				const uint length = window.GetMaxMessageLength();

				if (length > 20)
					Io::Screen() << Resource::String( IDS_SCREEN_LOAD_STATE_FROM ).Invoke( Path::Compact( file.name, length - 18 ) );
			}
		}

		void SaveStates::OnCmdStateSave(uint)
		{
			const Path path( paths.BrowseSave( Paths::File::STATE, Paths::SUGGEST ) );

			if (path.Length())
			{
				Collection::Buffer buffer;

				if (emulator.SaveState( buffer, paths.UseStateCompression() ) && paths.Save( buffer.Ptr(), buffer.Size(), Paths::File::STATE, path ))
				{
					const uint length = window.GetMaxMessageLength();

					if (length > 20)
						Io::Screen() << Resource::String( IDS_SCREEN_SAVE_STATE_TO ).Invoke( Path::Compact( path, length - 18 ) );
				}
			}
		}

		void SaveStates::OnCmdSlotSave(uint id)
		{
			SaveToSlot( id - IDM_FILE_QUICK_SAVE_STATE_SLOT_1 );
			Resume();
		}

		void SaveStates::OnCmdSlotSaveOldest(uint)
		{
			SaveToSlot( std::min_element( slots, slots + NUM_SLOTS, Slot::Compare() ) - slots );
			Resume();
		}

		void SaveStates::OnCmdSlotLoad(uint id)
		{
			LoadFromSlot( id - IDM_FILE_QUICK_LOAD_STATE_SLOT_1 );
			Resume();
		}

		void SaveStates::OnCmdSlotLoadNewest(uint)
		{
			LoadFromSlot( std::max_element( slots, slots + NUM_SLOTS, Slot::Compare() ) - slots );
			Resume();
		}

		void SaveStates::OnCmdAutoSaverOptions(uint)
		{
			autoSaver->Open();
			ToggleAutoSaver( false );
		}

		void SaveStates::OnCmdAutoSaverStart(uint)
		{
			ToggleAutoSaver( !autoSaveEnabled );
			Resume();
		}

		uint SaveStates::OnTimerAutoSave()
		{
			if (autoSaveEnabled)
			{
				const Path stateFile( autoSaver->GetStateFile() );

				if (stateFile.Length())
				{
					Collection::Buffer buffer;

					if
					(
						emulator.SaveState( buffer, paths.UseStateCompression(), Emulator::STICKY ) &&
						paths.Save( buffer.Ptr(), buffer.Size(), Paths::File::STATE, stateFile, Paths::STICKY ) &&
						autoSaver->ShouldNotify()
					)
					{
						const uint length = window.GetMaxMessageLength();

						if (length > 20)
							Io::Screen() << Resource::String( IDS_SCREEN_SAVE_STATE_TO ).Invoke( Path::Compact( stateFile, length - 18 ) );
					}
				}
				else
				{
					SaveToSlot( Window::AutoSaver::DEFAULT_SAVE_SLOT, autoSaver->ShouldNotify() );
				}
			}

			return autoSaveEnabled;
		}

		void SaveStates::ToggleAutoSaver(const bool enable)
		{
			menu[ IDM_OPTIONS_AUTOSAVER_START ].Text() << Resource::String(enable ? IDS_TEXT_STOP : IDS_TEXT_START);

			if (autoSaveEnabled != enable)
			{
				autoSaveEnabled = enable;
				Io::Screen() << Resource::String( enable ? IDS_AUTOSAVER_START : IDS_AUTOSAVER_STOP );
			}

			if (enable)
				window.Get().StartTimer( this, &SaveStates::OnTimerAutoSave, autoSaver->GetInterval() );
			else
				window.Get().StopTimer( this, &SaveStates::OnTimerAutoSave );
		}
	}
}
