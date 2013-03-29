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

#include "resource/resource.h"
#include "NstObjectPod.hpp"
#include "NstSystemRegistry.hpp"
#include "NstWindowUser.hpp"
#include "NstApplicationInstance.hpp"
#include "NstWindowParam.hpp"
#include "NstManagerEmulator.hpp"
#include "NstDialogPreferences.hpp"
#include "NstIoLog.hpp"
#include "NstResourceString.hpp"
#include <CommDlg.h>

namespace Nestopia
{
	namespace Window
	{
		NST_COMPILE_ASSERT
		(
			IDC_PREFERENCES_ASSOCIATE_UNF == IDC_PREFERENCES_ASSOCIATE_NES + 1 &&
			IDC_PREFERENCES_ASSOCIATE_FDS == IDC_PREFERENCES_ASSOCIATE_NES + 2 &&
			IDC_PREFERENCES_ASSOCIATE_NSF == IDC_PREFERENCES_ASSOCIATE_NES + 3
		);

		NST_COMPILE_ASSERT
		(
			IDS_PRIORITY_ABOVE_NORMAL == IDS_PRIORITY_NORMAL + 1 &&
			IDS_PRIORITY_HIGH         == IDS_PRIORITY_NORMAL + 2
		);

		struct Preferences::MenuColorWindow
		{
			COLORREF color;
			Rect rect;
		};

		Preferences::MenuColorWindow Preferences::menuColorWindows[2];

		class Preferences::Association
		{
		public:

			enum
			{
				NUM_EXTENSIONS = 4
			};

			explicit Association(bool=false);
			~Association();

			void Create(uint,uint);
			void Delete(uint);
			void Update(uint,uint);
			bool Enabled(uint) const;

		private:

			enum
			{
				EXTENSION,
				NAME,
				DESCRIPTION,
				NUM_KEYTYPES
			};

			System::Registry registry;
			bool refresh;
			bool updated;
			const bool notify;

			static wcstring const keyNames[NUM_EXTENSIONS][NUM_KEYTYPES];
		};

		wcstring const Preferences::Association::keyNames[NUM_EXTENSIONS][NUM_KEYTYPES] =
		{
			{ L".nes", L"Nestopia.nes", L"Nestopia iNES File"                },
			{ L".unf", L"Nestopia.unf", L"Nestopia UNIF File"                },
			{ L".fds", L"Nestopia.fds", L"Nestopia Famicom Disk System File" },
			{ L".nsf", L"Nestopia.nsf", L"Nestopia NES Sound File"           }
		};

		Preferences::Association::Association(bool n)
		: refresh(false), updated(false), notify(n) {}

		Preferences::Association::~Association()
		{
			if (refresh)
				System::Registry::UpdateAssociations();

			if (notify && updated)
			{
				User::Inform
				(
					IDS_DIALOG_PREFERENCES_REGISTRYUPDATED,
					IDS_DIALOG_PREFERENCES_REGISTRYUPDATED_TITLE
				);
			}
		}

		bool Preferences::Association::Enabled(uint index) const
		{
			HeapString tmp;
			return (registry[keyNames[index][EXTENSION]] >> tmp) && (tmp == keyNames[index][NAME]);
		}

		void Preferences::Association::Update(const uint index,const uint icon)
		{
			if (Enabled( index ))
			{
				HeapString path( Application::Instance::GetExePath() );

				// "nestopia.extension\DefaultIcon" <- "drive:\directory\nestopia.exe,icon"

				if (registry[keyNames[index][NAME]][L"DefaultIcon"] << (path << ',' << icon))
				{
					refresh = true;
					updated = true;
				}

				path.ShrinkTo( Application::Instance::GetExePath().Length() );

				// "nestopia.extension\Shell\Open\Command" <- "drive:\directory\nestopia.exe "%1"

				if (registry[keyNames[index][NAME]][L"Shell\\Open\\Command"] << (path << " \"%1\""))
					updated = true;
			}
		}

		void Preferences::Association::Create(const uint index,const uint icon)
		{
			// ".extension" will point to "nestopia.extension"

			const bool tmp = updated;
			updated = false;

			if (registry[keyNames[index][EXTENSION]] << keyNames[index][NAME])
				updated = true;

			if (registry[keyNames[index][NAME]] << keyNames[index][DESCRIPTION])
				updated = true;

			Update( index, icon );

			if (updated)
			{
				Io::Log() << "Preferences: creating registry keys: \"HKEY_CLASSES_ROOT\\"
                          << keyNames[index][EXTENSION]
                          << "\" and \"HKEY_CLASSES_ROOT\\"
                          << keyNames[index][NAME]
                          << "\"..\r\n";
			}
			else
			{
				updated = tmp;
			}
		}

		void Preferences::Association::Delete(const uint index)
		{
			bool log = false;

			// remove ".extension" (if default) and "nestopia.extension"

			if (registry[keyNames[index][EXTENSION]].Delete( keyNames[index][NAME] ))
				refresh = updated = log = true;

			if (registry[keyNames[index][NAME]].Delete())
				updated = log = true;

			if (log)
			{
				Io::Log() << "Preferences: deleting registry keys: \"HKEY_CLASSES_ROOT\\"
                          << keyNames[index][EXTENSION]
                          << "\" and \"HKEY_CLASSES_ROOT\\"
                          << keyNames[index][NAME]
                          << "\"..\r\n";
			}
		}

		const ushort Preferences::icons[Preferences::Association::NUM_EXTENSIONS][5] =
		{
			{ IDC_PREFERENCES_ICON_NES, IDI_NES, IDI_NES_J,  2,  3 },
			{ IDC_PREFERENCES_ICON_UNF, IDI_UNF, IDI_UNF_J,  4,  5 },
			{ IDC_PREFERENCES_ICON_FDS, IDI_FDS, IDI_FDS,    6,  6 },
			{ IDC_PREFERENCES_ICON_NSF, IDI_NSF, IDI_NSF_J,  7,  8 }
		};

		struct Preferences::Handlers
		{
			static const MsgHandler::Entry<Preferences> messages[];
			static const MsgHandler::Entry<Preferences> commands[];
		};

		const MsgHandler::Entry<Preferences> Preferences::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &Preferences::OnInitDialog  },
			{ WM_PAINT,      &Preferences::OnPaint       }
		};

		const MsgHandler::Entry<Preferences> Preferences::Handlers::commands[] =
		{
			{ IDC_PREFERENCES_STYLE_NES,                    &Preferences::OnCmdStyle            },
			{ IDC_PREFERENCES_STYLE_FAMICOM,                &Preferences::OnCmdStyle            },
			{ IDC_PREFERENCES_MENUCOLOR_DESKTOP_CHANGE,     &Preferences::OnCmdMenuColorChange  },
			{ IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_CHANGE,  &Preferences::OnCmdMenuColorChange  },
			{ IDC_PREFERENCES_MENUCOLOR_DESKTOP_DEFAULT,    &Preferences::OnCmdMenuColorDefault },
			{ IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_DEFAULT, &Preferences::OnCmdMenuColorDefault },
			{ IDC_PREFERENCES_DEFAULT,                      &Preferences::OnCmdDefault          },
			{ IDOK,                                         &Preferences::OnCmdOk               }
		};

		Preferences::Preferences(Managers::Emulator& e,const Configuration& cfg)
		:
		dialog   ( IDD_PREFERENCES, this, Handlers::messages, Handlers::commands ),
		emulator ( e )
		{
			NST_COMPILE_ASSERT
			(
				START_IN_FULLSCREEN      == IDC_PREFERENCES_STARTUP_FULLSCREEN    - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
				SUPPRESS_WARNINGS        == IDC_PREFERENCES_DISABLE_ROM_WARNINGS  - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
				FIRST_UNLOAD_ON_EXIT     == IDC_PREFERENCES_CLOSE_POWER_OFF       - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
				CONFIRM_EXIT             == IDC_PREFERENCES_CONFIRM_EXIT          - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
				RUN_IN_BACKGROUND        == IDC_PREFERENCES_RUN_IN_BACKGROUND     - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
				AUTOSTART_EMULATION      == IDC_PREFERENCES_BEGIN_EMULATION       - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
				SAVE_LOGFILE             == IDC_PREFERENCES_SAVE_LOGFILE          - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
				ALLOW_MULTIPLE_INSTANCES == IDC_PREFERENCES_MULTIPLE_INSTANCES    - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
				SAVE_LAUNCHER            == IDC_PREFERENCES_SAVE_LAUNCHER         - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
				CONFIRM_RESET            == IDC_PREFERENCES_CONFIRM_RESET         - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
				SAVE_CHEATS              == IDC_PREFERENCES_SAVE_CHEATCODES       - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
				SAVE_NETPLAY_GAMELIST    == IDC_PREFERENCES_SAVE_NETPLAY_GAMELIST - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
				SAVE_WINDOWPOS           == IDC_PREFERENCES_SAVE_WINDOWPOS        - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
				SAVE_LAUNCHERSIZE        == IDC_PREFERENCES_SAVE_LAUNCHERSIZE     - IDC_PREFERENCES_STARTUP_FULLSCREEN
			);

			Configuration::ConstSection preferences( cfg["preferences"] );

			{
				Configuration::ConstSection application( preferences["application"] );

				settings[ AUTOSTART_EMULATION      ] = !application[ "autostart"                ].No();
				settings[ RUN_IN_BACKGROUND        ] =  application[ "run-background"           ].Yes();
				settings[ START_IN_FULLSCREEN      ] =  application[ "start-fullscreen"         ].Yes();
				settings[ SUPPRESS_WARNINGS        ] =  application[ "suppress-warnings"        ].Yes();
				settings[ FIRST_UNLOAD_ON_EXIT     ] =  application[ "exit-power-off"           ].Yes();
				settings[ CONFIRM_EXIT             ] = !application[ "confirm-exit"             ].No();
				settings[ CONFIRM_RESET            ] =  application[ "confirm-reset"            ].Yes();
				settings[ ALLOW_MULTIPLE_INSTANCES ] =  application[ "allow-multiple-instances" ].Yes();

				{
					const GenericString priority( application[ "priority" ].Str() );

					if (priority == L"high")
					{
						settings.priority = PRIORITY_HIGH;
					}
					else if (priority == L"above normal")
					{
						settings.priority = PRIORITY_ABOVE_NORMAL;
					}
					else
					{
						settings.priority = PRIORITY_NORMAL;
					}
				}

				{
					const GenericString favored( application[ "favored-system" ].Str() );

					if (favored == L"nes-pal")
					{
						settings.favoredSystem = Nes::Machine::FAVORED_NES_PAL;
					}
					else if (favored == L"famicom")
					{
						settings.favoredSystem = Nes::Machine::FAVORED_FAMICOM;
					}
					else if (favored == L"dendy")
					{
						settings.favoredSystem = Nes::Machine::FAVORED_DENDY;
					}
					else
					{
						settings.favoredSystem = Nes::Machine::FAVORED_NES_NTSC;
					}
				}

				settings.alwaysAskSystem = application[ "favored-system-always-ask" ].Yes();
				settings.disableStatusMsg = application[ "disable-statusmsg" ].Yes();
			}

			{
				Configuration::ConstSection save( preferences["save"] );

				settings[ SAVE_LOGFILE             ] = !save[ "logfile"         ].No();
				settings[ SAVE_SETTINGS            ] = !save[ "settings"        ].No();
				settings[ SAVE_LAUNCHER            ] = !save[ "launcher"        ].No();
				settings[ SAVE_CHEATS              ] = !save[ "cheats"          ].No();
				settings[ SAVE_NETPLAY_GAMELIST    ] = !save[ "netplay-list"    ].No();
				settings[ SAVE_WINDOWPOS           ] =  save[ "window-main"     ].Yes();
				settings[ SAVE_LAUNCHERSIZE        ] =  save[ "window-launcher" ].Yes();
			}

			{
				Configuration::ConstSection appearance( preferences["appearance"] );

				settings.menuLookDesktop.enabled    = appearance[ "menu-desktop"    ][ "use-custom-color" ].Yes();
				settings.menuLookFullscreen.enabled = appearance[ "menu-fullscreen" ][ "use-custom-color" ].Yes();

				settings.menuLookDesktop.color    = appearance[ "menu-desktop"    ][ "custom-color" ].Int( DEFAULT_DESKTOP_MENU_COLOR );
				settings.menuLookFullscreen.color = appearance[ "menu-fullscreen" ][ "custom-color" ].Int( DEFAULT_FULLSCREEN_MENU_COLOR );

				Application::Instance::SetIconStyle
				(
					appearance[ "icon-style" ].Str() == L"famicom" ? Application::Instance::ICONSTYLE_FAMICOM :
                                                                     Application::Instance::ICONSTYLE_NES
				);
			}

			Association association;
			const uint iconOffset = (Application::Instance::GetIconStyle() == Application::Instance::ICONSTYLE_NES ? 3 : 4);

			for (uint i=0; i < Association::NUM_EXTENSIONS; ++i)
				association.Update( i, icons[i][iconOffset] );
		}

		void Preferences::Save(Configuration& cfg) const
		{
			Configuration::Section preferences( cfg["preferences"] );

			{
				Configuration::Section application( preferences["application"] );

				application[ "autostart"                ].YesNo() = settings[ AUTOSTART_EMULATION      ];
				application[ "run-background"           ].YesNo() = settings[ RUN_IN_BACKGROUND        ];
				application[ "start-fullscreen"         ].YesNo() = settings[ START_IN_FULLSCREEN      ];
				application[ "suppress-warnings"        ].YesNo() = settings[ SUPPRESS_WARNINGS        ];
				application[ "exit-power-off"           ].YesNo() = settings[ FIRST_UNLOAD_ON_EXIT     ];
				application[ "confirm-exit"             ].YesNo() = settings[ CONFIRM_EXIT             ];
				application[ "confirm-reset"            ].YesNo() = settings[ CONFIRM_RESET            ];
				application[ "allow-multiple-instances" ].YesNo() = settings[ ALLOW_MULTIPLE_INSTANCES ];

				application[ "priority" ].Str() =
				(
					settings.priority == PRIORITY_HIGH         ? "high"         :
					settings.priority == PRIORITY_ABOVE_NORMAL ? "above normal" :
                                                                 "normal"
				);

				application[ "favored-system" ].Str() =
				(
					settings.favoredSystem == Nes::Machine::FAVORED_NES_PAL ? "nes-pal"    :
					settings.favoredSystem == Nes::Machine::FAVORED_FAMICOM ? "famicom"    :
					settings.favoredSystem == Nes::Machine::FAVORED_DENDY   ? "dendy"      :
                                                                              "nes-ntsc"
				);

				application[ "favored-system-always-ask" ].YesNo() = settings.alwaysAskSystem;
				application[ "disable-statusmsg"         ].YesNo() = settings.disableStatusMsg;
			}

			{
				Configuration::Section save( preferences["save"] );

				save[ "logfile"         ].YesNo() = settings[ SAVE_LOGFILE          ];
				save[ "settings"        ].YesNo() = settings[ SAVE_SETTINGS         ];
				save[ "launcher"        ].YesNo() = settings[ SAVE_LAUNCHER         ];
				save[ "cheats"          ].YesNo() = settings[ SAVE_CHEATS           ];
				save[ "netplay-list"    ].YesNo() = settings[ SAVE_NETPLAY_GAMELIST ];
				save[ "window-main"     ].YesNo() = settings[ SAVE_WINDOWPOS        ];
				save[ "window-launcher" ].YesNo() = settings[ SAVE_LAUNCHERSIZE     ];
			}

			{
				Configuration::Section appearance( preferences["appearance"] );

				appearance[ "icon-style" ].Str() =
				(
					Application::Instance::GetIconStyle() == Application::Instance::ICONSTYLE_NES ? "nes" :
																									"famicom"
				);

				appearance[ "menu-desktop"    ][ "use-custom-color" ].YesNo() = settings.menuLookDesktop.enabled;
				appearance[ "menu-fullscreen" ][ "use-custom-color" ].YesNo() = settings.menuLookFullscreen.enabled;

				appearance[ "menu-desktop"    ][ "custom-color" ].Str() = HexString( 32, settings.menuLookDesktop.color );
				appearance[ "menu-fullscreen" ][ "custom-color" ].Str() = HexString( 32, settings.menuLookFullscreen.color );
			}
		}

		ibool Preferences::OnInitDialog(Param&)
		{
			for (uint i=0; i < 2; ++i)
			{
				MenuColorWindow& type = menuColorWindows[i];

				type.color = i ? settings.menuLookFullscreen.color : settings.menuLookDesktop.color;
				type.rect = dialog.Control(i ? IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_CHANGE : IDC_PREFERENCES_MENUCOLOR_DESKTOP_CHANGE).GetWindow().Coordinates();
				type.rect.Position() -= Point(type.rect.Width()+8,0);
				type.rect.ClientTransform( dialog );
			}

			dialog.Control( IDC_PREFERENCES_MENUCOLOR_DESKTOP_CHANGE ).Enable( settings.menuLookDesktop.enabled );
			dialog.Control( IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_CHANGE ).Enable( settings.menuLookFullscreen.enabled );

			dialog.CheckBox( IDC_PREFERENCES_MENUCOLOR_DESKTOP_DEFAULT ).Check( !settings.menuLookDesktop.enabled );
			dialog.CheckBox( IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_DEFAULT ).Check( !settings.menuLookFullscreen.enabled );

			for (uint i=0; i < NUM_SETTINGS; ++i)
			{
				if (i != SAVE_SETTINGS)
					dialog.CheckBox( IDC_PREFERENCES_STARTUP_FULLSCREEN + i ).Check( settings[i] );
			}

			{
				Association association;

				for (uint i=0; i < Association::NUM_EXTENSIONS; ++i)
					dialog.CheckBox( IDC_PREFERENCES_ASSOCIATE_NES + i ).Check( association.Enabled(i) );
			}

			dialog.RadioButton( IDC_PREFERENCES_FAVORED_NES_NTSC ).Check( settings.favoredSystem == Nes::Machine::FAVORED_NES_NTSC );
			dialog.RadioButton( IDC_PREFERENCES_FAVORED_NES_PAL  ).Check( settings.favoredSystem == Nes::Machine::FAVORED_NES_PAL  );
			dialog.RadioButton( IDC_PREFERENCES_FAVORED_FAMICOM  ).Check( settings.favoredSystem == Nes::Machine::FAVORED_FAMICOM  );
			dialog.RadioButton( IDC_PREFERENCES_FAVORED_DENDY    ).Check( settings.favoredSystem == Nes::Machine::FAVORED_DENDY    );

			dialog.CheckBox( IDC_PREFERENCES_FAVORED_ALWAYS_ASK ).Check( settings.alwaysAskSystem );
			dialog.CheckBox( IDC_PREFERENCES_DISABLE_STATUSMSG  ).Check( settings.disableStatusMsg );

			{
				Control::ComboBox priorities( dialog.ComboBox( IDC_PREFERENCES_PRIORITY ) );

				for (uint i=IDS_PRIORITY_NORMAL; i <= IDS_PRIORITY_HIGH; ++i)
					priorities.Add( Resource::String(i) );

				priorities[settings.priority].Select();
			}

			dialog.RadioButton( IDC_PREFERENCES_STYLE_NES ).Check( Application::Instance::GetIconStyle() == Application::Instance::ICONSTYLE_NES );
			dialog.RadioButton( IDC_PREFERENCES_STYLE_FAMICOM ).Check( Application::Instance::GetIconStyle() == Application::Instance::ICONSTYLE_FAMICOM );

			UpdateIconStyle();
			UpdateColors();

			return true;
		}

		void Preferences::UpdateIconStyle() const
		{
			const uint style = (dialog.RadioButton(IDC_PREFERENCES_STYLE_NES).Checked() ? 1 : 2);

			for (uint i=0; i < 5; ++i)
				dialog.SetItemIcon( icons[i][0], icons[i][style] );
		}

		void Preferences::UpdateColors() const
		{
			if (HDC const hDC = ::GetDC( dialog ))
			{
				HPEN const hPen = ::CreatePen( PS_SOLID, 1, RGB(0x00,0x00,0x00) );
				HPEN const hPenOld = static_cast<HPEN>(::SelectObject( hDC, hPen ));

				for (uint i=0; i < 2; ++i)
				{
					const MenuColorWindow& type = menuColorWindows[i];

					HBRUSH const hBrush = ::CreateSolidBrush( type.color );
					HBRUSH const hBrushOld = static_cast<HBRUSH>(::SelectObject( hDC, hBrush ));

					::Rectangle( hDC, type.rect.left, type.rect.top, type.rect.right, type.rect.bottom );

					::SelectObject( hDC, hBrushOld );
					::DeleteObject( hBrush );
				}

				::SelectObject( hDC, hPenOld );
				::DeleteObject( hPen );

				::ReleaseDC( dialog, hDC );
			}
		}

		ibool Preferences::OnPaint(Param&)
		{
			UpdateColors();
			return false;
		}

		ibool Preferences::OnCmdStyle(Param& param)
		{
			if (param.Button().Clicked())
				UpdateIconStyle();

			return true;
		}

		ibool Preferences::OnCmdMenuColorChange(Param& param)
		{
			if (param.Button().Clicked())
			{
				static COLORREF customColors[16] = {0};

				Object::Pod<CHOOSECOLOR> cc;

				MenuColorWindow& type = menuColorWindows[param.Button().GetId() == IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_CHANGE];

				cc.lStructSize  = sizeof(cc);
				cc.hwndOwner    = dialog;
				cc.lpCustColors = customColors;
				cc.rgbResult    = type.color;
				cc.Flags        = CC_FULLOPEN|CC_RGBINIT;

				if (::ChooseColor( &cc ))
				{
					type.color = cc.rgbResult;
					UpdateColors();
				}
			}

			return true;
		}

		ibool Preferences::OnCmdMenuColorDefault(Param& param)
		{
			if (param.Button().Clicked())
			{
				uint id;

				if (param.Button().GetId() == IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_DEFAULT)
					id = IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_CHANGE;
				else
					id = IDC_PREFERENCES_MENUCOLOR_DESKTOP_CHANGE;

				dialog.Control( id ).Enable( dialog.CheckBox(param.Button().GetId()).Unchecked() );
			}

			return true;
		}

		ibool Preferences::OnCmdDefault(Param&)
		{
			dialog.CheckBox( IDC_PREFERENCES_BEGIN_EMULATION       ).Check( true  );
			dialog.CheckBox( IDC_PREFERENCES_RUN_IN_BACKGROUND     ).Check( false );
			dialog.CheckBox( IDC_PREFERENCES_STARTUP_FULLSCREEN    ).Check( false );
			dialog.CheckBox( IDC_PREFERENCES_DISABLE_ROM_WARNINGS  ).Check( false );
			dialog.CheckBox( IDC_PREFERENCES_CLOSE_POWER_OFF       ).Check( false );
			dialog.CheckBox( IDC_PREFERENCES_CONFIRM_EXIT          ).Check( true  );
			dialog.CheckBox( IDC_PREFERENCES_CONFIRM_RESET         ).Check( false );
			dialog.CheckBox( IDC_PREFERENCES_MULTIPLE_INSTANCES    ).Check( false );
			dialog.CheckBox( IDC_PREFERENCES_SAVE_LOGFILE          ).Check( true  );
			dialog.CheckBox( IDC_PREFERENCES_SAVE_LAUNCHER         ).Check( true  );
			dialog.CheckBox( IDC_PREFERENCES_SAVE_CHEATCODES       ).Check( true  );
			dialog.CheckBox( IDC_PREFERENCES_SAVE_NETPLAY_GAMELIST ).Check( true  );
			dialog.CheckBox( IDC_PREFERENCES_SAVE_WINDOWPOS        ).Check( false );
			dialog.CheckBox( IDC_PREFERENCES_SAVE_LAUNCHERSIZE     ).Check( false );
			dialog.CheckBox( IDC_PREFERENCES_DISABLE_STATUSMSG     ).Check( false );

			dialog.ComboBox( IDC_PREFERENCES_PRIORITY )[ PRIORITY_NORMAL ].Select();

			dialog.RadioButton( IDC_PREFERENCES_FAVORED_NES_NTSC ).Check( true  );
			dialog.RadioButton( IDC_PREFERENCES_FAVORED_NES_PAL  ).Check( false );
			dialog.RadioButton( IDC_PREFERENCES_FAVORED_FAMICOM  ).Check( false );
			dialog.RadioButton( IDC_PREFERENCES_FAVORED_DENDY    ).Check( false );

			dialog.CheckBox( IDC_PREFERENCES_FAVORED_ALWAYS_ASK ).Check( false );

			dialog.CheckBox( IDC_PREFERENCES_MENUCOLOR_DESKTOP_DEFAULT ).Check( true );
			dialog.CheckBox( IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_DEFAULT ).Check( true );

			dialog.Control( IDC_PREFERENCES_MENUCOLOR_DESKTOP_CHANGE ).Enable( false );
			dialog.Control( IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_CHANGE ).Enable( false );

			menuColorWindows[0].color = DEFAULT_DESKTOP_MENU_COLOR;
			menuColorWindows[1].color = DEFAULT_FULLSCREEN_MENU_COLOR;

			UpdateColors();

			return true;
		}

		ibool Preferences::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				for (uint i=0; i < NUM_SETTINGS; ++i)
				{
					if (i != SAVE_SETTINGS)
						settings[i] = dialog.CheckBox( IDC_PREFERENCES_STARTUP_FULLSCREEN + i ).Checked();
				}

				settings.priority = static_cast<Priority>(dialog.ComboBox( IDC_PREFERENCES_PRIORITY ).Selection().GetIndex());

				settings.menuLookDesktop.color = menuColorWindows[0].color;
				settings.menuLookFullscreen.color = menuColorWindows[1].color;

				settings.menuLookDesktop.enabled = dialog.CheckBox( IDC_PREFERENCES_MENUCOLOR_DESKTOP_DEFAULT ).Unchecked();
				settings.menuLookFullscreen.enabled = dialog.CheckBox( IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_DEFAULT ).Unchecked();

				settings.favoredSystem =
				(
					dialog.RadioButton( IDC_PREFERENCES_FAVORED_NES_PAL ).Checked() ? Nes::Machine::FAVORED_NES_PAL :
					dialog.RadioButton( IDC_PREFERENCES_FAVORED_FAMICOM ).Checked() ? Nes::Machine::FAVORED_FAMICOM :
					dialog.RadioButton( IDC_PREFERENCES_FAVORED_DENDY   ).Checked() ? Nes::Machine::FAVORED_DENDY   :
                                                                                      Nes::Machine::FAVORED_NES_NTSC
				);

				settings.alwaysAskSystem = dialog.RadioButton( IDC_PREFERENCES_FAVORED_ALWAYS_ASK ).Checked();
				settings.disableStatusMsg = dialog.RadioButton( IDC_PREFERENCES_DISABLE_STATUSMSG ).Checked();

				Application::Instance::SetIconStyle( dialog.RadioButton(IDC_PREFERENCES_STYLE_NES).Checked() ? Application::Instance::ICONSTYLE_NES : Application::Instance::ICONSTYLE_FAMICOM );

				{
					Association association( true );
					const uint iconOffset = (Application::Instance::GetIconStyle() == Application::Instance::ICONSTYLE_NES ? 3 : 4);

					for (uint i=0; i < Association::NUM_EXTENSIONS; ++i)
					{
						if (dialog.CheckBox( IDC_PREFERENCES_ASSOCIATE_NES + i ).Checked())
							association.Create( i, icons[i][iconOffset] );
						else
							association.Delete( i );
					}
				}

				dialog.Close();
			}

			return true;
		}
	}
}
