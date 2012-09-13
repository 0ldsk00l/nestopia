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

#ifndef NST_DIRECTX_DIRECT2D_H
#define NST_DIRECTX_DIRECT2D_H

#pragma once

#include <set>
#include <vector>
#include "NstWindowRect.hpp"
#include "NstDirectX.hpp"

#ifdef NST_DEBUG
#define D3D_DEBUG_INFO
#endif

#include <d3d9.h>
#include <d3dx9.h>

namespace Nestopia
{
	namespace DirectX
	{
		class Direct2D
		{
		public:

			explicit Direct2D(HWND);
			~Direct2D();

			typedef Window::Rect Rect;
			typedef Window::Point Point;

			enum
			{
				MAX_SCANLINES = 100
			};

			struct Mode
			{
				typedef std::set<uchar> Rates;

				explicit Mode(uint=0,uint=0,uint=0);

				enum
				{
					MIN_WIDTH = 256,
					MIN_HEIGHT = 240,
					DEFAULT_RATE = 60,
					MAX_RATE = 120
				};

				bool operator == (const Mode&) const;
				bool operator <  (const Mode&) const;

				uint width, height, bpp;
				Rates rates;

				bool operator != (const Mode& mode) const
				{
					return !(*this == mode);
				}

				Point Size() const
				{
					return Point( width, height );
				}
			};

			struct Adapter : BaseAdapter
			{
				typedef std::set<Mode> Modes;

				enum DeviceType
				{
					DEVICE_HAL,
					DEVICE_HEL
				};

				enum Filter
				{
					FILTER_NONE,
					FILTER_BILINEAR
				};

				/** used to identify multiple monitors on the same video adapter */
				uint guidIndex;
				uint ordinal;
				DeviceType deviceType;
				Point maxScreenSize;
				bool videoMemScreen;
				bool anyTextureSize;
				bool canDoScanlineEffect;
				bool intervalTwo;
				bool intervalThree;
				bool intervalFour;
				bool modern;
				uint filters;
				Modes modes;
			};

			typedef std::vector<Adapter> Adapters;

			enum
			{
				RENDER_PICTURE = 0x01,
				RENDER_FPS     = 0x04,
				RENDER_MSG     = 0x08,
				RENDER_NFO     = 0x10
			};

			void SelectAdapter(const Adapters::const_iterator);
			void RenderScreen(uint);
			bool CanSwitchFullscreen(const Adapter::Modes::const_iterator) const;
			bool SwitchFullscreen(const Adapter::Modes::const_iterator);
			bool SwitchWindowed();
			void UpdateWindowView();
			void UpdateWindowView(const Point&,const Rect&,uint,int,Adapter::Filter,bool);
			void UpdateFullscreenView(const Rect&,const Point&,const Rect&,uint,int,Adapter::Filter,bool);
			void UpdateFrameRate(uint,bool,bool);
			void EnableDialogBoxMode(bool);
			bool Repair();

			enum ScreenShotResult
			{
				SCREENSHOT_OK,
				SCREENSHOT_UNSUPPORTED,
				SCREENSHOT_ERROR
			};

			ScreenShotResult SaveScreenShot(wcstring,uint) const;

		private:

			enum
			{
				INVALID_RECT = MAKE_HRESULT(SEVERITY_ERROR,0x123,2),
				TSL_PATCHES = 32
			};

			void FlushObjects();
			void InvalidateObjects();
			void ValidateObjects();

			class Base
			{
			public:

				Base();
				~Base();

				inline operator IDirect3D9& () const;

				static uint FormatToBpp(D3DFORMAT);
				static void FormatToMask(D3DFORMAT,ulong&,ulong&,ulong&);

			private:

				static IDirect3D9& Create();
				static const Adapters EnumerateAdapters(IDirect3D9&);

				IDirect3D9& com;
				const Adapters adapters;

			public:

				const Adapters& GetAdapters() const
				{
					return adapters;
				}

				const Adapter& GetAdapter(uint i) const
				{
					NST_ASSERT( i < adapters.size() );
					return adapters[i];
				}
			};

			class Device
			{
			public:

				Device(HWND,const Base&);

				void Create(IDirect3D9&,const Adapter&);

				bool CanSwitchFullscreen(const Mode&) const;
				bool CanToggleDialogBoxMode(bool) const;
				bool ResetFrameRate(uint,bool,bool,const Base&);
				uint GetMaxMessageLength() const;

				void SwitchFullscreen(const Mode&);
				void SwitchWindowed();

				NST_SINGLE_CALL HRESULT RenderScreen(uint,uint,uint) const;

				HRESULT ResetWindowClient(const Point&,HRESULT);
				HRESULT ToggleDialogBoxMode();
				HRESULT Repair(HRESULT);
				HRESULT Reset();

				inline operator IDirect3DDevice9& () const;

			private:

				void  Prepare() const;
				void  LogDisplaySwitch() const;
				uint  GetRefreshRate() const;
				DWORD GetPresentationFlags() const;
				D3DSWAPEFFECT GetSwapEffect() const;
				uint  GetDesiredPresentationRate(const Mode&) const;
				DWORD GetDesiredPresentationInterval(uint) const;
				DWORD GetDesiredPresentationInterval() const;
				bool  GetDisplayMode(D3DDISPLAYMODE&) const;

				struct Timing
				{
					Timing();

					bool autoHz;
					bool vsync;
					bool tripleBuffering;
					uchar frameRate;
				};

				class Fonts
				{
				public:

					Fonts();

					void Create(const Device&);
					void Destroy(bool);
					void OnReset() const;
					void OnLost() const;

					NST_SINGLE_CALL void Render(const D3DPRESENT_PARAMETERS&,uint) const;

				private:

					class Font
					{
					public:

						void Create(const Device&);
						void Destroy();
						void Update(const GenericString&);
						uint Width() const;
						void OnReset() const;
						void OnLost() const;

						inline bool CanDraw() const;
						inline void Draw(D3DCOLOR,DWORD,Rect) const;

					private:

						ComInterface<ID3DXFont> com;
						HeapString string;
						uint length;

					public:

						void Clear()
						{
							length = 0;
						}
					};

					Font fps;
					Font msg;
					Font nfo;
					uint width;

				public:

					void UpdateFps(const GenericString& string)
					{
						fps.Update( string );
					}

					void ClearFps()
					{
						fps.Clear();
					}

					void UpdateMsg(const GenericString& string)
					{
						msg.Update( string );
					}

					void ClearMsg()
					{
						msg.Clear();
					}

					void UpdateNfo(const GenericString& string)
					{
						nfo.Update( string );
					}

					void ClearNfo()
					{
						nfo.Clear();
					}

					uint Width() const
					{
						return width;
					}
				};

				ComInterface<IDirect3DDevice9> com;
				Timing timing;
				Fonts fonts;
				uchar ordinal;
				uchar intervalTwo;
				uchar intervalThree;
				uchar intervalFour;
				D3DPRESENT_PARAMETERS presentation;
				bool dialogBoxMode;

			public:

				uint GetOrdinal() const
				{
					return ordinal;
				}

				HRESULT ClearScreen() const
				{
					return com->Clear( 0, NULL, D3DCLEAR_TARGET, 0, 1.f, 0 );
				}

				HRESULT PresentScreen() const
				{
					return com->Present( NULL, NULL, NULL, NULL );
				}

				const D3DPRESENT_PARAMETERS& GetPresentation() const
				{
					return presentation;
				}

				void DrawFps(const GenericString& string)
				{
					fonts.UpdateFps( string );
				}

				void ClearFps()
				{
					fonts.ClearFps();
				}

				void DrawMsg(const GenericString& string)
				{
					fonts.UpdateMsg( string );
				}

				void ClearMsg()
				{
					fonts.ClearMsg();
				}

				void DrawNfo(const GenericString& string)
				{
					fonts.UpdateNfo( string );
				}

				void ClearNfo()
				{
					fonts.ClearNfo();
				}

				void EnableAutoFrequency(bool enable)
				{
					timing.autoHz = enable;
				}

				bool ThrottleRequired(uint speed) const
				{
					return
					(
						presentation.PresentationInterval == D3DPRESENT_INTERVAL_IMMEDIATE ||
						speed != timing.frameRate
					);
				}

				bool SmoothFrameRate() const
				{
					return presentation.PresentationInterval != D3DPRESENT_INTERVAL_IMMEDIATE;
				}
			};

			class Textures
			{
			public:

				explicit Textures(D3DFORMAT);

				void Update(const Adapter&,const Point&,uint,Adapter::Filter,bool);
				HRESULT Validate(IDirect3DDevice9&,const Adapter&,D3DFORMAT);
				void Invalidate();
				void Flush();
				bool SaveToFile(wcstring,D3DXIMAGE_FILEFORMAT) const;

				inline double GetScreenLeftU(uint) const;
				inline double GetScreenRightU(uint) const;
				inline double GetScreenTopV(uint) const;
				inline double GetScreenBottomV(uint) const;
				inline double GetEffectLeftU(uint) const;
				inline double GetEffectRightU(uint) const;
				inline double GetEffectTopV(uint) const;
				inline double GetEffectBottomV(uint) const;

			private:

				class Texture : public ImplicitBool<Texture>
				{
				public:

					void Invalidate();

				protected:

					explicit Texture(uint,D3DFORMAT);
					~Texture();

					bool Validate(IDirect3DDevice9&,D3DFORMAT,bool);

					static uint GetSquared(const Point&);

					ComInterface<IDirect3DTexture9> com;
					Object::Pod<D3DSURFACE_DESC> desc;
					Point size;
					const uint stage;

				public:

					bool operator ! () const
					{
						return !com;
					}

					void GetBitMask(ulong& r,ulong& g,ulong& b) const
					{
						Base::FormatToMask( desc.Format, r, g, b );
					}

					uint GetBitsPerPixel() const
					{
						return Base::FormatToBpp( desc.Format );
					}

					NST_FORCE_INLINE HRESULT Lock(D3DLOCKED_RECT& lockedRect) const
					{
						const RECT rect = {0,0,size.x,size.y};

						if (com)
						{
							return com->LockRect
							(
								0,
								&lockedRect,
								(desc.Usage & D3DUSAGE_DYNAMIC) ? NULL : &rect,
								(desc.Usage & D3DUSAGE_DYNAMIC) ? (D3DLOCK_DISCARD|D3DLOCK_NOSYSLOCK) : D3DLOCK_NOSYSLOCK
							);
						}
						else
						{
							return D3DERR_DEVICELOST;
						}
					}

					NST_FORCE_INLINE void Unlock() const
					{
						com->UnlockRect( 0 );
					}
				};

				class ScreenTexture : public Texture
				{
				public:

					explicit ScreenTexture(D3DFORMAT);

					void Update(const Adapter&,Point,bool);
					void Flush();
					bool Validate(IDirect3DDevice9&,D3DFORMAT,const Adapter&);
					bool SaveToFile(wcstring,D3DXIMAGE_FILEFORMAT) const;

					inline double GetLeftU(uint) const;
					inline double GetRightU(uint) const;
					inline double GetTopV(uint) const;
					inline double GetBottomV(uint) const;

				private:

					bool useVidMem;
				};

				class EffectTexture : public Texture
				{
				public:

					explicit EffectTexture(D3DFORMAT);

					void Update(const Adapter&,Point,uint);
					bool Validate(IDirect3DDevice9&,D3DFORMAT);

					inline double GetLeftU(uint) const;
					inline double GetRightU(uint) const;
					inline double GetTopV(uint) const;
					inline double GetBottomV(uint) const;

				private:

					uint scanlines;
					bool dirty;
				};

				ScreenTexture screenTexture;
				EffectTexture effectTexture;
				Adapter::Filter filter;

			public:

				NST_FORCE_INLINE HRESULT LockScreen(void*& data,long& pitch) const
				{
					D3DLOCKED_RECT lockedRect;
					const HRESULT hResult = screenTexture.Lock( lockedRect );

					if (SUCCEEDED(hResult))
					{
						data = lockedRect.pBits;
						pitch = lockedRect.Pitch;
					}

					return hResult;
				}

				NST_FORCE_INLINE void UnlockScreen() const
				{
					screenTexture.Unlock();
				}

				void GetScreenBitMask(ulong& r,ulong& g,ulong& b) const
				{
					screenTexture.GetBitMask( r, g, b );
				}

				uint GetScreenBitsPerPixel() const
				{
					return screenTexture.GetBitsPerPixel();
				}
			};

			class VertexBuffer
			{
			public:

				VertexBuffer();
				~VertexBuffer();

				void Update(const Rect&,const Rect&,int);
				HRESULT Validate(IDirect3DDevice9&,const Textures&);
				void Invalidate();

				inline uint NumVertices() const;

			private:

				enum
				{
					FVF = D3DFVF_XYZRHW|D3DFVF_TEX2
				};

				#pragma pack(push,1)

				struct Vertex
				{
					float x,y,z,rhw,u0,v0,u1,v1;
				};

				#pragma pack(pop)

				NST_COMPILE_ASSERT( sizeof(Vertex) == 32 );

				ComInterface<IDirect3DVertexBuffer9> com;
				Rect rect;
				Rect clip;
				uint numVertices;
				int screenCurvature;
				bool dirty;

			public:

				const Rect& GetRect() const
				{
					return rect;
				}
			};

			class IndexBuffer
			{
			public:

				IndexBuffer();
				~IndexBuffer();

				void Update(bool);
				HRESULT Validate(IDirect3DDevice9&);
				void Invalidate();

				inline uint NumStrips() const;

			private:

				ComInterface<IDirect3DIndexBuffer9> com;
				uint numStrips;
			};

			Base base;
			Device device;
			Textures textures;
			VertexBuffer vertexBuffer;
			IndexBuffer indexBuffer;
			HRESULT lastResult;

		public:

			bool ValidScreen() const
			{
				return SUCCEEDED(lastResult);
			}

			bool Windowed() const
			{
				return device.GetPresentation().Windowed;
			}

			bool ThrottleRequired(uint speed) const
			{
				return device.ThrottleRequired( speed ) || FAILED(lastResult);
			}

			const Adapters& GetAdapters() const
			{
				return base.GetAdapters();
			}

			const Adapter& GetAdapter() const
			{
				return base.GetAdapter( device.GetOrdinal() );
			}

			uint GetBitsPerPixel() const
			{
				return textures.GetScreenBitsPerPixel();
			}

			void GetBitMask(ulong& r,ulong& g,ulong& b) const
			{
				textures.GetScreenBitMask( r, g, b );
			}

			const Rect& GetScreenRect() const
			{
				return vertexBuffer.GetRect();
			}

			NST_FORCE_INLINE bool LockScreen(void*& data,long& pitch)
			{
				if (SUCCEEDED(lastResult))
					lastResult = textures.LockScreen( data, pitch );

				return SUCCEEDED(lastResult);
			}

			NST_FORCE_INLINE void UnlockScreen() const
			{
				NST_VERIFY( SUCCEEDED(lastResult) );
				textures.UnlockScreen();
			}

			bool ClearScreen()
			{
				if (SUCCEEDED(lastResult))
				{
					lastResult = device.ClearScreen();
					return SUCCEEDED(lastResult);
				}
				else
				{
					return lastResult == INVALID_RECT;
				}
			}

			bool PresentScreen()
			{
				if (SUCCEEDED(lastResult))
				{
					lastResult = device.PresentScreen();
					return SUCCEEDED(lastResult);
				}
				else
				{
					return lastResult == INVALID_RECT;
				}
			}

			void DrawFps(const GenericString& string)
			{
				device.DrawFps( string );
			}

			void ClearFps()
			{
				device.ClearFps();
			}

			void DrawMsg(const GenericString& string)
			{
				device.DrawMsg( string );
			}

			void ClearMsg()
			{
				device.ClearMsg();
			}

			void DrawNfo(const GenericString& string)
			{
				device.DrawNfo( string );
			}

			void ClearNfo()
			{
				device.ClearNfo();
			}

			void EnableAutoFrequency(bool enable)
			{
				device.EnableAutoFrequency( enable );
			}

			uint GetMaxMessageLength() const
			{
				return device.GetMaxMessageLength();
			}

			const Point GetFullscreenDisplayMode() const
			{
				return Point( device.GetPresentation().BackBufferWidth, device.GetPresentation().BackBufferHeight );
			}

			bool ModernGPU() const
			{
				return base.GetAdapters()[device.GetOrdinal()].modern;
			}

			bool SmoothFrameRate() const
			{
				return device.SmoothFrameRate();
			}
		};
	}
}

#endif
