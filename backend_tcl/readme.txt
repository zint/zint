    zint tcl binding readme
    -----------------------
    2024-12-09
    (C) Harald Oehlmann
    (C) Git Lost
    harald.oehlmann@users.sourceforge.net

What: tcl binding for zint bar code generator library

Build:
The header files of a TCL and Tk build are required for the build.
They are included in a TCL/Tk source tree or in an installed TCL/Tk.

There are 3 build environments available:
- configure based build system for Linux/MacOS/MSYS/CYGWIN (Windows)
 Contributed by Christian Werner.
- win/Makefile.vc for TCL/Tk MS-Visual Studio build environment
- zint_tcl.vxproj for interactive VisualStudio build.

Usage:

package require zint
zint help

Most options are identical to the command line tool.
Details may be found in the zint manual.

Demo:
The demo folder contains a visual demo program.
