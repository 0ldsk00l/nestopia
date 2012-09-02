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

#ifndef NST_IO_STREAM_H
#define NST_IO_STREAM_H

#pragma once

#include <iosfwd>
#include "NstCollectionVector.hpp"
#include "NstString.hpp"

namespace Nestopia
{
	namespace Io
	{
		class File;

		namespace Stream
		{
			class Out;

			class In
			{
			public:

				explicit In(const File&);
				explicit In(GenericString);
				explicit In(const Collection::Buffer&);
				In(const void*,uint);
				~In();

			private:

				friend class Out;

				class FileStream;
				class BufferStream;

				std::istream& stream;

			public:

				operator std::istream& ()
				{
					return stream;
				}
			};

			class InOut;

			class Out
			{
			public:

				explicit Out(const File&);
				explicit Out(GenericString);
				explicit Out(Collection::Buffer&);
				~Out();

			private:

				friend class InOut;

				class FileStream;
				class BufferStream;

				std::ostream& stream;

			public:

				operator std::ostream& ()
				{
					return stream;
				}
			};

			class InOut
			{
			public:

				enum
				{
					READ = 0x1,
					WRITE = 0x2,
					TRUNCATE = 0x4
				};

				explicit InOut(const File&);
				explicit InOut(GenericString,uint=READ|WRITE);
				~InOut();

				operator std::istream& ();
				operator std::ostream& ();

			private:

				class FileStream;

				std::iostream& stream;

			public:

				operator std::iostream& ()
				{
					return stream;
				}
			};
		}
	}
}

#endif
