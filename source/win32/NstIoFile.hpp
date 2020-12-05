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

#ifndef NST_IO_FILE_H
#define NST_IO_FILE_H

#pragma once

#include "language/resource.h"
#include "NstString.hpp"

namespace Nestopia
{
	namespace Io
	{
		class File
		{
		public:

			enum
			{
				READ = 0x01,
				WRITE = 0x02,
				EMPTY = 0x04,
				EXISTING = 0x08,
				SEQUENTIAL_ACCESS = 0x10,
				RANDOM_ACCESS = 0x20,
				WRITE_THROUGH = 0x40,
				COLLECT = READ|EXISTING|SEQUENTIAL_ACCESS,
				DUMP = WRITE|EMPTY|SEQUENTIAL_ACCESS
			};

			enum
			{
				UTF16_LE = 0xFEFF,
				UTF16_BE = 0xFFFE
			};

			enum
			{
				MAX_SIZE = 0x7FFFFFFF
			};

			enum Offset
			{
				BEGIN,
				CURRENT,
				END
			};

			enum Exception
			{
				ERR_READ = IDS_FILE_ERR_READ,
				ERR_WRITE = IDS_FILE_ERR_WRITE,
				ERR_SEEK = IDS_FILE_ERR_READ,
				ERR_OPEN = IDS_FILE_ERR_OPEN,
				ERR_NOT_FOUND = IDS_FILE_ERR_NOTFOUND,
				ERR_ALREADY_EXISTS = IDS_FILE_ERR_ALREADYEXISTS,
				ERR_READ_ONLY = IDS_FILE_ERR_READONLY,
				ERR_TOO_BIG = IDS_FILE_ERR_TOOBIG
			};

			File();
			File(GenericString,uint);
			~File();

			void Open(GenericString,uint);
			void Close();
			uint Seek(Offset,int=0) const;
			void Truncate() const;
			void Truncate(uint) const;
			void Flush() const;
			uint Size() const;

			void Write     (const void*,uint) const;
			uint WriteSome (const void*,uint) const;
			void Write8    (uint) const;
			void Write16   (uint) const;
			void Write32   (uint) const;
			void Read      (void*,uint) const;
			uint ReadSome  (void*,uint) const;
			uint Read8     () const;
			uint Read16    () const;
			uint Read32    () const;
			void Peek      (void*,uint) const;
			void Peek      (uint,void*,uint) const;
			uint Peek8     () const;
			uint Peek16    () const;
			uint Peek32    () const;

			void ReadText (String::Heap<char>&) const;
			void ReadText (String::Heap<wchar_t>&) const;
			void WriteText (cstring,uint,bool=false) const;
			void WriteText (wcstring,uint,bool=false) const;

			static void ParseText (cstring,uint,String::Heap<char>&);
			static void ParseText (cstring,uint,String::Heap<wchar_t>&);

		private:

			class Proxy
			{
				const File& file;

			public:

				Proxy(const File& file)
				: file(file) {}

				void operator = (int pos) const
				{
					file.Seek( BEGIN, pos );
				}

				void operator += (int count) const
				{
					file.Seek( CURRENT, count );
				}

				void operator -= (int count) const
				{
					file.Seek( CURRENT, -count );
				}

				operator uint () const
				{
					return file.Seek( CURRENT );
				}
			};

			class StreamProxy
			{
				const File& file;

			public:

				StreamProxy(const File& f)
				: file(f) {}

				template<typename T>
				const StreamProxy& operator << (const T& buffer) const
				{
					NST_COMPILE_ASSERT( sizeof(buffer[0]) == sizeof(char) );
					file.Write( buffer.Ptr(), buffer.Length() );
					return *this;
				}

				template<typename T> void operator >> (T& buffer) const
				{
					NST_COMPILE_ASSERT( sizeof(buffer[0]) == sizeof(char) );
					buffer.Resize( file.Size() - file.Position() );
					file.Read( buffer.Ptr(), buffer.Length() );
				}
			};

			void* handle;
			Path name;

		public:

			bool IsOpen() const
			{
				return handle;
			}

			const Path& GetName() const
			{
				return name;
			}

			StreamProxy Stream() const
			{
				return *this;
			}

			Proxy Position() const
			{
				return *this;
			}

			void Rewind() const
			{
				Seek( BEGIN );
			}

			static bool Delete(wcstring);
		};
	}
}

#endif
