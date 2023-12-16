Harald Oehlmann
2021-06-10

Why to use VC6 ?
It avoids DLL Hell as the runtime is present on all Windows Versions since XP.
I compile on Windows 10 64 bit.

How to compile:

$ZR is the zint root folder (this file is in $ZR\win32\zint_cmdline_vc6)

* get vc6 shell
if not available in the Start Menu, open a Command Prompt and run
"C:\Program Files (x86)\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT"

a) zlib (current version: 1.2.13)
* put zlib to $ZR\..\zlib
cd $ZR\..\zlib
nmake -f win32\Makefile.msc
-> generates zlib.lib, zlib1.dll

b) lpng (current version: 1.6.39)
* put libpng to $ZR\..\lpng
cd $ZR\..\lpng
nmake -f scripts\makefile.vcwin32
-> generates $ZR\..\lpng\libpng.lib

c) open zint_cmdline_vc6.dsw in this folder with the msvc6 gui and compile
