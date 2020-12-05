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

#include "NstManager.hpp"
#include "NstIoScreen.hpp"
#include "NstResourceString.hpp"
#include "NstManagerTapeRecorder.hpp"
#include "NstDialogTapeRecorder.hpp"
#include "../core/api/NstApiTapeRecorder.hpp"

namespace Nestopia
{
	namespace Managers
	{
		TapeRecorder::TapeRecorder(Emulator& e,const Configuration& cfg,Window::Menu& m,const Paths& paths)
		:
		Manager ( e, m, this, &TapeRecorder::OnEmuEvent ),
		dialog  ( new Window::TapeRecorder(cfg,paths) )
		{
			static const Window::Menu::CmdHandler::Entry<TapeRecorder> commands[] =
			{
				{ IDM_MACHINE_EXT_TAPE_FILE,   &TapeRecorder::OnCmdFile   },
				{ IDM_MACHINE_EXT_TAPE_RECORD, &TapeRecorder::OnCmdRecord },
				{ IDM_MACHINE_EXT_TAPE_PLAY,   &TapeRecorder::OnCmdPlay   },
				{ IDM_MACHINE_EXT_TAPE_STOP,   &TapeRecorder::OnCmdStop   }
			};

			menu.Commands().Add( this, commands );

			static const Window::Menu::PopupHandler::Entry<TapeRecorder> popups[] =
			{
				{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_EXT>::ID,                          &TapeRecorder::OnMenuExt     },
				{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_EXT,IDM_POS_MACHINE_EXT_TAPE>::ID, &TapeRecorder::OnMenuExtTape }
			};

			menu.Popups().Add( this, popups );
		}

		TapeRecorder::~TapeRecorder()
		{
		}

		void TapeRecorder::Save(Configuration& cfg) const
		{
			dialog->Save( cfg );
		}

		const Path TapeRecorder::GetFile(Path imagePath) const
		{
			if (dialog->UseImageNaming())
			{
				imagePath.Extension() = L"tp";
				return imagePath;
			}
			else
			{
				return dialog->GetCustomFile();
			}
		}

		bool TapeRecorder::Available() const
		{
			return
			(
				!emulator.NetPlayers() &&
				(!emulator.IsImage() || Nes::TapeRecorder(emulator).IsConnected())
			);
		}

		bool TapeRecorder::CanSetFile() const
		{
			return
			(
				!emulator.IsImage() &&
				!emulator.NetPlayers()
			);
		}

		bool TapeRecorder::CanPlay() const
		{
			return
			(
				emulator.IsOn() &&
				!emulator.NetPlayers() &&
				!emulator.IsLocked() &&
				Nes::TapeRecorder(emulator).IsPlayable() &&
				Nes::TapeRecorder(emulator).IsStopped()
			);
		}

		bool TapeRecorder::CanRecord() const
		{
			return
			(
				emulator.IsOn() &&
				!emulator.NetPlayers() &&
				!emulator.IsLocked() &&
				Nes::TapeRecorder(emulator).IsConnected() &&
				Nes::TapeRecorder(emulator).IsStopped()
			);
		}

		bool TapeRecorder::CanStop() const
		{
			return
			(
				emulator.IsOn() &&
				!emulator.NetPlayers() &&
				!emulator.IsLocked() &&
				!Nes::TapeRecorder(emulator).IsStopped()
			);
		}

		void TapeRecorder::OnMenuExt(const Window::Menu::PopupHandler::Param& param)
		{
			param.menu[ IDM_POS_MACHINE_EXT_TAPE ].Enable( !param.show || Available() );
		}

		void TapeRecorder::OnMenuExtTape(const Window::Menu::PopupHandler::Param& param)
		{
			param.menu[ IDM_MACHINE_EXT_TAPE_FILE   ].Enable( !param.show || CanSetFile() );
			param.menu[ IDM_MACHINE_EXT_TAPE_STOP   ].Enable( !param.show || CanStop()    );
			param.menu[ IDM_MACHINE_EXT_TAPE_PLAY   ].Enable( !param.show || CanPlay()    );
			param.menu[ IDM_MACHINE_EXT_TAPE_RECORD ].Enable( !param.show || CanRecord()  );
		}

		void TapeRecorder::OnCmdFile(uint)
		{
			if (CanSetFile())
				dialog->Open();
		}

		void TapeRecorder::OnCmdRecord(uint)
		{
			if (CanRecord())
			{
				Nes::TapeRecorder(emulator).Record();
				Resume();
			}
		}

		void TapeRecorder::OnCmdPlay(uint)
		{
			if (CanPlay())
			{
				Nes::TapeRecorder(emulator).Play();
				Resume();
			}
		}

		void TapeRecorder::OnCmdStop(uint)
		{
			if (CanStop())
			{
				Nes::TapeRecorder(emulator).Stop();
				Resume();
			}
		}

		void TapeRecorder::OnEmuEvent(const Emulator::Event event,Emulator::Data)
		{
			switch (event)
			{
				case Emulator::EVENT_TAPE_PLAYING:
				case Emulator::EVENT_TAPE_RECORDING:
				case Emulator::EVENT_TAPE_STOPPED:

					Io::Screen() << Resource::String
					(
						event == Emulator::EVENT_TAPE_PLAYING   ? IDS_SCREEN_TAPE_PLAYING :
						event == Emulator::EVENT_TAPE_RECORDING ? IDS_SCREEN_TAPE_RECORDING :
                                                                  IDS_SCREEN_TAPE_STOPPED
					);
					break;
			}
		}
	}
}
