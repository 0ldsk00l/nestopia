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

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include "NstCpu.hpp"
#include "NstHomebrew.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Homebrew::Homebrew(Cpu& c)
		: cpu(c)
		, exitAddress(0)
		, exitSet(false)
		, exitPort(NULL)
		, stdOutAddress(0)
		, stdOutSet(false)
		, stdOutPort(NULL)
		, stdErrAddress(0)
		, stdErrSet(false)
		, stdErrPort(NULL)
		{}

		Homebrew::~Homebrew()
		{
			ClearPorts();
		}

		void Homebrew::Reset()
		{
			ActivateExitPort();
			ActivateStdOutPort();
			ActivateStdErrPort();
		}

		void Homebrew::ClearPorts()
		{
			ClearExitPort();
			ClearStdOutPort();
			ClearStdErrPort();
		}

		Result Homebrew::SetExitPort
		(
			const word address,
			const bool activate
		)
		{
			if
			(
				exitSet
				&& exitAddress == address
				&& (!activate || exitPort != NULL)
			)
				return RESULT_NOP;

			ClearExitPort();

			exitAddress = address;
			exitSet = true;

			if (activate)
				return ActivateExitPort();
			else
				return RESULT_OK;
		}

		Result Homebrew::ClearExitPort()
		{
			exitSet = false;

			if (exitPort != NULL)
			{
				cpu.Unlink
				(
					exitAddress,
					this,
					&Homebrew::Peek_Exit,
					&Homebrew::Poke_Exit
				);
				exitPort = NULL;
				return RESULT_OK;
			}
			else
				return RESULT_NOP;
		}

		Result Homebrew::SetStdOutPort
		(
			const word address,
			const bool activate
		)
		{
			if
			(
				stdOutSet
				&& stdOutAddress == address
				&& (!activate || stdOutPort != NULL)
			)
				return RESULT_NOP;

			ClearStdOutPort();

			stdOutAddress = address;
			stdOutSet = true;

			if (activate)
				return ActivateStdOutPort();
			else
				return RESULT_OK;
		}

		Result Homebrew::ClearStdOutPort()
		{
			stdOutSet = false;

			if (stdOutPort != NULL)
			{
				cpu.Unlink
				(
					stdOutAddress,
					this,
					&Homebrew::Peek_StdOut,
					&Homebrew::Poke_StdOut
				);
				stdOutPort = NULL;
				return RESULT_OK;
			}
			else
				return RESULT_NOP;
		}

		Result Homebrew::SetStdErrPort
		(
			const word address,
			const bool activate
		)
		{
			if
			(
				stdErrSet
				&& stdErrAddress == address
				&& (!activate || stdErrPort != NULL)
			)
				return RESULT_NOP;

			ClearStdErrPort();

			stdErrAddress = address;
			stdErrSet = true;

			if (activate)
				return ActivateStdErrPort();
			else
				return RESULT_OK;
		}

		Result Homebrew::ClearStdErrPort()
		{
			stdErrSet = false;

			if (stdErrPort != NULL)
			{
				cpu.Unlink
				(
					stdErrAddress,
					this,
					&Homebrew::Peek_StdErr,
					&Homebrew::Poke_StdErr
				);
				stdErrPort = NULL;
				return RESULT_OK;
			}
			else
				return RESULT_NOP;
		}

		dword Homebrew::NumPorts() const
		{
			return (exitPort != NULL ? 1 : 0)
				+ (stdOutPort != NULL ? 1 : 0)
				+ (stdErrPort != NULL ? 1 : 0);
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_PEEK_A(Homebrew,Exit)
		{
			return exitPort->Peek( address );
		}

		NES_POKE_D(Homebrew,Exit)
		{
			std::exit( data );
		}

		NES_PEEK_A(Homebrew,StdOut)
		{
			return stdOutPort->Peek( address );
		}

		NES_POKE_D(Homebrew,StdOut)
		{
			std::cout << (static_cast<unsigned char>(data & 0xFF));

			if (data == '\n')
				std::cout << std::flush;
		}

		NES_PEEK_A(Homebrew,StdErr)
		{
			return stdErrPort->Peek( address );
		}

		NES_POKE_D(Homebrew,StdErr)
		{
			std::cerr << (static_cast<unsigned char>(data & 0xFF));

			if (data == '\n')
				std::cerr << std::flush;
		}

		Result Homebrew::ActivateExitPort()
		{
			if (exitSet && exitPort == NULL)
			{
				exitPort = cpu.Link
				(
					exitAddress,
					Cpu::LEVEL_HIGH,
					this,
					&Homebrew::Peek_Exit,
					&Homebrew::Poke_Exit
				);
				return RESULT_OK;
			}
			else
				return RESULT_NOP;
		}

		Result Homebrew::ActivateStdOutPort()
		{
			if (stdOutSet && stdOutPort == NULL)
			{
				stdOutPort = cpu.Link
				(
					stdOutAddress,
					Cpu::LEVEL_HIGH,
					this,
					&Homebrew::Peek_StdOut,
					&Homebrew::Poke_StdOut
				);
				return RESULT_OK;
			}
			else
				return RESULT_NOP;
		}

		Result Homebrew::ActivateStdErrPort()
		{
			if (stdErrSet && stdErrPort == NULL)
			{
				stdErrPort = cpu.Link
				(
					stdErrAddress,
					Cpu::LEVEL_HIGH,
					this,
					&Homebrew::Peek_StdErr,
					&Homebrew::Poke_StdErr
				);
				return RESULT_OK;
			}
			else
				return RESULT_NOP;
		}
	}
}
