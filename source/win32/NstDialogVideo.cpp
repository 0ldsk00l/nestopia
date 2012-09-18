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

#include <algorithm>
#include "NstIoLog.hpp"
#include "NstWindowUser.hpp"
#include "NstWindowParam.hpp"
#include "NstResourceString.hpp"
#include "NstManagerPaths.hpp"
#include "NstApplicationInstance.hpp"
#include "NstDialogPaletteEditor.hpp"
#include "NstDialogVideoDecoder.hpp"
#include "NstDialogVideo.hpp"

namespace Nestopia
{
	namespace Window
	{
		NST_COMPILE_ASSERT
		(
			IDC_VIDEO_NTSC_TOP    == IDC_VIDEO_NTSC_LEFT + 1 &&
			IDC_VIDEO_NTSC_RIGHT  == IDC_VIDEO_NTSC_LEFT + 2 &&
			IDC_VIDEO_NTSC_BOTTOM == IDC_VIDEO_NTSC_LEFT + 3 &&
			IDC_VIDEO_PAL_LEFT    == IDC_VIDEO_NTSC_LEFT + 4 &&
			IDC_VIDEO_PAL_TOP     == IDC_VIDEO_NTSC_LEFT + 5 &&
			IDC_VIDEO_PAL_RIGHT   == IDC_VIDEO_NTSC_LEFT + 6 &&
			IDC_VIDEO_PAL_BOTTOM  == IDC_VIDEO_NTSC_LEFT + 7
		);

		NST_COMPILE_ASSERT
		(
			IDC_VIDEO_32_BIT == IDC_VIDEO_16_BIT + 1
		);

		NST_COMPILE_ASSERT
		(
			IDC_VIDEO_PALETTE_YUV      == IDC_VIDEO_PALETTE_AUTO + 1 &&
			IDC_VIDEO_PALETTE_RGB      == IDC_VIDEO_PALETTE_AUTO + 2 &&
			IDC_VIDEO_PALETTE_CUSTOM   == IDC_VIDEO_PALETTE_AUTO + 3 &&
			IDC_VIDEO_PALETTE_PATH     == IDC_VIDEO_PALETTE_AUTO + 4 &&
			IDC_VIDEO_PALETTE_BROWSE   == IDC_VIDEO_PALETTE_AUTO + 5 &&
			IDC_VIDEO_PALETTE_CLEAR    == IDC_VIDEO_PALETTE_AUTO + 6 &&
			IDC_VIDEO_PALETTE_EDITOR   == IDC_VIDEO_PALETTE_AUTO + 7
		);

		NST_COMPILE_ASSERT
		(
			Nes::Video::MIN_BRIGHTNESS == -100 && Nes::Video::DEFAULT_BRIGHTNESS == 0 && Nes::Video::MAX_BRIGHTNESS == +100 &&
			Nes::Video::MIN_SATURATION == -100 && Nes::Video::DEFAULT_SATURATION == 0 && Nes::Video::MAX_SATURATION == +100 &&
			Nes::Video::MIN_CONTRAST   == -100 && Nes::Video::DEFAULT_CONTRAST   == 0 && Nes::Video::MAX_CONTRAST   == +100
		);

		struct Video::Handlers
		{
			static const MsgHandler::Entry<Video> messages[];
			static const MsgHandler::Entry<Video> commands[];
		};

		const MsgHandler::Entry<Video> Video::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &Video::OnInitDialog },
			{ WM_DESTROY,    &Video::OnDestroy    },
			{ WM_HSCROLL,    &Video::OnHScroll    }
		};

		const MsgHandler::Entry<Video> Video::Handlers::commands[] =
		{
			{ IDC_VIDEO_DEVICE,            &Video::OnCmdDevice         },
			{ IDC_VIDEO_MODE,              &Video::OnCmdMode           },
			{ IDC_VIDEO_16_BIT,            &Video::OnCmdBitDepth       },
			{ IDC_VIDEO_32_BIT,            &Video::OnCmdBitDepth       },
			{ IDC_VIDEO_EFFECTS,           &Video::OnCmdFilter         },
			{ IDC_VIDEO_FILTER_SETTINGS,   &Video::OnCmdFilterSettings },
			{ IDC_VIDEO_PALETTE_AUTO,      &Video::OnCmdPalType        },
			{ IDC_VIDEO_PALETTE_YUV,       &Video::OnCmdPalType        },
			{ IDC_VIDEO_PALETTE_RGB,       &Video::OnCmdPalType        },
			{ IDC_VIDEO_PALETTE_CUSTOM,    &Video::OnCmdPalType        },
			{ IDC_VIDEO_COLORS_ADVANCED,   &Video::OnCmdColorsAdvanced },
			{ IDC_VIDEO_COLORS_RESET,      &Video::OnCmdColorsReset    },
			{ IDC_VIDEO_PALETTE_BROWSE,    &Video::OnCmdPalBrowse      },
			{ IDC_VIDEO_PALETTE_CLEAR,     &Video::OnCmdPalClear       },
			{ IDC_VIDEO_PALETTE_EDITOR,    &Video::OnCmdPalEditor      },
			{ IDC_VIDEO_DEFAULT,           &Video::OnCmdDefault        },
			{ IDOK,                        &Video::OnCmdOk             }
		};

		Video::DefaultMode::DefaultMode()
		{
			if (::GetSystemMetrics( SM_CXSCREEN ) == DEFAULT_16_9_WIDTH && ::GetSystemMetrics( SM_CYSCREEN ) == DEFAULT_16_9_HEIGHT)
			{
				width = DEFAULT_16_9_WIDTH;
				height = DEFAULT_16_9_HEIGHT;
			}
			else
			{
				width = DEFAULT_4_3_WIDTH;
				height = DEFAULT_4_3_HEIGHT;
			}
		}

		const Video::DefaultMode Video::defaultMode;

		class Video::Settings::Backup
		{
			const Adapters::const_iterator adapter;
			const Modes::const_iterator mode;
			Filter::Settings* const filter;
			Filter::Settings filters[Filter::NUM_TYPES];
			const short screenCurvature;
			const bool autoPalette;
			const schar brightness;
			const schar saturation;
			const schar contrast;
			const schar hue;
			const schar sharpness;
			const schar colorResolution;
			const schar colorBleed;
			const schar colorArtifacts;
			const schar colorFringing;
			const Nes::Video::Decoder decoder;
			const Path palettePath;
			const Nes::Video::Palette::Mode paletteMode;
			const Nes::Video::Palette::CustomType paletteCustomType;
			uchar paletteData[Nes::Video::Palette::NUM_ENTRIES_EXT][3];

		public:

			Backup(const Settings& settings,Nes::Video nes)
			:
			adapter           (settings.adapter),
			mode              (settings.mode),
			filter            (settings.filter),
			screenCurvature   (settings.screenCurvature),
			autoPalette       (settings.autoPalette),
			brightness        (nes.GetBrightness()),
			saturation        (nes.GetSaturation()),
			contrast          (nes.GetContrast()),
			hue               (nes.GetHue()),
			sharpness         (nes.GetSharpness()),
			colorResolution   (nes.GetColorResolution()),
			colorBleed        (nes.GetColorBleed()),
			colorArtifacts    (nes.GetColorArtifacts()),
			colorFringing     (nes.GetColorFringing()),
			decoder           (nes.GetDecoder()),
			palettePath       (settings.palette),
			paletteMode       (nes.GetPalette().GetMode()),
			paletteCustomType (nes.GetPalette().GetCustomType())
			{
				for (uint i=0; i < Filter::NUM_TYPES; ++i)
					filters[i] = settings.filters[i];

				if (paletteMode == Nes::Video::Palette::MODE_CUSTOM)
					nes.GetPalette().GetCustom( paletteData, paletteCustomType );
			}

			void Restore(Settings& settings,Nes::Video nes) const
			{
				settings.adapter = adapter;
				settings.mode = mode;
				settings.filter = filter;

				for (uint i=0; i < Filter::NUM_TYPES; ++i)
					settings.filters[i] = filters[i];

				settings.screenCurvature = screenCurvature;
				settings.palette = palettePath;
				settings.autoPalette = autoPalette;

				nes.SetBrightness( brightness );
				nes.SetSaturation( saturation );
				nes.SetContrast( contrast );
				nes.SetHue( hue );
				nes.SetSharpness( sharpness );
				nes.SetColorResolution( colorResolution );
				nes.SetColorBleed( colorBleed );
				nes.SetColorArtifacts( colorArtifacts );
				nes.SetColorFringing( colorFringing );
				nes.SetDecoder( decoder );
				nes.GetPalette().SetMode( paletteMode );

				if (paletteMode == Nes::Video::Palette::MODE_CUSTOM)
					nes.GetPalette().SetCustom( paletteData, paletteCustomType );
			}
		};

		Video::Settings::Settings()
		: fullscreenScale(SCREEN_MATCHED), backup(NULL) {}

		Video::Settings::~Settings()
		{
			delete backup;
		}

		Video::Video
		(
			Managers::Emulator& emulator,
			const Adapters& a,
			const Managers::Paths& p,
			const Configuration& cfg
		)
		:
		adapters ( a ),
		nes      ( emulator ),
		dialog   ( IDD_VIDEO, this, Handlers::messages, Handlers::commands ),
		paths    ( p )
		{
			if (cfg["view"]["size"]["fullscreen"].Str() == L"stretched")
				settings.fullscreenScale = SCREEN_STRETCHED;

			Configuration::ConstSection video( cfg["video"] );

			settings.adapter = std::find
			(
				adapters.begin(),
				adapters.end(),
				System::Guid( video["device"].Str() )
			);

			if (settings.adapter == adapters.end())
				settings.adapter = adapters.begin();

			{
				Configuration::ConstSection fullscreen( video["fullscreen"] );

				settings.mode = settings.adapter->modes.find
				(
					Mode
					(
						fullscreen[ "width"  ].Int(),
						fullscreen[ "height" ].Int(),
						fullscreen[ "bpp"    ].Int()
					)
				);
			}

			if (settings.mode == settings.adapter->modes.end())
				settings.mode = GetDefaultMode();

			settings.texMem =
			(
				video["memory-pool"].Str() == L"system" ? Settings::TEXMEM_SYSMEM :
                                                          Settings::TEXMEM_VIDMEM
			);

			{
				Configuration::ConstSection region( video["region"] );

				settings.rects.ntsc.Set
				(
					region[ "ntsc" ][ "left"   ].Int( 0                  ),
					region[ "ntsc" ][ "top"    ].Int( NTSC_CLIP_TOP      ),
					region[ "ntsc" ][ "right"  ].Int( NES_WIDTH-1        ),
					region[ "ntsc" ][ "bottom" ].Int( NTSC_CLIP_BOTTOM-1 )
				);

				settings.rects.pal.Set
				(
					region[ "pal" ][ "left"   ].Int( 0                 ),
					region[ "pal" ][ "top"    ].Int( PAL_CLIP_TOP      ),
					region[ "pal" ][ "right"  ].Int( NES_WIDTH-1       ),
					region[ "pal" ][ "bottom" ].Int( PAL_CLIP_BOTTOM-1 )
				);
			}

			ValidateRects();

			{
				Configuration::ConstSection colors( video["colors"] );

				uint v;

				if (200 >= (v=colors["brightness"].Int( 100 )))
					Nes::Video(nes).SetBrightness( int(v) - 100 );

				if (200 >= (v=colors["saturation"].Int( 100 )))
					Nes::Video(nes).SetSaturation( int(v) - 100 );

				if (200 >= (v=colors["contrast"].Int( 100 )))
					Nes::Video(nes).SetContrast( int(v) - 100 );

				int hue = colors["hue"].Int( 0 );

				if (hue > 180 && hue <= 360)
					hue -= 360;

				if (hue >= Nes::Video::MIN_HUE && hue <= Nes::Video::MAX_HUE)
					Nes::Video(nes).SetHue( hue );
			}

			{
				settings.autoPalette = false;
				settings.palette = video["palette"]["file"].Str();

				Nes::Video::Palette::Mode mode;
				const GenericString type( video["palette"]["type"].Str() );

				if (type == L"yuv")
				{
					mode = Nes::Video::Palette::MODE_YUV;
				}
				else if (type == L"rgb")
				{
					mode = Nes::Video::Palette::MODE_RGB;
				}
				else if (type == L"custom" && settings.palette.Length())
				{
					mode = Nes::Video::Palette::MODE_CUSTOM;
				}
				else
				{
					settings.autoPalette = true;
					mode = Nes::Video::Palette::MODE_YUV;
				}

				Nes::Video(nes).GetPalette().SetMode( mode );
			}

			ImportPalette( settings.palette, Managers::Paths::QUIETLY );

			{
				int v;

				if (MAX_SCREEN_CURVATURE-MIN_SCREEN_CURVATURE >= (v=video["screen-curvature"].Int( MAX_SCREEN_CURVATURE )))
					v -= MAX_SCREEN_CURVATURE;

				if (v >= MIN_SCREEN_CURVATURE && v <= MAX_SCREEN_CURVATURE)
					settings.screenCurvature = v;
				else
					settings.screenCurvature = 0;
			}

			settings.autoHz = !video[ "auto-display-frequency" ].No();
			settings.tvAspect = video[ "tv-aspect-ratio" ].Yes();

			settings.filter = settings.filters + Filter::Load
			(
				cfg,
				settings.filters,
				Nes::Video(nes),
				settings.adapter->maxScreenSize,
				settings.adapter->filters & Adapter::FILTER_BILINEAR,
				settings.adapter->canDoScanlineEffect,
				GetDesiredPaletteMode()
			);

			VideoDecoder::Load( cfg, Nes::Video(nes) );

			UpdateFinalRects();
		}

		Video::~Video()
		{
		}

		void Video::Save(Configuration& cfg) const
		{
			cfg["view" ]["size"]["fullscreen"].Str() =
			(
				settings.fullscreenScale == SCREEN_STRETCHED ? "stretched" :
                                                               "matched"
			);

			Configuration::Section video( cfg["video"] );

			if (settings.adapter != adapters.end())
			{
				video["device"].Str() = settings.adapter->guid.GetString();

				if (settings.mode != settings.adapter->modes.end())
				{
					Configuration::Section fullscreen( video["fullscreen"] );

					fullscreen[ "width"  ].Int() = settings.mode->width;
					fullscreen[ "height" ].Int() = settings.mode->height;
					fullscreen[ "bpp"    ].Int() = settings.mode->bpp;
				}
			}

			video["memory-pool"].Str() =
			(
				settings.texMem == Settings::TEXMEM_SYSMEM ? "system" :
                                                             "video"
			);

			video[ "screen-curvature"       ].Int() = MAX_SCREEN_CURVATURE + settings.screenCurvature;
			video[ "auto-display-frequency" ].YesNo() = settings.autoHz;
			video[ "tv-aspect-ratio"        ].YesNo() = settings.tvAspect;

			{
				Configuration::Section palette( video["palette"] );

				palette["file"].Str() = settings.palette;

				GenericString type;

				if (settings.autoPalette)
				{
					type = L"auto";
				}
				else switch (Nes::Video(nes).GetPalette().GetMode())
				{
					case Nes::Video::Palette::MODE_YUV: type = L"yuv";    break;
					case Nes::Video::Palette::MODE_RGB: type = L"rgb";    break;
					default:                            type = L"custom"; break;
				}

				palette["type"].Str() = type;
			}

			{
				Configuration::Section region( video["region"] );

				region[ "ntsc" ][ "left"   ].Int() = settings.rects.ntsc.left;
				region[ "ntsc" ][ "top"    ].Int() = settings.rects.ntsc.top;
				region[ "ntsc" ][ "right"  ].Int() = settings.rects.ntsc.right - 1;
				region[ "ntsc" ][ "bottom" ].Int() = settings.rects.ntsc.bottom - 1;
				region[ "pal"  ][ "left"   ].Int() = settings.rects.pal.left;
				region[ "pal"  ][ "top"    ].Int() = settings.rects.pal.top;
				region[ "pal"  ][ "right"  ].Int() = settings.rects.pal.right - 1;
				region[ "pal"  ][ "bottom" ].Int() = settings.rects.pal.bottom - 1;
			}

			{
				Configuration::Section colors( video["colors"] );

				colors[ "brightness" ].Int() = 100 + Nes::Video(nes).GetBrightness();
				colors[ "saturation" ].Int() = 100 + Nes::Video(nes).GetSaturation();
				colors[ "contrast"   ].Int() = 100 + Nes::Video(nes).GetContrast();
				colors[ "hue"        ].Int() = Nes::Video(nes).GetHue() + (Nes::Video(nes).GetHue() < 0 ? 360 : 0);
			}

			Filter::Save
			(
				cfg,
				settings.filters,
				Nes::Video(nes),
				static_cast<Filter::Type>(settings.filter - settings.filters)
			);

			VideoDecoder::Save( cfg, Nes::Video(nes) );
		}

		Nes::Video::Palette::Mode Video::GetDesiredPaletteMode() const
		{
			if (Nes::Machine(nes).Is(Nes::Machine::VS) || Nes::Machine(nes).Is(Nes::Machine::PC10))
				return Nes::Video::Palette::MODE_RGB;
			else
				return Nes::Video::Palette::MODE_YUV;
		}

		void Video::UpdateAutoModes() const
		{
			const Nes::Video::Palette::Mode mode = GetDesiredPaletteMode();

			if (settings.autoPalette)
				Nes::Video(nes).GetPalette().SetMode( mode );

			VideoFilters::UpdateAutoModes( settings.filters, nes, mode );
		}

		uint Video::GetScanlines() const
		{
			if (settings.adapter->canDoScanlineEffect)
			{
				switch (settings.filter - settings.filters)
				{
					case Filter::TYPE_STD:

						return settings.filters[Filter::TYPE_STD].attributes[Filter::ATR_SCANLINES];

					case Filter::TYPE_NTSC:

						return settings.filters[Filter::TYPE_NTSC].attributes[Filter::ATR_SCANLINES];
				}
			}

			return 0;
		}

		const Rect Video::GetRenderState(Nes::Video::RenderState& state,const Point screen) const
		{
			typedef Nes::Video::RenderState State;

			Rect rect( Nes::Machine(nes).GetMode() == Nes::Machine::NTSC ? settings.rects.ntsc : settings.rects.pal );

			state.width = NES_WIDTH;
			state.height = NES_HEIGHT;
			state.filter = State::FILTER_NONE;

			uint scale = 1;

			switch (settings.filter - settings.filters)
			{
				case Filter::TYPE_STD:
					break;

				case Filter::TYPE_NTSC:

					if (settings.adapter->maxScreenSize.x >= NTSC_WIDTH)
					{
						rect.left  = (rect.left  * (NTSC_WIDTH - 4) + NES_WIDTH / 2) / NES_WIDTH + 3;
						rect.right = (rect.right * (NTSC_WIDTH - 4) + NES_WIDTH / 2) / NES_WIDTH + 3;

						state.width = NTSC_WIDTH;
						state.filter = State::FILTER_NTSC;
					}
					break;

				case Filter::TYPE_SCALEX:

					if (settings.adapter->maxScreenSize.x >= NES_WIDTH*2 && settings.adapter->maxScreenSize.y >= NES_HEIGHT*2)
					{
						int attribute = settings.filters[Filter::TYPE_SCALEX].attributes[Filter::ATR_TYPE];

						if (attribute == Filter::ATR_SCALEAX)
						{
							const Point nes( rect.Size() );

							if (screen.x >= nes.x*3 && screen.y >= nes.y*3)
							{
								attribute = Filter::ATR_SCALE3X;
							}
							else if (screen.x >= nes.x*2 && screen.y >= nes.y*2)
							{
								attribute = Filter::ATR_SCALE2X;
							}
						}

						switch (attribute)
						{
							case Filter::ATR_SCALE3X:

								if (settings.adapter->maxScreenSize.x >= NES_WIDTH*3 && settings.adapter->maxScreenSize.y >= NES_HEIGHT*3)
								{
									scale = 3;
									state.filter = State::FILTER_SCALE3X;
									break;
								}

							case Filter::ATR_SCALE2X:

								scale = 2;
								state.filter = State::FILTER_SCALE2X;
								break;
						}
					}
					break;

				case Filter::TYPE_HQX:

					if (settings.adapter->maxScreenSize.x >= NES_WIDTH*2 && settings.adapter->maxScreenSize.y >= NES_HEIGHT*2)
					{
						int attribute = settings.filters[Filter::TYPE_HQX].attributes[Filter::ATR_TYPE];

						if (attribute == Filter::ATR_HQAX)
						{
							const Point nes( rect.Size() );

							if (screen.x >= nes.x*4 && screen.y >= nes.y*4)
							{
								attribute = Filter::ATR_HQ4X;
							}
							else if (screen.x >= nes.x*3 && screen.y >= nes.y*3)
							{
								attribute = Filter::ATR_HQ3X;
							}
							else if (screen.x >= nes.x*2 && screen.y >= nes.y*2)
							{
								attribute = Filter::ATR_HQ2X;
							}
						}

						switch (attribute)
						{
							case Filter::ATR_HQ4X:

								if (settings.adapter->maxScreenSize.x >= NES_WIDTH*4 && settings.adapter->maxScreenSize.y >= NES_HEIGHT*4)
								{
									scale = 4;
									state.filter = State::FILTER_HQ4X;
									break;
								}

							case Filter::ATR_HQ3X:

								if (settings.adapter->maxScreenSize.x >= NES_WIDTH*3 && settings.adapter->maxScreenSize.y >= NES_HEIGHT*3)
								{
									scale = 3;
									state.filter = State::FILTER_HQ3X;
									break;
								}

							case Filter::ATR_HQ2X:

								scale = 2;
								state.filter = State::FILTER_HQ2X;
								break;
						}
					}
					break;

				case Filter::TYPE_2XSAI:

					if (settings.adapter->maxScreenSize.x >= NES_WIDTH*2 && settings.adapter->maxScreenSize.y >= NES_HEIGHT*2)
					{
						scale = 2;
						state.filter = State::FILTER_2XSAI;
					}
					break;
			}

			state.width = state.width * scale;
			state.height = state.height * scale;

			rect.left   *= scale;
			rect.top    *= scale;
			rect.right  *= scale;
			rect.bottom *= scale;

			return rect;
		}

		Video::Modes::const_iterator Video::GetDialogMode() const
		{
			for (Modes::const_iterator it(settings.adapter->modes.begin()), end(settings.adapter->modes.end()); it != end; ++it)
			{
				if (it->bpp == settings.mode->bpp && it->width >= MIN_DIALOG_WIDTH && it->height >= MIN_DIALOG_HEIGHT)
					return it;
			}

			return settings.adapter->modes.begin();
		}

		uint Video::GetFullscreenScaleMethod() const
		{
			return settings.filter == settings.filters+Filter::TYPE_NTSC ? settings.filters[Filter::TYPE_NTSC].attributes[Filter::ATR_RESCALE_PIC] ? 1 : 2 : 0;
		}

		void Video::UpdateFullscreenScaleMethod(uint prev)
		{
			if (settings.fullscreenScale != SCREEN_STRETCHED && prev != GetFullscreenScaleMethod())
				settings.fullscreenScale = SCREEN_MATCHED;
		}

		void Video::UpdateFinalRects()
		{
			for (uint i=0; i < 2; ++i)
			{
				Rect& rect = (i ? settings.rects.outPal : settings.rects.outNtsc);
				rect = (i ? settings.rects.pal : settings.rects.ntsc);

				if (settings.filter == settings.filters + Filter::TYPE_NTSC)
				{
					if (settings.tvAspect || !settings.filters[Filter::TYPE_NTSC].attributes[Filter::ATR_RESCALE_PIC])
					{
						rect.top *= 2;
						rect.bottom *= 2;
						rect.left  = (rect.left  * (NTSC_WIDTH - 4) + NES_WIDTH  / 2) / NES_WIDTH + 3;
						rect.right = (rect.right * (NTSC_WIDTH - 4) + NES_WIDTH  / 2) / NES_WIDTH + 3;
					}
				}
				else if (settings.tvAspect)
				{
					rect.left  = (rect.left  * TV_WIDTH + NES_WIDTH / 2) / NES_WIDTH;
					rect.right = (rect.right * TV_WIDTH + NES_WIDTH / 2) / NES_WIDTH;
				}
			}
		}

		void Video::ValidateRects()
		{
			Settings::Rects& r = settings.rects;

			r.ntsc.left   = NST_CLAMP( r.ntsc.left,   0,           NES_WIDTH  -1 );
			r.ntsc.top    = NST_CLAMP( r.ntsc.top,    0,           NES_HEIGHT -1 );
			r.ntsc.right  = NST_CLAMP( r.ntsc.right,  r.ntsc.left, NES_WIDTH  -1 ) + 1;
			r.ntsc.bottom = NST_CLAMP( r.ntsc.bottom, r.ntsc.top,  NES_HEIGHT -1 ) + 1;
			r.pal.left    = NST_CLAMP( r.pal.left,    0,           NES_WIDTH  -1 );
			r.pal.top     = NST_CLAMP( r.pal.top,     0,           NES_HEIGHT -1 );
			r.pal.right   = NST_CLAMP( r.pal.right,   r.pal.left,  NES_WIDTH  -1 ) + 1;
			r.pal.bottom  = NST_CLAMP( r.pal.bottom,  r.pal.top,   NES_HEIGHT -1 ) + 1;
		}

		void Video::ResetColors()
		{
			Nes::Video(nes).SetBrightness ( Nes::Video::DEFAULT_BRIGHTNESS );
			Nes::Video(nes).SetSaturation ( Nes::Video::DEFAULT_SATURATION );
			Nes::Video(nes).SetContrast   ( Nes::Video::DEFAULT_CONTRAST   );
			Nes::Video(nes).SetHue        ( Nes::Video::DEFAULT_HUE        );
		}

		Video::Modes::const_iterator Video::GetDefaultMode() const
		{
			for (uint bpp=16; bpp <= 32; bpp += 16)
			{
				const Modes::const_iterator it( settings.adapter->modes.find(Mode(defaultMode.width,defaultMode.height,bpp)) );

				if (it != settings.adapter->modes.end())
					return it;
			}

			return settings.adapter->modes.begin();
		}

		ibool Video::OnInitDialog(Param&)
		{
			NST_ASSERT( settings.backup == NULL );
			settings.backup = new Settings::Backup( settings, Nes::Video(nes) );

			{
				const Control::ComboBox comboBox( dialog.ComboBox(IDC_VIDEO_DEVICE) );

				for (Adapters::const_iterator it(adapters.begin()), end(adapters.end()); it != end; ++it)
				{
					comboBox.Add( it->name.Ptr() );

					if (settings.adapter == it)
						comboBox[it - adapters.begin()].Select();
				}
			}

			for (uint i=IDC_VIDEO_NTSC_LEFT; i <= IDC_VIDEO_PAL_BOTTOM; ++i)
				dialog.Edit( i ).Limit( 3 );

			dialog.Slider( IDC_VIDEO_COLORS_BRIGHTNESS ).SetRange( 0, Nes::Video::MAX_BRIGHTNESS-Nes::Video::MIN_BRIGHTNESS );
			dialog.Slider( IDC_VIDEO_COLORS_SATURATION ).SetRange( 0, Nes::Video::MAX_SATURATION-Nes::Video::MIN_SATURATION );
			dialog.Slider( IDC_VIDEO_COLORS_CONTRAST   ).SetRange( 0, Nes::Video::MAX_CONTRAST-Nes::Video::MIN_CONTRAST );
			dialog.Slider( IDC_VIDEO_COLORS_HUE        ).SetRange( 0, Nes::Video::MAX_HUE-Nes::Video::MIN_HUE );

			dialog.Slider( IDC_VIDEO_SC_SLIDER ).SetRange( 0, MAX_SCREEN_CURVATURE-MIN_SCREEN_CURVATURE );

			dialog.CheckBox( IDC_VIDEO_AUTO_HZ ).Check( settings.autoHz );

			dialog.RadioButton( settings.texMem == Settings::TEXMEM_VIDMEM ? IDC_VIDEO_NESTEXTURE_VIDMEM : IDC_VIDEO_NESTEXTURE_SYSMEM ).Check();

			UpdateDevice( *settings.mode );
			UpdateRects( settings.rects.ntsc, settings.rects.pal );
			UpdateColors();
			UpdateScreenCurvature();
			UpdatePalette();

			return true;
		}

		ibool Video::OnDestroy(Param&)
		{
			if (settings.backup)
			{
				settings.backup->Restore( settings, Nes::Video(nes) );

				delete settings.backup;
				settings.backup = NULL;
			}
			else
			{
				settings.autoHz = dialog.CheckBox(IDC_VIDEO_AUTO_HZ).Checked();

				if (dialog.RadioButton(IDC_VIDEO_NESTEXTURE_VIDMEM).Checked())
					settings.texMem = Settings::TEXMEM_VIDMEM;
				else
					settings.texMem = Settings::TEXMEM_SYSMEM;

				dialog.Control( IDC_VIDEO_NTSC_LEFT   ).Text() >> settings.rects.ntsc.left;
				dialog.Control( IDC_VIDEO_NTSC_TOP    ).Text() >> settings.rects.ntsc.top;
				dialog.Control( IDC_VIDEO_NTSC_RIGHT  ).Text() >> settings.rects.ntsc.right;
				dialog.Control( IDC_VIDEO_NTSC_BOTTOM ).Text() >> settings.rects.ntsc.bottom;
				dialog.Control( IDC_VIDEO_PAL_LEFT    ).Text() >> settings.rects.pal.left;
				dialog.Control( IDC_VIDEO_PAL_TOP     ).Text() >> settings.rects.pal.top;
				dialog.Control( IDC_VIDEO_PAL_RIGHT   ).Text() >> settings.rects.pal.right;
				dialog.Control( IDC_VIDEO_PAL_BOTTOM  ).Text() >> settings.rects.pal.bottom;

				ValidateRects();
			}

			UpdateFinalRects();

			return true;
		}

		ibool Video::OnHScroll(Param& param)
		{
			const int value = param.Slider().Scroll();

			switch (param.Slider().GetId())
			{
				case IDC_VIDEO_COLORS_BRIGHTNESS:

					Nes::Video(nes).SetBrightness( value + Nes::Video::MIN_BRIGHTNESS );
					break;

				case IDC_VIDEO_COLORS_SATURATION:

					Nes::Video(nes).SetSaturation( value + Nes::Video::MIN_SATURATION );
					break;

				case IDC_VIDEO_COLORS_CONTRAST:

					Nes::Video(nes).SetContrast( value + Nes::Video::MIN_CONTRAST );
					break;

				case IDC_VIDEO_COLORS_HUE:

					Nes::Video(nes).SetHue( value + Nes::Video::MIN_HUE );
					break;

				case IDC_VIDEO_SC_SLIDER:

					settings.screenCurvature = value + MIN_SCREEN_CURVATURE;
					UpdateScreenCurvature();
					Application::Instance::GetMainWindow().Redraw();

				default:

					return true;
			}

			UpdateColors();
			Application::Instance::GetMainWindow().Redraw();

			return true;
		}

		ibool Video::OnCmdDevice(Param& param)
		{
			if (param.ComboBox().SelectionChanged())
			{
				settings.adapter = adapters.begin();

				for (uint i=dialog.ComboBox( IDC_VIDEO_DEVICE ).Selection().GetIndex(); i; --i)
					++settings.adapter;

				UpdateDevice( *settings.mode );
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool Video::OnCmdBitDepth(Param& param)
		{
			if (param.Button().Clicked())
				UpdateResolutions( Mode(settings.mode->width,settings.mode->height,param.Button().GetId() == IDC_VIDEO_32_BIT ? 32 : 16) );

			return true;
		}

		ibool Video::OnCmdMode(Param& param)
		{
			if (param.ComboBox().SelectionChanged())
			{
				settings.mode = settings.adapter->modes.begin();

				for (uint i=dialog.ComboBox(IDC_VIDEO_MODE).Selection().Data(); i; --i)
					++settings.mode;
			}

			return true;
		}

		ibool Video::OnCmdFilter(Param& param)
		{
			if (param.ComboBox().SelectionChanged())
			{
				const uint method = GetFullscreenScaleMethod();
				settings.filter = settings.filters + dialog.ComboBox( IDC_VIDEO_EFFECTS ).Selection().Data();
				UpdateFullscreenScaleMethod( method );

				UpdatePalette();
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool Video::OnCmdFilterSettings(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint method = GetFullscreenScaleMethod();

				static const ushort idd[] =
				{
					IDD_VIDEO_FILTER_STD,
					IDD_VIDEO_FILTER_NTSC,
					IDD_VIDEO_FILTER_SCALEX,
					IDD_VIDEO_FILTER_HQX,
					IDD_VIDEO_FILTER_2XSAI
				};

				VideoFilters
				(
					Nes::Video(nes),
					idd[settings.filter-settings.filters],
					*settings.filter,
					settings.adapter->maxScreenSize,
					settings.adapter->filters & Adapter::FILTER_BILINEAR,
					settings.adapter->canDoScanlineEffect,
					GetDesiredPaletteMode()
				).Open();

				UpdateFullscreenScaleMethod( method );
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool Video::OnCmdPalType(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint cmd = param.Button().GetId();

				settings.autoPalette = (cmd == IDC_VIDEO_PALETTE_AUTO);

				Nes::Video(nes).GetPalette().SetMode
				(
					cmd == IDC_VIDEO_PALETTE_YUV    ? Nes::Video::Palette::MODE_YUV :
					cmd == IDC_VIDEO_PALETTE_RGB    ? Nes::Video::Palette::MODE_RGB :
					cmd == IDC_VIDEO_PALETTE_CUSTOM ? Nes::Video::Palette::MODE_CUSTOM :
                                                      GetDesiredPaletteMode()
				);

				UpdatePalette();
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool Video::OnCmdColorsReset(Param& param)
		{
			if (param.Button().Clicked())
			{
				ResetColors();
				UpdateColors();
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool Video::OnCmdColorsAdvanced(Param& param)
		{
			if (param.Button().Clicked())
			{
				VideoDecoder( nes ).Open();
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool Video::OnCmdPalBrowse(Param& param)
		{
			if (param.Button().Clicked())
			{
				const Path file
				(
					paths.BrowseLoad
					(
						Managers::Paths::File::PALETTE|Managers::Paths::File::ARCHIVE,
						Application::Instance::GetFullPath( settings.palette )
					)
				);

				if (file.Length())
				{
					settings.palette = file;

					ImportPalette( settings.palette, Managers::Paths::NOISY );
					UpdatePalette();
					Application::Instance::GetMainWindow().Redraw();
				}
			}

			return true;
		}

		ibool Video::OnCmdPalClear(Param& param)
		{
			if (param.Button().Clicked())
			{
				if (settings.palette.Length())
				{
					settings.palette.Destroy();

					if (Nes::Video(nes).GetPalette().GetMode() == Nes::Video::Palette::MODE_CUSTOM)
					{
						Nes::Video(nes).GetPalette().SetMode( GetDesiredPaletteMode() );
						settings.autoPalette = true;
					}

					UpdatePalette();
					Application::Instance::GetMainWindow().Redraw();
				}
			}

			return true;
		}

		ibool Video::OnCmdPalEditor(Param& param)
		{
			if (param.Button().Clicked())
			{
				Nes::Video video( nes );

				if (settings.palette.Empty())
					video.GetPalette().ResetCustom();

				{
					const Path path( PaletteEditor( video, paths, settings.palette ).Open() );

					if (settings.palette.Empty() && path.Length())
						settings.palette = path;
				}

				if (settings.palette.Length())
				{
					ImportPalette( settings.palette, Managers::Paths::QUIETLY );
					UpdatePalette();
				}

				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool Video::OnCmdDefault(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint method = GetFullscreenScaleMethod();

				settings.adapter = adapters.begin();
				settings.mode = GetDefaultMode();

				for (uint i=0; i < Filter::NUM_TYPES; ++i)
					settings.filters[i].Reset();

				settings.filter = settings.filters + Filter::TYPE_STD;

				UpdateDevice( *settings.mode );

				dialog.RadioButton( IDC_VIDEO_NESTEXTURE_VIDMEM ).Check();
				dialog.RadioButton( IDC_VIDEO_NESTEXTURE_SYSMEM ).Uncheck();

				UpdateRects
				(
					Rect( 0, NTSC_CLIP_TOP, NES_WIDTH, NTSC_CLIP_BOTTOM ),
					Rect( 0, PAL_CLIP_TOP, NES_WIDTH, PAL_CLIP_BOTTOM )
				);

				dialog.CheckBox( IDC_VIDEO_AUTO_HZ ).Check();

				ResetColors();
				UpdateColors();

				settings.autoPalette = true;

				settings.screenCurvature = 0;
				UpdateScreenCurvature();

				const Nes::Video::Palette::Mode mode = GetDesiredPaletteMode();
				Nes::Video video(nes);

				video.GetPalette().SetMode( mode );

				VideoFilters::UpdateAutoModes( settings.filters, video, mode );

				video.SetDecoder( Nes::Video::DECODER_CANONICAL );

				UpdateFullscreenScaleMethod( method );

				UpdatePalette();
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool Video::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				delete settings.backup;
				settings.backup = NULL;

				dialog.Close();
			}

			return true;
		}

		void Video::UpdateDevice(Mode mode)
		{
			dialog.RadioButton( IDC_VIDEO_NESTEXTURE_VIDMEM ).Enable( settings.adapter->videoMemScreen );
			dialog.RadioButton( IDC_VIDEO_NESTEXTURE_SYSMEM ).Enable( settings.adapter->videoMemScreen );

			uint available = 0;

			for (Modes::const_iterator it(settings.adapter->modes.begin()), end(settings.adapter->modes.end()); it != end; ++it)
			{
				switch (it->bpp)
				{
					case 16: available |= 16; break;
					case 32: available |= 32; break;
				}

				if (available == (16|32))
					break;
			}

			NST_ASSERT( available & (16|32) );

			dialog.Control( IDC_VIDEO_16_BIT ).Enable( available & 16 );
			dialog.Control( IDC_VIDEO_32_BIT ).Enable( available & 32 );

			switch (mode.bpp)
			{
				case 32: mode.bpp = ((available & 32) ? 32 : 16); break;
				case 16: mode.bpp = ((available & 16) ? 16 : 32); break;
			}

			UpdateResolutions( mode );
			UpdateFilters();
		}

		void Video::UpdateResolutions(Mode mode)
		{
			settings.mode = settings.adapter->modes.find( mode );

			if (settings.mode == settings.adapter->modes.end())
			{
				settings.mode = settings.adapter->modes.find( Mode(defaultMode.width,defaultMode.height,mode.bpp) );

				if (settings.mode == settings.adapter->modes.end())
					settings.mode = settings.adapter->modes.begin();

				mode = *settings.mode;
			}

			dialog.RadioButton( IDC_VIDEO_16_BIT ).Check( mode.bpp == 16 );
			dialog.RadioButton( IDC_VIDEO_32_BIT ).Check( mode.bpp == 32 );

			const Control::ComboBox comboBox( dialog.ComboBox(IDC_VIDEO_MODE) );
			comboBox.Clear();

			uint idx=0;
			HeapString string;

			for (Modes::const_iterator it(settings.adapter->modes.begin()), end(settings.adapter->modes.end()); it != end; ++it, ++idx)
			{
				if (mode.bpp == it->bpp)
				{
					string.Clear();
					comboBox.Add( (string << it->width << 'x' << it->height).Ptr() ).Data() = idx;

					if (mode.width == it->width && mode.height == it->height)
						comboBox[comboBox.Size() - 1].Select();
				}
			}
		}

		void Video::UpdateFilters()
		{
			const Control::ComboBox comboBox( dialog.ComboBox(IDC_VIDEO_EFFECTS) );
			comboBox.Clear();

			comboBox.Add( Resource::String(IDS_VIDEO_FILTER_STD) ).Data() = Filter::TYPE_STD;

			if (settings.adapter->maxScreenSize.x >= NTSC_WIDTH)
				comboBox.Add( Resource::String(IDS_VIDEO_FILTER_NTSC) ).Data() = Filter::TYPE_NTSC;

			if
			(
				settings.adapter->maxScreenSize.x >= NES_WIDTH*2 &&
				settings.adapter->maxScreenSize.y >= NES_HEIGHT*2
			)
			{

				comboBox.Add( Resource::String(IDS_VIDEO_FILTER_SCALEX) ).Data() = Filter::TYPE_SCALEX;
				comboBox.Add( Resource::String(IDS_VIDEO_FILTER_HQX) ).Data() = Filter::TYPE_HQX;
				comboBox.Add( Resource::String(IDS_VIDEO_FILTER_2XSAI) ).Data() = Filter::TYPE_2XSAI;
			}

			for (uint i=0, size=comboBox.Size(); i < size; ++i)
			{
				if (settings.filter-settings.filters == (Filter::Type) (Control::ComboBox::Value) comboBox[i].Data())
				{
					comboBox[i].Select();
					return;
				}
			}

			settings.filter = settings.filters + Filter::TYPE_STD;
			comboBox[0].Select();
		}

		void Video::UpdateColors() const
		{
			const int colors[] =
			{
				Nes::Video(nes).GetBrightness(),
				Nes::Video(nes).GetSaturation(),
				Nes::Video(nes).GetContrast(),
				Nes::Video(nes).GetHue()
			};

			dialog.Control( IDC_VIDEO_COLORS_RESET ).Enable
			(
				colors[0] != Nes::Video::DEFAULT_BRIGHTNESS ||
				colors[1] != Nes::Video::DEFAULT_SATURATION ||
				colors[2] != Nes::Video::DEFAULT_CONTRAST   ||
				colors[3] != Nes::Video::DEFAULT_HUE
			);

			dialog.Slider( IDC_VIDEO_COLORS_BRIGHTNESS ).Position() = colors[0] - Nes::Video::MIN_BRIGHTNESS;
			dialog.Slider( IDC_VIDEO_COLORS_SATURATION ).Position() = colors[1] - Nes::Video::MIN_SATURATION;
			dialog.Slider( IDC_VIDEO_COLORS_CONTRAST   ).Position() = colors[2] - Nes::Video::MIN_CONTRAST;
			dialog.Slider( IDC_VIDEO_COLORS_HUE        ).Position() = colors[3] - Nes::Video::MIN_HUE;

			dialog.Control( IDC_VIDEO_COLORS_BRIGHTNESS_VAL ).Text() << RealString( colors[0] / 100.0, 2 ).Ptr();
			dialog.Control( IDC_VIDEO_COLORS_SATURATION_VAL ).Text() << RealString( (colors[1] + 100) / 100.0, 2 ).Ptr();
			dialog.Control( IDC_VIDEO_COLORS_CONTRAST_VAL   ).Text() << RealString( (colors[2] + 100) / 100.0, 2 ).Ptr();
			dialog.Control( IDC_VIDEO_COLORS_HUE_VAL        ).Text() << colors[3];
		}

		void Video::UpdateScreenCurvature() const
		{
			dialog.Slider( IDC_VIDEO_SC_SLIDER ).Position() = settings.screenCurvature - MIN_SCREEN_CURVATURE;
			dialog.Control( IDC_VIDEO_SC_VALUE ).Text() << RealString( settings.screenCurvature / 10.0 ).Ptr();
		}

		void Video::UpdateRects(const Rect& ntsc,const Rect& pal) const
		{
			dialog.Control( IDC_VIDEO_NTSC_LEFT   ).Text() << uint( ntsc.left       );
			dialog.Control( IDC_VIDEO_NTSC_TOP    ).Text() << uint( ntsc.top        );
			dialog.Control( IDC_VIDEO_NTSC_RIGHT  ).Text() << uint( ntsc.right - 1  );
			dialog.Control( IDC_VIDEO_NTSC_BOTTOM ).Text() << uint( ntsc.bottom - 1 );
			dialog.Control( IDC_VIDEO_PAL_LEFT    ).Text() << uint( pal.left        );
			dialog.Control( IDC_VIDEO_PAL_TOP     ).Text() << uint( pal.top         );
			dialog.Control( IDC_VIDEO_PAL_RIGHT   ).Text() << uint( pal.right - 1   );
			dialog.Control( IDC_VIDEO_PAL_BOTTOM  ).Text() << uint( pal.bottom - 1  );
		}

		void Video::UpdatePalette() const
		{
			const Nes::Video::Palette::Mode mode = Nes::Video(nes).GetPalette().GetMode();

			dialog.Control( IDC_VIDEO_PALETTE_CLEAR  ).Enable( settings.palette.Length() );
			dialog.Control( IDC_VIDEO_PALETTE_CUSTOM ).Enable( settings.palette.Length() );
			dialog.Control( IDC_VIDEO_PALETTE_PATH   ).Enable( mode == Nes::Video::Palette::MODE_CUSTOM );

			dialog.Edit( IDC_VIDEO_PALETTE_PATH ) << settings.palette.Ptr();

			dialog.Control( IDC_VIDEO_COLORS_ADVANCED ).Enable( mode == Nes::Video::Palette::MODE_YUV );

			dialog.RadioButton( IDC_VIDEO_PALETTE_AUTO   ).Check( settings.autoPalette );
			dialog.RadioButton( IDC_VIDEO_PALETTE_YUV    ).Check( !settings.autoPalette && mode == Nes::Video::Palette::MODE_YUV    );
			dialog.RadioButton( IDC_VIDEO_PALETTE_RGB    ).Check( !settings.autoPalette && mode == Nes::Video::Palette::MODE_RGB    );
			dialog.RadioButton( IDC_VIDEO_PALETTE_CUSTOM ).Check( !settings.autoPalette && mode == Nes::Video::Palette::MODE_CUSTOM );
		}

		void Video::ImportPalette(Path& palette,Managers::Paths::Alert alert)
		{
			if (palette.Empty())
				return;

			palette.MakePretty();

			Managers::Paths::File file;

			if (paths.Load( file, Managers::Paths::File::PALETTE|Managers::Paths::File::ARCHIVE, Application::Instance::GetFullPath(palette), alert ))
			{
				if (file.data.Size() >= Nes::Video::Palette::NUM_ENTRIES*3)
				{
					Nes::Video::Palette::CustomType type = Nes::Video::Palette::STD_PALETTE;

					if (file.data.Size() >= Nes::Video::Palette::NUM_ENTRIES_EXT*3)
						type = Nes::Video::Palette::EXT_PALETTE;

					if (NES_SUCCEEDED(Nes::Video(nes).GetPalette().SetCustom( reinterpret_cast<Nes::Video::Palette::Colors>(file.data.Begin()), type )))
						return;
				}
			}

			if (alert == Managers::Paths::QUIETLY)
				Io::Log() << "Video: warning, custom palette file: \"" << palette << "\" invalid or not found!\r\n";
			else
				User::Fail( IDS_DIALOG_VIDEO_INVALID_PALETTE );

			palette.Destroy();
			Nes::Video(nes).GetPalette().SetMode( GetDesiredPaletteMode() );
		}
	}
}
