    zint tcl binding readme
    -----------------------
    2025-12-19
    (C) Harald Oehlmann
    (C) Git Lost
    harald.oehlmann@users.sourceforge.net

What: tcl binding for Zint Barcode Generator library

Build:
The header files of a Tcl and Tk build are required for the build.
They are included in a Tcl/Tk source tree or in an installed Tcl/Tk.

There are the standard TCL build environments available:

- configure based build system for Linux/macOS and MSYS/Cygwin/MSYS2 (Windows)
  (contributed by Christian Werner) - see "docs/manual.txt" Annex C for details
- "win\makefile.vc" for Microsoft Visual Studio command prompt (nmake) - see
  "win\README.txt" for details

The GS1 Syntax Engine must be available. See the instructions in "README.linux"
for Linux, "README.macos" for macOS, "README.bsd" for BSD, and "win32\README"
for Windows to install.

In the distributed binaries for Windows, the Tcl8 DLL "zint2160t.dll" is 32-bit,
built with MSVC 2015, and the Tcl9 DLL "tcl9zint2160.dll" is 64-bit, built with
MSVC 2022.

Usage:

package require zint
zint help

Most options are identical to the command line tool.
Details may be found in the zint manual.

Demo:
The demo folder contains a visual demo program.
