## About
This project is a fork of the original Nestopia source code, plus the 
Linux port. The purpose of the project is to enhance the original, and
ensure it continues to work on modern operating systems.

The following platforms are supported:
* Linux, BSD, Windows

This project depends on the following libraries:
gtk3, libsdl2, libepoxy, libarchive, zlib

## Installing Dependencies
Install dependencies required for building on Debian-based Linux distributions:
```
apt-get install build-essential autoconf autoconf-archive automake autotools-dev libgtk-3-dev libsdl2-dev libepoxy-dev libarchive-dev zlib1g-dev
```

## Building
To build using Autotools (optional arguments in square brackets):
```
autoreconf -vif
./configure [--enable-gui] [--enable-doc]
make
```
Optionally:
```
make install
```
In order to bootstrap the Autotools you will need:

1.  **Autoconf**; latest 2.69 release (http://www.gnu.org/software/autoconf/)

    GNU Autoconf produces the ./configure script from configure.ac.

2.  **Automake**; latest 1.15 release (http://www.gnu.org/software/automake/)

    GNU Automake produces the Makefile.in precursor, that is processed with ./configure to yield the final Makefile.

3.  **Autoconf Archive**; latest 2016.09.16 release (http://www.gnu.org/software/autoconf-archive/)

    The configure.ac requires a number of m4 macros from the Autoconf archive.
