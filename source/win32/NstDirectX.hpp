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

#ifndef NST_DIRECTX_H
#define NST_DIRECTX_H

#pragma once

#include "NstSystemGuid.hpp"

namespace Nestopia
{
	namespace DirectX
	{
		struct BaseAdapter
		{
			bool operator == (const BaseAdapter& a)  const { return guid == a.guid; }
			bool operator <  (const BaseAdapter& a)  const { return guid < a.guid;  }
			bool operator == (const System::Guid& g) const { return guid == g;      }

			System::Guid guid;
			HeapString name;
		};

		template<typename T>
		class ComInterface : public ImplicitBool< ComInterface<T> >
		{
			T* com;

		public:

			ComInterface()
			: com(NULL) {}

			~ComInterface()
			{
				if (com)
					com->Release();
			}

			ulong Release()
			{
				if (T* const p = com)
				{
					com = NULL;
					return p->Release();
				}

				return 0;
			}

			bool operator ! () const
			{
				return com == NULL;
			}

			T* operator * () const
			{
				return com;
			}

			T* operator -> () const
			{
				NST_ASSERT( com );
				return com;
			}

			T** operator & ()
			{
				NST_VERIFY( !com );
				return &com;
			}
		};
	}
}

#endif
