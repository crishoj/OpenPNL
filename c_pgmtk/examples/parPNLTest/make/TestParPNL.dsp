# Microsoft Developer Studio Project File - Name="TestParPNL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=TestParPNL - Win32 Parallel Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TestParPNL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TestParPNL.mak" CFG="TestParPNL - Win32 Parallel Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TestParPNL - Win32 Parallel Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "TestParPNL - Win32 Parallel Release" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TestParPNL - Win32 Parallel Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TestParPNL___Win32_Parallel_Debug"
# PROP BASE Intermediate_Dir "TestParPNL___Win32_Parallel_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\temp\TestParPNL\ParallelDebug/"
# PROP Intermediate_Dir "..\..\..\..\temp\TestParPNL\ParallelDebug/"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\include" /I "..\..\include" /I "..\..\src\include" /I "..\..\..\cxcore\cxcore\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MDd /w /W0 /Gm /GR /GX /Zi /Od /I "..\include" /I "..\..\..\include" /I "..\..\..\src\include" /I "..\..\..\..\cxcore\cxcore\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "BUILD_MPI" /D "BUILD_OMP" /YX /FD /GZ /Qopenmp /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcored.lib pnld.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /out:"..\..\..\bin\TestParPNLd.exe" /pdbtype:sept /libpath:"..\..\..\lib" /libpath:"..\..\..\cxcore\lib"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcored.lib pnld.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /out:"..\..\..\..\bin\TestParPNLd.exe" /pdbtype:sept /libpath:"..\..\..\..\lib" /libpath:"..\..\..\..\cxcore\lib"

!ELSEIF  "$(CFG)" == "TestParPNL - Win32 Parallel Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TestParPNL___Win32_Parallel_Release"
# PROP BASE Intermediate_Dir "TestParPNL___Win32_Parallel_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\temp\TestParPNL\ParallelRelease/"
# PROP Intermediate_Dir "..\..\..\..\temp\TestParPNL\ParallelRelease/"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\include" /I "..\..\include" /I "..\..\src\include" /I "..\..\..\cxcore\cxcore\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MD /w /W0 /GR /GX /O2 /I "..\include" /I "..\..\..\include" /I "..\..\..\src\include" /I "..\..\..\..\cxcore\cxcore\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "BUILD_MPI" /D "BUILD_OMP" /YX /FD /Qopenmp /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcore.lib pnl.lib /nologo /subsystem:console /machine:I386 /out:"..\..\..\bin\TestParPNL.exe" /libpath:"..\..\..\lib" /libpath:"..\..\..\cxcore\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcore.lib pnl.lib /nologo /subsystem:console /machine:I386 /out:"..\..\..\..\bin\TestParPNL.exe" /libpath:"..\..\..\..\lib" /libpath:"..\..\..\..\cxcore\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "TestParPNL - Win32 Parallel Debug"
# Name "TestParPNL - Win32 Parallel Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\CreateBNets.cpp
# End Source File
# Begin Source File

SOURCE=..\src\LoadBNet.cpp
# End Source File
# Begin Source File

SOURCE=..\src\parPNLTest.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\CreateBNets.h
# End Source File
# Begin Source File

SOURCE=..\include\LoadBNet.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
