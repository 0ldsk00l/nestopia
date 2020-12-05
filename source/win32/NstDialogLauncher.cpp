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

#include "resource/resource.h"
#include "NstResourceString.hpp"
#include "NstResourceIcon.hpp"
#include "NstWindowParam.hpp"
#include "NstSystemKeyboard.hpp"
#include "NstDialogLauncher.hpp"
#include "NstDialogInesHeader.hpp"

namespace Nestopia
{
	namespace Window
	{
		struct Launcher::Handlers
		{
			static const MsgHandler::Entry<Launcher> messages[];
			static const Menu::CmdHandler::Entry<Launcher> commands[];
			static const Control::NotificationHandler::Entry<Launcher> listNotifications[];
			static const Control::NotificationHandler::Entry<Launcher> treeNotifications[];
		};

		const MsgHandler::Entry<Launcher> Launcher::Handlers::messages[] =
		{
			{ WM_INITDIALOG,  &Launcher::OnInitDialog  },
			{ WM_CLOSE,       &Launcher::OnClose       },
			{ WM_DESTROY,     &Launcher::OnDestroy     },
			{ WM_SIZE,        &Launcher::OnSize        },
			{ WM_DROPFILES,   &Launcher::OnDropFiles   }
		};

		const Menu::CmdHandler::Entry<Launcher> Launcher::Handlers::commands[] =
		{
			{ IDM_LAUNCHER_FILE_RUN,                   &Launcher::OnCmdFileRun                   },
			{ IDM_LAUNCHER_FILE_EDITHEADER,            &Launcher::OnCmdEditHeader                },
			{ IDM_LAUNCHER_FILE_REFRESH,               &Launcher::OnCmdFileRefresh               },
			{ IDM_LAUNCHER_VIEW_SHOWGRIDS,             &Launcher::OnCmdViewShowGrids             },
			{ IDM_LAUNCHER_VIEW_SHOWDATABASECORRECTED, &Launcher::OnCmdViewShowDatabaseCorrected },
			{ IDM_LAUNCHER_VIEW_ONTOP,                 &Launcher::OnCmdViewShowOnTop             },
			{ IDM_LAUNCHER_OPTIONS_COLORS,             &Launcher::OnCmdOptionsColors             },
			{ IDM_LAUNCHER_OPTIONS_PATHS,              &Launcher::OnCmdOptionsPaths              }
		};

		const Control::NotificationHandler::Entry<Launcher> Launcher::Handlers::listNotifications[] =
		{
			{ LVN_GETDISPINFO,    &Launcher::OnListGetDisplayInfo    },
			{ LVN_KEYDOWN,        &Launcher::OnListKeyDown           },
			{ LVN_COLUMNCLICK,    &Launcher::OnListColumnClick       },
			{ LVN_ITEMACTIVATE,   &Launcher::OnListItemActivate      },
			{ LVN_ITEMCHANGED,    &Launcher::OnListItemChanged       },
			{ LVN_INSERTITEM,     &Launcher::OnListInsertItem        },
			{ LVN_DELETEALLITEMS, &Launcher::OnListDeleteAllItems    },
			{ LVN_DELETEITEM,     &Launcher::OnListDeleteItem        }
		};

		const Control::NotificationHandler::Entry<Launcher> Launcher::Handlers::treeNotifications[] =
		{
			{ TVN_SELCHANGING, &Launcher::OnTreeSelectionChanging }
		};

		Launcher::Launcher(const Nes::Cartridge::Database& database,const Managers::Paths& p,const Configuration& cfg)
		:
		dialog            ( IDD_LAUNCHER, this, Handlers::messages ),
		menu              ( IDR_MENU_LAUNCHER ),
		listNotifications ( IDC_LAUNCHER_LIST, dialog.Messages() ),
		treeNotifications ( IDC_LAUNCHER_TREE, dialog.Messages() ),
		statusBar         ( dialog, STATUSBAR_SECOND_FIELD_WIDTH ),
		list              ( dialog, menu.Commands(), p, cfg, database ),
		colors            ( cfg )
		{
			menu.Commands().Add( this, Handlers::commands );
			dialog.Commands().Add( CMD_ENTER, this, &Launcher::OnCmdEnter );
			listNotifications.Add( this, Handlers::listNotifications );
			treeNotifications.Add( this, Handlers::treeNotifications );

			{
				Configuration::ConstSection launcher( cfg["launcher"] );

				menu[IDM_LAUNCHER_VIEW_ONTOP].Check( launcher["view"]["show"]["on-top"].Yes() );

				Configuration::ConstSection size( launcher["window-size"] );

				initialSize.x = size["x"].Int();
				initialSize.y = size["y"].Int();

				if (!initialSize.x || !initialSize.y)
				{
					initialSize.x = 0;
					initialSize.y = 0;
				}
			}

			HeapString name;

			for (uint i=0; i < 6; ++i)
			{
				static const ushort keys[6][2] =
				{
					{ IDM_LAUNCHER_FILE_RUN,        VK_RETURN },
					{ IDM_LAUNCHER_FILE_EDITHEADER, VK_F4     },
					{ IDM_LAUNCHER_FILE_REFRESH,    VK_F5     },
					{ IDM_LAUNCHER_EDIT_FIND,       VK_F3     },
					{ IDM_LAUNCHER_EDIT_INSERT,     VK_INSERT },
					{ IDM_LAUNCHER_EDIT_REMOVE,     VK_DELETE }
				};

				menu[keys[i][0]].Text() >> name;
				menu[keys[i][0]].Text() << (name << '\t' << System::Keyboard::GetName( keys[i][1] ));
			}
		}

		Launcher::~Launcher()
		{
			Close();
		}

		void Launcher::Save(Configuration& cfg,bool saveSize,bool saveFiles)
		{
			Configuration::Section launcher( cfg["launcher"] );

			if (saveSize)
			{
				Configuration::Section size( cfg["window-size"] );

				size["x"].Int() = initialSize.x;
				size["y"].Int() = initialSize.y;
			}

			launcher["view"]["show"]["on-top"].YesNo() = menu[IDM_LAUNCHER_VIEW_ONTOP].Checked();

			list.Save( cfg, saveFiles );
			colors.Save( cfg );
		}

		void Launcher::Open(bool child)
		{
			menu[IDM_LAUNCHER_VIEW_ONTOP].Enable( !child );
			dialog.Open( child ? Dialog::MODELESS_CHILD : Dialog::MODELESS_FREE );
		}

		void Launcher::Close()
		{
			dialog.Close();
		}

		void Launcher::Synchronize(HWND hWnd) const
		{
			if (dialog.IsOpen() && menu[IDM_LAUNCHER_VIEW_ONTOP].Enabled() && menu[IDM_LAUNCHER_VIEW_ONTOP].Unchecked())
				dialog.Position().BringBehind( hWnd );
		}

		ibool Launcher::OnInitDialog(Param&)
		{
			menu.Hook( dialog );
			menu.Show();

			statusBar.Enable();

			if (menu[IDM_LAUNCHER_VIEW_ONTOP].Enabled())
				dialog.MakeTopMost( menu[IDM_LAUNCHER_VIEW_ONTOP].Checked() );

			list = dialog.ListView( IDC_LAUNCHER_LIST );
			tree = dialog.TreeView( IDC_LAUNCHER_TREE );

			dialog.Send
			(
				WM_SETICON,
				ICON_SMALL,
				static_cast<HICON>(Resource::Icon(Application::Instance::GetIconStyle() == Application::Instance::ICONSTYLE_NES ? IDI_PAD : IDI_PAD_J))
			);

			margin = dialog.Coordinates().Corner() - list.GetWindow().Coordinates().Corner();

			list.SetColors( colors.GetBackgroundColor(), colors.GetForegroundColor() );
			tree.SetColors( colors.GetBackgroundColor(), colors.GetForegroundColor() );

			menu[ IDM_LAUNCHER_VIEW_SHOWGRIDS ].Check( list.GetStyle() & LVS_EX_GRIDLINES );
			menu[ IDM_LAUNCHER_VIEW_SHOWDATABASECORRECTED ].Check( list.DatabaseCorrectionEnabled() );
			menu[ IDM_LAUNCHER_FILE_REFRESH ].Enable( list.CanRefresh() );

			if (initialSize.x && initialSize.y)
				dialog.Size() = initialSize;
			else
				initialSize = dialog.GetPlacement().Size();

			return true;
		}

		ibool Launcher::OnClose(Param&)
		{
			Close();
			return true;
		}

		ibool Launcher::OnDestroy(Param&)
		{
			tree.Close();
			list.Close();

			initialSize = dialog.GetPlacement().Size();

			return true;
		}

		ibool Launcher::OnSize(Param& param)
		{
			if (param.wParam != SIZE_MINIMIZED)
			{
				const Point corner( dialog.Coordinates().Corner() - margin - list.GetWindow().Position() );

				list.GetWindow().Size() = corner;
				tree.GetWindow().Size() = Point( tree.GetWindow().Coordinates().Width(), corner.y );
			}

			return true;
		}

		ibool Launcher::OnCmdEnter(Param&)
		{
			OnCmdFileRun();
			return true;
		}

		ibool Launcher::OnDropFiles(Param& param)
		{
			list.Insert( param );
			return true;
		}

		void Launcher::UpdateItemCount(const uint count) const
		{
			if (count == 0 || count == 1)
			{
				menu[IDM_LAUNCHER_EDIT_FIND].Enable( count );
				menu[IDM_LAUNCHER_EDIT_CLEAR].Enable( count );
			}

			static HeapString form( HeapString() << ' ' << Resource::String(IDS_TEXT_FILES) << ": " );

			const uint length = form.Length();
			statusBar.Text(StatusBar::SECOND_FIELD) << (form << count).Ptr();
			form.ShrinkTo( length );
		}

		void Launcher::OnListGetDisplayInfo(const NMHDR& nmhdr)
		{
			list.OnGetDisplayInfo( reinterpret_cast<LPARAM>(&nmhdr) );
		}

		void Launcher::OnListKeyDown(const NMHDR& nmhdr)
		{
			switch (reinterpret_cast<const NMLVKEYDOWN&>(nmhdr).wVKey)
			{
				case VK_INSERT: if (menu[ IDM_LAUNCHER_EDIT_INSERT     ].Enabled()) dialog.PostCommand( IDM_LAUNCHER_EDIT_INSERT     ); break;
				case VK_DELETE: if (menu[ IDM_LAUNCHER_EDIT_REMOVE     ].Enabled()) dialog.PostCommand( IDM_LAUNCHER_EDIT_REMOVE     ); break;
				case VK_F3:     if (menu[ IDM_LAUNCHER_EDIT_FIND       ].Enabled()) dialog.PostCommand( IDM_LAUNCHER_EDIT_FIND       ); break;
				case VK_F4:     if (menu[ IDM_LAUNCHER_FILE_EDITHEADER ].Enabled()) dialog.PostCommand( IDM_LAUNCHER_FILE_EDITHEADER ); break;
				case VK_F5:     if (menu[ IDM_LAUNCHER_FILE_REFRESH    ].Enabled()) dialog.PostCommand( IDM_LAUNCHER_FILE_REFRESH    ); break;
			}
		}

		void Launcher::OnListColumnClick(const NMHDR& nmhdr)
		{
			Application::Instance::Waiter wait;
			list.Sort( reinterpret_cast<const NMLISTVIEW&>(nmhdr).iSubItem );
		}

		void Launcher::OnListItemActivate(const NMHDR&)
		{
			OnCmdFileRun();
		}

		void Launcher::OnListItemChanged(const NMHDR& nmhdr)
		{
			const NMLISTVIEW& nmlv = reinterpret_cast<const NMLISTVIEW&>(nmhdr);

			if ((nmlv.uOldState ^ nmlv.uNewState) & LVIS_SELECTED)
			{
				if (nmlv.uNewState & LVIS_SELECTED)
				{
					if (const List::Files::Entry* const entry = list[nmlv.iItem])
					{
						{
							Path path( entry->GetPath(list.GetStrings()), entry->GetFile(list.GetStrings()) );

							if (path.Length() > MAX_PATH)
							{
								path.ShrinkTo( MAX_PATH-3 );
								path << "...";
							}

							statusBar.Text(StatusBar::FIRST_FIELD) << path.Ptr();
						}

						menu[IDM_LAUNCHER_FILE_RUN].Enable();
						menu[IDM_LAUNCHER_FILE_EDITHEADER].Enable( entry->GetType() == List::Files::Entry::NES );
						menu[IDM_LAUNCHER_EDIT_REMOVE].Enable();
					}
				}
				else
				{
					OnNoSelection();
				}
			}
		}

		void Launcher::OnListInsertItem(const NMHDR&)
		{
			UpdateItemCount( list.Size() );
		}

		void Launcher::OnListDeleteItem(const NMHDR&)
		{
			const uint size = list.Size();
			UpdateItemCount( size - (size != 0) );
		}

		void Launcher::OnListDeleteAllItems(const NMHDR&)
		{
			OnNoSelection();
			const uint size = list.Size();
			UpdateItemCount( size - (size != 0) );
		}

		void Launcher::OnTreeSelectionChanging(const NMHDR& nmhdr)
		{
			Application::Instance::Events::Signal( Application::Instance::EVENT_SYSTEM_BUSY );
			list.Draw( tree.GetType(reinterpret_cast<const NMTREEVIEW&>(nmhdr).itemNew.hItem) );
		}

		void Launcher::OnNoSelection() const
		{
			statusBar.Text(StatusBar::FIRST_FIELD).Clear();

			menu[IDM_LAUNCHER_FILE_RUN].Disable();
			menu[IDM_LAUNCHER_FILE_EDITHEADER].Disable();
			menu[IDM_LAUNCHER_EDIT_REMOVE].Disable();
		}

		void Launcher::OnCmdFileRun(uint)
		{
			if (const List::Files::Entry* const entry = list.GetSelection())
				Application::Instance::GetMainWindow().Send( Application::Instance::WM_NST_LAUNCH, 0, Path(entry->GetPath(list.GetStrings()),entry->GetFile(list.GetStrings())).Ptr() );
		}

		void Launcher::OnCmdFileRefresh(uint)
		{
			list.Refresh();
		}

		void Launcher::OnCmdEditHeader(uint)
		{
			if (const List::Files::Entry* const entry = list.GetSelection())
				Window::InesHeader( list.GetPaths() ).Open( Path(entry->GetPath(list.GetStrings()),entry->GetFile(list.GetStrings())) );
		}

		void Launcher::OnCmdViewShowGrids(uint)
		{
			menu[IDM_LAUNCHER_VIEW_SHOWGRIDS].Check( list.ToggleGrids() );
		}

		void Launcher::OnCmdViewShowDatabaseCorrected(uint)
		{
			menu[IDM_LAUNCHER_VIEW_SHOWDATABASECORRECTED].Check( list.ToggleDatabase() );
		}

		void Launcher::OnCmdViewShowOnTop(uint)
		{
			dialog.MakeTopMost( menu[IDM_LAUNCHER_VIEW_ONTOP].ToggleCheck() );
		}

		void Launcher::OnCmdOptionsPaths(uint)
		{
			list.OpenPathDialog();
			menu[IDM_LAUNCHER_FILE_REFRESH].Enable( list.CanRefresh() );
		}

		void Launcher::OnCmdOptionsColors(uint)
		{
			colors.Open();

			list.SetColors( colors.GetBackgroundColor(), colors.GetForegroundColor(), List::REPAINT );
			tree.SetColors( colors.GetBackgroundColor(), colors.GetForegroundColor(), Tree::REPAINT );
		}
	}
}
