Harald Oehlmann
2026-11-18

Why to use VC6 ?
It avoids DLL Hell as the runtime is present on all Windows Versions since XP.
I compile on Windows 10 64 bit.

How to compile:

$ZR is the zint root folder (this file is in $ZR\win32\zint_cmdline_vc6)

* get vc6 shell
if not available in the Start Menu, open a Command Prompt and run
"C:\Program Files (x86)\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT"

a) zlib (version: 1.3.1; for current version 1.3.2 see below)
* put zlib to $ZR\..\zlib
cd $ZR\..\zlib
nmake -f win32\Makefile.msc clean
nmake -f win32\Makefile.msc
-> generates zlib.lib, zlib1.dll
-> NOTE current version is 1.3.2 but not usable without hacking in
        "-Dz_off_t=long -DEWOULDBLOCK=EAGAIN" in both zlib and lpng

b) lpng (current version: 1.6.56)
* put libpng to $ZR\..\lpng
cd $ZR\..\lpng
nmake -f scripts\makefile.vcwin32 clean
nmake -f scripts\makefile.vcwin32
-> generates $ZR\..\lpng\libpng.lib

c) open zint_cmdline_vc6.dsw in this folder with the msvc6 gui and compile
