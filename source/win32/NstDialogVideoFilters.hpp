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

#ifndef NST_DIALOG_VIDEOFILTERS_H
#define NST_DIALOG_VIDEOFILTERS_H

#pragma once

#include "NstWindowDialog.hpp"

namespace Nestopia
{
	namespace Window
	{
		class VideoFilters
		{
		public:

			enum Type
			{
				TYPE_STD,
				TYPE_NTSC,
				TYPE_SCALEX,
				TYPE_HQX,
				TYPE_2XSAI,
				TYPE_XBR,
				NUM_TYPES
			};

			enum
			{
				ATR_FIELDMERGING_AUTO = 0,
				ATR_FIELDMERGING_ON,
				ATR_FIELDMERGING_OFF,
				ATR_SCALEAX = 0,
				ATR_SCALE2X,
				ATR_SCALE3X,
				ATR_HQAX = 0,
				ATR_HQ2X,
				ATR_HQ3X,
				ATR_HQ4X,
				ATR_AXBR = 0,
				ATR_2XBR,
				ATR_3XBR,
				ATR_4XBR,
				ATR_NONE = 0,
				ATR_SOME,
				ATR_ALL,
				ATR_DEFAULT = 0
			};

			enum
			{
				ATR_BILINEAR = 0,
				ATR_TYPE,
				ATR_SCANLINES = 1,
				ATR_RESCALE_PIC,
				ATR_FIELDMERGING,
				ATR_NO_AUTO_TUNING
			};

			struct Settings
			{
				void Reset();

				schar attributes[8];
			};

			VideoFilters(Nes::Video,uint,Settings&,const Point&,bool,bool,Nes::Video::Palette::Mode);
			~VideoFilters();

			static Type Load(const Configuration&,Settings (&)[NUM_TYPES],Nes::Video,const Point&,bool,bool,Nes::Video::Palette::Mode);
			static void Save(Configuration&,const Settings (&)[NUM_TYPES],Nes::Video,Type);

			static void UpdateAutoModes(const Settings (&)[NUM_TYPES],Nes::Video,Nes::Video::Palette::Mode);

			enum
			{
				NES_WIDTH  = Nes::Video::Output::WIDTH,
				NES_HEIGHT = Nes::Video::Output::HEIGHT,
				NTSC_WIDTH = Nes::Video::Output::NTSC_WIDTH
			};

		private:

			static void ResetAutoModes(Nes::Video,Nes::Video::Palette::Mode);
			static uint GetMaxScreenScale(const Point&);

			struct Handlers;

			struct Backup
			{
				Backup(const Settings&,const Nes::Video);

				const Settings settings;
				const schar sharpness;
				const schar resolution;
				const schar bleed;
				const schar artifacts;
				const schar fringing;
				const schar corner_rounding;
				const schar blend;
				bool restore;
			};

			ibool OnInitDialog    (Param&);
			ibool OnDestroy       (Param&);
			ibool OnHScroll       (Param&);
			ibool OnCmdOk         (Param&);
			ibool OnCmdDefault    (Param&);
			ibool OnCmdBilinear   (Param&);
			ibool OnCmdNtscTuning (Param&);
			ibool OnCmdNtscCable  (Param&);
			ibool OnCmdScaleX     (Param&);
			ibool OnCmdHqX        (Param&);
			ibool OnCmdxBR        (Param&);
			ibool OnCmdxBRRound   (Param&);
			ibool OnCmdAlpha	  (Param&);

			void UpdateScanlinesSlider() const;
			void UpdateNtscSliders() const;
			void UpdateNtscSlider(int,uint) const;
			void UpdateNtscTuning() const;

			Settings& settings;
			Backup backup;
			const uchar maxScreenScale;
			const bool canDoScanlines;
			const bool canDoNtsc;
			const bool canDoBilinear;
			const Nes::Video::Palette::Mode paletteMode;
			Nes::Video nes;
			Dialog dialog;

		public:

			void Open()
			{
				dialog.Open();
			}
		};
	}
}

#endif
