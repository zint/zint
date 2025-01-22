2025-01-22
(C) Harald Oehlmann
harald.oehlmann@users.sourceforge.net

To build using nmake on Windows:

Download the tcl and tk 8.6.16 or 9.0.1 source from:

  https://sourceforge.net/projects/tcl/files/Tcl/

Unzip them into a folder (example: c:\temp)

  In the following:
  <tcl> is c:\temp\tcl8.6.16 (or 9.0.1)
  <tk> is c:\temp\tk8.6.16 (or 9.0.1)
  <inst> is the Tcl/Tk install directory (example: c:\Tcl)
  <zint> is the Zint project root directory

Start a Visual Studio command prompt

  cd <tcl>\win
  nmake -f makefile.vc release
  nmake -f makefile.vc install INSTALLDIR=<inst>

  cd <tk>\win
  nmake -f makefile.vc release TCLDIR=<tcl>
  nmake -f makefile.vc install TCLDIR=<tcl> INSTALLDIR=<inst>

  cd <zint>\backend_tcl\win
  nmake -f makefile.vc zint TCLDIR=<tcl> TKDIR=<tk>
  nmake -f makefile.vc install TCLDIR=<tcl> INSTALLDIR=<inst>
