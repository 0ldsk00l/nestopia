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
#include "NstWindowUser.hpp"
#include "NstWindowParam.hpp"
#include "NstApplicationInstance.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogPaletteEditor.hpp"

namespace Nestopia
{
	namespace Window
	{
		NST_COMPILE_ASSERT
		(
			IDC_PALETTE_EDITOR_G_SLIDER == IDC_PALETTE_EDITOR_R_SLIDER + 1 &&
			IDC_PALETTE_EDITOR_B_SLIDER == IDC_PALETTE_EDITOR_R_SLIDER + 2
		);

		NST_COMPILE_ASSERT
		(
			IDC_PALETTE_EDITOR_G_VALUE == IDC_PALETTE_EDITOR_R_VALUE + 1 &&
			IDC_PALETTE_EDITOR_B_VALUE == IDC_PALETTE_EDITOR_R_VALUE + 2
		);

		bool PaletteEditor::showHex = false;

		struct PaletteEditor::Handlers
		{
			static const MsgHandler::Entry<PaletteEditor> messages[];
			static const MsgHandler::Entry<PaletteEditor> commands[];
		};

		const MsgHandler::Entry<PaletteEditor> PaletteEditor::Handlers::messages[] =
		{
			{ WM_INITDIALOG,  &PaletteEditor::OnInitDialog  },
			{ WM_PAINT,       &PaletteEditor::OnPaint       },
			{ WM_LBUTTONDOWN, &PaletteEditor::OnLButtonDown },
			{ WM_HSCROLL,     &PaletteEditor::OnHScroll     }
		};

		const MsgHandler::Entry<PaletteEditor> PaletteEditor::Handlers::commands[] =
		{
			{ IDC_PALETTE_EDITOR_HEX,    &PaletteEditor::OnCmdHex    },
			{ IDC_PALETTE_EDITOR_UNDO,   &PaletteEditor::OnCmdUndo   },
			{ IDC_PALETTE_EDITOR_REDO,   &PaletteEditor::OnCmdRedo   },
			{ IDC_PALETTE_EDITOR_RESET,  &PaletteEditor::OnCmdReset  },
			{ IDC_PALETTE_EDITOR_SAVE,   &PaletteEditor::OnCmdSave   },
			{ IDC_PALETTE_EDITOR_CUSTOM, &PaletteEditor::OnCmdMode   },
			{ IDC_PALETTE_EDITOR_YUV,    &PaletteEditor::OnCmdMode   },
			{ IDC_PALETTE_EDITOR_RGB,    &PaletteEditor::OnCmdMode   }
		};

		PaletteEditor::Settings::Settings(Nes::Video emulator)
		{
			mode = emulator.GetPalette().GetMode();
			customType = emulator.GetPalette().GetCustomType();
			brightness = emulator.GetBrightness();
			saturation = emulator.GetSaturation();
			hue = emulator.GetHue();

			emulator.GetPalette().SetMode( Nes::Video::Palette::MODE_CUSTOM );
			emulator.SetBrightness( Nes::Video::DEFAULT_BRIGHTNESS );
			emulator.SetSaturation( Nes::Video::DEFAULT_SATURATION );
			emulator.SetHue( Nes::Video::DEFAULT_HUE );
			emulator.GetPalette().GetCustom( palette, customType );
		}

		void PaletteEditor::Settings::Restore(Nes::Video emulator) const
		{
			emulator.GetPalette().SetMode( mode );
			emulator.SetBrightness( brightness );
			emulator.SetSaturation( saturation );
			emulator.SetHue( hue );
			emulator.GetPalette().SetCustom( palette, customType );
		}

		PaletteEditor::History::History()
		{
			Reset();
		}

		void PaletteEditor::History::Reset()
		{
			pos = 0;
			data[0][0] = STOP;
			data[LENGTH-1][0] = STOP;
		}

		void PaletteEditor::History::Add(uint index,uint color)
		{
			data[pos][0] = index;
			data[pos][1] = color;
			pos = (pos+1) & (LENGTH-1);
			data[pos][0] = STOP;
		}

		bool PaletteEditor::History::CanUndo() const
		{
			return data[(pos-1) & (LENGTH-1)][0] != STOP;
		}

		bool PaletteEditor::History::CanRedo() const
		{
			return data[pos][0] != STOP;
		}

		uint PaletteEditor::History::Undo(Nes::Video emulator)
		{
			NST_ASSERT( CanUndo() );

			uchar palette[64][3];
			emulator.GetPalette().GetCustom( palette, Nes::Video::Palette::STD_PALETTE );

			pos = (pos-1) & (LENGTH-1);
			const uint index = data[pos][0];
			std::swap( palette[index / 3][index % 3], data[pos][1] );

			emulator.GetPalette().SetCustom( palette );

			return index / 3;
		}

		uint PaletteEditor::History::Redo(Nes::Video emulator)
		{
			NST_ASSERT( CanRedo() );

			uchar palette[64][3];
			emulator.GetPalette().GetCustom( palette, Nes::Video::Palette::STD_PALETTE );

			const uint index = data[pos][0];
			std::swap( palette[index / 3][index % 3], data[pos][1] );
			pos = (pos+1) & (LENGTH-1);

			emulator.GetPalette().SetCustom( palette );

			return index / 3;
		}

		PaletteEditor::PaletteEditor(Nes::Video& e,const Managers::Paths& p,const Path& d)
		:
		dialog         (IDD_PALETTE_EDITOR,this,Handlers::messages,Handlers::commands),
		colorSelect    (0),
		emulator       (e),
		paths          (p),
		path           (d),
		sliderDragging (false),
		settings       (e)
		{
		}

		PaletteEditor::~PaletteEditor()
		{
			settings.Restore( emulator );
		}

		ibool PaletteEditor::OnInitDialog(Param&)
		{
			for (uint i=0; i < 3; ++i)
				dialog.Slider( IDC_PALETTE_EDITOR_R_SLIDER+i ).SetRange( 0, 255 );

			dialog.RadioButton( IDC_PALETTE_EDITOR_CUSTOM ).Check();
			dialog.CheckBox( IDC_PALETTE_EDITOR_HEX ).Check( showHex );

			UpdateMode( true );
			UpdateColors();

			return true;
		}

		ibool PaletteEditor::OnPaint(Param&)
		{
			UpdateColors();
			return false;
		}

		void PaletteEditor::UpdateColors()
		{
			if (HDC const hdc = ::GetDC( dialog ))
			{
				class Bitmap : BITMAPINFO
				{
					RGBQUAD nesColor;
					uchar pixels[BMP_COLOR_WIDTH * BMP_COLOR_HEIGHT];

				public:

					Bitmap()
					{
						std::memset( this, 0, sizeof(*this) );

						bmiHeader.biSize = sizeof(bmiHeader);
						bmiHeader.biWidth = BMP_COLOR_WIDTH;
						bmiHeader.biHeight = BMP_COLOR_HEIGHT;
						bmiHeader.biPlanes = 1;
						bmiHeader.biBitCount = 8;
						bmiHeader.biCompression = BI_RGB;

						for (uint y=BMP_COLOR_WIDTH+1; y < BMP_COLOR_WIDTH * (BMP_COLOR_HEIGHT-2); y += BMP_COLOR_WIDTH)
							std::memset( pixels + y, 1, BMP_COLOR_WIDTH-2 );
					}

					void Draw(HDC const hdc,const uchar (*NST_RESTRICT palette)[3],const uint selected)
					{
						const RGBQUAD selectColors[2] =
						{
							{BMP_COLOR_UNSELECT,BMP_COLOR_UNSELECT,BMP_COLOR_UNSELECT,0},
							{BMP_COLOR_SELECT,BMP_COLOR_SELECT,BMP_COLOR_SELECT,0}
						};

						for (uint y=0; y < BMP_COLUMNS; ++y)
						{
							for (uint x=0; x < BMP_ROWS; ++x)
							{
								const uint index = y * BMP_ROWS + x;

								*bmiColors = selectColors[selected == index];

								nesColor.rgbRed   = palette[index][0];
								nesColor.rgbGreen = palette[index][1];
								nesColor.rgbBlue  = palette[index][2];

								::SetDIBitsToDevice
								(
									hdc,
									BMP_START_X + x * BMP_COLOR_WIDTH,
									BMP_START_Y + y * BMP_COLOR_HEIGHT,
									BMP_COLOR_WIDTH,
									BMP_COLOR_HEIGHT,
									0,
									0,
									0,
									BMP_COLOR_HEIGHT,
									&pixels,
									this,
									DIB_RGB_COLORS
								);
							}
						}
					}
				};

				Bitmap bitmap;
				bitmap.Draw( hdc, emulator.GetPalette().GetColors(), colorSelect );

				::ReleaseDC( dialog, hdc );

				Application::Instance::GetMainWindow().Redraw();
			}
		}

		ibool PaletteEditor::OnLButtonDown(Param& param)
		{
			const Point point( LOWORD(param.lParam), HIWORD(param.lParam) );

			if (point.x >= BMP_START_X && point.x < BMP_END_X && point.y >= BMP_START_Y && point.y < BMP_END_Y)
			{
				colorSelect = ((point.y-BMP_START_Y) / BMP_COLOR_HEIGHT * BMP_ROWS) + ((point.x-BMP_START_X) / BMP_COLOR_WIDTH);
				UpdateColor();
				UpdateColors();
				return true;
			}

			return true;
		}

		ibool PaletteEditor::OnHScroll(Param& param)
		{
			switch (uint index = param.Slider().GetId())
			{
				case IDC_PALETTE_EDITOR_R_SLIDER:
				case IDC_PALETTE_EDITOR_G_SLIDER:
				case IDC_PALETTE_EDITOR_B_SLIDER:
				{
					index -= IDC_PALETTE_EDITOR_R_SLIDER;

					const uint color = dialog.Slider( IDC_PALETTE_EDITOR_R_SLIDER+index ).Position();
					const uchar (*NST_RESTRICT colors)[3] = emulator.GetPalette().GetColors();

					if (color != colors[colorSelect][index])
					{
						if (!sliderDragging)
						{
							sliderDragging = true;
							history.Add( colorSelect * 3 + index, colors[colorSelect][index] );
						}

						if (showHex)
							dialog.Control( IDC_PALETTE_EDITOR_R_VALUE+index ).Text() << HexString( 8, color, true ).Ptr();
						else
							dialog.Control( IDC_PALETTE_EDITOR_R_VALUE+index ).Text() << color;

						{
							uchar custom[64][3];
							std::memcpy( custom, colors, 64 * 3 );
							custom[colorSelect][index] = color;
							emulator.GetPalette().SetCustom( custom );
						}

						UpdateColors();
					}

					if (sliderDragging && param.Slider().Released())
					{
						sliderDragging = false;
						dialog.Control( IDC_PALETTE_EDITOR_REDO ).Disable();
						dialog.Control( IDC_PALETTE_EDITOR_UNDO ).Enable();
					}

					break;
				}
			}

			return true;
		}

		ibool PaletteEditor::OnCmdHex(Param& param)
		{
			if (param.Button().Clicked())
			{
				showHex = dialog.CheckBox( IDC_PALETTE_EDITOR_HEX ).Checked();
				UpdateColor();
			}

			return true;
		}

		ibool PaletteEditor::OnCmdUndo(Param& param)
		{
			if (param.Button().Clicked())
			{
				colorSelect = history.Undo( emulator );
				dialog.Control( IDC_PALETTE_EDITOR_REDO ).Enable();
				dialog.Control( IDC_PALETTE_EDITOR_UNDO ).Enable( history.CanUndo() );
				UpdateColor();
				UpdateColors();
			}

			return true;
		}

		ibool PaletteEditor::OnCmdRedo(Param& param)
		{
			if (param.Button().Clicked())
			{
				colorSelect = history.Redo( emulator );
				dialog.Control( IDC_PALETTE_EDITOR_UNDO ).Enable();
				dialog.Control( IDC_PALETTE_EDITOR_REDO ).Enable( history.CanRedo() );
				UpdateColor();
				UpdateColors();
			}

			return true;
		}

		ibool PaletteEditor::OnCmdReset(Param& param)
		{
			if (param.Button().Clicked())
			{
				history.Reset();
				emulator.GetPalette().SetCustom( settings.palette );
				UpdateMode( true );
			}

			return true;
		}

		ibool PaletteEditor::OnCmdSave(Param& param)
		{
			if (param.Button().Clicked())
			{
				const Path tmp( paths.BrowseSave( Managers::Paths::File::PALETTE, Managers::Paths::SUGGEST, path ) );

				if (tmp.Length())
				{
					uchar emphPalette[8*64][3];
					const uchar (*palette)[3];

					if
					(
						settings.customType == Nes::Video::Palette::EXT_PALETTE &&
						emulator.GetPalette().GetMode() == Nes::Video::Palette::MODE_CUSTOM &&
						User::Confirm( IDS_VIDEO_PALETTE_SAVE_EMPH )
					)
					{
						std::memcpy( emphPalette, emulator.GetPalette().GetColors(), 1*64*3 );
						std::memcpy( emphPalette + 64, settings.palette+64, 7*64*3 );
						palette = emphPalette;
					}
					else
					{
						palette = emulator.GetPalette().GetColors();
					}

					paths.Save
					(
						palette,
						palette == emphPalette ? 8*64*3 : 1*64*3,
						Managers::Paths::File::PALETTE,
						tmp
					);

					path = tmp;
					dialog.Close();
				}
			}

			return true;
		}

		ibool PaletteEditor::OnCmdMode(Param& param)
		{
			if (param.Button().Clicked())
				UpdateMode();

			return true;
		}

		void PaletteEditor::UpdateMode(const bool forceUpdate)
		{
			Nes::Video::Palette::Mode mode;

			if (dialog.RadioButton(IDC_PALETTE_EDITOR_CUSTOM).Checked())
			{
				mode = Nes::Video::Palette::MODE_CUSTOM;
			}
			else if (dialog.RadioButton(IDC_PALETTE_EDITOR_YUV).Checked())
			{
				mode = Nes::Video::Palette::MODE_YUV;
			}
			else
			{
				mode = Nes::Video::Palette::MODE_RGB;
			}

			if (emulator.GetPalette().SetMode( mode ) != Nes::RESULT_NOP || forceUpdate)
			{
				const bool custom = (mode == Nes::Video::Palette::MODE_CUSTOM);

				for (uint i=0; i < 3; ++i)
					dialog.Slider( IDC_PALETTE_EDITOR_R_SLIDER+i ).Enable( custom );

				dialog.Control( IDC_PALETTE_EDITOR_UNDO ).Enable( custom && history.CanUndo() );
				dialog.Control( IDC_PALETTE_EDITOR_REDO ).Enable( custom && history.CanRedo() );
				dialog.Control( IDC_PALETTE_EDITOR_RESET ).Enable( custom );

				UpdateColor();
				UpdateColors();
			}
		}

		void PaletteEditor::UpdateColor()
		{
			if (showHex)
				dialog.Edit( IDC_PALETTE_EDITOR_INDEX ) << HexString( 8, colorSelect, true ).Ptr();
			else
				dialog.Edit( IDC_PALETTE_EDITOR_INDEX ) << colorSelect;

			const uchar (&colors)[3] = emulator.GetPalette().GetColors()[colorSelect];

			for (uint i=0; i < 3; ++i)
			{
				const uint color = colors[i];
				dialog.Slider( IDC_PALETTE_EDITOR_R_SLIDER+i ).Position() = color;

				if (showHex)
					dialog.Control( IDC_PALETTE_EDITOR_R_VALUE+i ).Text() << HexString( 8, color, true ).Ptr();
				else
					dialog.Control( IDC_PALETTE_EDITOR_R_VALUE+i ).Text() << color;
			}
		}
	}
}

