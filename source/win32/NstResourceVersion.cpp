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

#include "NstObjectHeap.hpp"
#include "NstResourceVersion.hpp"

#if NST_MSVC
#pragma comment(lib,"version")
#endif

namespace Nestopia
{
	namespace Resource
	{
		Version::Version(wcstring const path,const VersionType versiontype)
		{
			NST_ASSERT( path );

			char buffer[] = "xx.xx";

			if (uint size = ::GetFileVersionInfoSize( path, 0 ))
			{
				Object::Heap<void> data( size );

				if (::GetFileVersionInfo( path, 0, size, data ))
				{
					wchar_t type[] = L"\\";
					void* ptr;

					if (::VerQueryValue( data, type, &ptr, &size ) && size == sizeof(info))
					{
						info = *static_cast<const VS_FIXEDFILEINFO*>(ptr);

						char* string = buffer;

						if (HIWORD(info.dwFileVersionMS))
							*string++ = '0' + HIWORD(versiontype == PRODUCT ? info.dwProductVersionMS : info.dwFileVersionMS);

						string[0] = '0' + LOWORD(versiontype == PRODUCT ? info.dwProductVersionMS : info.dwFileVersionMS);
						string[1] = '.';
						string[2] = '0' + HIWORD(versiontype == PRODUCT ? info.dwProductVersionLS : info.dwFileVersionLS);
						string[3] = '0' + LOWORD(versiontype == PRODUCT ? info.dwProductVersionLS : info.dwFileVersionLS);
						string[4] = '\0';
					}
				}
			}

			String::Heap<char>::operator = (buffer);
		}
	}
}
