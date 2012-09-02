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

#ifndef NST_WINDOW_RECT_H
#define NST_WINDOW_RECT_H

#pragma once

#include "NstWindowPoint.hpp"

namespace Nestopia
{
	namespace Window
	{
		struct Rect : RECT
		{
			Rect& ClientTransform(HWND);
			Rect& ScreenTransform(HWND);

			Rect(int v=0)
			{
				left = v;
				top = v;
				right = v;
				bottom = v;
			}

			Rect(const POINT& p)
			{
				left = 0;
				top = 0;
				right = p.x;
				bottom = p.y;
			}

			Rect(int l,int t,int r,int b)
			{
				left = l;
				top = t;
				right = r;
				bottom = b;
			}

			Rect(const RECT& r)
			{
				left = r.left;
				top = r.top;
				right = r.right;
				bottom = r.bottom;
			}

			Rect& Set(int l,int t,int r,int b)
			{
				left = l;
				top = t;
				right = r;
				bottom = b;
				return *this;
			}

			Rect& operator = (int v)
			{
				left = v;
				top = v;
				right = v;
				bottom = v;
				return *this;
			}

			Rect& operator = (const POINT& p)
			{
				left = 0;
				top = 0;
				right = p.x;
				bottom = p.y;
				return *this;
			}

			Rect& operator = (const RECT& r)
			{
				left = r.left;
				top = r.top;
				right = r.right;
				bottom = r.bottom;
				return *this;
			}

			bool operator == (const Rect& r) const
			{
				return
				(
					right == r.right &&
					bottom == r.bottom &&
					left == r.left &&
					top == r.top
				);
			}

			bool operator != (const Rect& r) const
			{
				return
				(
					right != r.right ||
					bottom != r.bottom ||
					left != r.left ||
					top != r.top
				);
			}

			bool operator ! () const
			{
				return (right | bottom | left | top) == 0;
			}

			long& operator [] (uint i)
			{
				return (&left)[i];
			}

			const long& operator [] (uint i) const
			{
				return (&left)[i];
			}

			bool Inside(const Point& p) const
			{
				return p.x >= left && p.x < right && p.y >= top && p.y < bottom;
			}

			bool Valid() const
			{
				return left <= right && top <= bottom;
			}

			bool Visible() const
			{
				return right - left > 0 && bottom - top > 0;
			}

		private:

			class PosProxy
			{
				Rect& r;

			public:

				PosProxy(Rect& rect)
				: r(rect) {}

				PosProxy& operator = (const Point& p)
				{
					r.right = p.x + (r.right - r.left);
					r.bottom = p.y + (r.bottom - r.top);
					r.left = p.x;
					r.top = p.y;
					return *this;
				}

				PosProxy& operator += (const Point& p)
				{
					r.left += p.x;
					r.top += p.y;
					r.right += p.x;
					r.bottom += p.y;
					return *this;
				}

				Point operator + (const Point& p) const
				{
					return Point( r.left + p.x, r.top + p.y );
				}

				PosProxy& operator -= (const Point& p)
				{
					r.left -= p.x;
					r.top -= p.y;
					r.right -= p.x;
					r.bottom -= p.y;
					return *this;
				}

				Point operator - (const Point& p) const
				{
					return Point( r.left - p.x, r.top - p.y );
				}

				operator Point () const
				{
					return Point(r.left,r.top);
				}
			};

			class WidthHeightProxy
			{
				long* p;

			public:

				WidthHeightProxy(long* point)
				: p(point) {}

				WidthHeightProxy& operator = (int xy)
				{
					p[2] = p[0] + xy;
					return *this;
				}

				WidthHeightProxy& operator += (int xy)
				{
					p[2] += xy;
					return *this;
				}

				int operator + (int xy) const
				{
					return p[2] - p[0] + xy;
				}

				WidthHeightProxy& operator -= (int xy)
				{
					p[2] -= xy;
					return *this;
				}

				int operator - (int xy) const
				{
					return p[2] - p[0] - xy;
				}

				operator int () const
				{
					return p[2] - p[0];
				}
			};

			class SizeProxy
			{
				Rect& r;

			public:

				SizeProxy(Rect& rect)
				: r(rect) {}

				SizeProxy& operator = (const Point& p)
				{
					r.right = r.left + p.x;
					r.bottom = r.top + p.y;
					return *this;
				}

				SizeProxy& operator += (const Point& p)
				{
					r.right += p.x;
					r.bottom += p.y;
					return *this;
				}

				Point operator + (const Point& p) const
				{
					return Point
					(
						(r.right - r.left) + p.x,
						(r.bottom - r.top) + p.y
					);
				}

				SizeProxy& operator -= (const Point& p)
				{
					r.right -= p.x;
					r.bottom -= p.y;
					return *this;
				}

				Point operator - (const Point& p) const
				{
					return Point
					(
						(r.right - r.left) - p.x,
						(r.bottom - r.top) - p.y
					);
				}

				operator Point () const
				{
					return Point
					(
						r.right - r.left,
						r.bottom - r.top
					);
				}
			};

			class CenterProxy
			{
				Rect& r;

			public:

				CenterProxy(Rect& rect)
				: r(rect) {}

				CenterProxy& operator = (const Point& p)
				{
					const Point size
					(
						r.right - r.left,
						r.bottom - r.top
					);

					r.left = p.x - (size.x / 2);
					r.right = p.x + (size.x / 2) + (size.x % 2);
					r.top = p.y - (size.y / 2);
					r.bottom = p.y + (size.y / 2) + (size.y % 2);

					return *this;
				}

				operator Point () const
				{
					return Point
					(
						r.left + ((r.right - r.left) / 2),
						r.top  + ((r.bottom - r.top) / 2)
					);
				}
			};

		public:

			PosProxy Position()
			{
				return *this;
			}

			Point Position() const
			{
				return Point( left, top );
			}

			SizeProxy Size()
			{
				return *this;
			}

			Point Size() const
			{
				return Point( right - left, bottom - top );
			}

			CenterProxy Center()
			{
				return *this;
			}

			Point Center() const
			{
				return Point
				(
					left + ((right - left) / 2),
					top  + ((bottom - top) / 2)
				);
			}

			Point& Corner()
			{
				return reinterpret_cast<Point&>(right);
			}

			const Point& Corner() const
			{
				return reinterpret_cast<const Point&>(right);
			}

			WidthHeightProxy Width()
			{
				return &left;
			}

			int Width() const
			{
				return right - left;
			}

			WidthHeightProxy Height()
			{
				return &top;
			}

			int Height() const
			{
				return bottom - top;
			}

			bool operator <  (const Rect& r) const { return Width() * Height() <  r.Width() * r.Height(); }
			bool operator >  (const Rect& r) const { return Width() * Height() >  r.Width() * r.Height(); }
			bool operator <= (const Rect& r) const { return Width() * Height() <= r.Width() * r.Height(); }
			bool operator >= (const Rect& r) const { return Width() * Height() >= r.Width() * r.Height(); }

			Rect operator * (const Rect& r) const
			{
				return Rect
				(
					left * r.left,
					top * r.top,
					right * r.right,
					bottom * r.bottom
				);
			}

			Rect operator * (const long v) const
			{
				return Rect
				(
					left * v,
					top * v,
					right * v,
					bottom * v
				);
			}

			Rect operator * (const Point& p) const
			{
				return Rect
				(
					left * p.x,
					top * p.y,
					right * p.x,
					bottom * p.y
				);
			}

			Rect operator / (const long v) const
			{
				return Rect
				(
					left / v,
					top / v,
					right / v,
					bottom / v
				);
			}

			Rect operator / (const Point& p) const
			{
				return Rect
				(
					left / p.x,
					top / p.y,
					right / p.x,
					bottom / p.y
				);
			}

			struct Window;
		};

		struct Rect::Window : Rect
		{
			Window(HWND hWnd)
			{
				::GetWindowRect( hWnd, this );
			}
		};
	}
}

#endif
