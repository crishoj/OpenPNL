# Microsoft Developer Studio Project File - Name="cxcorelib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=cxcorelib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cxcorelib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cxcorelib.mak" CFG="cxcorelib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cxcorelib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "cxcorelib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "cxcorelib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\_temp\cxcorelib_Rls"
# PROP Intermediate_Dir "..\..\_temp\cxcorelib_Rls"
# PROP Target_Dir ""
LINK32=link.exe
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W4 /GX /Zi /O2 /I "..\src" /I "..\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"_cxcore.h" /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\cxcorelib.lib"

!ELSEIF  "$(CFG)" == "cxcorelib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\_temp\cxcorelib_Dbg"
# PROP Intermediate_Dir "..\..\_temp\cxcorelib_Dbg"
# PROP Target_Dir ""
LINK32=link.exe
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /Zi /Od /I "..\src" /I "..\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\cxcorelibd.lib"

!ENDIF 

# Begin Target

# Name "cxcorelib - Win32 Release"
# Name "cxcorelib - Win32 Debug"
# Begin Group "Src"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
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

!IF  "$(CFG)" == "cxcorelib - Win32 Release"

# ADD CPP /Yc"_cxcore.h"

!ELSEIF  "$(CFG)" == "cxcorelib - Win32 Debug"

!ENDIF 

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

# PROP Default_Filter "h;hpp;hxx;hm;inl"
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
# End Target
# End Project
