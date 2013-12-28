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

#include "NstApplicationInstance.hpp"
#include "NstManagerEmulator.hpp"
#include "NstWindowParam.hpp"
#include "NstDialogVideoFilters.hpp"

namespace Nestopia
{
	namespace Window
	{
		NST_COMPILE_ASSERT
		(
			IDC_VIDEO_FILTER_NTSC_RESOLUTION_SLIDER == IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER + 1 &&
			IDC_VIDEO_FILTER_NTSC_COLORBLEED_SLIDER == IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER + 2 &&
			IDC_VIDEO_FILTER_NTSC_ARTIFACTS_SLIDER  == IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER + 3 &&
			IDC_VIDEO_FILTER_NTSC_FRINGING_SLIDER   == IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER + 4 &&

			IDC_VIDEO_FILTER_NTSC_RESOLUTION_VAL == IDC_VIDEO_FILTER_NTSC_SHARPNESS_VAL + 1 &&
			IDC_VIDEO_FILTER_NTSC_COLORBLEED_VAL == IDC_VIDEO_FILTER_NTSC_SHARPNESS_VAL + 2 &&
			IDC_VIDEO_FILTER_NTSC_ARTIFACTS_VAL  == IDC_VIDEO_FILTER_NTSC_SHARPNESS_VAL + 3 &&
			IDC_VIDEO_FILTER_NTSC_FRINGING_VAL   == IDC_VIDEO_FILTER_NTSC_SHARPNESS_VAL + 4 &&

			IDC_VIDEO_FILTER_NTSC_RESOLUTION_TEXT == IDC_VIDEO_FILTER_NTSC_SHARPNESS_TEXT + 1 &&
			IDC_VIDEO_FILTER_NTSC_COLORBLEED_TEXT == IDC_VIDEO_FILTER_NTSC_SHARPNESS_TEXT + 2 &&
			IDC_VIDEO_FILTER_NTSC_ARTIFACTS_TEXT  == IDC_VIDEO_FILTER_NTSC_SHARPNESS_TEXT + 3 &&
			IDC_VIDEO_FILTER_NTSC_FRINGING_TEXT   == IDC_VIDEO_FILTER_NTSC_SHARPNESS_TEXT + 4 &&

			IDC_VIDEO_FILTER_NTSC_SVIDEO == IDC_VIDEO_FILTER_NTSC_COMPOSITE + 1 &&
			IDC_VIDEO_FILTER_NTSC_RGB    == IDC_VIDEO_FILTER_NTSC_COMPOSITE + 2
		);

		struct VideoFilters::Handlers
		{
			static const MsgHandler::Entry<VideoFilters> messages[];
			static const MsgHandler::Entry<VideoFilters> commands[];
		};

		const MsgHandler::Entry<VideoFilters> VideoFilters::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &VideoFilters::OnInitDialog },
			{ WM_HSCROLL,    &VideoFilters::OnHScroll    },
			{ WM_DESTROY,    &VideoFilters::OnDestroy    }
		};

		const MsgHandler::Entry<VideoFilters> VideoFilters::Handlers::commands[] =
		{
			{ IDC_VIDEO_FILTER_BILINEAR,              &VideoFilters::OnCmdBilinear   },
			{ IDC_VIDEO_FILTER_NTSC_AUTO_TUNING,      &VideoFilters::OnCmdNtscTuning },
			{ IDC_VIDEO_FILTER_NTSC_COMPOSITE,        &VideoFilters::OnCmdNtscCable  },
			{ IDC_VIDEO_FILTER_NTSC_SVIDEO,           &VideoFilters::OnCmdNtscCable  },
			{ IDC_VIDEO_FILTER_NTSC_RGB,              &VideoFilters::OnCmdNtscCable  },
			{ IDC_VIDEO_FILTER_SCALEX_AUTO,           &VideoFilters::OnCmdScaleX     },
			{ IDC_VIDEO_FILTER_SCALEX_2X,             &VideoFilters::OnCmdScaleX     },
			{ IDC_VIDEO_FILTER_SCALEX_3X,             &VideoFilters::OnCmdScaleX     },
			{ IDC_VIDEO_FILTER_HQX_SCALING_AUTO,      &VideoFilters::OnCmdHqX        },
			{ IDC_VIDEO_FILTER_HQX_SCALING_2X,        &VideoFilters::OnCmdHqX        },
			{ IDC_VIDEO_FILTER_HQX_SCALING_3X,        &VideoFilters::OnCmdHqX        },
			{ IDC_VIDEO_FILTER_HQX_SCALING_4X,        &VideoFilters::OnCmdHqX        },
			{ IDC_VIDEO_FILTER_XBR_SCALING_AUTO,      &VideoFilters::OnCmdxBR        },
			{ IDC_VIDEO_FILTER_XBR_SCALING_2X,        &VideoFilters::OnCmdxBR        },
			{ IDC_VIDEO_FILTER_XBR_SCALING_3X,        &VideoFilters::OnCmdxBR        },
			{ IDC_VIDEO_FILTER_XBR_SCALING_4X,        &VideoFilters::OnCmdxBR        },
			{ IDC_VIDEO_FILTER_XBR_ROUNDING_ALL,      &VideoFilters::OnCmdxBRRound   },
			{ IDC_VIDEO_FILTER_XBR_ROUNDING_SOME,     &VideoFilters::OnCmdxBRRound   },
			{ IDC_VIDEO_FILTER_XBR_ROUNDING_NONE,     &VideoFilters::OnCmdxBRRound   },
			{ IDC_VIDEO_FILTER_BLEND,				  &VideoFilters::OnCmdAlpha		 },
			{ IDC_VIDEO_FILTER_DEFAULT,               &VideoFilters::OnCmdDefault    },
			{ IDOK,                                   &VideoFilters::OnCmdOk         }
		};

		void VideoFilters::Settings::Reset()
		{
			for (uint i=0; i < sizeof(array(attributes)); ++i)
				attributes[i] = 0;
		}

		VideoFilters::Backup::Backup(const Settings& s,const Nes::Video nes)
		:
		settings   ( s ),
		sharpness  ( nes.GetSharpness() ),
		resolution ( nes.GetColorResolution() ),
		bleed      ( nes.GetColorBleed() ),
		artifacts  ( nes.GetColorArtifacts() ),
		fringing   ( nes.GetColorFringing() ),
		corner_rounding ( nes.GetCornerRounding() ),
		blend ( nes.GetBlend() ),
		restore    ( true )
		{}

		VideoFilters::VideoFilters
		(
			Nes::Video n,
			uint idd,
			Settings& s,
			const Point& m,
			bool b,
			bool c,
			Nes::Video::Palette::Mode p
		)
		:
		settings       ( s ),
		backup         ( s, n ),
		maxScreenScale ( GetMaxScreenScale(m) ),
		canDoScanlines ( c ),
		canDoNtsc      ( m.x >= NTSC_WIDTH ),
		canDoBilinear  ( b ),
		paletteMode    ( p ),
		nes            ( n ),
		dialog         ( idd, this, Handlers::messages,Handlers::commands )
		{
		}

		VideoFilters::~VideoFilters()
		{
		}

		VideoFilters::Type VideoFilters::Load
		(
			const Configuration& cfg,
			Settings (&settings)[NUM_TYPES],
			Nes::Video nes,
			const Point& maxScreenSize,
			const bool canDoBilinear,
			const bool canDoScanlines,
			const Nes::Video::Palette::Mode paletteMode
		)
		{
			const uint maxScreenScale = GetMaxScreenScale(maxScreenSize);

			for (uint i=0; i < NUM_TYPES; ++i)
				settings[i].Reset();

			Configuration::ConstSection filters( cfg["video"]["filters"] );

			Type type = TYPE_STD;

			if (maxScreenScale >= 2 || maxScreenSize.x >= NTSC_WIDTH)
			{
				const GenericString string( filters["type"].Str() );

				if (string == L"ntsc")
				{
					if (maxScreenSize.x >= NTSC_WIDTH)
						type = TYPE_NTSC;
				}
				else if (string == L"scalex")
				{
					if (maxScreenScale >= 2)
						type = TYPE_SCALEX;
				}
				else if (string == L"hqx")
				{
					if (maxScreenScale >= 2)
						type = TYPE_HQX;
				}
				else if (string == L"2xsai")
				{
					if (maxScreenScale >= 2)
						type = TYPE_2XSAI;
				}
				else if (string == L"xbr")
				{
					if (maxScreenScale >= 2)
						type = TYPE_XBR;
				}
			}

			if (canDoBilinear)
			{
				for (uint i=0; i < NUM_TYPES; ++i)
				{
					NST_COMPILE_ASSERT(NUM_TYPES == 6);

					static cstring const types[] =
					{
						"standard", "ntsc", "scalex", "hqx", "2xsai", "xbr"
					};

					if (filters[types[i]]["bilinear"].Yes())
						settings[i].attributes[ATR_BILINEAR] = true;
				}
			}

			if (canDoScanlines)
			{
				uint v;

				if (100 >= (v=filters["standard"]["scanlines"].Int( 0 )))
					settings[TYPE_STD].attributes[ATR_SCANLINES] = v;

				if (100 >= (v=filters["ntsc"]["scanlines"].Int( 0 )))
					settings[TYPE_NTSC].attributes[ATR_SCANLINES] = v;
			}

			{
				const GenericString fieldMerging( filters["ntsc"]["field-merging"].Str() );

				settings[TYPE_NTSC].attributes[ATR_FIELDMERGING] =
				(
					fieldMerging == L"yes" ? ATR_FIELDMERGING_ON :
					fieldMerging == L"no"  ? ATR_FIELDMERGING_OFF :
                                             ATR_FIELDMERGING_AUTO
				);
			}

			{
				uint v;

				if (200 >= (v=filters["ntsc"]["sharpness"].Int( 100 )))
					nes.SetSharpness( int(v) - 100 );

				if (200 >= (v=filters["ntsc"]["resolution"].Int( 100 )))
                     nes.SetColorResolution( int(v) - 100 );

				if (200 >= (v=filters["ntsc"]["colorbleed"].Int( 100 )))
                     nes.SetColorBleed( int(v) - 100 );

				if (200 >= (v=filters["ntsc"]["artifacts"].Int( 100 )))
                     nes.SetColorArtifacts( int(v) - 100 );

				if (200 >= (v=filters["ntsc"]["fringing"].Int( 100 )))
					nes.SetColorFringing( int(v) - 100 );
			}

			settings[TYPE_NTSC].attributes[ATR_RESCALE_PIC] =
			(
				!filters["ntsc"]["tv-aspect"].Yes()
			);

			settings[TYPE_NTSC].attributes[ATR_NO_AUTO_TUNING] =
			(
				filters["ntsc"]["auto-tuning"].No()
			);

			nes.SetBlend(!filters["xbr"]["blend-pixels"].No());

			if (maxScreenScale >= 2)
			{
				GenericString scale( filters["scalex"]["scale"].Str() );

				settings[TYPE_SCALEX].attributes[ATR_TYPE] =
				(
					scale == L"3" ? (maxScreenScale >= 3 ? ATR_SCALE3X : ATR_SCALE2X) :
					scale == L"2" ? ATR_SCALE2X : ATR_SCALEAX
				);

				scale = filters["hqx"]["scale"].Str();

				settings[TYPE_HQX].attributes[ATR_TYPE] =
				(
					scale == L"4" ? (maxScreenScale >= 4 ? ATR_HQ4X : maxScreenScale >= 3 ? ATR_HQ3X : ATR_HQ2X) :
					scale == L"3" ? (maxScreenScale >= 3 ? ATR_HQ3X : ATR_HQ2X) :
					scale == L"2" ? ATR_HQ2X : ATR_HQAX
				);

				scale = filters["xbr"]["scale"].Str();

				settings[TYPE_XBR].attributes[ATR_TYPE] =
				(
					scale == L"4" ? (maxScreenScale >= 4 ? ATR_4XBR : maxScreenScale >= 3 ? ATR_3XBR : ATR_2XBR) :
					scale == L"3" ? (maxScreenScale >= 3 ? ATR_3XBR : ATR_2XBR) :
					scale == L"2" ? ATR_2XBR : ATR_AXBR
				);

				scale = filters["xbr"]["corner"].Str();

				nes.SetCornerRounding
				(
					scale == L"None" ? ATR_NONE :
					scale == L"All"  ? ATR_ALL :
									   ATR_SOME
				);
			}

			UpdateAutoModes( settings, nes, paletteMode );

			nes.ClearFilterUpdateFlag();

			return type;
		}

		void VideoFilters::Save
		(
			Configuration& cfg,
			const Settings (&settings)[NUM_TYPES],
			Nes::Video nes,
			const Type type
		)
		{
			Configuration::Section filters( cfg["video"]["filters"] );

			NST_COMPILE_ASSERT( NUM_TYPES == 6 );

			static cstring const types[] =
			{
				"standard", "ntsc", "scalex", "hqx", "2xsai", "xbr"
			};

			filters["type"].Str() = types[type];

			for (uint i=0; i < NUM_TYPES; ++i)
				filters[types[i]]["bilinear"].YesNo() = settings[i].attributes[ATR_BILINEAR];

			filters[ "standard" ][ "scanlines" ].Int() = settings[TYPE_STD].attributes[ATR_SCANLINES];
			filters[ "ntsc"     ][ "scanlines" ].Int() = settings[TYPE_NTSC].attributes[ATR_SCANLINES];

			filters["ntsc"]["field-merging"].Str() =
			(
				settings[TYPE_NTSC].attributes[ATR_FIELDMERGING] == ATR_FIELDMERGING_ON  ? "yes" :
				settings[TYPE_NTSC].attributes[ATR_FIELDMERGING] == ATR_FIELDMERGING_OFF ? "no" :
                                                                                           "auto"
			);

			filters[ "ntsc" ][ "sharpness"  ].Int() = nes.GetSharpness()       + 100;
			filters[ "ntsc" ][ "resolution" ].Int() = nes.GetColorResolution() + 100;
			filters[ "ntsc" ][ "colorbleed" ].Int() = nes.GetColorBleed()      + 100;
			filters[ "ntsc" ][ "artifacts"  ].Int() = nes.GetColorArtifacts()  + 100;
			filters[ "ntsc" ][ "fringing"   ].Int() = nes.GetColorFringing()   + 100;

			filters["ntsc"]["auto-tuning"].YesNo() =
			(
				!settings[TYPE_NTSC].attributes[ATR_NO_AUTO_TUNING]
			);

			filters["ntsc"]["tv-aspect"].YesNo() =
			(
				!settings[TYPE_NTSC].attributes[ATR_RESCALE_PIC]
			);

			filters["scalex"]["scale"].Str() =
			(
				settings[TYPE_SCALEX].attributes[ATR_TYPE] == ATR_SCALE3X ? "3" :
				settings[TYPE_SCALEX].attributes[ATR_TYPE] == ATR_SCALE2X ? "2" :
																			"auto"
			);

			filters["hqx"]["scale"].Str() =
			(
				settings[TYPE_HQX].attributes[ATR_TYPE] == ATR_HQ4X ? "4" :
				settings[TYPE_HQX].attributes[ATR_TYPE] == ATR_HQ3X ? "3" :
				settings[TYPE_HQX].attributes[ATR_TYPE] == ATR_HQ2X ? "2" :
                                                                      "auto"
			);

			filters["xbr"]["blend-pixels"].YesNo() = nes.GetBlend(); //<-- Note, conflicts with 2xbr
			filters["xbr"]["scale"].Str() =
			(
				settings[TYPE_XBR].attributes[ATR_TYPE] == ATR_4XBR ? "4" :
				settings[TYPE_XBR].attributes[ATR_TYPE] == ATR_3XBR ? "3" :
				settings[TYPE_XBR].attributes[ATR_TYPE] == ATR_2XBR ? "2" :
                                                                      "auto"
			);
			filters["xbr"]["corner"].Str() =
			(
				nes.GetCornerRounding() == ATR_ALL  ? "All"  :
				nes.GetCornerRounding() == ATR_NONE ? "None" :
												 	  "Some"
			);
		}

		uint VideoFilters::GetMaxScreenScale(const Point& p)
		{
			return
			(
				p.x >= NES_WIDTH*4 && p.y >= NES_HEIGHT*4 ? 4 :
				p.x >= NES_WIDTH*3 && p.y >= NES_HEIGHT*3 ? 3 :
				p.x >= NES_WIDTH*2 && p.y >= NES_HEIGHT*2 ? 2 : 1
			);
		}

		void VideoFilters::ResetAutoModes(Nes::Video nes,const Nes::Video::Palette::Mode mode)
		{
			nes.SetSharpness       ( mode == Nes::Video::Palette::MODE_YUV ? Nes::Video::DEFAULT_SHARPNESS_COMP        : Nes::Video::DEFAULT_SHARPNESS_RGB        );
			nes.SetColorResolution ( mode == Nes::Video::Palette::MODE_YUV ? Nes::Video::DEFAULT_COLOR_RESOLUTION_COMP : Nes::Video::DEFAULT_COLOR_RESOLUTION_RGB );
			nes.SetColorBleed      ( mode == Nes::Video::Palette::MODE_YUV ? Nes::Video::DEFAULT_COLOR_BLEED_COMP      : Nes::Video::DEFAULT_COLOR_BLEED_RGB      );
			nes.SetColorArtifacts  ( mode == Nes::Video::Palette::MODE_YUV ? Nes::Video::DEFAULT_COLOR_ARTIFACTS_COMP  : Nes::Video::DEFAULT_COLOR_ARTIFACTS_RGB  );
			nes.SetColorFringing   ( mode == Nes::Video::Palette::MODE_YUV ? Nes::Video::DEFAULT_COLOR_FRINGING_COMP   : Nes::Video::DEFAULT_COLOR_FRINGING_RGB   );
		}

		void VideoFilters::UpdateAutoModes(const Settings (&settings)[NUM_TYPES],Nes::Video nes,Nes::Video::Palette::Mode mode)
		{
			if (!settings[TYPE_NTSC].attributes[ATR_NO_AUTO_TUNING])
				ResetAutoModes( nes, mode );
		}

		ibool VideoFilters::OnInitDialog(Param&)
		{
			dialog.CheckBox(IDC_VIDEO_FILTER_BILINEAR).Check( settings.attributes[ATR_BILINEAR] );

			if (!canDoBilinear)
				dialog.CheckBox(IDC_VIDEO_FILTER_BILINEAR).Disable();

			uint idc;

			switch (dialog.GetId())
			{
				case IDD_VIDEO_FILTER_NTSC:

					dialog.RadioButton
					(
						settings.attributes[ATR_FIELDMERGING] == ATR_FIELDMERGING_ON  ? IDC_VIDEO_FILTER_NTSC_FIELDS_ON :
						settings.attributes[ATR_FIELDMERGING] == ATR_FIELDMERGING_OFF ? IDC_VIDEO_FILTER_NTSC_FIELDS_OFF :
																						IDC_VIDEO_FILTER_NTSC_FIELDS_AUTO
					).Check();

					dialog.CheckBox(IDC_VIDEO_FILTER_NTSC_AUTO_TUNING).Check( !settings.attributes[ATR_NO_AUTO_TUNING] );
					//dialog.CheckBox(IDC_VIDEO_FILTER_NTSC_TV_ASPECT).Check( !settings.attributes[ATR_RESCALE_PIC] );

					for (uint i=IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER; i <= IDC_VIDEO_FILTER_NTSC_FRINGING_SLIDER; ++i)
						dialog.Slider( i ).SetRange( 0, 200 );

					UpdateNtscSliders();
					UpdateNtscTuning();

				case IDD_VIDEO_FILTER_STD:

					if (!canDoScanlines)
					{
						dialog.Control(IDC_VIDEO_FILTER_SCANLINES_SLIDER).Disable();
						dialog.Control(IDC_VIDEO_FILTER_SCANLINES_VAL).Disable();
					}

					dialog.Slider(IDC_VIDEO_FILTER_SCANLINES_SLIDER).SetRange( 0, 100 );

					UpdateScanlinesSlider();
					break;

				case IDD_VIDEO_FILTER_SCALEX:

					switch (settings.attributes[ATR_TYPE])
					{
						case ATR_SCALE3X: idc = IDC_VIDEO_FILTER_SCALEX_3X;   break;
						case ATR_SCALE2X: idc = IDC_VIDEO_FILTER_SCALEX_2X;   break;
						default:          idc = IDC_VIDEO_FILTER_SCALEX_AUTO; break;
					}

					dialog.RadioButton(idc).Check();

					if (maxScreenScale < 3)
					{
						dialog.RadioButton(IDC_VIDEO_FILTER_SCALEX_AUTO).Disable();
						dialog.RadioButton(IDC_VIDEO_FILTER_SCALEX_2X).Disable();
						dialog.RadioButton(IDC_VIDEO_FILTER_SCALEX_3X).Disable();
					}
					break;

				case IDD_VIDEO_FILTER_HQX:

					switch (settings.attributes[ATR_TYPE])
					{
						case ATR_HQ4X: idc = IDC_VIDEO_FILTER_HQX_SCALING_4X;   break;
						case ATR_HQ3X: idc = IDC_VIDEO_FILTER_HQX_SCALING_3X;   break;
						case ATR_HQ2X: idc = IDC_VIDEO_FILTER_HQX_SCALING_2X;   break;
						default:       idc = IDC_VIDEO_FILTER_HQX_SCALING_AUTO; break;
					}

					dialog.RadioButton(idc).Check();

					if (maxScreenScale < 4)
					{
						if (maxScreenScale < 3)
						{
							dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_AUTO).Disable();
							dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_2X).Disable();
							dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_3X).Disable();
						}

						dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_4X).Disable();
					}
					break;

				case IDD_VIDEO_FILTER_2XSAI:
					break;

				case IDD_VIDEO_FILTER_XBR:
					dialog.RadioButton(IDC_VIDEO_FILTER_BLEND).Check(nes.GetBlend());
					switch (settings.attributes[ATR_TYPE])
					{
						case ATR_4XBR: idc = IDC_VIDEO_FILTER_XBR_SCALING_4X;   break;
						case ATR_3XBR: idc = IDC_VIDEO_FILTER_XBR_SCALING_3X;   break;
						case ATR_2XBR: idc = IDC_VIDEO_FILTER_XBR_SCALING_2X;   break;
						default:       idc = IDC_VIDEO_FILTER_XBR_SCALING_AUTO; break;
					}
					dialog.RadioButton(idc).Check();

					switch(nes.GetCornerRounding())
					{
						case ATR_NONE: idc = IDC_VIDEO_FILTER_XBR_ROUNDING_NONE; break;
						case ATR_ALL:  idc = IDC_VIDEO_FILTER_XBR_ROUNDING_ALL;  break;
						default:	   idc = IDC_VIDEO_FILTER_XBR_ROUNDING_SOME; break;
					}
					dialog.RadioButton(idc).Check();
					
					if (maxScreenScale < 4)
					{
						if (maxScreenScale < 3)
						{
							dialog.RadioButton(IDC_VIDEO_FILTER_XBR_SCALING_AUTO).Disable();
							dialog.RadioButton(IDC_VIDEO_FILTER_XBR_SCALING_2X).Disable();
							dialog.RadioButton(IDC_VIDEO_FILTER_XBR_SCALING_3X).Disable();
						}

						dialog.RadioButton(IDC_VIDEO_FILTER_XBR_SCALING_4X).Disable();
					}
					break;
			}

			return true;
		}

		ibool VideoFilters::OnDestroy(Param&)
		{
			if (backup.restore)
			{
				settings = backup.settings;

				nes.SetSharpness       ( backup.sharpness  );
				nes.SetColorResolution ( backup.resolution );
				nes.SetColorBleed      ( backup.bleed      );
				nes.SetColorArtifacts  ( backup.artifacts  );
				nes.SetColorFringing   ( backup.fringing   );
				nes.SetCornerRounding  ( backup.corner_rounding );
				nes.SetBlend           ( backup.blend      );
			}

			return true;
		}

		void VideoFilters::UpdateScanlinesSlider() const
		{
			dialog.Slider( IDC_VIDEO_FILTER_SCANLINES_SLIDER ).Position() = uint(settings.attributes[ATR_SCANLINES]);
			dialog.Edit( IDC_VIDEO_FILTER_SCANLINES_VAL ) << uint(settings.attributes[ATR_SCANLINES]);

			Application::Instance::GetMainWindow().Redraw();
		}

		void VideoFilters::UpdateNtscSliders() const
		{
			const uchar values[] =
			{
				100 + nes.GetSharpness(),
				100 + nes.GetColorResolution(),
				100 + nes.GetColorBleed(),
				100 + nes.GetColorArtifacts(),
				100 + nes.GetColorFringing()
			};

			for (uint i=0; i < 5; ++i)
			{
				dialog.Slider( IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER+i  ).Position() = values[i];
				UpdateNtscSlider( values[i], IDC_VIDEO_FILTER_NTSC_SHARPNESS_VAL+i );
			}
		}

		void VideoFilters::UpdateNtscSlider(int value,const uint idc) const
		{
			dialog.Edit( idc ).Text() << RealString( value / 100.0, 2 ).Ptr();
			Application::Instance::GetMainWindow().Redraw();
		}

		void VideoFilters::UpdateNtscTuning() const
		{
			const bool enabled = settings.attributes[ATR_NO_AUTO_TUNING];

			for (uint i=0; i < 5; ++i)
			{
				dialog.Control( IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER+i ).Enable( enabled );
				dialog.Control( IDC_VIDEO_FILTER_NTSC_SHARPNESS_VAL+i    ).Enable( enabled );
				dialog.Control( IDC_VIDEO_FILTER_NTSC_SHARPNESS_TEXT+i   ).Enable( enabled );
			}

			for (uint i=0; i < 3; ++i)
				dialog.Control( IDC_VIDEO_FILTER_NTSC_COMPOSITE+i ).Enable( enabled );
		}

		ibool VideoFilters::OnHScroll(Param& param)
		{
			const int value = param.Slider().Scroll();

			switch (param.Slider().GetId())
			{
				case IDC_VIDEO_FILTER_SCANLINES_SLIDER:

					if (settings.attributes[ATR_SCANLINES] != value)
					{
						settings.attributes[ATR_SCANLINES] = value;
						UpdateScanlinesSlider();
					}
					break;

				case IDC_VIDEO_FILTER_NTSC_SHARPNESS_SLIDER:

					if (nes.SetSharpness( value-100 ) != Nes::RESULT_NOP)
						UpdateNtscSlider( value, IDC_VIDEO_FILTER_NTSC_SHARPNESS_VAL );

					break;

				case IDC_VIDEO_FILTER_NTSC_RESOLUTION_SLIDER:

					if (nes.SetColorResolution( value-100 ) != Nes::RESULT_NOP)
						UpdateNtscSlider( value, IDC_VIDEO_FILTER_NTSC_RESOLUTION_VAL );

					break;

				case IDC_VIDEO_FILTER_NTSC_COLORBLEED_SLIDER:

					if (nes.SetColorBleed( value-100 ) != Nes::RESULT_NOP)
						UpdateNtscSlider( value, IDC_VIDEO_FILTER_NTSC_COLORBLEED_VAL );

					break;

				case IDC_VIDEO_FILTER_NTSC_ARTIFACTS_SLIDER:

					if (nes.SetColorArtifacts( value-100 ) != Nes::RESULT_NOP)
						UpdateNtscSlider( value, IDC_VIDEO_FILTER_NTSC_ARTIFACTS_VAL );

					break;

				case IDC_VIDEO_FILTER_NTSC_FRINGING_SLIDER:

					if (nes.SetColorFringing( value-100 ) != Nes::RESULT_NOP)
						UpdateNtscSlider( value, IDC_VIDEO_FILTER_NTSC_FRINGING_VAL );

					break;
			}

			return true;
		}

		ibool VideoFilters::OnCmdBilinear(Param& param)
		{
			if (param.Button().Clicked())
			{
				settings.attributes[ATR_BILINEAR] = bool(dialog.CheckBox( IDC_VIDEO_FILTER_BILINEAR ).Checked());
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool VideoFilters::OnCmdNtscTuning(Param& param)
		{
			if (param.Button().Clicked())
			{
				settings.attributes[ATR_NO_AUTO_TUNING] = dialog.CheckBox( IDC_VIDEO_FILTER_NTSC_AUTO_TUNING ).Unchecked();

				ResetAutoModes( nes, paletteMode );
				UpdateNtscTuning();
				UpdateNtscSliders();
			}

			return true;
		}

		ibool VideoFilters::OnCmdNtscCable(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint id = param.Button().GetId();

				nes.SetSharpness       (id == IDC_VIDEO_FILTER_NTSC_COMPOSITE ? Nes::Video::DEFAULT_SHARPNESS_COMP        : id == IDC_VIDEO_FILTER_NTSC_SVIDEO ? Nes::Video::DEFAULT_SHARPNESS_SVIDEO        : Nes::Video::DEFAULT_SHARPNESS_RGB        );
				nes.SetColorResolution (id == IDC_VIDEO_FILTER_NTSC_COMPOSITE ? Nes::Video::DEFAULT_COLOR_RESOLUTION_COMP : id == IDC_VIDEO_FILTER_NTSC_SVIDEO ? Nes::Video::DEFAULT_COLOR_RESOLUTION_SVIDEO : Nes::Video::DEFAULT_COLOR_RESOLUTION_RGB );
				nes.SetColorBleed      (id == IDC_VIDEO_FILTER_NTSC_COMPOSITE ? Nes::Video::DEFAULT_COLOR_BLEED_COMP      : id == IDC_VIDEO_FILTER_NTSC_SVIDEO ? Nes::Video::DEFAULT_COLOR_BLEED_SVIDEO      : Nes::Video::DEFAULT_COLOR_BLEED_RGB      );
				nes.SetColorArtifacts  (id == IDC_VIDEO_FILTER_NTSC_COMPOSITE ? Nes::Video::DEFAULT_COLOR_ARTIFACTS_COMP  : id == IDC_VIDEO_FILTER_NTSC_SVIDEO ? Nes::Video::DEFAULT_COLOR_ARTIFACTS_SVIDEO  : Nes::Video::DEFAULT_COLOR_ARTIFACTS_RGB  );
				nes.SetColorFringing   (id == IDC_VIDEO_FILTER_NTSC_COMPOSITE ? Nes::Video::DEFAULT_COLOR_FRINGING_COMP   : id == IDC_VIDEO_FILTER_NTSC_SVIDEO ? Nes::Video::DEFAULT_COLOR_FRINGING_SVIDEO   : Nes::Video::DEFAULT_COLOR_FRINGING_RGB   );

				UpdateNtscSliders();
			}

			return true;
		}

		ibool VideoFilters::OnCmdScaleX(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint id = param.Button().GetId();

				settings.attributes[ATR_TYPE] =
				(
					id == IDC_VIDEO_FILTER_SCALEX_3X ? ATR_SCALE3X :
					id == IDC_VIDEO_FILTER_SCALEX_2X ? ATR_SCALE2X :
                                                       ATR_SCALEAX
				);

				dialog.RadioButton( IDC_VIDEO_FILTER_SCALEX_AUTO ).Check( id == IDC_VIDEO_FILTER_SCALEX_AUTO );
				dialog.RadioButton( IDC_VIDEO_FILTER_SCALEX_2X   ).Check( id == IDC_VIDEO_FILTER_SCALEX_2X   );
				dialog.RadioButton( IDC_VIDEO_FILTER_SCALEX_3X   ).Check( id == IDC_VIDEO_FILTER_SCALEX_3X   );

				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool VideoFilters::OnCmdHqX(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint id = param.Button().GetId();

				settings.attributes[ATR_TYPE] =
				(
					id == IDC_VIDEO_FILTER_HQX_SCALING_2X ? ATR_HQ2X :
					id == IDC_VIDEO_FILTER_HQX_SCALING_3X ? ATR_HQ3X :
					id == IDC_VIDEO_FILTER_HQX_SCALING_4X ? ATR_HQ4X :
															ATR_HQAX
				);

				dialog.RadioButton( IDC_VIDEO_FILTER_HQX_SCALING_AUTO ).Check( id == IDC_VIDEO_FILTER_HQX_SCALING_AUTO );
				dialog.RadioButton( IDC_VIDEO_FILTER_HQX_SCALING_2X   ).Check( id == IDC_VIDEO_FILTER_HQX_SCALING_2X   );
				dialog.RadioButton( IDC_VIDEO_FILTER_HQX_SCALING_3X   ).Check( id == IDC_VIDEO_FILTER_HQX_SCALING_3X   );
				dialog.RadioButton( IDC_VIDEO_FILTER_HQX_SCALING_4X   ).Check( id == IDC_VIDEO_FILTER_HQX_SCALING_4X   );

				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool VideoFilters::OnCmdAlpha(Param& param)
		{
			if (param.Button().Clicked())
			{
				nes.SetBlend(dialog.RadioButton( IDC_VIDEO_FILTER_BLEND ).Checked());
				nes.ClearFilterUpdateFlag();
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool VideoFilters::OnCmdxBR(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint id = param.Button().GetId();

				settings.attributes[ATR_TYPE] =
				(
					id == IDC_VIDEO_FILTER_XBR_SCALING_2X ? ATR_2XBR :
					id == IDC_VIDEO_FILTER_XBR_SCALING_3X ? ATR_3XBR :
					id == IDC_VIDEO_FILTER_XBR_SCALING_4X ? ATR_4XBR :
															ATR_AXBR
				);

				dialog.RadioButton( IDC_VIDEO_FILTER_XBR_SCALING_AUTO ).Check( id == IDC_VIDEO_FILTER_XBR_SCALING_AUTO );
				dialog.RadioButton( IDC_VIDEO_FILTER_XBR_SCALING_2X   ).Check( id == IDC_VIDEO_FILTER_XBR_SCALING_2X   );
				dialog.RadioButton( IDC_VIDEO_FILTER_XBR_SCALING_3X   ).Check( id == IDC_VIDEO_FILTER_XBR_SCALING_3X   );
				dialog.RadioButton( IDC_VIDEO_FILTER_XBR_SCALING_4X   ).Check( id == IDC_VIDEO_FILTER_XBR_SCALING_4X   );

				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool VideoFilters::OnCmdxBRRound(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint id = param.Button().GetId();

				nes.SetCornerRounding
				(
					id == IDC_VIDEO_FILTER_XBR_ROUNDING_NONE ? ATR_NONE :
					id == IDC_VIDEO_FILTER_XBR_ROUNDING_ALL  ? ATR_ALL  :
															   ATR_SOME
				);
				nes.ClearFilterUpdateFlag();

				dialog.RadioButton( IDC_VIDEO_FILTER_XBR_ROUNDING_NONE ).Check( id == IDC_VIDEO_FILTER_XBR_ROUNDING_NONE );
				dialog.RadioButton( IDC_VIDEO_FILTER_XBR_ROUNDING_ALL   ).Check( id == IDC_VIDEO_FILTER_XBR_ROUNDING_ALL   );
				dialog.RadioButton( IDC_VIDEO_FILTER_XBR_ROUNDING_SOME   ).Check( id == IDC_VIDEO_FILTER_XBR_ROUNDING_SOME   );

				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool VideoFilters::OnCmdDefault(Param& param)
		{
			if (param.Button().Clicked())
			{
				settings.attributes[ATR_BILINEAR] = false;
				dialog.CheckBox( IDC_VIDEO_FILTER_BILINEAR ).Uncheck();

				switch (dialog.GetId())
				{
					case IDD_VIDEO_FILTER_NTSC:

						dialog.RadioButton( IDC_VIDEO_FILTER_NTSC_FIELDS_AUTO ).Check();
						dialog.RadioButton( IDC_VIDEO_FILTER_NTSC_FIELDS_ON ).Uncheck();
						dialog.RadioButton( IDC_VIDEO_FILTER_NTSC_FIELDS_OFF ).Uncheck();

						dialog.CheckBox( IDC_VIDEO_FILTER_NTSC_AUTO_TUNING ).Check();
						//dialog.CheckBox( IDC_VIDEO_FILTER_NTSC_TV_ASPECT ).Check();

						settings.attributes[ATR_NO_AUTO_TUNING] = false;

						ResetAutoModes( nes, paletteMode );
						UpdateNtscSliders();
						UpdateNtscTuning();

					case IDD_VIDEO_FILTER_STD:

						settings.attributes[ATR_SCANLINES] = 0;
						UpdateScanlinesSlider();
						break;

					case IDD_VIDEO_FILTER_SCALEX:

						settings.attributes[ATR_TYPE] = ATR_SCALEAX;
						dialog.RadioButton(IDC_VIDEO_FILTER_SCALEX_AUTO).Check();
						dialog.RadioButton(IDC_VIDEO_FILTER_SCALEX_2X).Uncheck();
						dialog.RadioButton(IDC_VIDEO_FILTER_SCALEX_3X).Uncheck();
						break;

					case IDD_VIDEO_FILTER_HQX:

						settings.attributes[ATR_TYPE] = ATR_HQAX;
						dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_AUTO).Check();
						dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_2X).Uncheck();
						dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_3X).Uncheck();
						dialog.RadioButton(IDC_VIDEO_FILTER_HQX_SCALING_4X).Uncheck();
						break;

					case IDD_VIDEO_FILTER_2XSAI:
						break;

					case IDD_VIDEO_FILTER_XBR:
						dialog.RadioButton(IDC_VIDEO_FILTER_BLEND).Check();
						settings.attributes[ATR_TYPE] = ATR_AXBR;
						dialog.RadioButton(IDC_VIDEO_FILTER_XBR_SCALING_AUTO).Check();
						dialog.RadioButton(IDC_VIDEO_FILTER_XBR_SCALING_2X).Uncheck();
						dialog.RadioButton(IDC_VIDEO_FILTER_XBR_SCALING_3X).Uncheck();
						dialog.RadioButton(IDC_VIDEO_FILTER_XBR_SCALING_4X).Uncheck();
						nes.SetCornerRounding(ATR_SOME);
						dialog.RadioButton(IDC_VIDEO_FILTER_XBR_ROUNDING_SOME).Check();
						dialog.RadioButton(IDC_VIDEO_FILTER_XBR_ROUNDING_ALL).Uncheck();
						dialog.RadioButton(IDC_VIDEO_FILTER_XBR_ROUNDING_NONE).Uncheck();
						break;
				}

				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool VideoFilters::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				switch (dialog.GetId())
				{
					case IDD_VIDEO_FILTER_NTSC:

						if (dialog.RadioButton(IDC_VIDEO_FILTER_NTSC_FIELDS_AUTO).Checked())
						{
							settings.attributes[ATR_FIELDMERGING] = ATR_FIELDMERGING_AUTO;
						}
						else if (dialog.RadioButton(IDC_VIDEO_FILTER_NTSC_FIELDS_ON).Checked())
						{
							settings.attributes[ATR_FIELDMERGING] = ATR_FIELDMERGING_ON;
						}
						else
						{
							settings.attributes[ATR_FIELDMERGING] = ATR_FIELDMERGING_OFF;
						}

						//settings.attributes[ATR_RESCALE_PIC] = bool(dialog.CheckBox(IDC_VIDEO_FILTER_NTSC_TV_ASPECT).Unchecked());
						break;
				}

				backup.restore = false;
				dialog.Close();
			}

			return true;
		}
	}
}
