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

#include <cstring>
#include "NstCollectionVector.hpp"

#ifdef NST_MSVC_OPTIMIZE
#pragma optimize("t", on)
#endif

namespace Nestopia
{
	namespace Collection
	{
		Vector<void>::Vector(const uint inSize)
		:
		data     (inSize ? std::malloc( inSize ) : NULL),
		capacity (inSize),
		size     (inSize)
		{}

		Vector<void>::Vector(const void* const NST_RESTRICT inData,const uint inSize)
		:
		data     (inSize ? std::memcpy( std::malloc( inSize ), inData, inSize ) : NULL),
		capacity (inSize),
		size     (inSize)
		{
			NST_ASSERT( bool(inData) >= bool(inSize) );
		}

		Vector<void>::Vector(const Vector<void>& base)
		:
		data     (base.size ? std::memcpy( std::malloc( base.size ), base.data, base.size ) : NULL),
		capacity (base.size),
		size     (base.size)
		{
		}

		bool Vector<void>::Valid(const void* const it) const
		{
			return it >= bytes && it <= bytes + size;
		}

		bool Vector<void>::InBound(const void* const it) const
		{
			return it >= bytes && it < bytes + size;
		}

		void Vector<void>::Assign(const void* const NST_RESTRICT inData,const uint inSize)
		{
			size = inSize;

			if (capacity < inSize)
				data = std::realloc( data, capacity = inSize );

			std::memcpy( data, inData, inSize );
		}

		void Vector<void>::Append(const void* const NST_RESTRICT inData,const uint inSize)
		{
			size += inSize;

			if (capacity < size)
				data = std::realloc( data, capacity = size * 2 );

			std::memcpy( bytes + (size - inSize), inData, inSize );
		}

		void Vector<void>::Insert(void* const offset,const void* const NST_RESTRICT inData,const uint inSize)
		{
			NST_ASSERT( Valid(offset) );

			if (inSize)
			{
				const uint pos = static_cast<uchar*>(offset) - bytes;
				const uint end = size - pos;

				size += inSize;

				if (capacity >= size)
				{
					std::memmove( static_cast<uchar*>(offset) + inSize, offset, end );

					if (inData)
						std::memcpy( offset, inData, inSize );
				}
				else
				{
					capacity = size * 2;

					void* const NST_RESTRICT next = std::malloc( capacity );

					std::memcpy( next, data, pos );

					if (inData)
						std::memcpy( static_cast<uchar*>(next) + pos, inData, inSize );

					std::memcpy( static_cast<uchar*>(next) + pos + inSize, offset, end );

					void* tmp = data;
					data = next;
					std::free( tmp );
				}
			}
		}

		void Vector<void>::Erase(void* const begin,void* const end)
		{
			NST_ASSERT( end >= begin && begin >= bytes && end <= bytes + size );

			const uint back = (bytes + size) - static_cast<uchar*>(end);
			size -= static_cast<uchar*>(end) - static_cast<uchar*>(begin);

			std::memmove( begin, end, back );
		}

		void Vector<void>::Destroy()
		{
			if (void* tmp = data)
			{
				data = NULL;
				capacity = 0;
				size = 0;
				std::free( tmp );
			}
		}

		void Vector<void>::operator = (const Vector<void>& base)
		{
			Assign( base.data, base.size );
		}

		void Vector<void>::Reserve(const uint inSize)
		{
			if (capacity < inSize)
				data = std::realloc( data, capacity = inSize );
		}

		void Vector<void>::Resize(const uint inSize)
		{
			size = inSize;
			Reserve( inSize );
		}

		void Vector<void>::Grow(const uint inSize)
		{
			Resize( size + inSize );
		}

		void Vector<void>::Defrag()
		{
			if (capacity != size)
				data = std::realloc( data, capacity = size );
		}

		void Vector<void>::Import(Vector<void>& base)
		{
			void* tmp = data;
			data = base.data;
			base.data = NULL;
			size = base.size;
			base.size = 0;
			capacity = base.capacity;
			base.capacity = 0;
			std::free( tmp );
		}
	}
}

#ifdef NST_MSVC_OPTIMIZE
#pragma optimize("", on)
#endif
