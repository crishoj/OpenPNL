# Microsoft Developer Studio Project File - Name="cxcore" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=cxcore - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cxcore.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cxcore.mak" CFG="cxcore - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cxcore - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "cxcore - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cxcore - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\temp\cxcore\Release"
# PROP Intermediate_Dir "..\..\..\temp\cxcore\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe -lib
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W4 /Zi /O2 /Ob2 /I "..\src" /I "..\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "CXCORE_DLL" /FR /Yu"_cxcore.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\..\bin\cxcore.dll" /implib:"../../../lib/cxcore.lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "cxcore - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\temp\cxcore\Debug"
# PROP Intermediate_Dir "..\..\..\temp\cxcore\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe -lib
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm- /Zi /Od /I "..\src" /I "..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "CXCORE_DLL" /FR /Yu"_cxcore.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:I386 /out:"..\..\..\bin\cxcored.dll" /implib:"../../../lib/cxcored.lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "cxcore - Win32 Release"
# Name "cxcore - Win32 Debug"
# Begin Group "Src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\cxalloc.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxarithm.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxarray.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxcmp.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxconvert.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxcopy.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxdatastructs.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxdrawing.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxdxt.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxerror.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxjacobieigens.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxlogic.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxmathfuncs.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxmatmul.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxmatrix.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxmean.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxmeansdv.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxminmaxloc.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxnorm.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxouttext.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxpersistence.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxprecomp.cpp
# ADD CPP /Yc"_cxcore.h"
# End Source File
# Begin Source File

SOURCE=..\src\cxrand.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxsumpixels.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxsvd.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxswitcher.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxtables.cpp
# End Source File
# Begin Source File

SOURCE=..\src\cxutils.cpp
# End Source File
# End Group
# Begin Group "Include"

# PROP Default_Filter ""
# Begin Group "External"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Include\cxcore.h
# End Source File
# Begin Source File

SOURCE=..\include\cxerror.h
# End Source File
# Begin Source File

SOURCE=..\include\cxmisc.h
# End Source File
# Begin Source File

SOURCE=..\include\cxtypes.h
# End Source File
# End Group
# Begin Group "Internal"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\_cxcore.h
# End Source File
# Begin Source File

SOURCE=..\src\_cxipp.h
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=..\src\_cxexport.def
# End Source File
# End Target
# End Project
