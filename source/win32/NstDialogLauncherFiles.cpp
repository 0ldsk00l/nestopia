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

#include <map>
#include <set>
#include "NstWindowUser.hpp"
#include "NstIoFile.hpp"
#include "NstIoArchive.hpp"
#include "NstIoStream.hpp"
#include "NstSystemThread.hpp"
#include "NstIoLog.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogLauncher.hpp"
#include "../core/NstCrc32.hpp"
#include "../core/NstChecksum.hpp"
#include "../core/NstXml.hpp"


namespace Nestopia
{
	namespace Window
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		Launcher::List::Files::Entry::Entry(uint t)
		:
		file       (0),
		path       (0),
		name       (0),
		pRom       (0),
		cRom       (0),
		wRam       (0),
		vRam       (0),
		mapper     (0),
		type       (t),
		attributes (0)
		{}

		class Launcher::List::Files::Inserter
		{
		public:

			typedef Paths::Settings Settings;
			typedef Settings::Include Include;

			Inserter
			(
				Strings&,
				Entries&,
				const Include,
				const Nes::Cartridge::Database&
			);

			bool Add(const GenericString);

		protected:

			void ReadFile(wcstring const,const Dialog&);

			enum Stop
			{
				STOP_SEARCH
			};

			enum
			{
				PATH_NOT_ADDED
			};

			const Include include;
			uint compressed;

			struct
			{
				Path string;
				bool incSubDir;
				uint reference;
			}   path;

			Strings strings;
			Entries entries;
			Strings& saveStrings;
			Entries& saveEntries;

		private:

			enum Type
			{
				TYPE_INVALID,
				TYPE_PROCESSED
			};

			typedef std::set<uint> FileChecksums;
			typedef Collection::Buffer Buffer;
			typedef Type (Inserter::*Parser)();

			Parser GetParser() const;

			inline uint Crc(const uint,const uint) const;

			bool UniqueFile();
			bool PrepareFile(const uint=1,const uint=0);
			Type ParseAny();
			Type ParseXml();
			Type ParseNes();
			Type ParseUnf();
			Type ParseFds();
			Type ParseNsf();
			Type ParsePatch();
			Type ParseArchive();
			void AddEntry(uint,const Nes::Cartridge::Profile&);
			void AddEntry(const Entry&);

			Buffer buffer;
			FileChecksums parsedFiles;
			const Nes::Cartridge::Database& imageDatabase;
		};

		Launcher::List::Files::Inserter::Inserter
		(
			Strings& v,
			Entries& e,
			const Include i,
			const Nes::Cartridge::Database& r
		)
		:
		include       ( i ),
		saveStrings   ( v ),
		saveEntries   ( e ),
		imageDatabase ( r )
		{}

		Launcher::List::Files::Inserter::Parser Launcher::List::Files::Inserter::GetParser() const
		{
			if (const uint extension = path.string.Extension().Id())
			{
				if (include[Include::ANY])
				{
					if (extension == FourCC<'x','m','l'>::V)
					{
						return include[Include::XML] ? &Inserter::ParseXml : NULL;
					}
					else
					{
						return &Inserter::ParseAny;
					}
				}
				else
				{
					switch (extension)
					{
						case FourCC<'n','e','s'>::V:     return include[Include::NES] ? &Inserter::ParseNes : NULL;
						case FourCC<'u','n','f'>::V:
						case FourCC<'u','n','i','f'>::V: return include[Include::UNF] ? &Inserter::ParseUnf : NULL;
						case FourCC<'x','m','l'>::V:     return include[Include::XML] ? &Inserter::ParseXml : NULL;
						case FourCC<'f','d','s'>::V:     return include[Include::FDS] ? &Inserter::ParseFds : NULL;
						case FourCC<'n','s','f'>::V:     return include[Include::NSF] ? &Inserter::ParseNsf : NULL;
						case FourCC<'i','p','s'>::V:
						case FourCC<'u','p','s'>::V:     return include[Include::PATCH] ? &Inserter::ParsePatch : NULL;
						case FourCC<'z','i','p'>::V:
						case FourCC<'r','a','r'>::V:
						case FourCC<'7','z'>::V:         return include[Include::ARCHIVE] ? &Inserter::ParseArchive : NULL;
					}
				}
			}
			else if (include[Include::ANY])
			{
				return &Inserter::ParseAny;
			}

			return NULL;
		}

		void Launcher::List::Files::Inserter::AddEntry(const Entry& entry)
		{
			entries.PushBack( entry );

			Entry& back = entries.Back();

			back.file = (strings << path.string.File());

			if (path.reference == PATH_NOT_ADDED)
				path.reference = (strings << path.string.Directory());

			NST_VERIFY( path.string.Directory() == strings[path.reference] );

			back.path = path.reference;

			if (entries.Size() == MAX_ENTRIES)
				throw STOP_SEARCH;
		}

		bool Launcher::List::Files::Inserter::Add(const GenericString fileName)
		{
			NST_ASSERT( fileName.Length() && fileName.Length() <= MAX_PATH );

			if (entries.Size() == MAX_ENTRIES)
				return false;

			compressed = 0;
			path.string = fileName;
			strings = saveStrings;

			{
				const int index = strings.Find( path.string.Directory() );

				if (index != Strings::NONE)
					path.reference = index;
				else
					path.reference = PATH_NOT_ADDED;
			}

			if (Parser const parser = GetParser())
			{
				try
				{
					(*this.*parser)();
				}
				catch (Io::File::Exception)
				{
					return false;
				}
				catch (Stop)
				{
					// reached file count limit
				}

				if (entries.Size())
				{
					saveEntries.PushBack( entries );
					saveStrings = strings;
					return true;
				}
			}

			return false;
		}

		void Launcher::List::Files::Inserter::ReadFile(wcstring const fileName,const Dialog& dialog)
		{
			path.string.File() = fileName;

			if (Parser const parser = GetParser())
			{
				if (dialog)
					dialog.Control( IDC_LAUNCHER_FILESEARCH_FILE ).Text() << path.string.Ptr();

				compressed = 0;
				buffer.Clear();

				try
				{
					(*this.*parser)();
				}
				catch (Io::File::Exception)
				{
					// file I/O failure, just skip it
				}
			}

			path.string.File().Clear();
		}

		inline uint Launcher::List::Files::Inserter::Crc(const uint start,const uint length) const
		{
			NST_ASSERT( buffer.Size() );
			return Nes::Core::Crc32::Compute( reinterpret_cast<const uchar*>(&buffer[start]), length );
		}

		bool Launcher::List::Files::Inserter::UniqueFile()
		{
			NST_ASSERT( buffer.Size() );
			return !include[Include::UNIQUE] || parsedFiles.insert(Crc(0,buffer.Size())).second;
		}

		bool Launcher::List::Files::Inserter::PrepareFile(const uint minSize,const uint fileId)
		{
			NST_ASSERT( path.string.Length() && minSize && minSize >= bool(fileId) * 4U );

			if (buffer.Empty())
			{
				const Io::File file( path.string, Io::File::COLLECT );
				const uint size = file.Size();

				if (size >= minSize && (!fileId || fileId == file.Peek32()))
				{
					buffer.Resize( size );
					file.Read( buffer.Ptr(), size );
					return true;
				}

				return false;
			}

			return
			(
				buffer.Size() >= minSize &&
				(!fileId || fileId == FourCC<>::T( buffer.Ptr() ))
			);
		}

		void Launcher::List::Files::Inserter::AddEntry(const uint flags,const Nes::Cartridge::Profile& profile)
		{
			Entry entry( flags );

			if (!profile.game.title.empty())
				entry.name = strings << profile.game.title.c_str();

			entry.pRom = profile.board.GetPrg() / Nes::Core::SIZE_1K;
			entry.cRom = profile.board.GetChr() / Nes::Core::SIZE_1K;
			entry.wRam = profile.board.GetWram() / Nes::Core::SIZE_1K;
			entry.vRam = profile.board.GetVram() / Nes::Core::SIZE_1K;

			if (profile.board.mapper != Nes::Cartridge::Profile::Board::NO_MAPPER)
				entry.mapper = profile.board.mapper;

			if (profile.board.HasBattery())
				entry.attributes |= Entry::ATR_BATTERY;

			if (profile.multiRegion)
			{
				entry.attributes |= Entry::ATR_NTSC_PAL;
			}
			else switch (profile.system.type)
			{
				case Nes::Cartridge::Profile::System::NES_PAL:
				case Nes::Cartridge::Profile::System::NES_PAL_A:
				case Nes::Cartridge::Profile::System::NES_PAL_B:
				case Nes::Cartridge::Profile::System::DENDY:

					entry.attributes |= Entry::ATR_PAL;
					break;

				default:

					entry.attributes |= Entry::ATR_NTSC;
					break;
			}

			entry.hash = profile.hash;

			AddEntry( entry );
		}

		Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseNes()
		{
			if (PrepareFile( 16, Managers::Paths::File::ID_INES ))
			{
				if (UniqueFile())
				{
					Io::Stream::In stream( buffer );
					Nes::Cartridge::Profile profile;
					Nes::Cartridge::ReadInes( stream, Nes::Machine::FAVORED_NES_NTSC, profile );

					if (!imageDatabase.FindEntry( profile.hash, Nes::Machine::FAVORED_NES_NTSC ))
					{
						uint length = buffer.Size();

						if (length >= 16+Nes::Core::SIZE_8K)
						{
							length -= 16;

							if (length > Nes::Cartridge::NesHeader::MAX_PRG_ROM + Nes::Cartridge::NesHeader::MAX_CHR_ROM)
								length = Nes::Cartridge::NesHeader::MAX_PRG_ROM + Nes::Cartridge::NesHeader::MAX_CHR_ROM;
							else
								length -= length % Nes::Core::SIZE_8K;

							if (length != profile.board.GetPrg() + profile.board.GetChr())
							{
								Nes::Cartridge::Profile::Hash hash;
								hash.Compute( buffer.At(16), length );

								if (imageDatabase.FindEntry( hash, Nes::Machine::FAVORED_NES_NTSC ))
									profile.hash = hash;
							}
						}
					}

					AddEntry( Entry::NES | compressed, profile );
				}

				return TYPE_PROCESSED;
			}

			return TYPE_INVALID;
		}

		Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseUnf()
		{
			if (PrepareFile( 32, Managers::Paths::File::ID_UNIF ))
			{
				if (UniqueFile())
				{
					Io::Stream::In stream( buffer );
					Nes::Cartridge::Profile profile;
					Nes::Cartridge::ReadUnif( stream, Nes::Machine::FAVORED_NES_NTSC, profile );

					AddEntry( Entry::UNF | compressed, profile );
				}

				return TYPE_PROCESSED;
			}

			return TYPE_INVALID;
		}

		Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseXml()
		{
			if (PrepareFile())
			{
				if (UniqueFile())
				{
					Io::Stream::In stream( buffer );
					Nes::Cartridge::Profile profile;

					if (NES_SUCCEEDED(Nes::Cartridge::ReadRomset( stream, Nes::Machine::FAVORED_NES_NTSC, Nes::Machine::DONT_ASK_PROFILE, profile )))
						AddEntry( Entry::XML | compressed, profile );
				}

				return TYPE_PROCESSED;
			}

			return TYPE_INVALID;
		}

		Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseFds()
		{
			const bool hasHeader = PrepareFile( 16, Managers::Paths::File::ID_FDS );

			if (hasHeader || PrepareFile( 4, Managers::Paths::File::ID_FDS_RAW ))
			{
				if (UniqueFile())
				{
					Entry entry( Entry::FDS | compressed );

					const uint size = buffer.Size() - (hasHeader ? 16 : 0);
					entry.pRom = (size / Nes::Core::SIZE_1K) + (size % Nes::Core::SIZE_1K != 0);
					entry.wRam = 32;
					entry.vRam = 8;
					entry.attributes |= Entry::ATR_DEFAULT_FDS;

					AddEntry( entry );
				}

				return TYPE_PROCESSED;
			}

			return TYPE_INVALID;
		}

		Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseNsf()
		{
			if (PrepareFile( 128, Managers::Paths::File::ID_NSF ))
			{
				if (UniqueFile())
				{
					enum
					{
						NSF_CHIP_FDS  = 0x4,
						NSF_CHIP_MMC5 = 0x8
					};

					#pragma pack(push,1)

					struct Header
					{
						uchar pad1[14];
						char  name[32];
						uchar pad2[32];
						char  maker[32];
						uchar pad3[12];
						uchar mode;
						uchar chip;
						uchar pad4[4];
					};

					#pragma pack(pop)

					NST_COMPILE_ASSERT( sizeof(Header) == 128 );

					Header& header = reinterpret_cast<Header&>( buffer.Front() );

					Entry entry( Entry::NSF | compressed );

					const uint size = buffer.Size() - sizeof(Header);
					entry.pRom = (size / Nes::Core::SIZE_1K) + (size % Nes::Core::SIZE_1K != 0);

					switch (header.mode & 0x3U)
					{
						case 0x0: entry.attributes |= Entry::ATR_NTSC;     break;
						case 0x1: entry.attributes |= Entry::ATR_PAL;      break;
						default:  entry.attributes |= Entry::ATR_NTSC_PAL; break;
					}

					switch (header.chip)
					{
						case NSF_CHIP_MMC5: entry.wRam = 8+1;  break;
						case NSF_CHIP_FDS:  entry.wRam = 8+32; break;
						default:            entry.wRam = 8;    break;
					}

					header.pad2[0] = '\0'; // in case string is not terminated
					entry.name = strings.Import( header.name );

					AddEntry( entry );
				}

				return TYPE_PROCESSED;
			}

			return TYPE_INVALID;
		}

		Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParsePatch()
		{
			if (PrepareFile( 5, Managers::Paths::File::ID_IPS ) || PrepareFile( 4, Managers::Paths::File::ID_UPS ))
			{
				if (UniqueFile())
				{
					Entry entry( Entry::PATCH | compressed );
					AddEntry( entry );

					return TYPE_PROCESSED;
				}
			}

			return TYPE_INVALID;
		}

		Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseArchive()
		{
			compressed = Entry::ARCHIVE;

			Io::File file;

			try
			{
				file.Open( path.string, Io::File::READ|Io::File::EXISTING );
			}
			catch (Io::File::Exception)
			{
				return TYPE_PROCESSED;
			}

			const Io::Archive archive( file );
			const uint length = path.string.Length();

			for (uint i=0; i < archive.NumFiles(); ++i)
			{
				NST_VERIFY( archive[i].GetName().Length() );

				// will look like: "archive.zip <image.nes>"
				path.string << " <" << archive[i].GetName();

				Parser const parser = GetParser();

				if (parser && parser != &Inserter::ParseArchive)
				{
					path.string << '>';
					buffer.Resize( archive[i].Size() );

					if (archive[i].Uncompress( buffer.Ptr() ))
						(*this.*parser)();
				}

				path.string.ShrinkTo( length );
			}

			return TYPE_PROCESSED;
		}

		Launcher::List::Files::Inserter::Type Launcher::List::Files::Inserter::ParseAny()
		{
			const bool notCompressed = buffer.Empty();

			if
			(
				(!include[Include::NES] || ParseNes() == TYPE_INVALID) &&
				(!include[Include::UNF] || ParseUnf() == TYPE_INVALID) &&
				(!include[Include::FDS] || ParseFds() == TYPE_INVALID) &&
				(!include[Include::NSF] || ParseNsf() == TYPE_INVALID) &&
				(!include[Include::PATCH] || ParsePatch() == TYPE_INVALID) &&
				( include[Include::ARCHIVE] && notCompressed)
			)
				ParseArchive();

			return TYPE_PROCESSED;
		}

		class Launcher::List::Files::Searcher : Inserter
		{
		public:

			Searcher
			(
				Strings&,
				Entries&,
				const Settings&,
				const Nes::Cartridge::Database&
			);

			void Search();

		private:

			enum Abort
			{
				ABORT_SEARCH
			};

			typedef std::set<HeapString> SearchedPaths;

			ibool OnInitDialog (Param&);

			bool UniquePath();
			void Start(System::Thread::Terminator);
			void Search(System::Thread::Terminator);
			void ReadPath(wcstring const,System::Thread::Terminator);

			const Settings::Folders& folders;
			SearchedPaths searchedPaths;
			Dialog dialog;
			System::Thread thread;
		};

		Launcher::List::Files::Searcher::Searcher
		(
			Strings& v,
			Entries& e,
			const Settings& s,
			const Nes::Cartridge::Database& r
		)
		:
		Inserter (v,e,s.include,r),
		folders  (s.folders),
		dialog   (IDD_LAUNCHER_SEARCH,WM_INITDIALOG,this,&Searcher::OnInitDialog)
		{}

		void Launcher::List::Files::Searcher::Start(System::Thread::Terminator terminator)
		{
			try
			{
				for (Settings::Folders::const_iterator it(folders.begin()), end(folders.end()); it != end; ++it)
				{
					if (it->path.Length())
					{
						path.string = it->path;

						if (UniquePath())
						{
							path.incSubDir = it->incSubDir;
							path.reference = PATH_NOT_ADDED;
							Search( terminator );
						}
					}
				}
			}
			catch (Stop)
			{
			}
			catch (...)
			{
				static_cast<Generic&>(dialog).Close();
				return;
			}

			static_cast<Generic&>(dialog).Close();

			saveStrings = strings;
			saveEntries = entries;
		}

		ibool Launcher::List::Files::Searcher::OnInitDialog(Param&)
		{
			thread.Start( System::Thread::Callback(this,&Searcher::Start) );
			return true;
		}

		void Launcher::List::Files::Searcher::Search()
		{
			if (folders.size() && (include.Word() & Include::FILES))
			{
				bool enabled = !Application::Instance::GetMainWindow().Enable( false );
				dialog.Open();
				Application::Instance::GetMainWindow().Enable( enabled );
			}
		}

		void Launcher::List::Files::Searcher::Search(System::Thread::Terminator terminate)
		{
			NST_ASSERT( path.string.Length() );

			struct FileFinder
			{
				WIN32_FIND_DATA data;
				HANDLE const handle;

				FileFinder(wcstring path)
				: handle(::FindFirstFile( path, &data )) {}

				~FileFinder()
				{
					if (handle != INVALID_HANDLE_VALUE)
						::FindClose( handle );
				}
			};

			path.string.File() = "*.*";

			FileFinder findFile( path.string.Ptr() );

			path.string.File().Clear();

			if (findFile.handle != INVALID_HANDLE_VALUE)
			{
				do
				{
					if (terminate)
						throw ABORT_SEARCH;

					if (!(findFile.data.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)))
					{
						if (findFile.data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
							ReadPath( findFile.data.cFileName, terminate );
						else
							ReadFile( findFile.data.cFileName, dialog );
					}
				}
				while (::FindNextFile( findFile.handle, &findFile.data ));
			}
		}

		bool Launcher::List::Files::Searcher::UniquePath()
		{
			NST_ASSERT( path.string.Length() );

			SearchedPaths::iterator it( searchedPaths.lower_bound( path.string ) );

			if (it == searchedPaths.end() || *it != path.string)
			{
				searchedPaths.insert( it, path.string );
				return true;
			}

			return false;
		}

		void Launcher::List::Files::Searcher::ReadPath(wcstring const subDir,System::Thread::Terminator terminator)
		{
			NST_ASSERT( subDir );

			if (path.incSubDir && *subDir && *subDir != '.')
			{
				path.string.Directory() += subDir;

				if (UniquePath())
				{
					const uint reference = path.reference;
					path.reference = PATH_NOT_ADDED;
					Search( terminator );
					path.reference = reference;
				}

				path.string.Directory() -= 1;
			}
		}

		Launcher::List::Files::Files()
		:
		dirty  (false),
		loaded (!Application::Instance::GetExePath(L"launcher.xml").FileExists())
		{
			if (loaded)
				Io::Log() << "Launcher: database file \"launcher.xml\" not present\r\n";
		}

		void Launcher::List::Files::Load()
		{
			if (loaded)
				return;

			loaded = true;

			try
			{
				typedef Nes::Core::Xml Xml;
				Xml xml;

				{
					Io::Stream::In stream( Application::Instance::GetExePath(L"launcher.xml") );
					xml.Read( stream );
				}

				if (!xml.GetRoot().IsType( L"launcher" ) || !xml.GetRoot().GetAttribute( L"version" ).IsValue(L"1.1"))
					throw 1;

				for (Xml::Node node(xml.GetRoot().GetFirstChild()); node; node=node.GetNextSibling())
				{
					Entry entry;

					if (node.IsType( L"romset" ))
					{
						entry.type = Entry::XML;
					}
					else if (node.IsType( L"ines" ))
					{
						entry.type = Entry::NES;
					}
					else if (node.IsType( L"unif" ))
					{
						entry.type = Entry::UNF;
					}
					else if (node.IsType( L"fds" ))
					{
						entry.type = Entry::FDS;
					}
					else if (node.IsType( L"nsf" ))
					{
						entry.type = Entry::NSF;
					}
					else if (node.IsType( L"patch" ))
					{
						entry.type = Entry::PATCH;
					}
					else
					{
						throw 1;
					}

					wcstring string;

					if (!*(string=node.GetChild( L"file" ).GetValue()))
						throw 1;

					entry.file = strings << string;

					if (!*(string=node.GetChild( L"dir" ).GetValue()))
						throw 1;

					entry.path = strings << string;

					if (node.GetChild( L"archive" ).IsValue( L"yes" ))
						entry.type |= Entry::ARCHIVE;

					switch (entry.type & Entry::ALL)
					{
						case Entry::XML:
						case Entry::UNF:
						case Entry::NES:
						{
							if (*(string=node.GetChild( L"name" ).GetValue()))
								entry.name = strings << string;

							const Xml::Node system( node.GetChild( L"system" ) );

							if (system.IsValue( L"vs" ))
							{
								entry.attributes = Entry::ATR_VS;
							}
							else if (system.IsValue( L"pc10" ))
							{
								entry.attributes = Entry::ATR_PC10;
							}
							else if (system.IsValue( L"pal" ))
							{
								entry.attributes = Entry::ATR_PAL;
							}
							else if (system.IsValue( L"ntsc/pal" ))
							{
								entry.attributes = Entry::ATR_NTSC_PAL;
							}
							else
							{
								entry.attributes = Entry::ATR_NTSC;
							}

							entry.pRom = node.GetChild( L"prg" ).GetUnsignedValue() & 0xFFFF;
							entry.cRom = node.GetChild( L"chr" ).GetUnsignedValue() & 0xFFFF;
							entry.wRam = node.GetChild( L"wram" ).GetUnsignedValue() & 0xFFFF;
							entry.vRam = node.GetChild( L"vram" ).GetUnsignedValue() & 0xFFFF;
							entry.mapper = node.GetChild( L"mapper" ).GetUnsignedValue() & 0xFFFF;

							if (node.GetChild( L"battery" ).IsValue( L"yes" ))
								entry.attributes |= Entry::ATR_BATTERY;

							entry.hash.Assign( node.GetChild( L"sha1" ).GetValue(), node.GetChild( L"crc" ).GetValue() );
							break;
						}

						case Entry::FDS:

							entry.pRom = node.GetChild( L"prg" ).GetUnsignedValue() & 0xFFFF;
							entry.wRam = 32;
							entry.attributes = Entry::ATR_DEFAULT_FDS;
							break;

						case Entry::NSF:
						{
							if (*string)
								entry.name = strings << string;

							entry.pRom = node.GetChild( L"prg" ).GetUnsignedValue() & 0xFFFF;
							entry.wRam = node.GetChild( L"wram" ).GetUnsignedValue() & 0xFFFF;

							const Xml::Node system( node.GetChild( L"system" ) );

							if (system.IsValue( L"pal" ))
							{
								entry.attributes = Entry::ATR_PAL;
							}
							else if (system.IsValue( L"ntsc/pal" ))
							{
								entry.attributes = Entry::ATR_NTSC_PAL;
							}
							else
							{
								entry.attributes = Entry::ATR_NTSC;
							}
							break;
						}
					}

					entries.PushBack( entry );
				}
			}
			catch (...)
			{
				dirty = true;

				Clear();
				User::Warn( IDS_LAUNCHER_ERR_LOAD_DB );
			}

			Defrag();
		}

		void Launcher::List::Files::Save()
		{
			if (dirty)
			{
				const Path fileName( Application::Instance::GetExePath(L"launcher.xml") );

				if (entries.Size())
				{
					try
					{
						typedef Nes::Core::Xml Xml;

						Xml xml;
						Xml::Node root( xml.Create(L"launcher") );
						root.AddAttribute( L"version", L"1.1" );

						for (Entries::ConstIterator it(entries.Begin()), end(entries.End()); it != end; ++it)
						{
							wcstring type;

							switch (it->type & Entry::ALL)
							{
								case Entry::NES:   type = L"ines";    break;
								case Entry::UNF:   type = L"unif";    break;
								case Entry::XML:   type = L"romset";  break;
								case Entry::FDS:   type = L"fds";     break;
								case Entry::NSF:   type = L"nsf";     break;
								case Entry::PATCH: type = L"patch";   break;
								default: continue;
							}

							Xml::Node node( root.AddChild( type ) );

							node.AddChild( L"file", strings[it->file] );
							node.AddChild( L"dir", strings[it->path] );

							if (it->type & Entry::ARCHIVE)
								node.AddChild( L"archive", L"yes" );

							wchar_t buffer[32];

							switch (it->type & Entry::ALL)
							{
								case Entry::NSF:
								{
									NST_ASSERT( !(it->attributes & ~uint(Entry::ATR_NTSC_PAL)) );

									if (it->name)
										node.AddChild( L"name", strings[it->name] );

									wcstring system = L"ntsc";

									if (it->attributes & Entry::ATR_PAL)
									{
										if (it->attributes & Entry::ATR_NTSC)
											system = L"ntsc/pal";
										else
											system = L"pal";
									}

									node.AddChild( L"system", system );

									if (it->pRom)
									{
										std::swprintf( buffer, L"%u", uint(it->pRom) );
										node.AddChild( L"prg", buffer );
									}

									if (it->wRam)
									{
										std::swprintf( buffer, L"%u", uint(it->wRam) );
										node.AddChild( L"wram", buffer );
									}

									break;
								}

								case Entry::FDS:

									if (it->pRom)
									{
										std::swprintf( buffer, L"%u", uint(it->pRom) );
										node.AddChild( L"prg", buffer );
									}
									break;

								case Entry::XML:
								case Entry::UNF:
								case Entry::NES:
								{
									if (it->name)
										node.AddChild( L"name", strings[it->name] );

									wcstring system = L"ntsc";

									if (it->attributes & Entry::ATR_VS)
									{
										system = L"vs";
									}
									else if (it->attributes & Entry::ATR_PC10)
									{
										system = L"pc10";
									}
									else if (it->attributes & Entry::ATR_PAL)
									{
										if (it->attributes & Entry::ATR_NTSC)
											system = L"ntsc/pal";
										else
											system = L"pal";
									}

									node.AddChild( L"system", system );

									if (it->pRom)
									{
										std::swprintf( buffer, L"%u", uint(it->pRom) );
										node.AddChild( L"prg", buffer );
									}

									if (it->cRom)
									{
										std::swprintf( buffer, L"%u", uint(it->cRom) );
										node.AddChild( L"chr", buffer );
									}

									if (it->wRam)
									{
										std::swprintf( buffer, L"%u", uint(it->wRam) );
										node.AddChild( L"wram", buffer );
									}

									if (it->vRam)
									{
										std::swprintf( buffer, L"%u", uint(it->vRam) );
										node.AddChild( L"vram", buffer );
									}

									if (it->attributes & Entry::ATR_BATTERY)
										node.AddChild( L"battery", L"yes" );

									if (it->mapper)
									{
										std::swprintf( buffer, L"%u", uint(it->mapper) );
										node.AddChild( L"mapper", buffer );
									}

									if (it->hash)
									{
										std::swprintf( buffer, L"%08X", uint(it->hash.GetCrc32()) );
										node.AddChild( L"crc", buffer );

										wchar_t sha1[Entry::Hash::SHA1_WORD_LENGTH*8+1];
										sha1[Entry::Hash::SHA1_WORD_LENGTH*8] = '\0';

										for (uint i=0; i < Entry::Hash::SHA1_WORD_LENGTH; ++i)
											std::swprintf( sha1+i*8, L"%08X", uint(it->hash.GetSha1()[i]) );

										node.AddChild( L"sha1", sha1 );
									}

									break;
								}
							}
						}

						Collection::Buffer buffer;

						{
							Io::Stream::Out stream( buffer );
							xml.Write( root, stream );
						}

						xml.Destroy();

						Io::File( fileName, Io::File::DUMP ).Write( buffer.Ptr(), buffer.Size() );

						Io::Log() << "Launcher: database saved to \"launcher.xml\"\r\n";
					}
					catch (...)
					{
						User::Warn( IDS_LAUNCHER_ERR_SAVE_DB );
					}
				}
				else if (fileName.FileExists())
				{
					if (Io::File::Delete( fileName.Ptr() ))
						Io::Log() << "Launcher: empty database, deleted \"launcher.xml\"\r\n";
					else
						Io::Log() << "Launcher: warning, couldn't delete \"launcher.xml\"!\r\n";
				}
			}
		}

		void Launcher::List::Files::Defrag()
		{
			typedef std::map<GenericString,uint> References;

			References references;

			if (entries.Size())
			{
				Entries tmp;
				tmp.Reserve( entries.Size() );

				for (Entries::ConstIterator it(entries.Begin()), end(entries.End()); it != end; ++it)
				{
					if (it->type)
					{
						if ( it->file ) references[strings[it->file]] = 0;
						if ( it->path ) references[strings[it->path]] = 0;
						if ( it->name ) references[strings[it->name]] = 0;

						tmp.PushBack( *it );
					}
				}

				if (entries.Size() != tmp.Size())
					entries = tmp;
			}

			if (entries.Size())
			{
				Strings tmp( strings.Size() );

				for (References::iterator it(references.begin()), end(references.end()); it != end; ++it)
					it->second = (tmp << it->first);

				for (Entries::Iterator it(entries.Begin()), end(entries.End()); it != end; ++it)
				{
					if ( it->file ) it->file = references.find( strings[ it->file  ] )->second;
					if ( it->path ) it->path = references.find( strings[ it->path  ] )->second;
					if ( it->name ) it->name = references.find( strings[ it->name  ] )->second;
				}

				strings = tmp;
			}
			else
			{
				Clear();
			}
		}

		bool Launcher::List::Files::Insert(const Nes::Cartridge::Database& imageDatabase,const GenericString fileName)
		{
			return dirty |=
			(
				fileName.Length() &&
				Inserter( strings, entries, Paths::Settings::Include(true), imageDatabase ).Add( fileName )
			);
		}

		bool Launcher::List::Files::ShouldDefrag() const
		{
			uint garbage = 0;

			for (Entries::ConstIterator it(entries.Begin()), end(entries.End()); it != end; ++it)
			{
				garbage += (it->type == 0);

				if (garbage > GARBAGE_THRESHOLD)
					return true;
			}

			return false;
		}

		void Launcher::List::Files::Clear()
		{
			if (entries.Size())
				dirty = true;

			entries.Destroy();
			strings.Clear();
		}

		void Launcher::List::Files::Refresh
		(
			const Paths::Settings& settings,
			const Nes::Cartridge::Database& imageDatabase
		)
		{
			dirty = true;
			Searcher( strings, entries, settings, imageDatabase ).Search();
		}

		Nes::Cartridge::Database::Entry Launcher::List::Files::Entry::SearchDb(const Nes::Cartridge::Database* db) const
		{
			Nes::Cartridge::Database::Entry entry;

			if (db && (type & (NES|UNF|XML)))
				entry = db->FindEntry( hash, Nes::Machine::FAVORED_NES_NTSC );

			return entry;
		}

		uint Launcher::List::Files::Entry::GetSystem(const Nes::Cartridge::Database* db) const
		{
			if (const Nes::Cartridge::Database::Entry entry = SearchDb( db ))
			{
				if (entry.IsMultiRegion())
				{
					return SYSTEM_NTSC_PAL;
				}
				else switch (entry.GetSystem())
				{
					case Nes::Cartridge::Profile::System::NES_NTSC:
					case Nes::Cartridge::Profile::System::FAMICOM:
						return SYSTEM_NTSC;

					case Nes::Cartridge::Profile::System::NES_PAL:
					case Nes::Cartridge::Profile::System::NES_PAL_A:
					case Nes::Cartridge::Profile::System::NES_PAL_B:
					case Nes::Cartridge::Profile::System::DENDY:
						return SYSTEM_PAL;

					case Nes::Cartridge::Profile::System::VS_UNISYSTEM:
					case Nes::Cartridge::Profile::System::VS_DUALSYSTEM:
						return SYSTEM_VS;

					case Nes::Cartridge::Profile::System::PLAYCHOICE_10:
						return SYSTEM_PC10;
				}
			}
			else
			{
				if (attributes & ATR_VS)
				{
					return SYSTEM_VS;
				}
				else if (attributes & ATR_PC10)
				{
					return SYSTEM_PC10;
				}
				else if ((attributes & ATR_NTSC_PAL) == ATR_NTSC_PAL)
				{
					return SYSTEM_NTSC_PAL;
				}
				else if (attributes & ATR_NTSC)
				{
					return SYSTEM_NTSC;
				}
				else if (attributes & ATR_PAL)
				{
					return SYSTEM_PAL;
				}
			}

			return SYSTEM_UNKNOWN;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}

