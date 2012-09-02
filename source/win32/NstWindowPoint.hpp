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

#ifndef NST_WINDOW_POINT_H
#define NST_WINDOW_POINT_H

#pragma once

#include "NstMain.hpp"
#include <windows.h>

namespace Nestopia
{
	namespace Window
	{
		struct Point : POINT
		{
			enum Scaling
			{
				SCALE_BELOW,
				SCALE_ABOVE,
				SCALE_NEAREST
			};

			uint ScaleToFit(uint,uint,Scaling,uint=UINT_MAX);

			Point(int v=0)
			{
				x = v;
				y = v;
			}

			Point(int a,int b)
			{
				x = a;
				y = b;
			}

			Point(const POINT& p)
			{
				x = p.x;
				y = p.y;
			}

			Point(const RECT& r)
			{
				x = r.right - r.left;
				y = r.bottom - r.top;
			}

			Point& Set(int a,int b)
			{
				x = a;
				y = b;
				return *this;
			}

			Point& operator = (const POINT& p)
			{
				x = p.x;
				y = p.y;
				return *this;
			}

			Point& operator = (int v)
			{
				x = v;
				y = v;
				return *this;
			}

			Point& operator = (const RECT& r)
			{
				x = r.right - r.left;
				y = r.bottom - r.top;
				return *this;
			}

			bool operator == (const Point& p) const { return x == p.x && y == p.y; }
			bool operator != (const Point& p) const { return x != p.x || y != p.y; }
			bool operator <  (const Point& p) const { return x * y <  p.x * p.y;   }
			bool operator >  (const Point& p) const { return x * y >  p.x * p.y;   }
			bool operator <= (const Point& p) const { return x * y <= p.x * p.y;   }
			bool operator >= (const Point& p) const { return x * y >= p.x * p.y;   }

			bool operator ! () const
			{
				return (x | y) == 0;
			}

			long& operator [] (uint i)
			{
				return (&x)[i];
			}

			const long& operator [] (uint i) const
			{
				return (&x)[i];
			}

			Point& operator += (const Point& p) { x += p.x; y += p.y; return *this; }
			Point& operator -= (const Point& p) { x -= p.x; y -= p.y; return *this; }
			Point& operator *= (const Point& p) { x *= p.x; y *= p.y; return *this; }
			Point& operator /= (const Point& p) { x /= p.x; y /= p.y; return *this; }

			Point operator + (const Point& p) const { return Point( x + p.x, y + p.y ); }
			Point operator - (const Point& p) const { return Point( x - p.x, y - p.y ); }
			Point operator * (const Point& p) const { return Point( x * p.x, y * p.y ); }
			Point operator / (const Point& p) const { return Point( x / p.x, y / p.y ); }

			friend Point operator + (int v,const Point& p) { return Point( p.x + v, p.y + v ); }
			friend Point operator * (int v,const Point& p) { return Point( p.x * v, p.y * v ); }

			Point Center() const
			{
				return Point( x / 2, y / 2 );
			}

			uint ScaleToFit(const Point& p,Scaling s,uint f=UINT_MAX)
			{
				return ScaleToFit( p.x, p.y, s, f );
			}

			Point& ClientTransform(HWND hWnd)
			{
				::ScreenToClient( hWnd, this );
				return *this;
			}

			Point& ScreenTransform(HWND hWnd)
			{
				::ClientToScreen( hWnd, this );
				return *this;
			}

			struct Client;
			struct Picture;
			struct NonClient;
		};

		struct Point::Client : Point
		{
			Client(HWND);
		};

		struct Point::Picture : Point
		{
			Picture(HWND);
		};

		struct Point::NonClient : Point
		{
			NonClient(HWND);
		};
	}
}

#endif
