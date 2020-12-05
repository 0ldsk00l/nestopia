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

#ifndef NST_MANAGER_MOVIE_H
#define NST_MANAGER_MOVIE_H

#pragma once

#include "NstObjectHeap.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Movie;
	}

	namespace Io
	{
		namespace Stream
		{
			class InOut;
		}
	}

	namespace Managers
	{
		class Paths;

		class Movie : Manager
		{
		public:

			Movie(Emulator&,Window::Menu&,const Paths&);
			~Movie();

			enum Alert
			{
				NOISY,
				QUIET
			};

			bool Load(const Path&,Alert);

		private:

			void OnMenu         (const Window::Menu::PopupHandler::Param&);
			void OnEmuEvent     (Emulator::Event,Emulator::Data);
			void OnCmdFile      (uint);
			void OnCmdRecord    (uint);
			void OnCmdPlay      (uint);
			void OnCmdStop      (uint);
			void OnCmdRewind    (uint);
			void OnCmdForward   (uint);
			void OnCmdExportAvi (uint);

			class File
			{
			public:

				explicit File(Emulator&);
				~File();

				enum Pos
				{
					POS_BEG,
					POS_END
				};

				enum Mode
				{
					MODE_PLAY,
					MODE_RECORD
				};

				bool Start(Mode);
				void Stop(Pos=POS_BEG);
				void Update(const Path&);
				bool Rewind();
				bool Forward();

				bool CanPlay() const;
				bool CanRecord() const;
				bool CanStop() const;
				bool CanRewind() const;
				bool CanForward() const;

				const Path& GetPath() const;

			private:

				bool IsReady() const;
				bool CanSetPos() const;

				Io::Stream::InOut* stream;
				Pos pos;
				Emulator& emulator;
				Path path;
			};

			File file;
			Object::Heap<Window::Movie> dialog;
			const Paths& paths;
		};
	}
}

#endif
