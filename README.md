## About
This project is a fork of the original Nestopia source code, plus the 
Linux port. The purpose of the project is to make sure people who want
a standalone GUI for the Nestopia emulator have this option available.

Current Project Goals:
* Move the GTK+ project focus to a simple UI on top of a community maintained core
* Merge win32 sources back into main project and set up CI builds for Windows binaries
* Allow loading different forks/revisions of the emulator core (Maybe)

Contributing/Issues:
* Contributions will be reviewed for anything related to the standalone GUI builds
* Issues related to core emulation will be closed. This project no longer maintains the core emulator.
* When not using a tagged release, please understand that the code is volatile and nothing is set in stone.

The following platforms are supported:
* Linux, BSD, Windows

Libretro notes:
The libretro port is no longer maintained in this repo, and is now maintained by the
libretro community. For libretro-specific issues, please use the libretro repository:
https://github.com/libretro/nestopia

This project depends on the following libraries:
gtk3, libsdl2, libepoxy, libarchive, zlib

## Installing Dependencies
Install dependencies required for building on Debian-based Linux distributions:
```
apt-get install build-essential autoconf autoconf-archive automake autotools-dev libgtk-3-dev libsdl2-dev libepoxy-dev libarchive-dev zlib1g-dev
```

## GTK Build
To build using Autotools (optional arguments in square brackets):
```
autoreconf -vif
./configure [--enable-doc]
make
```
Optionally:
```
make install
```
## Win32 Build
To build the win32 solution with Visual Studio 2010:
1. Ensure you have the DirectX 9 SDK
2. Manually zip NstDatabase.xml to the destination source/core/database/NstDatabase.zip
3. Open projects/nestopia.sln
4. Build in release mode
