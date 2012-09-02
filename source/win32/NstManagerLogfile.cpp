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

#include "NstIoLog.hpp"
#include "NstDialogLogfile.hpp"
#include "NstManager.hpp"
#include "NstManagerPreferences.hpp"
#include "NstManagerLogfile.hpp"
#include "../core/api/NstApiUser.hpp"

namespace Nestopia
{
	namespace Managers
	{
		struct Logfile::Callbacks
		{
			static void NST_CALLBACK DoOutput(Nes::User::UserData data,wcstring text,uint length)
			{
				NST_ASSERT( data && text );

				if (length)
				{
				#ifdef NST_DEBUG
					::OutputDebugString( text );
				#endif

					Logfile& log = *static_cast<Logfile*>(data);

					if (log.preferences[Preferences::SAVE_LOGFILE])
					{

						try
						{
							if (log.file.IsOpen())
							{
								if (log.file.Size() > MAX_SIZE)
									log.file.Truncate( log.msgOffset );
							}
							else
							{
								log.Open();
							}

							log.file.Write( text, length * sizeof(wchar_t) );
						}
						catch (Io::File::Exception)
						{
							log.Close();
						}
					}
				}
			}

			static void NST_CALLBACK DoCharOutput(Nes::User::UserData data,cstring text,ulong length)
			{
				NST_ASSERT( data && text );

				Logfile& log = *static_cast<Logfile*>(data);

				if (length && log.preferences[Preferences::SAVE_LOGFILE])
				{
					const HeapString string( text, length );
					DoOutput( data, string.Ptr(), string.Length() );
				}
			}
		};

		Logfile::Logfile(Emulator& e,Window::Menu& m,const Preferences& p)
		:
		Manager     ( e, m, this, &Logfile::OnEmuEvent, IDM_VIEW_LOGFILE, &Logfile::OnCommand ),
		preferences ( p )
		{
			Io::Log::SetCallback( this, Callbacks::DoOutput );
			Nes::User::logCallback.Set( &Callbacks::DoCharOutput, this );

			UpdateMenu();
		}

		Logfile::~Logfile()
		{
			Io::Log::UnsetCallback();
			Nes::User::logCallback.Unset();
		}

		bool Logfile::Available() const
		{
			return emulator.NetPlayers() == 0 && file.IsOpen();
		}

		void Logfile::UpdateMenu() const
		{
			menu[IDM_VIEW_LOGFILE].Enable( Available() );
		}

		void Logfile::OnEmuEvent(const Emulator::Event event,Emulator::Data)
		{
			switch (event)
			{
				case Emulator::EVENT_NETPLAY_MODE:

					UpdateMenu();
					break;
			}
		}

		void Logfile::OnCommand(uint)
		{
			if (file.IsOpen())
			{
				try
				{
					file.Rewind();

					HeapString string;
					file.ReadText( string );

					if (Window::Logfile().Open( string.Ptr() ))
						file.Truncate( msgOffset );
				}
				catch (Io::File::Exception)
				{
					Close();
				}
			}
		}

		void Logfile::Open()
		{
			file.Open
			(
				Application::Instance::GetExePath(L"nestopia.log"),
				Io::File::READ|Io::File::WRITE|Io::File::EMPTY|Io::File::SEQUENTIAL_ACCESS
			);

			HeapString text;

			text << "Nestopia log file version "
                 << Application::Instance::GetVersion()
                 << "\r\n-----------------------------------\r\n\r\n";

			file.WriteText( text.Ptr(), text.Length(), true );
			msgOffset = file.Position();

			UpdateMenu();
		}

		void Logfile::Close()
		{
			Io::Log::UnsetCallback();
			Nes::User::logCallback.Unset();

			file.Close();

			UpdateMenu();
		}
	}
}
