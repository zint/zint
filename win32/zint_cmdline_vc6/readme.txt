Harald Oehlmann
2016-01-12

Why to use VC6 ?
It avoids DLL Hell as the runtime is present on all Windows Versions since XP.
I compile on Windows 10 64 bit.

How to compile:

$ZR is the zint root folder (this file is in $ZR/win32/zint_cmdline_vc6)

a) zlib (current version: 1.2.8)
* put zlib to $ZR\..\zlib
* get vc6 shell
cd $ZR\..\zlib
nmake -f win32\Makefile.msc
-> generates zlib.lib, zlib1.dll
-> generates $ZR\..\lpng\libpng.lib
b) lpng (current version: 1.6.20)
* put libpng to $ZR/../lpng
cd $ZR\..\lpng
nmake -f scripts\makefile.vcwin32
-> generates $ZR\..\lpng\libpng.lib
c) open the files in this folder with the msvc6 gui and compile