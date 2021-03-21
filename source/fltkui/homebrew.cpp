/*
 * Nestopia UE
 *
 * Copyright (C) 2018-2018 Phil Smith
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "homebrew.h"

extern Emulator emulator;

void nst_homebrew_init() {
	Homebrew homebrew(emulator);
	homebrew.ClearPorts();

	if (conf.misc_homebrew_exit != -1)
	{
		homebrew.SetExitPort(conf.misc_homebrew_exit);
	}

	if (conf.misc_homebrew_stdout != -1)
	{
		homebrew.SetStdOutPort(conf.misc_homebrew_stdout);
	}

	if (conf.misc_homebrew_stderr != -1)
	{
		homebrew.SetStdErrPort(conf.misc_homebrew_stderr);
	}
}
