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

#ifndef NST_APPLICATION_INSTANCE_H
#define NST_APPLICATION_INSTANCE_H

#pragma once

#include "NstObjectDelegate.hpp"
#include "NstCollectionVector.hpp"
#include "NstApplicationConfiguration.hpp"
#include "NstWindowGeneric.hpp"

namespace Nestopia
{
	namespace Application
	{
		class Instance
		{
		public:

			Instance();
			~Instance();

			void Save();

			static const Path& GetExePath();
			static const Path GetExePath(const GenericString);
			static const Path GetLongPath(wcstring);
			static const Path GetTmpPath(GenericString=GenericString());
			static const Path GetFullPath(const GenericString);
			static const String::Heap<char>& GetVersion();

			static uint NumChildWindows();
			static void ShowChildWindows(bool=true);
			static Window::Generic GetChildWindow(uint=0);
			static Window::Generic GetMainWindow();
			static Window::Generic GetActiveWindow();

			class Language;

			static Language& GetLanguage();

			enum IconStyle
			{
				ICONSTYLE_NES,
				ICONSTYLE_FAMICOM
			};

			static IconStyle GetIconStyle();
			static void SetIconStyle(IconStyle);

			enum
			{
				WM_NST_COMMAND_RESUME = WM_APP + 55,
				WM_NST_LAUNCH = WM_APP + 56,
				COPYDATA_OPENFILE_ID = 0xDEAF
			};

			enum Event
			{
				EVENT_WINDOW_CREATE = 1,
				EVENT_WINDOW_DESTROY,
				EVENT_SYSTEM_BUSY,
				EVENT_FULLSCREEN,
				EVENT_DESKTOP
			};

			class Events
			{
			public:

				struct WindowCreateParam
				{
					HWND hWnd;
					uint x;
					uint y;
				};

				struct WindowDestroyParam
				{
					HWND hWnd;
				};

			private:

				typedef Object::Delegate<void,Event,const void*> Callback;

				struct Callbacks : Collection::Vector<Callback>
				{
					~Callbacks();
				};

				static void Add(const Callback&);

				static Callbacks callbacks;

			public:

				static void Remove(const void*);
				static void Signal(Event,const void* = NULL);

				template<typename Data,typename Code>
				static void Add(Data* data,Code code)
				{
					Add( Callback(data,code) );
				}
			};

		private:

			Configuration cfg;

			struct Global;
			static Global global;

			static const wchar_t appClassName[];

		public:

			Configuration& GetConfiguration2()
			{
				return cfg;
			}

			static wcstring GetClassName()
			{
				return appClassName;
			}

			class Waiter
			{
				HCURSOR const hCursor;

			public:

				Waiter();

				~Waiter()
				{
					::SetCursor( hCursor );
				}
			};

			class Locker
			{
				HWND const hWnd;
				const bool enabled;

			public:

				Locker();
				~Locker();

				bool CheckInput(int) const;
			};
		};
	}
}

#endif
