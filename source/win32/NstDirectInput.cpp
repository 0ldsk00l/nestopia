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

#include <new>
#include "language/resource.h"
#include "NstApplicationException.hpp"
#include "NstApplicationInstance.hpp"
#include "NstSystemKeyboard.hpp"
#include "NstDirectInput.hpp"
#include "NstIoScreen.hpp"
#include "NstIoLog.hpp"

#if NST_MSVC
#pragma comment(lib,"dinput8")
#pragma comment(lib,"dxguid")
#endif

namespace Nestopia
{
	namespace DirectX
	{
		HeapString DirectInput::Keyboard::keyNames[MAX_KEYS];

		struct DirectInput::Joystick::Lookup
		{
			uint (*code)(const void* const);
			ushort offset;
			ushort axis;
			wcstring name;
		};

		const DirectInput::Joystick::Lookup DirectInput::Joystick::table[TABLE_KEYS] =
		{
			{ KeyPosDir,   DIJOFS_Y,         AXIS_Y,        L"+y"   },
			{ KeyPosDir,   DIJOFS_X,         AXIS_X,        L"+x"   },
			{ KeyNegDir,   DIJOFS_Y,         AXIS_Y,        L"-y"   },
			{ KeyNegDir,   DIJOFS_X,         AXIS_X,        L"-x"   },
			{ KeyPosDir,   DIJOFS_Z,         AXIS_Z,        L"+z"   },
			{ KeyNegDir,   DIJOFS_Z,         AXIS_Z,        L"-z"   },
			{ KeyPosDir,   DIJOFS_RY,        AXIS_RY,       L"+ry"  },
			{ KeyPosDir,   DIJOFS_RX,        AXIS_RX,       L"+rx"  },
			{ KeyPosDir,   DIJOFS_RY,        AXIS_RY,       L"-ry"  },
			{ KeyNegDir,   DIJOFS_RX,        AXIS_RX,       L"-rx"  },
			{ KeyPosDir,   DIJOFS_RZ,        AXIS_RZ,       L"+rz"  },
			{ KeyNegDir,   DIJOFS_RZ,        AXIS_RZ,       L"-rz"  },
			{ KeyNegDir,   DIJOFS_SLIDER(0), AXIS_SLIDER_0, L"-s0"  },
			{ KeyPosDir,   DIJOFS_SLIDER(0), AXIS_SLIDER_0, L"+s0"  },
			{ KeyNegDir,   DIJOFS_SLIDER(1), AXIS_SLIDER_1, L"-s1"  },
			{ KeyPosDir,   DIJOFS_SLIDER(1), AXIS_SLIDER_1, L"+s1"  },
			{ KeyPovUp,    DIJOFS_POV(0),    AXIS_POV_0,    L"-p0y" },
			{ KeyPovRight, DIJOFS_POV(0),    AXIS_POV_0,    L"+p0x" },
			{ KeyPovDown,  DIJOFS_POV(0),    AXIS_POV_0,    L"+p0y" },
			{ KeyPovLeft,  DIJOFS_POV(0),    AXIS_POV_0,    L"-p0x" },
			{ KeyPovUp,    DIJOFS_POV(1),    AXIS_POV_1,    L"-p1y" },
			{ KeyPovRight, DIJOFS_POV(1),    AXIS_POV_1,    L"+p1x" },
			{ KeyPovDown,  DIJOFS_POV(1),    AXIS_POV_1,    L"+p1y" },
			{ KeyPovLeft,  DIJOFS_POV(1),    AXIS_POV_1,    L"-p1x" },
			{ KeyPovUp,    DIJOFS_POV(2),    AXIS_POV_2,    L"-p2y" },
			{ KeyPovRight, DIJOFS_POV(2),    AXIS_POV_2,    L"+p2x" },
			{ KeyPovDown,  DIJOFS_POV(2),    AXIS_POV_2,    L"+p2y" },
			{ KeyPovLeft,  DIJOFS_POV(2),    AXIS_POV_2,    L"-p2x" },
			{ KeyPovUp,    DIJOFS_POV(3),    AXIS_POV_3,    L"-p3y" },
			{ KeyPovRight, DIJOFS_POV(3),    AXIS_POV_3,    L"+p3x" },
			{ KeyPovDown,  DIJOFS_POV(3),    AXIS_POV_3,    L"+p3y" },
			{ KeyPovLeft,  DIJOFS_POV(3),    AXIS_POV_3,    L"-p3x" }
		};

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		inline void DirectInput::Joystick::Calibrator::Fix(DIJOYSTATE& state) const
		{
			state.lX -= lX;
			state.lY -= lY;
			state.lZ -= lZ;
			state.lRx -= lRx;
			state.lRy -= lRy;
			state.lRz -= lRz;
		}

		NST_FORCE_INLINE void DirectInput::Keyboard::Poll()
		{
			if (enabled)
			{
				HRESULT hResult;

				if (FAILED(hResult=com.Poll()) || FAILED(hResult=com.GetDeviceState( Buffer::SIZE, buffer )))
					OnError( hResult );
			}
		}

		NST_FORCE_INLINE void DirectInput::Joystick::Poll()
		{
			if (enabled)
			{
				HRESULT hResult;

				if (SUCCEEDED(hResult=com.Poll()) && SUCCEEDED(hResult=com.GetDeviceState( sizeof(state), &state )))
					calibrator.Fix( state );
				else
					OnError( hResult );
			}
		}

		void DirectInput::Poll()
		{
			keyboard.Poll();

			for (Joysticks::Iterator it(joysticks.Begin()), end(joysticks.End()); it != end; ++it)
				it->Poll();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		DirectInput::Base::Base(HWND const h)
		: com(Create()), hWnd(h) {}

		DirectInput::Base::~Base()
		{
			com.Release();
		}

		IDirectInput8& DirectInput::Base::Create()
		{
			Io::Log() << "DirectInput: initializing..\r\n";

			IDirectInput8* com;

			if (FAILED(::DirectInput8Create( ::GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&com), NULL )))
				throw Application::Exception( IDS_ERR_FAILED, L"DirectInput8Create()" );

			return *com;
		}

		DirectInput::DirectInput(HWND const hWnd)
		: base(hWnd), keyboard(base)
		{
			if (SUCCEEDED(base.com.EnumDevices( DI8DEVCLASS_GAMECTRL, EnumJoysticks, this, DIEDFL_ATTACHEDONLY )))
				Io::Log() << "DirectInput: found " << joysticks.Size() << " attached joystick(s)\r\n";
			else
				Io::Log() << "DirectInput: IDirectInput8::EnumDevices() failed! No joysticks can be used!\r\n";
		}

		DirectInput::~DirectInput()
		{
			for (uint i=joysticks.Size(); i; )
				joysticks[--i].Joystick::~Joystick();
		}

		BOOL CALLBACK DirectInput::EnumJoysticks(LPCDIDEVICEINSTANCE const instance,LPVOID const context)
		{
			if (instance)
			{
				DirectInput& directInput = *static_cast<DirectInput*>(context);

				if (directInput.joysticks.Size() == MAX_JOYSTICKS)
				{
					Io::Log() << "DirectInput: warning, device count limit reached, stopping enumeration..\r\n";
					return DIENUM_STOP;
				}

				Io::Log() << "DirectInput: enumerating device - name: "
                          << (*instance->tszProductName ? instance->tszProductName : L"unknown")
                          << ", GUID: "
                          << System::Guid( instance->guidInstance ).GetString()
                          << "\r\n";

				directInput.joysticks.Grow();

				try
				{
					new (&directInput.joysticks.Back()) Joystick( directInput.base, *instance );
				}
				catch (Joystick::Exception)
				{
					directInput.joysticks.Shrink();
					Io::Log() << "DirectInput: warning, bogus device, continuing enumeration..\r\n";
				}
			}

			return DIENUM_CONTINUE;
		}

		void DirectInput::Acquire()
		{
			keyboard.Acquire();

			for (Joysticks::Iterator it(joysticks.Begin()), end(joysticks.End()); it != end; ++it)
				it->Acquire();
		}

		void DirectInput::Unacquire()
		{
			keyboard.Unacquire();

			for (Joysticks::Iterator it(joysticks.Begin()), end(joysticks.End()); it != end; ++it)
				it->Unacquire();
		}

		void DirectInput::Calibrate()
		{
			for (Joysticks::Iterator it(joysticks.Begin()), end(joysticks.End()); it != end; ++it)
				it->Calibrate();
		}

		void DirectInput::BeginScanMode(HWND hWnd) const
		{
			keyboard.BeginScanMode( hWnd );

			for (Joysticks::ConstIterator it(joysticks.Begin()), end(joysticks.End()); it != end; ++it)
				it->BeginScanMode();
		}

		void DirectInput::EndScanMode() const
		{
			keyboard.EndScanMode();

			for (Joysticks::ConstIterator it(joysticks.Begin()), end(joysticks.End()); it != end; ++it)
				it->EndScanMode();
		}

		void DirectInput::Build(const Key* const keys,const uint count)
		{
			keyboard.Enable( false );

			for (const Key *it=keys, *const end=keys+count; it != end; ++it)
			{
				if (keyboard.Assigned( *it ))
				{
					keyboard.Enable( true );
					break;
				}
			}

			for (Joysticks::Iterator it(joysticks.Begin()), end(joysticks.End()); it != end; ++it)
			{
				it->Enable( false );

				for (const Key *jt=keys, *const jend=keys+count; jt != jend; ++jt)
				{
					if (it->Assigned( *jt ))
					{
						it->Enable( true );
						break;
					}
				}
			}
		}

		DirectInput::ScanResult DirectInput::ScanKey(Key& key,const ScanMode scanMode)
		{
			const ScanResult scan = (scanMode == SCAN_MODE_ALL ? keyboard.Scan( key ) : SCAN_NO_KEY);

			if (scan != SCAN_GOOD_KEY)
			{
				if (scan == SCAN_NO_KEY)
				{
					for (Joysticks::Iterator it(joysticks.Begin()), end(joysticks.End()); it != end; ++it)
					{
						if (it->Scan( key ))
							return SCAN_GOOD_KEY;
					}
				}

				key.Unmap();
			}

			return scan;
		}

		bool DirectInput::MapKey(Key& key,wcstring const name,const System::Guid* const guids,const uint numGuids) const
		{
			key.Unmap();

			if
			(
				name && *name &&
				(name[0] != '.' || name[1] != '.' || name[2] != '.' || name[3] != '\0')
			)
			{
				if
				(
					(name[0] == '(') &&
					(name[1] == 'j' || name[1] == 'J') &&
					(name[2] == 'o' || name[2] == 'O') &&
					(name[3] == 'y' || name[3] == 'Y') &&
					(name[4] == ' ') &&
					(name[5] >= '0' && name[5] <= '9') &&
					(
						(name[6] == ')' && name[7] == ' ') ||
						(name[6] >= '0' && name[6] <= '9' && name[7] == ')' && name[8] == ' ')
					)
				)
				{
					uint index = name[5] - '0';

					if (name[6] != ')')
						index = (index * 10) + (name[6] - '0');

					if (index < NST_MIN(MAX_JOYSTICKS,numGuids))
					{
						const System::Guid& guid = guids[index];

						for (Joysticks::ConstIterator it(joysticks.Begin()), end(joysticks.End()); it != end; ++it)
						{
							if (it->GetGuid() == guid)
								return it->Map( key, name + (name[7] == ' ' ? 8 : 9) );
						}
					}
				}
				else
				{
					return keyboard.Map( key, name );
				}
			}

			return false;
		}

		const HeapString DirectInput::GetKeyName(const Key& key) const
		{
			if (key.Assigned())
			{
				if (keyboard.Assigned( key ))
				{
					return keyboard.GetName( key );
				}
				else
				{
					for (uint i=0, n=joysticks.Size(); i < n; ++i)
					{
						if (joysticks[i].Assigned( key ))
							return HeapString("(joy ") << i << ") " << joysticks[i].GetName( key );
					}

					HeapString name;

					if (key.vKey & FCONTROL)
						name << System::Keyboard::GetName( VK_CONTROL ) << '+';

					if (key.vKey & FALT)
						name << System::Keyboard::GetName( VK_MENU ) << '+';

					if (key.vKey & FSHIFT)
						name << System::Keyboard::GetName( VK_SHIFT ) << '+';

					name << System::Keyboard::GetName( key.vKey >> 8 );

					return name;
				}
			}

			return L"...";
		}

		DirectInput::Device::Device(IDirectInputDevice8& c)
		: com(c), enabled(false) {}

		DirectInput::Device::~Device()
		{
			com.Unacquire();
			com.Release();
		}

		void DirectInput::Device::Enable(bool enable)
		{
			enabled = enable;
		}

		bool DirectInput::Device::Acquire(void* const data,const uint size)
		{
			return enabled && SUCCEEDED(com.Acquire()) && SUCCEEDED(com.Poll()) && SUCCEEDED(com.GetDeviceState( size, data ));
		}

		void DirectInput::Device::Unacquire()
		{
			com.Unacquire();
		}

		DirectInput::Keyboard::Keyboard(Base& base)
		: Device(Create(base.com)), hWnd(base.hWnd)
		{
			for (uint i=0; i < MAX_KEYS; ++i)
				keyNames[i] << i;

			com.EnumObjects( EnumObjects, NULL, DIDFT_BUTTON );
			SetCooperativeLevel( base.hWnd, COOPERATIVE_FLAGS );
			Clear();
		}

		IDirectInputDevice8& DirectInput::Keyboard::Create(IDirectInput8& base)
		{
			IDirectInputDevice8* com;

			if (FAILED(base.CreateDevice( GUID_SysKeyboard, &com, NULL )))
				throw Application::Exception( IDS_ERR_FAILED, L"IDirectInput8::CreateDevice()" );

			if (FAILED(com->SetDataFormat( &c_dfDIKeyboard )))
			{
				com->Release();
				throw Application::Exception( IDS_ERR_FAILED, L"IDirectInputDevice8::SetDataFormat()" );
			}

			return *com;
		}

		BOOL CALLBACK DirectInput::Keyboard::EnumObjects(LPCDIDEVICEOBJECTINSTANCE const obj,LPVOID)
		{
			NST_VERIFY( obj->dwOfs < MAX_KEYS && *obj->tszName );

			if (obj->dwOfs < MAX_KEYS && *obj->tszName)
			{
				HeapString& string = keyNames[obj->dwOfs];
				string = obj->tszName;

				::CharUpperBuff( string.Ptr(), 1 );

				if (string.Length() > 1)
					::CharLowerBuff( string.Ptr() + 1, string.Length() - 1 );
			}

			return DIENUM_CONTINUE;
		}

		void DirectInput::Keyboard::SetCooperativeLevel(HWND const hWnd,const DWORD flags) const
		{
			if (FAILED(com.SetCooperativeLevel( hWnd, flags )))
				throw Application::Exception( IDS_ERR_FAILED, L"IDirectInputDevice8::SetCooperativeLevel()" );
		}

		bool DirectInput::Keyboard::Map(Key& key,wcstring name) const
		{
			for (uint i=0; i < MAX_KEYS; ++i)
			{
				if (keyNames[i] == name)
					return Map( key, i );
			}

			return false;
		}

		bool DirectInput::Keyboard::Map(Key& key,const uint code) const
		{
			if (code && code <= 0xFF && code != DIK_LWIN)
			{
				key.data = buffer + code;
				key.code = KeyDown;
				return true;
			}

			return false;
		}

		void DirectInput::Keyboard::BeginScanMode(HWND hWndScan) const
		{
			com.Unacquire();
			SetCooperativeLevel( hWndScan, SCAN_COOPERATIVE_FLAGS );
			com.Acquire();
		}

		void DirectInput::Keyboard::EndScanMode() const
		{
			com.Unacquire();
			SetCooperativeLevel( hWnd, COOPERATIVE_FLAGS );
		}

		bool DirectInput::Keyboard::Scan(uchar (&data)[MAX_KEYS]) const
		{
			if (SUCCEEDED(com.Poll()) && SUCCEEDED(com.GetDeviceState( MAX_KEYS, data )))
				return true;

			std::memset( data, 0, MAX_KEYS );

			return false;
		}

		DirectInput::ScanResult DirectInput::Keyboard::Scan(Key& key) const
		{
			uchar data[MAX_KEYS];

			if (Scan( data ))
			{
				for (uint i=0; i < MAX_KEYS; ++i)
				{
					if (data[i] & 0x80U)
					{
						switch (i)
						{
							case DIK_CAPITAL:
							case DIK_NUMLOCK:
							case DIK_SCROLL:
							case DIK_KANA:
							case DIK_KANJI:
								continue;

							default:
								return Map( key, i ) ? SCAN_GOOD_KEY : SCAN_INVALID_KEY;
						}
					}
				}
			}

			return SCAN_NO_KEY;
		}

		void DirectInput::Keyboard::OnError(const HRESULT hResult)
		{
			NST_ASSERT( FAILED(hResult) );

			switch (hResult)
			{
				case DIERR_INPUTLOST:
				case DIERR_NOTACQUIRED:

					if (::GetActiveWindow() == hWnd)
					{
						Acquire();
						break;
					}

				default:

					Clear();
					break;
			}
		}

		bool DirectInput::Keyboard::Assigned(const Key& key) const
		{
			return key.data >= buffer && key.data < (buffer + Buffer::SIZE);
		}

		wcstring DirectInput::Keyboard::GetName(const Key& key) const
		{
			NST_VERIFY( Assigned(key) );
			return keyNames[key.data - buffer].Ptr();
		}

		void DirectInput::Keyboard::Clear()
		{
			std::memset( buffer, 0, Buffer::SIZE );
		}

		void DirectInput::Keyboard::Acquire()
		{
			if (!Device::Acquire( buffer, Buffer::SIZE ))
				Clear();
		}

		void DirectInput::Keyboard::Unacquire()
		{
			Device::Unacquire();
			Clear();
		}

		DirectInput::Joystick::Joystick(Base& base,const DIDEVICEINSTANCE& instance)
		:
		Device      (Create(base,instance.guidInstance)),
		caps        (com,instance),
		calibrated  (false),
		scanEnabled (true),
		deadZone    (UINT_MAX),
		axes        (DEFAULT_AXES)
		{
			SetAxisDeadZone( DEFAULT_DEADZONE );

			if (SUCCEEDED(com.Acquire()))
			{
				if (SUCCEEDED(com.Poll()))
					com.GetDeviceState( sizeof(state), &state );

				com.Unacquire();
			}
		}

		DirectInput::Joystick::Calibrator::Calibrator()
		:
		lX  (0),
		lY  (0),
		lZ  (0),
		lRx (0),
		lRy (0),
		lRz (0)
		{}

		DirectInput::Joystick::Caps::Context::Context(Caps& c,IDirectInputDevice8& d)
		: caps(c), com(d), numButtons(0) {}

		DirectInput::Joystick::Caps::Caps(IDirectInputDevice8& com,const DIDEVICEINSTANCE& instance)
		: axes(0), guid(instance.guidInstance), name(*instance.tszProductName ? instance.tszProductName : L"unknown")
		{
			Context context( *this, com );

			if (FAILED(com.EnumObjects( EnumObjectsProc, &context, DIDFT_ALL )) || !(context.numButtons|axes))
				throw ERR_API;
		}

		IDirectInputDevice8& DirectInput::Joystick::Create(Base& base,const GUID& guid)
		{
			IDirectInputDevice8* com;

			if (FAILED(base.com.CreateDevice( guid, &com, NULL )))
				throw ERR_API;

			if
			(
				FAILED(com->SetDataFormat( &c_dfDIJoystick )) ||
				FAILED(com->SetCooperativeLevel( base.hWnd, DISCL_BACKGROUND|DISCL_NONEXCLUSIVE ))
			)
			{
				com->Release();
				throw ERR_API;
			}

			return *com;
		}

		BOOL CALLBACK DirectInput::Joystick::Caps::EnumObjectsProc(LPCDIDEVICEOBJECTINSTANCE const info,LPVOID const ref)
		{
			Context& context = *static_cast<Context*>(ref);

			if (info->guidType == GUID_Button)
			{
				++context.numButtons;
			}
			else
			{
				uint flag;

                     if ( info->guidType == GUID_XAxis  ) flag = AXIS_X;
				else if ( info->guidType == GUID_YAxis  ) flag = AXIS_Y;
				else if ( info->guidType == GUID_ZAxis  ) flag = AXIS_Z;
				else if ( info->guidType == GUID_RxAxis ) flag = AXIS_RX;
				else if ( info->guidType == GUID_RyAxis ) flag = AXIS_RY;
				else if ( info->guidType == GUID_RzAxis ) flag = AXIS_RZ;
				else if ( info->guidType == GUID_POV    )
				{
					if (context.caps.axes & AXIS_POV_3)
					{
						return DIENUM_CONTINUE;
					}
					else if (context.caps.axes & AXIS_POV_2)
					{
						flag = AXIS_POV_3;
					}
					else if (context.caps.axes & AXIS_POV_1)
					{
						flag = AXIS_POV_2;
					}
					else if (context.caps.axes & AXIS_POV_0)
					{
						flag = AXIS_POV_1;
					}
					else
					{
						flag = AXIS_POV_0;
					}
				}
				else if ( info->guidType == GUID_Slider )
				{
					if (context.caps.axes & AXIS_SLIDER_1)
					{
						return DIENUM_CONTINUE;
					}
					else if (context.caps.axes & AXIS_SLIDER_0)
					{
						flag = AXIS_SLIDER_1;
					}
					else
					{
						flag = AXIS_SLIDER_0;
					}
				}
				else
				{
					return DIENUM_CONTINUE;
				}

				if (info->dwType & DIDFT_AXIS)
				{
					Object::Pod<DIPROPRANGE> diprg;

					diprg.diph.dwSize       = sizeof(diprg);
					diprg.diph.dwHeaderSize = sizeof(diprg.diph);
					diprg.diph.dwHow        = DIPH_BYID;
					diprg.diph.dwObj        = info->dwType;
					diprg.lMin              = AXIS_MIN_RANGE;
					diprg.lMax              = AXIS_MAX_RANGE;

					if (FAILED(context.com.SetProperty( DIPROP_RANGE, &diprg.diph )))
					{
						if (FAILED(context.com.GetProperty( DIPROP_RANGE, &diprg.diph )) || diprg.lMin >= 0 || diprg.lMax <= 0)
						{
							Io::Log() << "DirectInput: warning, SetProperty(DIPROP_RANGE) failed, skipping axis..\r\n";
							return DIENUM_CONTINUE;
						}
					}
				}

				context.caps.axes |= flag;
			}

			return DIENUM_CONTINUE;
		}

		bool DirectInput::Joystick::SetAxisDeadZone(const uint value)
		{
			NST_ASSERT( value <= DEADZONE_MAX );

			if (deadZone != value)
			{
				deadZone = value;

				Object::Pod<DIPROPDWORD> diprd;

				diprd.diph.dwSize       = sizeof(diprd);
				diprd.diph.dwHeaderSize = sizeof(diprd.diph);
				diprd.diph.dwHow        = DIPH_DEVICE;
				diprd.dwData            = value * 100;

				com.SetProperty( DIPROP_DEADZONE, &diprd.diph );
				return true;
			}

			return false;
		}

		void DirectInput::Joystick::Clear()
		{
			state.Clear();
			state.rgdwPOV[3] = state.rgdwPOV[2] = state.rgdwPOV[1] = state.rgdwPOV[0] = DWORD(~0UL);
		}

		void DirectInput::Joystick::Acquire()
		{
			if (Device::Acquire( &state, sizeof(state) ))
			{
				if (!calibrated)
				{
					calibrated = true;
					calibrator.Reset( state, false );
				}

				calibrator.Fix( state );
			}
			else
			{
				Clear();
			}
		}

		void DirectInput::Joystick::Unacquire()
		{
			Device::Unacquire();
			Clear();
		}

		inline void DirectInput::Joystick::Calibrator::Reset(DIJOYSTATE& state,bool full)
		{
			lX = full ? state.lX : 0;
			lY = full ? state.lY : 0;
			lZ = state.lZ;
			lRx = state.lRx;
			lRy = state.lRy;
			lRz = state.lRz;
		}

		void DirectInput::Joystick::Calibrate()
		{
			if (SUCCEEDED(com.Acquire()))
			{
				Object::Pod<DIJOYSTATE> tmp;

				if (SUCCEEDED(com.Poll()) && SUCCEEDED(com.GetDeviceState( sizeof(tmp), &tmp )))
				{
					calibrated = true;
					calibrator.Reset( tmp, true );
				}

				com.Unacquire();
			}
		}

		void DirectInput::Joystick::BeginScanMode() const
		{
			com.Acquire();
		}

		void DirectInput::Joystick::EndScanMode() const
		{
			com.Unacquire();
		}

		bool DirectInput::Joystick::Scan(Key& key)
		{
			DIJOYSTATE tmp;

			if (scanEnabled && SUCCEEDED(com.Poll()) && SUCCEEDED(com.GetDeviceState( sizeof(tmp), &tmp )))
			{
				if (!calibrated)
				{
					calibrated = true;
					calibrator.Reset( tmp, false );
				}

				calibrator.Fix( tmp );

				for (uint i=0; i < Caps::NUM_BUTTONS; ++i)
				{
					if (tmp.rgbButtons[i] & 0x80U)
					{
						key.data = state.rgbButtons + i;
						key.code = KeyDown;
						return true;
					}
				}

				for (uint i=0; i < TABLE_KEYS; ++i)
				{
					if (caps.axes & axes & table[i].axis)
					{
						key.data = reinterpret_cast<const BYTE*>(&tmp) + table[i].offset;
						key.code = table[i].code;

						if (key.GetState())
						{
							key.data = reinterpret_cast<const BYTE*>(&state) + table[i].offset;
							return true;
						}
					}
				}
			}

			return false;
		}

		bool DirectInput::Joystick::Map(Key& key,wcstring const name) const
		{
			if (*name)
			{
				if (name[0] >= '0' && name[0] <= '9')
				{
					uint index = name[0] - '0';

					if (name[1] >= '0' && name[1] <= '9')
						index = (index * 10) + (name[1] - '0');

					if (index < Caps::NUM_BUTTONS)
					{
						key.data = state.rgbButtons + index;
						key.code = KeyDown;
						return true;
					}
				}
				else
				{
					const GenericString axis( name );

					for (uint i=TABLE_KEYS; i; )
					{
						if (caps.axes & table[--i].axis)
						{
							if (axis == table[i].name)
							{
								key.data = reinterpret_cast<const BYTE*>(&state) + table[i].offset;
								key.code = table[i].code;
								return true;
							}
						}
					}
				}
			}

			return false;
		}

		bool DirectInput::Joystick::Assigned(const Key& key) const
		{
			return
			(
				key.data >= reinterpret_cast<const BYTE*>(&state) &&
				key.data <  reinterpret_cast<const BYTE*>(&state) + sizeof(state)
			);
		}

		wcstring DirectInput::Joystick::GetName(const Key& key) const
		{
			NST_VERIFY( Assigned(key) );

			if (key.code == KeyDown)
			{
				static wchar_t button[] = L"xx";

				const uint index = key.data - state.rgbButtons;
				button[0] = index < 10 ? '0' + index : '0' + index / 10;
				button[1] = index < 10 ? '\0'        : '0' + index % 10;

				return button;
			}

			for (const Lookup* it = table; ; ++it)
			{
				if (key.data == reinterpret_cast<const BYTE*>(&state) + it->offset && key.code == it->code)
					return it->name;
			}
		}

		void DirectInput::Joystick::OnError(const HRESULT hResult)
		{
			NST_ASSERT( FAILED(hResult) );

			switch (hResult)
			{
				case DIERR_INPUTLOST:
				case DIERR_NOTACQUIRED:

					Acquire();
					break;

				case DIERR_UNPLUGGED:

					Enable( false );
					Io::Screen() << L"Error! Joystick unplugged!";

				default:

					Clear();
					break;
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		uint DirectInput::KeyDown(const void* const data)
		{
			return 0U - (*static_cast<const BYTE*>(data) >> 7);
		}

		uint DirectInput::KeyPosDir(const void* const data)
		{
			if (LONG_MIN >> (sizeof(long) * CHAR_BIT - 1) == ~0UL)
				return (-*static_cast<const long*>(data) >> (sizeof(long) * CHAR_BIT - 1));
			else
				return (*static_cast<const long*>(data) > 0) ? ~0U : 0U;
		}

		uint DirectInput::KeyNegDir(const void* const data)
		{
			if (LONG_MIN >> (sizeof(long) * CHAR_BIT - 1) == ~0UL)
				return (*static_cast<const long*>(data) >> (sizeof(long) * CHAR_BIT - 1));
			else
				return (*static_cast<const long*>(data) < 0) ? ~0U : 0U;
		}

		uint DirectInput::KeyPovUp(const void* const data)
		{
			const DWORD pov = *static_cast<const DWORD*>(data);

			return
			(
				(pov & Joystick::POV_CENTER) != Joystick::POV_CENTER &&
				(pov >= Joystick::POV_UPLEFT || pov <= Joystick::POV_UPRIGHT)
			)   ? ~0U : 0U;
		}

		uint DirectInput::KeyPovRight(const void* const data)
		{
			const DWORD pov = *static_cast<const DWORD*>(data);

			return
			(
				(pov & Joystick::POV_CENTER) != Joystick::POV_CENTER &&
				(pov >= Joystick::POV_UPRIGHT && pov <= Joystick::POV_DOWNRIGHT)
			)   ? ~0U : 0U;
		}

		uint DirectInput::KeyPovDown(const void* const data)
		{
			const DWORD pov = *static_cast<const DWORD*>(data);

			return
			(
				(pov & Joystick::POV_CENTER) != Joystick::POV_CENTER &&
				(pov >= Joystick::POV_DOWNRIGHT && pov <= Joystick::POV_DOWNLEFT)
			)   ? ~0U : 0U;
		}

		uint DirectInput::KeyPovLeft(const void* const data)
		{
			const DWORD pov = *static_cast<const DWORD*>(data);

			return
			(
				(pov & Joystick::POV_CENTER) != Joystick::POV_CENTER &&
				(pov >= Joystick::POV_DOWNLEFT && pov <= Joystick::POV_UPLEFT)
			)   ? ~0U : 0U;
		}

		uint DirectInput::KeyNone(const void* const)
		{
			return 0;
		}

		bool DirectInput::Key::GetToggle(bool& prev) const
		{
			if (GetState())
			{
				if (!prev)
				{
					prev = true;
					return true;
				}
			}
			else
			{
				prev = false;
			}

			return false;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		bool DirectInput::Key::MapVirtualKey(const uint code,const uint vk1,const uint vk2,const uint vk3)
		{
			Unmap();

			if (!code || code > 0xFF)
				return false;

			vKey = code << 8;

			if (vk1 == VK_MENU || vk2 == VK_MENU || vk3 == VK_MENU)
				vKey |= FALT;

			if (vk1 == VK_SHIFT || vk2 == VK_SHIFT || vk3 == VK_SHIFT)
				vKey |= FSHIFT;

			if (vk1 == VK_CONTROL || vk2 == VK_CONTROL || vk3 == VK_CONTROL)
				vKey |= FCONTROL;

			// forbidden keys:
			//
			// ALT+F4
			// ALT+F6
			// ALT+TAB
			// ALT+SPACE
			// ALT+ESC
			// CTRL+F4
			// CTRL+ESC
			// CTRL+ALT+DELETE
			// LWIN

			switch (code)
			{
				case VK_F4:

					if (vKey & FCONTROL)
						return false;

				case VK_F6:
				case VK_TAB:
				case VK_SPACE:

					if (vKey & FALT)
						return false;

					break;

				case VK_ESCAPE:

					if (vKey & (FCONTROL|FALT))
						return false;

					break;

				case VK_DELETE:

					if ((vKey & (FCONTROL|FALT)) == (FCONTROL|FALT))
						return false;

					break;

				case VK_LWIN:
					return false;
			}

			return true;
		}

		bool DirectInput::Key::MapVirtualKey(GenericString name)
		{
			Unmap();

			if (name.Empty())
				return false;

			const GenericString vkNames[3] =
			{
				System::Keyboard::GetName( VK_CONTROL ),
				System::Keyboard::GetName( VK_MENU ),
				System::Keyboard::GetName( VK_SHIFT )
			};

			uint vk[3] = {0,0,0};

			for (uint i=0; i < 3; ++i)
			{
				for (uint j=0; j < 3; ++j)
				{
					if (!vk[j] && name.Length() > vkNames[j].Length() && name(0,vkNames[j].Length()) == vkNames[j] && name[vkNames[j].Length()] == '+')
					{
						vk[j] = (j==0 ? VK_CONTROL : j==1 ? VK_MENU : VK_SHIFT);
						name = name( vkNames[j].Length() + 1 );
						break;
					}
				}
			}

			return MapVirtualKey( System::Keyboard::GetKey( name ), vk[0], vk[1], vk[2] );
		}

		bool DirectInput::Key::IsVirtualKey() const
		{
			return (code == KeyNone && data);
		}

		bool DirectInput::Key::GetVirtualKey(ACCEL& accel) const
		{
			if (code == KeyNone && data)
			{
				accel.fVirt = (vKey & 0xFF) | FVIRTKEY;
				accel.key = vKey >> 8;
				return true;
			}

			accel.fVirt = 0;
			accel.key = 0;
			return false;
		}
	}
}
