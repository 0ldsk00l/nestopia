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

#ifndef NST_OBJECT_HEAP_H
#define NST_OBJECT_HEAP_H

#pragma once

#include "NstMain.hpp"

namespace Nestopia
{
	namespace Object
	{
		template<typename T,uint N=0> class Heap
		{
			Heap(const Heap&);

			T* const array;

		public:

			enum
			{
				SIZE = N
			};

			Heap()
			: array(new T [SIZE]) {}

			~Heap()
			{
				typedef char TypeComplete[sizeof(T)];
				delete [] array;
			}

			operator T* () const
			{
				return array;
			}
		};

		template<typename T> class Heap<T,0>
		{
			Heap(const Heap&);

			T& ref;

		public:

			Heap()
			: ref(*(new T)) {}

			explicit Heap(T* t)
			: ref(*t)
			{
				NST_ASSERT(t);
			}

			~Heap()
			{
				typedef char TypeComplete[sizeof(T)];
				delete &ref;
			}

			T& operator * () const
			{
				return ref;
			}

			T* operator -> () const
			{
				return &ref;
			}
		};

		template<>
		class Heap<void,0>
		{
			Heap(const Heap&);

			void* const ref;

		public:

			explicit Heap(void* t)
			: ref(t)
			{
				NST_ASSERT( t );
			}

			explicit Heap(uint size)
			: ref(operator new (size))
			{
				NST_ASSERT( size );
			}

			~Heap()
			{
				operator delete (ref);
			}

			operator void* () const
			{
				return ref;
			}
		};
	}
}

#endif
