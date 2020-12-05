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

#ifndef NST_DIRECTX_DIRECTINPUT_H
#define NST_DIRECTX_DIRECTINPUT_H

#pragma once

#define DIRECTINPUT_VERSION 0x0800

#include "NstDirectX.hpp"
#include "NstObjectPod.hpp"
#include "NstObjectHeap.hpp"
#include "NstCollectionVector.hpp"
#include <dinput.h>

namespace Nestopia
{
	namespace DirectX
	{
		class DirectInput
		{
			class Keyboard;
			class Joystick;

		public:

			explicit DirectInput(HWND);
			~DirectInput();

			enum
			{
				MAX_JOYSTICKS     = 32,
				NUM_KEYBOARD_KEYS = 256,
				AXIS_X            = 0x001,
				AXIS_Y            = 0x002,
				AXIS_Z            = 0x004,
				AXIS_RX           = 0x008,
				AXIS_RY           = 0x010,
				AXIS_RZ           = 0x020,
				AXIS_SLIDER_0     = 0x040,
				AXIS_SLIDER_1     = 0x080,
				AXIS_POV_0        = 0x100,
				AXIS_POV_1        = 0x200,
				AXIS_POV_2        = 0x400,
				AXIS_POV_3        = 0x800,
				AXIS_ALL          = 0xFFF,
				DEADZONE_MAX      = 100,
				DEFAULT_AXES      = AXIS_X|AXIS_Y|AXIS_Z|AXIS_RX|AXIS_RY|AXIS_RZ|AXIS_POV_0|AXIS_POV_1|AXIS_POV_2|AXIS_POV_3,
				DEFAULT_DEADZONE  = 50
			};

			enum ScanMode
			{
				SCAN_MODE_ALL,
				SCAN_MODE_JOY
			};

			enum ScanResult
			{
				SCAN_INVALID_KEY = -1,
				SCAN_NO_KEY,
				SCAN_GOOD_KEY
			};

			class Key;

			void Acquire();
			void Unacquire();
			void Calibrate();
			void Poll();
			void Build(const Key*,uint);
			bool MapKey(Key&,wcstring,const System::Guid* = NULL,uint=0) const;
			const HeapString GetKeyName(const Key&) const;

			void BeginScanMode(HWND) const;
			ScanResult ScanKey(Key&,ScanMode=SCAN_MODE_ALL);
			void EndScanMode() const;

			class Key
			{
				friend class DirectInput;
				friend class Keyboard;
				friend class Joystick;

			public:

				bool MapVirtualKey(uint,uint,uint,uint);
				bool MapVirtualKey(GenericString);
				bool GetVirtualKey(ACCEL&) const;
				bool IsVirtualKey() const;
				bool GetToggle(bool&) const;

			private:

				union
				{
					const BYTE* data;
					DWORD vKey;
				};

				uint (*code)(const void* const);

			public:

				Key()
				: data(NULL), code(KeyNone) {}

				bool operator == (const Key& key) const
				{
					return data == key.data && code == key.code;
				}

				void Unmap()
				{
					data = NULL;
					code = KeyNone;
				}

				bool Assigned() const
				{
					return data;
				}

				uint GetState() const
				{
					return code( data );
				}

				template<typename Value>
				void GetState(Value& value,uint mask) const
				{
					value |= code( data ) & mask;
				}
			};

		private:

			class Base
			{
				static IDirectInput8& Create();

			public:

				explicit Base(HWND);
				~Base();

				IDirectInput8& com;
				HWND const hWnd;
			};

			class Device
			{
			public:

				void Enable(bool);

			protected:

				explicit Device(IDirectInputDevice8&);
				~Device();

				bool Acquire(void*,uint);
				void Unacquire();

				IDirectInputDevice8& com;
				bool enabled;
			};

			class Keyboard : public Device
			{
			public:

				explicit Keyboard(Base&);

				enum
				{
					MAX_KEYS = NUM_KEYBOARD_KEYS
				};

				void Acquire();
				void Unacquire();

				NST_FORCE_INLINE void Poll();

				bool Map(Key&,wcstring) const;
				bool Map(Key&,uint) const;

				void BeginScanMode(HWND) const;
				bool Scan(uchar (&)[MAX_KEYS]) const;
				ScanResult Scan(Key&) const;
				void EndScanMode() const;

				bool Assigned(const Key&) const;
				wcstring GetName(const Key&) const;

			private:

				enum
				{
					COOPERATIVE_FLAGS = DISCL_FOREGROUND|DISCL_NONEXCLUSIVE|DISCL_NOWINKEY,
					SCAN_COOPERATIVE_FLAGS = DISCL_BACKGROUND|DISCL_NONEXCLUSIVE
				};

				static BOOL CALLBACK EnumObjects(LPCDIDEVICEOBJECTINSTANCE,LPVOID);
				static IDirectInputDevice8& Create(IDirectInput8&);

				void SetCooperativeLevel(HWND,DWORD) const;
				void Clear();

				NST_NO_INLINE void OnError(HRESULT);

				typedef Object::Heap<BYTE,MAX_KEYS> Buffer;

				Buffer buffer;
				HWND const hWnd;

				static HeapString keyNames[MAX_KEYS];

			public:

				const uchar* GetBuffer() const
				{
					return buffer;
				}
			};

			class Joystick : public Device
			{
			public:

				Joystick(Base&,const DIDEVICEINSTANCE&);

				enum
				{
					POV_CENTER    = 0xFFFF,
					POV_UPRIGHT   =  45 * DI_DEGREES,
					POV_DOWNRIGHT = 135 * DI_DEGREES,
					POV_DOWNLEFT  = 225 * DI_DEGREES,
					POV_UPLEFT    = 315 * DI_DEGREES
				};

				enum Exception
				{
					ERR_API
				};

				void Acquire();
				void Unacquire();
				void Calibrate();

				NST_FORCE_INLINE void Poll();

				bool Map(Key&,wcstring) const;

				void BeginScanMode() const;
				bool Scan(Key&);
				void EndScanMode() const;

				bool Assigned(const Key&) const;
				bool SetAxisDeadZone(uint);
				wcstring GetName(const Key&) const;

			private:

				static IDirectInputDevice8& Create(Base&,const GUID&);

				void Clear();

				NST_NO_INLINE void OnError(HRESULT);

				class Caps
				{
					struct Context
					{
						Context(Caps&,IDirectInputDevice8&);

						Caps& caps;
						IDirectInputDevice8& com;
						uint numButtons;
					};

					enum
					{
						AXIS_MIN_RANGE = -1000,
						AXIS_MAX_RANGE = +1000
					};

					static BOOL CALLBACK EnumObjectsProc(LPCDIDEVICEOBJECTINSTANCE,LPVOID);

				public:

					Caps(IDirectInputDevice8&,const DIDEVICEINSTANCE&);

					enum
					{
						NUM_POVS    = 4,
						NUM_BUTTONS = 32
					};

					uint axes;
					const System::Guid guid;
					const HeapString name;
				};

				class Calibrator
				{
					long lX;
					long lY;
					long lZ;
					long lRx;
					long lRy;
					long lRz;

				public:

					Calibrator();

					inline void Reset(DIJOYSTATE&,bool);
					inline void Fix(DIJOYSTATE&) const;
				};

				const Caps caps;
				Object::Pod<DIJOYSTATE> state;
				Calibrator calibrator;
				bool calibrated;
				bool scanEnabled;
				uint deadZone;
				uint axes;

				enum
				{
					TABLE_KEYS = 32
				};

				struct Lookup;
				static const Lookup table[TABLE_KEYS];

			public:

				const System::Guid& GetGuid() const
				{
					return caps.guid;
				}

				const HeapString& GetName() const
				{
					return caps.name;
				}

				uint GetAxisDeadZone() const
				{
					return deadZone;
				}

				uint GetAvailableAxes() const
				{
					return caps.axes;
				}

				uint GetScannerAxes() const
				{
					return axes;
				}

				void ScanEnable(bool enable)
				{
					scanEnabled = enable;
				}

				bool ScanEnabled() const
				{
					return scanEnabled;
				}

				void SetScannerAxes(uint flags)
				{
					NST_ASSERT( flags <= AXIS_ALL );
					axes = flags;
				}

				void SetScannerAxes(uint flags,bool on)
				{
					NST_ASSERT( flags <= AXIS_ALL );
					axes = (on ? axes | flags : axes & ~flags);
				}
			};

			typedef Collection::Vector<Joystick> Joysticks;

			static BOOL CALLBACK EnumJoysticks(LPCDIDEVICEINSTANCE,LPVOID);

			static uint KeyDown     (const void* const);
			static uint KeyNegDir   (const void* const);
			static uint KeyPosDir   (const void* const);
			static uint KeyPovUp    (const void* const);
			static uint KeyPovRight (const void* const);
			static uint KeyPovDown  (const void* const);
			static uint KeyPovLeft  (const void* const);
			static uint KeyNone     (const void* const);

			Base base;
			Keyboard keyboard;
			Joysticks joysticks;

		public:

			bool MapKeyboard(Key& key,uint code) const
			{
				key.Unmap();
				return keyboard.Map( key, code );
			}

			uint NumJoysticks() const
			{
				return joysticks.Size();
			}

			bool JoystickScanEnabled(uint index) const
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].ScanEnabled();
			}

			void ScanEnableJoystick(uint index,bool enable)
			{
				NST_ASSERT( index < joysticks.Size() );
				joysticks[index].ScanEnable( enable );
			}

			const System::Guid& GetJoystickGuid(uint index) const
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].GetGuid();
			}

			const HeapString& GetJoystickName(uint index) const
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].GetName();
			}

			void Calibrate(uint index)
			{
				NST_ASSERT( index < joysticks.Size() );
				joysticks[index].Calibrate();
			}

			bool SetAxisDeadZone(uint index,uint deadZone)
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].SetAxisDeadZone( deadZone );
			}

			uint GetAxisDeadZone(uint index) const
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].GetAxisDeadZone();
			}

			uint GetAvailableAxes(uint index) const
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].GetAvailableAxes();
			}

			void SetScannerAxes(uint index,uint axes)
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].SetScannerAxes( axes );
			}

			void SetScannerAxes(uint index,uint axes,bool state)
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].SetScannerAxes( axes, state );
			}

			uint GetScannerAxes(uint index) const
			{
				NST_ASSERT( index < joysticks.Size() );
				return joysticks[index].GetScannerAxes();
			}

			const uchar* GetKeyboardBuffer() const
			{
				return keyboard.GetBuffer();
			}

			bool AnyPressed()
			{
				Key key;
				return ScanKey( key ) != SCAN_NO_KEY;
			}
		};
	}
}

#endif
