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

#include "language/resource.h"
#include "NstResourceCursor.hpp"
#include "NstResourceVersion.hpp"
#include "NstApplicationException.hpp"
#include "NstApplicationLanguage.hpp"
#include <CommCtrl.h>
#include <ObjBase.h>

#if NST_MSVC
#pragma comment(lib,"comctl32")
#endif

namespace Nestopia
{
	namespace Application
	{
		const wchar_t Instance::appClassName[] = L"Nestopia";

		struct Instance::Global
		{
			Global();

			struct Paths
			{
				explicit Paths(HINSTANCE);

				Path exePath;
				Path wrkPath;
			};

			struct Hooks
			{
				explicit Hooks(HINSTANCE);
				~Hooks();

				void OnCreate(const CREATESTRUCT&,HWND);
				void OnDestroy(HWND);

				static LRESULT CALLBACK CBTProc(int,WPARAM,LPARAM);

				enum
				{
					CHILDREN_RESERVE = 8
				};

				typedef Collection::Vector<HWND> Children;

				HHOOK const handle;
				HWND window;
				Children children;
			};

			const Paths paths;
			const Resource::Version version;
			Language language;
			Hooks hooks;
			IconStyle iconStyle;
		};

		Instance::Global Instance::global;
		Instance::Events::Callbacks Instance::Events::callbacks;

		Instance::Global::Paths::Paths(HINSTANCE const hInstance)
		{
			uint length;

			do
			{
				exePath.Reserve( exePath.Capacity() + 255 );
				length = ::GetModuleFileName( hInstance, exePath.Ptr(), exePath.Capacity() );
			}
			while (length == exePath.Capacity());

			exePath.ShrinkTo( length );
			exePath.MakePretty( false );
			exePath.Defrag();

			wrkPath = ".";
			wrkPath.MakeAbsolute( true );
			wrkPath.Defrag();
		}

		Instance::Global::Hooks::Hooks(HINSTANCE const hInstance)
		: handle(::SetWindowsHookEx( WH_CBT, CBTProc, hInstance, ::GetCurrentThreadId() ))
		{
			children.Reserve( CHILDREN_RESERVE );
		}

		Instance::Global::Hooks::~Hooks()
		{
			if (handle)
				::UnhookWindowsHookEx( handle );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		void Instance::Global::Hooks::OnCreate(const CREATESTRUCT& createStruct,HWND const hWnd)
		{
			NST_ASSERT( hWnd );

			if (window == createStruct.hwndParent || !window)
			{
				{
					static const wchar_t menuClassName[] = L"#32768"; // documented on MSDN

					enum
					{
						MAX_LENGTH = NST_MAX( sizeof(array(appClassName)) - 1,
                                     NST_MAX( sizeof(array(STATUSCLASSNAME)) - 1, sizeof(array(menuClassName)) - 1 ))
					};

					String::Stack<MAX_LENGTH,wchar_t> name;
					name.ShrinkTo( ::GetClassName( hWnd, name.Ptr(), MAX_LENGTH+1 ) );

					if (name.Equal( menuClassName ) || name.Equal( STATUSCLASSNAME ) || name.Equal( L"IME" ))
						return;

					if (window)
					{
						children.PushBack( hWnd );
						Events::Signal( EVENT_SYSTEM_BUSY );
					}
					else if (name.Equal( appClassName ))
					{
						window = hWnd;
					}
					else
					{
						return;
					}
				}

				const Events::WindowCreateParam param =
				{
					hWnd,
					createStruct.cx > 0 ? createStruct.cx : 0,
					createStruct.cy > 0 ? createStruct.cy : 0
				};

				Events::Signal( EVENT_WINDOW_CREATE, &param );
			}
		}

		void Instance::Global::Hooks::OnDestroy(HWND const hWnd)
		{
			NST_ASSERT( hWnd );

			Children::Iterator const child = children.Find( hWnd );

			if (child)
			{
				Events::Signal( EVENT_SYSTEM_BUSY );
			}
			else if (window != hWnd)
			{
				return;
			}

			{
				const Events::WindowDestroyParam param = { hWnd };
				Events::Signal( EVENT_WINDOW_DESTROY, &param );
			}

			if (child)
				children.Erase( child );
			else
				window = NULL;
		}

		LRESULT CALLBACK Instance::Global::Hooks::CBTProc(const int nCode,const WPARAM wParam,const LPARAM lParam)
		{
			NST_COMPILE_ASSERT( HCBT_CREATEWND >= 0 && HCBT_DESTROYWND >= 0 );

			switch (nCode)
			{
				case HCBT_CREATEWND:

					NST_ASSERT( lParam );

					if (const CREATESTRUCT* const createStruct = reinterpret_cast<const CBT_CREATEWND*>(lParam)->lpcs)
					{
						NST_VERIFY( wParam );
						global.hooks.OnCreate( *createStruct, reinterpret_cast<HWND>(wParam) );
					}

					return 0;

				case HCBT_DESTROYWND:

					NST_VERIFY( wParam );

					if (wParam)
						global.hooks.OnDestroy( reinterpret_cast<HWND>(wParam) );

					return 0;
			}

			return nCode < 0 ? ::CallNextHookEx( global.hooks.handle, nCode, wParam, lParam ) : 0;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		Instance::Global::Global()
		:
		paths     ( ::GetModuleHandle(NULL) ),
		version   ( paths.exePath.Ptr(), Resource::Version::PRODUCT ),
		hooks     ( ::GetModuleHandle(NULL) ),
		iconStyle ( ICONSTYLE_NES )
		{
		}

		Instance::Language& Instance::GetLanguage()
		{
			return global.language;
		}

		const String::Heap<char>& Instance::GetVersion()
		{
			return global.version;
		}

		const Path& Instance::GetExePath()
		{
			return global.paths.exePath;
		}

		const Path Instance::GetExePath(const GenericString file)
		{
			return Path( global.paths.exePath.Directory(), file );
		}

		const Path Instance::GetFullPath(const GenericString relPath)
		{
			Path curPath( "." );
			curPath.MakeAbsolute( true );

			const BOOL succeeded = ::SetCurrentDirectory( global.paths.wrkPath.Ptr() );

			Path wrkPath( relPath );
			wrkPath.MakeAbsolute();

			if (succeeded)
				::SetCurrentDirectory( curPath.Ptr() );

			return wrkPath;
		}

		const Path Instance::GetLongPath(wcstring const shortPath)
		{
			Path longPath;

			longPath.Reserve( 255 );
			uint length = ::GetLongPathName( shortPath, longPath.Ptr(), longPath.Capacity() + 1 );

			if (length > longPath.Capacity() + 1)
			{
				longPath.Reserve( length - 1 );
				length = ::GetLongPathName( shortPath, longPath.Ptr(), longPath.Capacity() + 1 );
			}

			if (!length)
				return shortPath;

			const wchar_t slashed = GenericString(shortPath).Back();

			longPath.ShrinkTo( length );
			longPath.MakePretty( slashed == '\\' || slashed == '/' );

			return longPath;
		}

		const Path Instance::GetTmpPath(GenericString file)
		{
			Path path;
			path.Reserve( 255 );

			if (uint length = ::GetTempPath( path.Capacity() + 1, path.Ptr() ))
			{
				if (length > path.Capacity() + 1)
				{
					path.Reserve( length - 1 );
					length = ::GetTempPath( path.Capacity() + 1, path.Ptr() );
				}

				if (length)
					path = GetLongPath( path.Ptr() );
			}

			if (path.Length())
				path.MakePretty( true );
			else
				path << ".\\";

			if (file.Empty())
				file = L"nestopia.tmp";

			path << file;

			return path;
		}

		Instance::Events::Callbacks::~Callbacks()
		{
			NST_VERIFY( Empty() );
		}

		void Instance::Events::Add(const Callback& callback)
		{
			NST_ASSERT( bool(callback) && !callbacks.Find( callback ) );
			callbacks.PushBack( callback );
		}

		void Instance::Events::Signal(const Event event,const void* param)
		{
			for (Callbacks::ConstIterator it(callbacks.Begin()), end(callbacks.End()); it != end; ++it)
				(*it)( event, param );
		}

		void Instance::Events::Remove(const void* const instance)
		{
			for (Callbacks::Iterator it(callbacks.Begin()); it != callbacks.End(); ++it)
			{
				if (it->VoidPtr() == instance)
				{
					callbacks.Erase( it );
					break;
				}
			}
		}

		Window::Generic Instance::GetMainWindow()
		{
			return global.hooks.window;
		}

		uint Instance::NumChildWindows()
		{
			return global.hooks.children.Size();
		}

		Window::Generic Instance::GetChildWindow(uint i)
		{
			return global.hooks.children[i];
		}

		Window::Generic Instance::GetActiveWindow()
		{
			HWND hWnd = ::GetActiveWindow();
			return hWnd ? hWnd : global.hooks.window;
		}

		Instance::IconStyle Instance::GetIconStyle()
		{
			return global.iconStyle;
		}

		void Instance::SetIconStyle(IconStyle style)
		{
			global.iconStyle = style;
		}

		void Instance::ShowChildWindows(bool show)
		{
			const uint state = (show ? SW_SHOWNA : SW_HIDE);

			for (Global::Hooks::Children::ConstIterator it(global.hooks.children.Begin()), end(global.hooks.children.End()); it != end; ++it)
				::ShowWindow( *it, state );
		}

		Instance::Waiter::Waiter()
		: hCursor(::SetCursor(Resource::Cursor::GetWait())) {}

		Instance::Locker::Locker()
		: hWnd(GetMainWindow()), enabled(::IsWindowEnabled(hWnd))
		{
			::EnableWindow( hWnd, false );
			::LockWindowUpdate( hWnd );
		}

		bool Instance::Locker::CheckInput(int vKey) const
		{
			if (::GetAsyncKeyState( vKey ) & 0x8000U)
			{
				while (::GetAsyncKeyState( vKey ) & 0x8000U)
					::Sleep( 10 );

				return true;
			}

			return false;
		}

		Instance::Locker::~Locker()
		{
			::LockWindowUpdate( NULL );
			::EnableWindow( hWnd, enabled );
			for (MSG msg;::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE|PM_QS_INPUT ););
		}

		Instance::Instance()
		{
			if (!static_cast<const Configuration&>(cfg)["preferences"]["application"]["allow-multiple-instances"].Yes())
			{
				::CreateMutex( NULL, true, L"Nestopia Mutex" );

				if (::GetLastError() == ERROR_ALREADY_EXISTS)
				{
					if (Window::Generic window = Window::Generic::Find( appClassName ))
					{
						window.Activate();

						if (const uint length = cfg.GetStartupFile().Length())
						{
							COPYDATASTRUCT cds;

							cds.dwData = COPYDATA_OPENFILE_ID;
							cds.cbData = (length + 1) * sizeof(wchar_t);
							cds.lpData = const_cast<wchar_t*>(cfg.GetStartupFile().Ptr());

							window.Send( WM_COPYDATA, 0, &cds );
						}
					}

					throw Exception();
				}
			}

			if (global.paths.exePath.Empty())
				throw Exception( L"unicows.dll file is missing!" );

			global.language.Load( cfg );

			if (global.hooks.handle == NULL)
				throw Exception( IDS_ERR_FAILED, L"SetWindowsHookEx()" );

			if (FAILED(::CoInitializeEx( NULL, COINIT_APARTMENTTHREADED )))
				throw Exception( IDS_ERR_FAILED, L"CoInitializeEx()" );

			Object::Pod<INITCOMMONCONTROLSEX> initCtrlEx;

			initCtrlEx.dwSize = sizeof(initCtrlEx);
			initCtrlEx.dwICC = ICC_WIN95_CLASSES;
			::InitCommonControlsEx( &initCtrlEx );
		}

		Instance::~Instance()
		{
			::CoUninitialize();
		}

		void Instance::Save()
		{
			global.language.Save( cfg );
		}
	}
}
