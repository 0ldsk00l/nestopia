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

#include "NstWindowUser.hpp"
#include "NstWindowDropFiles.hpp"
#include "NstResourceString.hpp"
#include "NstDialogFind.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogLauncher.hpp"
#include <Shlwapi.h>

namespace Nestopia
{
	namespace Window
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		wcstring Launcher::List::Strings::GetMapper(const uint value)
		{
			if (value-1 < 4095)
			{
				uint count = mappers.Size();

				if (count <= value)
				{
					const uint size = NST_MAX(256,count+value+1);
					mappers.Resize( size );

					do
					{
						_itow( count, mappers[count].string, 10 );
					}
					while (++count != size);
				}

				return mappers[value].string;
			}
			else
			{
				return L"-";
			}
		}

		wcstring Launcher::List::Strings::GetSize(uint value)
		{
			if (value)
			{
				SizeString& string = sizes[value];

				if (string.Empty())
					string << value << 'k';

				return string.Ptr();
			}

			return L"-";
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Launcher::List::Strings::Flush()
		{
			sizes.clear();
			mappers.Destroy();
		}

		Launcher::List::List
		(
			Dialog& dialog,
			Menu::CmdHandler& cmdHandler,
			const Managers::Paths& p,
			const Configuration& cfg,
			const Nes::Cartridge::Database& database
		)
		:
		imageDatabase    ( database ),
		useImageDatabase ( NULL ),
		typeFilter       ( 0 ),
		style            ( STYLE ),
		finder           ( dialog ),
		paths            ( cfg ),
		columns          ( cfg ),
		pathManager      ( p )
		{
			static const Menu::CmdHandler::Entry<Launcher::List> commands[] =
			{
				{ IDM_LAUNCHER_EDIT_FIND,         &List::OnCmdEditFind         },
				{ IDM_LAUNCHER_EDIT_INSERT,       &List::OnCmdEditInsert       },
				{ IDM_LAUNCHER_EDIT_REMOVE,       &List::OnCmdEditDelete       },
				{ IDM_LAUNCHER_EDIT_CLEAR,        &List::OnCmdEditClear        },
				{ IDM_LAUNCHER_VIEW_ALIGNCOLUMNS, &List::OnCmdViewAlignColumns },
				{ IDM_LAUNCHER_OPTIONS_COLUMNS,   &List::OnCmdOptionsColumns   }
			};

			cmdHandler.Add( this, commands );

			Configuration::ConstSection show( cfg["launcher"]["view"]["show"] );

			if (!show["grid-lines"].No())
				style |= LVS_EX_GRIDLINES;

			if (!show["image-database-adjusted"].No())
				useImageDatabase = &imageDatabase;
		}

		Launcher::List::~List()
		{
		}

		void Launcher::List::operator = (const Control::ListView& listView)
		{
			files.Load();

			typeFilter = 0;

			ctrl = listView;
			ctrl.StyleEx() = style;

			ReloadListColumns();

			ctrl.Reserve( files.Count() );
			ctrl.Columns().Align();

			InvalidateRect( ctrl.GetWindow(), NULL, false );
		}

		void Launcher::List::Close()
		{
			finder.Close();
			UpdateColumnOrder();
			columns.Update( order );
			strings.Flush();
		}

		void Launcher::List::Insert(const Param& param)
		{
			DropFiles dropFiles( param );

			if (dropFiles.Inside( ctrl.GetHandle() ))
			{
				uint anyInserted = false;

				for (uint i=0, n=dropFiles.Size(); i < n; ++i)
					anyInserted |= uint(files.Insert( imageDatabase, dropFiles[i] ));

				if (anyInserted && !Optimize())
					Redraw();
			}
		}

		void Launcher::List::Add(wcstring const fileName)
		{
			if (files.Insert( imageDatabase, fileName ) && !Optimize())
				Redraw();
		}

		void Launcher::List::Save(Configuration& cfg,bool saveFiles)
		{
			paths.Save( cfg );
			columns.Save( cfg );

			if (saveFiles)
				files.Save();

			Configuration::Section show( cfg["launcher"]["view"]["show"] );

			show["grid-lines"].YesNo() = style & LVS_EX_GRIDLINES;
			show["image-database-adjusted"].YesNo() = useImageDatabase;
		}

		void Launcher::List::SetColors(const uint bg,const uint fg,const Updater redraw) const
		{
			ctrl.SetBkColor( bg );
			ctrl.SetTextBkColor( bg );
			ctrl.SetTextColor( fg );

			if (redraw)
				ctrl.Redraw();
		}

		void Launcher::List::Redraw()
		{
			Application::Instance::Waiter wait;
			Generic::LockDraw lock( ctrl.GetHandle() );

			ctrl.Clear();

			if (const uint count = files.Count())
			{
				uint size = 0;

				for (uint i=0; i < count; ++i)
					size += (files[i].GetType() & typeFilter) != 0;

				if (size)
				{
					ctrl.Reserve( size );

					for (uint i=0; i < count; ++i)
					{
						if (files[i].GetType() & typeFilter)
							ctrl.Add( GenericString(), &files[i] );
					}

					Sort();
					ctrl.Columns().Align();
				}
			}
		}

		bool Launcher::List::Optimize()
		{
			if (files.ShouldDefrag())
			{
				files.Defrag();
				Redraw();
				return true;
			}

			return false;
		}

		bool Launcher::List::CanRefresh() const
		{
			return
			(
				(paths.GetSettings().folders.size()) &&
				(paths.GetSettings().include.Word() & Paths::Settings::Include::TYPES)
			);
		}

		void Launcher::List::Refresh()
		{
			if (CanRefresh())
			{
				{
					Application::Instance::Waiter wait;
					ctrl.Clear();
				}

				files.Refresh( paths.GetSettings(), imageDatabase );
				ctrl.Reserve( files.Count() );
				Redraw();
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		void Launcher::List::OnGetDisplayInfo(LPARAM lParam)
		{
			LVITEM& item = reinterpret_cast<NMLVDISPINFO*>(lParam)->item;

			if (item.mask & LVIF_TEXT)
			{
				const Files::Entry& entry = *reinterpret_cast<const Files::Entry*>( item.lParam );

				switch (columns.GetType( item.iSubItem ))
				{
					case Columns::TYPE_FILE:

						item.pszText = const_cast<wchar_t*>( entry.GetFile(files.GetStrings()) );
						break;

					case Columns::TYPE_SYSTEM:
					{
						NST_COMPILE_ASSERT
						(
							Files::Entry::SYSTEM_UNKNOWN  == 0 &&
							Files::Entry::SYSTEM_PC10     == 1 &&
							Files::Entry::SYSTEM_VS       == 2 &&
							Files::Entry::SYSTEM_PAL      == 3 &&
							Files::Entry::SYSTEM_NTSC     == 4 &&
							Files::Entry::SYSTEM_NTSC_PAL == 5
						);

						static const wchar_t lut[][9] =
						{
							L"-",
							L"pc10",
							L"vs",
							L"pal",
							L"ntsc",
							L"ntsc/pal"
						};

						item.pszText = const_cast<wchar_t*>( lut[entry.GetSystem( useImageDatabase )] );
						break;
					}

					case Columns::TYPE_BATTERY:

						item.pszText = const_cast<wchar_t*>
						(
							(entry.GetType() & (Files::Entry::NES|Files::Entry::UNF)) ?
							entry.GetBattery( useImageDatabase ) ? L"yes" : L"no" : L"-"
						);
						break;

					case Columns::TYPE_DUMP:
					{
						NST_COMPILE_ASSERT
						(
							Nes::Cartridge::Profile::Dump::OK      == 0 &&
							Nes::Cartridge::Profile::Dump::BAD     == 1 &&
							Nes::Cartridge::Profile::Dump::UNKNOWN == 2
						);

						static const wchar_t lut[][4] =
						{
							L"ok",
							L"bad",
							L"-"
						};

						item.pszText = const_cast<wchar_t*>( lut[entry.GetDump( useImageDatabase )] );
						break;
					}

					case Columns::TYPE_NAME:

						item.pszText = const_cast<wchar_t*>( entry.GetName( files.GetStrings(), useImageDatabase ) );
						break;

					case Columns::TYPE_FOLDER:

						item.pszText = const_cast<wchar_t*>( entry.GetPath( files.GetStrings() ) );
						break;

					case Columns::TYPE_PROM:

						item.pszText = const_cast<wchar_t*>( strings.GetSize(entry.GetPRom(useImageDatabase)) );
						break;

					case Columns::TYPE_CROM:

						if (const uint cRom = entry.GetCRom( useImageDatabase ))
							item.pszText = const_cast<wchar_t*>( strings.GetSize( cRom ) );
						else
							item.pszText = const_cast<wchar_t*>( L"-" );
						break;

					case Columns::TYPE_MAPPER:

						item.pszText = const_cast<wchar_t*>( strings.GetMapper(entry.GetMapper( useImageDatabase )) );
						break;

					case Columns::TYPE_WRAM:

						if (const uint wRam = entry.GetWRam( useImageDatabase ))
							item.pszText = const_cast<wchar_t*>( strings.GetSize( wRam ) );
						else
							item.pszText = const_cast<wchar_t*>( L"-" );
						break;

					case Columns::TYPE_VRAM:

						if (const uint vRam = entry.GetVRam( useImageDatabase ))
							item.pszText = const_cast<wchar_t*>( strings.GetSize( vRam ) );
						else
							item.pszText = const_cast<wchar_t*>( L"-" );
						break;
				}
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Launcher::List::ReloadListColumns() const
		{
			ctrl.Columns().Clear();

			for (uint i=0; i < columns.Count(); ++i)
				ctrl.Columns().Insert( i, Resource::String(columns.GetStringId(i)).Ptr() );
		}

		void Launcher::List::UpdateColumnOrder()
		{
			int array[Columns::NUM_TYPES];
			ctrl.Columns().GetOrder( array, columns.Count() );

			for (uint i=0; i < columns.Count(); ++i)
				order[i] = columns.GetType( array[i] );
		}

		void Launcher::List::UpdateSortColumnOrder(const uint firstSortColumn)
		{
			int array[Columns::NUM_TYPES];
			ctrl.Columns().GetOrder( array, columns.Count() );

			order[0] = columns.GetType( firstSortColumn );

			for (uint i=0, j=1; i < columns.Count(); ++i)
			{
				if (firstSortColumn != array[i])
					order[j++] = columns.GetType( array[i] );
			}
		}

		void Launcher::List::Sort(const uint firstSortColumn)
		{
			if (ctrl.Size() > 1)
			{
				UpdateSortColumnOrder( firstSortColumn );
				ctrl.Sort( this, &List::Sorter );
			}
		}

		int Launcher::List::Sorter(const void* obj1,const void* obj2)
		{
			const Files::Entry& a = *static_cast<const Files::Entry*>( obj1 );
			const Files::Entry& b = *static_cast<const Files::Entry*>( obj2 );

			for (uint i=0, n=columns.Count(); i < n; ++i)
			{
				switch (order[i])
				{
					case Columns::TYPE_FILE:

						if (const int ret = ::StrCmp( a.GetFile(files.GetStrings()), b.GetFile(files.GetStrings()) ))
							return ret;

						continue;

					case Columns::TYPE_SYSTEM:
					{
						const uint system[] =
						{
							a.GetSystem( useImageDatabase ),
							b.GetSystem( useImageDatabase )
						};

						if (system[0] == system[1])
							continue;

						return system[0] < system[1] ? +1 : -1;
					}

					case Columns::TYPE_MAPPER:
					{
						const uint mapper[] =
						{
							a.GetMapper( useImageDatabase ),
							b.GetMapper( useImageDatabase )
						};

						if (mapper[0] == mapper[1])
							continue;

						return mapper[0] > mapper[1] ? +1 : -1;
					}

					case Columns::TYPE_PROM:
					{
						const uint pRom[] =
						{
							a.GetPRom( useImageDatabase ),
							b.GetPRom( useImageDatabase )
						};

						if (pRom[0] == pRom[1])
							continue;

						return pRom[0] > pRom[1] ? +1 : -1;
					}

					case Columns::TYPE_CROM:
					{
						const uint cRom[] =
						{
							a.GetCRom( useImageDatabase ),
							b.GetCRom( useImageDatabase )
						};

						if (cRom[0] == cRom[1])
							continue;

						return cRom[0] > cRom[1] ? +1 : -1;
					}

					case Columns::TYPE_WRAM:
					{
						const uint wRam[] =
						{
							a.GetWRam( useImageDatabase ),
							b.GetWRam( useImageDatabase )
						};

						if (wRam[0] == wRam[1])
							continue;

						return wRam[0] > wRam[1] ? +1 : -1;
					}

					case Columns::TYPE_VRAM:
					{
						const uint vRam[] =
						{
							a.GetVRam( useImageDatabase ),
							b.GetVRam( useImageDatabase )
						};

						if (vRam[0] == vRam[1])
							continue;

						return vRam[0] > vRam[1] ? +1 : -1;
					}

					case Columns::TYPE_BATTERY:
					{
						const uint battery[] =
						{
							a.GetBattery( useImageDatabase ) + bool(a.GetType() & (List::Files::Entry::NES|List::Files::Entry::UNF)),
							b.GetBattery( useImageDatabase ) + bool(b.GetType() & (List::Files::Entry::NES|List::Files::Entry::UNF))
						};

						if (battery[0] == battery[1])
							continue;

						return battery[0] < battery[1] ? +1 : -1;
					}

					case Columns::TYPE_DUMP:
					{
						const uint dump[] =
						{
							a.GetDump( useImageDatabase ),
							b.GetDump( useImageDatabase )
						};

						if (dump[0] == dump[1])
							continue;

						return dump[0] > dump[1] ? +1 : -1;
					}

					case Columns::TYPE_NAME:
					{
						wcstring const names[] =
						{
							a.GetName( files.GetStrings(), useImageDatabase ),
							b.GetName( files.GetStrings(), useImageDatabase )
						};

						if (names[0][0] != '-' && names[1][0] == '-') return -1;
						if (names[0][0] == '-' && names[1][0] != '-') return +1;

						if (const int ret = ::StrCmp( names[0], names[1] ))
							return ret;

						continue;
					}

					case Columns::TYPE_FOLDER:

						if (const int ret = ::StrCmp( a.GetPath(files.GetStrings()), b.GetPath(files.GetStrings()) ))
							return ret;

						continue;
				}
			}

			return 0;
		}

		void Launcher::List::OnFind(GenericString string,const uint flags)
		{
			const uint count = ctrl.Size();

			if (count > 1 && string.Length())
			{
				const uint column = ctrl.Columns().GetIndex(0);
				const int selection = ctrl.Selection().GetIndex();
				const uint wrap = selection > 0 ? selection : 0;
				uint index = wrap;
				bool found;

				HeapString item;

				do
				{
					if (flags & Finder::DOWN)
					{
						if (++index == count)
							index = 0;
					}
					else
					{
						if (--index == ~0U)
							index = count - 1;
					}

					ctrl[index].Text( column ) >> item;

					if (flags & Finder::WHOLEWORD)
					{
						found = item.Length() == string.Length() && ::StrIsIntlEqual( (flags & Finder::MATCHCASE), item.Ptr(), string.Ptr(), string.Length() );
					}
					else if (flags & Finder::MATCHCASE)
					{
						found = ::StrStr( item.Ptr(), string.Ptr() );
					}
					else
					{
						found = ::StrStrI( item.Ptr(), string.Ptr() );
					}
				}
				while (!found && index != wrap);

				if (found)
				{
					if (selection >= 0)
						ctrl[selection].Select( false );

					ctrl[index].Select();
					ctrl[index].Show();
				}
				else
				{
					User::Inform( IDS_TEXT_SEARCH_NOT_FOUND, IDS_TEXT_FIND );
				}
			}
		}

		void Launcher::List::OnCmdEditFind(uint)
		{
			finder.Open( this, &List::OnFind );
		}

		void Launcher::List::OnCmdEditInsert(uint)
		{
			enum
			{
				FILE_TYPES =
				(
					Managers::Paths::File::IMAGE |
					Managers::Paths::File::PATCH |
					Managers::Paths::File::ARCHIVE
				)
			};

			Add( pathManager.BrowseLoad( FILE_TYPES, GenericString(), Managers::Paths::DONT_CHECK_FILE ).Ptr() );
		}

		void Launcher::List::OnCmdEditDelete(uint)
		{
			Application::Instance::Waiter wait;

			int last = -1;

			for (int index = ctrl.Selection().GetIndex(); index != -1; index = ctrl.Selection().GetIndex())
			{
				last = index;
				void* const entry = ctrl[index].Data();
				ctrl[index].Delete();
				files.Disable( static_cast<Files::Entry*>(entry) );
			}

			if (ctrl.Size())
			{
				if (last != -1)
					ctrl[last].Select();
			}
			else if (typeFilter == Files::Entry::ALL)
			{
				files.Clear();
			}
		}

		void Launcher::List::OnCmdEditClear(uint)
		{
			Application::Instance::Waiter wait;
			ctrl.Clear();
			files.Clear();
		}

		void Launcher::List::OnCmdViewAlignColumns(uint)
		{
			Application::Instance::Waiter wait;
			ctrl.Columns().Align();
		}

		void Launcher::List::OnCmdOptionsColumns(uint)
		{
			UpdateColumnOrder();

			columns.Update( order );
			columns.Open();

			Application::Instance::Waiter wait;
			Generic::LockDraw lock( ctrl.GetHandle() );

			ReloadListColumns();
			Sort();
			ctrl.Columns().Align();
		}
	}
}
