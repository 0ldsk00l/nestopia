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
#include "NstObjectHeap.hpp"
#include "NstIoScreen.hpp"
#include "NstWindowUser.hpp"
#include "NstSystemKeyboard.hpp"
#include "NstManager.hpp"
#include "NstManagerFds.hpp"
#include "NstDialogFds.hpp"

namespace Nestopia
{
	namespace Managers
	{
		NST_COMPILE_ASSERT
		(
			IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_B == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A +  1 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_2_SIDE_A == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A +  2 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_2_SIDE_B == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A +  3 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_3_SIDE_A == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A +  4 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_3_SIDE_B == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A +  5 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_4_SIDE_A == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A +  6 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_4_SIDE_B == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A +  7 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_5_SIDE_A == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A +  8 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_5_SIDE_B == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A +  9 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_6_SIDE_A == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + 10 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_6_SIDE_B == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + 11 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_7_SIDE_A == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + 12 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_7_SIDE_B == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + 13 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_8_SIDE_A == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + 14 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_8_SIDE_B == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + 15
		);

		struct Fds::Callbacks
		{
			NST_COMPILE_ASSERT( Nes::Fds::NUM_DRIVE_CALLBACKS == 3 );

			static void NST_CALLBACK OnDiskAccessNumLock(Nes::Fds::UserData,Nes::Fds::Motor motor)
			{
				System::Keyboard::ToggleIndicator( System::Keyboard::NUM_LOCK, motor );
			}

			static void NST_CALLBACK OnDiskAccessScrollLock(Nes::Fds::UserData,Nes::Fds::Motor motor)
			{
				System::Keyboard::ToggleIndicator( System::Keyboard::SCROLL_LOCK, motor );
			}

			static void NST_CALLBACK OnDiskAccessCapsLock(Nes::Fds::UserData,Nes::Fds::Motor motor)
			{
				System::Keyboard::ToggleIndicator( System::Keyboard::CAPS_LOCK, motor );
			}

			static void NST_CALLBACK OnDiskAccessScreen(Nes::Fds::UserData,Nes::Fds::Motor motor)
			{
				Io::Screen(15000) << (motor != Nes::Fds::MOTOR_OFF ? Resource::String(motor == Nes::Fds::MOTOR_READ ? IDS_SCREEN_FDS_READING : IDS_SCREEN_FDS_WRITING).Ptr() : L"");
			}
		};

		Fds::Fds(Emulator& e,const Configuration& cfg,Window::Menu& m,const Paths& paths)
		:
		Manager  ( e, m, this, &Fds::OnEmuEvent ),
		dialog   ( new Window::Fds(e,cfg,paths) )
		{
			static const Window::Menu::CmdHandler::Entry<Fds> commands[] =
			{
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_B, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_2_SIDE_A, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_2_SIDE_B, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_3_SIDE_A, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_3_SIDE_B, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_4_SIDE_A, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_4_SIDE_B, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_5_SIDE_A, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_5_SIDE_B, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_6_SIDE_A, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_6_SIDE_B, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_7_SIDE_A, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_7_SIDE_B, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_8_SIDE_A, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_8_SIDE_B, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_CHANGE_SIDE,          &Fds::OnCmdChangeSide },
				{ IDM_MACHINE_EXT_FDS_EJECT_DISK,           &Fds::OnCmdEjectDisk  },
				{ IDM_MACHINE_EXT_FDS_OPTIONS,              &Fds::OnCmdOptions    }
			};

			menu.Commands().Add( this, commands );

			static const Window::Menu::PopupHandler::Entry<Fds> popups[] =
			{
				{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_EXT,IDM_POS_MACHINE_EXT_FDS>::ID,                                &Fds::OnMenuExtFds       },
				{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_EXT,IDM_POS_MACHINE_EXT_FDS,IDM_POS_MACHINE_EXT_FDS_INSERT>::ID, &Fds::OnMenuExtFdsInsert }
			};

			menu.Popups().Add( this, popups );

			UpdateSettings();
			UpdateMenuDisks();
		}

		Fds::~Fds()
		{
			Nes::Fds::driveCallback.Unset();
		}

		void Fds::Save(Configuration& cfg) const
		{
			dialog->Save( cfg );
		}

		void Fds::UpdateSettings() const
		{
			const Window::Fds::Led led = dialog->GetLed();

			Nes::Fds::driveCallback.Set
			(
				led == Window::Fds::LED_SCREEN      ? Callbacks::OnDiskAccessScreen :
				led == Window::Fds::LED_NUM_LOCK    ? Callbacks::OnDiskAccessNumLock :
				led == Window::Fds::LED_CAPS_LOCK   ? Callbacks::OnDiskAccessCapsLock :
				led == Window::Fds::LED_SCROLL_LOCK ? Callbacks::OnDiskAccessScrollLock :
                                                      NULL,
				NULL
			);
		}

		void Fds::UpdateMenuDisks() const
		{
			const Window::Menu::Item subMenu
			(
				menu[IDM_POS_MACHINE][IDM_POS_MACHINE_EXT][IDM_POS_MACHINE_EXT_FDS][IDM_POS_MACHINE_EXT_FDS_INSERT]
			);

			subMenu.Enable( emulator.IsFds() );
			subMenu.Clear();

			if (uint numSides = Nes::Fds(emulator).GetNumSides())
			{
				NST_VERIFY( numSides <= MAX_SIDES );

				if (numSides > MAX_SIDES)
					numSides = MAX_SIDES;

				for (uint i=0; i < numSides; ++i)
				{
					menu.Insert
					(
						subMenu[i],
						IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + i,
						Resource::String( (i % 2) ? IDS_FDS_DISK_SIDE_B : IDS_FDS_DISK_SIDE_A ).Invoke( wchar_t('1' + i / 2) )
					);
				}
			}
		}

		bool Fds::CanInsertDisk() const
		{
			return
			(
				emulator.IsFds() &&
				!emulator.IsLocked() &&
				emulator.GetPlayer() == Emulator::MASTER
			);
		}

		bool Fds::CanEjectDisk() const
		{
			return
			(
				Nes::Fds(emulator).IsAnyDiskInserted() &&
				!emulator.IsLocked() &&
				emulator.GetPlayer() == Emulator::MASTER
			);
		}

		bool Fds::CanChangeSide() const
		{
			return
			(
				Nes::Fds(emulator).CanChangeDiskSide() &&
				!emulator.IsLocked() &&
				emulator.GetPlayer() == Emulator::MASTER
			);
		}

		void Fds::OnMenuExtFds(const Window::Menu::PopupHandler::Param& param)
		{
			param.menu[ IDM_MACHINE_EXT_FDS_CHANGE_SIDE ].Enable( !param.show || CanChangeSide() );
			param.menu[ IDM_MACHINE_EXT_FDS_EJECT_DISK  ].Enable( !param.show || CanEjectDisk()  );
		}

		void Fds::OnMenuExtFdsInsert(const Window::Menu::PopupHandler::Param& param)
		{
			if (uint numSides = Nes::Fds(emulator).GetNumSides())
			{
				if (numSides > MAX_SIDES)
					numSides = MAX_SIDES;

				const bool checked = (param.show && Nes::Fds(emulator).IsAnyDiskInserted());

				param.menu[IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + (checked ? Nes::Fds(emulator).GetCurrentDisk() * 2 + Nes::Fds(emulator).GetCurrentDiskSide() : 0)].Check
				(
					IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A,
					IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + numSides,
					checked
				);

				const bool enabled = (!checked || CanInsertDisk());

				do
				{
					param.menu[IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + --numSides].Enable( enabled );
				}
				while (numSides);
			}
		}

		void Fds::OnCmdInsertDisk(uint disk)
		{
			if (CanInsertDisk())
			{
				emulator.SendCommand( Emulator::COMMAND_DISK_INSERT, disk - IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A );
				Resume();
			}
		}

		void Fds::OnCmdChangeSide(uint)
		{
			if (CanChangeSide())
			{
				emulator.SendCommand( Emulator::COMMAND_DISK_INSERT, Nes::Fds(emulator).GetCurrentDisk() * 2 + (Nes::Fds(emulator).GetCurrentDiskSide() == 0) );
				Resume();
			}
		}

		void Fds::OnCmdEjectDisk(uint)
		{
			if (CanEjectDisk())
			{
				emulator.SendCommand( Emulator::COMMAND_DISK_EJECT );
				Resume();
			}
		}

		void Fds::OnCmdOptions(uint)
		{
			dialog->Open();
			UpdateSettings();
		}

		void Fds::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_DISK_INSERT:
				case Emulator::EVENT_DISK_EJECT:

					Io::Screen() << Resource::String( event == Emulator::EVENT_DISK_EJECT ? IDS_SCREEN_DISK_EJECTED : (data % 2U ? IDS_SCREEN_DISK_SIDE_B_INSERTED : IDS_SCREEN_DISK_SIDE_A_INSERTED) ).Invoke( wchar_t('1' + data / 2U) );
					break;

				case Emulator::EVENT_LOAD:
				case Emulator::EVENT_UNLOAD:

					UpdateMenuDisks();

					if (emulator.IsFds())
						emulator.SendCommand( Emulator::COMMAND_DISK_INSERT );

					break;

				case Emulator::EVENT_DISK_NONSTANDARD:

					Window::User::Warn( IDS_FDS_NONSTANDARD );
					break;

				case Emulator::EVENT_DISK_QUERY_BIOS:

					dialog->QueryBiosFile();
					break;

				case Emulator::EVENT_NETPLAY_MODE:

					menu[IDM_MACHINE_EXT_FDS_OPTIONS].Enable( !data );
					break;
			}
		}
	}
}
