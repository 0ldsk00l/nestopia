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

#ifndef NST_SYSTEM_GUID_H
#define NST_SYSTEM_GUID_H

#pragma once

#include "NstObjectPod.hpp"
#include "NstString.hpp"
#include <windows.h>

namespace Nestopia
{
	namespace System
	{
		class Guid : public Object::Pod<GUID>
		{
			typedef Object::Pod<GUID> Pod;

		public:

			Guid(const GenericString&);

			enum
			{
				STRING_LENGTH = 36
			};

			typedef String::Stack<STRING_LENGTH> Name;

			Name GetString() const;

		private:

			enum Exception
			{
				ERR_INVALID_STRING
			};

			static ulong ConvertData(GenericString);
			void FromString(GenericString);

		public:

			Guid() {}

			Guid(const GUID& guid)
			: Pod(guid) {}

			Guid& operator = (const GUID& guid)
			{
				Pod::operator = (guid);
				return *this;
			}

			Guid& operator = (const GenericString& string)
			{
				FromString( string );
				return *this;
			}
		};
	}
}

#endif
