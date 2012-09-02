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

#ifndef NST_MANAGER_PATHS_H
#define NST_MANAGER_PATHS_H

#pragma once

#include "language/resource.h"
#include "NstObjectHeap.hpp"
#include "NstCollectionBitSet.hpp"
#include "NstManager.hpp"

namespace Nestopia
{
	namespace Io
	{
		class File;
		class Archive;
	}

	namespace Window
	{
		class Paths;
	}

	namespace Managers
	{
		class Paths : Manager
		{
		public:

			Paths(Emulator&,const Configuration&,Window::Menu&);
			~Paths();

			struct File
			{
				File();
				~File();

				typedef Collection::Buffer Data;
				typedef HeapString Text;
				typedef Collection::BitSet Types;

				enum Type
				{
					NONE    = 0x00000,
					INES    = 0x00001,
					UNIF    = 0x00002,
					FDS     = 0x00004,
					NSF     = 0x00008,
					BATTERY = 0x00010,
					TAPE    = 0x00020,
					STATE   = 0x00040,
					SLOTS   = 0x00080,
					IPS     = 0x00100,
					UPS     = 0x00200,
					MOVIE   = 0x00400,
					ROM     = 0x00800,
					XML     = 0x01000,
					PALETTE = 0x02000,
					WAVE    = 0x04000,
					AVI     = 0x08000,
					ARCHIVE = 0x10000
				};

				enum
				{
					CARTRIDGE = INES|UNIF|XML,
					GAME = CARTRIDGE|FDS,
					IMAGE = GAME|NSF,
					PATCH = IPS|UPS
				};

				enum
				{
					ID_INES    = FourCC<'N','E','S',0x1A>::V,
					ID_UNIF    = FourCC<'U','N','I','F'>::V,
					ID_FDS     = FourCC<'F','D','S',0x1A>::V,
					ID_NSF     = FourCC<'N','E','S','M'>::V,
					ID_IPS     = FourCC<'P','A','T','C'>::V,
					ID_UPS     = FourCC<'U','P','S','1'>::V,
					ID_NST     = FourCC<'N','S','T',0x1A>::V,
					ID_NSV     = FourCC<'N','S','V',0x1A>::V,
					ID_ZIP     = FourCC<'P','K',0x03,0x04>::V,
					ID_RAR     = FourCC<'R','a','r','!'>::V,
					ID_7Z      = FourCC<'7','z',0xBC,0xAF>::V,
					ID_FDS_RAW = FourCC<0x01,0x2A,0x4E,0x49>::V
				};

				Type type;
				Path name;
				Data data;
			};

			enum Method
			{
				DONT_SUGGEST,
				SUGGEST
			};

			enum Checking
			{
				CHECK_FILE,
				DONT_CHECK_FILE
			};

			enum Alert
			{
				QUIETLY,
				NOISY,
				STICKY
			};

			void Save(Configuration&) const;

			void FixFile(File::Type,Path&) const;
			bool FindFile(Path&) const;
			bool LocateFile(Path&,File::Types) const;

			Path GetCheatPath() const;
			Path GetCheatPath(const Path&) const;
			Path GetPatchPath(const Path&,File::Type) const;
			Path GetSavePath(const Path&,File::Type) const;
			Path GetScreenShotPath() const;
			Path GetSamplesPath() const;

			bool SaveSlotExportingEnabled() const;
			bool SaveSlotImportingEnabled() const;
			bool AutoLoadCheatsEnabled() const;
			bool AutoSaveCheatsEnabled() const;
			bool UseStateCompression() const;
			bool BypassPatchValidation() const;

			bool CheckFile
			(
				Path&,
				File::Types,
				Alert=QUIETLY,
				uint=IDS_TITLE_ERROR
			)   const;

			Path BrowseLoad
			(
				File::Types,
				GenericString = GenericString(),
				Checking=CHECK_FILE
			)   const;

			Path BrowseSave
			(
				File::Types,
				Method=DONT_SUGGEST,
				GenericString = GenericString()
			)   const;

			File::Type Load
			(
				File&,
				File::Types,
				GenericString = GenericString(),
				Alert=NOISY
			)   const;

			bool Save
			(
				const void*,
				uint,
				File::Type,
				Path,
				Alert=NOISY
			)   const;

			const Path GetDefaultDirectory(File::Types) const;

		private:

			class Filter;

			void OnMenu(uint);
			void OnEmuEvent(Emulator::Event,Emulator::Data);

			static wcstring GetDefaultExtension(File::Types);

			void UpdateSettings();
			void UpdateRecentImageDirectory(const Path&,File::Types) const;

			static File::Type CheckFile
			(
				File::Types,
				uint,
				uint
			);

			static File::Type LoadFromFile
			(
				Path&,
				File::Data*,
				File::Types
			);

			static File::Type LoadFromArchive
			(
				const Io::Archive&,
				Path&,
				File::Data*,
				const GenericString&,
				File::Types
			);

			mutable Path recentImageDir;
			Object::Heap<Window::Paths> dialog;
		};
	}
}

#endif
