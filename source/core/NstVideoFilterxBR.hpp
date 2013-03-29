#ifndef NST_VIDEO_FILTER_XBR_H
#define NST_VIDEO_FILTER_XBR_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			struct YUVPixel
			{
			public:
				dword rgb; //<-- Stores a RGB pixel in output format.
				dword yuv;

				inline static YUVPixel FromWord(const dword num, int bpp);
				inline static YUVPixel FromDWord(const dword num);
				inline bool isLike(YUVPixel px) const;

				/**
				 * Gets a 15-bit rgb number. (Only used as debuging aid)
				 */
				template<dword R_MASK, dword R_SHIFT> inline byte getRed() const;
				template<dword G_MASK, dword G_SHIFT> inline byte getGreen() const;
				template<dword B_MASK, dword B_SHIFT> inline byte getBlue() const;
				inline byte getY() const;
				inline byte getU() const;
				inline byte getV() const;

				inline bool operator==(const YUVPixel &rhs) const;
				inline bool operator!=(const YUVPixel &rhs) const;

				inline uint YuvDifference(YUVPixel px) const;
			};

			class Renderer::FilterxBR : public Renderer::Filter
			{
			public:

				explicit FilterxBR(const RenderState&, const bool blend, const schar corner_rounding);

				static bool Check(const RenderState&);

			private:
				~FilterxBR();
				void freeCache() const;
				void initCache() const;

				typedef void (FilterxBR::*Path)(const Input&,const Output&);
				static Path GetPath(const RenderState&, const bool blend, const schar corner_rounding);

				void Blit(const Input&,const Output&,uint);
				void Transform(const byte (&)[PALETTE][3],Input::Palette&) const;

				template<typename T, dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND, bool ALL, bool SOME, bool NONE>
					void Xbr4X(const Input&,const Output&);

				template<typename T, dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND, bool ALL, bool SOME, bool NONE>
					void Xbr3X(const Input&,const Output&);

				template<typename T, dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND, bool ALL, bool SOME, bool NONE> 
					void Xbr2X(const Input&,const Output&);

				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND, bool ALL, bool SOME, bool NONE>
				inline void Kernel2X(YUVPixel pe, YUVPixel pi, YUVPixel ph, YUVPixel pf, YUVPixel pg, 
					YUVPixel pc, YUVPixel pd, YUVPixel pb, YUVPixel f4, YUVPixel i4, YUVPixel h5, 
					YUVPixel i5, YUVPixel &n1, YUVPixel &n2, YUVPixel &n3);

				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND, bool ALL, bool SOME, bool NONE>
				inline void Kernel3X(const YUVPixel pe, const YUVPixel pi, 
					const YUVPixel ph, const YUVPixel pf, const YUVPixel pg, 
					const YUVPixel pc, const YUVPixel pd, const YUVPixel pb, 
					const YUVPixel f4, const YUVPixel i4, const YUVPixel h5, 
					const YUVPixel i5, YUVPixel &n2, YUVPixel &n5, YUVPixel &n6,
					YUVPixel &n7, YUVPixel &n8) const;
				
				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND, bool ALL, bool SOME, bool NONE>
				inline void Kernel4X(const YUVPixel pe, const YUVPixel pi, 
					const YUVPixel ph, const YUVPixel pf, const YUVPixel pg, 
					const YUVPixel pc, const YUVPixel pd, const YUVPixel pb, 
					const YUVPixel f4, const YUVPixel i4, const YUVPixel h5, 
					const YUVPixel i5, YUVPixel &n15, YUVPixel &n14, YUVPixel &n11, YUVPixel &n3,
					YUVPixel &n7, YUVPixel &n10, YUVPixel &n13, YUVPixel &n12) const;

				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT>
				inline void Left2_2X(YUVPixel &n3, YUVPixel &n2, YUVPixel pixel);

				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND>
				inline void LeftUp2_3X(YUVPixel &n7, YUVPixel &n5, YUVPixel &n6, YUVPixel &n2, YUVPixel &n8, const YUVPixel pixel) const;

				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND>
				inline void LeftUp2_4X(YUVPixel &n15, YUVPixel &n14, YUVPixel &n11, YUVPixel &n13, YUVPixel &n12, YUVPixel &n10, YUVPixel &n7, YUVPixel &n3, const YUVPixel pixel) const;

				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND>
				inline void Left2_3X(YUVPixel &n7, YUVPixel &n5, YUVPixel &n6, YUVPixel &n8, const YUVPixel pixel) const;

				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND>
				inline void Left2_4X(YUVPixel &n15, YUVPixel &n14, YUVPixel &n11, YUVPixel &n13, YUVPixel &n12, YUVPixel &n10, const YUVPixel pixel) const;

				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT>
				inline void Up2_2X(YUVPixel &n3, YUVPixel &n1, YUVPixel pixel);

				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND>
				inline void Up2_3X(YUVPixel &n5, YUVPixel &n6,  YUVPixel &n2,  YUVPixel &n8, const YUVPixel pixel) const;

				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND>
				inline void Up2_4X(YUVPixel &n15, YUVPixel &n14, YUVPixel &n11, YUVPixel &n3, YUVPixel &n7, YUVPixel &n10, const YUVPixel pixel) const;

				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT>
				inline void Dia_2X(YUVPixel &n3, YUVPixel pixel);

				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND>
				inline void Dia_3X(YUVPixel &n8, YUVPixel &n5, YUVPixel &n7, const YUVPixel pixel) const;

				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT, bool BLEND>
				inline void Dia_4X(YUVPixel &n15, YUVPixel &n14, YUVPixel &n11, const YUVPixel pixel) const;

				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT>
				inline void AlphaBlend32W(YUVPixel &dst, const YUVPixel src) const;

				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT>
				inline void AlphaBlend64W(YUVPixel &dst, const YUVPixel src) const;

				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT>
				inline void AlphaBlend128W(YUVPixel &dst, const YUVPixel src) const;

				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT>
				inline void AlphaBlend192W(YUVPixel &dst, const YUVPixel src) const;

				template<dword R_MASK, dword R_SHIFT, dword G_MASK, dword G_SHIFT, dword B_MASK, dword B_SHIFT>
				inline void AlphaBlend224W(YUVPixel &dst, const YUVPixel src) const;

				inline YUVPixel& getPixel(dword col) const;

				//YUV cache. It works like this:
				//There's a 32KB lookup table where each index corresponds with a 15-bit
				//RGB color. This means one can convert a RGB color to YUV by making a
				//lookup in this table.
				//
				//For 32-bit RGB colors one have to reduce the color to 15-bit before
				//doing the lookup.
				// 
				YUVPixel** _index;

				//Whenever to blend pixels or not. Unblended give a crisper but jagged image
				const bool _blend;

				//Execution path
				const Path path;
			};
		}
	}
}

#endif
