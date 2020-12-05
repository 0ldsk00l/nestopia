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

#include <cstring>
#include "NstResourceString.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDialog.hpp"
#include "NstWindowUser.hpp"
#include "NstApplicationInstance.hpp"
#include <ShellAPI.h>

namespace Nestopia
{
	namespace Window
	{
		namespace User
		{
			class InputDialog
			{
				wcstring const text;
				wcstring const title;
				HeapString& response;
				Dialog dialog;

				ibool OnInitDialog(Param&)
				{
					dialog.Text() << title;

					if (text)
						dialog.Edit(IDC_USER_INPUT_TEXT) << text;

					if (response.Length())
						dialog.Edit(IDC_USER_INPUT_EDIT) << response.Ptr();

					return true;
				}

				ibool OnCmdOk(Param&)
				{
					dialog.Edit(IDC_USER_INPUT_EDIT) >> response;
					dialog.Close(true);
					return true;
				}

				ibool OnCmdAbort(Param&)
				{
					dialog.Close(false);
					return true;
				}

			public:

				InputDialog(wcstring const t,wcstring const i,HeapString& r)
				: text(t), title(i), response(r), dialog(IDD_USER_INPUT)
				{
					dialog.Messages().Add( WM_INITDIALOG, this, &InputDialog::OnInitDialog );
					dialog.Commands().Add( IDOK, this, &InputDialog::OnCmdOk );
					dialog.Commands().Add( IDABORT, this, &InputDialog::OnCmdAbort );
				}

				INT_PTR Open()
				{
					return dialog.Open();
				}
			};

			class ChooseDialog
			{
				wcstring const title;
				wcstring const otherCmd;
				wcstring* const choices;
				const uint* indices;
				const uint numChoices;
				Dialog dialog;

				ibool OnInitDialog(Param&)
				{
					dialog.Text() << title;

					dialog.Control(IDABORT).Text() << otherCmd;

					const Window::Control::ListBox listBox( dialog.ListBox(IDC_CHOOSE_LIST) );
					Window::Control::ListBox::HScrollBar hScrollBar( listBox.GetWindow() );

					listBox.Reserve( numChoices );

					for (uint i=0; i < numChoices; ++i)
					{
						hScrollBar.Update( choices[i], std::wcslen(choices[i]) );
						listBox.Add( choices[i] ).Data() = (indices ? indices[i] : i);
					}

					NST_VERIFY( listBox.Size() );

					listBox[0].Select();

					return true;
				}

				ibool OnCmdDblClk(Window::Param& param)
				{
					if (HIWORD(param.wParam) == LBN_DBLCLK)
					{
						dialog.Close( dialog.ListBox(IDC_CHOOSE_LIST).Selection().Data() + 1 );
						return true;
					}

					return false;
				}

				ibool OnCmdOk(Param& param)
				{
					if (param.Button().Clicked())
						dialog.Close( dialog.ListBox(IDC_CHOOSE_LIST).Selection().Data() + 1 );

					return true;
				}

			public:

				ChooseDialog(wcstring t,wcstring o,wcstring* c,const uint* d,uint n)
				: title(t), otherCmd(o), choices(c), indices(d), numChoices(n), dialog(IDD_CHOOSE)
				{
					dialog.Messages().Add( WM_INITDIALOG, this, &ChooseDialog::OnInitDialog );
					dialog.Commands().Add( IDC_CHOOSE_LIST, this, &ChooseDialog::OnCmdDblClk );
					dialog.Commands().Add( IDOK, this, &ChooseDialog::OnCmdOk );
				}

				uint Open()
				{
					return dialog.Open();
				}
			};

			static int Present(wcstring const text,wcstring const title,const uint flags)
			{
				return ::MessageBox
				(
					Application::Instance::GetActiveWindow(),
					text,
					title,
					MB_TOPMOST|MB_SETFOREGROUND|flags
				);
			}

			void Fail(wcstring const text,wcstring const title)
			{
				Present( text, title && *title ? title : L"Nestopia Error", MB_OK|MB_ICONERROR );
			}

			void Fail(wcstring const text,const uint titleId)
			{
				Fail( text, Resource::String(titleId ? titleId : IDS_TITLE_ERROR).Ptr() );
			}

			void Fail(const uint textId,const uint titleId)
			{
				Fail( Resource::String(textId).Ptr(), titleId );
			}

			void Warn(wcstring const text,wcstring const title)
			{
				Present( text, title && *title ? title : L"Nestopia Warning", MB_OK|MB_ICONWARNING );
			}

			void Warn(wcstring const text,const uint titleId)
			{
				Warn( text, Resource::String(titleId ? titleId : IDS_TITLE_WARNING).Ptr() );
			}

			void Warn(const uint textId,const uint titleId)
			{
				Warn( Resource::String(textId).Ptr(), titleId );
			}

			void Inform(wcstring const text,wcstring const title)
			{
				Present( text, title && *title ? title : L"Nestopia", MB_OK|MB_ICONINFORMATION );
			}

			void Inform(wcstring const text,const uint titleId)
			{
				Inform( text, Resource::String(titleId ? titleId : IDS_TITLE_NESTOPIA).Ptr() );
			}

			void Inform(const uint textId,const uint titleId)
			{
				Inform( Resource::String(textId).Ptr(), titleId );
			}

			bool Confirm(wcstring const text,wcstring const title)
			{
				return Present( text, title && *title ? title : L"Nestopia", MB_YESNO|MB_ICONQUESTION ) == IDYES;
			}

			bool Confirm(wcstring const text,const uint titleId)
			{
				return Confirm( text, Resource::String(titleId ? titleId : IDS_TITLE_NESTOPIA).Ptr() );
			}

			bool Confirm(const uint textId,const uint titleId)
			{
				return Confirm( Resource::String(textId), titleId );
			}

			bool Input (HeapString& response,wcstring const text,wcstring const title)
			{
				return InputDialog( text, title ? title : Resource::String(IDS_TITLE_NESTOPIA), response ).Open();
			}

			uint Choose(uint titleId,uint cmdOtherId,wcstring* choices,uint numChoices,const uint* indices)
			{
				return ChooseDialog
				(
					titleId ? Resource::String(titleId).Ptr() : L"Choose",
					cmdOtherId ? Resource::String(cmdOtherId).Ptr() : L"Abort",
					choices,
					indices,
					numChoices
				).Open();
			}
		}
	}
}
