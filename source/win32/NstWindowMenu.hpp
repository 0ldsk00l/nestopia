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

#ifndef NST_WINDOW_MENU_H
#define NST_WINDOW_MENU_H

#pragma once

#include "language/resource.h"
#include "NstResourceMenu.hpp"
#include "NstSystemAccelerator.hpp"
#include "NstWindowCustom.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Menu
		{
		public:

			explicit Menu(uint);
			~Menu();

			void Hook(Custom&);
			void Unhook();
			uint Height() const;
			bool Visible() const;
			void Show(bool=true) const;
			bool Toggle() const;
			void EnableAccelerator(bool);
			void SetKeys(const ACCEL*,uint);
			void SetColor(COLORREF) const;
			void ResetColor() const;
			void ToggleModeless(bool) const;

			class Item;

			void Insert(const Item&,uint,GenericString) const;

		private:

			static void Clear(HMENU);
			static uint NumItems(HMENU);
			static void Redraw(const Custom*);

		public:

			void Redraw() const
			{
				Redraw( window );
			}

			class Item
			{
				friend class Menu;

				class Stream
				{
					friend class Menu;
					friend class Item;

					const Item& item;

					uint GetLength() const;
					bool GetFullString(wchar_t*,uint) const;
					void SetFullString(wcstring) const;

					template<typename T>
					void GetFullString(T& string) const
					{
						string.Resize( GetLength() );
						GetFullString( string.Ptr(), string.Length() );
					}

					explicit Stream(const Item& i)
					: item(i) {}

				public:

					void operator << (const GenericString&) const;

					template<typename T>
					uint operator >> (T& string) const
					{
						GetFullString( string );
						string.FirstOf( '\t' ).Clear();
						return string.Length();
					}
				};

			public:

				enum Type
				{
					NONE,
					SUBMENU,
					COMMAND,
					SEPARATOR
				};

				Type GetType() const;

				bool Enable (bool=true) const;
				bool Check  (bool=true) const;
				void Check  (uint,uint,bool) const;
				void Check  (uint,uint) const;

				bool Enabled () const;
				bool Checked () const;

				bool Disabled  () const { return !Enabled(); }
				bool Unchecked () const { return !Checked(); }

				bool ToggleCheck()  const { return !Check( Unchecked() ); }
				bool ToggleEnable() const { return !Enable( Disabled() ); }

				bool Disable () const { return Enable (false); }
				bool Uncheck () const { return Check  (false); }

				uint GetCommand() const;
				void Remove() const;
				void Clear() const;
				uint NumItems() const;
				bool Exists() const;

				Stream Text() const
				{
					return Stream( *this );
				}

			private:

				inline uint Flag() const;

				HMENU hMenu;
				uint pos;
				const Custom* const window;

			public:

				Item(const Custom* w,HMENU h=NULL,uint p=0)
				: hMenu(h), pos(p), window(w) {}

				HMENU GetHandle() const
				{
					return hMenu;
				}

				Item operator [] (uint subPos) const
				{
					return Item( window, ::GetSubMenu( hMenu, pos ), subPos );
				}
			};

			typedef Collection::Router<void,uint> CmdHandler;

			class PopupHandler
			{
				friend class Menu;

				struct Key
				{
					HMENU const hKey;
					const Item item;

					Key(const Custom* w,HMENU h,uint p)
					: hKey(::GetSubMenu(h,p)), item(w,h,p) {}

					explicit Key(WPARAM wParam)
					: hKey(reinterpret_cast<HMENU>(wParam)), item(NULL) {}

					operator HMENU() const
					{
						return hKey;
					}
				};

				Menu& menu;

			public:

				template<uint A,uint B=UINT_MAX,uint C=UINT_MAX,uint D=UINT_MAX> struct Pos
				{
					enum { ID = A | (B + 1) << 8 | (C + 2) << 16 | (D + 3) << 24 };
				};

				template<uint A> struct Pos<A,UINT_MAX,UINT_MAX,UINT_MAX>
				{
					enum { ID = A };
				};

				template<uint A,uint B> struct Pos<A,B,UINT_MAX,UINT_MAX>
				{
					enum { ID = A | (B + 1) << 8 };
				};

				template<uint A,uint B,uint C> struct Pos<A,B,C,UINT_MAX>
				{
					enum { ID = A | (B + 1) << 8 | (C + 2) << 16 };
				};

				struct Param
				{
					Item menu;
					bool show;

					Param(const Item& i,bool s)
					: menu(i), show(s) {}
				};

				template<typename Data> struct Entry
				{
					typedef void (Data::*Function)(const Param&);

					uint id;
					Function function;
				};

			private:

				typedef Collection::Router<void,const Param&,Key> Handler;

				Key GetKey(uint) const;

				template<typename Data>
				void Add(Data*,const Entry<Data>*,uint);

				PopupHandler(Menu& m)
				: menu(m) {}

			public:

				template<typename Data,uint COUNT>
				void Add(Data* data,const Entry<Data>(&list)[COUNT])
				{
					Add( data, list, COUNT );
				}

				void Remove(const void* data)
				{
					menu.popupHandler.Hooks().Remove( data );
				}
			};

		private:

			ibool OnCommand         (Param&);
			ibool OnInitMenuPopup   (Param&);
			ibool OnUninitMenuPopup (Param&);
			void  OnCreate          (Param&);
			void  OnDestroy         (Param&);

			enum {IDM_OFFSET = 100};

			Resource::Menu handle;
			Custom* window;
			System::Accelerator accelerator;
			CmdHandler cmdHandler;
			MsgHandler::Callback cmdCallback;
			PopupHandler::Handler popupHandler;
			bool acceleratorEnabled;

			class Instances
			{
			public:

				static void Update(Menu*);
				static void Remove(Menu*);
				static void EnableAccelerators(bool);

			private:

				static void Update();
				static bool TranslateNone   (MSG&);
				static bool TranslateSingle (MSG&);
				static bool TranslateMulti  (MSG&);

				typedef Collection::Vector<const Menu*> Menus;
				typedef bool (*Translator)(MSG&);

				static Translator translator;
				static Menus menus;
				static bool acceleratorsEnabled;

			public:

				static bool TransAccelerator(MSG& msg)
				{
					return translator( msg );
				}
			};

		public:

			HMENU GetHandle() const
			{
				return handle;
			}

			uint NumItems() const
			{
				return NumItems( handle );
			}

			Item operator [] (uint pos) const
			{
				return Item( window, handle, pos );
			}

			void Hide() const
			{
				Show( false );
			}

			void Clear() const
			{
				Clear( handle );
			}

			CmdHandler& Commands()
			{
				return cmdHandler;
			}

			PopupHandler Popups()
			{
				return *this;
			}

			static bool TransAccelerator(MSG& msg)
			{
				return Instances::TransAccelerator( msg );
			}

			static void EnableAccelerators(bool enable)
			{
				Instances::EnableAccelerators( enable );
			}
		};

		template<typename Data>
		void Menu::PopupHandler::Add(Data* const data,const Entry<Data>* list,const uint count)
		{
			for (const Entry<Data>* const end = list + count; list != end; ++list)
				menu.popupHandler.Hooks().Add( GetKey(list->id), data, list->function );
		}
	}
}

#endif
