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

#include "NstApplicationException.hpp"
#include "NstApplicationLanguage.hpp"
#include "NstResourceIcon.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDialog.hpp"
#include "NstWindowDynamic.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Dialog::NoTaskbarWindow : Dynamic
		{
		public:

			NoTaskbarWindow(Window::Generic fakeParent)
			{
				Context context;

				const Point pos( fakeParent.Coordinates().Position() );

				context.className = L"Helper";
				context.x         = pos.x;
				context.y         = pos.y + 30;
				context.width     = 8;
				context.height    = 8;
				context.exStyle   = WS_EX_TOOLWINDOW;

				Create( context );
			}

			Generic GetHandle() const
			{
				return *this;
			}
		};

		Dialog::Instances Dialog::instances;
		Dialog::ModelessDialogs::Instances Dialog::ModelessDialogs::instances;
		Dialog::ModelessDialogs::Processor Dialog::ModelessDialogs::processor = Dialog::ModelessDialogs::ProcessNone;

		void Dialog::ModelessDialogs::Update()
		{
			switch (instances.Size())
			{
				case 0:  processor = ProcessNone; break;
				case 1:  processor = ProcessSingle; break;
				default: processor = ProcessMultiple; break;
			}
		}

		void Dialog::ModelessDialogs::Add(HWND const hWnd)
		{
			instances.PushBack( hWnd );
			Update();
		}

		bool Dialog::ModelessDialogs::Remove(HWND const hWnd)
		{
			if (Instances::Iterator const instance = instances.Find( hWnd ))
			{
				instances.Erase( instance );
				Update();
				return true;
			}

			return false;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		bool Dialog::ModelessDialogs::ProcessNone(MSG&)
		{
			NST_ASSERT( instances.Size() == 0 );

			return false;
		}

		bool Dialog::ModelessDialogs::ProcessSingle(MSG& msg)
		{
			NST_ASSERT( instances.Size() == 1 );

			return ::IsDialogMessage( instances.Front(), &msg );
		}

		bool Dialog::ModelessDialogs::ProcessMultiple(MSG& msg)
		{
			NST_ASSERT( instances.Size() >= 2 );

			Instances::ConstIterator dialog = instances.Begin();
			Instances::ConstIterator const end = instances.End();

			do
			{
				if (::IsDialogMessage( *dialog, &msg ))
					return true;
			}
			while (++dialog != end);

			return false;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		Dialog::Dialog(uint i)
		: id(i)
		{
			Init();
		}

		void Dialog::Init()
		{
			noTaskbarWindow = NULL;

			static const MsgHandler::HookEntry<Dialog> hooks[] =
			{
				{ WM_ENTERMENULOOP, &Dialog::OnIdle       },
				{ WM_ENTERSIZEMOVE, &Dialog::OnIdle       },
				{ WM_SYSCOMMAND,    &Dialog::OnSysCommand },
				{ WM_NCLBUTTONDOWN, &Dialog::OnNclButton  },
				{ WM_NCRBUTTONDOWN, &Dialog::OnNcrButton  },
				{ WM_MOUSEACTIVATE, &Dialog::OnIdle       }
			};

			msgHandler.Hooks().Add( this, hooks );
		}

		INT_PTR Dialog::Open(const Type type)
		{
			if (hWnd)
			{
				if (Visible())
					Activate();

				return 0;
			}

			Application::Instance::Events::Signal( Application::Instance::EVENT_SYSTEM_BUSY );

			NST_ASSERT( !noTaskbarWindow );

			if (type == MODAL)
			{
				for (uint i=0; i < 3; ++i)
				{
					static const uchar idc[3] = {IDOK,IDCANCEL,IDABORT};

					if (!cmdHandler( idc[i] ))
						cmdHandler.Add( idc[i], this, &Dialog::OnClose );
				}
			}

			if (cmdHandler.Size() && !msgHandler( WM_COMMAND ))
				msgHandler.Add( WM_COMMAND, this, &Dialog::OnCommand );

			if (!msgHandler( WM_CLOSE ))
				msgHandler.Add( WM_CLOSE, this, &Dialog::OnClose );

			if (type == MODAL)
			{
				return DialogBoxParam
				(
					Application::Instance::GetLanguage().GetResourceHandle(),
					MAKEINTRESOURCE(id),
					Application::Instance::GetActiveWindow(),
					DlgProc,
					reinterpret_cast<LPARAM>(this)
				);
			}

			Application::Instance::Waiter wait;

			if (type == MODELESS_FREE)
				noTaskbarWindow = new NoTaskbarWindow( Application::Instance::GetActiveWindow() );

			CreateDialogParam
			(
				Application::Instance::GetLanguage().GetResourceHandle(),
				MAKEINTRESOURCE(id),
				noTaskbarWindow ? noTaskbarWindow->GetHandle() : Application::Instance::GetActiveWindow(),
				DlgProc,
				reinterpret_cast<LPARAM>(this)
			);

			if (hWnd == NULL)
				throw Application::Exception( IDS_ERR_FAILED, L"CreateDialogParam()" );

			ModelessDialogs::Add( hWnd );

			return 0;
		}

		void Dialog::Close(const int ret)
		{
			if (hWnd)
			{
				Application::Instance::Events::Signal( Application::Instance::EVENT_SYSTEM_BUSY );

				if (ModelessDialogs::Remove( hWnd ))
				{
					::DestroyWindow( hWnd );
					NST_VERIFY( hWnd == NULL );
				}
				else
				{
					::EndDialog( hWnd, ret );
				}
			}

			delete noTaskbarWindow;
			noTaskbarWindow = NULL;
		}

		Dialog::~Dialog()
		{
			Close();
		}

		void Dialog::SetItemIcon(uint item,uint id) const
		{
			::SendDlgItemMessage( hWnd, item, STM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(static_cast<HICON>(Resource::Icon(id))) );
		}

		ibool Dialog::OnClose(Param&)
		{
			Close();
			return true;
		}

		void Dialog::OnIdle(Param&)
		{
			Application::Instance::Events::Signal( Application::Instance::EVENT_SYSTEM_BUSY );
		}

		void Dialog::OnSysCommand(Param& param)
		{
			switch (param.wParam & 0xFFF0)
			{
				case SC_MINIMIZE:
				case SC_RESTORE:

					Application::Instance::Events::Signal( Application::Instance::EVENT_SYSTEM_BUSY );
					break;
			}
		}

		void Dialog::OnNclButton(Param& param)
		{
			switch (param.wParam)
			{
				case HTCAPTION:
				case HTMINBUTTON:
				case HTMAXBUTTON:
				case HTCLOSE:

					Application::Instance::Events::Signal( Application::Instance::EVENT_SYSTEM_BUSY );
					break;
			}
		}

		void Dialog::OnNcrButton(Param& param)
		{
			switch (param.wParam)
			{
				case HTCAPTION:
				case HTSYSMENU:
				case HTMINBUTTON:

					Application::Instance::Events::Signal( Application::Instance::EVENT_SYSTEM_BUSY );
					break;
			}
		}

		void Dialog::Fetch(HWND const handle)
		{
			NST_ASSERT( handle && !hWnd );

			hWnd = handle;
			instances.PushBack( this );
		}

		void Dialog::Ditch(Instances::Iterator const instance)
		{
			NST_ASSERT( hWnd );

			ModelessDialogs::Remove( hWnd );
			instances.Erase( instance );
			hWnd = NULL;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		ibool Dialog::OnCommand(Param& param)
		{
			if (const MsgHandler::Item* const item = cmdHandler( LOWORD(param.wParam) ))
				return item->callback( param );

			return false;
		}

		BOOL CALLBACK Dialog::DlgProc(HWND const hWnd,const uint uMsg,WPARAM const wParam,LPARAM const lParam)
		{
			if (uMsg == WM_INITDIALOG && lParam)
				reinterpret_cast<Dialog*>(lParam)->Fetch( hWnd );

			BOOL ret = false;

			for (Instances::Iterator it(instances.Begin()), end(instances.End()); it != end; ++it)
			{
				Dialog& dialog = **it;

				if (dialog.hWnd == hWnd)
				{
					if (const MsgHandler::Item* const item = dialog.msgHandler( uMsg ))
					{
						Param param = {wParam,lParam,0,hWnd};
						ret = item->callback( param );
					}

					if (uMsg == WM_DESTROY)
						dialog.Ditch( it );

					break;
				}
			}

			return ret;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
