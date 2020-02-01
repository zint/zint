# Microsoft Developer Studio Project File - Name="zint_tcl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=zint_tcl - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zint_tcl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zint_tcl.mak" CFG="zint_tcl - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zint_tcl - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "zint_tcl - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "zint_tcl - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZINT_TCL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\backend" /I "C:\myprograms\tcl8.5\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZINT_TCL_EXPORTS" /D "NO_PNG" /FR /YX /FD /D ZINT_VERSION="\"2.7.1\"" /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib tclstub85.lib tkstub85.lib /nologo /dll /machine:I386 /out:"zint.dll" /libpath:"C:\myprograms\tcl8.5\lib"

!ELSEIF  "$(CFG)" == "zint_tcl - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZINT_TCL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\backend" /I "C:\myprograms\tcl8.5\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZINT_TCL_EXPORTS" /D "NO_PNG" /FR /YX /FD /GZ /D ZINT_VERSION="\"2.7.1\"" /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib tclstub85.lib tkstub85.lib /nologo /dll /debug /machine:I386 /out:"Debug/zint.dll" /pdbtype:sept /libpath:"C:\myprograms\tcl8.5\lib"

!ENDIF 

# Begin Target

# Name "zint_tcl - Win32 Release"
# Name "zint_tcl - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\backend\2of5.c
# End Source File
# Begin Source File

SOURCE=..\backend\auspost.c
# End Source File
# Begin Source File

SOURCE=..\backend\aztec.c
# End Source File
# Begin Source File

SOURCE=..\backend\bmp.c
# End Source File
# Begin Source File

SOURCE=..\backend\codablock.c
# End Source File
# Begin Source File

SOURCE=..\backend\code.c
# End Source File
# Begin Source File

SOURCE=..\backend\code1.c
# End Source File
# Begin Source File

SOURCE=..\backend\code128.c
# End Source File
# Begin Source File

SOURCE=..\backend\code16k.c
# End Source File
# Begin Source File

SOURCE=..\backend\code49.c
# End Source File
# Begin Source File

SOURCE=..\backend\common.c
# End Source File
# Begin Source File

SOURCE=..\backend\composite.c
# End Source File
# Begin Source File

SOURCE=..\backend\dmatrix.c
# End Source File
# Begin Source File

SOURCE=..\backend\dotcode.c
# End Source File
# Begin Source File

SOURCE=..\backend\eci.c
# End Source File
# Begin Source File

SOURCE=..\backend\emf.c
# End Source File
# Begin Source File

SOURCE=..\backend\gb18030.c
# End Source File
# Begin Source File

SOURCE=..\backend\gb2312.c
# End Source File
# Begin Source File

SOURCE=..\backend\general_field.c
# End Source File
# Begin Source File

SOURCE=..\backend\gif.c
# End Source File
# Begin Source File

SOURCE=..\backend\gridmtx.c
# End Source File
# Begin Source File

SOURCE=..\backend\gs1.c
# End Source File
# Begin Source File

SOURCE=..\backend\hanxin.c
# End Source File
# Begin Source File

SOURCE=..\backend\imail.c
# End Source File
# Begin Source File

SOURCE=..\backend\large.c
# End Source File
# Begin Source File

SOURCE=..\backend\library.c
# End Source File
# Begin Source File

SOURCE=..\backend\mailmark.c
# End Source File
# Begin Source File

SOURCE=..\backend\maxicode.c
# End Source File
# Begin Source File

SOURCE=..\backend\medical.c
# End Source File
# Begin Source File

SOURCE=..\backend\pcx.c
# End Source File
# Begin Source File

SOURCE=..\backend\pdf417.c
# End Source File
# Begin Source File

SOURCE=..\backend\plessey.c
# End Source File
# Begin Source File

SOURCE=..\backend\png.c
# End Source File
# Begin Source File

SOURCE=..\backend\postal.c
# End Source File
# Begin Source File

SOURCE=..\backend\ps.c
# End Source File
# Begin Source File

SOURCE=..\backend\qr.c
# End Source File
# Begin Source File

SOURCE=..\backend\raster.c
# End Source File
# Begin Source File

SOURCE=..\backend\reedsol.c
# End Source File
# Begin Source File

SOURCE=..\backend\rss.c
# End Source File
# Begin Source File

SOURCE=..\backend\sjis.c
# End Source File
# Begin Source File

SOURCE=..\backend\svg.c
# End Source File
# Begin Source File

SOURCE=..\backend\telepen.c
# End Source File
# Begin Source File

SOURCE=..\backend\tif.c
# End Source File
# Begin Source File

SOURCE=..\backend\ultra.c
# End Source File
# Begin Source File

SOURCE=..\backend\upcean.c
# End Source File
# Begin Source File

SOURCE=..\backend\vector.c
# End Source File
# Begin Source File

SOURCE=.\zint.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
