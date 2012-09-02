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

#ifndef NST_DIALOG_VIDEO_H
#define NST_DIALOG_VIDEO_H

#pragma once

#include "NstWindowDialog.hpp"
#include "NstDialogVideoFilters.hpp"
#include "NstDirect2D.hpp"
#include "../core/api/NstApiMachine.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Video
		{
		public:

			typedef DirectX::Direct2D::Adapter Adapter;
			typedef DirectX::Direct2D::Adapters Adapters;
			typedef DirectX::Direct2D::Mode Mode;
			typedef Adapter::Modes Modes;

			enum
			{
				NES_WIDTH            = Nes::Video::Output::WIDTH,
				NES_HEIGHT           = Nes::Video::Output::HEIGHT,
				NTSC_WIDTH           = Nes::Video::Output::NTSC_WIDTH,
				MIN_DIALOG_WIDTH     = 640,
				MIN_DIALOG_HEIGHT    = 480,
				SCREEN_MATCHED       = 8,
				SCREEN_STRETCHED     = 255,
				MIN_SCREEN_CURVATURE = -10,
				MAX_SCREEN_CURVATURE = +10
			};

			Video(Managers::Emulator&,const Adapters&,const Managers::Paths&,const Configuration&);
			~Video();

			void Save(Configuration&) const;
			void UpdateAutoModes() const;
			const Rect GetRenderState(Nes::Video::RenderState&,const Point) const;
			uint GetScanlines() const;
			Modes::const_iterator GetDialogMode() const;

		private:

			struct Handlers;

			uint GetFullscreenScaleMethod() const;
			void UpdateFullscreenScaleMethod(uint);
			void UpdateNtscFilter() const;
			void UpdateFinalRects();
			Nes::Video::Palette::Mode GetDesiredPaletteMode() const;

			enum
			{
				NTSC_CLIP_TOP = 8,
				NTSC_CLIP_BOTTOM = NES_HEIGHT - 8,
				PAL_CLIP_TOP = 1,
				PAL_CLIP_BOTTOM = NES_HEIGHT - 0,
				TV_WIDTH = 288
			};

			typedef VideoFilters Filter;

			struct Settings
			{
				Settings();
				~Settings();

				class Backup;

				struct Rects
				{
					Rect ntsc;
					Rect pal;
					Rect outNtsc;
					Rect outPal;
				};

				enum TexMem
				{
					TEXMEM_VIDMEM,
					TEXMEM_SYSMEM
				};

				Adapters::const_iterator adapter;
				TexMem texMem;
				Modes::const_iterator mode;
				Filter::Settings* filter;
				Filter::Settings filters[Filter::NUM_TYPES];
				Rects rects;
				Path palette;
				Nes::Video::Palette::Mode lockedMode;
				int screenCurvature;
				uchar fullscreenScale;
				bool autoPalette;
				bool autoHz;
				bool tvAspect;
				const Backup* backup;
			};

			ibool OnInitDialog        (Param&);
			ibool OnHScroll           (Param&);
			ibool OnCmdDevice         (Param&);
			ibool OnCmdMode           (Param&);
			ibool OnCmdFilter         (Param&);
			ibool OnCmdFilterSettings (Param&);
			ibool OnCmdBitDepth       (Param&);
			ibool OnCmdRam            (Param&);
			ibool OnCmdColorsAdvanced (Param&);
			ibool OnCmdColorsReset    (Param&);
			ibool OnCmdPalType        (Param&);
			ibool OnCmdPalBrowse      (Param&);
			ibool OnCmdPalClear       (Param&);
			ibool OnCmdPalEditor      (Param&);
			ibool OnCmdDefault        (Param&);
			ibool OnCmdOk             (Param&);
			ibool OnDestroy           (Param&);

			void UpdateDevice(Mode);
			void UpdateResolutions(Mode);
			void UpdateFilters();
			void UpdateRects(const Rect&,const Rect&) const;
			void UpdateColors() const;
			void UpdateScreenCurvature() const;
			void UpdatePalette() const;
			void ImportPalette(Path&,Managers::Paths::Alert);
			void ValidateRects();

			void ResetRects();
			void ResetColors();

			Modes::const_iterator GetDefaultMode() const;

			Settings settings;
			const Adapters& adapters;
			Managers::Emulator& nes;
			Dialog dialog;
			const Managers::Paths& paths;

			class DefaultMode
			{
				enum
				{
					DEFAULT_4_3_WIDTH = 640,
					DEFAULT_4_3_HEIGHT = 480,
					DEFAULT_16_9_WIDTH = 1680,
					DEFAULT_16_9_HEIGHT = 1050
				};

			public:

				DefaultMode();

				uint width;
				uint height;
			};

			static const DefaultMode defaultMode;

		public:

			void Open()
			{
				dialog.Open();
			}

			bool IsOpen() const
			{
				return dialog.IsOpen();
			}

			const Rect& GetInputRect() const
			{
				return (Nes::Machine(nes).GetMode() == Nes::Machine::NTSC ? settings.rects.ntsc : settings.rects.pal);
			}

			bool ToggleTvAspect()
			{
				settings.tvAspect = !settings.tvAspect;
				UpdateFinalRects();
				return settings.tvAspect;
			}

			bool TvAspect() const
			{
				return settings.tvAspect;
			}

			Modes::const_iterator GetMode() const
			{
				return settings.mode;
			}

			Adapters::const_iterator GetAdapter() const
			{
				return settings.adapter;
			}

			Adapter::Filter GetTextureFilter() const
			{
				if (settings.filter->attributes[Filter::ATR_BILINEAR] && (settings.adapter->filters & Adapter::FILTER_BILINEAR))
					return Adapter::FILTER_BILINEAR;
				else
					return Adapter::FILTER_NONE;
			}

			int GetScreenCurvature() const
			{
				return settings.screenCurvature;
			}

			uint GetFullscreenScale() const
			{
				return settings.fullscreenScale;
			}

			void SetFullscreenScale(uint scale)
			{
				settings.fullscreenScale = scale;
			}

			bool UseAutoFrequency() const
			{
				return settings.autoHz;
			}

			bool UseAutoFieldMerging() const
			{
				return settings.filters[Filter::TYPE_NTSC].attributes[Filter::ATR_FIELDMERGING] == Filter::ATR_FIELDMERGING_AUTO;
			}

			bool EnableFieldMerging() const
			{
				return settings.filters[Filter::TYPE_NTSC].attributes[Filter::ATR_FIELDMERGING] == Filter::ATR_FIELDMERGING_ON;
			}

			bool PutTextureInVideoMemory() const
			{
				return settings.texMem == Settings::TEXMEM_VIDMEM;
			}

			const Rect& GetNesRect(Nes::Machine::Mode mode) const
			{
				return mode == Nes::Machine::NTSC ? settings.rects.outNtsc : settings.rects.outPal;
			}

			const Rect& GetNesRect() const
			{
				return GetNesRect( Nes::Machine(nes).GetMode() );
			}
		};
	}
}

#endif
