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

#include "language/resource.h"
#include "NstIoLog.hpp"
#include "NstApplicationException.hpp"
#include "NstDirect2D.hpp"
#include "NstIoScreen.hpp"

#if NST_MSVC
#pragma comment(lib,"d3d9")
#pragma comment(lib,"d3dx9")
#endif

namespace Nestopia
{
	namespace DirectX
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		inline Direct2D::Base::operator IDirect3D9& () const
		{
			return com;
		}

		inline Direct2D::Device::operator IDirect3DDevice9& () const
		{
			return **com;
		}

		inline bool Direct2D::Device::Fonts::Font::CanDraw() const
		{
			return length && com;
		}

		inline uint Direct2D::VertexBuffer::NumVertices() const
		{
			return numVertices;
		}

		inline uint Direct2D::IndexBuffer::NumStrips() const
		{
			return numStrips;
		}

		inline double Direct2D::Textures::ScreenTexture::GetLeftU(uint x) const
		{
			return double(NST_MIN(size.x,x)) / desc.Width;
		}

		inline double Direct2D::Textures::ScreenTexture::GetRightU(uint x) const
		{
			return double(NST_MIN(size.x,x)) / desc.Width;
		}

		inline double Direct2D::Textures::ScreenTexture::GetTopV(uint y) const
		{
			return double(NST_MIN(size.y,y)) / desc.Height;
		}

		inline double Direct2D::Textures::ScreenTexture::GetBottomV(uint y) const
		{
			return double(NST_MIN(size.y,y)) / desc.Height;
		}

		inline double Direct2D::Textures::EffectTexture::GetLeftU(uint) const
		{
			return 0.0;
		}

		inline double Direct2D::Textures::EffectTexture::GetRightU(uint) const
		{
			return 1.0;
		}

		inline double Direct2D::Textures::EffectTexture::GetTopV(uint y) const
		{
			return double(NST_MIN(size.y,y)) / desc.Height * 2;
		}

		inline double Direct2D::Textures::EffectTexture::GetBottomV(uint y) const
		{
			return double(NST_MIN(size.y,y)) / desc.Height * 2;
		}

		inline double Direct2D::Textures::GetScreenLeftU(uint x) const
		{
			return screenTexture.GetLeftU(x);
		}

		inline double Direct2D::Textures::GetScreenRightU(uint x) const
		{
			return screenTexture.GetRightU(x);
		}

		inline double Direct2D::Textures::GetScreenTopV(uint y) const
		{
			return screenTexture.GetTopV(y);
		}

		inline double Direct2D::Textures::GetScreenBottomV(uint y) const
		{
			return screenTexture.GetBottomV(y);
		}

		inline double Direct2D::Textures::GetEffectLeftU(uint x) const
		{
			return effectTexture.GetLeftU(x);
		}

		inline double Direct2D::Textures::GetEffectRightU(uint x) const
		{
			return effectTexture.GetRightU(x);
		}

		inline double Direct2D::Textures::GetEffectTopV(uint y) const
		{
			return effectTexture.GetTopV(y);
		}

		inline double Direct2D::Textures::GetEffectBottomV(uint y) const
		{
			return effectTexture.GetBottomV(y);
		}

		inline void Direct2D::Device::Fonts::Font::Draw(const D3DCOLOR color,const DWORD flags,Rect rect) const
		{
			com->DrawText( NULL, string.Ptr(), length, &rect, flags, color );
		}

		NST_SINGLE_CALL void Direct2D::Device::Fonts::Render(const D3DPRESENT_PARAMETERS& presentation,const uint state) const
		{
			const uint width = presentation.BackBufferWidth;
			const uint height = presentation.BackBufferHeight;

			if (!presentation.Windowed)
			{
				if ((state & RENDER_FPS) && fps.CanDraw())
				{
					for (uint i=0; i < 2; ++i)
					{
						fps.Draw
						(
							i ? D3DCOLOR_ARGB(0xFF,0xA5,0xB5,0x40) : D3DCOLOR_ARGB(0xFF,0x2A,0x35,0x10),
							DT_SINGLELINE|TA_BOTTOM|TA_RIGHT|DT_NOCLIP,
							Rect(width-31,height-31,width-i-3,height-i-3)
						);
					}
				}

				if ((state & RENDER_MSG) && msg.CanDraw())
				{
					for (uint i=0; i < 2; ++i)
					{
						msg.Draw
						(
							i ? D3DCOLOR_ARGB(0xFF,0xFF,0x20,0x20) : D3DCOLOR_ARGB(0xFF,0x20,0x20,0xA0),
							DT_SINGLELINE|TA_BOTTOM|TA_LEFT|DT_NOCLIP,
							Rect(4-i,height-31,width,height-i-3)
						);
					}
				}
			}

			if ((state & RENDER_NFO) && nfo.CanDraw())
			{
				for (uint i=0; i < 2; ++i)
				{
					nfo.Draw
					(
						i ? D3DCOLOR_ARGB(0xFF,0x20,0xFF,0x20) : D3DCOLOR_ARGB(0xFF,0x20,0x60,0x20),
						TA_TOP|TA_LEFT|DT_NOCLIP,
						Rect(16-i,16-i,width,height)
					);
				}
			}
		}

		NST_SINGLE_CALL HRESULT Direct2D::Device::RenderScreen(const uint state,const uint numIndexedStrips,const uint numVertices) const
		{
			HRESULT hResult = com->BeginScene();

			if (SUCCEEDED(hResult))
			{
				if (state & RENDER_PICTURE)
				{
					if (numIndexedStrips)
						hResult = com->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, numVertices, 0, numIndexedStrips );
					else
						hResult = com->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
				}

				fonts.Render( presentation, state );
				com->EndScene();
			}

			return hResult;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		Direct2D::Direct2D(HWND hWnd)
		:
		device     ( hWnd, base ),
		textures   ( device.GetPresentation().BackBufferFormat ),
		lastResult ( D3D_OK )
		{
			ValidateObjects();
		}

		Direct2D::~Direct2D()
		{
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		void Direct2D::InvalidateObjects()
		{
			indexBuffer.Invalidate();
			vertexBuffer.Invalidate();
			textures.Invalidate();
		}

		void Direct2D::FlushObjects()
		{
			textures.Flush();
		}

		void Direct2D::ValidateObjects()
		{
			if (SUCCEEDED(lastResult))
			{
				lastResult = textures.Validate( device, GetAdapter(), device.GetPresentation().BackBufferFormat );

				if (SUCCEEDED(lastResult))
				{
					lastResult = indexBuffer.Validate( device );

					if (SUCCEEDED(lastResult))
						lastResult = vertexBuffer.Validate( device, textures );
				}
			}
		}

		void Direct2D::RenderScreen(uint state)
		{
			if (SUCCEEDED(lastResult))
				lastResult = device.RenderScreen( state, indexBuffer.NumStrips(), vertexBuffer.NumVertices() );
		}

		void Direct2D::SelectAdapter(const Adapters::const_iterator adapter)
		{
			if (device.GetOrdinal() != adapter->ordinal)
			{
				InvalidateObjects();
				device.Create( base, *adapter );
				lastResult = INVALID_RECT;
			}
		}

		bool Direct2D::CanSwitchFullscreen(const Adapter::Modes::const_iterator mode) const
		{
			return device.CanSwitchFullscreen( *mode );
		}

		bool Direct2D::SwitchFullscreen(const Adapter::Modes::const_iterator mode)
		{
			if (CanSwitchFullscreen( mode ))
			{
				FlushObjects();
				device.SwitchFullscreen( *mode );
				lastResult = D3D_OK;
				ValidateObjects();
				return true;
			}

			return false;
		}

		bool Direct2D::SwitchWindowed()
		{
			if (!device.GetPresentation().Windowed)
			{
				FlushObjects();
				device.SwitchWindowed();
				lastResult = D3D_OK;
				ValidateObjects();
				return true;
			}

			return false;
		}

		void Direct2D::EnableDialogBoxMode(const bool enable)
		{
			if (device.CanToggleDialogBoxMode( enable ))
			{
				FlushObjects();
				lastResult = device.ToggleDialogBoxMode();
				ValidateObjects();
			}
		}

		bool Direct2D::Reset()
		{
			if (FAILED(lastResult) && lastResult != INVALID_RECT)
			{
				FlushObjects();
				lastResult = device.Repair( lastResult );
				ValidateObjects();
			}

			return SUCCEEDED(lastResult);
		}

		void Direct2D::UpdateWindowView()
		{
			const Point::Picture picture( device.GetPresentation().hDeviceWindow );

			if (picture.x > 0 && picture.y > 0)
			{
				const Point::Client client( device.GetPresentation().hDeviceWindow );
				NST_ASSERT( client.x >= picture.x && client.y >= picture.y );

				if
				(
					client.x != device.GetPresentation().BackBufferWidth ||
					client.y != device.GetPresentation().BackBufferHeight ||
					lastResult == INVALID_RECT
				)
				{
					FlushObjects();
					lastResult = device.ResetWindowClient( client, lastResult );
				}

				ValidateObjects();
			}
			else
			{
				lastResult = INVALID_RECT;
			}
		}

		void Direct2D::UpdateWindowView
		(
			const Point& screen,
			const Rect& clipping,
			const uint scanlines,
			const int screenCurvature,
			const Adapter::Filter filter,
			const bool useVidMem
		)
		{
			const Point::Picture picture( device.GetPresentation().hDeviceWindow );

			if (picture.x > 0 && picture.y > 0)
			{
				textures.Update( base.GetAdapter(device.GetOrdinal()), screen, scanlines, filter, useVidMem );
				vertexBuffer.Update( picture, clipping, screenCurvature );
				indexBuffer.Update( screenCurvature );

				const Point::Client client( device.GetPresentation().hDeviceWindow );
				NST_ASSERT( client.x >= picture.x && client.y >= picture.y );

				if
				(
					client.x != device.GetPresentation().BackBufferWidth ||
					client.y != device.GetPresentation().BackBufferHeight ||
					lastResult == INVALID_RECT
				)
				{
					FlushObjects();
					lastResult = device.ResetWindowClient( client, lastResult );
				}

				ValidateObjects();
			}
			else
			{
				lastResult = INVALID_RECT;
			}
		}

		void Direct2D::UpdateFullscreenView
		(
			const Rect& picture,
			const Point& screen,
			const Rect& clipping,
			const uint scanlines,
			const int screenCurvature,
			const Adapter::Filter filter,
			const bool useVidMem
		)
		{
			NST_ASSERT( picture.Width() && picture.Height() );

			textures.Update( base.GetAdapter(device.GetOrdinal()), screen, scanlines, filter, useVidMem );
			vertexBuffer.Update( picture, clipping, screenCurvature );
			indexBuffer.Update( screenCurvature );
			ValidateObjects();
		}

		void Direct2D::UpdateFrameRate(const uint frameRate,const bool vsync,const bool tripleBuffering)
		{
			if (device.ResetFrameRate( frameRate, vsync, tripleBuffering, base ))
			{
				FlushObjects();

				if (SUCCEEDED(lastResult))
				{
					lastResult = device.Reset();
					ValidateObjects();
				}
			}
		}

		Direct2D::ScreenShotResult Direct2D::SaveScreenShot(wcstring const file,const uint ext) const
		{
			NST_ASSERT( file && *file );

			if (SUCCEEDED(lastResult))
			{
				D3DXIMAGE_FILEFORMAT format;

				switch (ext)
				{
					case MAKEFOURCC('p','n','g','\0'): format = D3DXIFF_PNG; break;
					case MAKEFOURCC('j','p','g','\0'): format = D3DXIFF_JPG; break;
					case MAKEFOURCC('b','m','p','\0'): format = D3DXIFF_BMP; break;
					default: return SCREENSHOT_UNSUPPORTED;
				}

				if (textures.SaveToFile( file, format ))
					return SCREENSHOT_OK;
			}

			return SCREENSHOT_ERROR;
		}

		Direct2D::Mode::Mode(uint w,uint h,uint b)
		: width(w), height(h), bpp(b) {}

		bool Direct2D::Mode::operator == (const Mode& mode) const
		{
			return width == mode.width && height == mode.height && bpp == mode.bpp;
		}

		bool Direct2D::Mode::operator < (const Mode& mode) const
		{
			if ( width  < mode.width  ) return true;
			if ( width  > mode.width  ) return false;
			if ( height < mode.height ) return true;
			if ( height > mode.height ) return false;
			if ( bpp    < mode.bpp    ) return true;
			if ( bpp    > mode.bpp    ) return false;

			return false;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		Direct2D::Base::Base()
		: com(Create()), adapters(EnumerateAdapters(com)) {}

		Direct2D::Base::~Base()
		{
			com.Release();
		}

		IDirect3D9& Direct2D::Base::Create()
		{
			IDirect3D9* com;

			if (NULL != (com = ::Direct3DCreate9( D3D_SDK_VERSION )))
			{
				return *com;
			}
			else if (NULL != (com = ::Direct3DCreate9( D3D9b_SDK_VERSION ))) // unofficial, it may work, it may not work
			{
				return *com;
			}
			else
			{
				throw Application::Exception( IDS_ERR_D3D_FAILED );
			}
		}

		const Direct2D::Adapters Direct2D::Base::EnumerateAdapters(IDirect3D9& d3d)
		{
			NST_COMPILE_ASSERT( D3DADAPTER_DEFAULT == 0 );

			Io::Log() << "Direct3D: initializing..\r\n";

			Adapters adapters;

			for (uint ordinal=0, numAdapters=d3d.GetAdapterCount(); ordinal < NST_MIN(numAdapters,255); ++ordinal)
			{
				D3DADAPTER_IDENTIFIER9 identifier;

				if (SUCCEEDED(d3d.GetAdapterIdentifier( ordinal, 0, &identifier )))
				{
					if (!adapters.empty() && adapters.back().guid == identifier.DeviceIdentifier)
						continue;

					Io::Log() << "Direct3D: enumerating device - name: "
                              << (*identifier.Description ? identifier.Description : "unknown")
                              << ", GUID: "
                              << System::Guid( identifier.DeviceIdentifier ).GetString()
                              << "\r\n";

					Adapter::Modes modes;

					for (uint format=0; format < 2; ++format)
					{
						const D3DFORMAT type = (format ? D3DFMT_X8R8G8B8 : D3DFMT_R5G6B5);

						for (uint mode=0, numModes=d3d.GetAdapterModeCount( ordinal, type ); mode < numModes; ++mode)
						{
							D3DDISPLAYMODE display;

							if (FAILED(d3d.EnumAdapterModes( ordinal, type, mode, &display )))
								continue;

							if (display.Width < Mode::MIN_WIDTH || display.Height < Mode::MIN_HEIGHT || display.RefreshRate > Mode::MAX_RATE)
								continue;

							// C++ standard vagueness, sometimes set::iterator == set::const_iterator
							const_cast<Mode::Rates&>(modes.insert(Mode( display.Width, display.Height, format ? 32 : 16 )).first->rates).insert( display.RefreshRate );
						}
					}

					if (modes.empty())
					{
						Io::Log() << "Direct3D: found no valid display mode, continuing enumeration..\r\n";
					}
					else
					{
						D3DCAPS9 caps;

						if (FAILED(d3d.GetDeviceCaps( ordinal, D3DDEVTYPE_HAL, &caps )))
						{
							if (FAILED(d3d.GetDeviceCaps( ordinal, D3DDEVTYPE_REF, &caps )))
							{
								Io::Log() << "Direct3D: warning, bogus device, continuing enumeration..\r\n";
								continue;
							}
							else
							{
								Io::Log() << "Direct3D: performance warning, this is a REF device only!\r\n";
							}
						}

						adapters.push_back( Adapter() );
						Adapter& adapter = adapters.back();

						adapter.guid                = identifier.DeviceIdentifier;
						adapter.name                = (*identifier.Description ? identifier.Description : "Unknown");
						adapter.ordinal             = ordinal;
						adapter.deviceType          = (caps.DeviceType != D3DDEVTYPE_REF ? Adapter::DEVICE_HAL : Adapter::DEVICE_HEL);
						adapter.maxScreenSize       = Point(caps.MaxTextureWidth,caps.MaxTextureHeight);
						adapter.videoMemScreen      = caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES;
						adapter.anyTextureSize      = (caps.TextureCaps & (D3DPTEXTURECAPS_SQUAREONLY|D3DPTEXTURECAPS_POW2|D3DPTEXTURECAPS_NONPOW2CONDITIONAL)) == 0;
						adapter.canDoScanlineEffect = (caps.MaxSimultaneousTextures >= 2) && (caps.TextureOpCaps & D3DTEXOPCAPS_MODULATE) && (caps.TextureAddressCaps & D3DPTADDRESSCAPS_WRAP);
						adapter.intervalTwo         = caps.PresentationIntervals & D3DPRESENT_INTERVAL_TWO;
						adapter.intervalThree       = caps.PresentationIntervals & D3DPRESENT_INTERVAL_THREE;
						adapter.intervalFour        = caps.PresentationIntervals & D3DPRESENT_INTERVAL_FOUR;
						adapter.filters             = 0;
						adapter.modern              = (caps.PixelShaderVersion >= D3DPS_VERSION(2,0));
						adapter.modes               = modes;

						if ((caps.TextureFilterCaps & (D3DPTFILTERCAPS_MINFLINEAR|D3DPTFILTERCAPS_MAGFLINEAR)) == (D3DPTFILTERCAPS_MINFLINEAR|D3DPTFILTERCAPS_MAGFLINEAR))
							adapter.filters |= Adapter::FILTER_BILINEAR;

						Io::Log log;

						log << "Direct3D: dynamic textures: " << (adapter.videoMemScreen ? "supported\r\n" : "unsupported\r\n")
							<< "Direct3D: texture bilinear filtering: " << ((adapter.filters & Adapter::FILTER_BILINEAR) ? "supported\r\n" : "unsupported\r\n")
							<< "Direct3D: max texture dimensions: " << caps.MaxTextureWidth << 'x' << caps.MaxTextureHeight
							<< "\r\nDirect3D: scanline effect: " << (adapter.canDoScanlineEffect ? "supported\r\n" : "unsupported\r\n")
							<< "Direct3D: vsync on every second refresh: " << (adapter.intervalTwo ? "supported\r\n" : "unsupported\r\n")
							<< "Direct3D: vsync on every third refresh: " << (adapter.intervalThree ? "supported\r\n" : "unsupported\r\n")
							<< "Direct3D: found " << modes.size() << " display modes\r\n"
							<< "Direct3D: supported monitor frequencies: ";

						Mode::Rates rates;

						for (Adapter::Modes::const_iterator it(modes.begin()), end(modes.end()); it != end; ++it)
							rates.insert( it->rates.begin(), it->rates.end() );

						for (Mode::Rates::const_iterator it(rates.begin()), end(rates.end());; )
						{
							log << uint(*it);

							if (++it != end)
							{
								log << "hz, ";
							}
							else
							{
								log << "hz\r\n";
								break;
							}
						}
					}
				}
			}

			if (adapters.empty())
				throw Application::Exception( L"Found no valid display adapter!" );

			return adapters;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		uint Direct2D::Base::FormatToBpp(const D3DFORMAT format)
		{
			switch (format)
			{
				case D3DFMT_X8R8G8B8:
				case D3DFMT_X8B8G8R8:
				case D3DFMT_A8R8G8B8:
				case D3DFMT_A8B8G8R8:
				case D3DFMT_A2R10G10B10:
				case D3DFMT_A2B10G10R10:
					return 32;

				case D3DFMT_R5G6B5:
				case D3DFMT_X1R5G5B5:
				case D3DFMT_X4R4G4B4:
				case D3DFMT_A1R5G5B5:
				case D3DFMT_A4R4G4B4:
				case D3DFMT_A8R3G3B2:
					return 16;
			}

			return 0;
		}

		void Direct2D::Base::FormatToMask(const D3DFORMAT format,ulong& r,ulong& g,ulong& b)
		{
			switch (format)
			{
				case D3DFMT_X8R8G8B8:
				case D3DFMT_A8R8G8B8:    r = 0x00FF0000; g = 0x0000FF00; b = 0x000000FF; break;
				case D3DFMT_X8B8G8R8:
				case D3DFMT_A8B8G8R8:    r = 0x000000FF; g = 0x0000FF00; b = 0x00FF0000; break;
				case D3DFMT_A2R10G10B10: r = 0x3FF00000; g = 0x000FFC00; b = 0x000003FF; break;
				case D3DFMT_A2B10G10R10: r = 0x000003FF; g = 0x000FFC00; b = 0x3FF00000; break;
				case D3DFMT_R5G6B5:      r = 0xF800;     g = 0x07E0;     b = 0x001F;     break;
				case D3DFMT_X1R5G5B5:
				case D3DFMT_A1R5G5B5:    r = 0x7C00;     g = 0x03E0;     b = 0x001F;     break;
				case D3DFMT_X4R4G4B4:
				case D3DFMT_A4R4G4B4:    r = 0x0F00;     g = 0x00F0;     b = 0x000F;     break;
				case D3DFMT_A8R3G3B2:    r = 0x00E0;     g = 0x001C;     b = 0x0003;     break;
				default:                 r = 0;          g = 0;          b = 0;          break;
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		Direct2D::Device::Device(HWND const hWnd,const Base& base)
		{
			NST_ASSERT( hWnd );

			presentation.BackBufferWidth            = 0;
			presentation.BackBufferHeight           = 0;
			presentation.BackBufferFormat           = D3DFMT_UNKNOWN;
			presentation.BackBufferCount            = 1;
			presentation.MultiSampleType            = D3DMULTISAMPLE_NONE;
			presentation.MultiSampleQuality         = 0;
			presentation.SwapEffect                 = D3DSWAPEFFECT_DISCARD;
			presentation.hDeviceWindow              = hWnd;
			presentation.Windowed                   = true;
			presentation.EnableAutoDepthStencil     = false;
			presentation.AutoDepthStencilFormat     = D3DFMT_UNKNOWN;
			presentation.Flags                      = 0;
			presentation.FullScreen_RefreshRateInHz = 0;
			presentation.PresentationInterval       = D3DPRESENT_INTERVAL_IMMEDIATE;

			dialogBoxMode = false;

			Create( base, base.GetAdapter(0) );
		}

		void Direct2D::Device::Create(IDirect3D9& d3d,const Adapter& adapter)
		{
			ordinal = adapter.ordinal;
			intervalTwo = adapter.intervalTwo;
			intervalThree = adapter.intervalThree;
			intervalFour = adapter.intervalFour;

			fonts.Destroy( true );
			com.Release();

			NST_VERIFY( !!Point::Client(presentation.hDeviceWindow) );

			uint buffers = (timing.tripleBuffering ? 2 : 1);
			presentation.BackBufferCount = buffers;
			presentation.Flags = GetPresentationFlags();
			presentation.SwapEffect = GetSwapEffect();
			DWORD flags = D3DCREATE_PUREDEVICE|D3DCREATE_HARDWARE_VERTEXPROCESSING;

			for (;;)
			{
				const HRESULT hResult = d3d.CreateDevice
				(
					adapter.ordinal,
					adapter.deviceType == Adapter::DEVICE_HAL ? D3DDEVTYPE_HAL : D3DDEVTYPE_REF,
					presentation.hDeviceWindow,
					flags,
					&presentation,
					&com
				);

				if (SUCCEEDED(hResult))
				{
					break;
				}
				else if (hResult == D3DERR_DEVICELOST)
				{
					throw Application::Exception( L"Can't start! Direct3D is busy!" );
				}
				else if (buffers != presentation.BackBufferCount)
				{
					buffers = presentation.BackBufferCount;
					Io::Log() << "Direct3D: Warning! IDirect3D9::CreateDevice() failed, retrying with one back-buffer only..\r\n";
				}
				else if (flags == (D3DCREATE_PUREDEVICE|D3DCREATE_HARDWARE_VERTEXPROCESSING))
				{
					flags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
					Io::Log() << "Direct3D: Warning! IDirect3D9::CreateDevice() failed, retrying without a pure device..\r\n";
				}
				else if (flags == D3DCREATE_HARDWARE_VERTEXPROCESSING)
				{
					flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
					Io::Log() << "Direct3D: Warning! IDirect3D9::CreateDevice() failed, retrying with software vertex processing mode..\r\n";
				}
				else
				{
					if (HDC const hdc = ::GetDC( NULL ))
					{
						const int bits = ::GetDeviceCaps( hdc, BITSPIXEL );
						::ReleaseDC( NULL, hdc );

						if (bits && bits != 16 && bits != 32)
							throw Application::Exception( IDS_ERR_BAD_BPP );
					}

					throw Application::Exception( IDS_ERR_D3D_DEVICE_FAILED );
				}
			}

			Prepare();
			fonts.Create( *this );

			Io::Log() << "Direct3D: creating "
                      << (adapter.deviceType == Adapter::DEVICE_HAL ? "HAL device #" : "REF device #")
                      << adapter.ordinal
                      << "\r\n";

			LogDisplaySwitch();
		}

		bool Direct2D::Device::GetDisplayMode(D3DDISPLAYMODE& displayMode) const
		{
			IDirect3D9* base;

			if (SUCCEEDED(com->GetDirect3D( &base )))
			{
				const HRESULT hResult = base->GetAdapterDisplayMode( ordinal, &displayMode );
				base->Release();

				if (SUCCEEDED(hResult))
					return true;
			}

			displayMode.Width = presentation.BackBufferWidth;
			displayMode.Height = presentation.BackBufferHeight;

			return false;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		DWORD Direct2D::Device::GetPresentationFlags() const
		{
			return (dialogBoxMode && !presentation.Windowed) ? D3DPRESENTFLAG_LOCKABLE_BACKBUFFER : 0;
		}

		D3DSWAPEFFECT Direct2D::Device::GetSwapEffect() const
		{
			return (dialogBoxMode && !presentation.Windowed) || (presentation.BackBufferCount > 1) ? D3DSWAPEFFECT_DISCARD : D3DSWAPEFFECT_COPY;
		}

		uint Direct2D::Device::GetDesiredPresentationRate(const Mode& mode) const
		{
			if (presentation.Windowed)
			{
				return 0;
			}
			else if (timing.autoHz)
			{
				int match = INT_MAX;
				Mode::Rates::const_iterator close(mode.rates.begin());

				for (Mode::Rates::const_iterator it(mode.rates.end()), begin(mode.rates.begin());; )
				{
					--it;

					for (uint i=5; --i; )
					{
						int diff = int(timing.frameRate * i) - int(*it);

						if (diff == 0)
							return *it;

						if (diff < 0)
							diff = int(*it) - int(timing.frameRate * i);

						if (match > diff)
						{
							match = diff;
							close = it;
						}
					}

					if (it == begin)
						break;
				}

				return *close;
			}
			else for (Mode::Rates::const_iterator it(mode.rates.begin()), end(mode.rates.end()); it != end; ++it)
			{
				if (*it == Mode::DEFAULT_RATE)
					return Mode::DEFAULT_RATE;
			}

			return 0;
		}

		DWORD Direct2D::Device::GetDesiredPresentationInterval(const uint rate) const
		{
			if (!timing.vsync || rate % timing.frameRate)
			{
				return D3DPRESENT_INTERVAL_IMMEDIATE;
			}
			else if (!presentation.Windowed)
			{
				if (timing.frameRate * 4 == rate && intervalFour)
				{
					return D3DPRESENT_INTERVAL_FOUR;
				}
				else if (timing.frameRate * 3 == rate && intervalThree)
				{
					return D3DPRESENT_INTERVAL_THREE;
				}
				else if (timing.frameRate * 2 == rate && intervalTwo)
				{
					return D3DPRESENT_INTERVAL_TWO;
				}
			}

			return timing.frameRate == rate ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
		}

		uint Direct2D::Device::GetRefreshRate() const
		{
			if (presentation.Windowed)
			{
				D3DDISPLAYMODE mode;
				return GetDisplayMode( mode ) ? mode.RefreshRate : 0;
			}
			else
			{
				return presentation.FullScreen_RefreshRateInHz;
			}
		}

		DWORD Direct2D::Device::GetDesiredPresentationInterval() const
		{
			return GetDesiredPresentationInterval( GetRefreshRate() );
		}

		HRESULT Direct2D::Device::Reset()
		{
			fonts.OnLost();

			const uint oldInterval = presentation.PresentationInterval;
			presentation.PresentationInterval = GetDesiredPresentationInterval();
			uint buffers = timing.tripleBuffering ? 2 : 1;
			presentation.BackBufferCount = buffers;
			presentation.Flags = GetPresentationFlags();
			presentation.SwapEffect = GetSwapEffect();

			for (;;)
			{
				const HRESULT hResult = com->Reset( &presentation );

				if (SUCCEEDED(hResult))
				{
					break;
				}
				else if (hResult == D3DERR_DEVICELOST)
				{
					return D3DERR_DEVICELOST;
				}
				else if (buffers != presentation.BackBufferCount)
				{
					buffers = presentation.BackBufferCount;
					Io::Log() << "Direct3D: Warning! IDirect3DDevice9::Reset() failed, retrying with one back-buffer only..\r\n";
				}
				else throw Application::Exception
				(
					IDS_ERR_FAILED,
					hResult == D3DERR_INVALIDCALL         ? L"IDirect3DDevice9::Reset() (code: D3DERR_INVALIDCALL)"         :
					hResult == D3DERR_OUTOFVIDEOMEMORY    ? L"IDirect3DDevice9::Reset() (code: D3DERR_OUTOFVIDEOMEMORY)"    :
					hResult == D3DERR_DRIVERINTERNALERROR ? L"IDirect3DDevice9::Reset() (code: D3DERR_DRIVERINTERNALERROR)" :
					hResult == E_OUTOFMEMORY              ? L"IDirect3DDevice9::Reset() (code: E_OUTOFMEMORY)"              :
															L"IDirect3DDevice9::Reset()"
				);
			}

			if (!presentation.Windowed && dialogBoxMode && FAILED(com->SetDialogBoxMode( true )))
				throw Application::Exception( IDS_ERR_FAILED, L"IDirect3DDevice9::SetDialogBoxMode()" );

			Prepare();
			fonts.OnReset();

			if (presentation.PresentationInterval != oldInterval)
			{
				Io::Log() <<
				(
					presentation.PresentationInterval == D3DPRESENT_INTERVAL_IMMEDIATE ? "Direct3D: disabling VSYNC\r\n" :
					presentation.PresentationInterval == D3DPRESENT_INTERVAL_TWO       ? "Direct3D: enabling VSYNC on second refresh\r\n" :
					presentation.PresentationInterval == D3DPRESENT_INTERVAL_THREE     ? "Direct3D: enabling VSYNC on third refresh\r\n" :
                                                                                         "Direct3D: enabling VSYNC\r\n"
				);
			}

			if (!presentation.Windowed && ::GetMenu( presentation.hDeviceWindow ))
				::DrawMenuBar( presentation.hDeviceWindow );

			return D3D_OK;
		}

		void Direct2D::Device::Prepare() const
		{
			com->SetRenderState( D3DRS_ZWRITEENABLE, false        );
			com->SetRenderState( D3DRS_COLORVERTEX,  false        );
			com->SetRenderState( D3DRS_CULLMODE,     D3DCULL_NONE );
			com->SetRenderState( D3DRS_LIGHTING,     false        );
		}

		void Direct2D::Device::LogDisplaySwitch() const
		{
			Io::Log log;
			log << "Direct3D: entering ";

			D3DDISPLAYMODE mode;

			if (GetDisplayMode( mode ))
			{
				log << mode.Width
					<< 'x'
					<< mode.Height
					<< 'x'
					<< Base::FormatToBpp(mode.Format)
					<< ' '
					<< mode.RefreshRate
					<< "hz ";
			}

			log << (presentation.Windowed ? "window mode\r\n" : "full-screen mode\r\n");
		}

		uint Direct2D::Device::GetMaxMessageLength() const
		{
			return fonts.Width() ? (presentation.BackBufferWidth - fonts.Width() * 7) / fonts.Width() : 64;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		HRESULT Direct2D::Device::Repair(const HRESULT lastError)
		{
			NST_ASSERT( FAILED(lastError) );

			uint id = 0;
			wcstring msg;

			switch (lastError)
			{
				case D3DERR_DEVICELOST:
				case D3DERR_DEVICENOTRESET:

					switch (com->TestCooperativeLevel())
					{
						case D3DERR_DEVICELOST:

							return D3DERR_DEVICELOST;

						case D3DERR_DEVICENOTRESET:

							return Reset();

						case D3DERR_DRIVERINTERNALERROR:

							msg = L"Internal video driver error! Try upgrading it!";
							break;

						default:

							id = IDS_ERR_FAILED;
							msg = L"IDirect3DDevice9::TestCooperativeLevel()";
							break;
					}

				case D3DERR_DRIVERINTERNALERROR:

					msg = L"Internal video driver error! Try upgrading it!";
					break;

				case E_OUTOFMEMORY:

					msg = L"Out of memory!";
					break;

				case D3DERR_OUTOFVIDEOMEMORY:

					msg = L"Out of video memory!";
					break;

				default:

					msg = L"Unknown Direct3D error!";
					break;
			}

			throw Application::Exception( id, msg );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		bool Direct2D::Device::CanToggleDialogBoxMode(bool enable) const
		{
			return !presentation.Windowed && dialogBoxMode != enable;
		}

		bool Direct2D::Device::CanSwitchFullscreen(const Mode& mode) const
		{
			return
			(
				presentation.Windowed ||
				presentation.BackBufferWidth != mode.width ||
				presentation.BackBufferHeight != mode.height ||
				presentation.BackBufferFormat != (mode.bpp == 16 ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8) ||
				presentation.FullScreen_RefreshRateInHz != GetDesiredPresentationRate( mode )
			);
		}

		void Direct2D::Device::SwitchFullscreen(const Mode& mode)
		{
			presentation.Windowed = false;
			presentation.BackBufferWidth = mode.width;
			presentation.BackBufferHeight = mode.height;
			presentation.BackBufferFormat = (mode.bpp == 16 ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8);
			presentation.FullScreen_RefreshRateInHz = GetDesiredPresentationRate( mode );

			if (FAILED(Reset()))
				throw Application::Exception( L"Couldn't switch display mode!" );

			fonts.Create( *this );
			LogDisplaySwitch();
		}

		HRESULT Direct2D::Device::ToggleDialogBoxMode()
		{
			NST_ASSERT( !presentation.Windowed );

			if (dialogBoxMode)
			{
				dialogBoxMode = false;
				com->SetDialogBoxMode( false );
			}
			else
			{
				dialogBoxMode = true;
			}

			return Reset();
		}

		void Direct2D::Device::SwitchWindowed()
		{
			fonts.Destroy( false );

			presentation.Windowed = true;
			presentation.BackBufferWidth = 0;
			presentation.BackBufferHeight = 0;
			presentation.BackBufferFormat = D3DFMT_UNKNOWN;
			presentation.FullScreen_RefreshRateInHz = 0;

			if (dialogBoxMode)
			{
				dialogBoxMode = false;
				com->SetDialogBoxMode( false );
			}

			if (FAILED(Reset()))
				throw Application::Exception( L"Couldn't switch display mode!" );

			fonts.Create( *this );
			LogDisplaySwitch();
		}

		HRESULT Direct2D::Device::ResetWindowClient(const Point& client,HRESULT hResult)
		{
			NST_ASSERT( presentation.Windowed && client.x > 0 && client.y > 0 );

			presentation.BackBufferWidth = client.x;
			presentation.BackBufferHeight = client.y;

			if (SUCCEEDED(hResult) || hResult == INVALID_RECT)
				hResult = Reset();

			return hResult;
		}

		bool Direct2D::Device::ResetFrameRate(uint frameRate,bool vsync,bool tripleBuffering,const Base& base)
		{
			timing.frameRate = frameRate;
			timing.vsync = vsync;

			bool update = false;

			if (timing.tripleBuffering != tripleBuffering)
			{
				timing.tripleBuffering = tripleBuffering;
				update = true;
			}

			if (!presentation.Windowed)
			{
				const Mode mode
				(
					presentation.BackBufferWidth,
					presentation.BackBufferHeight,
					presentation.BackBufferFormat == D3DFMT_X8R8G8B8 ? 32 : 16
				);

				frameRate = GetDesiredPresentationRate( *base.GetAdapter(ordinal).modes.find(mode) );

				if (presentation.FullScreen_RefreshRateInHz != frameRate)
				{
					presentation.FullScreen_RefreshRateInHz = frameRate;
					update = true;
				}
			}

			return update || presentation.PresentationInterval != GetDesiredPresentationInterval();
		}

		Direct2D::Device::Fonts::Fonts()
		: width(0) {}

		void Direct2D::Device::Fonts::Font::Create(const Device& device)
		{
			wcstring fontName = L"System";
			uint fontHeight = 12;

			D3DDISPLAYMODE mode;
			device.GetDisplayMode( mode );

			if (mode.Width > 320 && mode.Height > 240)
			{
				fontHeight = mode.Height / (device.presentation.Windowed ? 32 : 16);

				switch (PRIMARYLANGID(::GetUserDefaultLangID()))
				{
					case LANG_JAPANESE: fontName = L"MS Gothic"; break;
					case LANG_CHINESE:  fontName = L"MS Hei";    break;
					case LANG_KOREAN:   fontName = L"GulimChe";  break;
					default:            fontName = L"Arial";     break;
				}
			}

			if (com)
			{
				Object::Pod<D3DXFONT_DESC> desc;
				com->GetDesc( &desc );

				if (desc.Height == int(fontHeight) && bool(desc.Width > 320 && desc.Height > 240) == bool(mode.Width > 320 && mode.Height > 240))
					return;

				com.Release();
			}

			::D3DXCreateFont
			(
				*device.com,
				fontHeight,
				0,
				FW_NORMAL,
				1,
				false,
				DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS,
				DEFAULT_QUALITY,
				DEFAULT_PITCH|FF_DONTCARE,
				fontName,
				&com
			);
		}

		uint Direct2D::Device::Fonts::Font::Width() const
		{
			TEXTMETRIC metric;

			if (com && SUCCEEDED(com->GetTextMetrics( &metric )))
				return metric.tmAveCharWidth;
			else
				return 0;
		}

		void Direct2D::Device::Fonts::Font::Destroy()
		{
			length = 0;
			com.Release();
		}

		void Direct2D::Device::Fonts::Font::OnReset() const
		{
			if (com)
				com->OnResetDevice();
		}

		void Direct2D::Device::Fonts::Font::OnLost() const
		{
			if (com)
				com->OnLostDevice();
		}

		void Direct2D::Device::Fonts::Font::Update(const GenericString& newstring)
		{
			string = newstring;
			length = newstring.Length();

			if (length && com)
				com->PreloadText( string.Ptr(), string.Length() );
		}

		void Direct2D::Device::Fonts::Create(const Device& device)
		{
			nfo.Create( device );

			if (!device.presentation.Windowed)
			{
				fps.Create( device );
				msg.Create( device );
			}

			width = nfo.Width();
		}

		void Direct2D::Device::Fonts::Destroy(const bool newDevice)
		{
			width = 0;

			fps.Destroy();
			msg.Destroy();

			if (newDevice)
				nfo.Destroy();
			else
				nfo.OnReset();
		}

		void Direct2D::Device::Fonts::OnReset() const
		{
			fps.OnReset();
			msg.OnReset();
			nfo.OnReset();
		}

		void Direct2D::Device::Fonts::OnLost() const
		{
			fps.OnLost();
			msg.OnLost();
			nfo.OnLost();
		}

		Direct2D::Device::Timing::Timing()
		:
		autoHz          (false),
		vsync           (false),
		tripleBuffering (false),
		frameRate       (Mode::DEFAULT_RATE)
		{
		}

		Direct2D::VertexBuffer::VertexBuffer()
		: numVertices(4), screenCurvature(0), dirty(false) {}

		Direct2D::VertexBuffer::~VertexBuffer()
		{
			Invalidate();
		}

		void Direct2D::VertexBuffer::Update(const Rect& picture,const Rect& c,const int s)
		{
			NST_ASSERT( picture.Width() > 0 && picture.Height() > 0 && c.Width() > 0 && c.Height() > 0 );

			dirty = true;

			rect = picture;
			clip = c;
			screenCurvature = s;

			const uint n = (s ? (TSL_PATCHES+1) * (TSL_PATCHES+1) : 4);

			if (numVertices != n)
			{
				numVertices = n;
				Invalidate();
			}
		}

		void Direct2D::VertexBuffer::Invalidate()
		{
			if (com)
			{
				IDirect3DDevice9* device;

				if (SUCCEEDED(com->GetDevice( &device )))
				{
					device->SetStreamSource( 0, NULL, 0, 0 );
					device->Release();
				}

				com.Release();
			}
		}

		HRESULT Direct2D::VertexBuffer::Validate(IDirect3DDevice9& device,const Textures& textures)
		{
			if (!com)
			{
				const HRESULT hResult = device.CreateVertexBuffer
				(
					numVertices * sizeof(Vertex),
					D3DUSAGE_WRITEONLY,
					FVF,
					D3DPOOL_MANAGED,
					&com,
					NULL
				);

				if (FAILED(hResult))
				{
					if (hResult == D3DERR_DEVICELOST)
						return D3DERR_DEVICELOST;
					else
						throw Application::Exception( IDS_ERR_FAILED, L"IDirect3DDevice9::CreateVertexBuffer()" );
				}

				dirty = true;
			}

			if (dirty)
			{
				dirty = false;

				void* ptr;
				const HRESULT hResult = com->Lock( 0, 0, &ptr, D3DLOCK_NOSYSLOCK );

				if (SUCCEEDED(hResult))
				{
					Vertex* NST_RESTRICT v = static_cast<Vertex*>(ptr);

					dirty = true;

					if (screenCurvature)
					{
						const float z = 1.f - screenCurvature / 40.f;
						const D3DXVECTOR2 p0( rect.left - 0.499f, rect.top - 0.499f );
						const D3DXVECTOR2 p1( rect.right - 0.499f, rect.bottom - 0.499f );
						const D3DXVECTOR2 t( rect.Width() * z, rect.Height() * z );

						for (uint y=0; y <= TSL_PATCHES; ++y)
						{
							D3DXVECTOR2 vy;

							float weight = y / float(TSL_PATCHES);
							::D3DXVec2Hermite( &vy, &p0, &t, &p1, &t, weight );

							vy.x = textures.GetScreenBottomV(clip.top + clip.Height() * weight);
							float x1 = textures.GetEffectBottomV(clip.top + clip.Height() * weight);

							for (uint x=0; x <= TSL_PATCHES; ++x, ++v)
							{
								D3DXVECTOR2 vx;

								weight = x / float(TSL_PATCHES);
								::D3DXVec2Hermite( &vx, &p0, &t, &p1, &t, weight );

								v->x   = vx.x;
								v->y   = vy.y;
								v->z   = 0.f;
								v->rhw = 1.f;
								v->u0  = textures.GetScreenLeftU(clip.left + clip.Width() * weight);
								v->u1  = textures.GetEffectLeftU(clip.left + clip.Width() * weight);
								v->v0  = vy.x;
								v->v1  = x1;
							}
						}
					}
					else
					{
						v[0].x   = rect.left - 0.499f;
						v[0].y   = rect.top - 0.499f;
						v[0].z   = 0.f;
						v[0].rhw = 1.f;
						v[0].u0  = textures.GetScreenLeftU( clip.left );
						v[0].u1  = textures.GetEffectLeftU( clip.left );
						v[0].v0  = textures.GetScreenTopV( clip.top );
						v[0].v1  = textures.GetEffectTopV( clip.top );
						v[1].x   = rect.left - 0.499f;
						v[1].y   = rect.bottom - 0.499f;
						v[1].z   = 0.f;
						v[1].rhw = 1.f;
						v[1].u0  = textures.GetScreenLeftU( clip.left );
						v[1].u1  = textures.GetEffectLeftU( clip.left );
						v[1].v0  = textures.GetScreenBottomV( clip.bottom );
						v[1].v1  = textures.GetEffectBottomV( clip.bottom );
						v[2].x   = rect.right - 0.499f;
						v[2].y   = rect.top - 0.499f;
						v[2].z   = 0.f;
						v[2].rhw = 1.f;
						v[2].u0  = textures.GetScreenRightU( clip.right );
						v[2].u1  = textures.GetEffectRightU( clip.right );
						v[2].v0  = textures.GetScreenTopV( clip.top );
						v[2].v1  = textures.GetEffectTopV( clip.top );
						v[3].x   = rect.right - 0.499f;
						v[3].y   = rect.bottom - 0.499f;
						v[3].z   = 0.f;
						v[3].rhw = 1.f;
						v[3].u0  = textures.GetScreenRightU( clip.right );
						v[3].u1  = textures.GetEffectRightU( clip.right );
						v[3].v0  = textures.GetScreenBottomV( clip.bottom );
						v[3].v1  = textures.GetEffectBottomV( clip.bottom );
					}

					com->Unlock();
				}
				else if (hResult == D3DERR_DEVICELOST)
				{
					return D3DERR_DEVICELOST;
				}
				else
				{
					throw Application::Exception( IDS_ERR_FAILED, L"IDirect3DVertexBuffer9::Lock()" );
				}

				com->PreLoad();
			}

			if (FAILED(device.SetFVF( FVF )))
				throw Application::Exception( IDS_ERR_FAILED, L"IDirect3DDevice9::SetFVF()" );

			if (FAILED(device.SetStreamSource( 0, *com, 0, sizeof(Vertex) )))
				throw Application::Exception( IDS_ERR_FAILED, L"IDirect3DDevice9::SetStreamSource()" );

			return D3D_OK;
		}

		Direct2D::IndexBuffer::IndexBuffer()
		: numStrips(0) {}

		Direct2D::IndexBuffer::~IndexBuffer()
		{
			Invalidate();
		}

		void Direct2D::IndexBuffer::Update(bool s)
		{
			if (bool(numStrips) != s)
			{
				numStrips = (s ? (TSL_PATCHES * 2 + 1) * TSL_PATCHES - 1 : 0);
				Invalidate();
			}
		}

		void Direct2D::IndexBuffer::Invalidate()
		{
			if (com)
			{
				IDirect3DDevice9* device;

				if (SUCCEEDED(com->GetDevice( &device )))
				{
					device->SetIndices( NULL );
					device->Release();
				}

				com.Release();
			}
		}

		HRESULT Direct2D::IndexBuffer::Validate(IDirect3DDevice9& device)
		{
			NST_ASSERT( !com || numStrips );

			if (numStrips)
			{
				if (!com)
				{
					const HRESULT hResult = device.CreateIndexBuffer
					(
						(((TSL_PATCHES * 2 + 1) * TSL_PATCHES) + 1) * sizeof(WORD),
						D3DUSAGE_WRITEONLY,
						D3DFMT_INDEX16,
						D3DPOOL_MANAGED,
						&com,
						NULL
					);

					if (SUCCEEDED(hResult))
					{
						void* ptr;
						const HRESULT hResult = com->Lock( 0, 0, &ptr, D3DLOCK_NOSYSLOCK );

						if (SUCCEEDED(hResult))
						{
							WORD* NST_RESTRICT data = static_cast<WORD*>(ptr);

							for (uint p=0, i=0, n=TSL_PATCHES+1;;)
							{
								uint j = i + n;

								do
								{
									*data++ = i;
									*data++ = i++ + n;
								}
								while (i < j);

								i += n-1;

								do
								{
									*data++ = i-- + n;
									*data++ = i;
								}
								while (i > j);

								i += n;
								p += 2;

								if (p == n-1)
								{
									*data++ = i;
									break;
								}
							}

							com->Unlock();
						}
						else if (hResult == D3DERR_DEVICELOST)
						{
							return D3DERR_DEVICELOST;
						}
						else
						{
							throw Application::Exception( IDS_ERR_FAILED, L"IDirect3DIndexBuffer9::Lock()" );
						}
					}
					else if (hResult == D3DERR_DEVICELOST)
					{
						return D3DERR_DEVICELOST;
					}
					else
					{
						throw Application::Exception( IDS_ERR_FAILED, L"IDirect3DDevice9::CreateIndexBuffer()" );
					}
				}

				com->PreLoad();

				if (FAILED(device.SetIndices( *com )))
					throw Application::Exception( IDS_ERR_FAILED, L"IDirect3DDevice9::SetIndices()" );
			}

			return D3D_OK;
		}

		Direct2D::Textures::Textures(D3DFORMAT backBufferFormat)
		:
		screenTexture (backBufferFormat),
		effectTexture (backBufferFormat),
		filter        (Adapter::FILTER_NONE)
		{}

		void Direct2D::Textures::Update(const Adapter& adapter,const Point& screen,const uint scanlines,const Adapter::Filter f,const bool useVidMem)
		{
			NST_ASSERT( screen.x > 0 && screen.y > 0 );

			filter = f;

			effectTexture.Update( adapter, screen, scanlines );
			screenTexture.Update( adapter, screen, useVidMem );
		}

		void Direct2D::Textures::Flush()
		{
			screenTexture.Flush();
		}

		void Direct2D::Textures::Invalidate()
		{
			effectTexture.Invalidate();
			screenTexture.Invalidate();
		}

		HRESULT Direct2D::Textures::Validate(IDirect3DDevice9& device,const Adapter& adapter,const D3DFORMAT backBufferFormat)
		{
			if (screenTexture.Validate( device, backBufferFormat, adapter ) && effectTexture.Validate( device, backBufferFormat ))
			{
				const D3DTEXTUREFILTERTYPE type = (filter == Adapter::FILTER_NONE ? D3DTEXF_POINT : D3DTEXF_LINEAR);

				for (uint i=0, n = (effectTexture ? 2 : 1); i < n; ++i)
				{
					device.SetSamplerState( i, D3DSAMP_MINFILTER, type );
					device.SetSamplerState( i, D3DSAMP_MAGFILTER, type );
				}

				return D3D_OK;
			}

			return D3DERR_DEVICELOST;
		}

		bool Direct2D::Textures::SaveToFile(wcstring const file,const D3DXIMAGE_FILEFORMAT type) const
		{
			return screenTexture.SaveToFile( file, type );
		}

		Direct2D::Textures::Texture::Texture(uint s,D3DFORMAT f)
		: size(256,256), stage(s)
		{
			desc.Width = size.x;
			desc.Height = size.y;
			desc.Format = f;
		}

		Direct2D::Textures::Texture::~Texture()
		{
			Invalidate();
		}

		uint Direct2D::Textures::Texture::GetSquared(const Point& p)
		{
			uint squared = NST_MAX(p.x,p.y);

			squared--;
			squared |= squared >> 1;
			squared |= squared >> 2;
			squared |= squared >> 4;
			squared |= squared >> 8;
			squared |= squared >> 16;
			squared++;

			return squared;
		}

		void Direct2D::Textures::Texture::Invalidate()
		{
			if (com)
			{
				IDirect3DDevice9* device;

				if (SUCCEEDED(com->GetDevice( &device )))
				{
					device->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_DISABLE );
					device->SetTexture( stage, NULL );
					device->Release();
				}

				com.Release();
			}
		}

		bool Direct2D::Textures::Texture::Validate(IDirect3DDevice9& device,const D3DFORMAT desiredFormat,const bool dynamicUsage)
		{
			if (!com)
			{
				const HRESULT hResult = ::D3DXCreateTexture
				(
					&device,
					desc.Width,
					desc.Height,
					1,
					dynamicUsage ? D3DUSAGE_DYNAMIC : 0,
					desiredFormat,
					dynamicUsage ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
					&com
				);

				if (SUCCEEDED(hResult))
				{
					if (FAILED(com->GetLevelDesc( 0, &desc )))
						throw Application::Exception( IDS_ERR_FAILED, L"IDirect3DDevice9::GetLevelDesc()" );

					if (desc.Width >= size.x && desc.Height >= size.y)
					{
						if (!GetBitsPerPixel())
							throw Application::Exception( L"Unsupported bits-per-pixel format!" );

						return true;
					}
					else
					{
						throw Application::Exception( L"Maximum texture dimension too small!" );
					}
				}
				else if (hResult == D3DERR_DEVICELOST)
				{
					return false;
				}
				else
				{
					throw Application::Exception( IDS_ERR_FAILED, L"IDirect3DDevice9::CreateTexture()" );
				}
			}

			return false;
		}

		Direct2D::Textures::ScreenTexture::ScreenTexture(D3DFORMAT f)
		: Texture(0,f), useVidMem(false) {}

		void Direct2D::Textures::ScreenTexture::Update(const Adapter& adapter,Point newSize,const bool wantVidMem)
		{
			size = newSize;

			if (!adapter.anyTextureSize)
				newSize = GetSquared(newSize);

			if (desc.Width != newSize.x || desc.Height != newSize.y)
			{
				desc.Width = newSize.x;
				desc.Height = newSize.y;
				Invalidate();
			}

			if (useVidMem != wantVidMem)
			{
				useVidMem = wantVidMem;
				Invalidate();
			}
		}

		void Direct2D::Textures::ScreenTexture::Flush()
		{
			if (desc.Pool == D3DPOOL_DEFAULT)
				Invalidate();
		}

		bool Direct2D::Textures::ScreenTexture::Validate(IDirect3DDevice9& device,const D3DFORMAT desiredFormat,const Adapter& adapter)
		{
			if (!Texture::Validate( device, desiredFormat, useVidMem && adapter.videoMemScreen ) && !com)
				return false;

			device.SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
			device.SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			device.SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

			device.SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
			device.SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

			if (FAILED(device.SetTexture( 0, *com )))
				throw Application::Exception( IDS_ERR_FAILED, L"IDirect3DDevice9::SetTexture()" );

			return true;
		}

		bool Direct2D::Textures::ScreenTexture::SaveToFile(wcstring const file,const D3DXIMAGE_FILEFORMAT type) const
		{
			NST_ASSERT( file && *file && size.x && size.y );

			if (!com)
				return false;

			ComInterface<IDirect3DSurface9> surface;

			{
				ComInterface<IDirect3DDevice9> device;

				if (FAILED(com->GetDevice( &device )))
					return false;

				if (FAILED(device->CreateOffscreenPlainSurface( size.x, size.y, D3DFMT_R8G8B8, D3DPOOL_SCRATCH, &surface, NULL )))
					return false;
			}

			ulong masks[3];
			GetBitMask(masks[0],masks[1],masks[2]);

			uint shifts[3];

			for (uint i=0; i < 3; ++i)
			{
				NST_ASSERT( masks[i] );

				for (shifts[i]=0; !(masks[i] & 0x1); ++shifts[i])
					masks[i] >>= 1;
			}

			D3DLOCKED_RECT dstLock;

			if (FAILED(surface->LockRect( &dstLock, NULL, D3DLOCK_NOSYSLOCK )))
				return false;

			D3DLOCKED_RECT srcLock;
			const RECT rect = {0,0,size.x,size.y};

			if (FAILED(com->LockRect( 0, &srcLock, &rect, D3DLOCK_READONLY|D3DLOCK_NOSYSLOCK )))
			{
				surface->UnlockRect();
				return false;
			}

			BYTE* NST_RESTRICT dst = static_cast<BYTE*>(dstLock.pBits);
			const uint dstPitch = dstLock.Pitch - size.x * 3;

			if (GetBitsPerPixel() == 16)
			{
				const WORD* NST_RESTRICT src = static_cast<WORD*>(srcLock.pBits);
				const uint srcPitch = srcLock.Pitch - size.x * sizeof(WORD);

				for (uint y=size.y; y; --y)
				{
					for (const BYTE* const end=dst+size.x*3; dst != end; dst += 3)
					{
						const uint pixel = *src++;

						dst[2] = ((pixel >> shifts[0] & masks[0]) * 0xFF + masks[0] / 2) / masks[0];
						dst[1] = ((pixel >> shifts[1] & masks[1]) * 0xFF + masks[1] / 2) / masks[1];
						dst[0] = ((pixel >> shifts[2] & masks[2]) * 0xFF + masks[2] / 2) / masks[2];
					}

					src = reinterpret_cast<const WORD*>(reinterpret_cast<const BYTE*>(src) + srcPitch);
					dst += dstPitch;
				}
			}
			else
			{
				const DWORD* NST_RESTRICT src = static_cast<DWORD*>(srcLock.pBits);
				const uint srcPitch = srcLock.Pitch - size.x * sizeof(DWORD);

				for (uint y=size.y; y; --y)
				{
					for (const BYTE* const end=dst+size.x*3; dst != end; dst += 3)
					{
						const uint pixel = *src++;

						dst[2] = ((pixel >> shifts[0] & masks[0]) * 0xFF + masks[0] / 2) / masks[0];
						dst[1] = ((pixel >> shifts[1] & masks[1]) * 0xFF + masks[1] / 2) / masks[1];
						dst[0] = ((pixel >> shifts[2] & masks[2]) * 0xFF + masks[2] / 2) / masks[2];
					}

					src = reinterpret_cast<const DWORD*>(reinterpret_cast<const BYTE*>(src) + srcPitch);
					dst += dstPitch;
				}
			}

			com->UnlockRect( 0 );
			surface->UnlockRect();

			return SUCCEEDED(::D3DXSaveSurfaceToFile( file, type, *surface, NULL, &rect ));
		}

		Direct2D::Textures::EffectTexture::EffectTexture(D3DFORMAT f)
		: Texture(1,f), scanlines(0), dirty(false) {}

		void Direct2D::Textures::EffectTexture::Update(const Adapter& adapter,Point newSize,const uint newScanlines)
		{
			NST_ASSERT( newScanlines <= MAX_SCANLINES );

			newSize.x = 1;
			size = newSize;

			if (!adapter.anyTextureSize)
				newSize = GetSquared(newSize);

			if (desc.Width != newSize.x || desc.Height != newSize.y)
			{
				desc.Width = newSize.x;
				desc.Height = newSize.y;
				Invalidate();
			}

			if (scanlines != newScanlines)
			{
				dirty = true;

				if ((scanlines > 0) != (newScanlines > 0))
					Invalidate();

				scanlines = newScanlines;
			}
		}

		bool Direct2D::Textures::EffectTexture::Validate(IDirect3DDevice9& device,const D3DFORMAT desiredFormat)
		{
			NST_ASSERT( !com || scanlines );

			if (scanlines)
			{
				if (Texture::Validate( device, desiredFormat, false ))
				{
					dirty = true;
				}
				else if (!com)
				{
					return false;
				}

				if (dirty)
				{
					dirty = false;

					ulong mask[3];
					GetBitMask(mask[0],mask[1],mask[2]);

					const uint intensity[2] =
					{
						mask[0] | mask[1] | mask[2],
						((mask[0] * (100 - scanlines) / 100) & mask[0]) |
						((mask[1] * (100 - scanlines) / 100) & mask[1]) |
						((mask[2] * (100 - scanlines) / 100) & mask[2])
					};

					D3DLOCKED_RECT lockedRect;
					const RECT rect = {0,0,desc.Width,desc.Height};
					const HRESULT hResult = com->LockRect( 0, &lockedRect, &rect, D3DLOCK_NOSYSLOCK );

					if (SUCCEEDED(hResult))
					{
						if (GetBitsPerPixel() == 32)
						{
							DWORD* NST_RESTRICT dst = static_cast<DWORD*>(lockedRect.pBits);
							const uint pitch = lockedRect.Pitch - desc.Width * sizeof(DWORD);

							for (uint y=desc.Height/2; y; --y)
							{
								for (uint i=0; i < 2; ++i)
								{
									for (uint x=desc.Width; x; --x)
										*dst++ = intensity[i];

									dst = reinterpret_cast<DWORD*>(reinterpret_cast<BYTE*>(dst) + pitch);
								}
							}
						}
						else
						{
							WORD* NST_RESTRICT dst = static_cast<WORD*>(lockedRect.pBits);
							const uint pitch = lockedRect.Pitch - desc.Width * sizeof(WORD);

							for (uint y=desc.Height/2; y; --y)
							{
								for (uint i=0; i < 2; ++i)
								{
									for (uint x=desc.Width; x; --x)
										*dst++ = intensity[i];

									dst = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(dst) + pitch);
								}
							}
						}

						com->UnlockRect(0);
					}
					else if (hResult == D3DERR_DEVICELOST)
					{
						return false;
					}
					else
					{
						throw Application::Exception( IDS_ERR_FAILED, L"IDirect3DTexture9::LockRect()" );
					}

					com->PreLoad();
				}

				device.SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE );
				device.SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
				device.SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );

				device.SetSamplerState( 1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
				device.SetSamplerState( 1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );

				if (FAILED(device.SetTexture( 1, *com )))
					throw Application::Exception( IDS_ERR_FAILED, L"IDirect3DDevice9::SetTexture()" );
			}

			return true;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
