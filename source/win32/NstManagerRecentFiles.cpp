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
#include "NstManagerRecentFiles.hpp"

namespace Nestopia
{
	namespace Managers
	{
		NST_COMPILE_ASSERT
		(
			IDM_FILE_RECENT_2 == IDM_FILE_RECENT_1 + 1 &&
			IDM_FILE_RECENT_3 == IDM_FILE_RECENT_1 + 2 &&
			IDM_FILE_RECENT_4 == IDM_FILE_RECENT_1 + 3 &&
			IDM_FILE_RECENT_5 == IDM_FILE_RECENT_1 + 4 &&
			IDM_FILE_RECENT_6 == IDM_FILE_RECENT_1 + 5 &&
			IDM_FILE_RECENT_7 == IDM_FILE_RECENT_1 + 6 &&
			IDM_FILE_RECENT_8 == IDM_FILE_RECENT_1 + 7 &&
			IDM_FILE_RECENT_9 == IDM_FILE_RECENT_1 + 8
		);

		RecentFiles::RecentFiles(Emulator& e,const Configuration& cfg,Window::Menu& m)
		: Manager(e,m,this,&RecentFiles::OnEmuEvent)
		{
			static const Window::Menu::CmdHandler::Entry<RecentFiles> commands[] =
			{
				{ IDM_FILE_RECENT_1,     &RecentFiles::OnMenu  },
				{ IDM_FILE_RECENT_2,     &RecentFiles::OnMenu  },
				{ IDM_FILE_RECENT_3,     &RecentFiles::OnMenu  },
				{ IDM_FILE_RECENT_4,     &RecentFiles::OnMenu  },
				{ IDM_FILE_RECENT_5,     &RecentFiles::OnMenu  },
				{ IDM_FILE_RECENT_6,     &RecentFiles::OnMenu  },
				{ IDM_FILE_RECENT_7,     &RecentFiles::OnMenu  },
				{ IDM_FILE_RECENT_8,     &RecentFiles::OnMenu  },
				{ IDM_FILE_RECENT_9,     &RecentFiles::OnMenu  },
				{ IDM_FILE_RECENT_LOCK,  &RecentFiles::OnLock  },
				{ IDM_FILE_RECENT_CLEAR, &RecentFiles::OnClear }
			};

			menu.Commands().Add( this, commands );

			Configuration::ConstSection files( cfg["paths"]["recent"]["files"] );

			menu[IDM_FILE_RECENT_LOCK].Check( files["locked"].Yes() );

			uint count = 0;

			Path path;

			for (uint i=0; i < MAX_FILES; ++i)
			{
				path = files["file"][i].Str();

				if (path.Length())
				{
					path.MakePretty();
					path.Insert( 0, "&x ", 3 );
					path[1] = '1' + count;

					menu[IDM_FILE_RECENT_1 + count++].Text() << path;
				}
			}

			menu[IDM_FILE_RECENT_CLEAR].Enable( count );

			for (count += IDM_FILE_RECENT_1; count <= IDM_FILE_RECENT_9; ++count)
				menu[count].Remove();
		}

		void RecentFiles::Save(Configuration& cfg) const
		{
			Configuration::Section files( cfg["paths"]["recent"]["files"] );

			files["locked"].YesNo() = menu[IDM_FILE_RECENT_LOCK].Checked();

			HeapString path;

			for (uint i=0; i < MAX_FILES && menu[IDM_FILE_RECENT_1 + i].Text() >> path; ++i)
				files["file"][i].Str() = path(3);
		}

		void RecentFiles::OnMenu(uint cmd)
		{
			HeapString path;

			if ((menu[cmd].Text() >> path) > 3)
				Application::Instance::GetMainWindow().Send( Application::Instance::WM_NST_LAUNCH, 0, path(3).Ptr() );
		}

		void RecentFiles::OnLock(uint)
		{
			menu[IDM_FILE_RECENT_LOCK].ToggleCheck();
		}

		void RecentFiles::OnClear(uint)
		{
			menu[IDM_FILE_RECENT_CLEAR].Disable();

			for (uint i=IDM_FILE_RECENT_1; i <= IDM_FILE_RECENT_9; ++i)
				menu[i].Remove();
		}

		void RecentFiles::Add(const uint idm,const HeapString& name) const
		{
			if (menu[idm].Exists())
			{
				menu[idm].Text() << name;
			}
			else
			{
				Window::Menu::Item item( menu[IDM_POS_FILE][IDM_POS_FILE_RECENTFILES] );
				menu.Insert( item[item.NumItems() - 3], idm, name );
			}
		}

		void RecentFiles::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_LOAD:

					if (menu[IDM_FILE_RECENT_LOCK].Unchecked())
					{
						HeapString items[MAX_FILES];
						HeapString curPath( emulator.GetStartPath() );

						for (uint i=IDM_FILE_RECENT_1, j=0; i <= IDM_FILE_RECENT_9 && menu[i].Text() >> items[j]; ++i)
						{
							if (items[j](3) != curPath)
								items[j][1] = ('2' + j), ++j;
						}

						curPath.Insert( 0, "&1 ", 3 );
						Add( IDM_FILE_RECENT_1, curPath );

						for (uint i=0; i < MAX_FILES-1 && items[i].Length(); ++i)
							Add( IDM_FILE_RECENT_2 + i, items[i] );

						menu[IDM_FILE_RECENT_CLEAR].Enable();
					}
					break;

				case Emulator::EVENT_NETPLAY_MODE:

					menu[IDM_POS_FILE][IDM_POS_FILE_RECENTFILES].Enable( !data );
					break;
			}
		}
	}
}
