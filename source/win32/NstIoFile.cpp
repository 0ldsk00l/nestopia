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
#include "NstCollectionVector.hpp"
#include <windows.h>

namespace Nestopia
{
	namespace Io
	{
		NST_COMPILE_ASSERT
		(
			File::BEGIN == FILE_BEGIN &&
			File::CURRENT == FILE_CURRENT &&
			File::END == FILE_END
		);

		File::File()
		: handle(NULL) {}

		File::File(const GenericString fileName,const uint mode)
		: handle(NULL)
		{
			Open( fileName, mode );
		}

		File::~File()
		{
			Close();
		}

		void File::Open(const GenericString n,const uint mode)
		{
			NST_ASSERT( (mode & (WRITE|READ)) && ((mode & WRITE) || !(mode & EMPTY)) && (mode & (RANDOM_ACCESS|SEQUENTIAL_ACCESS)) != (RANDOM_ACCESS|SEQUENTIAL_ACCESS) );

			Close();

			if (n.Empty())
				throw ERR_NOT_FOUND;

			handle = ::CreateFile
			(
				(name=n).Ptr(),
				(
					(mode & (READ|WRITE)) == (READ|WRITE) ? (GENERIC_READ|GENERIC_WRITE) :
					(mode & (READ|WRITE)) == (READ)       ? (GENERIC_READ) :
					(mode & (READ|WRITE)) == (WRITE)      ? (GENERIC_WRITE) :
															0
				),
				(
					(mode & (READ|WRITE)) == (WRITE) ? 0 : FILE_SHARE_READ
				),
				NULL,
				(
					(mode & (EMPTY|EXISTING)) == (EMPTY)          ? CREATE_ALWAYS :
					(mode & (EMPTY|EXISTING)) == (EMPTY|EXISTING) ? TRUNCATE_EXISTING :
					(mode & (EMPTY|EXISTING)) == (EXISTING)       ? OPEN_EXISTING :
					(mode & (WRITE))                              ? OPEN_ALWAYS :
																	OPEN_EXISTING
				),
				(
					(
						(mode & (WRITE|WRITE_THROUGH)) == (WRITE)               ? (FILE_ATTRIBUTE_NORMAL) :
						(mode & (WRITE|WRITE_THROUGH)) == (WRITE|WRITE_THROUGH) ? (FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH) : 0
					)
					|
					(
						(mode & (READ|RANDOM_ACCESS))     == (READ|RANDOM_ACCESS)     ? FILE_FLAG_RANDOM_ACCESS :
						(mode & (READ|SEQUENTIAL_ACCESS)) == (READ|SEQUENTIAL_ACCESS) ? FILE_FLAG_SEQUENTIAL_SCAN : 0
					)
				),
				NULL
			);

			if (handle && handle != INVALID_HANDLE_VALUE)
			{
				if (!(mode & EMPTY))
				{
					try
					{
						Size();
					}
					catch (...)
					{
						Close();
						throw;
					}
				}
			}
			else
			{
				handle = NULL;
				name.Clear();

				switch (::GetLastError())
				{
					case ERROR_FILE_NOT_FOUND:
					case ERROR_PATH_NOT_FOUND:

						throw ERR_NOT_FOUND;

					case ERROR_ALREADY_EXISTS:

						throw ERR_ALREADY_EXISTS;

					case ERROR_ACCESS_DENIED:

						if (mode & WRITE)
							throw ERR_READ_ONLY;

					default:

						throw ERR_OPEN;
				}
			}
		}

		void File::Close()
		{
			name.Clear();

			if (void* const tmp = handle)
			{
				handle = NULL;
				::CloseHandle( tmp );
			}
		}

		void File::Write(const void* const data,const uint size) const
		{
			NST_ASSERT( IsOpen() && bool(data) >= bool(size) );

			if (size)
			{
				DWORD written = 0;

				if (!::WriteFile( handle, data, size, &written, NULL ) || written != size)
					throw ERR_WRITE;
			}
		}

		uint File::WriteSome(const void* const data,const uint size) const
		{
			NST_ASSERT( IsOpen() && bool(data) >= bool(size) );

			if (size)
			{
				DWORD written = 0;

				if (!::WriteFile( handle, data, size, &written, NULL ))
					throw ERR_WRITE;

				return written;
			}

			return 0;
		}

		void File::Write8(uint data) const
		{
			const uchar buffer = data;
			Write( &buffer, 1 );
		}

		void File::Write16(uint data) const
		{
			const uchar buffer[] = { data & 0xFF, data >> 8 };
			Write( buffer, 2 );
		}

		void File::Write32(uint data) const
		{
			const uchar buffer[] = { data & 0xFF, data >> 8 & 0xFF, data >> 16 & 0xFF, data >> 24 };
			Write( buffer, 4 );
		}

		void File::Read(void* const data,const uint size) const
		{
			NST_ASSERT( IsOpen() && bool(data) >= bool(size) );

			if (size)
			{
				DWORD read = 0;

				if (!::ReadFile( handle, data, size, &read, NULL ) || read != size)
					throw ERR_READ;
			}
		}

		uint File::ReadSome(void* const data,uint size) const
		{
			NST_ASSERT( IsOpen() && bool(data) >= bool(size) );

			if (size)
			{
				DWORD read = 0;

				if (!::ReadFile( handle, data, size, &read, NULL ))
					throw ERR_READ;

				return read;
			}

			return 0;
		}

		void File::Peek(void* const data,const uint size) const
		{
			const uint pos = Position();
			Read( data, size );
			Position() = pos;
		}

		void File::Peek(const uint from,void* const data,const uint size) const
		{
			const uint pos = Position();
			Position() = from;
			Read( data, size );
			Position() = pos;
		}

		uint File::Peek8() const
		{
			uchar buffer;
			Peek( &buffer, 1 );
			return buffer;
		}

		uint File::Peek16() const
		{
			uchar buffer[2];
			Peek( buffer, 2 );
			return buffer[0] | uint(buffer[1]) << 8;
		}

		uint File::Peek32() const
		{
			uchar buffer[4];
			Peek( buffer, 4 );
			return buffer[0] | uint(buffer[1]) << 8 | uint(buffer[2]) << 16 | uint(buffer[3]) << 24;
		}

		uint File::Seek(const Offset offset,const int distance) const
		{
			NST_ASSERT( IsOpen() );

			const DWORD pos = ::SetFilePointer( handle, distance, NULL, offset );

			if (pos == INVALID_SET_FILE_POINTER && ::GetLastError() != NO_ERROR)
				throw ERR_SEEK;

			return pos;
		}

		uint File::Read8() const
		{
			uchar buffer;
			Read( &buffer, 1 );
			return buffer;
		}

		uint File::Read16() const
		{
			uchar buffer[2];
			Read( buffer, 2 );
			return buffer[0] | buffer[1] << 8;
		}

		uint File::Read32() const
		{
			uchar buffer[4];
			Read( buffer, 4 );
			return buffer[0] | buffer[1] << 8 | uint(buffer[2]) << 16 | uint(buffer[3]) << 24;
		}

		uint File::Size() const
		{
			NST_ASSERT( IsOpen() );

			DWORD words[2];

			words[1] = 0;
			words[0] = ::GetFileSize( handle, &words[1] );

			if (words[0] == INVALID_FILE_SIZE && ::GetLastError() != NO_ERROR)
				throw ERR_SEEK;

			if (words[1] || words[0] > MAX_SIZE)
				throw ERR_TOO_BIG;

			return words[0];
		}

		void File::Truncate() const
		{
			NST_ASSERT( IsOpen() );

			if (!::SetEndOfFile( handle ))
				throw ERR_SEEK;
		}

		void File::Truncate(const uint pos) const
		{
			Position() = pos;
			Truncate();
		}

		void File::Flush() const
		{
			NST_ASSERT( IsOpen() );
			::FlushFileBuffers( handle );
		}

		void File::ReadText(String::Heap<char>& string) const
		{
			string.Resize( Size() - Position() );

			if (string.Length())
				Read( string.Ptr(), string.Length() );
		}

		void File::ReadText(String::Heap<wchar_t>& string) const
		{
			String::Heap<char> buffer;
			ReadText( buffer );
			ParseText( buffer.Ptr(), buffer.Length(), string );
		}

		void File::WriteText(cstring const string,const uint length,bool) const
		{
			Write( string, length );
		}

		void File::WriteText(wcstring string,uint length,bool forceUnicode) const
		{
			if (length)
			{
				if (forceUnicode || String::Generic<wchar_t>(string,length).Wide())
				{
					Collection::Buffer buffer( length * 2 + 2 );
					char* NST_RESTRICT dst = buffer.Ptr();

					*dst++ = UTF16_LE & 0xFF;
					*dst++ = UTF16_LE >> 8;

					do
					{
						*dst++ = *string & 0xFF;
						*dst++ = *string++ >> 8;
					}
					while (--length);

					Write( buffer.Ptr(), buffer.Size() );
				}
				else
				{
					const String::Heap<char> tmp( string, length );
					Write( tmp.Ptr(), tmp.Length() );
				}
			}
		}

		void File::ParseText(cstring src,const uint length,String::Heap<char>& string)
		{
			string.Assign( src, length );
		}

		void File::ParseText(cstring src,const uint length,String::Heap<wchar_t>& string)
		{
			if (length)
			{
				const uint utf = (length >= 2) ? (src[0] | uint(src[1]) << 8) : 0;

				if (utf == UTF16_LE || utf == UTF16_BE)
				{
					NST_VERIFY( length > 2 && length % 2 == 0 );

					string.Resize( length / 2 - 1 );

					if (string.Length())
					{
						wchar_t* NST_RESTRICT dst = string.Ptr();
						const wchar_t* const end = dst + string.Length();

						if (utf == UTF16_LE)
						{
							do
							{
								src += 2;
								*dst++ = src[0] | src[1] << 8;
							}
							while (dst != end);
						}
						else
						{
							do
							{
								src += 2;
								*dst++ = src[1] | src[0] << 8;
							}
							while (dst != end);
						}
					}
				}
				else
				{
					string.Assign( src, length );
				}
			}
			else
			{
				string.Clear();
			}
		}

		bool File::Delete(wcstring const path)
		{
			NST_ASSERT( path && *path );
			return ::DeleteFile( path );
		}
	}
}
