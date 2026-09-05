/*
   Hyllian's 3xBR v3.3a-b-c
   
   Copyright (C) 2011, 2012 Hyllian/Jararaca - sergiogdb@gmail.com

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <cmath>
#include "NstAssert.hpp"
#include "NstVideoRenderer.hpp"
#include "NstVideoFilterxBR.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			/**
			 * Constructor
			 */
			Renderer::FilterxBR::FilterxBR(const RenderState& state, const bool blend, const schar corner_rounding)
			:
			_blend(blend),
			Filter (state),
			path   (GetPath(state, blend, corner_rounding))
			{
				_index = new YUVPixel*[32768];

				//Todo: When a setting is changed before starting a game, "transform" will
				//not be called for some reason. (This is a quick workaround)
				initCache();
			}

			/**
			 * Creates a RGB to YUV lookup table
			 */
			void Renderer::FilterxBR::initCache() const
			{
				for(int c=0; c < 32768; c++) { //Hmm, 32000+ should be enough
					YUVPixel *yuv_pt = new YUVPixel;
					*yuv_pt = YUVPixel::FromWord(c, format.bpp);
					_index[c] = yuv_pt;
				}
			}

			Renderer::FilterxBR::~FilterxBR()
			{
				freeCache();
				free(_index);
			}

			void Renderer::FilterxBR::freeCache() const
			{
				//Frees cached colors (note that null pointes can be freed safely).
				for(int c=0; c < 32768; c++)
					free(_index[c]);
			}

			Renderer::FilterxBR::Path Renderer::FilterxBR::GetPath(const RenderState& state, const bool blend, const schar corner_rounding)
			{
				if (state.bits.count == 32)
				{
					//Color mode 888
					if (state.filter == RenderState::FILTER_2XBR)
					{
						if (blend)
						{
							if (corner_rounding == 0)
								return &FilterxBR::Xbr2X<dword, 0xFF0000, 16, 0xFF00, 8, 0xFF, 0, true, false, false, true>;
							if (corner_rounding == 1)
								return &FilterxBR::Xbr2X<dword, 0xFF0000, 16, 0xFF00, 8, 0xFF, 0, true, false, true, false>;
							return &FilterxBR::Xbr2X<dword, 0xFF0000, 16, 0xFF00, 8, 0xFF, 0, true, true, false, false>;
						}

						if (corner_rounding == 0)
							return &FilterxBR::Xbr2X<dword, 0xFF0000, 16, 0xFF00, 8, 0xFF, 0, false, false, false, true>;
						if (corner_rounding == 1)
							return &FilterxBR::Xbr2X<dword, 0xFF0000, 16, 0xFF00, 8, 0xFF, 0, false, false, true, false>;
						return &FilterxBR::Xbr2X<dword, 0xFF0000, 16, 0xFF00, 8, 0xFF, 0, false, true, false, false>;
					}
					else if (state.filter == RenderState::FILTER_3XBR)
					{
						if (blend) 
						{
							if (corner_rounding == 0)
								return &FilterxBR::Xbr3X<dword, 0xFF0000, 16, 0xFF00, 8, 0xFF, 0, true, false, false, true>;
							if (corner_rounding == 1)
								return &FilterxBR::Xbr3X<dword, 0xFF0000, 16, 0xFF00, 8, 0xFF, 0, true, false, true, false>;
							return &FilterxBR::Xbr3X<dword, 0xFF0000, 16, 0xFF00, 8, 0xFF, 0, true, true, false, false>;
						}

						if (corner_rounding == 0)
							return &FilterxBR::Xbr3X<dword, 0xFF0000, 16, 0xFF00, 8, 0xFF, 0, false, false, false, true>;
						if (corner_rounding == 1)
							return &FilterxBR::Xbr3X<dword, 0xFF0000, 16, 0xFF00, 8, 0xFF, 0, false, false, true, false>;
						return &FilterxBR::Xbr3X<dword, 0xFF0000, 16, 0xFF00, 8, 0xFF, 0, false, true, false, false>;
					}
					if (blend) 
					{
						if (corner_rounding == 0)
							return &FilterxBR::Xbr4X<dword, 0xFF0000, 16, 0xFF00, 8, 0xFF, 0, true, false, false, true>;
						if (corner_rounding == 1)
							return &FilterxBR::Xbr4X<dword, 0xFF0000, 16, 0xFF00, 8, 0xFF, 0, true, false, true, false>;
						return &FilterxBR::Xbr4X<dword, 0xFF0000, 16, 0xFF00, 8, 0xFF, 0, true, true, false, false>;
					}
					
					if (corner_rounding == 0)
						return &FilterxBR::Xbr4X<dword, 0xFF0000, 16, 0xFF00, 8, 0xFF, 0, false, false, false, true>;
					if (corner_rounding == 1)
						return &FilterxBR::Xbr4X<dword, 0xFF0000, 16, 0xFF00, 8, 0xFF, 0, false, false, true, false>;
					return &FilterxBR::Xbr4X<dword, 0xFF0000, 16, 0xFF00, 8, 0xFF, 0, false, true, false, false>;
				}
				else if (state.bits.mask.g == 0x07E0)
				{
					//Color mode 565
					if (state.filter == RenderState::FILTER_2XBR)
					{
						if (blend)
						{
							if (corner_rounding == 0)
								return &FilterxBR::Xbr2X<word, 0xF800, 8, 0x7C0, 3, 0x1F, 3, true, false, false, true>;
							if (corner_rounding == 1)
								return &FilterxBR::Xbr2X<word, 0xF800, 8, 0x7C0, 3, 0x1F, 3, true, false, true, false>;
							return &FilterxBR::Xbr2X<word, 0xF800, 8, 0x7C0, 3, 0x1F, 3, true, true, false, false>;
						}
						if (corner_rounding == 0)
							return &FilterxBR::Xbr2X<word, 0xF800, 8, 0x7C0, 3, 0x1F, 3, false, false, false, true>;
						if (corner_rounding == 1)
							return &FilterxBR::Xbr2X<word, 0xF800, 8, 0x7C0, 3, 0x1F, 3, false, false, true, false>;
						return &FilterxBR::Xbr2X<word, 0xF800, 8, 0x7C0, 3, 0x1F, 3, false, true, false, false>;
					}
					else if (state.filter == RenderState::FILTER_3XBR)
					{
						if (blend) 
						{
							if (corner_rounding == 0)
								return &FilterxBR::Xbr3X<word, 0xF800, 8, 0x7C0, 3, 0x1F, 3, true, false, false, true>;
							if (corner_rounding == 1)
								return &FilterxBR::Xbr3X<word, 0xF800, 8, 0x7C0, 3, 0x1F, 3, true, false, true, false>;
							return &FilterxBR::Xbr3X<word, 0xF800, 8, 0x7C0, 3, 0x1F, 3, true, true, false, false>;
						}

						if (corner_rounding == 0)
							return &FilterxBR::Xbr3X<word, 0xF800, 8, 0x7C0, 3, 0x1F, 3, false, false, false, true>;
						if (corner_rounding == 1)
							return &FilterxBR::Xbr3X<word, 0xF800, 8, 0x7C0, 3, 0x1F, 3, false, false, true, false>;
						return &FilterxBR::Xbr3X<word, 0xF800, 8, 0x7C0, 3, 0x1F, 3, false, true, false, false>;
					}
					if (blend) 
					{
						if (corner_rounding == 0)
							return &FilterxBR::Xbr4X<word, 0xF800, 8, 0x7C0, 3, 0x1F, 3, true, false, false, true>;
						if (corner_rounding == 1)
							return &FilterxBR::Xbr4X<word, 0xF800, 8, 0x7C0, 3, 0x1F, 3, true, false, true, false>;
						return &FilterxBR::Xbr4X<word, 0xF800, 8, 0x7C0, 3, 0x1F, 3, true, true, false, false>;
					}
					if (corner_rounding == 0)
						return &FilterxBR::Xbr4X<word, 0xF800, 8, 0x7C0, 3, 0x1F, 3, false, false, false, true>;
					if (corner_rounding == 1)
						return &FilterxBR::Xbr4X<word, 0xF800, 8, 0x7C0, 3, 0x1F, 3, false, false, true, false>;
					return &FilterxBR::Xbr4X<word, 0xF800, 8, 0x7C0, 3, 0x1F, 3, false, true, false, false>;
				}
				else
				{
					//Color mode 555
					if (state.filter == RenderState::FILTER_2XBR)
					{
						if (blend)
						{
							if (corner_rounding == 0)
								return &FilterxBR::Xbr2X<word, 0x7C00, 7, 0x7C0, 3, 0x1F, 3, true, false, false, true>;
							if (corner_rounding == 1)	
								return &FilterxBR::Xbr2X<word, 0x7C00, 7, 0x7C0, 3, 0x1F, 3, true, false, true, false>;
							return &FilterxBR::Xbr2X<word, 0x7C00, 7, 0x7C0, 3, 0x1F, 3, true, true, false, false>;
						}
						if (corner_rounding == 0)
							return &FilterxBR::Xbr2X<word, 0x7C00, 7, 0x7C0, 3, 0x1F, 3, false, false, false, true>;
						if (corner_rounding == 1)
							return &FilterxBR::Xbr2X<word, 0x7C00, 7, 0x7C0, 3, 0x1F, 3, false, false, true, false>;
						return &FilterxBR::Xbr2X<word, 0x7C00, 7, 0x7C0, 3, 0x1F, 3, false, true, false, false>;
					}
					else if (state.filter == RenderState::FILTER_3XBR)
					{
						if (blend) 
						{
							if (corner_rounding == 0)
								return &FilterxBR::Xbr3X<word, 0x7C00, 7, 0x7C0, 3, 0x1F, 3, true, false, false, true>;
							if (corner_rounding == 1)
								return &FilterxBR::Xbr3X<word, 0x7C00, 7, 0x7C0, 3, 0x1F, 3, true, false, true, false>;
							return &FilterxBR::Xbr3X<word, 0x7C00, 7, 0x7C0, 3, 0x1F, 3, true, true, false, false>;
						}

						if (corner_rounding == 0)
							return &FilterxBR::Xbr3X<word, 0x7C00, 7, 0x7C0, 3, 0x1F, 3, false, false, false, true>;
						if (corner_rounding == 1)
							return &FilterxBR::Xbr3X<word, 0x7C00, 7, 0x7C0, 3, 0x1F, 3, false, false, true, false>;
						return &FilterxBR::Xbr3X<word, 0x7C00, 7, 0x7C0, 3, 0x1F, 3, false, true, false, false>;
					}
					if (blend) 
					{
						if (corner_rounding == 0)
							return &FilterxBR::Xbr4X<word, 0x7C00, 7, 0x7C0, 3, 0x1F, 3, true, false, false, true>;
						if (corner_rounding == 1)
							return &FilterxBR::Xbr4X<word, 0x7C00, 7, 0x7C0, 3, 0x1F, 3, true, false, true, false>;
						return &FilterxBR::Xbr4X<word, 0x7C00, 7, 0x7C0, 3, 0x1F, 3, true, true, false, false>;
					}
					if (corner_rounding == 0)
						return &FilterxBR::Xbr4X<word, 0x7C00, 7, 0x7C0, 3, 0x1F, 3, false, false, false, true>;
					if (corner_rounding == 1)
						return &FilterxBR::Xbr4X<word, 0x7C00, 7, 0x7C0, 3, 0x1F, 3, false, false, true, false>;
					return &FilterxBR::Xbr4X<word, 0x7C00, 7, 0x7C0, 3, 0x1F, 3, false, true, false, false>;
				}
				//return NULL;
			}

			/**
			 * Checks if this filter can be used with the render state
			 *
			 * Currently only supports 888, 565 and 555 output.
			 */
			bool Renderer::FilterxBR::Check(const RenderState& state)
			{
				return
				(
					(state.filter == RenderState::FILTER_2XBR || state.filter == RenderState::FILTER_3XBR || state.filter == RenderState::FILTER_4XBR )&& (
					(state.bits.count == 16 && state.bits.mask.b == 0x001F && ((state.bits.mask.g == 0x07E0 && state.bits.mask.r == 0xF800) || (state.bits.mask.g == 0x03E0 && state.bits.mask.r == 0x7C00))) ||//*/
					(state.bits.count == 32 && state.bits.mask.r == 0xFF0000 && state.bits.mask.g == 0x00FF00 && state.bits.mask.b == 0x0000FF)
					)
				);
			}

			/**
			 * 4x filtering, with blend support
			 */
			template<typename T, dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND, bool ALL, bool SOME, bool NONE>
			void Renderer::FilterxBR::Xbr4X(const Input& input,const Output& output)
			{
				#pragma region Sets up pointers to source pixels

				//Gets the pixels to filter. NST_RESTRICT tells the compiler to not alias
				//the pointer.
				const word* NST_RESTRICT src = input.pixels;

				//Size of a raster line in output
				const long pitch = (output.pitch * 3) + output.pitch - (WIDTH*4 * sizeof(T));

				//Creates a non-aliased array with four enteries. First is the destination pixels
				//cast into the type of pointer this function has been templated to use, the others
				//points at the start of the next three lines. 
				T* NST_RESTRICT dst[4] =
				{
					static_cast<T*>(output.pixels),
					reinterpret_cast<T*>(static_cast<byte*>(output.pixels) + output.pitch),
					reinterpret_cast<T*>(static_cast<byte*>(output.pixels) + output.pitch * 2),
					reinterpret_cast<T*>(static_cast<byte*>(output.pixels) + output.pitch * 3)
				};

				//const long pad = output.pitch - long(sizeof(dword) * WIDTH);
				const uint MAX_PIXELS = WIDTH * HEIGHT;

				#pragma endregion

				for (int y=0; y < MAX_PIXELS; y += WIDTH)
				{
					#pragma region Clamps y coords

					int ym1 = y - WIDTH, ym2 = y - 2*WIDTH;
					if (ym1 < 0) ym1 = y;
					if (ym2 < 0) ym2 = y;
					int y1 = y + WIDTH, y2 = y + 2*WIDTH;
					if (y1 >= MAX_PIXELS) y1 = y;
					if (y2 >= MAX_PIXELS) y2 = y;

					#pragma endregion

					for (int x=0; x < WIDTH; ++x, dst[0] += 4, dst[1] += 4, dst[2] += 4, dst[3] += 4)
					{
						#pragma region Clamps x coords

						int xm1 = x - 1, xm2 = x - 2;
						if (xm1 < 0) xm1 = 0;
						if (xm2 < 0) xm1 = 0;
						int x1 = x + 1, x2 = x + 2;
						if (x1 >= WIDTH) x1 = WIDTH - 1;
						if (x2 >= WIDTH) x2 = WIDTH - 1;

						#pragma endregion

						#pragma region Fetches pixels and converts to YUV

						//Result pixels
						YUVPixel e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, ea, eb, ec, ed, ee, ef;
						
						//Fetches pixels and converts to YUV
						YUVPixel pa, pb, pc, pd, pe, pf, pg, ph, pi, a1, b1, c1, a0, d0, g0, c4, f4, i4, g5, h5, i5;
						pa = getPixel(input.palette[src[xm1 + ym1]]);
						pb = getPixel(input.palette[src[x + ym1]]);
						pc = getPixel(input.palette[src[x1 + ym1]]);

						pd = getPixel(input.palette[src[xm1 + y]]);
						pe = e0 = e1 = e2 = e3 = e4 = e5 = e6 = e7= e8 = e9 = ea = eb = ec = ed = ee = ef = getPixel(input.palette[src[x + y]]);;
						pf = getPixel(input.palette[src[x1 + y]]);

						pg = getPixel(input.palette[src[xm1 + y1]]);
						ph = getPixel(input.palette[src[x + y1]]);
						pi = getPixel(input.palette[src[x1 + y1]]);

						a1 = getPixel(input.palette[src[xm1 + ym2]]);
						b1 = getPixel(input.palette[src[x + ym2]]);
						c1 = getPixel(input.palette[src[x1 + ym2]]);

						a0 = getPixel(input.palette[src[xm2 + ym1]]);
						d0 = getPixel(input.palette[src[xm2 + y]]);
						g0 = getPixel(input.palette[src[xm2 + y1]]);

						c4 = getPixel(input.palette[src[x2 + ym1]]);
						f4 = getPixel(input.palette[src[x2 + y]]);
						i4 = getPixel(input.palette[src[x2 + y1]]);

						g5 = getPixel(input.palette[src[xm1 + y2]]);
						h5 = getPixel(input.palette[src[x + y2]]);
						i5 = getPixel(input.palette[src[x1 + y2]]);

						#pragma endregion

						#pragma region Filters pixel

						Kernel4X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND, ALL, SOME, NONE>(pe, pi, ph, pf, pg, pc, pd, pb, f4, i4, h5, i5, ef, ee, eb, e3, e7, ea, ed, ec);
						Kernel4X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND, ALL, SOME, NONE>(pe, pc, pf, pb, pi, pa, ph, pd, b1, c1, f4, c4, e3, e7, e2, e0, e1, e6, eb, ef);
						Kernel4X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND, ALL, SOME, NONE>(pe, pa, pb, pd, pc, pg, pf, ph, d0, a0, b1, a1, e0, e1, e4, ec, e8, e5, e2, e3);
						Kernel4X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND, ALL, SOME, NONE>(pe, pg, pd, ph, pa, pi, pb, pf, h5, g5, d0, g0, ec, e8, ed, ef, ee, e9, e4, e0);

						#pragma endregion

						#pragma region Writes out result

						//The result is 16 pixels (as the image is expanded by 4x)

						dst[0][0] = (T) e0.rgb;
						dst[0][1] = (T) e1.rgb;
						dst[0][2] = (T) e2.rgb;
						dst[0][3] = (T) e3.rgb;
						dst[1][0] = (T) e4.rgb;
						dst[1][1] = (T) e5.rgb;
						dst[1][2] = (T) e6.rgb;
						dst[1][3] = (T) e7.rgb;
						dst[2][0] = (T) e8.rgb;
						dst[2][1] = (T) e9.rgb;
						dst[2][2] = (T) ea.rgb;
						dst[2][3] = (T) eb.rgb;
						dst[3][0] = (T) ec.rgb;
						dst[3][1] = (T) ed.rgb;
						dst[3][2] = (T) ee.rgb;
						dst[3][3] = (T) ef.rgb;

						#pragma endregion
					}

					#pragma region Moves dest to the next next line.

					dst[0] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[0]) + pitch);
					dst[1] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[1]) + pitch);
					dst[2] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[2]) + pitch);
					dst[3] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[3]) + pitch);

					#pragma endregion
				}
			}

			/**
			 * 3x filtering, with blend support
			 */
			template<typename T, dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND, bool ALL, bool SOME, bool NONE>
			void Renderer::FilterxBR::Xbr3X(const Input& input,const Output& output)
			{
				#pragma region Sets up pointers to source pixels

				//Gets the pixels to filter. NST_RESTRICT tells the compiler to not alias
				//the pointer.
				const word* NST_RESTRICT src = input.pixels;

				//Size of a raster line in output
				const long pitch = (output.pitch * 2) + output.pitch - (WIDTH*3 * sizeof(T));

				//Creates a non-aliased array with three enteries. First is the destination pixels
				//cast into the type of pointer this function has been templated to use, the others
				//points at the start of the next two lines.
				T* NST_RESTRICT dst[3] =
				{
					static_cast<T*>(output.pixels),
					reinterpret_cast<T*>(static_cast<byte*>(output.pixels) + output.pitch),
					reinterpret_cast<T*>(static_cast<byte*>(output.pixels) + output.pitch * 2)
				};

				//const long pad = output.pitch - long(sizeof(dword) * WIDTH);
				const uint MAX_PIXELS = WIDTH * HEIGHT;

				#pragma endregion

				for (int y=0; y < MAX_PIXELS; y += WIDTH)
				{
					#pragma region Clamps y coords

					int ym1 = y - WIDTH, ym2 = y - 2*WIDTH;
					if (ym1 < 0) ym1 = y;
					if (ym2 < 0) ym2 = y;
					int y1 = y + WIDTH, y2 = y + 2*WIDTH;
					if (y1 >= MAX_PIXELS) y1 = y;
					if (y2 >= MAX_PIXELS) y2 = y;

					#pragma endregion

					for (int x=0; x < WIDTH; ++x, dst[0] += 3, dst[1] += 3, dst[2] += 3)
					{
						#pragma region Clamps x coords

						int xm1 = x - 1, xm2 = x - 2;
						if (xm1 < 0) xm1 = 0;
						if (xm2 < 0) xm1 = 0;
						int x1 = x + 1, x2 = x + 2;
						if (x1 >= WIDTH) x1 = WIDTH - 1;
						if (x2 >= WIDTH) x2 = WIDTH - 1;

						#pragma endregion

						#pragma region Fetches pixels and converts to YUV

						//Result pixels
						YUVPixel e0, e1, e2, e3, e4, e5, e6, e7, e8;
						
						//Fetches pixels and converts to YUV
						YUVPixel pa, pb, pc, pd, pe, pf, pg, ph, pi, a1, b1, c1, a0, d0, g0, c4, f4, i4, g5, h5, i5;
						pa = getPixel(input.palette[src[xm1 + ym1]]);
						pb = getPixel(input.palette[src[x + ym1]]);
						pc = getPixel(input.palette[src[x1 + ym1]]);

						pd = getPixel(input.palette[src[xm1 + y]]);
						pe = e0 = e1 = e2 = e3 = e4 = e5 = e6 = e7= e8 = getPixel(input.palette[src[x + y]]);;
						pf = getPixel(input.palette[src[x1 + y]]);

						pg = getPixel(input.palette[src[xm1 + y1]]);
						ph = getPixel(input.palette[src[x + y1]]);
						pi = getPixel(input.palette[src[x1 + y1]]);

						a1 = getPixel(input.palette[src[xm1 + ym2]]);
						b1 = getPixel(input.palette[src[x + ym2]]);
						c1 = getPixel(input.palette[src[x1 + ym2]]);

						a0 = getPixel(input.palette[src[xm2 + ym1]]);
						d0 = getPixel(input.palette[src[xm2 + y]]);
						g0 = getPixel(input.palette[src[xm2 + y1]]);

						c4 = getPixel(input.palette[src[x2 + ym1]]);
						f4 = getPixel(input.palette[src[x2 + y]]);
						i4 = getPixel(input.palette[src[x2 + y1]]);

						g5 = getPixel(input.palette[src[xm1 + y2]]);
						h5 = getPixel(input.palette[src[x + y2]]);
						i5 = getPixel(input.palette[src[x1 + y2]]);

						#pragma endregion

						#pragma region Filters pixel

						Kernel3X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND, ALL, SOME, NONE>(pe, pi, ph, pf, pg, pc, pd, pb, f4, i4, h5, i5, e2, e5, e6, e7, e8);
						Kernel3X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND, ALL, SOME, NONE>(pe, pc, pf, pb, pi, pa, ph, pd, b1, c1, f4, c4, e0, e1, e8, e5, e2);
						Kernel3X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND, ALL, SOME, NONE>(pe, pa, pb, pd, pc, pg, pf, ph, d0, a0, b1, a1, e6, e3, e2, e1, e0);
						Kernel3X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND, ALL, SOME, NONE>(pe, pg, pd, ph, pa, pi, pb, pf, h5, g5, d0, g0, e8, e7, e0, e3, e6);

						#pragma endregion

						#pragma region Writes out result

						//The result is nine pixels (as the image is expanded)

						dst[0][0] = (T) e0.rgb;
						dst[0][1] = (T) e1.rgb;
						dst[0][2] = (T) e2.rgb;
						dst[1][0] = (T) e3.rgb;
						dst[1][1] = (T) e4.rgb;
						dst[1][2] = (T) e5.rgb;
						dst[2][0] = (T) e6.rgb;
						dst[2][1] = (T) e7.rgb;
						dst[2][2] = (T) e8.rgb;

						#pragma endregion
					}

					#pragma region Moves dest to the next next line.

					dst[0] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[0]) + pitch);
					dst[1] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[1]) + pitch);
					dst[2] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[2]) + pitch);

					#pragma endregion
				}
			}

			/**
			 * This function does the actual filtering. Input is "NES CLUT" stuff, output is plain
			 * 16-bit per color (RGB 565 or 555) or 32-bit per color (ARGB 0888) AFAICT
			 *
			 * Implements 2xBR
			 */
			template<typename T, dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND, bool ALL, bool SOME, bool NONE>
			void Renderer::FilterxBR::Xbr2X(const Input& input,const Output& output)
			{
				#pragma region Sets up pointers to source pixels

				//Gets the pixels to filter. NST_RESTRICT tells the compiler to not alias
				//the pointer.
				const word* NST_RESTRICT src = input.pixels;

				//Size of a raster line in output
				const long pitch = output.pitch;

				//Creates a non-aliased array with two enteries. First is the destination pixels
				//cast into the type of pointer this function has been templated to use, the other
				//points at the start of the next line.
				T* NST_RESTRICT dst[2] =
				{
					static_cast<T*>(output.pixels),
					reinterpret_cast<T*>(static_cast<byte*>(output.pixels) + pitch)
				};
				//const long pad = output.pitch - long(sizeof(dword) * WIDTH);
				const uint MAX_PIXELS = WIDTH * HEIGHT;

				#pragma endregion

				for (int y=0; y < MAX_PIXELS; y += WIDTH)
				{
					#pragma region Clamps y coords

					//Clamps y coords
					int ym1 = y - WIDTH, ym2 = y - 2*WIDTH;
					if (ym1 < 0) ym1 = y;
					if (ym2 < 0) ym2 = y;
					int y1 = y + WIDTH, y2 = y + 2*WIDTH;
					if (y1 >= MAX_PIXELS) y1 = y;
					if (y2 >= MAX_PIXELS) y2 = y;

					#pragma endregion

					for (int x=0; x < WIDTH; ++x, dst[0] += 2, dst[1] += 2)
					{
						#pragma region Clamps x coords

						//Clamps x coords
						int xm1 = x - 1, xm2 = x - 2;
						if (xm1 < 0) xm1 = 0;
						if (xm2 < 0) xm1 = 0;
						int x1 = x + 1, x2 = x + 2;
						if (x1 >= WIDTH) x1 = WIDTH - 1;
						if (x2 >= WIDTH) x2 = WIDTH - 1;

						#pragma endregion

						#pragma region Fetches pixels and converts to YUV

						//Result pixels
						YUVPixel e0, e1, e2, e3;
						
						//Fetches pixels and converts to YUV
						YUVPixel pa, pb, pc, pd, pe, pf, pg, ph, pi, a1, b1, c1, a0, d0, g0, c4, f4, i4, g5, h5, i5;
						pa = getPixel(input.palette[src[xm1 + ym1]]);
						pb = getPixel(input.palette[src[x + ym1]]);
						pc = getPixel(input.palette[src[x1 + ym1]]);

						pd = getPixel(input.palette[src[xm1 + y]]);
						pe = e0 = e1 = e2 = e3 = getPixel(input.palette[src[x + y]]);;
						pf = getPixel(input.palette[src[x1 + y]]);

						pg = getPixel(input.palette[src[xm1 + y1]]);
						ph = getPixel(input.palette[src[x + y1]]);
						pi = getPixel(input.palette[src[x1 + y1]]);

						a1 = getPixel(input.palette[src[xm1 + ym2]]);
						b1 = getPixel(input.palette[src[x + ym2]]);
						c1 = getPixel(input.palette[src[x1 + ym2]]);

						a0 = getPixel(input.palette[src[xm2 + ym1]]);
						d0 = getPixel(input.palette[src[xm2 + y]]);
						g0 = getPixel(input.palette[src[xm2 + y1]]);

						c4 = getPixel(input.palette[src[x2 + ym1]]);
						f4 = getPixel(input.palette[src[x2 + y]]);
						i4 = getPixel(input.palette[src[x2 + y1]]);

						g5 = getPixel(input.palette[src[xm1 + y2]]);
						h5 = getPixel(input.palette[src[x + y2]]);
						i5 = getPixel(input.palette[src[x1 + y2]]);

						#pragma endregion

						#pragma region Filters pixel

						Kernel2X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND, ALL, SOME, NONE>(pe, pi, ph, pf, pg, pc, pd, pb, f4, i4, h5, i5, e1, e2, e3);
						Kernel2X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND, ALL, SOME, NONE>(pe, pc, pf, pb, pi, pa, ph, pd, b1, c1, f4, c4, e0, e3, e1);
						Kernel2X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND, ALL, SOME, NONE>(pe, pa, pb, pd, pc, pg, pf, ph, d0, a0, b1, a1, e2, e1, e0);
						Kernel2X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND, ALL, SOME, NONE>(pe, pg, pd, ph, pa, pi, pb, pf, h5, g5, d0, g0, e3, e0, e2);

						#pragma endregion

						#pragma region Writes out result

						dst[0][0] = (T) e0.rgb;
						dst[0][1] = (T) e1.rgb;
						dst[1][0] = (T) e2.rgb;
						dst[1][1] = (T) e3.rgb;

						#pragma endregion
					}

					#pragma region Moves dest to the next next line.

					dst[0] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[1]) + (pitch - long(sizeof(T) * WIDTH*2)));
					dst[1] = reinterpret_cast<T*>(reinterpret_cast<byte*>(dst[0]) + pitch);

					#pragma endregion
				}
			}

			void Renderer::FilterxBR::Blit(const Input& input,const Output& output,uint)
			{
				(*this.*path)( input, output );
			}

			#pragma region Kernels

			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND, bool ALL, bool SOME, bool NONE>
			void Renderer::FilterxBR::Kernel2X(YUVPixel pe, YUVPixel pi, YUVPixel ph, YUVPixel pf, YUVPixel pg, 
				YUVPixel pc, YUVPixel pd, YUVPixel pb, YUVPixel f4, YUVPixel i4, YUVPixel h5, 
				YUVPixel i5, YUVPixel &n1, YUVPixel &n2, YUVPixel &n3)
			{
				if (!(pe != ph && pe != pf))
					return;
				uint e = (pe.YuvDifference(pc) + pe.YuvDifference(pg) + pi.YuvDifference(h5) + pi.YuvDifference(f4)) + (ph.YuvDifference(pf) << 2);
				uint i = (ph.YuvDifference(pd) + ph.YuvDifference(i5) + pf.YuvDifference(i4) + pf.YuvDifference(pb)) + (pe.YuvDifference(pi) << 2);
				YUVPixel px = (pe.YuvDifference(pf) <= pe.YuvDifference(ph)) ? pf : ph;
				
				//A
				if (NONE && ((e < i) && (!pf.isLike(pb) && !pf.isLike(pc) || !ph.isLike(pd) && !ph.isLike(pg) || pe.isLike(pi) && (!pf.isLike(f4) && !pf.isLike(i4) || !ph.isLike(h5) && !ph.isLike(i5)) || pe.isLike(pg) || pe.isLike(pc)))
	
				//B
				    || SOME && ((e < i) && (!pf.isLike(pb) && !ph.isLike(pd) || pe.isLike(pi) && (!pf.isLike(i4) && !ph.isLike(i5)) || pe.isLike(pg) || pe.isLike(pc))) 
	
				//C
					|| ALL && (e < i))
				{
					uint ke = pf.YuvDifference(pg);
					uint ki = ph.YuvDifference(pc);
					bool ex2 = (pe != pc && pb != pc);
					bool ex3 = (pe != pg && pd != pg);
					if (((ke << 1) <= ki) && ex3 || (ke >= (ki << 1)) && ex2) {
						if (BLEND)
						{
							if (((ke << 1) <= ki) && ex3)
								Left2_2X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n3, n2, px);
							if ((ke >= (ki << 1)) && ex2)
								Up2_2X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n3, n1, px);
						}else { n3 = px; }
					} else if (BLEND)
						Dia_2X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n3, px);

				} else if (BLEND && e <= i) {
					AlphaBlend64W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n3, px);
				}
			}

			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND, bool ALL, bool SOME, bool NONE>
			void Renderer::FilterxBR::Kernel3X(const YUVPixel pe, const YUVPixel pi, 
					const YUVPixel ph, const YUVPixel pf, const YUVPixel pg, 
					const YUVPixel pc, const YUVPixel pd, const YUVPixel pb, 
					const YUVPixel f4, const YUVPixel i4, const YUVPixel h5, 
					const YUVPixel i5, YUVPixel &n2, YUVPixel &n5, YUVPixel &n6,
					YUVPixel &n7, YUVPixel &n8) const
			{
				if (!(pe != ph && pe != pf)) return;
				uint e = (pe.YuvDifference(pc) + pe.YuvDifference(pg) + pi.YuvDifference(h5) + pi.YuvDifference(f4)) + (ph.YuvDifference(pf) << 2);
				uint i = (ph.YuvDifference(pd) + ph.YuvDifference(i5) + pf.YuvDifference(i4) + pf.YuvDifference(pb)) + (pe.YuvDifference(pi) << 2);

				//A
				if (NONE && ((e < i) && (!pf.isLike(pb) && !pf.isLike(pc) || !ph.isLike(pd) && !ph.isLike(pg) || pe.isLike(pi) && (!pf.isLike(f4) && !pf.isLike(i4) || !ph.isLike(h5) && !ph.isLike(i5)) || pe.isLike(pg) || pe.isLike(pc)))
	
				//B
				    || SOME && ((e < i) && (!pf.isLike(pb) && !ph.isLike(pd) || pe.isLike(pi) && (!pf.isLike(i4) && !ph.isLike(i5)) || pe.isLike(pg) || pe.isLike(pc))) 
	
				//C
					|| ALL && (e < i))
				{
					uint ke = pf.YuvDifference(pg);
					uint ki = ph.YuvDifference(pc);
					bool ex2 = (pe != pc && pb != pc);
					bool ex3 = (pe != pg && pd != pg);
					YUVPixel px = (pe.YuvDifference(pf) <= pe.YuvDifference(ph)) ? pf : ph;
					if (((ke << 1) <= ki) && ex3 && (ke >= (ki << 1)) && ex2) {
						LeftUp2_3X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND>(n7, n5, n6, n2, n8, px);
					} else if (((ke << 1) <= ki) && ex3) {
						Left2_3X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND>(n7, n5, n6, n8, px);
					} else if ((ke >= (ki << 1)) && ex2) {
						Up2_3X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND>(n5, n7, n2, n8, px);
					} else {
						Dia_3X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND>(n8, n5, n7, px);
					}
				} else if (BLEND && e <= i) {
					AlphaBlend128W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n8, (pe.YuvDifference(pf) <= pe.YuvDifference(ph)) ? pf : ph);
				}
			}
			
			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND, bool ALL, bool SOME, bool NONE>
			void Renderer::FilterxBR::Kernel4X(const YUVPixel pe, const YUVPixel pi, 
					const YUVPixel ph, const YUVPixel pf, const YUVPixel pg, 
					const YUVPixel pc, const YUVPixel pd, const YUVPixel pb, 
					const YUVPixel f4, const YUVPixel i4, const YUVPixel h5, 
					const YUVPixel i5, YUVPixel &n15, YUVPixel &n14, YUVPixel &n11, YUVPixel &n3,
					YUVPixel &n7, YUVPixel &n10, YUVPixel &n13, YUVPixel &n12) const
			{
				if (!(pe != ph && pe != pf)) return;
				uint e = (pe.YuvDifference(pc) + pe.YuvDifference(pg) + pi.YuvDifference(h5) + pi.YuvDifference(f4)) + (ph.YuvDifference(pf) << 2);
				uint i = (ph.YuvDifference(pd) + ph.YuvDifference(i5) + pf.YuvDifference(i4) + pf.YuvDifference(pb)) + (pe.YuvDifference(pi) << 2);
				YUVPixel px = (pe.YuvDifference(pf) <= pe.YuvDifference(ph)) ? pf : ph;

				//A
				if (NONE && ((e < i) && (!pf.isLike(pb) && !pf.isLike(pc) || !ph.isLike(pd) && !ph.isLike(pg) || pe.isLike(pi) && (!pf.isLike(f4) && !pf.isLike(i4) || !ph.isLike(h5) && !ph.isLike(i5)) || pe.isLike(pg) || pe.isLike(pc)))
	
				//B
				    || SOME && ((e < i) && (!pf.isLike(pb) && !ph.isLike(pd) || pe.isLike(pi) && (!pf.isLike(i4) && !ph.isLike(i5)) || pe.isLike(pg) || pe.isLike(pc))) 
	
				//C
					|| ALL && (e < i))
				{
					uint ke = pf.YuvDifference(pg);
					uint ki = ph.YuvDifference(pc);
					bool ex2 = (pe != pc && pb != pc);
					bool ex3 = (pe != pg && pd != pg);
					if (((ke << 1) <= ki) && ex3 || (ke >= (ki << 1)) && ex2) {
						if (((ke << 1) <= ki) && ex3)
							Left2_4X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND>(n15, n14, n11, n13, n12, n10, px);
						if ((ke >= (ki << 1)) && ex2)
							Up2_4X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND>(n15, n14, n11, n3, n7, n10, px);
					} else
						Dia_4X<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT, BLEND>(n15, n14, n11, px);
				} else if (BLEND && e <= i) {
					AlphaBlend128W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n15, px);
				}
			}

			#pragma endregion

			void Renderer::FilterxBR::Transform(const byte (&src)[PALETTE][3],Input::Palette& dst) const
			{
				freeCache();
				initCache();

				//Truncates colors to 15-bit
				//Idea: Insert the true colors into the YUV cache. There's no real harm and will
				//give colors almost as good as the true 32-bit solution.
				if (format.bpp == 32)
				{
					for(int i=0, ncol=0; i < PALETTE; i++, ncol++)
					{	//Converts to 1555
						dst[i] =
						(
							(src[i][0] & 0xF8) << 7 |
							(src[i][1] & 0xF8) <<  2 |
							(src[i][2] & 0xF8) >>  3
						);
						dword col = src[i][0] << 16 | src[i][1] << 8 | src[i][2];
						*_index[dst[i]] = YUVPixel::FromDWord(col);
					}
				}
				else if (format.bpp == 16)
				{
					//Removes the extra 1 bit of precision
					for(int i=0, ncol=0; i < PALETTE; i++, ncol++)
					{	//Converts to 1555
						dst[i] =
						(
							(src[i][0] & 0xF8) << 7 |
							(src[i][1] & 0xF8) <<  2 |
							(src[i][2] & 0xF8) >>  3
						);
						*_index[dst[i]] = YUVPixel::FromWord(dst[i], format.bpp);
					}
				}
				else //Assumes "Filter::Transform" spits out '1'-5-5-5
					Filter::Transform( src, dst );
			}

			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT>
			void Renderer::FilterxBR::Left2_2X(YUVPixel &n3, YUVPixel &n2, YUVPixel pixel)
			{
				AlphaBlend192W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n3, pixel);
				AlphaBlend64W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n2, pixel);
			}

			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT>
			void Renderer::FilterxBR::Up2_2X(YUVPixel &n3, YUVPixel &n1, YUVPixel pixel)
			{
				AlphaBlend192W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n3, pixel);
				AlphaBlend64W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n1, pixel);
			}

			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT>
			void Renderer::FilterxBR::Dia_2X(YUVPixel &n3, YUVPixel pixel)
			{
				AlphaBlend128W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n3, pixel);
			}

			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND>
			void Renderer::FilterxBR::LeftUp2_3X(YUVPixel &n7, YUVPixel &n5, YUVPixel &n6, YUVPixel &n2, YUVPixel &n8, const YUVPixel pixel) const
			{
				if (BLEND) {
					AlphaBlend192W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n7, pixel);
					AlphaBlend64W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n6, pixel);
				} else { n7 = pixel; }
				n5 = n7;
				n2 = n6;
				n8 = pixel;
			}

			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND>
			void Renderer::FilterxBR::LeftUp2_4X(YUVPixel &n15, YUVPixel &n14, YUVPixel &n11, YUVPixel &n13, YUVPixel &n12, YUVPixel &n10, YUVPixel &n7, YUVPixel &n3, const YUVPixel pixel) const
			{
				if (BLEND) {
					AlphaBlend192W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n13, pixel);
					AlphaBlend64W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n12, pixel);
				} else { n13 = pixel; }
				n15 = n14 = n11 = pixel;
				n10 = n3 = n12;
				n7 = n13;
			}

			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND>
			void Renderer::FilterxBR::Left2_3X(YUVPixel &n7, YUVPixel &n5, YUVPixel &n6, YUVPixel &n8, const YUVPixel pixel) const
			{
				if (BLEND) {
					AlphaBlend192W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n7, pixel);
					AlphaBlend64W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n5, pixel);
					AlphaBlend64W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n6, pixel);
				} else { n7 = pixel; }
				n8 = pixel;
			}

			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND>
			void Renderer::FilterxBR::Left2_4X(YUVPixel &n15, YUVPixel &n14, YUVPixel &n11, YUVPixel &n13, YUVPixel &n12, YUVPixel &n10, const YUVPixel pixel) const
			{
				if (BLEND)
				{
					AlphaBlend192W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n11, pixel);
					AlphaBlend192W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n13, pixel);
					AlphaBlend64W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n10, pixel);
					AlphaBlend64W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n12, pixel);
				}
				else { n11 = pixel; n13 = pixel; }
				n14 = pixel;
				n15 = pixel;
			}

			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND>
			void Renderer::FilterxBR::Up2_3X(YUVPixel &n5, YUVPixel &n7,  YUVPixel &n2,  YUVPixel &n8, const YUVPixel pixel) const
			{
				if (BLEND) {
					AlphaBlend192W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n5, pixel);
					AlphaBlend64W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n7, pixel);
					AlphaBlend64W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n2, pixel);
				} else { n5 = pixel; }
				n8 = pixel;
			}
			
			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND>
			void Renderer::FilterxBR::Up2_4X(YUVPixel &n15, YUVPixel &n14, YUVPixel &n11, YUVPixel &n3, YUVPixel &n7, YUVPixel &n10, const YUVPixel pixel) const
			{
				if (BLEND) {
					AlphaBlend192W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n14, pixel);
					AlphaBlend192W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n7, pixel);
					AlphaBlend64W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n10, pixel);
					AlphaBlend64W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n3, pixel);
				} else { n14 = pixel; n7 = pixel; }
				n11 = pixel;
				n15 = pixel;
			}

			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND>
			void Renderer::FilterxBR::Dia_3X(YUVPixel &n8, YUVPixel &n5, YUVPixel &n7, const YUVPixel pixel) const
			{
				if (BLEND) {
					AlphaBlend224W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n8, pixel);
					AlphaBlend32W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n5, pixel);
					AlphaBlend32W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n7, pixel);
				} else { n8 = pixel; }
			}

			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND>
			void Renderer::FilterxBR::Dia_4X(YUVPixel &n15, YUVPixel &n14, YUVPixel &n11, const YUVPixel pixel) const
			{
				if (BLEND) {
					AlphaBlend128W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n11, pixel);
					AlphaBlend128W<R_MASK, R_SHIFT, G_MASK, G_SHIFT, B_MASK, B_SHIFT>(n14, pixel);
				}
				n15 = pixel;
			}


			//===========================
			// Alpha blending
			// This impl. crunches down the interpolation to 15-bit, needlessly.
			//  Currently these functions works something like this:
			//  1. Two YUV pixels are fed in.
			//  2. The pixels are converted to 15-Bit RGB
			//  3. The pixels are blended
			//  4. The pixels are converted back to YUV by the getPixel function
			//
			//  New way should be something like this (Todo)
			//  1. Add together the 32-bit RGB pixels
			//  2. Return the result as a YUVPixel with the blended RGB data, and
			//     no YUV data.
			//
			//  i.e. col = ... with "& 0xFF" for 32bit and "& 0xF8" for 16/15 bit and 7/2/3 changed to R_SHIFT/G_SHIFT/B_SHIFT
			//	     dst = new YUVPixel
			//       dst.rgb = col
			//
			//  One could then take things a little further but having it return the result directly instead of packing it into
			//  a YUV struct. Would make the code easier to read too.
			//===========================

			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT>
			void Renderer::FilterxBR::AlphaBlend32W(YUVPixel &dst, const YUVPixel src) const
			{
				dword col = (((dst.getRed<R_MASK, R_SHIFT>() * 7 + src.getRed<R_MASK, R_SHIFT>()) / 8) & 0xF8) << 7 |
						   (((dst.getGreen<G_MASK, G_SHIFT>() * 7 + src.getGreen<G_MASK, G_SHIFT>()) / 8) & 0xF8) << 2 |
						   (((dst.getBlue<B_MASK, B_SHIFT>() * 7 + src.getBlue<B_MASK, B_SHIFT>()) / 8) & 0xF8) >> 3;

				dst = getPixel(col);
			}

			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT>
			void Renderer::FilterxBR::AlphaBlend64W(YUVPixel &dst, const YUVPixel src) const
			{
				dword col = (((dst.getRed<R_MASK, R_SHIFT>() * 3 + src.getRed<R_MASK, R_SHIFT>()) / 4) & 0xF8) << 7 |
						   (((dst.getGreen<G_MASK, G_SHIFT>() * 3 + src.getGreen<G_MASK, G_SHIFT>()) / 4) & 0xF8) << 2 |
						   (((dst.getBlue<B_MASK, B_SHIFT>() * 3 + src.getBlue<B_MASK, B_SHIFT>()) / 4) & 0xF8) >> 3;

				dst = getPixel(col);
			}

			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT>
			void Renderer::FilterxBR::AlphaBlend128W(YUVPixel &dst, const YUVPixel src) const
			{
				dword col = (((dst.getRed<R_MASK, R_SHIFT>() + src.getRed<R_MASK, R_SHIFT>()) / 2) & 0xF8) << 7 |
						    (((dst.getGreen<G_MASK, G_SHIFT>() + src.getGreen<G_MASK, G_SHIFT>()) / 2) & 0xF8) << 2 |
						    (((dst.getBlue<B_MASK, B_SHIFT>() + src.getBlue<B_MASK, B_SHIFT>()) / 2) & 0xF8) >> 3;

				dst = getPixel(col);
			}

			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT>
			void Renderer::FilterxBR::AlphaBlend192W(YUVPixel &dst, const YUVPixel src) const
			{
				dword col = (((dst.getRed<R_MASK, R_SHIFT>() + src.getRed<R_MASK, R_SHIFT>() *3) / 4) & 0xF8) << 7 |
						    (((dst.getGreen<G_MASK, G_SHIFT>() + src.getGreen<G_MASK, G_SHIFT>() *3) / 4) & 0xF8) << 2 |
						    (((dst.getBlue<B_MASK, B_SHIFT>() + src.getBlue<B_MASK, B_SHIFT>() *3) / 4) & 0xF8) >> 3;
				  
				dst = getPixel(col);
			}

			template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT>
			void Renderer::FilterxBR::AlphaBlend224W(YUVPixel &dst, const YUVPixel src) const
			{
				dword col = (((dst.getRed<R_MASK, R_SHIFT>() + src.getRed<R_MASK, R_SHIFT>() * 7) / 8) & 0xF8) << 7 |
						    (((dst.getGreen<G_MASK, G_SHIFT>() + src.getGreen<G_MASK, G_SHIFT>() * 7) / 8) & 0xF8) << 2 |
						    (((dst.getBlue<B_MASK, B_SHIFT>() + src.getBlue<B_MASK, B_SHIFT>() * 7) / 8) & 0xF8) >> 3;
				  
				dst = getPixel(col);
			}

			#pragma region Pixel functions

			//===========================
			// Pixel functions
			//===========================

			YUVPixel& Renderer::FilterxBR::getPixel(dword col) const
			{
				//Using a 32KB lookup cache
				return *_index[col & 0x7FFF];
			}

			/**
			 * Creates a YUV pixel from a 15-bit RGB color. 
			 */
			YUVPixel YUVPixel::FromWord(const dword num, int bpp)
			{
				int r = (num & 0x7C00) >> 7;
				int g = (num & 0x3E0) >> 2;
				int b = (num & 0x1F) << 3;

				//Note, using the same formula as ImageResizer. Note that ImageResizer
				//caches the YUV calculations, that might be a good idea to do here too.
				YUVPixel px;
				dword Luminance = r * 0.299f + g * 0.587f + b * 0.114f; //Alt: (0.257 * R) + (0.504 * G) + (0.098 * B) + 16 
				dword ChrominanceU = 127.5f + r * 0.5f - g * 0.418688f - b * 0.081312f; //Alt: -(0.148 * R) - (0.291 * G) + (0.439 * B) + 128
				dword ChrominanceV = 127.5f - r * 0.168736f - g * 0.331264f + b * 0.5f; //Alt: (0.439 * R) - (0.368 * G) - (0.071 * B) + 128 
				px.yuv = Luminance << 16 | ChrominanceU << 8 | ChrominanceV;

				if (bpp == 32)
				{
					//Rounds colors towards dark/light
					if (r > 127) r |=  7;
					if (g > 127) g |=  7;
					if (b > 127) b |=  7;

					px.rgb = r << 16 | g << 8 | b << 0;
				}
				else if (bpp == 16)
				{
					if (g > 127) g |=  4;
					px.rgb = r << 8 | g << 3 | b >> 3;
				}
				else
				{	//Todo: is RGB555 1-5-5-5? That's what this assumes
					px.rgb = r << 7 | g << 2 | b >> 3;
				}

				return px;
			}

			/**
			 * Creates a YUV pixel from a 32-bit RGB color. 
			 */
			YUVPixel YUVPixel::FromDWord(const dword num)
			{
				int r = (num & 0xFF0000) >> 16;
				int g = (num & 0xFF00) >> 8;
				int b = (num & 0xFF) >> 0;

				YUVPixel px;
				dword Luminance = r * 0.299f + g * 0.587f + b * 0.114f; //Alt: (0.257 * R) + (0.504 * G) + (0.098 * B) + 16 
				dword ChrominanceU = 127.5f + r * 0.5f - g * 0.418688f - b * 0.081312f; //Alt: -(0.148 * R) - (0.291 * G) + (0.439 * B) + 128
				dword ChrominanceV = 127.5f - r * 0.168736f - g * 0.331264f + b * 0.5f; //Alt: (0.439 * R) - (0.368 * G) - (0.071 * B) + 128 
				px.yuv = Luminance << 16 | ChrominanceU << 8 | ChrominanceV;

				//Composes final output pixel
				px.rgb = r << 16 | g << 8 | b << 0;

				return px;
			}

			uint YUVPixel::YuvDifference(YUVPixel px) const
			{
				return (uint)(
					48 * abs(getY() - px.getY())
					+ 6 * abs(getV() - px.getV())
					+ 7 * abs(getU() - px.getU())
				);

				//return abs((int)(yuv - px.yuv));
			}

			/**
			 * Tests if the pixels are similar looking
			 */
			bool YUVPixel::isLike(YUVPixel px) const
			{
			  return rgb == px.rgb;
				//return abs((int)(yuv - px.yuv)) < 155;
			}

			template<dword R_MASK, dword R_SHIFT> byte YUVPixel::getRed() const   { return (rgb & R_MASK) >> R_SHIFT; }
			template<dword G_MASK, dword G_SHIFT> byte YUVPixel::getGreen() const { return (rgb & G_MASK) >> G_SHIFT; }
			template<dword B_MASK, dword B_SHIFT> byte YUVPixel::getBlue() const  { return (rgb & B_MASK) << B_SHIFT; }
			byte YUVPixel::getY() const { return (yuv & 0xFF0000) >> 16; }
			byte YUVPixel::getU() const { return (yuv & 0xFF00) >> 8; }
			byte YUVPixel::getV() const { return (yuv & 0xFF) >> 0; }

			bool YUVPixel::operator==(const YUVPixel &px) const
			{
				return rgb == px.rgb;
			}

			bool YUVPixel::operator!=(const YUVPixel &px) const
			{
				return rgb != px.rgb;
			}

			#pragma endregion
		}
	}
}
