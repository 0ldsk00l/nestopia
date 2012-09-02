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

#ifndef NST_CTRL_EDIT_H
#define NST_CTRL_EDIT_H

#pragma once

#include "NstCtrlStandard.hpp"

namespace Nestopia
{
	namespace Window
	{
		namespace Control
		{
			class Edit : public Generic
			{
				class TryText
				{
					const Edit& edit;

				public:

					TryText(const Edit& e)
					: edit(e) {}

					template<typename T>
					void operator << (const T* t) const
					{
						if (*t)
							edit.Text() << t;
					}

					template<typename T>
					uint operator >> (T& t) const
					{
						return (edit.Text() >> t);
					}
				};

			public:

				Edit(HWND hWnd=NULL)
				: Generic( hWnd ) {}

				Edit(HWND hWnd,uint id)
				: Generic( hWnd, id ) {}

				void Limit(uint) const;
				void SetNumberOnly(bool=true) const;

				void Clear() const
				{
					Text().Clear();
				}

				template<typename T>
				void operator << (const T& t) const
				{
					Text() << t;
				}

				template<typename T>
				uint operator >> (T& t) const
				{
					return (Text() >> t);
				}

				TryText Try() const
				{
					return *this;
				}
			};
		}
	}
}

#endif
