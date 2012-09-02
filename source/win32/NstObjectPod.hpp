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

#ifndef NST_OBJECT_POD_H
#define NST_OBJECT_POD_H

#pragma once

#include <cstring>
#include "NstMain.hpp"

namespace Nestopia
{
	namespace Object
	{
		template<typename T> class Pod : public T
		{
			void Copy(const T& type)
			{
				std::memcpy( static_cast<T*>(this), &type, sizeof(T) );
			}

			int Compare(const T& type) const
			{
				return std::memcmp( static_cast<const T*>(this), &type, sizeof(T) );
			}

		public:

			void Clear()
			{
				std::memset( static_cast<T*>(this), 0, sizeof(T) );
			}

			Pod()
			{
				NST_COMPILE_ASSERT( sizeof(*this) == sizeof(T) );
				Clear();
			}

			Pod(const T& type)
			{
				Copy( type );
			}

			Pod& operator = (const T& type)
			{
				Copy( type );
				return *this;
			}

			bool operator == (const T& type) const
			{
				return Compare( type ) == 0;
			}

			bool operator != (const T& type) const
			{
				return Compare( type ) != 0;
			}

			bool operator < (const T& type) const
			{
				return Compare( type ) < 0;
			}

			bool operator > (const T& type) const
			{
				return Compare( type ) > 0;
			}
		};
	}
}

#endif
