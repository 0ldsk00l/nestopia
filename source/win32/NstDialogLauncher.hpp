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

#ifndef NST_DIALOG_LAUNCHER_H
#define NST_DIALOG_LAUNCHER_H

#pragma once

#include <vector>
#include <map>
#include "NstCollectionBitSet.hpp"
#include "NstWindowMenu.hpp"
#include "NstWindowStatusBar.hpp"
#include "NstWindowDialog.hpp"
#include "NstDialogFind.hpp"
#include "../core/api/NstApiCartridge.hpp"
#include <CommCtrl.h>

namespace Nes
{
	using namespace Api;
}

namespace Nestopia
{
	namespace Io
	{
		class File;
	}

	namespace Managers
	{
		class Paths;
	}

	namespace Window
	{
		class Launcher
		{
		public:

			Launcher(const Nes::Cartridge::Database&,const Managers::Paths&,const Configuration&);
			~Launcher();

			void Save(Configuration&,bool,bool);
			void Open(bool);
			void Synchronize(HWND) const;
			void Close();

		private:

			struct Handlers;

			enum
			{
				STATUSBAR_SECOND_FIELD_WIDTH = 14,
				CMD_ENTER = 1
			};

			void UpdateItemCount(uint) const;
			void OnNoSelection() const;

			ibool OnInitDialog (Param&);
			ibool OnDropFiles  (Param&);
			ibool OnSize       (Param&);
			ibool OnCmdEnter   (Param&);
			ibool OnClose      (Param&);
			ibool OnDestroy    (Param&);

			void OnCmdFileRun                   (uint=0);
			void OnCmdFileRefresh               (uint);
			void OnCmdEditHeader                (uint);
			void OnCmdViewShowGrids             (uint);
			void OnCmdViewShowDatabaseCorrected (uint);
			void OnCmdViewShowOnTop             (uint);
			void OnCmdOptionsPaths              (uint);
			void OnCmdOptionsColors             (uint);

			void OnListGetDisplayInfo    (const NMHDR&);
			void OnListKeyDown           (const NMHDR&);
			void OnListColumnClick       (const NMHDR&);
			void OnListItemActivate      (const NMHDR&);
			void OnListItemChanged       (const NMHDR&);
			void OnListInsertItem        (const NMHDR&);
			void OnListDeleteItem        (const NMHDR&);
			void OnListDeleteAllItems    (const NMHDR&);
			void OnTreeSelectionChanging (const NMHDR&);

			class List
			{
			public:

				List
				(
					Dialog&,
					Menu::CmdHandler&,
					const Managers::Paths&,
					const Configuration&,
					const Nes::Cartridge::Database&
				);

				~List();

				void operator = (const Control::ListView&);

				enum Updater
				{
					DONT_REPAINT,
					REPAINT
				};

				void Add(wcstring);
				void Close();
				void Save(Configuration&,bool);
				void Sort(uint=0);
				bool CanRefresh() const;
				void Refresh();
				void Insert(const Param&);
				void SetColors(uint,uint,Updater=DONT_REPAINT) const;
				void OnGetDisplayInfo(LPARAM);

				class Paths
				{
				public:

					explicit Paths(const Configuration&);

					void Save(Configuration&) const;

					struct Settings
					{
						struct Folder
						{
							Path path;
							bool incSubDir;
						};

						typedef std::vector<Folder> Folders;

						struct Include : Collection::BitSet
						{
							enum
							{
								NES,UNF,XML,FDS,NSF,PATCH,ARCHIVE,ANY,UNIQUE
							};

							enum
							{
								TYPES = NES|UNF|XML|FDS|NSF|PATCH,
								FILES = TYPES|ARCHIVE
							};

							explicit Include(bool a=false)
							: Collection::BitSet( (0xFF^0x40) | uint(a) << 6 ) {}
						};

						Include include;
						Folders folders;
					};

				private:

					struct Handlers;

					enum
					{
						LIMIT = 999
					};

					ibool OnInitDialog (Param&);
					ibool OnCmdAdd     (Param&);
					ibool OnCmdRemove  (Param&);
					ibool OnCmdClear   (Param&);
					ibool OnCmdOk      (Param&);

					void OnKeyDown     (const NMHDR&);
					void OnItemChanged (const NMHDR&);
					void OnInsertItem  (const NMHDR&);
					void OnDeleteItem  (const NMHDR&);

					Settings settings;
					Dialog dialog;
					const Control::NotificationHandler notifications;

				public:

					void Open()
					{
						dialog.Open();
					}

					const Settings& GetSettings() const
					{
						return settings;
					}
				};

				class Files
				{
					class Inserter;
					class Searcher;

				public:

					class Strings
					{
						HeapString container;

					public:

						explicit Strings(uint=0);

						typedef uint Index;

						enum
						{
							NONE = -1
						};

						int  Find(GenericString) const;
						void Clear();

						template<typename T>
						Index operator << (const T& t)
						{
							uint pos = container.Length();
							container << t << '\0';
							return pos;
						}

						Index Import(cstring t)
						{
							uint pos = container.Length();
							container.Import( t );
							container << '\0';
							return pos;
						}

						wcstring operator [] (uint i) const
						{
							return container.Ptr() + i;
						}

						uint Size() const
						{
							return container.Length() * (container.Wide() ? 2 : 1);
						}
					};

					Files();

					void Load();
					void Save();
					void Refresh(const Paths::Settings&,const Nes::Cartridge::Database&);
					bool Insert(const Nes::Cartridge::Database&,GenericString);
					void Clear();
					bool ShouldDefrag() const;
					void Defrag();

					class Entry
					{
						friend class Files;
						friend class Inserter;
						friend class Searcher;

					public:

						typedef Nes::Cartridge::Database Db;

						enum Type
						{
							NES     = 0x01,
							UNF     = 0x02,
							XML     = 0x04,
							FDS     = 0x08,
							NSF     = 0x10,
							PATCH   = 0x20,
							ARCHIVE = 0x80,
							ALL     = NES|UNF|XML|FDS|NSF|PATCH
						};

						enum
						{
							SYSTEM_UNKNOWN,
							SYSTEM_PC10,
							SYSTEM_VS,
							SYSTEM_PAL,
							SYSTEM_NTSC,
							SYSTEM_NTSC_PAL
						};

						uint GetSystem(const Db*) const;

					private:

						explicit Entry(uint=0);

						Db::Entry SearchDb(const Db*) const;

						enum
						{
							ATR_BATTERY     = 0x08,
							ATR_PC10        = 0x10,
							ATR_VS          = 0x20,
							ATR_PAL         = 0x40,
							ATR_NTSC        = 0x80,
							ATR_NTSC_PAL    = ATR_PAL|ATR_NTSC,
							ATR_DEFAULT_FDS = ATR_NTSC
						};

						Strings::Index file;
						Strings::Index path;
						Strings::Index name;

						typedef Nes::Cartridge::Profile::Hash Hash;

						Hash hash;

						ushort pRom;
						ushort cRom;
						ushort wRam;
						ushort vRam;
						ushort mapper;
						uchar type;
						uchar attributes;

					public:

						uint GetType() const
						{
							return type;
						}

						wcstring GetPath(const Strings& strings) const
						{
							return strings[path];
						}

						wcstring GetFile(const Strings& strings) const
						{
							return strings[file];
						}

						wcstring GetName(const Strings& strings,const Db* db) const
						{
							if (const Db::Entry entry = SearchDb( db ))
							{
								wcstring title = entry.GetTitle();

								if (*title)
									return title;
							}

							return name ? strings[name] : L"-";
						}

						uint GetPRom(const Db* db=NULL) const
						{
							if (const Db::Entry entry = SearchDb( db ))
								return entry.GetPrgRom() / Nes::Core::SIZE_1K;
							else
								return pRom;
						}

						uint GetCRom(const Db* db=NULL) const
						{
							if (const Db::Entry entry = SearchDb( db ))
								return entry.GetChrRom() / Nes::Core::SIZE_1K;
							else
								return cRom;
						}

						uint GetWRam(const Db* db=NULL) const
						{
							if (const Db::Entry entry = SearchDb( db ))
								return entry.GetWram() / Nes::Core::SIZE_1K;
							else
								return wRam;
						}

						uint GetVRam(const Db* db=NULL) const
						{
							if (const Db::Entry entry = SearchDb( db ))
								return entry.GetVram() / Nes::Core::SIZE_1K;
							else
								return vRam;
						}

						bool GetBattery(const Db* db=NULL) const
						{
							if (const Db::Entry entry = SearchDb( db ))
								return entry.HasBattery();
							else
								return (attributes & ATR_BATTERY);
						}

						uint GetMapper(const Db* db=NULL) const
						{
							if (const Db::Entry entry = SearchDb( db ))
								return entry.GetMapper();
							else
								return mapper;
						}

						Nes::Cartridge::Profile::Dump::State GetDump(const Db* db=NULL) const
						{
							if (const Db::Entry entry = SearchDb( db ))
								return entry.GetDumpState();
							else
								return Nes::Cartridge::Profile::Dump::UNKNOWN;
						}
					};

					typedef Collection::Vector<Entry> Entries;

				private:

					enum Exception
					{
						ERR_CORRUPT_DATA
					};

					enum
					{
						MAX_ENTRIES = 0xFFFFF,
						GARBAGE_THRESHOLD = 127
					};

					ushort dirty;
					ushort loaded;
					Strings strings;
					Entries entries;

				public:

					uint Count() const
					{
						return entries.Size();
					}

					const Entry& operator [] (uint i) const
					{
						return entries[i];
					}

					void Disable(Entry* entry)
					{
						if (entry->type)
						{
							entry->type = 0;
							dirty = true;
						}
					}

					const Strings& GetStrings() const
					{
						return strings;
					}
				};

				class Columns
				{
				public:

					explicit Columns(const Configuration&);

					void Update(const uchar*);
					void Save(Configuration&) const;

					enum Type
					{
						TYPE_FILE,
						TYPE_SYSTEM,
						TYPE_MAPPER,
						TYPE_PROM,
						TYPE_CROM,
						TYPE_WRAM,
						TYPE_VRAM,
						TYPE_BATTERY,
						TYPE_DUMP,
						TYPE_NAME,
						TYPE_FOLDER,
						NUM_TYPES,
						NUM_DEFAULT_SELECTED_TYPES = 9,
						NUM_DEFAULT_AVAILABLE_TYPES = NUM_TYPES - NUM_DEFAULT_SELECTED_TYPES
					};

				private:

					struct Handlers;

					void Reset();
					void Add(uint,uint);
					void UpdateButtonRemove();
					void UpdateButtonAdd();

					ibool OnInitDialog   (Param&);
					ibool OnCmdSelected  (Param&);
					ibool OnCmdAvailable (Param&);
					ibool OnCmdAdd       (Param&);
					ibool OnCmdRemove    (Param&);
					ibool OnCmdDefault   (Param&);
					ibool OnCmdOk        (Param&);

					typedef Collection::Vector<uchar> Types;

					Types available;
					Types selected;
					Dialog dialog;

					static wcstring const cfgStrings[NUM_TYPES];

				public:

					uint Count() const
					{
						return selected.Size();
					}

					Type GetType(uint i) const
					{
						return Type(selected[i]);
					}

					uint GetStringId(uint i) const
					{
						return IDS_LAUNCHER_COLUMN_FILE + selected[i];
					}

					void Open()
					{
						dialog.Open();
					}
				};

			private:

				class Strings
				{
				public:

					wcstring GetMapper(uint);
					wcstring GetSize(uint);
					void Flush();

				private:

					struct ValueString
					{
						wchar_t string[5];
					};

					typedef String::Stack<10+1> SizeString;
					typedef std::map<uint,SizeString> Sizes;
					typedef Collection::Vector<ValueString> Mappers;

					Sizes sizes;
					Mappers mappers;
				};

				enum
				{
					STYLE =
					(
						LVS_EX_FULLROWSELECT |
						LVS_EX_TWOCLICKACTIVATE |
						LVS_EX_HEADERDRAGDROP
					)
				};

				void ReloadListColumns() const;
				void UpdateColumnOrder();
				void UpdateSortColumnOrder(uint);
				void Redraw();
				int  Sorter(const void*,const void*);
				void OnFind(GenericString,uint);
				bool Optimize();

				void OnCmdEditFind         (uint);
				void OnCmdEditInsert       (uint);
				void OnCmdEditDelete       (uint);
				void OnCmdEditClear        (uint);
				void OnCmdViewAlignColumns (uint);
				void OnCmdOptionsColumns   (uint);

				Control::ListView ctrl;

				const Nes::Cartridge::Database imageDatabase;
				const Nes::Cartridge::Database* useImageDatabase;

				uchar order[Columns::NUM_TYPES];

				uint typeFilter;
				uint style;

				Finder finder;
				Paths paths;
				Files files;
				Columns columns;
				Strings strings;
				const Managers::Paths& pathManager;

			public:

				Generic GetWindow() const
				{
					return ctrl.GetWindow();
				}

				void OpenPathDialog()
				{
					paths.Open();
				}

				void Draw(uint type)
				{
					typeFilter = type;
					Redraw();
				}

				bool DatabaseCorrectionEnabled() const
				{
					return useImageDatabase;
				}

				bool ToggleDatabase()
				{
					useImageDatabase = (useImageDatabase ? NULL : &imageDatabase);
					ctrl.Redraw();
					return useImageDatabase;
				}

				bool ToggleGrids()
				{
					style ^= LVS_EX_GRIDLINES;
					ctrl.StyleEx() = style;
					return style & LVS_EX_GRIDLINES;
				}

				bool HitTest(const Point& point) const
				{
					return ctrl.HitTest( point.x, point.y );
				}

				uint GetStyle() const
				{
					return ctrl.StyleEx();
				}

				HWND GetHandle() const
				{
					return ctrl.GetHandle();
				}

				uint NumPaths() const
				{
					return paths.GetSettings().folders.size();
				}

				uint Size() const
				{
					return ctrl.Size();
				}

				const Files::Entry* operator [] (uint i) const
				{
					return static_cast<const Files::Entry*>(static_cast<const void*>( ctrl[i].Data() ));
				}

				const Files::Entry* GetSelection() const
				{
					int index = ctrl.Selection().GetIndex();
					return index >= 0 ? (*this)[index] : NULL;
				}

				const Files::Strings& GetStrings() const
				{
					return files.GetStrings();
				}

				const Managers::Paths& GetPaths() const
				{
					return pathManager;
				}
			};

			class Tree
			{
			public:

				Tree();

				void operator = (const Control::TreeView&);

				enum Updater {DONT_REPAINT,REPAINT};

				void SetColors(uint,uint,Updater=DONT_REPAINT) const;
				uint GetType(HTREEITEM) const;
				void Close();

			private:

				Control::TreeView ctrl;
				uint selection;
				const Control::TreeView::ImageList imageList;

			public:

				Generic GetWindow() const
				{
					return ctrl.GetWindow();
				}
			};

			class Colors
			{
			public:

				explicit Colors(const Configuration&);

				void Save(Configuration&) const;

			private:

				struct Handlers;

				enum
				{
					DEF_BACKGROUND_COLOR = RGB(0xFF,0xFF,0xFF),
					DEF_FOREGROUND_COLOR = RGB(0x00,0x00,0x00)
				};

				struct Type
				{
					inline Type(int,int,int,int);

					COLORREF color;
					const Rect rect;
				};

				void UpdateColor(const Type&) const;
				void UpdateColors() const;
				void ChangeColor(COLORREF&);

				ibool OnInitDialog          (Param&);
				ibool OnPaint               (Param&);
				ibool OnCmdChangeBackground (Param&);
				ibool OnCmdChangeForeground (Param&);
				ibool OnCmdDefault          (Param&);

				Type background;
				Type foreground;

				COLORREF customColors[16];

				Dialog dialog;

			public:

				COLORREF GetBackgroundColor() const
				{
					return background.color;
				}

				COLORREF GetForegroundColor() const
				{
					return foreground.color;
				}

				void Open()
				{
					dialog.Open();
				}
			};

			Dialog dialog;
			Menu menu;
			Control::NotificationHandler listNotifications;
			Control::NotificationHandler treeNotifications;
			StatusBar statusBar;
			Tree tree;
			List list;
			Point margin;
			Colors colors;
			Point initialSize;
		};
	}
}

#endif
