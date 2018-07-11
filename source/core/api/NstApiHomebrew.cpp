////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2018-2018 Phil Smith
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
#include "../NstMachine.hpp"
#include "../NstHomebrew.hpp"
#include "NstApiHomebrew.hpp"
#include "NstApiMachine.hpp"

namespace Nes
{
	namespace Api
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Result Homebrew::SetExitPort(ushort address) throw()
		{
			if (emulator.tracker.IsLocked( true ))
				return RESULT_ERR_NOT_READY;

			try
			{
				if (emulator.homebrew == NULL)
					emulator.homebrew = new Core::Homebrew( emulator.cpu );

				return emulator.tracker.TryResync
				(
					emulator.homebrew->SetExitPort
					(
						address,
						emulator.Is(Machine::GAME)
					),
					true
				);
			}
			catch (const std::bad_alloc&)
			{
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return RESULT_ERR_GENERIC;
			}
		}

		Result Homebrew::ClearExitPort() throw()
		{
			if (emulator.tracker.IsLocked( true ))
				return RESULT_ERR_NOT_READY;

			if (!emulator.homebrew)
				return RESULT_ERR_INVALID_PARAM;

			const Result result = emulator.tracker.TryResync
			(
				emulator.homebrew->ClearExitPort(),
				true
			);

			if (!emulator.homebrew->NumPorts())
			{
				delete emulator.homebrew;
				emulator.homebrew = NULL;
			}

			return result;
		}

		Result Homebrew::SetStdOutPort(ushort address) throw()
		{
			if (emulator.tracker.IsLocked( true ))
				return RESULT_ERR_NOT_READY;

			try
			{
				if (emulator.homebrew == NULL)
					emulator.homebrew = new Core::Homebrew( emulator.cpu );

				return emulator.tracker.TryResync
				(
					emulator.homebrew->SetStdOutPort
					(
						address,
						emulator.Is(Machine::GAME)
					),
					true
				);
			}
			catch (const std::bad_alloc&)
			{
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return RESULT_ERR_GENERIC;
			}
		}

		Result Homebrew::ClearStdOutPort() throw()
		{
			if (emulator.tracker.IsLocked( true ))
				return RESULT_ERR_NOT_READY;

			if (!emulator.homebrew)
				return RESULT_ERR_INVALID_PARAM;

			const Result result = emulator.tracker.TryResync
			(
				emulator.homebrew->ClearStdOutPort(),
				true
			);

			if (!emulator.homebrew->NumPorts())
			{
				delete emulator.homebrew;
				emulator.homebrew = NULL;
			}

			return result;
		}

		Result Homebrew::SetStdErrPort(ushort address) throw()
		{
			if (emulator.tracker.IsLocked( true ))
				return RESULT_ERR_NOT_READY;

			try
			{
				if (emulator.homebrew == NULL)
					emulator.homebrew = new Core::Homebrew( emulator.cpu );

				return emulator.tracker.TryResync
				(
					emulator.homebrew->SetStdErrPort
					(
						address,
						emulator.Is(Machine::GAME)
					),
					true
				);
			}
			catch (const std::bad_alloc&)
			{
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return RESULT_ERR_GENERIC;
			}
		}

		Result Homebrew::ClearStdErrPort() throw()
		{
			if (emulator.tracker.IsLocked( true ))
				return RESULT_ERR_NOT_READY;

			if (!emulator.homebrew)
				return RESULT_ERR_INVALID_PARAM;

			const Result result = emulator.tracker.TryResync
			(
				emulator.homebrew->ClearStdErrPort(),
				true
			);

			if (!emulator.homebrew->NumPorts())
			{
				delete emulator.homebrew;
				emulator.homebrew = NULL;
			}

			return result;
		}

		ulong Homebrew::NumPorts() const throw()
		{
			return emulator.homebrew ? emulator.homebrew->NumPorts() : 0;
		}

		Result Homebrew::ClearPorts() throw()
		{
			if (emulator.tracker.IsLocked( true ))
				return RESULT_ERR_NOT_READY;

			if (!emulator.homebrew)
				return RESULT_NOP;

			if (emulator.homebrew->NumPorts())
				emulator.tracker.Resync( true );

			delete emulator.homebrew;
			emulator.homebrew = NULL;

			return RESULT_OK;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}

