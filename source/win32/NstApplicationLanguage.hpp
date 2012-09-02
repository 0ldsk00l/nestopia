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

#ifndef NST_APPLICATION_LANGUAGE_H
#define NST_APPLICATION_LANGUAGE_H

#pragma once

#include <vector>
#include "NstSystemDll.hpp"
#include "NstApplicationInstance.hpp"

namespace Nestopia
{
	namespace Application
	{
		class Instance::Language
		{
		public:

			void Load(const Configuration&);
			void Save(Configuration&) const;

			typedef std::vector<Path> Paths;

			void UpdateResource(wcstring);
			void EnumerateResources(Paths&) const;

		private:

			void Load(wcstring);

			struct Resource : System::Dll
			{
				bool Load(wcstring);

				Path path;
			};

			Resource resource;

		public:

			HMODULE GetResourceHandle() const
			{
				return resource.GetHandle();
			}

			const Path& GetResourcePath() const
			{
				return resource.path;
			}
		};
	}
}

#endif
