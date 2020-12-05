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

#include "NstIoStream.hpp"
#include "NstResourceString.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDropFiles.hpp"
#include "NstWindowUser.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogCheats.hpp"
#include "NstApplicationInstance.hpp"
#include "../core/NstXml.hpp"
#include "../core/api/NstApiCartridge.hpp"
#include <CommCtrl.h>

namespace Nestopia
{
	namespace Window
	{
		class Cheats::MainDialog
		{
		public:

			MainDialog(Codes&,Codes&,bool&,bool&,Searcher&,Managers::Emulator&,const Managers::Paths&);

		private:

			struct Handlers;

			class Section
			{
			public:

				Section(uint,Dialog&,Codes&,const bool&,bool&,Searcher&,Managers::Emulator&,const Managers::Paths&);

				void UpdateHexView() const;

			private:

				struct Handlers;

				class CodeDialog
				{
				public:

					struct Context
					{
						Code code;
						bool edit;
						bool& hex;
						Searcher& searcher;

						Context(bool& h,Searcher& s)
						: edit(false), hex(h), searcher(s) {}
					};

					CodeDialog(Managers::Emulator&,const Managers::Paths&,Context&);

				private:

					struct Handlers;

					ibool OnInitDialog(Param&);
					ibool OnDestroy(Param&);

					ibool OnCmdSubmit      (Param&);
					ibool OnCmdValidate    (Param&);
					ibool OnCmdGameCurrent (Param&);
					ibool OnCmdGameBrowse  (Param&);
					ibool OnCmdHex         (Param&);
					ibool OnCmdType        (Param&);
					ibool OnSearchType     (Param&);
					ibool OnCmdReset       (Param&);

					void OnItemChanged(const NMHDR&);

					void UpdateInput() const;
					void UpdateHexView(bool) const;
					void UpdateSearchList() const;

					bool GetRawCode   (NesCode&) const;
					bool GetGenieCode (NesCode&) const;
					bool GetRockyCode (NesCode&) const;

					void SetRawCode   (const NesCode&) const;
					void SetGenieCode (const NesCode&) const;
					void SetRockyCode (const NesCode&) const;

					uint GetSearchValue(uint) const;
					void SetSearchValue(uint,uint) const;

					void AddSearchEntry(Control::ListView,uint) const;

					Context& context;
					Managers::Emulator& emulator;
					const Managers::Paths& paths;
					Dialog dialog;
					Control::NotificationHandler notificationHandler;

				public:

					bool Open()
					{
						return dialog.Open();
					}
				};

				enum
				{
					ADD,
					EDIT,
					REMOVE,
					IMPORT,
					EXPORT,
					CLEAR,
					NUM_CONTROLS
				};

				enum Column
				{
					COLUMN_CODE,
					COLUMN_ADDRESS,
					COLUMN_VALUE,
					COLUMN_COMPARE,
					COLUMN_GAME,
					COLUMN_DESCRIPTION
				};

				enum
				{
					NUM_COLUMNS = 6
				};

				int Sorter(const void*,const void*);
				void RefreshListView();
				void SetCode(int=-1);
				void AddCodes(const Codes&);
				void AddToListView(const Code&) const;

				void OnInitDialog (Param&);
				void OnDropFiles  (Param&);

				ibool OnCmdAdd    (Param&);
				ibool OnCmdEdit   (Param&);
				ibool OnCmdRemove (Param&);
				ibool OnCmdExport (Param&);
				ibool OnCmdImport (Param&);
				ibool OnCmdClear  (Param&);

				void OnKeyDown     (const NMHDR&);
				void OnItemChanged (const NMHDR&);
				void OnInsertItem  (const NMHDR&);
				void OnDeleteItem  (const NMHDR&);
				void OnColumnClick (const NMHDR&);

				const uint id;
				Codes& codes;
				const bool& showHexMainDialog;
				bool& showHexSubDialogs;
				Searcher& searcher;
				const Dialog& dialog;
				Control::NotificationHandler notificationHandler;
				Control::ListView listView;
				Control::Generic controls[NUM_CONTROLS];
				Column sortColumns[NUM_COLUMNS];
				Managers::Emulator& emulator;
				const Managers::Paths& paths;
			};

			ibool OnInitDialog (Param&);
			ibool OnCmdShowHex (Param&);

			Dialog dialog;
			Section permanentView;
			Section temporaryView;
			bool& showHex;

		public:

			void Open()
			{
				dialog.Open();
			}
		};

		Cheats::Cheats(Managers::Emulator& e,const Configuration& cfg,const Managers::Paths& p)
		:
		paths    ( p ),
		emulator ( e )
		{
			Configuration::ConstSection cheats( cfg["cheats"] );

			showHexMainDialog = cheats["show-hex-main-view"].Yes();
			showHexSubDialogs = cheats["show-hex-code-view"].Yes();

			for (uint i=0; i < MAX_CODES; ++i)
			{
				if (Configuration::ConstSection cheat=cheats["cheat"][i])
				{
					Code code;

					uint data = cheat["address"].Int();

					if (data > 0xFFFF)
						continue;

					code.address = data;

					data = cheat["value"].Int();

					if (data > 0xFF)
						continue;

					code.value = data;

					if (Configuration::ConstSection compare=cheat["compare"])
					{
						data = compare.Int();

						if (data > 0xFF)
							continue;

						code.compare = data;
					}

					code.enabled = !cheat["enabled"].No();

					code.crc = cheat["game"].Int();

					code.description = cheat["description"].Str();
					code.description.Trim();

					codes[PERMANENT_CODES].insert( code );
				}
				else
				{
					break;
				}
			}
		}

		Cheats::~Cheats()
		{
		}

		void Cheats::Save(Configuration& cfg) const
		{
			Configuration::Section cheats( cfg["cheats"] );

			cheats["show-hex-main-view"].YesNo() = showHexMainDialog;
			cheats["show-hex-code-view"].YesNo() = showHexSubDialogs;

			uint i = 0;
			for (Codes::const_iterator it(codes[PERMANENT_CODES].begin()), end(codes[PERMANENT_CODES].end()); it != end; ++it)
			{
				Configuration::Section cheat( cheats["cheat"][i++] );

				cheat[ "enabled" ].YesNo() = it->enabled;

				if (it->crc)
					cheat[ "game" ].Str() = HexString( 32, it->crc ).Ptr();

				cheat[ "address" ].Str() = HexString( 16, it->address ).Ptr();
				cheat[ "value"   ].Str() = HexString(  8, it->value   ).Ptr();

				if (it->compare != Code::NO_COMPARE)
					cheat["compare"].Str() = HexString( 8, it->compare ).Ptr();

				if (it->description.Length())
					cheat["description"].Str() = it->description.Ptr();
			}
		}

		void Cheats::Flush()
		{
			searcher.filter = Searcher::NO_FILTER;
			searcher.a = 0;
			searcher.b = 0;

			codes[TEMPORARY_CODES].clear();
		}

		void Cheats::Open()
		{
			MainDialog( codes[PERMANENT_CODES], codes[TEMPORARY_CODES], showHexMainDialog, showHexSubDialogs, searcher, emulator, paths ).Open();
		}

		bool Cheats::Load(const Path& path)
		{
			return Import( codes[TEMPORARY_CODES], path );
		}

		bool Cheats::Save(const Path& path) const
		{
			return Export( codes[TEMPORARY_CODES], path );
		}

		bool Cheats::Import(Codes& codes,const Path& path)
		{
			try
			{
				typedef Nes::Core::Xml Xml;
				Xml xml;

				{
					Io::Stream::In stream( path );
					xml.Read( stream );
				}

				if (!xml.GetRoot().IsType( L"cheats" ))
					return false;

				for (Xml::Node node(xml.GetRoot().GetFirstChild()); node && codes.size() < MAX_CODES; node=node.GetNextSibling())
				{
					if (!node.IsType( L"cheat" ))
						continue;

					Code code;

					if (const Xml::Node address=node.GetChild( L"address" ))
					{
						uint v;

						if (0xFFFF < (v=address.GetUnsignedValue()))
							continue;

						code.address = v;

						if (const Xml::Node value=node.GetChild( L"value" ))
						{
							if (0xFF < (v=value.GetUnsignedValue()))
								continue;

							code.value = v;
						}

						if (const Xml::Node compare=node.GetChild( L"compare" ))
						{
							if (0xFF < (v=compare.GetUnsignedValue()))
								continue;

							code.compare = v;
						}
					}
					else
					{
						NesCode nesCode;

						if (const Xml::Node genie=node.GetChild( L"genie" ))
						{
							if (NES_FAILED(Nes::Cheats::GameGenieDecode( String::Heap<char>(genie.GetValue()).Ptr(), nesCode )))
								continue;
						}
						else if (const Xml::Node rocky=node.GetChild( L"rocky" ))
						{
							if (NES_FAILED(Nes::Cheats::ProActionRockyDecode( String::Heap<char>(rocky.GetValue()).Ptr(), nesCode )))
								continue;
						}
						else
						{
							continue;
						}

						code.FromNesCode( nesCode );
					}

					code.description = node.GetChild( L"description" ).GetValue();
					code.enabled = !node.GetAttribute( L"enabled" ).IsValue( L"0" );
					code.crc = node.GetAttribute( L"game" ).GetUnsignedValue();

					codes.insert( code );
				}
			}
			catch (...)
			{
				return false;
			}

			return true;
		}

		bool Cheats::Export(const Codes& codes,const Path& path)
		{
			if (!path.Length() || codes.empty())
				return false;

			try
			{
				typedef Nes::Core::Xml Xml;

				Xml xml;
				Xml::Node root( xml.GetRoot() );

				root = xml.Create( L"cheats" );
				root.AddAttribute( L"version", L"1.0" );

				for (Codes::const_iterator it(codes.begin()), end(codes.end()); it != end; ++it)
				{
					Xml::Node node( root.AddChild( L"cheat" ) );
					node.AddAttribute( L"enabled", it->enabled ? L"1" : L"0" );

					if (it->crc)
						node.AddAttribute( L"game", HexString( 32, it->crc ).Ptr() );

					char buffer[9];

					if (NES_SUCCEEDED(Nes::Cheats::GameGenieEncode( it->ToNesCode(), buffer )))
						node.AddChild( L"genie", HeapString(buffer).Ptr() );

					if (NES_SUCCEEDED(Nes::Cheats::ProActionRockyEncode( it->ToNesCode(), buffer )))
						node.AddChild( L"rocky", HeapString(buffer).Ptr() );

					node.AddChild( L"address", HexString( 16, it->address ).Ptr() );
					node.AddChild( L"value",   HexString( 8,  it->value   ).Ptr() );

					if (it->compare != Code::NO_COMPARE)
						node.AddChild( L"compare", HexString( 8, it->compare ).Ptr() );

					if (it->description.Length())
						node.AddChild( L"description", it->description.Ptr() );
				}

				Io::Stream::Out stream( path );
				xml.Write( root, stream );
			}
			catch (...)
			{
				return false;
			}

			return true;
		}

		struct Cheats::MainDialog::Handlers
		{
			static const MsgHandler::Entry<MainDialog> messages[];
			static const MsgHandler::Entry<MainDialog> commands[];
		};

		const MsgHandler::Entry<Cheats::MainDialog> Cheats::MainDialog::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &MainDialog::OnInitDialog }
		};

		const MsgHandler::Entry<Cheats::MainDialog> Cheats::MainDialog::Handlers::commands[] =
		{
			{ IDC_CHEATS_SHOW_HEX, &MainDialog::OnCmdShowHex }
		};

		Cheats::MainDialog::MainDialog
		(
			Codes& permanentCodes,
			Codes& temporaryCodes,
			bool& showHexMainDialog,
			bool& showHexSubDialogs,
			Searcher& searcher,
			Managers::Emulator& emulator,
			const Managers::Paths& paths
		)
		:
		dialog        ( IDD_CHEATS, this, Handlers::messages, Handlers::commands ),
		permanentView ( 0, dialog, permanentCodes, showHexMainDialog, showHexSubDialogs, searcher, emulator, paths ),
		temporaryView ( 1, dialog, temporaryCodes, showHexMainDialog, showHexSubDialogs, searcher, emulator, paths ),
		showHex       ( showHexMainDialog )
		{
		}

		ibool Cheats::MainDialog::OnInitDialog(Param&)
		{
			dialog.CheckBox( IDC_CHEATS_SHOW_HEX ).Check( showHex );
			return true;
		}

		ibool Cheats::MainDialog::OnCmdShowHex(Param& param)
		{
			if (param.Button().Clicked())
			{
				showHex = dialog.CheckBox( IDC_CHEATS_SHOW_HEX ).Checked();
				permanentView.UpdateHexView();
				temporaryView.UpdateHexView();
			}

			return true;
		}

		struct Cheats::MainDialog::Section::Handlers
		{
			static const Control::NotificationHandler::Entry<Section> notifications[];
		};

		const Control::NotificationHandler::Entry<Cheats::MainDialog::Section> Cheats::MainDialog::Section::Handlers::notifications[] =
		{
			{ LVN_KEYDOWN,     &Section::OnKeyDown     },
			{ LVN_ITEMCHANGED, &Section::OnItemChanged },
			{ LVN_INSERTITEM,  &Section::OnInsertItem  },
			{ LVN_DELETEITEM,  &Section::OnDeleteItem  },
			{ LVN_COLUMNCLICK, &Section::OnColumnClick }
		};

		Cheats::MainDialog::Section::Section
		(
			uint i,
			Dialog& d,
			Codes& c,
			const bool& h,
			bool& a,
			Searcher& s,
			Managers::Emulator& e,
			const Managers::Paths& p
		)
		:
		id                  ( i ),
		codes               ( c ),
		showHexMainDialog   ( h ),
		showHexSubDialogs   ( a ),
		searcher            ( s ),
		dialog              ( d ),
		notificationHandler ( i == 0 ? IDC_CHEATS_STATIC_CODES : IDC_CHEATS_TEMP_CODES, d.Messages(), this, Handlers::notifications ),
		emulator            ( e ),
		paths               ( p )
		{
			static const MsgHandler::Entry<Section> commands[2][6] =
			{
				{
					{ IDC_CHEATS_STATIC_ADD,    &Section::OnCmdAdd    },
					{ IDC_CHEATS_STATIC_EDIT,   &Section::OnCmdEdit   },
					{ IDC_CHEATS_STATIC_REMOVE, &Section::OnCmdRemove },
					{ IDC_CHEATS_STATIC_EXPORT, &Section::OnCmdExport },
					{ IDC_CHEATS_STATIC_IMPORT, &Section::OnCmdImport },
					{ IDC_CHEATS_STATIC_CLEAR,  &Section::OnCmdClear  }
				},
				{
					{ IDC_CHEATS_TEMP_ADD,      &Section::OnCmdAdd    },
					{ IDC_CHEATS_TEMP_EDIT,     &Section::OnCmdEdit   },
					{ IDC_CHEATS_TEMP_REMOVE,   &Section::OnCmdRemove },
					{ IDC_CHEATS_TEMP_EXPORT,   &Section::OnCmdExport },
					{ IDC_CHEATS_TEMP_IMPORT,   &Section::OnCmdImport },
					{ IDC_CHEATS_TEMP_CLEAR,    &Section::OnCmdClear  }
				}
			};

			d.Commands().Add( this, commands[i != 0] );

			static const MsgHandler::HookEntry<Section> hooks[] =
			{
				{ WM_INITDIALOG, &Section::OnInitDialog },
				{ WM_DROPFILES,  &Section::OnDropFiles  }
			};

			d.Messages().Hooks().Add( this, hooks );

			for (uint i=0; i < NUM_COLUMNS; ++i)
				sortColumns[i] = static_cast<Column>(i);
		}

		void Cheats::MainDialog::Section::OnInitDialog(Param& param)
		{
			listView = Control::ListView( param.hWnd, id == 0 ? IDC_CHEATS_STATIC_CODES : IDC_CHEATS_TEMP_CODES );
			listView.StyleEx() = LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES;

			listView.Columns().Clear();

			for (uint i=0; i < NUM_COLUMNS; ++i)
			{
				static const ushort columns[NUM_COLUMNS] =
				{
					IDS_CHEAT_CODE,
					IDS_CHEAT_ADDRESS,
					IDS_CHEAT_VALUE,
					IDS_CHEAT_COMPARE,
					IDS_CHEAT_GAME,
					IDS_CHEAT_DESCRIPTION
				};

				listView.Columns().Insert( i, Resource::String(columns[i]) );
			}

			controls[ ADD    ] = Control::Generic( param.hWnd, id ? IDC_CHEATS_TEMP_ADD    : IDC_CHEATS_STATIC_ADD    );
			controls[ EDIT   ] = Control::Generic( param.hWnd, id ? IDC_CHEATS_TEMP_EDIT   : IDC_CHEATS_STATIC_EDIT   );
			controls[ REMOVE ] = Control::Generic( param.hWnd, id ? IDC_CHEATS_TEMP_REMOVE : IDC_CHEATS_STATIC_REMOVE );
			controls[ IMPORT ] = Control::Generic( param.hWnd, id ? IDC_CHEATS_TEMP_IMPORT : IDC_CHEATS_STATIC_IMPORT );
			controls[ EXPORT ] = Control::Generic( param.hWnd, id ? IDC_CHEATS_TEMP_EXPORT : IDC_CHEATS_STATIC_EXPORT );
			controls[ CLEAR  ] = Control::Generic( param.hWnd, id ? IDC_CHEATS_TEMP_CLEAR  : IDC_CHEATS_STATIC_CLEAR  );

			controls[ EDIT   ].Disable();
			controls[ REMOVE ].Disable();
			controls[ EXPORT ].Disable();
			controls[ CLEAR  ].Disable();

			if (!codes.empty())
			{
				listView.Reserve( codes.size() );

				for (Codes::iterator it(codes.begin()), end(codes.end()); it != end; ++it)
					AddToListView( *it );
			}

			RefreshListView();
		}

		ibool Cheats::MainDialog::Section::OnCmdAdd(Param& param)
		{
			if (param.Button().Clicked())
				SetCode();

			return true;
		}

		ibool Cheats::MainDialog::Section::OnCmdEdit(Param& param)
		{
			if (param.Button().Clicked())
				SetCode( listView.Selection().GetIndex() );

			return true;
		}

		ibool Cheats::MainDialog::Section::OnCmdRemove(Param& param)
		{
			if (param.Button().Clicked())
			{
				NST_VERIFY( !codes.empty() );
				listView.Selection().Delete();
			}

			return true;
		}

		ibool Cheats::MainDialog::Section::OnCmdExport(Param& param)
		{
			if (param.Button().Clicked() && !codes.empty())
			{
				Path path( paths.BrowseSave( Managers::Paths::File::XML, Managers::Paths::SUGGEST, paths.GetCheatPath() ) );
				paths.FixFile( Managers::Paths::File::XML, path );

				if (path.Length())
				{
					if (path.FileExists() && User::Confirm( IDS_CHEATS_EXPORTEXISTING ))
					{
						Codes combinedCodes( codes );

						if (!Import( combinedCodes, path ))
							User::Warn( IDS_CHEATS_EXPORTEXISTING_ERROR );

						if (!Export( combinedCodes, path ))
							User::Fail( IDS_FILE_ERR_INVALID );
					}
					else
					{
						if (!Export( codes, path ))
							User::Fail( IDS_FILE_ERR_INVALID );
					}
				}
			}

			return true;
		}

		ibool Cheats::MainDialog::Section::OnCmdImport(Param& param)
		{
			if (param.Button().Clicked())
			{
				const Path path( paths.BrowseLoad( Managers::Paths::File::XML, paths.GetCheatPath() ) );

				if (path.Length())
				{
					Codes newCodes;

					if (Import( newCodes, path ))
						AddCodes( newCodes );
					else
						User::Fail( IDS_FILE_ERR_INVALID );
				}
			}

			return true;
		}

		ibool Cheats::MainDialog::Section::OnCmdClear(Param& param)
		{
			if (param.Button().Clicked())
			{
				NST_VERIFY( !codes.empty() );
				listView.Clear();
				RefreshListView();
			}

			return true;
		}

		void Cheats::MainDialog::Section::AddCodes(const Codes& newCodes)
		{
			bool refresh = false;

			for (Codes::const_iterator it(newCodes.begin()), end(newCodes.end()); it != end; ++it)
			{
				std::pair<Codes::iterator,bool> result( codes.insert( *it ) );

				if (result.second)
				{
					refresh = true;
					AddToListView( *result.first );
				}
			}

			if (refresh)
				RefreshListView();
		}

		void Cheats::MainDialog::Section::UpdateHexView() const
		{
			for (uint i=0, n=listView.Size(); i < n; ++i)
			{
				const Code* const code = static_cast<Code*>(static_cast<void*>(listView[i].Data()));
				NST_VERIFY( code );

				if (code)
				{
					listView[i].Text( COLUMN_VALUE ) << (showHexMainDialog ? HexString( 8, code->value ).Ptr() : (String::Stack<8>() << code->value).Ptr());

					if (code->compare != Code::NO_COMPARE)
						listView[i].Text( COLUMN_COMPARE ) << (showHexMainDialog ? HexString( 8, code->compare ).Ptr() : (String::Stack<8>() << code->compare).Ptr());
				}
			}
		}

		void Cheats::MainDialog::Section::OnKeyDown(const NMHDR& nmhdr)
		{
			switch (reinterpret_cast<const NMLVKEYDOWN&>(nmhdr).wVKey)
			{
				case VK_INSERT:

					SetCode();
					break;

				case VK_DELETE:

					listView.Selection().Delete();
					break;
			}
		}

		void Cheats::MainDialog::Section::OnItemChanged(const NMHDR& nmhdr)
		{
			const NMLISTVIEW& nm = reinterpret_cast<const NMLISTVIEW&>(nmhdr);

			if ((nm.uOldState ^ nm.uNewState) & LVIS_SELECTED)
			{
				controls[ REMOVE ].Enable( nm.uNewState & LVIS_SELECTED );
				controls[ EDIT ].Enable( nm.uNewState & LVIS_SELECTED );
			}

			if ((nm.uOldState ^ nm.uNewState) & LVIS_STATEIMAGEMASK)
			{
				NST_VERIFY( nm.lParam );

				if (nm.lParam)
				{
					// As documented on MSDN the image index for the checked box is 2 (unchecked is 1)
					reinterpret_cast<Code*>(nm.lParam)->enabled = ((nm.uNewState & LVIS_STATEIMAGEMASK) == INDEXTOSTATEIMAGEMASK( 2 ));
				}
			}
		}

		void Cheats::MainDialog::Section::OnInsertItem(const NMHDR&)
		{
			if (listView.Size() == 1)
			{
				controls[ CLEAR ].Enable();
				controls[ EXPORT ].Enable();
			}
		}

		void Cheats::MainDialog::Section::OnDeleteItem(const NMHDR& nmhdr)
		{
			const Code* const code = reinterpret_cast<Code*>(reinterpret_cast<const NMLISTVIEW&>(nmhdr).lParam);
			NST_VERIFY( code );

			if (code)
			{
				Codes::iterator it(codes.find( *code ));
				NST_VERIFY( it != codes.end() );

				if (it != codes.end())
					codes.erase( it );
			}

			if (listView.Size() == 1)
			{
				controls[ CLEAR ].Disable();
				controls[ EXPORT ].Disable();
				controls[ REMOVE ].Disable();
			}
		}

		void Cheats::MainDialog::Section::OnColumnClick(const NMHDR& nmhdr)
		{
			uint column = reinterpret_cast<const NMLISTVIEW&>(nmhdr).iSubItem;
			NST_VERIFY( column <= NUM_COLUMNS );

			if (column <= NUM_COLUMNS)
			{
				sortColumns[0] = static_cast<Column>(column);

				for (uint i=1, next=0; i < NUM_COLUMNS; ++i, ++next)
				{
					if (next == column)
						next++;

					sortColumns[i] = static_cast<Column>(next);
				}

				if (!codes.empty())
					listView.Sort( this, &Section::Sorter );
			}
		}

		void Cheats::MainDialog::Section::AddToListView(const Code& code) const
		{
			const uint index = listView.Add( code.ToGenieCode(), &code, code.enabled );

			listView[index].Text( COLUMN_ADDRESS     ) << HexString( 16, code.address ).Ptr();
			listView[index].Text( COLUMN_VALUE       ) << (showHexMainDialog ? HexString(  8, code.value   ).Ptr() : (String::Stack<8>() << code.value).Ptr());
			listView[index].Text( COLUMN_COMPARE     ) << (code.compare != Code::NO_COMPARE ? showHexMainDialog ? HexString( 8, code.compare ).Ptr() : (String::Stack<8>() << code.compare).Ptr() : L"-");
			listView[index].Text( COLUMN_GAME        ) << (code.crc ? HexString( 32, code.crc ).Ptr() : L"-");
			listView[index].Text( COLUMN_DESCRIPTION ) << (code.description.Length() ? code.description.Ptr() : L"-");
		}

		void Cheats::MainDialog::Section::OnDropFiles(Param& param)
		{
			DropFiles dropFiles( param );

			if (dropFiles.Inside( listView.GetHandle() ))
			{
				Codes newCodes;

				for (uint i=0, n=dropFiles.Size(); i < n; ++i)
					Import( newCodes, dropFiles[i] );

				AddCodes( newCodes );
			}
		}

		void Cheats::MainDialog::Section::RefreshListView()
		{
			if (!codes.empty())
				listView.Sort( this, &Section::Sorter );

			listView.Columns().Align();
		}

		int Cheats::MainDialog::Section::Sorter(const void* obj1,const void* obj2)
		{
			const Code& a = *static_cast<const Code*>( obj1 );
			const Code& b = *static_cast<const Code*>( obj2 );

			for (uint i=0; i < NUM_COLUMNS; ++i)
			{
				switch (sortColumns[i])
				{
					case COLUMN_CODE:
					{
						const Code::GenieCode ga( a.ToGenieCode() );
						const Code::GenieCode gb( b.ToGenieCode() );

						if (gb == L"-")
						{
							if (ga != L"-")
								return -1;
						}
						else if (ga == L"-")
						{
							if (gb != L"-")
								return 1;
						}
						else
						{
							if (ga < gb)
								return -1;

							if (ga > gb)
								return 1;
						}
						continue;
					}

					case COLUMN_ADDRESS:

						if (a.address < b.address)
							return -1;

						if (a.address > b.address)
							return 1;

						continue;

					case COLUMN_VALUE:

						if (a.value < b.value)
							return -1;

						if (a.value > b.value)
							return 1;

						continue;

					case COLUMN_COMPARE:

						if (a.compare < b.compare)
							return -1;

						if (a.compare > b.compare)
							return 1;

						continue;

					case COLUMN_GAME:

						if ((a.crc ? a.crc : ~0U) < (b.crc ? b.crc : ~0U))
							return -1;

						if ((a.crc ? a.crc : ~0U) > (b.crc ? b.crc : ~0U))
							return 1;

						continue;

					case COLUMN_DESCRIPTION:

						if (!b.description.Length())
						{
							if (a.description.Length())
								return -1;
						}
						else if (!a.description.Length())
						{
							if (b.description.Length())
								return 1;
						}
						else
						{
							if (a.description < b.description)
								return -1;

							if (a.description > b.description)
								return 1;
						}
						continue;

					default: NST_UNREACHABLE();
				}
			}

			return 0;
		}

		void Cheats::MainDialog::Section::SetCode(int index)
		{
			CodeDialog::Context context( showHexSubDialogs, searcher );

			if (index >= 0)
			{
				if (const void* const existing = listView[index].Data())
				{
					const Codes::const_iterator it( codes.find( *static_cast<const Code*>(existing) ) );

					if (it != codes.end())
					{
						context.code = *it;
						context.edit = true;
					}
				}
			}

			if (CodeDialog( emulator, paths, context ).Open())
			{
				if (index >= 0)
					listView[index].Delete();

				const std::pair<Codes::iterator,bool> result( codes.insert( context.code ) );

				if (result.second)
				{
					AddToListView( *result.first );
					RefreshListView();
				}
			}
		}

		struct Cheats::MainDialog::Section::CodeDialog::Handlers
		{
			static const MsgHandler::Entry<CodeDialog> messages[];
			static const MsgHandler::Entry<CodeDialog> commands[];
		};

		const MsgHandler::Entry<Cheats::MainDialog::Section::CodeDialog> Cheats::MainDialog::Section::CodeDialog::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &CodeDialog::OnInitDialog }
		};

		const MsgHandler::Entry<Cheats::MainDialog::Section::CodeDialog> Cheats::MainDialog::Section::CodeDialog::Handlers::commands[] =
		{
			{ IDC_CHEATS_ADDCODE_SUBMIT,             &CodeDialog::OnCmdSubmit      },
			{ IDC_CHEATS_ADDCODE_VALIDATE,           &CodeDialog::OnCmdValidate    },
			{ IDC_CHEATS_ADDCODE_GAME_CURRENT,       &CodeDialog::OnCmdGameCurrent },
			{ IDC_CHEATS_ADDCODE_GAME_BROWSE,        &CodeDialog::OnCmdGameBrowse  },
			{ IDC_CHEATS_ADDCODE_USE_HEX,            &CodeDialog::OnCmdHex         },
			{ IDC_CHEATS_ADDCODE_USE_RAW,            &CodeDialog::OnCmdType        },
			{ IDC_CHEATS_ADDCODE_USE_GENIE,          &CodeDialog::OnCmdType        },
			{ IDC_CHEATS_ADDCODE_USE_ROCKY,          &CodeDialog::OnCmdType        },
			{ IDC_CHEATS_ADDCODE_SEARCH_NONE,        &CodeDialog::OnSearchType     },
			{ IDC_CHEATS_ADDCODE_SEARCH_R0_A_R1_B,   &CodeDialog::OnSearchType     },
			{ IDC_CHEATS_ADDCODE_SEARCH_R0_A_R0R1_B, &CodeDialog::OnSearchType     },
			{ IDC_CHEATS_ADDCODE_SEARCH_R0R1_B,      &CodeDialog::OnSearchType     },
			{ IDC_CHEATS_ADDCODE_SEARCH_R0_L_R1,     &CodeDialog::OnSearchType     },
			{ IDC_CHEATS_ADDCODE_SEARCH_R0_G_R1,     &CodeDialog::OnSearchType     },
			{ IDC_CHEATS_ADDCODE_SEARCH_R0_N_R1,     &CodeDialog::OnSearchType     },
			{ IDC_CHEATS_ADDCODE_SEARCH_RESET,       &CodeDialog::OnCmdReset       }
		};

		Cheats::MainDialog::Section::CodeDialog::CodeDialog(Managers::Emulator& e,const Managers::Paths& p,Context& c)
		:
		context             (c),
		emulator            (e),
		paths               (p),
		dialog              (IDD_CHEATS_ADDCODE,this,Handlers::messages,Handlers::commands),
		notificationHandler (IDC_CHEATS_ADDCODE_SEARCH_LIST,dialog.Messages())
		{
			static const Control::NotificationHandler::Entry<CodeDialog> notifications[] =
			{
				{ LVN_ITEMCHANGED, &CodeDialog::OnItemChanged }
			};

			notificationHandler.Add( this, notifications );
		}

		ibool Cheats::MainDialog::Section::CodeDialog::OnInitDialog(Param&)
		{
			dialog.Edit( IDC_CHEATS_ADDCODE_ADDRESS ).Limit( 4   );
			dialog.Edit( IDC_CHEATS_ADDCODE_DESC    ).Limit( 256 );
			dialog.Edit( IDC_CHEATS_ADDCODE_GENIE   ).Limit( 8   );
			dialog.Edit( IDC_CHEATS_ADDCODE_ROCKY   ).Limit( 8   );
			dialog.Edit( IDC_CHEATS_ADDCODE_GAME    ).Limit( 8   );

			dialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW   ).Check( true  );
			dialog.RadioButton( IDC_CHEATS_ADDCODE_USE_GENIE ).Check( false );
			dialog.RadioButton( IDC_CHEATS_ADDCODE_USE_ROCKY ).Check( false );

			dialog.Control( IDC_CHEATS_ADDCODE_GAME_CURRENT ).Enable( Nes::Cartridge(emulator).GetProfile() );

			UpdateInput();

			if (emulator.IsGameOn())
			{
				Control::ListView listView( dialog.ListView(IDC_CHEATS_ADDCODE_SEARCH_LIST) );

				listView.StyleEx() = LVS_EX_FULLROWSELECT;

				static wcstring const columns[] =
				{
					L"Index", L"R0", L"R1"
				};

				listView.Columns().Set( columns );

				if (context.searcher.filter == Searcher::NO_FILTER)
				{
					context.searcher.filter = IDC_CHEATS_ADDCODE_SEARCH_NONE;
					std::memcpy( context.searcher.ram, Nes::Cheats(emulator).GetRam(), Nes::Cheats::RAM_SIZE );

					dialog.Control( IDC_CHEATS_ADDCODE_SEARCH_RESET ).Disable();
				}
				else if (std::memcmp( context.searcher.ram, Nes::Cheats(emulator).GetRam(), Nes::Cheats::RAM_SIZE ) == 0)
				{
					dialog.Control( IDC_CHEATS_ADDCODE_SEARCH_RESET ).Disable();
				}

				dialog.RadioButton( context.searcher.filter ).Check();
			}
			else
			{
				NST_COMPILE_ASSERT
				(
					IDC_CHEATS_ADDCODE_SEARCH_TEXT_B      - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A ==  1 &&
					IDC_CHEATS_ADDCODE_SEARCH_A           - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A ==  2 &&
					IDC_CHEATS_ADDCODE_SEARCH_B           - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A ==  3 &&
					IDC_CHEATS_ADDCODE_SEARCH_LIST        - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A ==  4 &&
					IDC_CHEATS_ADDCODE_SEARCH_NONE        - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A ==  5 &&
					IDC_CHEATS_ADDCODE_SEARCH_R0_A_R1_B   - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A ==  6 &&
					IDC_CHEATS_ADDCODE_SEARCH_R0_A_R0R1_B - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A ==  7 &&
					IDC_CHEATS_ADDCODE_SEARCH_R0R1_B      - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A ==  8 &&
					IDC_CHEATS_ADDCODE_SEARCH_R0_L_R1     - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A ==  9 &&
					IDC_CHEATS_ADDCODE_SEARCH_R0_G_R1     - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A == 10 &&
					IDC_CHEATS_ADDCODE_SEARCH_R0_N_R1     - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A == 11 &&
					IDC_CHEATS_ADDCODE_SEARCH_RESET       - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A == 12
				);

				for (uint i=IDC_CHEATS_ADDCODE_SEARCH_TEXT_A; i <= IDC_CHEATS_ADDCODE_SEARCH_RESET; ++i)
					dialog.Control( i ).Disable();
			}

			dialog.CheckBox( IDC_CHEATS_ADDCODE_USE_HEX ).Check( context.hex );
			UpdateHexView( false );

			if (context.edit)
			{
				const NesCode nesCode( context.code.ToNesCode() );

				SetRawCode( nesCode );
				SetGenieCode( nesCode );
				SetRockyCode( nesCode );

				if (context.code.crc)
					dialog.Edit( IDC_CHEATS_ADDCODE_GAME ) << HexString( 32, context.code.crc, true ).Ptr();

				if (context.code.description.Length())
					dialog.Edit( IDC_CHEATS_ADDCODE_DESC ) << context.code.description.Ptr();
			}

			return true;
		}

		ibool Cheats::MainDialog::Section::CodeDialog::OnDestroy(Param&)
		{
			context.searcher.a = GetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_A );
			context.searcher.b = GetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_B );

			return true;
		}

		ibool Cheats::MainDialog::Section::CodeDialog::OnCmdSubmit(Param& param)
		{
			if (param.Button().Clicked())
			{
				bool result;
				NesCode nesCode;

				if (dialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW ).Checked())
				{
					result = GetRawCode( nesCode );
				}
				else if (dialog.RadioButton( IDC_CHEATS_ADDCODE_USE_GENIE ).Checked())
				{
					result = GetGenieCode( nesCode );
				}
				else
				{
					result = GetRockyCode( nesCode );
				}

				if (result)
				{
					context.code.FromNesCode( nesCode );

					HeapString crc;

					if (dialog.Edit( IDC_CHEATS_ADDCODE_GAME ) >> crc)
					{
						crc.Insert( 0, "0x" );
						crc >> context.code.crc;
					}
					else
					{
						context.code.crc = 0;
					}

					dialog.Edit( IDC_CHEATS_ADDCODE_DESC ) >> context.code.description;

					dialog.Close( true );
				}
				else
				{
					User::Warn( IDS_CHEATS_INVALID_CODE, IDS_CHEATS );
				}
			}

			return true;
		}

		ibool Cheats::MainDialog::Section::CodeDialog::OnCmdValidate(Param& param)
		{
			if (param.Button().Clicked())
			{
				uint id;
				NesCode code;

				if (dialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW ).Checked())
				{
					id = GetRawCode( code ) ? IDC_CHEATS_ADDCODE_USE_RAW : 0;
				}
				else if (dialog.RadioButton( IDC_CHEATS_ADDCODE_USE_GENIE ).Checked())
				{
					id = GetGenieCode( code ) ? IDC_CHEATS_ADDCODE_USE_GENIE : 0;
				}
				else
				{
					id = GetRockyCode( code ) ? IDC_CHEATS_ADDCODE_USE_ROCKY : 0;
				}

				if (id)
				{
					if (id != IDC_CHEATS_ADDCODE_USE_RAW)
						SetRawCode( code );

					if (id != IDC_CHEATS_ADDCODE_USE_GENIE)
						SetGenieCode( code );

					if (id != IDC_CHEATS_ADDCODE_USE_ROCKY)
						SetRockyCode( code );
				}

				dialog.Edit( IDC_CHEATS_ADDCODE_RESULT ) << Resource::String(id ? IDS_CHEATS_RESULT_VALID : IDS_CHEATS_RESULT_INVALID);
			}

			return true;
		}

		ibool Cheats::MainDialog::Section::CodeDialog::OnCmdGameCurrent(Param& param)
		{
			if (param.Button().Clicked())
			{
				if (const Nes::Cartridge::Profile* const profile = Nes::Cartridge(emulator).GetProfile())
					dialog.Edit( IDC_CHEATS_ADDCODE_GAME ) << HexString( 32, profile->hash.GetCrc32(), true ).Ptr();
			}

			return true;
		}

		ibool Cheats::MainDialog::Section::CodeDialog::OnCmdGameBrowse(Param& param)
		{
			if (param.Button().Clicked())
			{
				Managers::Paths::File file;

				if (paths.Load( file, Managers::Paths::File::CARTRIDGE|Managers::Paths::File::ARCHIVE  ))
				{
					bool loaded = false;

					Io::Stream::In stream( file.data );
					Nes::Cartridge::Profile profile;

					switch (file.type)
					{
						case Managers::Paths::File::INES: loaded = NES_SUCCEEDED(Nes::Cartridge::ReadInes( stream, Nes::Machine::FAVORED_NES_NTSC, profile )); break;
						case Managers::Paths::File::UNIF: loaded = NES_SUCCEEDED(Nes::Cartridge::ReadUnif( stream, Nes::Machine::FAVORED_NES_NTSC, profile )); break;
						case Managers::Paths::File::XML:  loaded = NES_SUCCEEDED(Nes::Cartridge::ReadRomset( stream, Nes::Machine::FAVORED_NES_NTSC, false, profile )); break;
					}

					if (loaded)
					{
						NST_VERIFY( profile.hash.GetCrc32() );

						if (const uint crc = profile.hash.GetCrc32())
							dialog.Edit( IDC_CHEATS_ADDCODE_GAME ) << HexString( 32, crc, true ).Ptr();
						else
							dialog.Edit( IDC_CHEATS_ADDCODE_GAME ).Clear();
					}
					else
					{
						User::Fail( IDS_FILE_ERR_INVALID );
					}
				}
			}

			return true;
		}

		ibool Cheats::MainDialog::Section::CodeDialog::OnCmdHex(Param& param)
		{
			if (param.Button().Clicked())
				UpdateHexView( true );

			return true;
		}

		ibool Cheats::MainDialog::Section::CodeDialog::OnCmdType(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint cmd = param.Button().GetId();

				dialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW   ).Check( cmd == IDC_CHEATS_ADDCODE_USE_RAW   );
				dialog.RadioButton( IDC_CHEATS_ADDCODE_USE_GENIE ).Check( cmd == IDC_CHEATS_ADDCODE_USE_GENIE );
				dialog.RadioButton( IDC_CHEATS_ADDCODE_USE_ROCKY ).Check( cmd == IDC_CHEATS_ADDCODE_USE_ROCKY );

				UpdateInput();
			}

			return true;
		}

		ibool Cheats::MainDialog::Section::CodeDialog::OnSearchType(Param& param)
		{
			if (param.Button().Clicked())
			{
				context.searcher.filter = param.Button().GetId();
				UpdateSearchList();
			}

			return true;
		}

		ibool Cheats::MainDialog::Section::CodeDialog::OnCmdReset(Param& param)
		{
			if (param.Button().Clicked())
			{
				dialog.Control( IDC_CHEATS_ADDCODE_SEARCH_RESET ).Disable();
				std::memcpy( context.searcher.ram, Nes::Cheats(emulator).GetRam(), Nes::Cheats::RAM_SIZE );
				UpdateSearchList();
			}

			return true;
		}

		void Cheats::MainDialog::Section::CodeDialog::OnItemChanged(const NMHDR& nmhdr)
		{
			const NMLISTVIEW& nm = reinterpret_cast<const NMLISTVIEW&>(nmhdr);

			if ((nm.uNewState & LVIS_SELECTED) > (nm.uOldState & LVIS_SELECTED))
			{
				NST_VERIFY( nm.lParam <= 0xFFFF );

				if (nm.lParam <= 0xFFFF)
				{
					dialog.Edit( IDC_CHEATS_ADDCODE_ADDRESS ) << HexString( 16, nm.lParam, true ).Ptr();

					if (dialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW ).Unchecked())
					{
						dialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW   ).Check( true  );
						dialog.RadioButton( IDC_CHEATS_ADDCODE_USE_GENIE ).Check( false );
						dialog.RadioButton( IDC_CHEATS_ADDCODE_USE_ROCKY ).Check( false );

						UpdateInput();
					}
				}
			}
		}

		void Cheats::MainDialog::Section::CodeDialog::UpdateInput() const
		{
			const bool raw = dialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW ).Checked();
			const bool genie = !raw && dialog.RadioButton( IDC_CHEATS_ADDCODE_USE_GENIE ).Checked();
			const bool rocky = !raw && !genie;

			dialog.Control( IDC_CHEATS_ADDCODE_VALUE   ).Enable( raw   );
			dialog.Control( IDC_CHEATS_ADDCODE_COMPARE ).Enable( raw   );
			dialog.Control( IDC_CHEATS_ADDCODE_ADDRESS ).Enable( raw   );
			dialog.Control( IDC_CHEATS_ADDCODE_GENIE   ).Enable( genie );
			dialog.Control( IDC_CHEATS_ADDCODE_ROCKY   ).Enable( rocky );
		}

		void Cheats::MainDialog::Section::CodeDialog::UpdateHexView(bool changed) const
		{
			NesCode code;

			if (changed)
			{
				changed = GetRawCode( code );

				if (emulator.IsGameOn())
				{
					context.searcher.a = GetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_A );
					context.searcher.b = GetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_B );
				}

				context.hex = dialog.CheckBox( IDC_CHEATS_ADDCODE_USE_HEX ).Checked();
			}

			const uint digits = context.hex ? 2 : 3;

			dialog.Edit( IDC_CHEATS_ADDCODE_VALUE   ).Limit( digits );
			dialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ).Limit( digits );
			dialog.Edit( IDC_CHEATS_ADDCODE_VALUE   ).SetNumberOnly( digits == 3 );
			dialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ).SetNumberOnly( digits == 3 );

			if (changed)
			{
				SetRawCode( code );
			}
			else
			{
				dialog.Edit( IDC_CHEATS_ADDCODE_VALUE ).Clear();
				dialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ).Clear();
			}

			if (emulator.IsGameOn())
			{
				dialog.Edit( IDC_CHEATS_ADDCODE_SEARCH_A ).Limit( digits );
				dialog.Edit( IDC_CHEATS_ADDCODE_SEARCH_B ).Limit( digits );
				dialog.Edit( IDC_CHEATS_ADDCODE_SEARCH_A ).SetNumberOnly( digits == 3 );
				dialog.Edit( IDC_CHEATS_ADDCODE_SEARCH_B ).SetNumberOnly( digits == 3 );

				SetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_A, context.searcher.a );
				SetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_B, context.searcher.b );

				UpdateSearchList();
			}
		}

		void Cheats::MainDialog::Section::CodeDialog::UpdateSearchList() const
		{
			Application::Instance::Waiter wait;

			Control::ListView list( dialog.ListView(IDC_CHEATS_ADDCODE_SEARCH_LIST) );

			const uchar values[] =
			{
				GetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_A ),
				GetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_B )
			};

			list.Clear();
			list.Reserve( Nes::Cheats::RAM_SIZE );

			Nes::Cheats::Ram ram = Nes::Cheats(emulator).GetRam();

			switch (context.searcher.filter)
			{
				case IDC_CHEATS_ADDCODE_SEARCH_NONE:

					for (uint i=0; i < Nes::Cheats::RAM_SIZE; ++i)
						AddSearchEntry( list, i );

					break;

				case IDC_CHEATS_ADDCODE_SEARCH_R0_N_R1:

					for (uint i=0; i < Nes::Cheats::RAM_SIZE; ++i)
					{
						if (context.searcher.ram[i] != ram[i])
							AddSearchEntry( list, i );
					}
					break;

				case IDC_CHEATS_ADDCODE_SEARCH_R0_L_R1:

					for (uint i=0; i < Nes::Cheats::RAM_SIZE; ++i)
					{
						if (context.searcher.ram[i] < ram[i])
							AddSearchEntry( list, i );
					}
					break;

				case IDC_CHEATS_ADDCODE_SEARCH_R0_G_R1:

					for (uint i=0; i < Nes::Cheats::RAM_SIZE; ++i)
					{
						if (context.searcher.ram[i] > ram[i])
							AddSearchEntry( list, i );
					}
					break;

				case IDC_CHEATS_ADDCODE_SEARCH_R0_A_R1_B:

					for (uint i=0; i < Nes::Cheats::RAM_SIZE; ++i)
					{
						if (context.searcher.ram[i] == values[0] && ram[i] == values[1])
							AddSearchEntry( list, i );
					}
					break;

				case IDC_CHEATS_ADDCODE_SEARCH_R0_A_R0R1_B:

					for (uint i=0; i < Nes::Cheats::RAM_SIZE; ++i)
					{
						if (context.searcher.ram[i] == values[0] && ((context.searcher.ram[i] - ram[i]) & 0xFFU) == values[1])
							AddSearchEntry( list, i );
					}
					break;

				case IDC_CHEATS_ADDCODE_SEARCH_R0R1_B:

					for (uint i=0; i < Nes::Cheats::RAM_SIZE; ++i)
					{
						if (((context.searcher.ram[i] - ram[i]) & 0xFFU) == values[1])
							AddSearchEntry( list, i );
					}
					break;
			}

			list.Columns().Align();
		}

		bool Cheats::MainDialog::Section::CodeDialog::GetRawCode(NesCode& code) const
		{
			HeapString string;

			if (!(dialog.Edit( IDC_CHEATS_ADDCODE_ADDRESS ) >> string))
				return false;

			string.Insert( 0, "0x" );

			uint value;

			if (!(string >> value) || value > 0xFFFF)
				return false;

			code.address = value;

			if (!(dialog.Edit( IDC_CHEATS_ADDCODE_VALUE ) >> string))
				return false;

			if (context.hex)
				string.Insert( 0, "0x" );

			if (!(string >> value) || value > 0xFF)
				return false;

			code.value = value;

			if (dialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ) >> string)
			{
				if (context.hex)
					string.Insert( 0, "0x" );

				if (!(string >> value) || value > 0xFF)
					return false;

				code.compare = value;
				code.useCompare = true;
			}
			else
			{
				code.compare = 0;
				code.useCompare = false;
			}

			return true;
		}

		void Cheats::MainDialog::Section::CodeDialog::SetRawCode(const NesCode& code) const
		{
			dialog.Edit( IDC_CHEATS_ADDCODE_ADDRESS ) << HexString( 16, code.address, true ).Ptr();

			if (context.hex)
				dialog.Edit( IDC_CHEATS_ADDCODE_VALUE ) << HexString( 8, code.value, true ).Ptr();
			else
				dialog.Edit( IDC_CHEATS_ADDCODE_VALUE ) << uint(code.value);

			if (!code.useCompare)
			{
				dialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ).Clear();
			}
			else if (context.hex)
			{
				dialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ) << HexString( 8, code.compare, true ).Ptr();
			}
			else
			{
				dialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ) << uint(code.compare);
			}
		}

		bool Cheats::MainDialog::Section::CodeDialog::GetGenieCode(NesCode& code) const
		{
			String::Heap<char> string;
			dialog.Edit( IDC_CHEATS_ADDCODE_GENIE ) >> string;
			return NES_SUCCEEDED(Nes::Cheats::GameGenieDecode( string.Ptr(), code ));
		}

		void Cheats::MainDialog::Section::CodeDialog::SetGenieCode(const NesCode& code) const
		{
			char characters[8+1];

			if (NES_SUCCEEDED(Nes::Cheats::GameGenieEncode( code, characters )))
				dialog.Edit( IDC_CHEATS_ADDCODE_GENIE ) << characters;
			else
				dialog.Edit( IDC_CHEATS_ADDCODE_GENIE ).Clear();
		}

		bool Cheats::MainDialog::Section::CodeDialog::GetRockyCode(NesCode& code) const
		{
			String::Heap<char> string;
			dialog.Edit( IDC_CHEATS_ADDCODE_ROCKY ) >> string;
			return NES_SUCCEEDED(Nes::Cheats::ProActionRockyDecode( string.Ptr(), code ));
		}

		void Cheats::MainDialog::Section::CodeDialog::SetRockyCode(const NesCode& code) const
		{
			char characters[8+1];

			if (NES_SUCCEEDED(Nes::Cheats::ProActionRockyEncode( code, characters )))
				dialog.Edit( IDC_CHEATS_ADDCODE_ROCKY ) << characters;
			else
				dialog.Edit( IDC_CHEATS_ADDCODE_ROCKY ).Clear();
		}

		uint Cheats::MainDialog::Section::CodeDialog::GetSearchValue(const uint id) const
		{
			uint value = 0;

			if (context.hex)
			{
				HeapString string;

				if (dialog.Edit( id ) >> string)
				{
					string.Insert( 0, "0x" );

					if (!(string >> value))
						value = 0;
				}
			}
			else
			{
				if (!(dialog.Edit( id ) >> value) || value > 0xFF)
					value = 0;
			}

			return value;
		}

		void Cheats::MainDialog::Section::CodeDialog::SetSearchValue(const uint id,const uint value) const
		{
			if (context.hex)
				dialog.Edit( id ) << HexString( 8, value, true ).Ptr();
			else
				dialog.Edit( id ) << uint(value);
		}

		void Cheats::MainDialog::Section::CodeDialog::AddSearchEntry(Control::ListView list,const uint address) const
		{
			const int index = list.Add( HexString( 16, address, true ), address );

			if (context.hex)
			{
				list[index].Text(1) << HexString( 8, context.searcher.ram[address], true ).Ptr();
				list[index].Text(2) << HexString( 8, Nes::Cheats(emulator).GetRam()[address], true ).Ptr();
			}
			else
			{
				list[index].Text(1) << String::Num<wchar_t>( uint(context.searcher.ram[address]) ).Ptr();
				list[index].Text(2) << String::Num<wchar_t>( uint(Nes::Cheats(emulator).GetRam()[address]) ).Ptr();
			}
		}

		Cheats::Code::Code()
		:
		crc     (0),
		address (0),
		compare (NO_COMPARE),
		value   (0),
		enabled (true)
		{
		}

		bool Cheats::Code::operator < (const Code& code) const
		{
			if (address < code.address)
				return true;

			if (address > code.address)
				return false;

			if (value < code.value)
				return true;

			if (value > code.value)
				return false;

			if (compare < code.compare)
				return true;

			if (compare > code.compare)
				return false;

			if ((crc ? crc : ~0U) < (code.crc ? code.crc : ~0U))
				return true;

			return false;
		}

		Cheats::NesCode Cheats::Code::ToNesCode() const
		{
			return NesCode
			(
				address,
				value,
				compare != NO_COMPARE ? compare : 0,
				compare != NO_COMPARE
			);
		}

		void Cheats::Code::FromNesCode(const NesCode& code)
		{
			address = code.address;
			value = code.value;
			compare = (code.useCompare ? code.compare : NO_COMPARE);
		}

		Cheats::Code::GenieCode Cheats::Code::ToGenieCode() const
		{
			char characters[8+1];

			if (NES_SUCCEEDED(Nes::Cheats::GameGenieEncode( ToNesCode(), characters )))
				return characters;
			else
				return L"-";
		}

		Cheats::Searcher::Searcher()
		: filter(NO_FILTER), a(0), b(0) {}
	}
}
