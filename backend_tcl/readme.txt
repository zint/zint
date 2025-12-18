    zint tcl binding readme
    -----------------------
    2025-01-22
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

The GS1 syntax engine may be added optionally.
https://github.com/gs1/gs1-syntax-engine
Please look to the top of makefile.vc to activate it.

The GS1 syntax engine project currently only allows to build a dll and an import lib.
This hopefully changes in future.
Remark, that the syntax engine dll may not be included in a starkit or zipkit.

Usage:

package require zint
zint help

Most options are identical to the command line tool.
Details may be found in the zint manual.

Demo:
The demo folder contains a visual demo program.
