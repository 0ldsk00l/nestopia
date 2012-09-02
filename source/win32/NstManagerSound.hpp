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

#ifndef NST_MANAGER_SOUND_H
#define NST_MANAGER_SOUND_H

#pragma once

#include "NstDirectSound.hpp"
#include "NstObjectHeap.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Sound;
	}

	namespace Managers
	{
		class Paths;
		class Preferences;

		class Sound : Manager
		{
		public:

			Sound(Window::Custom&,Window::Menu&,Emulator&,const Paths&,const Preferences&,const Configuration&);
			~Sound();

			void StartEmulation() const;
			void StopEmulation();
			void Save(Configuration&) const;

		private:

			void OnMenuOptionsSound(uint);
			void OnEmuEvent(Emulator::Event,Emulator::Data);
			void Disable(cstring=NULL);
			void UpdateSettings();
			bool CanRunInBackground() const;

			struct Callbacks;
			class Recorder;

			Nes::Sound::Output* emuOutput;
			const Paths& paths;
			const Preferences& preferences;
			Nes::Sound::Output output;
			DirectX::DirectSound directSound;
			Object::Heap<Window::Sound> dialog;
			Object::Heap<Recorder> recorder;

		public:

			Nes::Sound::Output* GetOutput()
			{
				return emuOutput;
			}
		};
	}
}

#endif
