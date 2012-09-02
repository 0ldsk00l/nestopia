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

#ifndef NST_DIALOG_PALETTEEDITOR_H
#define NST_DIALOG_PALETTEEDITOR_H

#pragma once

#include "NstWindowDialog.hpp"

namespace Nestopia
{
	namespace Window
	{
		class PaletteEditor
		{
		public:

			PaletteEditor(Nes::Video&,const Managers::Paths&,const Path&);
			~PaletteEditor();

		private:

			struct Handlers;

			enum
			{
				BMP_START_X = 8,
				BMP_START_Y = 8,
				BMP_COLOR_WIDTH = 24,
				BMP_COLOR_HEIGHT = 24,
				BMP_ROWS = 16,
				BMP_COLUMNS = 4,
				BMP_END_X = BMP_START_X + BMP_COLOR_WIDTH * BMP_ROWS,
				BMP_END_Y = BMP_START_Y + BMP_COLOR_HEIGHT * BMP_COLUMNS,
				BMP_COLOR_SELECT = 0xFF,
				BMP_COLOR_UNSELECT = 0x50
			};

			struct Settings
			{
				explicit Settings(Nes::Video);

				void Restore(Nes::Video) const;

				Nes::Video::Palette::Mode mode;
				Nes::Video::Palette::CustomType customType;
				uint brightness;
				uint saturation;
				uint hue;
				uchar palette[8*64][3];
			};

			class History
			{
			public:

				History();

				void Reset();
				void Add(uint,uint);
				bool CanUndo() const;
				bool CanRedo() const;
				uint Undo(Nes::Video);
				uint Redo(Nes::Video);

			private:

				enum
				{
					LENGTH = 128,
					STOP = 0xFF
				};

				uint pos;
				uchar data[LENGTH][2];
			};

			void UpdateMode(bool=false);
			void UpdateColor();
			void UpdateColors();

			ibool OnInitDialog  (Param&);
			ibool OnPaint       (Param&);
			ibool OnLButtonDown (Param&);
			ibool OnHScroll     (Param&);
			ibool OnCmdHex      (Param&);
			ibool OnCmdUndo     (Param&);
			ibool OnCmdRedo     (Param&);
			ibool OnCmdReset    (Param&);
			ibool OnCmdSave     (Param&);
			ibool OnCmdMode     (Param&);

			Dialog dialog;
			uint colorSelect;
			Nes::Video emulator;
			const Managers::Paths& paths;
			Path path;
			ibool sliderDragging;
			History history;
			const Settings settings;

			static bool showHex;

		public:

			const Path& Open()
			{
				dialog.Open();
				return path;
			}
		};
	}
}

#endif
