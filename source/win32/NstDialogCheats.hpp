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

#ifndef NST_DIALOG_CHEATS_H
#define NST_DIALOG_CHEATS_H

#pragma once

#include <set>
#include "NstWindowDialog.hpp"
#include "../core/api/NstApiCheats.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Cheats
		{
		public:

			Cheats(Managers::Emulator&,const Configuration&,const Managers::Paths&);
			~Cheats();

			enum CodesType
			{
				PERMANENT_CODES,
				TEMPORARY_CODES
			};

			enum
			{
				MAX_CODES = 0xFFFF
			};

			typedef Nes::Cheats::Code NesCode;

			struct Code
			{
				Code();

				enum
				{
					NO_COMPARE = 0xFFFF
				};

				typedef String::Stack<8,wchar_t> GenieCode;

				NesCode ToNesCode() const;
				void FromNesCode(const NesCode&);
				GenieCode ToGenieCode() const;

				bool operator < (const Code&) const;

				HeapString description;
				uint crc;
				ushort address;
				ushort compare;
				uchar value;
				bool enabled;
			};

			typedef std::set<Code> Codes;

			void Open();
			void Save(Configuration&) const;
			void Flush();
			bool Load(const Path&);
			bool Save(const Path&) const;

		private:

			class MainDialog;

			struct Searcher
			{
				Searcher();

				enum
				{
					NO_FILTER = 0xFFFF
				};

				ushort filter;
				uchar a;
				uchar b;
				uchar ram[Nes::Cheats::RAM_SIZE];
			};

			static bool Import(Codes&,const Path&);
			static bool Export(const Codes&,const Path&);

			const Managers::Paths& paths;
			Managers::Emulator& emulator;
			Codes codes[2];
			Searcher searcher;
			bool showHexMainDialog;
			bool showHexSubDialogs;

		public:

			const Codes& GetCodes(CodesType type) const
			{
				return codes[type];
			}
		};
	}
}

#endif
