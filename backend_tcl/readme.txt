    zint tcl binding readme
    -----------------------
    2014-06-30
    (C) Harald Oehlmann
    harald.oehlmann@users.sourceforge.net

What: tcl binding for zint bar code generator library

Build:
The header files of a TCL and Tk build are required for the build.

- MS-VC6 project file "zint_tcl.dsp" may be opened by the GUI.
- Linux/Unix build is provided by the configure script.
  Thanks to Christian Werner for that.

Usage:

load zint.dll
zint help

Most options are identical to the command line tool.
Details may be found in the zint manual.

Demo:
The demo folder contains a visual demo program.
