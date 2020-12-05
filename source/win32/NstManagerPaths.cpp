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

#include "NstIoFile.hpp"
#include "NstIoArchive.hpp"
#include "NstIoScreen.hpp"
#include "NstWindowUser.hpp"
#include "NstResourceString.hpp"
#include "NstDialogBrowse.hpp"
#include "NstDialogPaths.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerPathsFilter.hpp"

namespace Nestopia
{
	namespace Managers
	{
		Paths::File::File()
		: type(NONE)
		{
		}

		Paths::File::~File()
		{
		}

		Paths::Paths(Emulator& e,const Configuration& cfg,Window::Menu& m)
		:
		Manager ( e, m, this, &Paths::OnEmuEvent, IDM_OPTIONS_PATHS, &Paths::OnMenu ),
		dialog  ( new Window::Paths(cfg) )
		{
			recentImageDir = cfg["paths"]["images"]["recent-directory"].Str();

			UpdateSettings();
		}

		Paths::~Paths()
		{
		}

		void Paths::Save(Configuration& cfg) const
		{
			dialog->Save( cfg );

			if (recentImageDir.DirectoryExists())
				cfg["paths"]["images"]["recent-directory"].Str() = recentImageDir;
		}

		void Paths::UpdateSettings()
		{
			emulator.WriteProtectCartridge
			(
				dialog->GetSetting(Window::Paths::READONLY_CARTRIDGE)
			);
		}

		void Paths::OnMenu(uint)
		{
			dialog->Open();
			UpdateSettings();
		}

		void Paths::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_NETPLAY_MODE:

					menu[IDM_OPTIONS_PATHS].Enable( !data );
					break;
			}
		}

		bool Paths::SaveSlotExportingEnabled() const
		{
			return dialog->GetSetting(Window::Paths::AUTO_EXPORT_STATE_SLOTS);
		}

		bool Paths::SaveSlotImportingEnabled() const
		{
			return dialog->GetSetting(Window::Paths::AUTO_IMPORT_STATE_SLOTS);
		}

		bool Paths::AutoLoadCheatsEnabled() const
		{
			return dialog->GetSetting(Window::Paths::CHEATS_AUTO_LOAD);
		}

		bool Paths::AutoSaveCheatsEnabled() const
		{
			return dialog->GetSetting(Window::Paths::CHEATS_AUTO_SAVE);
		}

		bool Paths::UseStateCompression() const
		{
			return dialog->GetSetting(Window::Paths::COMPRESS_STATES);
		}

		bool Paths::BypassPatchValidation() const
		{
			return dialog->GetSetting(Window::Paths::PATCH_BYPASS_VALIDATION);
		}

		bool Paths::LocateFile(Path& path,const File::Types types) const
		{
			NST_ASSERT( path.File().Length() );

			static const struct { uint type; wchar_t extension[4]; } lut[19] =
			{
				{ File::INES,              L"nes" },
				{ File::UNIF,              L"unf" },
				{ File::FDS,               L"fds" },
				{ File::NSF,               L"nsf" },
				{ File::BATTERY,           L"sav" },
				{ File::TAPE,              L"tp"  },
				{ File::STATE|File::SLOTS, L"nst" },
				{ File::IPS,               L"ips" },
				{ File::UPS,               L"ups" },
				{ File::MOVIE,             L"nsv" },
				{ File::ROM,               L"rom" },
				{ File::XML,               L"xml" },
				{ File::PALETTE,           L"pal" },
				{ File::WAVE,              L"wav" },
				{ File::AVI,               L"avi" },
				{ File::ARCHIVE,           L"zip" },
				{ File::ARCHIVE,           L"rar" },
				{ File::ARCHIVE,           L"7z"  }
			};

			for (uint i=0; i < 18; ++i)
			{
				if (types( lut[i].type ))
				{
					path.Directory() = GetDefaultDirectory( lut[i].type );
					path.Extension() = lut[i].extension;

					if (path.FileExists())
						return true;
				}
			}

			return false;
		}

		bool Paths::FindFile(Path& path) const
		{
			NST_ASSERT( path.File().Length() );

			path.Directory() = Application::Instance::GetExePath().Directory();

			if (path.FileExists())
				return true;

			for (uint i=0; i < Window::Paths::NUM_DIRS; ++i)
			{
				path.Directory() = dialog->GetDirectory( static_cast<Window::Paths::Type>(i) );

				if (path.FileExists())
					return true;
			}

			path.Directory() = recentImageDir;

			if (path.FileExists())
				return true;

			path.Directory().Clear();

			return false;
		}

		void Paths::UpdateRecentImageDirectory(const Path& path,const File::Types types) const
		{
			NST_ASSERT( path.Length() );

			if (types( File::IMAGE ))
				recentImageDir = path.Directory();
		}

		Path Paths::GetScreenShotPath() const
		{
			Path path;

			path.Directory() = dialog->GetDirectory( Window::Paths::DIR_SCREENSHOT );
			path.File() = emulator.GetImagePath().Target().File();
			path.Extension().Clear();

			const uint offset = path.Length() + 1;
			path << L"_xxx." << dialog->GetScreenShotExtension();

			for (uint i=1; i < 1000; ++i)
			{
				path[offset+0] = '0' + (i / 100);
				path[offset+1] = '0' + (i % 100 / 10);
				path[offset+2] = '0' + (i % 10);

				if (!path.FileExists())
					break;
			}

			return path;
		}

		wcstring Paths::GetDefaultExtension(const File::Types types)
		{
			return
			(
				types( File::INES              ) ? L"nes" :
				types( File::UNIF              ) ? L"unf" :
				types( File::FDS               ) ? L"fds" :
				types( File::NSF               ) ? L"nsf" :
				types( File::XML               ) ? L"xml" :
				types( File::BATTERY           ) ? L"sav" :
				types( File::TAPE              ) ? L"tp"  :
				types( File::STATE|File::SLOTS ) ? L"nst" :
				types( File::IPS               ) ? L"ips" :
				types( File::UPS               ) ? L"ups" :
				types( File::MOVIE             ) ? L"nsv" :
				types( File::ROM               ) ? L"rom" :
				types( File::ROM               ) ? L"xml" :
				types( File::PALETTE           ) ? L"pal" :
				types( File::WAVE              ) ? L"wav" :
				types( File::AVI               ) ? L"avi" :
				types( File::ARCHIVE           ) ? L"zip" :
                                                   L""
			);
		}

		const Path Paths::GetDefaultDirectory(const File::Types types) const
		{
			Window::Paths::Type type;

			if (types( File::IMAGE|File::ROM ))
			{
				if (dialog->GetSetting(Window::Paths::USE_LAST_IMAGE_DIR) && recentImageDir.DirectoryExists())
					return recentImageDir;

				type = Window::Paths::DIR_IMAGE;
			}
			else if (types( File::STATE|File::SLOTS|File::MOVIE ))
			{
				type = Window::Paths::DIR_STATE;
			}
			else if (types( File::BATTERY|File::TAPE ))
			{
				type = Window::Paths::DIR_SAVE;
			}
			else if (types( File::PATCH ))
			{
				type = Window::Paths::DIR_PATCHES;
			}
			else
			{
				return Application::Instance::GetExePath().Directory();
			}

			return dialog->GetDirectory( type );
		}

		bool Paths::CheckFile(Path& path,const File::Types types,const Alert alert,const uint title) const
		{
			NST_ASSERT( types.Word() );

			try
			{
				if (LoadFromFile( path, NULL, types ))
					return true;
			}
			catch (int ids)
			{
				if (alert == NOISY)
				{
					Window::User::Fail( ids, title );
				}
				else if (alert == STICKY)
				{
					Io::Screen() << Resource::String(title) << ' ' << Resource::String(ids);
				}
			}

			path.Clear();
			return false;
		}

		Path Paths::BrowseLoad(const File::Types types,const GenericString dir,const Checking checking) const
		{
			NST_ASSERT( types.Word() );

			Path path
			(
				Window::Browser::OpenFile
				(
					Filter( types ).Ptr(),
					dir.Length() ? Path(dir) : GetDefaultDirectory( types ),
					GetDefaultExtension( types )
				)
			);

			if (path.Length())
			{
				UpdateRecentImageDirectory( path, types );

				if (checking == CHECK_FILE)
					CheckFile( path, types, NOISY );
			}

			return path;
		}

		Path Paths::BrowseSave(const File::Types types,const Method method,const GenericString initPath) const
		{
			NST_ASSERT( types.Word() );

			Path path( initPath );

			if (path.Directory().Empty())
				path.Directory() = GetDefaultDirectory( types );

			if (method == SUGGEST && path.File().Empty())
				path.File() = emulator.GetImagePath().Target().File();

			path.Extension() = GetDefaultExtension( types );
			path = Window::Browser::SaveFile( Filter(types).Ptr(), path );

			if (path.Length())
				UpdateRecentImageDirectory( path, types );

			return path;
		}

		void Paths::FixFile(const File::Type type,Path& path) const
		{
			if (path.File().Length())
			{
				if (path.Directory().Empty())
					path.Directory() = GetDefaultDirectory( type );

				if (path.Extension().Empty())
					path.Extension() = GetDefaultExtension( type );
			}
			else
			{
				path.Clear();
			}
		}

		Path Paths::GetSavePath(const Path& image,const File::Type type) const
		{
			NST_ASSERT( image.Length() );

			Path save;

			if (type & File::GAME)
			{
				save.Set
				(
					dialog->GetDirectory( (type & File::CARTRIDGE) ? Window::Paths::DIR_SAVE : Window::Paths::DIR_PATCHES ),
					image.Target().File(),
					(type & File::CARTRIDGE) ? L"sav" : L"ups"
				);
			}

			return save;
		}

		Path Paths::GetCheatPath() const
		{
			return dialog->GetDirectory( Window::Paths::DIR_CHEATS );
		}

		Path Paths::GetCheatPath(const Path& image) const
		{
			return Path( dialog->GetDirectory( Window::Paths::DIR_CHEATS ), image.Target().File(), L"xml" );
		}

		Path Paths::GetPatchPath(const Path& image,const File::Type type) const
		{
			Path patch;

			if ((type & File::CARTRIDGE) && dialog->GetSetting( Window::Paths::PATCH_AUTO_APPLY ))
			{
				patch.Set( dialog->GetDirectory( Window::Paths::DIR_PATCHES ), image.Target().File(), L"ups" );

				if (!patch.FileExists())
					patch.Extension() = L"ips";
			}

			return patch;
		}

		Path Paths::GetSamplesPath() const
		{
			return dialog->GetDirectory( Window::Paths::DIR_SAMPLES );
		}

		Paths::File::Type Paths::Load
		(
			File& file,
			const File::Types types,
			const GenericString path,
			const Alert alert
		)   const
		{
			if (path.Length())
			{
				file.name = path;

				if (file.name.Directory().Empty())
					file.name.Directory() = GetDefaultDirectory( types );
			}
			else
			{
				file.name = BrowseLoad( types );
			}

			if (file.name.Empty() || (alert == QUIETLY && !file.name.FileExists()))
				return File::NONE;

			try
			{
				Application::Instance::Waiter wait;
				file.type = LoadFromFile( file.name, &file.data, types );
			}
			catch (int ids)
			{
				if (alert == NOISY)
				{
					Window::User::Fail( ids, IDS_TITLE_ERROR );
				}
				else if (alert == STICKY)
				{
					Io::Screen() << Resource::String(IDS_TITLE_ERROR) << ' ' << Resource::String(ids);
				}

				file.type = File::NONE;
			}

			if (file.type == File::NONE)
			{
				file.name.Clear();
				file.data.Clear();
			}

			return file.type;
		}

		bool Paths::Save
		(
			const void* const data,
			const uint size,
			const File::Type type,
			Path path,
			const Alert alert
		)   const
		{
			if (path.Directory().Empty())
				path.Directory() = GetDefaultDirectory( type );

			try
			{
				Io::File( path, Io::File::DUMP|Io::File::WRITE_THROUGH ).Write( data, size );
			}
			catch (Io::File::Exception ids)
			{
				if (alert == NOISY)
				{
					Window::User::Fail( ids, IDS_TITLE_ERROR );
				}
				else if (alert == STICKY)
				{
					Io::Screen() << Resource::String(IDS_TITLE_ERROR) << ' ' << Resource::String(ids);
				}

				return false;
			}

			return true;
		}

		Paths::File::Type Paths::LoadFromFile(Path& path,File::Data* const data,const File::Types types)
		{
			NST_ASSERT( path.Length() );

			const GenericString fileInArchive( path.FileInArchive() );
			GenericString filePath( path );

			if (fileInArchive.Length())
				filePath = path.Archive();

			try
			{
				Io::File file( filePath, Io::File::READ|Io::File::EXISTING );

				const File::Type type = CheckFile( types, file.Peek32(), path.Extension().Id() );

				if (type == File::NONE)
					throw IDS_FILE_ERR_INVALID;

				if (type == File::ARCHIVE)
					return LoadFromArchive( Io::Archive(file), path, data, fileInArchive, types );

				if (fileInArchive.Length())
					throw IDS_FILE_ERR_INVALID;

				if (data)
					file.Stream() >> *data;

				return type;
			}
			catch (Io::File::Exception id)
			{
				throw int(id);
			}
		}

		Paths::File::Type Paths::LoadFromArchive
		(
			const Io::Archive& archive,
			Path& path,
			File::Data* const data,
			const GenericString& fileInArchive,
			const File::Types types
		)
		{
			uint index;

			if (fileInArchive.Length())
			{
				index = archive.Find( fileInArchive );
			}
			else
			{
				uint count = 0;
				GenericString filter[32];

				if (types( File::INES    )) filter[count++] = L"nes";
				if (types( File::FDS     )) filter[count++] = L"fds";
				if (types( File::NSF     )) filter[count++] = L"nsf";
				if (types( File::BATTERY )) filter[count++] = L"sav";
				if (types( File::TAPE    )) filter[count++] = L"tp";
				if (types( File::STATE   )) filter[count++] = L"nst";
				if (types( File::MOVIE   )) filter[count++] = L"nsv";
				if (types( File::IPS     )) filter[count++] = L"ips";
				if (types( File::UPS     )) filter[count++] = L"ups";
				if (types( File::ROM     )) filter[count++] = L"rom";
				if (types( File::XML     )) filter[count++] = L"xml";
				if (types( File::PALETTE )) filter[count++] = L"pal";
				if (types( File::WAVE    )) filter[count++] = L"wav";
				if (types( File::AVI     )) filter[count++] = L"avi";

				if (types( File::UNIF ))
				{
					filter[count++] = L"unf";
					filter[count++] = L"unif";
				}

				if (types( File::SLOTS ))
				{
					filter[count++] = L"ns1";
					filter[count++] = L"ns2";
					filter[count++] = L"ns3";
					filter[count++] = L"ns4";
					filter[count++] = L"ns5";
					filter[count++] = L"ns6";
					filter[count++] = L"ns7";
					filter[count++] = L"ns8";
					filter[count++] = L"ns9";
				}

				// If more than one file in the archive let the user choose which to load,
				// non-valid files will be filtered out by named extension detection

				index = archive.UserSelect( filter, count );
			}

			if (!index)
			{
				return File::NONE;
			}
			else if (index == Io::Archive::NO_FILES)
			{
				throw IDS_FILE_ERR_NOTHING_IN_ARCHIVE;
			}

			--index;

			File::Type type = File::ARCHIVE;

			if (data)
			{
				data->Resize( archive[index].Size() );

				if (data->Size() < 4 || !archive[index].Uncompress( data->Ptr() ))
					throw IDS_FILE_ERR_INVALID;

				type = CheckFile
				(
					types,
					FourCC<>::T( data->Ptr() ),
					archive[index].GetName().Extension().Id()
				);

				if (type == File::NONE || type == File::ARCHIVE)
					throw IDS_FILE_ERR_INVALID;
			}

			if (fileInArchive.Empty())
				path << " <" << archive[index].GetName() << '>';

			return type;
		}

		Paths::File::Type Paths::CheckFile(const File::Types types,const uint fileId,const uint extensionId)
		{
			File::Type type = File::NONE;

			switch (fileId)
			{
				case File::ID_INES:    if (types( File::INES              )) type = File::INES;    break;
				case File::ID_UNIF:    if (types( File::UNIF              )) type = File::UNIF;    break;
				case File::ID_FDS:
				case File::ID_FDS_RAW: if (types( File::FDS               )) type = File::FDS;     break;
				case File::ID_NSF:     if (types( File::NSF               )) type = File::NSF;     break;
				case File::ID_IPS:     if (types( File::IPS               )) type = File::IPS;     break;
				case File::ID_UPS:     if (types( File::UPS               )) type = File::UPS;     break;
				case File::ID_NSV:     if (types( File::MOVIE             )) type = File::MOVIE;   break;
				case File::ID_NST:     if (types( File::STATE|File::SLOTS )) type = File::STATE;   break;
				case File::ID_ZIP:
				case File::ID_7Z:
				case File::ID_RAR:     if (types( File::ARCHIVE           )) type = File::ARCHIVE; break;

				default:

					switch (extensionId)
					{
						// raw or text file, must check the file extension

						case FourCC<'s','a','v'>::V: if (types( File::BATTERY )) type = File::BATTERY; break;
						case FourCC<'t','p'>::V:     if (types( File::TAPE    )) type = File::TAPE;    break;
						case FourCC<'r','o','m'>::V: if (types( File::ROM     )) type = File::ROM;     break;
						case FourCC<'x','m','l'>::V: if (types( File::XML     )) type = File::XML;     break;
						case FourCC<'p','a','l'>::V: if (types( File::PALETTE )) type = File::PALETTE; break;
						case FourCC<'w','a','v'>::V: if (types( File::WAVE    )) type = File::WAVE;    break;
						case FourCC<'a','v','i'>::V: if (types( File::AVI     )) type = File::AVI;     break;

						case FourCC<'n','e','s'>::V:
						case FourCC<'u','n','f'>::V:
						case FourCC<'u','n','i'>::V:
						case FourCC<'f','d','s'>::V:
						case FourCC<'n','s','f'>::V:
						case FourCC<'i','p','s'>::V:
						case FourCC<'u','p','s'>::V:
						case FourCC<'n','s','v'>::V:
						case FourCC<'n','s','t'>::V:
						case FourCC<'n','s','0'>::V:
						case FourCC<'n','s','1'>::V:
						case FourCC<'n','s','2'>::V:
						case FourCC<'n','s','3'>::V:
						case FourCC<'n','s','4'>::V:
						case FourCC<'n','s','5'>::V:
						case FourCC<'n','s','6'>::V:
						case FourCC<'n','s','7'>::V:
						case FourCC<'n','s','8'>::V:
						case FourCC<'n','s','9'>::V:
						case FourCC<'z','i','p'>::V:
						case FourCC<'r','a','r'>::V:
						case FourCC<'7','z'>::V:

							// either corrupt data or wrong extension, bail..

							break;

						default:

							// extension is unknown, but the file may still be valid,
							// allow it to pass if only one file type was selected

							if (types(~uint(File::ARCHIVE|File::IMAGE)) == File::ROM)
							{
								type = File::ROM;
							}
							else
							{
								switch (types(~uint(File::ARCHIVE)))
								{
									case File::XML:     type = File::XML;     break;
									case File::BATTERY: type = File::BATTERY; break;
									case File::TAPE:    type = File::TAPE;    break;
									case File::PALETTE: type = File::PALETTE; break;
								}
							}
					}
			}

			return type;
		}
	}
}
