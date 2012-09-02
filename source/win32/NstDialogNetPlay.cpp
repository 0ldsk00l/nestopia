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
#include "NstIoFile.hpp"
#include "NstApplicationInstance.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDropFiles.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogNetPlay.hpp"
#include "../core/NstXml.hpp"
#include <CommCtrl.h>

namespace Nestopia
{
	namespace Window
	{
		struct Netplay::Handlers
		{
			static const MsgHandler::Entry<Netplay> messages[];
			static const MsgHandler::Entry<Netplay> commands[];
			static const Control::NotificationHandler::Entry<Netplay> notifications[];
		};

		const MsgHandler::Entry<Netplay> Netplay::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &Netplay::OnInitDialog },
			{ WM_DROPFILES,  &Netplay::OnDropFiles  }
		};

		const MsgHandler::Entry<Netplay> Netplay::Handlers::commands[] =
		{
			{ IDC_NETPLAY_ADD,             &Netplay::OnAdd        },
			{ IDC_NETPLAY_REMOVE,          &Netplay::OnRemove     },
			{ IDC_NETPLAY_CLEAR,           &Netplay::OnClear      },
			{ IDC_NETPLAY_DEFAULT,         &Netplay::OnDefault    },
			{ IDC_NETPLAY_LAUNCH,          &Netplay::OnLaunch     },
			{ IDC_NETPLAY_PLAY_FULLSCREEN, &Netplay::OnFullscreen }
		};

		const Control::NotificationHandler::Entry<Netplay> Netplay::Handlers::notifications[] =
		{
			{ LVN_KEYDOWN,     &Netplay::OnKeyDown        },
			{ LVN_ITEMCHANGED, &Netplay::OnItemChanged    },
			{ LVN_INSERTITEM,  &Netplay::OnInsertItem     },
			{ LVN_DELETEITEM,  &Netplay::OnDeleteItem     }
		};

		bool Netplay::Games::Less::operator () (const Path& a,const Path& b) const
		{
			return a.Target().File() < b.Target().File();
		}

		Netplay::Games::Games()
		: state(UNINITIALIZED) {}

		Netplay::Netplay(Managers::Emulator& e,const Managers::Paths& p,bool fullscreen)
		:
		dialog        ( IDD_NETPLAY, this, Handlers::messages, Handlers::commands ),
		doFullscreen  ( fullscreen ),
		paths         ( p ),
		emulator      ( e ),
		notifications ( IDC_NETPLAY_GAMELIST, dialog.Messages(), this, Handlers::notifications )
		{
		}

		Netplay::~Netplay()
		{
		}

		void Netplay::LoadFile()
		{
			try
			{
				const Path path( Application::Instance::GetExePath(L"netplaylist.xml") );

				if (path.FileExists())
				{
					typedef Nes::Core::Xml Xml;
					Xml xml;

					{
						Io::Stream::In stream( path );
						xml.Read( stream );
					}

					if (!xml.GetRoot().IsType( L"netplaylist" ))
						throw 1;

					for (Xml::Node node(xml.GetRoot().GetFirstChild()); node; node=node.GetNextSibling())
					{
						if (!node.IsType( L"file" ))
							throw 1;

						Add( node.GetValue() );
					}

					Io::Log() << "Netplay: loaded game list from \"netplaylist.xml\"\r\n";
				}
				else
				{
					Io::Log() << "Netplay: game list file \"netplaylist.xml\" not present..\r\n";
				}
			}
			catch (...)
			{
				games.state = Games::DIRTY;
				Io::Log() << "Netplay: warning, couldn't load game list \"netplaylist.xml\"!\r\n";
			}
		}

		void Netplay::SaveFile() const
		{
			if (games.state == Games::DIRTY)
			{
				const Path path( Application::Instance::GetExePath(L"netplaylist.xml") );

				if (!games.paths.empty())
				{
					try
					{
						typedef Nes::Core::Xml Xml;

						Xml xml;
						Xml::Node root( xml.Create( L"netplaylist" ) );
						root.AddAttribute( L"version", L"1.0" );

						for (Games::Paths::const_iterator it(games.paths.begin()), end(games.paths.end()); it != end; ++it)
							root.AddChild( L"file", it->Ptr() );

						Io::Stream::Out stream( path );
						xml.Write( root, stream );

						Io::Log() << "Netplay: saved game list to \"netplaylist.xml\"\r\n";
					}
					catch (...)
					{
						Io::Log() << "Netplay: warning, couldn't save game list to \"netplaylist.xml\"!\r\n";
					}
				}
				else if (path.FileExists())
				{
					if (Io::File::Delete( path.Ptr() ))
						Io::Log() << "Netplay: game list empty, deleted \"netplaylist.xml\"\r\n";
					else
						Io::Log() << "Netplay: warning, couldn't delete \"netplaylist.xml\"!\r\n";
				}
			}
		}

		wcstring Netplay::GetPath(wcstring const path) const
		{
			Games::Paths::const_iterator it(games.paths.find( path ));
			return it != games.paths.end() ? it->Ptr() : NULL;
		}

		void Netplay::Add(Path path)
		{
			enum
			{
				TYPES = Managers::Paths::File::GAME|Managers::Paths::File::ARCHIVE
			};

			if (path.Length() && games.paths.size() < Games::LIMIT && paths.CheckFile( path, TYPES ) && games.paths.insert( path ).second)
			{
				games.state = Games::DIRTY;

				if (dialog)
					dialog.ListView( IDC_NETPLAY_GAMELIST ).Add( path.Target().File() );
			}
		}

		ibool Netplay::OnInitDialog(Param&)
		{
			dialog.CheckBox( IDC_NETPLAY_PLAY_FULLSCREEN ).Check( doFullscreen );
			dialog.CheckBox( IDC_NETPLAY_REMOVE ).Disable();

			if (games.state == Games::UNINITIALIZED)
			{
				LoadFile();
			}
			else
			{
				Control::ListView list( dialog.ListView( IDC_NETPLAY_GAMELIST ) );
				list.Reserve( games.paths.size() );

				for (Games::Paths::const_iterator it(games.paths.begin()), end(games.paths.end()); it != end; ++it)
					list.Add( it->Target().File().Ptr() );
			}

			dialog.CheckBox( IDC_NETPLAY_CLEAR ).Enable( games.paths.size() );
			dialog.CheckBox( IDC_NETPLAY_LAUNCH ).Enable( games.paths.size() );

			return true;
		}

		ibool Netplay::OnAdd(Param& param)
		{
			if (param.Button().Clicked())
				Add( paths.BrowseLoad(Managers::Paths::File::GAME|Managers::Paths::File::ARCHIVE) );

			return true;
		}

		ibool Netplay::OnRemove(Param& param)
		{
			if (param.Button().Clicked())
				dialog.ListView( IDC_NETPLAY_GAMELIST ).Selection().Delete();

			return true;
		}

		ibool Netplay::OnClear(Param& param)
		{
			if (param.Button().Clicked())
				dialog.ListView( IDC_NETPLAY_GAMELIST ).Clear();

			return true;
		}

		ibool Netplay::OnDefault(Param& param)
		{
			if (param.Button().Clicked())
			{
				doFullscreen = false;
				dialog.CheckBox( IDC_NETPLAY_PLAY_FULLSCREEN ).Check( false );
			}

			return true;
		}

		ibool Netplay::OnLaunch(Param& param)
		{
			if (param.Button().Clicked())
				dialog.Close( LAUNCH );

			return true;
		}

		ibool Netplay::OnFullscreen(Param& param)
		{
			if (param.Button().Clicked())
				doFullscreen = dialog.CheckBox( IDC_NETPLAY_PLAY_FULLSCREEN ).Checked();

			return true;
		}

		ibool Netplay::OnDropFiles(Param& param)
		{
			DropFiles dropFiles( param );

			if (dropFiles.Inside( dialog.ListView( IDC_NETPLAY_GAMELIST ).GetWindow() ))
			{
				for (uint i=0, n=dropFiles.Size(); i < n; ++i)
					Add( dropFiles[i] );
			}

			return true;
		}

		void Netplay::OnKeyDown(const NMHDR& nmhdr)
		{
			switch (reinterpret_cast<const NMLVKEYDOWN&>(nmhdr).wVKey)
			{
				case VK_INSERT: dialog.PostCommand( IDC_NETPLAY_ADD    ); break;
				case VK_DELETE: dialog.PostCommand( IDC_NETPLAY_REMOVE ); break;
			}
		}

		void Netplay::OnItemChanged(const NMHDR& nmhdr)
		{
			const NMLISTVIEW& nm = reinterpret_cast<const NMLISTVIEW&>(nmhdr);

			if ((nm.uOldState ^ nm.uNewState) & LVIS_SELECTED)
				dialog.CheckBox( IDC_NETPLAY_REMOVE ).Enable( nm.uNewState & LVIS_SELECTED );
		}

		void Netplay::OnInsertItem(const NMHDR&)
		{
			dialog.CheckBox( IDC_NETPLAY_CLEAR ).Enable();
			dialog.CheckBox( IDC_NETPLAY_LAUNCH ).Enable();
		}

		void Netplay::OnDeleteItem(const NMHDR& nmhdr)
		{
			Games::Paths::iterator it(games.paths.begin());

			for (int i=reinterpret_cast<const NMLISTVIEW&>(nmhdr).iItem; i > 0; --i)
				++it;

			games.paths.erase( it );
			games.state = Games::DIRTY;

			if (dialog.ListView( IDC_NETPLAY_GAMELIST ).Size() <= 1)
			{
				dialog.CheckBox( IDC_NETPLAY_CLEAR ).Disable();
				dialog.CheckBox( IDC_NETPLAY_LAUNCH ).Disable();
				dialog.CheckBox( IDC_NETPLAY_REMOVE ).Disable();
			}
		}
	}
}
