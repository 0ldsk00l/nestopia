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

#include "NstObjectHeap.hpp"
#include "NstIoLog.hpp"
#include "NstManager.hpp"
#include "NstManagerPaths.hpp"
#include "NstManagerCheats.hpp"
#include "NstDialogCheats.hpp"
#include "../core/api/NstApiCartridge.hpp"

namespace Nestopia
{
	namespace Managers
	{
		Cheats::Cheats(Emulator& e,const Configuration& cfg,Window::Menu& m,const Paths& p)
		:
		Manager ( e, m, this, &Cheats::OnEmuEvent, IDM_OPTIONS_CHEATS, &Cheats::OnCmdOptions ),
		paths   ( p ),
		game    ( false ),
		dialog  ( new Window::Cheats(e,cfg,p) )
		{
		}

		Cheats::~Cheats()
		{
		}

		void Cheats::Save(Configuration& cfg) const
		{
			dialog->Save( cfg );
		}

		void Cheats::Load() const
		{
			if (game && paths.AutoLoadCheatsEnabled())
			{
				const Path path( paths.GetCheatPath( emulator.GetImagePath() ) );

				if (dialog->Load( path ))
					Io::Log() << "Cheats: loaded cheats from \"" << path << "\"\r\n";
			}

			Update();
		}

		void Cheats::Update() const
		{
			Nes::Cheats cheats( emulator );
			cheats.ClearCodes();

			if (game)
			{
				for (uint crc = Nes::Cartridge(emulator).GetProfile() ? Nes::Cartridge(emulator).GetProfile()->hash.GetCrc32() : 0, i = 0; i < 2; ++i)
				{
					const Window::Cheats::Codes& codes = dialog->GetCodes( i ? Window::Cheats::PERMANENT_CODES : Window::Cheats::TEMPORARY_CODES );

					for (Window::Cheats::Codes::const_iterator it(codes.begin()), end(codes.end()); it != end; ++it)
					{
						if (it->enabled && (it->crc == 0 || it->crc == crc))
							cheats.SetCode( it->ToNesCode() );
					}
				}
			}
		}

		void Cheats::Flush() const
		{
			Nes::Cheats cheats( emulator );
			cheats.ClearCodes();

			if (game && paths.AutoSaveCheatsEnabled())
			{
				const Path path( paths.GetCheatPath( emulator.GetImagePath() ) );

				if (dialog->Save( path ))
					Io::Log() << "Cheats: saved cheats to \"" << path << "\"\r\n";
			}

			dialog->Flush();
		}

		void Cheats::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_LOAD:

					game = emulator.IsGame();
					Load();
					break;

				case Emulator::EVENT_UNLOAD:

					Flush();
					game = false;
					break;

				case Emulator::EVENT_NETPLAY_MODE:

					menu[IDM_OPTIONS_CHEATS].Enable( !data );
					break;
			}
		}

		void Cheats::OnCmdOptions(uint)
		{
			dialog->Open();
			Update();
		}
	}
}
