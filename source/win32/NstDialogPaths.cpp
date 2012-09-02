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

#include "NstResourceString.hpp"
#include "NstApplicationInstance.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowUser.hpp"
#include "NstDialogPaths.hpp"
#include "NstDialogBrowse.hpp"

namespace Nestopia
{
	namespace Window
	{
		NST_COMPILE_ASSERT
		(
			IDC_PATHS_BATTERY     - IDC_PATHS_IMAGE == IDC_PATHS_BATTERY_BROWSE     - IDC_PATHS_IMAGE_BROWSE &&
			IDC_PATHS_NST         - IDC_PATHS_IMAGE == IDC_PATHS_NST_BROWSE         - IDC_PATHS_IMAGE_BROWSE &&
			IDC_PATHS_SAMPLES     - IDC_PATHS_IMAGE == IDC_PATHS_SAMPLES_BROWSE     - IDC_PATHS_IMAGE_BROWSE &&
			IDC_PATHS_CHEATS      - IDC_PATHS_IMAGE == IDC_PATHS_CHEATS_BROWSE      - IDC_PATHS_IMAGE_BROWSE &&
			IDC_PATHS_PATCHES     - IDC_PATHS_IMAGE == IDC_PATHS_PATCHES_BROWSE     - IDC_PATHS_IMAGE_BROWSE &&
			IDC_PATHS_SCREENSHOTS - IDC_PATHS_IMAGE == IDC_PATHS_SCREENSHOTS_BROWSE - IDC_PATHS_IMAGE_BROWSE
		);

		struct Paths::Lut
		{
			struct A
			{
				ushort type;
				ushort dlg;
				wcstring def;
			};

			struct B
			{
				ushort type;
				ushort dlg;
			};

			struct C
			{
				ushort type;
				ushort dlg;
				wcstring cfg;
			};

			static const A dirs[NUM_DIRS];
			static const B flags[NUM_FLAGS];
			static const C screenShots[NUM_SCREENSHOTS];
		};

		const Paths::Lut::A Paths::Lut::dirs[NUM_DIRS] =
		{
			{ DIR_IMAGE,      IDC_PATHS_IMAGE,       L""              },
			{ DIR_SAVE,       IDC_PATHS_BATTERY,     L"save\\"        },
			{ DIR_STATE,      IDC_PATHS_NST,         L"states\\"      },
			{ DIR_SAMPLES,    IDC_PATHS_SAMPLES,     L"samples\\"     },
			{ DIR_CHEATS,     IDC_PATHS_CHEATS,      L"cheats\\"      },
			{ DIR_PATCHES,    IDC_PATHS_PATCHES,     L"patches\\"     },
			{ DIR_SCREENSHOT, IDC_PATHS_SCREENSHOTS, L"screenshots\\" }
		};

		const Paths::Lut::B Paths::Lut::flags[NUM_FLAGS] =
		{
			{ USE_LAST_IMAGE_DIR,      IDC_PATHS_IMAGE_LAST                },
			{ READONLY_CARTRIDGE,      IDC_PATHS_BATTERY_PROTECT           },
			{ AUTO_IMPORT_STATE_SLOTS, IDC_PATHS_NST_AUTO_IMPORT           },
			{ AUTO_EXPORT_STATE_SLOTS, IDC_PATHS_NST_AUTO_EXPORT           },
			{ CHEATS_AUTO_LOAD,        IDC_PATHS_CHEATS_AUTO_LOAD          },
			{ CHEATS_AUTO_SAVE,        IDC_PATHS_CHEATS_AUTO_SAVE          },
			{ PATCH_AUTO_APPLY,        IDC_PATHS_PATCHES_AUTO_APPLY        },
			{ PATCH_BYPASS_VALIDATION, IDC_PATHS_PATCHES_BYPASS_VALIDATION },
			{ COMPRESS_STATES,         IDC_PATHS_NST_COMPRESS              }
		};

		const Paths::Lut::C Paths::Lut::screenShots[NUM_SCREENSHOTS] =
		{
			{ SCREENSHOT_PNG,  IDC_PATHS_SCREENSHOTS_PNG,  L"png" },
			{ SCREENSHOT_JPEG, IDC_PATHS_SCREENSHOTS_JPEG, L"jpg" },
			{ SCREENSHOT_BMP,  IDC_PATHS_SCREENSHOTS_BMP,  L"bmp" }
		};

		struct Paths::Handlers
		{
			static const MsgHandler::Entry<Paths> messages[];
			static const MsgHandler::Entry<Paths> commands[];
		};

		const MsgHandler::Entry<Paths> Paths::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &Paths::OnInitDialog }
		};

		const MsgHandler::Entry<Paths> Paths::Handlers::commands[] =
		{
			{ IDC_PATHS_IMAGE_BROWSE,       &Paths::OnCmdBrowse      },
			{ IDC_PATHS_BATTERY_BROWSE,     &Paths::OnCmdBrowse      },
			{ IDC_PATHS_NST_BROWSE,         &Paths::OnCmdBrowse      },
			{ IDC_PATHS_SAMPLES_BROWSE,     &Paths::OnCmdBrowse      },
			{ IDC_PATHS_CHEATS_BROWSE,      &Paths::OnCmdBrowse      },
			{ IDC_PATHS_PATCHES_BROWSE,     &Paths::OnCmdBrowse      },
			{ IDC_PATHS_SCREENSHOTS_BROWSE, &Paths::OnCmdBrowse      },
			{ IDC_PATHS_IMAGE_LAST,         &Paths::OnCmdLastVisited },
			{ IDC_PATHS_DEFAULT,            &Paths::OnCmdDefault     },
			{ IDOK,                         &Paths::OnCmdOk          }
		};

		inline Paths::Settings::Flags::Flags()
		: Collection::BitSet
		(
			1U << USE_LAST_IMAGE_DIR      |
			1U << AUTO_IMPORT_STATE_SLOTS |
			1U << AUTO_EXPORT_STATE_SLOTS |
			1U << COMPRESS_STATES
		)
		{}

		Paths::Settings::Settings()
		: screenShotFormat(SCREENSHOT_PNG) {}

		Paths::Paths(const Configuration& cfg)
		: dialog(IDD_PATHS,this,Handlers::messages,Handlers::commands)
		{
			Configuration::ConstSection paths( cfg["paths"] );

			{
				Configuration::ConstSection images( paths["images"] );

				settings.dirs[DIR_IMAGE] = images["directory"].Str();

				if (images["use-recent-directory"].Yes())
				{
					settings.flags[USE_LAST_IMAGE_DIR] = true;
				}
				else if (images["use-recent-directory"].No())
				{
					settings.flags[USE_LAST_IMAGE_DIR] = false;
				}
			}

			{
				Configuration::ConstSection saves( paths["saves"] );

				settings.dirs[DIR_SAVE] = saves["directory"].Str();

				if (saves["force-read-only"].Yes())
				{
					settings.flags[READONLY_CARTRIDGE] = true;
				}
				else if (saves["force-read-only"].No())
				{
					settings.flags[READONLY_CARTRIDGE] = false;
				}
			}

			{
				Configuration::ConstSection states( paths["states"] );

				settings.dirs[DIR_STATE] = states["directory"].Str();

				if (states["use-compression"].Yes())
				{
					settings.flags[COMPRESS_STATES] = true;
				}
				else if (states["use-compression"].No())
				{
					settings.flags[COMPRESS_STATES] = false;
				}

				if (states["auto-import"].Yes())
				{
					settings.flags[AUTO_IMPORT_STATE_SLOTS] = true;
				}
				else if (states["auto-import"].No())
				{
					settings.flags[AUTO_IMPORT_STATE_SLOTS] = false;
				}

				if (states["auto-export"].Yes())
				{
					settings.flags[AUTO_EXPORT_STATE_SLOTS] = true;
				}
				else if (states["auto-export"].No())
				{
					settings.flags[AUTO_EXPORT_STATE_SLOTS] = false;
				}
			}

			{
				Configuration::ConstSection samples( paths["samples"] );

				settings.dirs[DIR_SAMPLES] = samples["directory"].Str();
			}

			{
				Configuration::ConstSection patches( paths["cheats"] );

				settings.dirs[DIR_CHEATS] = patches["directory"].Str();

				if (patches["auto-load"].Yes())
				{
					settings.flags[CHEATS_AUTO_LOAD] = true;
				}
				else if (patches["auto-load"].No())
				{
					settings.flags[CHEATS_AUTO_LOAD] = false;
				}

				if (patches["auto-save"].Yes())
				{
					settings.flags[CHEATS_AUTO_SAVE] = true;
				}
				else if (patches["auto-save"].No())
				{
					settings.flags[CHEATS_AUTO_SAVE] = false;
				}
			}

			{
				Configuration::ConstSection patches( paths["patches"] );

				settings.dirs[DIR_PATCHES] = patches["directory"].Str();

				if (patches["auto-apply"].Yes())
				{
					settings.flags[PATCH_AUTO_APPLY] = true;
				}
				else if (patches["auto-apply"].No())
				{
					settings.flags[PATCH_AUTO_APPLY] = false;
				}

				if (patches["bypass-validation"].Yes())
				{
					settings.flags[PATCH_BYPASS_VALIDATION] = true;
				}
				else if (patches["bypass-validation"].No())
				{
					settings.flags[PATCH_BYPASS_VALIDATION] = false;
				}
			}

			{
				Configuration::ConstSection screenshots( paths["screenshots"] );

				settings.dirs[DIR_SCREENSHOT] = screenshots["directory"].Str();

				const GenericString format( screenshots["format"].Str() );

				if (format.Length())
				{
					for (uint i=0; i < NUM_SCREENSHOTS; ++i)
					{
						if (format == Lut::screenShots[i].cfg)
						{
							settings.screenShotFormat = static_cast<ScreenShotFormat>(Lut::screenShots[i].type);
							break;
						}
					}
				}
			}

			for (uint i=0; i < NUM_DIRS; ++i)
				UpdateDirectory( i );
		}

		Paths::~Paths()
		{
		}

		void Paths::Save(Configuration& cfg) const
		{
			Configuration::Section paths( cfg["paths"] );

			{
				Configuration::Section images( paths["images"] );

				images[ "directory"            ].Str() = settings.dirs[DIR_IMAGE];
				images[ "use-recent-directory" ].YesNo() = settings.flags[USE_LAST_IMAGE_DIR];
			}

			{
				Configuration::Section saves( paths["saves"] );

				saves[ "directory"       ].Str() = settings.dirs[DIR_SAVE];
				saves[ "force-read-only" ].YesNo() = settings.flags[READONLY_CARTRIDGE];
			}

			{
				Configuration::Section states( paths["states"] );

				states[ "directory"       ].Str() = settings.dirs[DIR_STATE];
				states[ "use-compression" ].YesNo() = settings.flags[COMPRESS_STATES];
				states[ "auto-import"     ].YesNo() = settings.flags[AUTO_IMPORT_STATE_SLOTS];
				states[ "auto-export"     ].YesNo() = settings.flags[AUTO_EXPORT_STATE_SLOTS];
			}

			{
				Configuration::Section samples( paths["samples"] );

				samples[ "directory" ].Str() = settings.dirs[DIR_SAMPLES];
			}

			{
				Configuration::Section patches( paths["cheats"] );

				patches[ "directory" ].Str() = settings.dirs[DIR_CHEATS];
				patches[ "auto-load" ].YesNo() = settings.flags[CHEATS_AUTO_LOAD];
				patches[ "auto-save" ].YesNo() = settings.flags[CHEATS_AUTO_SAVE];
			}

			{
				Configuration::Section patches( paths["patches"] );

				patches[ "directory"  ].Str() = settings.dirs[DIR_PATCHES];
				patches[ "auto-apply" ].YesNo() = settings.flags[PATCH_AUTO_APPLY];
				patches[ "bypass-validation" ].YesNo() = settings.flags[PATCH_BYPASS_VALIDATION];
			}

			{
				Configuration::Section screenshots( paths["screenshots"] );

				screenshots[ "directory" ].Str() = settings.dirs[DIR_SCREENSHOT];
				screenshots[ "format"    ].Str() = Lut::screenShots[settings.screenShotFormat].cfg;
			}
		}

		const GenericString Paths::GetScreenShotExtension() const
		{
			switch (settings.screenShotFormat)
			{
				case SCREENSHOT_JPEG: return L"jpg";
				case SCREENSHOT_BMP:  return L"bmp";
				default:              return L"png";
			}
		}

		const Path Paths::GetDirectory(Type type) const
		{
			return Application::Instance::GetFullPath( settings.dirs[type] );
		}

		void Paths::UpdateDirectory(const uint i)
		{
			Path def( Application::Instance::GetExePath(Lut::dirs[i].def) );
			Path& dir = settings.dirs[Lut::dirs[i].type];
			bool useDef;

			if (dir.Length())
			{
				dir.MakePretty( true );
				useDef = (dir == def);
			}
			else
			{
				dir = def;
				useDef = true;
			}

			def = Application::Instance::GetFullPath( dir );

			if (::GetFileAttributes( def.Ptr() ) == INVALID_FILE_ATTRIBUTES)
			{
				if (useDef || User::Confirm( Resource::String(IDS_FILE_ASK_CREATE_DIR).Invoke(def) ))
				{
					if (!::CreateDirectory( def.Ptr(), NULL ))
					{
						if (useDef)
							dir = Application::Instance::GetExePath().Directory();
						else
							User::Fail( IDS_FILE_ERR_CREATE_DIR );
					}
				}
			}
		}

		void Paths::Update(const bool reset) const
		{
			for (uint i=0; i < NUM_DIRS; ++i)
				dialog.Edit( Lut::dirs[i].dlg ) << (reset ? Application::Instance::GetExePath(Lut::dirs[i].def).Ptr() : settings.dirs[Lut::dirs[i].type].Ptr());

			Settings::Flags flags;

			if (!reset)
				flags = settings.flags;

			for (uint i=0; i < NUM_FLAGS; ++i)
				dialog.CheckBox( Lut::flags[i].dlg ).Check( flags[Lut::flags[i].type] );

			ScreenShotFormat screenShotFormat = SCREENSHOT_PNG;

			if (!reset)
				screenShotFormat = settings.screenShotFormat;

			for (uint i=0; i < NUM_SCREENSHOTS; ++i)
				dialog.RadioButton( Lut::screenShots[i].dlg ).Check( Lut::screenShots[i].type == screenShotFormat );

			UpdateLastVisited();
		}

		void Paths::UpdateLastVisited() const
		{
			bool unchecked = dialog.CheckBox( IDC_PATHS_IMAGE_LAST ).Unchecked();
			dialog.Control( IDC_PATHS_IMAGE ).Enable( unchecked );
			dialog.Control( IDC_PATHS_IMAGE_BROWSE ).Enable( unchecked );
		}

		ibool Paths::OnInitDialog(Param&)
		{
			Update( false );
			return true;
		}

		ibool Paths::OnCmdDefault(Param& param)
		{
			if (param.Button().Clicked())
				Update( true );

			return true;
		}

		ibool Paths::OnCmdBrowse(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint id = IDC_PATHS_IMAGE + (param.Button().GetId() - IDC_PATHS_IMAGE_BROWSE);
				dialog.Edit( id ).Try() << Browser::SelectDirectory().Ptr();
			}

			return true;
		}

		ibool Paths::OnCmdLastVisited(Param& param)
		{
			if (param.Button().Clicked())
				UpdateLastVisited();

			return true;
		}

		ibool Paths::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				for (uint i=0; i < NUM_DIRS; ++i)
				{
					dialog.Edit( Lut::dirs[i].dlg ) >> settings.dirs[Lut::dirs[i].type];
					UpdateDirectory( i );
				}

				for (uint i=0; i < NUM_FLAGS; ++i)
					settings.flags[Lut::flags[i].type] = dialog.CheckBox( Lut::flags[i].dlg ).Checked();

				for (uint i=0; i < NUM_SCREENSHOTS; ++i)
				{
					if (dialog.RadioButton( Lut::screenShots[i].dlg ).Checked())
					{
						settings.screenShotFormat = static_cast<ScreenShotFormat>(Lut::screenShots[i].type);
						break;
					}
				}

				dialog.Close();
			}

			return true;
		}
	}
}
