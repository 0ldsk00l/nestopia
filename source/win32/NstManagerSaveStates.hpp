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

#ifndef NST_MANAGER_SAVESTATES_H
#define NST_MANAGER_SAVESTATES_H

#pragma once

#include "NstSystemTime.hpp"

namespace Nestopia
{
	namespace Window
	{
		class AutoSaver;
		class Main;
	}

	namespace Managers
	{
		class SaveStates : Manager
		{
		public:

			SaveStates(Emulator&,Window::Menu&,const Paths&,const Window::Main&);
			~SaveStates();

			void Load(Collection::Buffer&,GenericString=GenericString()) const;

		private:

			enum
			{
				NUM_SLOTS = 9
			};

			struct Slot
			{
				struct Compare;

				System::Time time;
				Collection::Buffer data;
			};

			void ImportSlots(bool);
			void ExportSlot(uint);
			void UpdateMenuTexts() const;

			void SaveToSlot(uint,bool=true);
			void LoadFromSlot(uint,bool=true);

			void ToggleAutoSaver(bool);

			void OnEmuEvent(Emulator::Event,Emulator::Data);
			uint OnTimerAutoSave();

			void OnCmdStateLoad        (uint);
			void OnCmdStateSave        (uint);
			void OnCmdSlotSave         (uint);
			void OnCmdSlotSaveOldest   (uint);
			void OnCmdSlotLoad         (uint);
			void OnCmdSlotLoadNewest   (uint);
			void OnCmdAutoSaverOptions (uint);
			void OnCmdAutoSaverStart   (uint);

			const Window::Main& window;
			const Paths& paths;
			bool autoSaveEnabled;
			Object::Heap<Window::AutoSaver> autoSaver;
			Slot slots[NUM_SLOTS];
		};
	}
}

#endif
