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

#ifndef NST_DIALOG_NETPLAY_H
#define NST_DIALOG_NETPLAY_H

#pragma once

#include <set>
#include "NstWindowDialog.hpp"

namespace Nestopia
{
	namespace Managers
	{
		class Paths;
		class Emulator;
	}

	namespace Window
	{
		class Netplay
		{
		public:

			Netplay(Managers::Emulator&,const Managers::Paths&,bool);
			~Netplay();

			void SaveFile() const;
			wcstring GetPath(wcstring) const;

			class Chat
			{
			public:

				typedef int (WINAPI *Callback)(char*);

				explicit Chat(Callback);
				~Chat();

				void Close();
				void Open();

			private:

				struct Handlers;

				ibool OnInit    (Param&);
				ibool OnCommand (Param&);

				Dialog dialog;
				const Callback callback;
				String::Heap<char> text;
			};

		private:

			struct Handlers;

			enum
			{
				LAUNCH = 0xB00B
			};

			struct Games
			{
				Games();

				struct Less
				{
					bool operator () (const Path&,const Path&) const;
				};

				typedef std::set<Path,Less> Paths;

				enum
				{
					LIMIT = 100
				};

				enum State
				{
					CLEAN,
					DIRTY,
					UNINITIALIZED
				};

				State state;
				Paths paths;
			};

		public:

			typedef Games::Paths GamePaths;

		private:

			void LoadFile();
			void Add(Path);

			ibool OnInitDialog (Param&);
			ibool OnAdd        (Param&);
			ibool OnRemove     (Param&);
			ibool OnClear      (Param&);
			ibool OnDefault    (Param&);
			ibool OnLaunch     (Param&);
			ibool OnFullscreen (Param&);
			ibool OnDropFiles  (Param&);

			void OnKeyDown     (const NMHDR&);
			void OnItemChanged (const NMHDR&);
			void OnInsertItem  (const NMHDR&);
			void OnDeleteItem  (const NMHDR&);

			Dialog dialog;
			bool doFullscreen;
			const Managers::Paths& paths;
			Managers::Emulator& emulator;
			const Control::NotificationHandler notifications;
			Games games;

		public:

			bool Open()
			{
				return dialog.Open() == LAUNCH;
			}

			bool ShouldGoFullscreen() const
			{
				return doFullscreen;
			}

			const GamePaths& GetGamePaths() const
			{
				return games.paths;
			}
		};
	}
}

#endif
