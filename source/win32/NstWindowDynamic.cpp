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
#include "NstObjectPod.hpp"
#include "NstApplicationException.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDynamic.hpp"

namespace Nestopia
{
	namespace Window
	{
		Dynamic::Instances Dynamic::instances;

		Dynamic::Dynamic()
		{
			Initialize();
		}

		Dynamic::~Dynamic()
		{
			Destroy();
		}

		void Dynamic::Initialize()
		{
			Messages().Add( WM_NCDESTROY, this, &Dynamic::OnNcDestroy );
		}

		void Dynamic::Create(Context& create)
		{
			NST_ASSERT( create.className && *create.className );

			Object::Pod<WNDCLASSEX> winClass;

			winClass.cbSize        = sizeof(winClass);
			winClass.style         = create.classStyle;
			winClass.lpfnWndProc   = WndProcCreate;
			winClass.hInstance     = ::GetModuleHandle(NULL);
			winClass.hCursor       = create.hCursor;
			winClass.hbrBackground = create.hBackground;
			winClass.lpszClassName = create.className;
			winClass.hIcon         =
			winClass.hIconSm       = create.hIcon;

			if (!RegisterClassEx( &winClass ))
				throw Application::Exception( IDS_ERR_FAILED, L"RegisterClassEx()" );

			className = create.className;

			CreateWindowEx
			(
				create.exStyle,
				create.className,
				create.windowName,
				create.winStyle,
				create.x,
				create.y,
				create.width,
				create.height,
				create.hParent,
				create.hMenu,
				::GetModuleHandle(NULL),
				this
			);

			if (!hWnd)
				throw Application::Exception( IDS_ERR_FAILED, L"CreateWindowEx()" );
		}

		void Dynamic::Destroy()
		{
			Custom::Destroy();

			if (className.Length())
			{
				UnregisterClass( className.Ptr(), ::GetModuleHandle(NULL) );
				className.Clear();
			}
		}

		void Dynamic::OnCreate(HWND const handle)
		{
			NST_ASSERT( handle && !hWnd );

			hWnd = handle;
			instances.PushBack( this );

			const LONG_PTR ptr = reinterpret_cast<LONG_PTR>( instances.Size() == 1 ? WndProcSingle : WndProcMulti );

			if (!::SetWindowLongPtr( hWnd, GWLP_WNDPROC, ptr ))
				throw Application::Exception( IDS_ERR_FAILED, L"SetWindowLongPtr()" );
		}

		ibool Dynamic::OnNcDestroy(Param&)
		{
			NST_ASSERT( hWnd );

			Instances::Iterator instance;

			for (instance = instances.Ptr(); (*instance)->hWnd != hWnd; ++instance);

			instances.Erase( instance );
			hWnd = NULL;

			return false;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		LRESULT CALLBACK Dynamic::WndProcSingle(HWND const hWnd,const uint uMsg,WPARAM const wParam,LPARAM const lParam)
		{
			if (const MsgHandler::Item* const item = instances.Front()->msgHandler( uMsg ))
			{
				Param param = {wParam,lParam,0,hWnd};

				if (item->callback( param ))
					return param.lResult;
			}

			return ::DefWindowProc( hWnd, uMsg, wParam, lParam );
		}

		LRESULT CALLBACK Dynamic::WndProcMulti(HWND const hWnd,const uint uMsg,WPARAM const wParam,LPARAM const lParam)
		{
			Instances::ConstIterator instance;

			for (instance = instances.Ptr(); (*instance)->hWnd != hWnd; ++instance);

			if (const MsgHandler::Item* const item = (*instance)->msgHandler( uMsg ))
			{
				Param param = {wParam,lParam,0,hWnd};

				if (item->callback( param ))
					return param.lResult;
			}

			return ::DefWindowProc( hWnd, uMsg, wParam, lParam );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		LRESULT CALLBACK Dynamic::WndProcCreate(HWND const hWnd,const uint uMsg,WPARAM const wParam,LPARAM const lParam)
		{
			if (uMsg == WM_CREATE && lParam)
			{
				if (Dynamic* const instance = static_cast<Dynamic*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams))
				{
					instance->OnCreate( hWnd );

					if (const MsgHandler::Item* const item = instance->msgHandler( uMsg ))
					{
						Param param = {wParam,lParam,0,hWnd};

						if (item->callback( param ))
							return param.lResult;
					}
				}
			}

			return ::DefWindowProc( hWnd, uMsg, wParam, lParam );
		}
	}
}
