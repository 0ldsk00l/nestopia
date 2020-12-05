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

#include "NstObjectPod.hpp"
#include "NstApplicationInstance.hpp"
#include "NstDialogBrowse.hpp"
#include <CommDlg.h>
#include <ShlObj.h>

namespace Nestopia
{
	namespace Window
	{
		const Path Browser::OpenFile(wchar_t* const filter,const Path dir,const Path ext)
		{
			for (uint i=0; filter[i]; ++i)
			{
				if (filter[i] == '\t')
					filter[i] = '\0';
			}

			Path path;
			path.Reserve( MAX_PATH*2 );

			Object::Pod<OPENFILENAME> ofn;

			ofn.lStructSize     = sizeof(ofn);
			ofn.hwndOwner       = Application::Instance::GetActiveWindow();
			ofn.lpstrFile       = path.Ptr();
			ofn.nMaxFile        = path.Capacity();
			ofn.lpstrInitialDir = dir.Length() ? dir.Ptr() : L".";
			ofn.Flags           = OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;

			if (filter)
			{
				ofn.lpstrFilter = filter;
				ofn.nFilterIndex = 1;
			}

			if (ext.Length())
				ofn.lpstrDefExt = ext.Ptr();

			if (::GetOpenFileName( &ofn ))
				path.Validate();
			else
				path.Clear();

			return path;
		}

		const Path Browser::SaveFile(wchar_t* const filter,Path initial)
		{
			Path path;
			path.Reserve( MAX_PATH*2 );

			const Path extension( initial.Extension() );

			if (initial.File().Length() && initial.File()[0] != '.')
				path = initial.File();

			initial.File().Clear();

			Object::Pod<OPENFILENAME> ofn;

			ofn.lStructSize     = sizeof(ofn);
			ofn.hwndOwner       = Application::Instance::GetActiveWindow();
			ofn.lpstrFile       = path.Ptr();
			ofn.nMaxFile        = path.Capacity();
			ofn.lpstrInitialDir = initial.Length() ? initial.Ptr() : L".";
			ofn.Flags           = OFN_EXPLORER|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;

			if (filter)
			{
				for (uint i=0; filter[i]; ++i)
				{
					if (filter[i] == '\t')
						filter[i] = '\0';
				}

				ofn.lpstrFilter = filter;
				ofn.nFilterIndex = 1;
			}

			if (extension.Length())
				ofn.lpstrDefExt = extension.Ptr();

			if (::GetSaveFileName( &ofn ))
				path.Validate();
			else
				path.Clear();

			return path;
		}

		const Path Browser::SelectDirectory()
		{
			Path path;
			path.Reserve( MAX_PATH*2 );

			Object::Pod<BROWSEINFO> bi;

			bi.hwndOwner      = Application::Instance::GetActiveWindow();
			bi.pszDisplayName = path.Ptr();
			bi.ulFlags        = BIF_RETURNONLYFSDIRS;

			if (LPITEMIDLIST const idl = ::SHBrowseForFolder( &bi ))
			{
				if (::SHGetPathFromIDList( idl, path.Ptr() ) && path.Validate())
					path.MakePretty( true );
				else
					path.Clear();

				IMalloc* pMalloc;

				if (SUCCEEDED(::SHGetMalloc( &pMalloc )))
				{
					pMalloc->Free( idl );
					pMalloc->Release();
				}
			}
			else
			{
				path.Clear();
			}

			return path;
		}
	}
}
