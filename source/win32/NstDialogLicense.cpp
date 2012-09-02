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

#include "resource/resource.h"
#include "NstWindowParam.hpp"
#include "NstResourceFile.hpp"
#include "NstDialogLicense.hpp"

namespace Nestopia
{
	namespace Window
	{
		struct License::Handlers
		{
			static const MsgHandler::Entry<License> messages[];
		};

		const MsgHandler::Entry<License> License::Handlers::messages[] =
		{
			{ WM_INITDIALOG,     &License::OnInitDialog     },
			{ WM_CTLCOLORSTATIC, &License::OnCtlColorStatic }
		};

		License::License()
		: dialog(IDD_LICENSE,this,Handlers::messages) {}

		ibool License::OnInitDialog(Param&)
		{
			Collection::Buffer buffer;

			if (Resource::File( IDR_LICENSE, L"License" ).Uncompress( buffer ))
			{
				buffer.PushBack('\0');
				dialog.Control( IDC_LICENSE_EDIT ).Text() << buffer.Ptr();
			}

			return true;
		}

		ibool License::OnCtlColorStatic(Param& param)
		{
			NST_COMPILE_ASSERT( sizeof(ibool) == sizeof(BOOL) );

			if (reinterpret_cast<HWND>(param.lParam) == ::GetDlgItem( param.hWnd, IDC_LICENSE_EDIT ))
				return reinterpret_cast<ibool>(::GetSysColorBrush( COLOR_WINDOW ));
			else
				return false;
		}
	}
}
