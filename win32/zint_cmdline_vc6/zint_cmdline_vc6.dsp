# Microsoft Developer Studio Project File - Name="zint_cmdline_vc6" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=zint_cmdline_vc6 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zint_cmdline_vc6.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zint_cmdline_vc6.mak" CFG="zint_cmdline_vc6 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zint_cmdline_vc6 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "zint_cmdline_vc6 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "zint_cmdline_vc6 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\backend" /I "..\..\..\zlib" /I "..\..\..\lpng" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /D ZINT_VERSION="\"2.13.0.9\"" /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libpng.lib zlib.lib /nologo /subsystem:console /machine:I386 /out:"Release/zint.exe" /libpath:"..\..\..\zlib" /libpath:"..\..\..\lpng"

!ELSEIF  "$(CFG)" == "zint_cmdline_vc6 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\backend" /I "..\..\..\lpng" /I "..\..\..\zlib" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "ZINT_NO_PNG" /D "DEBUG" /YX /FD /GZ /D ZINT_VERSION="\"2.13.0.9\"" /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libpng.lib zlib.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/zint.exe" /pdbtype:sept /libpath:"..\..\..\lpng" /libpath:"..\..\..\zlib"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "zint_cmdline_vc6 - Win32 Release"
# Name "zint_cmdline_vc6 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\backend\2of5.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\auspost.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\aztec.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\bc412.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\bmp.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\codablock.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\code.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\code1.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\code128.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\code16k.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\code49.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\common.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\composite.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\dmatrix.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\dotcode.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\dxfilmedge.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\eci.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\emf.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\filemem.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\general_field.c
# End Source File
# Begin Source File

SOURCE=..\..\getopt\getopt.c
# End Source File
# Begin Source File

SOURCE=..\..\getopt\getopt1.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\gif.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\gridmtx.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\gs1.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\hanxin.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\imail.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\large.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\library.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\mailmark.c
# End Source File
# Begin Source File

SOURCE=..\..\frontend\main.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\maxicode.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\medical.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\output.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\pcx.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\pdf417.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\plessey.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\png.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\postal.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\ps.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\qr.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\raster.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\reedsol.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\rss.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\svg.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\telepen.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\tif.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\ultra.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\upcean.c
# End Source File
# Begin Source File

SOURCE=..\..\backend\vector.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\zint.rc
# End Source File
# Begin Source File

SOURCE=.\zint_black_vc6.ico
# End Source File
# End Group
# End Target
# End Project
