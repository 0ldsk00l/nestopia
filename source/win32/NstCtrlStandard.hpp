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

#ifndef NST_CTRL_STANDARD_H
#define NST_CTRL_STANDARD_H

#pragma once

#include <map>
#include "NstWindowGeneric.hpp"

namespace Nestopia
{
	namespace Window
	{
		namespace Control
		{
			class Generic
			{
			public:

				bool FixedFont() const;
				Point GetMaxTextSize() const;

			protected:

				Window::Generic control;

			public:

				Generic(HWND hWnd=NULL)
				: control(hWnd) {}

				Generic(HWND hWnd,uint id)
				: control( ::GetDlgItem( hWnd, id ) )
				{
					NST_VERIFY( control );
				}

				void Enable(bool state=true) const
				{
					control.Enable( state );
				}

				void Disable() const
				{
					Enable( false );
				}

				bool Enabled() const
				{
					return control.Enabled();
				}

				void Redraw() const
				{
					control.Redraw();
				}

				Window::Generic GetWindow() const
				{
					return control;
				}

				HWND GetHandle() const
				{
					return control;
				}

				Window::Generic::Stream Text() const
				{
					return control;
				}
			};

			class NotificationHandler
			{
			public:

				template<typename T> struct Entry
				{
					uint msg;
					void (T::*function)(const NMHDR&);
				};

			private:

				typedef Object::Delegate<void,const NMHDR&> Callback;
				typedef std::map<uint,Callback> Items;

				const uint control;
				Items items;
				MsgHandler& msgHandler;

				void Initialize();
				void OnNotify(Param&);

				template<typename T>
				void Add(T*,const Entry<T>*,uint);

				Callback& operator () (uint id)
				{
					return items[id];
				}

			public:

				NotificationHandler(uint,MsgHandler&);

				template<typename T,uint N>
				NotificationHandler(uint,MsgHandler&,T*,const Entry<T>(&)[N]);

				~NotificationHandler();

				template<typename T,uint N>
				void Add(T* data,const Entry<T>(&entries)[N])
				{
					Add( data, entries, N );
				}
			};

			template<typename T,uint N>
			NotificationHandler::NotificationHandler
			(
				const uint id,
				MsgHandler& m,
				T* const data,
				const Entry<T>(&entries)[N]
			)
			: control(id), msgHandler(m)
			{
				Initialize();
				Add( data, entries, N );
			}

			template<typename T>
			void NotificationHandler::Add(T* const object,const Entry<T>* it,const uint count)
			{
				for (const Entry<T>* const end = it + count; it != end; ++it)
					(*this)( it->msg ).Set( object, it->function );
			}
		}
	}
}

#endif
