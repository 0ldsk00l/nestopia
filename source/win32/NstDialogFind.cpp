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

#include "NstWindowDialog.hpp"
#include "NstDialogFind.hpp"

namespace Nestopia
{
	namespace Window
	{
		Finder::Finder(Custom& p)
		: parent(p)
		{
			struct Hook
			{
				static UINT_PTR CALLBACK FRProc(HWND hWnd,UINT uMsg,WPARAM,LPARAM lParam)
				{
					switch (uMsg)
					{
						case WM_INITDIALOG:
						{
							Generic& window = *reinterpret_cast<Generic*>(reinterpret_cast<FINDREPLACE*>(lParam)->lCustData);
							window = hWnd;
							::SetWindowLongPtr( hWnd, GWL_USERDATA, reinterpret_cast<LPARAM>(&window) );
							Dialog::RegisterModeless( hWnd );
							return true;
						}

						case WM_DESTROY:

							*reinterpret_cast<Generic*>(::GetWindowLongPtr( hWnd, GWL_USERDATA )) = NULL;
							Dialog::UnregisterModeless( hWnd );
							return false;
					}

					return false;
				}
			};

			if (const uint id = ::RegisterWindowMessage(FINDMSGSTRING))
			{
				p.Messages().Add( id, this, &Finder::OnMsg );

				findReplace.lStructSize = sizeof(findReplace);
				findReplace.lpstrFindWhat = NULL;
				findReplace.wFindWhatLen = BUFFER_SIZE;
				findReplace.lCustData = reinterpret_cast<LPARAM>(&window);
				findReplace.lpfnHook = Hook::FRProc;
			}
		}

		void Finder::Close()
		{
			if (window)
				window.Destroy();
		}

		Finder::~Finder()
		{
			Close();

			delete [] findReplace.lpstrFindWhat;
			findReplace.lpstrFindWhat = NULL;
		}

		void Finder::Open(const Callback& c,const uint flags)
		{
			NST_VERIFY( findReplace.lStructSize );

			if (window == NULL && findReplace.lStructSize)
			{
				callback = c;

				if (findReplace.lpstrFindWhat == NULL)
				{
					findReplace.lpstrFindWhat = new wchar_t [BUFFER_SIZE+1];
					findReplace.lpstrFindWhat[0] = '\0';
				}

				findReplace.Flags = (flags & (DOWN|WHOLEWORD|MATCHCASE)) | FR_ENABLEHOOK;
				findReplace.hwndOwner = parent;

				::FindText( &findReplace );
			}
		}

		ibool Finder::OnMsg(Param&)
		{
			if (findReplace.Flags & FR_DIALOGTERM)
			{
				Close();
			}
			else if (findReplace.Flags & FR_FINDNEXT)
			{
				callback( findReplace.lpstrFindWhat, findReplace.Flags & (FR_WHOLEWORD|FR_MATCHCASE|FR_DOWN) );
			}

			return true;
		}
	}
}
