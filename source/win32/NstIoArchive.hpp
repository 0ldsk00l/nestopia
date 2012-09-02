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

#ifndef NST_IO_ARCHIVE_H
#define NST_IO_ARCHIVE_H

#pragma once

#include <vector>
#include "NstString.hpp"

namespace Nestopia
{
	namespace Io
	{
		class File;

		class Archive
		{
		public:

			Archive();
			explicit Archive(const File&);
			Archive(const void*,uint);
			~Archive();

			bool Open(const File&);
			bool Open(const void*,uint);
			void Close();
			uint Find(GenericString) const;

			enum
			{
				FIRST_FILE    = 1,
				NO_FILES      = INT_MAX,
				MAX_ITEM_SIZE = INT_MAX-1,
				FILE_ID_ZIP   = FourCC<'P','K',0x03,0x04>::V,
				FILE_ID_7Z    = FourCC<'7','z',0xBC,0xAF>::V,
				FILE_ID_RAR   = FourCC<'R','a','r','!'>::V
			};

			uint UserSelect() const;
			uint UserSelect(const GenericString*,uint) const;

		private:

			bool Open(const File*,const void*,uint);

			class Codec;
			class UnZip;
			class UnRar;
			class Un7zip;

			class Item
			{
				Codec* codec;
				Path name;
				uint size;
				uint index;

			public:

				inline Item(Codec*,wcstring,uint,uint);

				uint Uncompress(void*) const;

				uint Size() const
				{
					return size;
				}

				const Path& GetName() const
				{
					return name;
				}
			};

			typedef std::vector<Item> Items;

			Codec* codec;
			Items files;

		public:

			uint NumFiles() const
			{
				return files.size();
			}

			const Item& operator [] (uint i) const
			{
				return files[i];
			}
		};
	}
}

#endif
