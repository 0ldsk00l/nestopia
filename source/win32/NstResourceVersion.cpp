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

#include <stdio.h>
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

			char buffer[] = "xxxx.xxxx.xxxx.xxxx\0";

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

						//fetches the version numbers
						WORD v1 = HIWORD(versiontype == PRODUCT ? info.dwProductVersionMS : info.dwFileVersionMS);
						WORD v2 = LOWORD(versiontype == PRODUCT ? info.dwProductVersionMS : info.dwFileVersionMS);
						WORD v3 = HIWORD(versiontype == PRODUCT ? info.dwProductVersionLS : info.dwFileVersionLS);
						WORD v4 = LOWORD(versiontype == PRODUCT ? info.dwProductVersionLS : info.dwFileVersionLS);

						if (v1)
						{
							if (v4)
							{
								sprintf(buffer, "%d.%d.%d.%d", v1, v2, v3, v4); //maps 1,2,0,4 to "1.2.0.4"
							}
							else
							{
								sprintf(buffer, "%d.%d.%d", v1, v2, v3);	//maps 1,2,0,0 to "1.2.0"
							}
						}
						else
						{
							if (v4)
							{
								sprintf(buffer, "%d.%d.%d", v2, v3, v4);	//maps 0,1,2,3 to "1.2.3"
							}
							else
							{
								sprintf(buffer, "%d.%d", v2, v3);	//maps 0,1,2,0 to "1.2"
							}
						}
					}
				}
			}

			String::Heap<char>::operator = (buffer);
		}
	}
}
