## About
This project started as a fork of the original Nestopia source code, plus the
Linux port. The purpose of the project is to make sure people who want
a standalone GUI for the Nestopia emulator have this option available.

Project Goals:
* Maintain a functional GUI release of the Nestopia emulator on major desktop platforms

Contributing/Issues:
* Contributions will be reviewed for anything related to the standalone GUI builds
* Issues related to core emulation will be closed. This project no longer maintains the core emulator. Please submit issues about core emulation upstream at https://gitlab.com/jgemu/nestopia
* When not using a tagged release, please understand that the code is volatile and nothing is set in stone.

## Cheats
If you want the best database of cheats available in Nestopia format, make sure you check out
[Mighty Mo's Cheat Code Pack](https://github.com/mightymo77/MightyMos-Cheat-Code-Pack/releases).

## Win32 Build
To build the win32 solution with Visual Studio 2010:
1. Ensure you have the DirectX 9 SDK
2. Manually zip NstDatabase.xml to the destination source/core/database/NstDatabase.zip
3. Open projects/nestopia.sln
4. Build in release mode

## Linux/macOS Build
!!! WARNING !!!
Currently this build is being transitioned to using Nestopia JG with the QTea frontend:
```
https://gitlab.com/jgemu/nestopia
https://gitlab.com/jgemu/qtea
```

Helper scripts and releases will be hosted here in the future. Stop worrying, everything will be fine by release time.
