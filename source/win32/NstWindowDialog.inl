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

namespace Nestopia
{
	namespace Window
	{
		template<typename Owner,typename MsgArray>
		Dialog::Dialog(uint i,Owner* owner,MsgArray& msgArray)
		:
		Custom ( owner, msgArray ),
		id     ( i )
		{
			Init();
		}

		template<typename Owner,typename MsgArray,typename CmdArray>
		Dialog::Dialog(uint i,Owner* owner,MsgArray& msgArray,CmdArray& cmdArray)
		:
		Custom     ( owner, msgArray ),
		cmdHandler ( owner, cmdArray ),
		id         ( i )
		{
			Init();
		}

		template<typename Owner,typename Msg>
		Dialog::Dialog(uint i,uint mid,Owner* owner,Msg msg)
		:
		Custom ( mid, owner, msg ),
		id     ( i )
		{
			Init();
		}
	}
}
