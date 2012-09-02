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

#include "NstDialogLauncher.hpp"
#include <CommDlg.h>

namespace Nestopia
{
	namespace Window
	{
		inline Launcher::Colors::Type::Type(int l,int t,int r,int b)
		: rect(l,t,r,b) {}

		struct Launcher::Colors::Handlers
		{
			static const MsgHandler::Entry<Colors> messages[];
			static const MsgHandler::Entry<Colors> commands[];
		};

		const MsgHandler::Entry<Launcher::Colors> Launcher::Colors::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &Colors::OnInitDialog },
			{ WM_PAINT,      &Colors::OnPaint      }
		};

		const MsgHandler::Entry<Launcher::Colors> Launcher::Colors::Handlers::commands[] =
		{
			{ IDC_LAUNCHER_COLORS_BG_CHANGE, &Colors::OnCmdChangeBackground },
			{ IDC_LAUNCHER_COLORS_FG_CHANGE, &Colors::OnCmdChangeForeground },
			{ IDC_LAUNCHER_COLORS_DEFAULT,   &Colors::OnCmdDefault          }
		};

		Launcher::Colors::Colors(const Configuration& cfg)
		:
		background (20,94,74,114),
		foreground (20,33,74,53),
		dialog     (IDD_LAUNCHER_COLORS,this,Handlers::messages,Handlers::commands)
		{
			Configuration::ConstSection color( cfg["launcher"]["view"]["colors"] );

			background.color = color["background"].Int( DEF_BACKGROUND_COLOR );
			foreground.color = color["foreground"].Int( DEF_FOREGROUND_COLOR );
		}

		void Launcher::Colors::Save(Configuration& cfg) const
		{
			Configuration::Section color( cfg["launcher"]["view"]["colors"] );

			color[ "foreground" ].Str() = HexString( 32, foreground.color );
			color[ "background" ].Str() = HexString( 32, background.color );
		}

		void Launcher::Colors::UpdateColor(const Type& type) const
		{
			if (HDC const hDC = ::GetDC( dialog ))
			{
				HPEN const hPen = ::CreatePen( PS_SOLID, 1, RGB(0x00,0x00,0x00) );
				HPEN const hPenOld = static_cast<HPEN>(::SelectObject( hDC, hPen ));

				HBRUSH const hBrush = ::CreateSolidBrush( type.color );
				HBRUSH const hBrushOld = static_cast<HBRUSH>(::SelectObject( hDC, hBrush ));

				::Rectangle( hDC, type.rect.left, type.rect.top, type.rect.right, type.rect.bottom );

				::SelectObject( hDC, hBrushOld );
				::DeleteObject( hBrush );

				::SelectObject( hDC, hPenOld );
				::DeleteObject( hPen );

				::ReleaseDC( dialog, hDC );
			}
		}

		void Launcher::Colors::ChangeColor(COLORREF& color)
		{
			Object::Pod<CHOOSECOLOR> cc;

			cc.lStructSize  = sizeof(cc);
			cc.hwndOwner    = dialog;
			cc.lpCustColors = customColors;
			cc.rgbResult    = color;
			cc.Flags        = CC_FULLOPEN|CC_RGBINIT;

			if (::ChooseColor( &cc ))
				color = cc.rgbResult;
		}

		void Launcher::Colors::UpdateColors() const
		{
			UpdateColor( background );
			UpdateColor( foreground );
		}

		ibool Launcher::Colors::OnInitDialog(Param&)
		{
			UpdateColors();
			return true;
		}

		ibool Launcher::Colors::OnPaint(Param&)
		{
			UpdateColors();
			return false;
		}

		ibool Launcher::Colors::OnCmdChangeBackground(Param&)
		{
			ChangeColor( background.color );
			UpdateColor( background );
			return true;
		}

		ibool Launcher::Colors::OnCmdChangeForeground(Param&)
		{
			ChangeColor( foreground.color );
			UpdateColor( foreground );
			return true;
		}

		ibool Launcher::Colors::OnCmdDefault(Param&)
		{
			background.color = DEF_BACKGROUND_COLOR;
			foreground.color = DEF_FOREGROUND_COLOR;
			UpdateColors();
			return true;
		}
	}
}
