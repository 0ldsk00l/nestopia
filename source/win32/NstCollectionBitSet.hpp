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

#ifndef NST_COLLECTION_BITSET_H
#define NST_COLLECTION_BITSET_H

#pragma once

#include "NstMain.hpp"

namespace Nestopia
{
	namespace Collection
	{
		class BitSet
		{
		public:

			typedef uint Bits;

		private:

			Bits bits;

		public:

			class Bit
			{
				Bits& bits;
				const Bits mask;

			public:

				Bit(Bits& b,Bits m)
				: bits(b), mask(m) {}

				operator bool () const
				{
					return bits & mask;
				}

				void operator = (bool set)
				{
					bits = set ? (bits | mask) : (bits & ~mask);
				}
			};

			BitSet(Bits b=0)
			: bits(b) {}

			Bit operator [] (uint index)
			{
				return Bit( bits, 1U << index );
			}

			bool operator [] (uint index) const
			{
				return bits & (1U << index);
			}

			uint operator () (uint mask) const
			{
				return bits & mask;
			}

			Bits& Word()
			{
				return bits;
			}

			const Bits& Word() const
			{
				return bits;
			}
		};
	}
}

#endif
