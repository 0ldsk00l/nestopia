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

#include "NstIoScreen.hpp"
#include "NstResourceString.hpp"
#include "NstResourceClipboard.hpp"
#include "NstSystemKeyboard.hpp"
#include "NstManager.hpp"
#include "NstManagerInput.hpp"

namespace Nestopia
{
	namespace Managers
	{
		Input::Clipboard::Clipboard(Emulator& e,Window::Menu& m)
		:
		Manager (e,m),
		paste   (false)
		{
			static const Window::Menu::CmdHandler::Entry<Clipboard> commands[] =
			{
				{ IDM_MACHINE_EXT_KEYBOARD_PASTE, &Clipboard::OnCmdMachineKeyboardPaste }
			};

			menu.Commands().Add( this, commands );

			static const Window::Menu::PopupHandler::Entry<Clipboard> popups[] =
			{
				{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_EXT,IDM_POS_MACHINE_EXT_KEYBOARD>::ID, &Clipboard::OnMenuKeyboard }
			};

			menu.Popups().Add( this, popups );

			HeapString name;
			menu[IDM_MACHINE_EXT_KEYBOARD_PASTE].Text() >> name;
			menu[IDM_MACHINE_EXT_KEYBOARD_PASTE].Text() << (name << '\t' << System::Keyboard::GetName( VK_F12 ));
		}

		void Input::Clipboard::Clear()
		{
			paste = false;
			buffer.Destroy();
		}

		bool Input::Clipboard::CanPaste() const
		{
			return emulator.IsGameOn() && buffer.Empty() && Resource::Clipboard::Available() &&
			(
				Nes::Input(emulator).IsControllerConnected( Nes::Input::FAMILYKEYBOARD ) ||
				Nes::Input(emulator).IsControllerConnected( Nes::Input::SUBORKEYBOARD  )
			);
		}

		uint Input::Clipboard::Query(const uchar* const NST_RESTRICT keyboard,const Type type)
		{
			const bool prev = paste;
			paste = false;

			if (buffer.Empty())
			{
				if (prev || (keyboard[DIK_F12] & 0x80U))
				{
					Resource::Clipboard resource;

					if (wcstring string = resource)
					{
						pos = 0;
						releasing = 0;
						hold = 0;
						shifted = false;

						bool kana = false;

						for (wchar_t p; '\0' != (p = *string); ++string)
						{
							bool mode = false;

							if (p < 32)
							{
								if (p != '\r')
									continue;
							}
							else if (p >= 'A' && p <= 'Z')
							{
								p = p - 'A' + 'a';
							}
							else if (p > 122)
							{
								if (p == 165) // ''
								{
									p = '\\';
								}
								else if (type == SUBOR)
								{
									if (p > 125)
										continue;
								}
								else if (p > 0xFF00)
								{
									switch (p)
									{
										case 0xFF71: p = '1';  break;
										case 0xFF72: p = '2';  break;
										case 0xFF73: p = '3';  break;
										case 0xFF74: p = '4';  break;
										case 0xFF75: p = '5';  break;

										case 0xFF67: p = '!';  break;
										case 0xFF68: p = '\"'; break;
										case 0xFF69: p = '#';  break;
										case 0xFF6A: p = '$';  break;
										case 0xFF6B: p = '%';  break;

										case 0xFF6F: p = 'c' | 0x80; break;

										case 0xFF76: p = 'q';  break;
										case 0xFF77: p = 'w';  break;
										case 0xFF78: p = 'e';  break;
										case 0xFF79: p = 'r';  break;
										case 0xFF7A: p = 't';  break;

										case 0xFF9E:

											if (buffer.Length())
												buffer.Back() |= 0x100;

											continue;

										case 0xFF9F:

											if (buffer.Length())
												buffer.Back() |= 0x80;

											continue;

										case 0xFF7B: p = 'a';  break;
										case 0xFF7C: p = 's';  break;
										case 0xFF7D: p = 'd';  break;
										case 0xFF7E: p = 'f';  break;
										case 0xFF7F: p = 'g';  break;

										case 0xFF80: p = 'z';  break;
										case 0xFF81: p = 'x';  break;
										case 0xFF82: p = 'c';  break;
										case 0xFF83: p = 'v';  break;
										case 0xFF84: p = 'b';  break;

										case 0xFF85: p = '6';  break;
										case 0xFF86: p = '7';  break;
										case 0xFF87: p = '8';  break;
										case 0xFF88: p = '9';  break;
										case 0xFF89: p = '0';  break;

										case 0xFF8A: p = 'y';  break;
										case 0xFF8B: p = 'u';  break;
										case 0xFF8C: p = 'i';  break;
										case 0xFF8D: p = 'o';  break;
										case 0xFF8E: p = 'p';  break;

										case 0xFF8F: p = 'h';  break;
										case 0xFF90: p = 'j';  break;
										case 0xFF91: p = 'k';  break;
										case 0xFF92: p = 'l';  break;
										case 0xFF93: p = ';';  break;

										case 0xFF94: p = 'n';  break;
										case 0xFF95: p = 'm';  break;
										case 0xFF96: p = ',';  break;

										case 0xFF6C: p = 'n' | 0x80; break;
										case 0xFF6D: p = 'm' | 0x80; break;
										case 0xFF6E: p = '<';        break;

										case 0xFF97: p = '-';  break;
										case 0xFF98: p = '^';  break;
										case 0xFF99: p = '\\'; break;
										case 0xFF9A: p = '@';  break;
										case 0xFF9B: p = '[';  break;

										case 0xFF9C: p = '.';  break;
										case 0xFF66: p = '-';  break;
										case 0xFF9D: p = '\t'; break;

										case 0xFF61: p = ']';        break;
										case 0xFF62: p = '[' | 0x80; break;
										case 0xFF63: p = ']' | 0x80; break;

										default: continue;
									}

									mode = true;
								}
								else
								{
									continue;
								}
							}

							if (kana != mode)
							{
								kana = mode;
								buffer << char(0xFF);
							}

							buffer << (p);
						}

						if (kana)
							buffer << char(0xFF);

						if (buffer.Length())
							Io::Screen() << Resource::String( IDS_SCREEN_TEXT_PASTE ).Invoke( buffer.Length() );
					}
				}
			}
			else
			{
				if (keyboard[DIK_ESCAPE] & 0x80U)
					Clear();
			}

			return buffer.Length();
		}

		void Input::Clipboard::OnCmdMachineKeyboardPaste(uint)
		{
			paste = CanPaste();
			Resume();
		}

		void Input::Clipboard::OnMenuKeyboard(const Window::Menu::PopupHandler::Param& param)
		{
			param.menu[IDM_MACHINE_EXT_KEYBOARD_PASTE].Enable( !param.show || CanPaste() );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		void Input::Clipboard::operator ++ ()
		{
			hold = (hold + 1) & 7;

			if (!hold)
			{
				shifted = false;
				releasing = 32;

				if (++pos == buffer.Length())
					Clear();
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
