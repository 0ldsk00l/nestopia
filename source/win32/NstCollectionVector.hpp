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

#ifndef NST_COLLECTION_VECTOR_H
#define NST_COLLECTION_VECTOR_H

#pragma once

#include <cstdlib>
#include "NstMain.hpp"

namespace Nestopia
{
	namespace Collection
	{
		template<typename T> class Vector;

		template<>
		class Vector<void>
		{
		protected:

			union
			{
				void* data;
				uchar* bytes;
			};

			uint capacity;
			uint size;

			explicit Vector(uint);
			Vector(const Vector&);
			Vector(const void* NST_RESTRICT,uint);

			void operator = (const Vector&);

			void Assign(const void* NST_RESTRICT,uint);
			void Append(const void* NST_RESTRICT,uint);
			void Insert(void*,const void* NST_RESTRICT,uint);
			void Erase(void*,void*);
			void Reserve(uint);
			void Resize(uint);
			void Grow(uint);

			bool Valid(const void*) const;
			bool InBound(const void*) const;

			Vector()
			: data(NULL), capacity(0), size(0) {}

			~Vector()
			{
				NST_ASSERT
				(
					capacity >= size &&
					bool(data) >= bool(size) &&
					bool(data) >= bool(capacity)
				);

				std::free( data );
			}

			void Shrink(uint inSize)
			{
				NST_ASSERT( size >= inSize );
				size -= inSize;
			}

		public:

			void Destroy();
			void Defrag();
			void Import(Vector&);

			bool Empty() const
			{
				return !size;
			}

			void Clear()
			{
				size = 0;
			}
		};

		template<typename T> class Vector : public Vector<void>
		{
		public:

			typedef T Type;
			typedef T* Iterator;
			typedef const T* ConstIterator;

			enum
			{
				ITEM_SIZE = sizeof(Type)
			};

			Vector() {}

			Vector(const Type* items,uint count)
			: Vector<void>(items,ITEM_SIZE * count) {}

			explicit Vector(uint count)
			: Vector<void>(count * ITEM_SIZE) {}

			Vector(const Vector<T>& vector)
			: Vector<void>(vector) {}

			Vector& operator = (const Vector<T>& vector)
			{
				Vector<void>::operator = (vector);
				return *this;
			}

			void PushBack(const Type& item)
			{
				Vector<void>::Append( &item, ITEM_SIZE );
			}

			void PushBack(const Vector<T>& vector)
			{
				Vector<void>::Append( vector.data, vector.size );
			}

			void Assign(ConstIterator items,uint count)
			{
				Vector<void>::Assign( items, ITEM_SIZE * count );
			}

			void Append(ConstIterator items,uint count)
			{
				Vector<void>::Append( items, ITEM_SIZE * count );
			}

			void Insert(Iterator pos,ConstIterator items,uint count)
			{
				Vector<void>::Insert( pos, items, ITEM_SIZE * count );
			}

			void Insert(Iterator pos,const Type& item)
			{
				Vector<void>::Insert( pos, &item, ITEM_SIZE );
			}

			void Erase(Iterator begin,Iterator end)
			{
				Vector<void>::Erase( begin, end );
			}

			void Erase(Iterator offset,uint count=1)
			{
				Vector<void>::Erase( offset, offset + count );
			}

			Type& operator [] (uint i)
			{
				return static_cast<Type*>(data)[i];
			}

			const Type& operator [] (uint i) const
			{
				return static_cast<Type*>(data)[i];
			}

			Type* Ptr()
			{
				return static_cast<Type*>(data);
			}

			const Type* Ptr() const
			{
				return static_cast<const Type*>(data);
			}

			Iterator Begin()
			{
				return static_cast<Iterator>(data);
			}

			ConstIterator Begin() const
			{
				return static_cast<ConstIterator>(data);
			}

			Iterator End()
			{
				return reinterpret_cast<Iterator>(bytes + size);
			}

			ConstIterator End() const
			{
				return reinterpret_cast<ConstIterator>(bytes + size);
			}

			Iterator At(uint pos)
			{
				return static_cast<Iterator>(data) + pos;
			}

			ConstIterator At(uint pos) const
			{
				return static_cast<ConstIterator>(data) + pos;
			}

			Type& Front()
			{
				NST_ASSERT( size );
				return *static_cast<Iterator>(data);
			}

			const Type& Front() const
			{
				NST_ASSERT( size );
				return *static_cast<ConstIterator>(data);
			}

			Type& Back()
			{
				NST_ASSERT( size );
				return *(reinterpret_cast<Iterator>(bytes + size) - 1);
			}

			const Type& Back() const
			{
				NST_ASSERT( size );
				return *(reinterpret_cast<ConstIterator>(bytes + size) - 1);
			}

			uint Size() const
			{
				NST_ASSERT( size % ITEM_SIZE == 0 );
				return size / ITEM_SIZE;
			}

			uint Length() const
			{
				return Size();
			}

			uint Capacity() const
			{
				NST_ASSERT( capacity % ITEM_SIZE == 0 );
				return capacity / ITEM_SIZE;
			}

			void Reserve(uint count)
			{
				Vector<void>::Reserve( count * ITEM_SIZE );
			}

			void Resize(uint count)
			{
				Vector<void>::Resize( count * ITEM_SIZE );
			}

			void SetTo(uint count)
			{
				size = count * ITEM_SIZE;
				NST_ASSERT( capacity >= size );
			}

			void Grow(uint count=1)
			{
				Vector<void>::Grow( count * ITEM_SIZE );
			}

			void Shrink(uint count=1)
			{
				Vector<void>::Shrink( count * ITEM_SIZE );
			}

			bool InBound(ConstIterator it) const
			{
				return Vector<void>::InBound( it );
			}

			bool Valid(ConstIterator it) const
			{
				return Vector<void>::Valid( it );
			}

			template<typename Value>
			ConstIterator Find(const Value&) const;

			template<typename Value>
			Iterator Find(const Value& value)
			{
				ConstIterator const it = static_cast<const Vector<T>*>(this)->Find( value );
				return reinterpret_cast<Type*>(bytes + (reinterpret_cast<const uchar*>(it) - bytes));
			}
		};

		template<typename T> template<typename Value>
		typename Vector<T>::ConstIterator Vector<T>::Find(const Value& value) const
		{
			for (ConstIterator it(Ptr()), end(End()); it != end; ++it)
				if (*it == value)
					return it;

			return NULL;
		}
	}
}

#endif
