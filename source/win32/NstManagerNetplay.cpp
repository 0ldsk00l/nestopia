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
#include "NstIoScreen.hpp"
#include "NstSystemDll.hpp"
#include "NstResourceString.hpp"
#include "NstWindowParam.hpp"
#include "NstSystemThread.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogNetplay.hpp"
#include "NstManagerNetplay.hpp"
#include "../kaillera/kailleraclient.h"
#include "../core/api/NstApiMachine.hpp"
#include "../core/api/NstApiFds.hpp"
#include <Shlwapi.h>

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED WM_NULL
#endif

namespace Nestopia
{
	namespace Managers
	{
		class Netplay::Dll : System::Dll
		{
			typedef int (WINAPI *GetVersionFunc)(char*);
			typedef int (WINAPI *InitFunc)();
			typedef int (WINAPI *ShutdownFunc)();
			typedef int (WINAPI *SetInfosFunc)(kailleraInfos*);
			typedef int (WINAPI *SelectServerDialogFunc)(HWND);
			typedef int (WINAPI *ModifyPlayValuesFunc)(void*,int);
			typedef int (WINAPI *ChatSendFunc)(char*);
			typedef int (WINAPI *EndGameFunc)();

			InitFunc const Init;
			ShutdownFunc const Shutdown;

		public:

			using System::Dll::operator !;

			GetVersionFunc         const GetVersion;
			SetInfosFunc           const SetInfos;
			SelectServerDialogFunc const SelectServerDialog;
			ModifyPlayValuesFunc   const ModifyPlayValues;
			ChatSendFunc           const ChatSend;
			EndGameFunc            const EndGame;

			Dll()
			:
			System::Dll        (L"kailleraclient.dll"),
			Init               (Fetch< InitFunc               >( "_kailleraInit@0"               )),
			Shutdown           (Fetch< ShutdownFunc           >( "_kailleraShutdown@0"           )),
			GetVersion         (Fetch< GetVersionFunc         >( "_kailleraGetVersion@4"         )),
			SetInfos           (Fetch< SetInfosFunc           >( "_kailleraSetInfos@4"           )),
			SelectServerDialog (Fetch< SelectServerDialogFunc >( "_kailleraSelectServerDialog@4" )),
			ModifyPlayValues   (Fetch< ModifyPlayValuesFunc   >( "_kailleraModifyPlayValues@8"   )),
			ChatSend           (Fetch< ChatSendFunc           >( "_kailleraChatSend@4"           )),
			EndGame            (Fetch< EndGameFunc            >( "_kailleraEndGame@0"            ))
			{
				if
				(
					GetVersion &&
					Init &&
					Shutdown &&
					SetInfos &&
					SelectServerDialog &&
					ModifyPlayValues &&
					ChatSend &&
					EndGame
				)
					Init();
				else
					Unload();
			}

			~Dll()
			{
				if (*this)
					Shutdown();
			}
		};

		class Netplay::Kaillera : Manager
		{
		public:

			Kaillera(Emulator&,Window::Menu&,const Paths&,Window::Custom&,bool);
			~Kaillera();

			enum Exception
			{
				ERR_LOAD
			};

			void ToggleConnection();
			void Chat();
			bool Close() const;

		private:

			enum
			{
				MAX_PLAYERS         = 8,
				MASTER              = 1,
				WM_NST_OPEN_CLIENT  = WM_APP + 57,
				WM_NST_CLOSE_CLIENT = WM_APP + 58,
				WM_NST_START_GAME   = WM_APP + 59
			};

			class Command
			{
				enum
				{
					PACKET_TYPE                 = 0x0F,
					PACKET_DATA                 = 0xF0,
					PACKET_DATA_REGION_PAL      = 0x01,
					PACKET_DATA_ADAPTER_FAMICOM = 0x02,
					PACKET_DATA_SHIFT           = 4,
					PACKET_STARTUP              = 1,
					PACKET_RESET                = 2,
					PACKET_INSERT_DISK          = 3,
					PACKET_EJECT_DISK           = 4,
					PACKET_INSERT_COIN          = 5
				};

				uint command;

				struct
				{
					Nes::Input::UserData data;
					Nes::Input::Controllers::VsSystem::PollCallback code;
				}   coinCallback;

				struct
				{
					bool regionPal;
					bool adapterFamicom;
					bool unlimSprites;
				}   settings;

			public:

				void Begin()
				{
					if (Nes::Machine(instance->emulator).Is(Nes::Machine::VS))
					{
						Nes::Input::Controllers::VsSystem::callback.Get( coinCallback.code, coinCallback.data );
						Nes::Input::Controllers::VsSystem::callback.Unset();
					}
					else
					{
						coinCallback.code = NULL;
						coinCallback.data = NULL;
					}

					settings.regionPal = (Nes::Machine(instance->emulator).GetMode() == Nes::Machine::PAL);
					settings.adapterFamicom = (Nes::Input(instance->emulator).GetConnectedAdapter() == Nes::Input::ADAPTER_FAMICOM);
					settings.unlimSprites = Nes::Video(instance->emulator).AreUnlimSpritesEnabled();

					Nes::Video(instance->emulator).EnableUnlimSprites( false );

					if (instance->network.player == MASTER)
					{
						command = PACKET_STARTUP;

						if (settings.regionPal)
							command |= uint(PACKET_DATA_REGION_PAL) << PACKET_DATA_SHIFT;

						if (settings.adapterFamicom)
							command |= uint(PACKET_DATA_ADAPTER_FAMICOM) << PACKET_DATA_SHIFT;
					}
					else
					{
						command = 0;
					}
				}

				void Send(Emulator::Command input,uint state)
				{
					NST_COMPILE_ASSERT( Emulator::NUM_COMMANDS == 3 );
					NST_VERIFY( command == 0 );

					if (command == 0 && instance->network.player == MASTER)
					{
						switch (input)
						{
							case Emulator::COMMAND_RESET:

								command = PACKET_RESET | (state & 0x1) << PACKET_DATA_SHIFT;
								break;

							case Emulator::COMMAND_DISK_INSERT:

								command = PACKET_INSERT_DISK | state << PACKET_DATA_SHIFT;
								break;

							case Emulator::COMMAND_DISK_EJECT:

								command = PACKET_EJECT_DISK;
								break;
						}
					}
				}

				NST_FORCE_INLINE uint GetCode()
				{
					if (instance->network.player != MASTER)
						return 0;

					if (coinCallback.code)
					{
						Nes::Input::Controllers::VsSystem vs;
						coinCallback.code( coinCallback.data, vs );

						if (vs.insertCoin & (Nes::Input::Controllers::VsSystem::COIN_1|Nes::Input::Controllers::VsSystem::COIN_2))
						{
							return PACKET_INSERT_COIN |
							(
								((vs.insertCoin & Nes::Input::Controllers::VsSystem::COIN_1) ? (0x1U << PACKET_DATA_SHIFT) : 0) |
								((vs.insertCoin & Nes::Input::Controllers::VsSystem::COIN_2) ? (0x2U << PACKET_DATA_SHIFT) : 0)
							);
						}
					}

					const uint code = command;
					command = 0;
					return code;
				}

				NST_FORCE_INLINE bool Dispatch(const uint packet,Nes::Input::Controllers& controllers)
				{
					if (packet)
					{
						const uint data = packet >> PACKET_DATA_SHIFT;

						switch (packet & PACKET_TYPE)
						{
							case PACKET_INSERT_COIN:

								controllers.vsSystem.insertCoin =
								(
									((data & 0x1) ? Nes::Input::Controllers::VsSystem::COIN_1 : 0U) |
									((data & 0x2) ? Nes::Input::Controllers::VsSystem::COIN_2 : 0U)
								);
								break;

							case PACKET_RESET:

								Nes::Machine(instance->emulator).Reset( data & 0x1 );
								break;

							case PACKET_INSERT_DISK:

								Nes::Fds(instance->emulator).InsertDisk( data / 2, data % 2 );
								break;

							case PACKET_EJECT_DISK:

								Nes::Fds(instance->emulator).EjectDisk();
								break;

							case PACKET_STARTUP:

								Nes::Machine(instance->emulator).SetMode( (data & PACKET_DATA_REGION_PAL) ? Nes::Machine::PAL : Nes::Machine::NTSC );
								Nes::Input(instance->emulator).ConnectAdapter( (data & PACKET_DATA_ADAPTER_FAMICOM) ? Nes::Input::ADAPTER_FAMICOM : Nes::Input::ADAPTER_NES );
								break;

							default:

								NST_DEBUG_MSG("unknown netplay package");
								break;
						}

						return true;
					}

					return false;
				}

				void End()
				{
					if (coinCallback.code)
					{
						Nes::Input::Controllers::VsSystem::callback.Set( coinCallback.code, coinCallback.data );
						coinCallback.code = NULL;
						coinCallback.data = NULL;
					}

					Nes::Machine(instance->emulator).SetMode( settings.regionPal ? Nes::Machine::PAL : Nes::Machine::NTSC );
					Nes::Input(instance->emulator).ConnectAdapter( settings.adapterFamicom ? Nes::Input::ADAPTER_FAMICOM : Nes::Input::ADAPTER_NES );
					Nes::Video(instance->emulator).EnableUnlimSprites( settings.unlimSprites );
				}
			};

			class Input
			{
				struct
				{
					Nes::Input::UserData data;
					Nes::Input::Controllers::Pad::PollCallback code;
				}   pollCallback;

			public:

				void Capture()
				{
					Nes::Input::Controllers::Pad::callback.Get( pollCallback.code, pollCallback.data );
					Nes::Input::Controllers::Pad::callback.Unset();

					NST_ASSERT( pollCallback.code );
				}

				NST_FORCE_INLINE uint GetCode() const
				{
					uint index = instance->network.player - 1U;

					if (index < 4)
					{
						index = Nes::Input(instance->emulator).GetConnectedController(index) - uint(Nes::Input::PAD1);
						NST_VERIFY( index < 4 );

						if (index < 4)
						{
							Nes::Input::Controllers::Pad pad;
							pollCallback.code( pollCallback.data, pad, index );
							return pad.buttons;
						}
					}

					return 0;
				}

				NST_FORCE_INLINE void Dispatch(uint port,uint packet,Nes::Input::Controllers& controllers) const
				{
					NST_ASSERT( port < 4 );

					uint index = Nes::Input(instance->emulator).GetConnectedController(port) - uint(Nes::Input::PAD1);
					NST_VERIFY( index < 4 );

					if (index < 4)
						controllers.pad[index].buttons = packet;
				}

				void Release() const
				{
					Nes::Input::Controllers::Pad::callback.Set( pollCallback.code, pollCallback.data );
				}
			};

			friend class Command;
			friend class Input;

			struct Callbacks;
			class Client;

			void Disconnect();
			void StartNetwork(System::Thread::Terminator);

			ibool OnOpenClient  (Window::Param&);
			ibool OnCloseClient (Window::Param&);
			ibool OnStartGame   (Window::Param&);
			ibool OnEnable      (Window::Param&);

			void OnEmuFrame   (Nes::Input::Controllers&);
			void OnEmuCommand (Emulator::Command,Emulator::Data);
			void OnEmuEvent   (Emulator::Event,Emulator::Data);

			const Dll dll;
			Window::Custom& window;
			Window::Netplay::Chat chat;
			Window::Netplay dialog;
			System::Thread thread;
			Window::MsgHandler::Callback enableCallback;

			struct
			{
				bool connected;
				Command command;
				Input input;
				uint player;
				uint players;
				Path game;
			}   network;

			static Kaillera* instance;

		public:

			bool ShouldGoFullscreen() const
			{
				return dialog.ShouldGoFullscreen();
			}

			void SaveFile() const
			{
				dialog.SaveFile();
			}
		};

		struct Netplay::Kaillera::Callbacks
		{
			static int WINAPI Start(char* game,int player,int players)
			{
				if (game && *game && player > 0 && players > 0 && player-1U < players)
				{
					instance->network.game = game;
					instance->network.player = player;
					instance->network.players = players;
					instance->window.Post( WM_NST_START_GAME );
				}
				else
				{
					instance->dll.EndGame();
					NST_DEBUG_MSG("Kaillera::Start() failed!");
				}

				return 0;
			}

			static void WINAPI ClientDrop(char* nick,int playerNum)
			{
				static const HeapString player( HeapString() << Resource::String(IDS_TEXT_PLAYER) << ' ' );
				static const HeapString droppedOut( HeapString() << ") " << Resource::String(IDS_TEXT_DROPPEDOUT) );

				if (nick && *nick)
					Io::Screen() << player << playerNum << " (" << HeapString().Import(nick) << droppedOut;
			}

			static void WINAPI ChatRecieve(char* nick,char* text)
			{
				static const HeapString says( HeapString() << ' ' << Resource::String(IDS_TEXT_SAYS) << ": " );

				if (nick && *nick && text && *text)
					Io::Screen() << HeapString().Import(nick) << says << HeapString().Import(text);
			}
		};

		class Netplay::Kaillera::Client
		{
			// Uses a hook for monitoring the Kaillera windows activity.
			// The bug seems to be located in the Kaillera code so I have
			// to resolve to some dirty hacks to prevent the message queue
			// from entering an infinite loop. This will happen if the user
			// closes the main server list window while others are open.

			struct Instance
			{
				HHOOK hHook;
				DWORD threadId;

				Instance()
				: hHook(NULL) {}
			};

			DWORD visualStyles;

			static Instance instance;

			class Callbacks
			{
				static NST_NO_INLINE bool IsKaillera(const Window::Generic window)
				{
					HeapString name;
					window.Text() >> name;

					return
					(
						(name.Length() >= 8 && name(0,8) == L"Kaillera" ) ||
						(name.Length() >= 6 && name(0,6) == L"Anti3D"   )
					);
				}

			public:

				static BOOL CALLBACK Destroy(HWND hWnd,LPARAM)
				{
					if (IsKaillera( hWnd ))
						::SendMessage( hWnd, WM_SYSCOMMAND, SC_CLOSE, 0 );

					return true;
				}

				static BOOL CALLBACK Find(HWND hWnd,LPARAM lParam)
				{
					if (::GetParent( hWnd ) && IsKaillera( hWnd ))
					{
						*reinterpret_cast<HWND*>(lParam) = hWnd;
						return false;
					}

					return true;
				}

				static BOOL CALLBACK Show(HWND hWnd,LPARAM lParam)
				{
					if (IsKaillera( hWnd ))
						::ShowWindow( hWnd, lParam );

					return true;
				}
			};

			template<typename T>
			static void Enumerate(BOOL (CALLBACK* callback)(HWND,LPARAM),T t)
			{
				::EnumThreadWindows( instance.threadId, callback, LPARAM(t) );
			}

			static bool IsZombie(HWND hWnd)
			{
				if (!::GetParent( hWnd ))
				{
					hWnd = NULL;
					Enumerate( Callbacks::Find, &hWnd );

					if (hWnd)
						return true;
				}

				return false;
			}

			static LRESULT CALLBACK MessageSpy(int iCode,WPARAM wParam,LPARAM lParam)
			{
				if (iCode == HC_ACTION)
				{
					MSG& msg = *reinterpret_cast<MSG*>(lParam);

					if (msg.message == WM_CLOSE && IsZombie( msg.hwnd ))
						msg.message = WM_NULL;
				}

				return ::CallNextHookEx( instance.hHook, iCode, wParam, lParam );
			}

			void DisableVisualStyles()
			{
				// Kaillera doesn't like XP Visual Styles

				struct ComCtl32
				{
					static bool IsVersion6()
					{
						const System::Dll comctl32( L"comctl32.dll" );

						if (DLLGETVERSIONPROC const getVersion = comctl32.Fetch<DLLGETVERSIONPROC>("DllGetVersion"))
						{
							DLLVERSIONINFO info;
							info.cbSize = sizeof(info);

							return getVersion( &info ) == NOERROR && info.dwMajorVersion >= 6;
						}

						return false;
					}
				};

				visualStyles = 0;

				static const bool isVersion6 = ComCtl32::IsVersion6();

				if (isVersion6)
				{
					const System::Dll uxtheme( L"uxtheme.dll" );

					typedef DWORD (STDAPICALLTYPE* GetProperty)();
					typedef void (STDAPICALLTYPE* SetProperty)(DWORD);

					if (GetProperty const getProperty = uxtheme.Fetch<GetProperty>("GetThemeAppProperties"))
					{
						visualStyles = getProperty();

						if (visualStyles)
						{
							if (SetProperty const setProperty = uxtheme.Fetch<SetProperty>("SetThemeAppProperties"))
							{
								setProperty( 0 );
								Application::Instance::GetMainWindow().Post( WM_THEMECHANGED );
							}
						}
					}
				}
			}

			void RestoreVisualStyles()
			{
				if (visualStyles)
				{
					const System::Dll uxtheme( L"uxtheme.dll" );

					typedef void (STDAPICALLTYPE* SetProperty)(DWORD);

					if (SetProperty const setProperty = uxtheme.Fetch<SetProperty>("SetThemeAppProperties"))
					{
						setProperty( visualStyles );
						visualStyles = 0;
						Application::Instance::GetMainWindow().Post( WM_THEMECHANGED );
					}
				}
			}

		public:

			NST_NO_INLINE Client()
			{
				NST_ASSERT( !instance.hHook );

				instance.threadId = ::GetCurrentThreadId();

				instance.hHook = ::SetWindowsHookEx
				(
					WH_GETMESSAGE,
					MessageSpy,
					::GetModuleHandle(NULL),
					instance.threadId
				);

				if (!instance.hHook)
					throw "SetWindowsHookEx() failed!";

				Kaillera::instance->window.Post( WM_NST_OPEN_CLIENT );

				DisableVisualStyles();
			}

			static void Run()
			{
				Kaillera::instance->dll.SelectServerDialog( NULL );
			}

			static void Show()
			{
				if (instance.hHook)
					Enumerate( Callbacks::Show, SW_SHOW );
			}

			static void Hide()
			{
				if (instance.hHook)
					Enumerate( Callbacks::Show, SW_HIDE );
			}

			static void Close()
			{
				if (instance.hHook)
					Enumerate( Callbacks::Destroy, 0 );
			}

			NST_NO_INLINE ~Client()
			{
				NST_ASSERT( instance.hHook );

				::UnhookWindowsHookEx( instance.hHook );
				instance.hHook = NULL;

				RestoreVisualStyles();

				Kaillera::instance->window.Post( WM_NST_CLOSE_CLIENT );
			}
		};

		Netplay::Kaillera* Netplay::Kaillera::instance = NULL;
		Netplay::Kaillera::Client::Instance Netplay::Kaillera::Client::instance;

		Netplay::Kaillera::Kaillera
		(
			Emulator& e,
			Window::Menu& m,
			const Paths& paths,
			Window::Custom& w,
			const bool doFullscreen
		)
		:
		Manager ( e, m, this, &Kaillera::OnEmuEvent ),
		window  ( w ),
		chat    ( dll.ChatSend ),
		dialog  ( e, paths, doFullscreen )
		{
			if (!dll)
				throw ERR_LOAD;

			NST_ASSERT( instance == NULL );

			instance = this;
		}

		Netplay::Kaillera::~Kaillera()
		{
			instance = NULL;
		}

		void Netplay::Kaillera::StartNetwork(System::Thread::Terminator)
		{
			Client().Run();
		}

		ibool Netplay::Kaillera::OnOpenClient(Window::Param&)
		{
			network.input.Capture();
			emulator.BeginNetplayMode();
			return true;
		}

		ibool Netplay::Kaillera::OnCloseClient(Window::Param&)
		{
			network.input.Release();
			window.Messages().Remove( this );
			emulator.EndNetplayMode();
			return true;
		}

		bool Netplay::Kaillera::Close() const
		{
			if (emulator.NetPlayers())
			{
				if (emulator.IsImage())
					emulator.Unload();
				else
					Client::Close();

				return false;
			}

			return true;
		}

		ibool Netplay::Kaillera::OnStartGame(Window::Param&)
		{
			emulator.StartNetplay
			(
				this,
				&Kaillera::OnEmuFrame,
				&Kaillera::OnEmuCommand,
				network.player,
				network.players
			);

			Application::Instance::GetMainWindow().Send
			(
				Application::Instance::WM_NST_LAUNCH,
				Paths::File::GAME|Paths::File::ARCHIVE,
				dialog.GetPath(network.game.Ptr())
			);

			if (!emulator.IsOn())
			{
				emulator.StopNetplay();
				dll.EndGame();
			}

			return true;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		void Netplay::Kaillera::OnEmuFrame(Nes::Input::Controllers& controllers)
		{
			controllers.vsSystem.insertCoin = 0;

			if (network.connected)
			{
				uchar packets[MAX_PLAYERS][2] = {{0},{0}};

				packets[0][0] = network.input.GetCode();
				packets[0][1] = network.command.GetCode();

				if (dll.ModifyPlayValues( packets, 2 ) != -1)
				{
					network.command.Dispatch( packets[0][1], controllers );

					for (uint i=0, n=NST_MIN(4,network.players); i < n; ++i)
						network.input.Dispatch( i, packets[i][0], controllers );

					return;
				}
			}

			network.connected = false;
			window.PostCommand( IDM_NETPLAY_CONNECTION );
		}

		void Netplay::Kaillera::OnEmuCommand(Emulator::Command command,Emulator::Data data)
		{
			if (network.connected)
				network.command.Send( command, data );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Netplay::Kaillera::Chat()
		{
			chat.Open();
		}

		void Netplay::Kaillera::ToggleConnection()
		{
			if (emulator.NetPlayers())
			{
				Close();
			}
			else if (!emulator.IsImage() && dialog.Open())
			{
				static const Window::MsgHandler::Entry<Kaillera> messages[] =
				{
					{ WM_NST_OPEN_CLIENT,  &Kaillera::OnOpenClient  },
					{ WM_NST_CLOSE_CLIENT, &Kaillera::OnCloseClient },
					{ WM_NST_START_GAME,   &Kaillera::OnStartGame   }
				};

				window.Messages().Add( this, messages );

				NST_ASSERT( dialog.GetGamePaths().size() );

				String::Heap<char> strings;

				for (Window::Netplay::GamePaths::const_iterator it(dialog.GetGamePaths().begin()), end(dialog.GetGamePaths().end()); it != end; ++it)
					strings << it->Target().File() << '\0';

				String::Heap<char> name;
				name << "Nestopia " << Application::Instance::GetVersion();

				kailleraInfos info;

				info.appName               = name.Ptr();
				info.gameList              = strings.Ptr();
				info.gameCallback          = Callbacks::Start;
				info.chatReceivedCallback  = Callbacks::ChatRecieve;
				info.clientDroppedCallback = Callbacks::ClientDrop;
				info.moreInfosCallback     = NULL;

				dll.SetInfos( &info );

				thread.Start( System::Thread::Callback(this,&Kaillera::StartNetwork) );
			}
		}

		ibool Netplay::Kaillera::OnEnable(Window::Param& param)
		{
			if (!param.wParam)
				window.Send( WM_ENABLE, true, 0 );

			return true;
		}

		void Netplay::Kaillera::OnEmuEvent(const Emulator::Event event,Emulator::Data)
		{
			if (emulator.NetPlayers())
			{
				static Window::MsgHandler::Callback old;

				switch (event)
				{
					case Emulator::EVENT_POWER_ON:

						if (emulator.IsGame())
						{
							Client::Hide();

							network.connected = true;
							network.command.Begin();

							menu[IDM_NETPLAY_CHAT].Enable();

							if (dialog.ShouldGoFullscreen())
								window.SendCommand( IDM_VIEW_SWITCH_SCREEN );

							enableCallback = window.Messages()[WM_ENABLE].Replace( this, &Kaillera::OnEnable );
						}
						break;

					case Emulator::EVENT_POWER_OFF:

						if (emulator.IsGame())
						{
							network.connected = false;
							network.command.End();

							menu[IDM_NETPLAY_CHAT].Disable();

							chat.Close();

							if (dialog.ShouldGoFullscreen())
								window.SendCommand( IDM_VIEW_SWITCH_SCREEN );

							window.Messages()[WM_ENABLE] = enableCallback;

							Client::Show();
							dll.EndGame();
						}
						break;

					case Emulator::EVENT_UNLOAD:

						emulator.StopNetplay();
						break;
				}
			}
		}

		Netplay::Netplay
		(
			Emulator& e,
			const Configuration& cfg,
			Window::Menu& m,
			const Paths& p,
			Window::Custom& w
		)
		:
		Manager      ( e, m, this, &Netplay::OnEmuEvent ),
		kaillera     ( NULL ),
		window       ( w ),
		paths        ( p ),
		fullscreen   ( false ),
		doFullscreen ( cfg["netplay"]["fullscreen"].Yes() )
		{
			menu[IDM_NETPLAY_CONNECTION].Text() << Resource::String( IDS_MENU_NETPLAY_CONNECT );
			menu[IDM_NETPLAY_CHAT].Disable();

			const Dll dll;

			Io::Log log;

			if (!dll)
			{
				log << "Kaillera: file \"kailleraclient.dll\" not found or initialization failed. "
                       "netplay will be disabled!\r\n";

				menu[IDM_NETPLAY_CONNECTION].Disable();
			}
			else
			{
				Application::Instance::Events::Add( this, &Netplay::OnAppEvent );

				static const Window::Menu::CmdHandler::Entry<Netplay> commands[] =
				{
					{ IDM_NETPLAY_CONNECTION, &Netplay::OnCmdConnection },
					{ IDM_NETPLAY_CHAT,       &Netplay::OnCmdChat       }
				};

				menu.Commands().Add( this, commands );

				char version[16];
				version[0] = '\0';

				dll.GetVersion( version );
				version[15] = '\0';

				if (*version)
				{
					log << "Kaillera: found \"kailleraclient.dll\" version " << version << "\r\n";

					if (std::strcmp( version, "0.9" ))
						log << "Kaillera: warning, the DLL file may be incompatible with Nestopia!\r\n";
				}
				else
				{
					log << "Kaillera: warning, unknown version of \"kailleraclient.dll\"!\r\n";
				}

				UpdateMenu();
			}
		}

		Netplay::~Netplay()
		{
			Application::Instance::Events::Remove( this );
			delete kaillera;
		}

		void Netplay::Save(Configuration& cfg,const bool saveGameList) const
		{
			cfg["netplay"]["fullscreen"].YesNo() = (kaillera ? kaillera->ShouldGoFullscreen() : doFullscreen);

			if (kaillera && saveGameList)
				kaillera->SaveFile();
		}

		bool Netplay::Close() const
		{
			return kaillera ? kaillera->Close() : true;
		}

		void Netplay::UpdateMenu() const
		{
			menu[IDM_NETPLAY_CONNECTION].Enable
			(
				emulator.NetPlayers() || (!fullscreen && !emulator.IsImage())
			);
		}

		void Netplay::OnEmuEvent(const Emulator::Event event,const Emulator::Data data)
		{
			switch (event)
			{
				case Emulator::EVENT_NETPLAY_MODE:

					menu[IDM_NETPLAY_CONNECTION].Text() << Resource::String( data ? IDS_MENU_NETPLAY_DISCONNECT : IDS_MENU_NETPLAY_CONNECT );

				case Emulator::EVENT_LOAD:
				case Emulator::EVENT_UNLOAD:

					UpdateMenu();
					break;
			}
		}

		void Netplay::OnAppEvent(Application::Instance::Event event,const void*)
		{
			switch (event)
			{
				case Application::Instance::EVENT_DESKTOP:
				case Application::Instance::EVENT_FULLSCREEN:

					fullscreen = (event == Application::Instance::EVENT_FULLSCREEN);
					UpdateMenu();
					break;
			}
		}

		void Netplay::OnCmdConnection(uint)
		{
			if (kaillera == NULL)
			{
				try
				{
					kaillera = new Kaillera( emulator, menu, paths, window, doFullscreen );
				}
				catch (Kaillera::Exception)
				{
					return;
				}
			}

			kaillera->ToggleConnection();
		}

		void Netplay::OnCmdChat(uint)
		{
			if (kaillera)
				kaillera->Chat();
		}
	}
}
