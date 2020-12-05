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

#include "NstWindowCustom.hpp"
#include "NstWindowParam.hpp"
#include "NstCtrlStandard.hpp"
#include "NstString.hpp"

namespace Nestopia
{
	namespace Window
	{
		namespace Control
		{
			bool Generic::FixedFont() const
			{
				bool fixed = false;

				if (HFONT const hFontNew = reinterpret_cast<HFONT>(control.Send( WM_GETFONT )))
				{
					if (HDC const hdc = ::GetDC( control ))
					{
						if (HFONT const hFontOld = reinterpret_cast<HFONT>(::SelectObject( hdc, hFontNew )))
						{
							TEXTMETRIC tm;

							if (::GetTextMetrics( hdc, &tm ) && !(tm.tmPitchAndFamily & TMPF_FIXED_PITCH))
								fixed = true;

							::SelectObject( hdc, hFontOld );
						}

						::ReleaseDC( control, hdc );
					}
				}

				return fixed;
			}

			Point Generic::GetMaxTextSize() const
			{
				Point size;

				if (HFONT const hFontNew = reinterpret_cast<HFONT>(control.Send( WM_GETFONT )))
				{
					if (HDC const hdc = ::GetDC( control ))
					{
						if (HFONT const hFontOld = reinterpret_cast<HFONT>(::SelectObject( hdc, hFontNew )))
						{
							TEXTMETRIC tm;

							if (::GetTextMetrics( hdc, &tm ))
							{
								const uint width = control.Coordinates().Width();

								HeapString text;
								control.Text() >> text;

								for (wcstring s=text.Ptr(), n=s; *s; s=n, ++n)
								{
									while (*n && *n != '\n')
										++n;

									SIZE tsize;

									if (::GetTextExtentExPoint( hdc, s, n-s, width, NULL, NULL, &tsize ))
										size.x = NST_MAX(size.x,tsize.cx);

									size.y += tm.tmHeight;
								}

								if (size.x)
									size.x += 4*4;

								if (size.y)
									size.y += 4*2;
							}

							::SelectObject( hdc, hFontOld );
						}

						::ReleaseDC( control, hdc );
					}
				}

				return size;
			}

			NotificationHandler::NotificationHandler(const uint id,MsgHandler& m)
			: control(id), msgHandler(m)
			{
				Initialize();
			}

			NotificationHandler::~NotificationHandler()
			{
				msgHandler.Hooks().Remove( this );
			}

			void NotificationHandler::Initialize()
			{
				msgHandler.Hooks().Add( WM_NOTIFY, this, &NotificationHandler::OnNotify );
			}

			void NotificationHandler::OnNotify(Param& param)
			{
				NST_ASSERT( param.lParam );

				const NMHDR& nmhdr = *reinterpret_cast<const NMHDR*>(param.lParam);

				if (control == nmhdr.idFrom)
				{
					Items::iterator it(items.find( nmhdr.code ));

					if (it != items.end())
						it->second( nmhdr );
				}
			}
		}
	}
}
