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

#include "NstManager.hpp"
#include "NstManagerRecentDirs.hpp"

namespace Nestopia
{
	namespace Managers
	{
		NST_COMPILE_ASSERT
		(
			IDM_FILE_RECENT_DIR_2 == IDM_FILE_RECENT_DIR_1 + 1 &&
			IDM_FILE_RECENT_DIR_3 == IDM_FILE_RECENT_DIR_1 + 2 &&
			IDM_FILE_RECENT_DIR_4 == IDM_FILE_RECENT_DIR_1 + 3 &&
			IDM_FILE_RECENT_DIR_5 == IDM_FILE_RECENT_DIR_1 + 4
		);

		RecentDirs::RecentDirs(Emulator& e,const Configuration& cfg,Window::Menu& m)
		: Manager (e,m,this,&RecentDirs::OnEmuEvent)
		{
			static const Window::Menu::CmdHandler::Entry<RecentDirs> commands[] =
			{
				{ IDM_FILE_RECENT_DIR_1,     &RecentDirs::OnMenu  },
				{ IDM_FILE_RECENT_DIR_2,     &RecentDirs::OnMenu  },
				{ IDM_FILE_RECENT_DIR_3,     &RecentDirs::OnMenu  },
				{ IDM_FILE_RECENT_DIR_4,     &RecentDirs::OnMenu  },
				{ IDM_FILE_RECENT_DIR_5,     &RecentDirs::OnMenu  },
				{ IDM_FILE_RECENT_DIR_LOCK,  &RecentDirs::OnLock  },
				{ IDM_FILE_RECENT_DIR_CLEAR, &RecentDirs::OnClear }
			};

			menu.Commands().Add( this, commands );

			Configuration::ConstSection directories( cfg["paths"]["recent"]["directories"] );

			menu[IDM_FILE_RECENT_DIR_LOCK].Check( directories["locked"].Yes() );

			uint count = 0;

			Path dir;

			for (uint i=0; i < MAX_DIRS; ++i)
			{
				dir = directories["directory"][i].Str();

				if (dir.Length())
				{
					dir.MakePretty();
					dir.Insert( 0, "&x ", 3 );
					dir[1] = '1' + count;

					menu[IDM_FILE_RECENT_DIR_1 + count++].Text() << dir;
				}
			}

			menu[IDM_FILE_RECENT_DIR_CLEAR].Enable( count );

			for (count += IDM_FILE_RECENT_DIR_1; count <= IDM_FILE_RECENT_DIR_5; ++count)
				menu[count].Remove();
		}

		void RecentDirs::Save(Configuration& cfg) const
		{
			Configuration::Section directories( cfg["paths"]["recent"]["directories"] );

			directories["locked"].YesNo() = menu[IDM_FILE_RECENT_DIR_LOCK].Checked();

			HeapString dir;

			for (uint i=0; i < MAX_DIRS && menu[IDM_FILE_RECENT_DIR_1 + i].Text() >> dir; ++i)
				directories["directory"][i].Str() = dir(3);
		}

		void RecentDirs::OnMenu(uint cmd)
		{
			HeapString dir;

			if ((menu[cmd].Text() >> dir) > 3)
				Application::Instance::GetMainWindow().Send( Application::Instance::WM_NST_LAUNCH, 0, dir(3).Ptr() );
		}

		void RecentDirs::OnLock(uint)
		{
			menu[IDM_FILE_RECENT_DIR_LOCK].ToggleCheck();
		}

		void RecentDirs::OnClear(uint)
		{
			for (uint i=IDM_FILE_RECENT_DIR_1; i <= IDM_FILE_RECENT_DIR_5; ++i)
				menu[i].Remove();

			menu[IDM_FILE_RECENT_DIR_CLEAR].Disable();
		}

		void RecentDirs::Add(const uint idm,const HeapString& name) const
		{
			if (menu[idm].Exists())
			{
				menu[idm].Text() << name;
			}
			else
			{
				Window::Menu::Item item( menu[IDM_POS_FILE][IDM_POS_FILE_RECENTDIRS] );
				menu.Insert( item[item.NumItems() - 3], idm, name );
			}
		}

		void RecentDirs::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_LOAD:

					if (menu[IDM_FILE_RECENT_DIR_LOCK].Unchecked())
					{
						HeapString items[MAX_DIRS];
						HeapString curDir( emulator.GetStartPath().Directory() );

						for (uint i=IDM_FILE_RECENT_DIR_1, j=0; i <= IDM_FILE_RECENT_DIR_5 && menu[i].Text() >> items[j]; ++i)
						{
							if (items[j](3) != curDir)
								items[j][1] = ('2' + j), ++j;
						}

						curDir.Insert( 0, "&1 ", 3 );
						Add( IDM_FILE_RECENT_DIR_1, curDir );

						for (uint i=0; i < MAX_DIRS-1 && items[i].Length(); ++i)
							Add( IDM_FILE_RECENT_DIR_2 + i, items[i] );

						menu[IDM_FILE_RECENT_DIR_CLEAR].Enable();
					}
					break;

				case Emulator::EVENT_NETPLAY_MODE:

					menu[IDM_POS_FILE][IDM_POS_FILE_RECENTDIRS].Enable( !data );
					break;
			}
		}
	}
}
