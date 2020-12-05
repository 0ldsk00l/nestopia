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

#include <iostream>
#include "NstIoFile.hpp"
#include "NstIoStream.hpp"

namespace Nestopia
{
	namespace Io
	{
		namespace Stream
		{
			class In::FileStream : public std::istream
			{
			public:

				class Buffer : public std::streambuf
				{
				public:

					explicit Buffer(const File&,bool=false);
					explicit Buffer(GenericString);
					~Buffer();

				protected:

					const File& file;

				private:

					const bool internal;

					int_type underflow();
					int_type uflow();
					std::streamsize xsgetn(char*,std::streamsize);
					std::streampos seekoff(std::streamoff,std::ios::seekdir,std::ios::openmode);
					std::streampos seekpos(std::streampos,std::ios::openmode);

					#if NST_MSVC >= 1400 // ugh
					std::streamsize _Xsgetn_s(char*,std::size_t,std::streamsize);
					#endif
				};

			private:

				Buffer buffer;

			public:

				explicit FileStream(const File& f)
				: std::istream(&buffer), buffer(f) {}

				explicit FileStream(GenericString filename)
				: std::istream(&buffer), buffer(filename) {}
			};

			class In::BufferStream : public std::istream
			{
				class Buffer : public std::streambuf
				{
				public:

					explicit Buffer(const void*,uint);

				private:

					uint pos;
					const char* const data;
					const uint size;

					int_type underflow();
					int_type uflow();
					std::streamsize xsgetn(char*,std::streamsize);
					std::streampos seekoff(std::streamoff,std::ios::seekdir,std::ios::openmode);
					std::streampos seekpos(std::streampos,std::ios::openmode);

					#if NST_MSVC >= 1400 // ugh
					std::streamsize _Xsgetn_s(char*,std::size_t,std::streamsize);
					#endif
				};

				Buffer buffer;

			public:

				explicit BufferStream(const void* data,uint size)
				: std::istream(&buffer), buffer(data,size) {}
			};

			In::In(const File& file)
			: stream(*new FileStream(file))
			{
			}

			In::In(GenericString filename)
			: stream(*new FileStream(filename))
			{
			}

			In::In(const Collection::Buffer& buffer)
			: stream(*new BufferStream(buffer.Ptr(),buffer.Size()))
			{
			}

			In::In(const void* data,uint size)
			: stream(*new BufferStream(data,size))
			{
			}

			In::~In()
			{
				delete &stream;
			}

			In::FileStream::Buffer::Buffer(const File& f,bool i)
			: file(f), internal(i)
			{
				setg( NULL, NULL, NULL );
			}

			In::FileStream::Buffer::Buffer(GenericString filename)
			: file(*new Io::File(filename,Io::File::READ|Io::File::EXISTING)), internal(true)
			{
				setg( NULL, NULL, NULL );
			}

			In::FileStream::Buffer::~Buffer()
			{
				if (internal)
					delete &file;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("t", on)
			#endif

			In::FileStream::Buffer::int_type In::FileStream::Buffer::underflow()
			{
				try
				{
					return file.Peek8();
				}
				catch (...)
				{
					return traits_type::eof();
				}
			}

			In::FileStream::Buffer::int_type In::FileStream::Buffer::uflow()
			{
				try
				{
					return file.Read8();
				}
				catch (...)
				{
					return traits_type::eof();
				}
			}

			std::streamsize In::FileStream::Buffer::xsgetn(char* output,std::streamsize count)
			{
				NST_ASSERT( count >= 0 );

				try
				{
					return file.ReadSome( output, count );
				}
				catch (...)
				{
					return 0;
				}
			}

			#if NST_MSVC >= 1400
			std::streamsize In::FileStream::Buffer::_Xsgetn_s(char* output,std::size_t,std::streamsize count)
			{
				return Buffer::xsgetn( output, count );
			}
			#endif

			std::streampos In::FileStream::Buffer::seekoff(std::streamoff off,std::ios::seekdir dir,std::ios::openmode)
			{
				NST_ASSERT( (dir == std::ios::beg || dir == std::ios::cur || dir == std::ios::end) );

				try
				{
					return file.Seek
					(
						dir == std::ios::beg ? File::BEGIN :
						dir == std::ios::end ? File::END :
                                               File::CURRENT,
						off
					);
				}
				catch (...)
				{
					return std::streamoff(-1);
				}
			}

			std::streampos In::FileStream::Buffer::seekpos(std::streampos p,std::ios::openmode mode)
			{
				return seekoff( p, std::ios::beg, mode );
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			In::BufferStream::Buffer::Buffer(const void* d,uint s)
			: pos(0), data(static_cast<const char*>(d)), size(s)
			{
				setg( NULL, NULL, NULL );
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("t", on)
			#endif

			In::BufferStream::Buffer::int_type In::BufferStream::Buffer::underflow()
			{
				NST_VERIFY( pos <= size );
				return pos < size ? data[pos] : traits_type::eof();
			}

			In::BufferStream::Buffer::int_type In::BufferStream::Buffer::uflow()
			{
				NST_VERIFY( pos <= size );
				return pos < size ? data[pos++] : traits_type::eof();
			}

			std::streamsize In::BufferStream::Buffer::xsgetn(char* output,std::streamsize count)
			{
				NST_ASSERT( count >= 0 );
				NST_VERIFY( count + pos <= size + 1 );

				if (count + pos > size)
					count = pos < size ? size - pos : 0;

				std::memcpy( output, data + pos, count );
				pos += count;

				return count;
			}

			#if NST_MSVC >= 1400
			std::streamsize In::BufferStream::Buffer::_Xsgetn_s(char* output,std::size_t,std::streamsize count)
			{
				return Buffer::xsgetn( output, count );
			}
			#endif

			std::streampos In::BufferStream::Buffer::seekoff(std::streamoff off,std::ios::seekdir dir,std::ios::openmode mode)
			{
				NST_ASSERT( (mode == std::ios::in) && (dir == std::ios::beg || dir == std::ios::cur || dir == std::ios::end) );

				if (dir == std::ios::cur)
				{
					off += int(pos);
				}
				else if (dir == std::ios::end)
				{
					off += int(size);
				}

				NST_VERIFY( off >= 0 && off <= size );

				if (off >= 0 && off <= size)
				{
					pos = off;
					return off;
				}
				else
				{
					return std::streamoff(-1);
				}
			}

			std::streampos In::BufferStream::Buffer::seekpos(std::streampos p,std::ios::openmode mode)
			{
				return seekoff( p, std::ios::beg, mode );
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			class Out::FileStream : public std::ostream
			{
			public:

				class Buffer : public In::FileStream::Buffer
				{
				public:

					explicit Buffer(const File&,bool=false);
					explicit Buffer(GenericString);

				private:

					int_type overflow(int_type);
					std::streamsize xsputn(const char*,std::streamsize);
				};

			private:

				Buffer buffer;

			public:

				explicit FileStream(const File& f)
				: std::ostream(&buffer), buffer(f) {}

				explicit FileStream(GenericString filename)
				: std::ostream(&buffer), buffer(filename) {}
			};

			class Out::BufferStream : public std::ostream
			{
				class Buffer : public std::streambuf
				{
				public:

					explicit Buffer(Collection::Buffer&);

				private:

					uint pos;
					Collection::Buffer& data;

					int_type overflow(int_type);
					std::streamsize xsputn(const char*,std::streamsize);
					std::streampos seekoff(std::streamoff,std::ios::seekdir,std::ios::openmode);
					std::streampos seekpos(std::streampos,std::ios::openmode);
				};

				Buffer buffer;

			public:

				explicit BufferStream(Collection::Buffer& b)
				: std::ostream(&buffer), buffer(b) {}
			};

			Out::Out(const File& file)
			: stream(*new FileStream(file))
			{
			}

			Out::Out(GenericString filename)
			: stream(*new FileStream(filename))
			{
			}

			Out::Out(Collection::Buffer& buffer)
			: stream(*new BufferStream(buffer))
			{
			}

			Out::~Out()
			{
				delete &stream;
			}

			Out::FileStream::Buffer::Buffer(const File& file,bool i)
			: In::FileStream::Buffer(file,i)
			{
				setp( NULL, NULL );
			}

			Out::FileStream::Buffer::Buffer(GenericString filename)
			: In::FileStream::Buffer(*new Io::File(filename,Io::File::WRITE|Io::File::EMPTY),true)
			{
				setp( NULL, NULL );
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("t", on)
			#endif

			Out::FileStream::Buffer::int_type Out::FileStream::Buffer::overflow(int_type c)
			{
				if (!traits_type::eq_int_type(traits_type::eof(),c))
				{
					try
					{
						file.Write8( c );
					}
					catch (...)
					{
						c = traits_type::eof();
					}
				}

				return c;
			}

			std::streamsize Out::FileStream::Buffer::xsputn(const char* input,std::streamsize count)
			{
				NST_ASSERT( count >= 0 );

				try
				{
					return file.WriteSome( input, count );
				}
				catch (...)
				{
					return 0;
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			Out::BufferStream::Buffer::Buffer(Collection::Buffer& buffer)
			: pos(0), data(buffer)
			{
				setp( NULL, NULL );
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("t", on)
			#endif

			Out::BufferStream::Buffer::int_type Out::BufferStream::Buffer::overflow(int_type c)
			{
				if (!traits_type::eq_int_type(traits_type::eof(),c))
				{
					if (data.Size() == pos)
					{
						++pos;
						data.PushBack( c );
					}
					else if (data.Size() > pos)
					{
						data[pos++] = c;
					}
					else
					{
						c = traits_type::eof();
					}
				}

				return c;
			}

			std::streamsize Out::BufferStream::Buffer::xsputn(const char* input,std::streamsize count)
			{
				NST_ASSERT( count >= 0 );

				const uint cur = pos;
				pos += count;

				if (pos > data.Capacity())
					data.Reserve( pos * 2 );

				if (pos > data.Size())
					data.SetTo( pos );

				std::memcpy( data.Ptr() + cur, input, count );

				return count;
			}

			std::streampos Out::BufferStream::Buffer::seekoff(std::streamoff off,std::ios::seekdir dir,std::ios::openmode mode)
			{
				NST_ASSERT( (mode == std::ios::out) && (dir == std::ios::beg || dir == std::ios::cur || dir == std::ios::end) );

				if (dir == std::ios::cur)
				{
					off += int(pos);
				}
				else if (dir == std::ios::end)
				{
					off += int(data.Size());
				}

				NST_VERIFY( off >= 0 && off <= data.Size() );

				if (off >= 0 && off <= data.Size())
				{
					pos = off;
					return off;
				}
				else
				{
					return std::streamoff(-1);
				}
			}

			std::streampos Out::BufferStream::Buffer::seekpos(std::streampos p,std::ios::openmode mode)
			{
				return seekoff( p, std::ios::beg, mode );
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			class InOut::FileStream : public std::iostream
			{
				class Buffer : public Out::FileStream::Buffer
				{
				public:

					explicit Buffer(const File& file)
					: Out::FileStream::Buffer(file) {}

					explicit Buffer(GenericString filename,uint flags)
					: Out::FileStream::Buffer
					(
						*new Io::File
						(
							filename,
							((flags &     READ) ? Io::File::READ  : 0) |
							((flags &    WRITE) ? Io::File::WRITE : 0) |
							((flags & TRUNCATE) ? Io::File::EMPTY : 0)
						),
						true
					)
					{}
				};

				Buffer buffer;

			public:

				explicit FileStream(const File& file)
				: std::iostream(&buffer), buffer(file) {}

				explicit FileStream(GenericString filename,uint flags)
				: std::iostream(&buffer), buffer(filename,flags) {}
			};

			InOut::InOut(const File& file)
			: stream(*new FileStream(file))
			{
			}

			InOut::InOut(GenericString filename,uint flags)
			: stream(*new FileStream(filename,flags))
			{
			}

			InOut::~InOut()
			{
				delete &stream;
			}

			InOut::operator std::istream& ()
			{
				return stream;
			}

			InOut::operator std::ostream& ()
			{
				return stream;
			}
		}
	}
}
