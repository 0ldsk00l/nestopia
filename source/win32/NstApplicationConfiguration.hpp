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

#ifndef NST_APPLICATION_CONFIGURATION_H
#define NST_APPLICATION_CONFIGURATION_H

#pragma once

#include "NstString.hpp"

namespace Nestopia
{
	namespace Application
	{
		class Configuration
		{
			struct Node;

		public:

			Configuration();
			~Configuration();

			void Reset(bool=true);
			void EnableSaving(bool=true);
			const Path& GetStartupFile() const;

			class ConstSection : public ImplicitBool<ConstSection>
			{
			public:

				ConstSection operator [] (cstring) const;
				ConstSection operator [] (uint) const;

				GenericString Str() const;

				ulong Int() const;
				ulong Int(ulong) const;

				bool Yes() const;
				bool No() const;

			private:

				const Node* node;

			public:

				ConstSection(const Node* n)
				: node(n) {}

				bool operator ! () const
				{
					return !node;
				}
			};

			class Section
			{
			public:

				Section operator [] (cstring);
				Section operator [] (uint);

				HeapString& Str();

			private:

				Node* node;

				class IntProxy
				{
					Node& node;

				public:

					IntProxy(Node& n)
					: node(n) {}

					void operator = (ulong);
				};

				class YesNoProxy
				{
					Node& node;

				public:

					YesNoProxy(Node& n)
					: node(n) {}

					void operator = (bool);
				};

			public:

				Section(Node* n)
				: node(n) {}

				IntProxy Int()
				{
					return *node;
				}

				YesNoProxy YesNo()
				{
					return *node;
				}
			};

			Section operator [] (cstring);
			ConstSection operator [] (cstring) const;

		private:

			Node& root;
			bool save;
			Path startupFile;
		};
	}
}

#endif
