# Microsoft Developer Studio Project File - Name="tests" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=tests - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tests.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tests.mak" CFG="tests - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tests - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "tests - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tests - Win32 Release"

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
# ADD CPP /nologo /MD /W4 /GR /GX /Zi /O2 /I "..\include" /I "..\..\c_pgmtk\include" /I "..\..\c_pgmtk\src\include" /I "..\..\cxcore\cxcore\include" /I "..\..\trs\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 pnlhigh.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcore.lib pnl.lib /nologo /subsystem:console /debug /machine:I386 /out:"..\bin\toktest.exe" /libpath:"..\bin\\" /libpath:"..\..\cxcore\lib" /libpath:"..\..\lib"

!ELSEIF  "$(CFG)" == "tests - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\..\temp\pnlhigh_test\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /w /W0 /Gm /GR /GX /Zi /Od /I "..\include" /I "..\..\c_pgmtk\include" /I "..\..\c_pgmtk\src\include" /I "..\..\cxcore\cxcore\include" /I "..\..\trs\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "TEST" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 pnlhighd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcore.lib pnld.lib /nologo /subsystem:console /debug /machine:I386 /out:"..\bin\toktestd.exe" /pdbtype:sept /libpath:"..\..\lib" /libpath:"..\bin\\" /libpath:"..\..\cxcore\lib"

!ENDIF 

# Begin Target

# Name "tests - Win32 Release"
# Name "tests - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Test.cpp
# End Source File
# Begin Source File

SOURCE=.\TestCommon.cpp
# End Source File
# Begin Source File

SOURCE=.\TestCondGaussian.cpp
# End Source File
# Begin Source File

SOURCE=.\TestGaussianBNet.cpp
# End Source File
# Begin Source File

SOURCE=.\TestLimid.cpp
# End Source File
# Begin Source File

SOURCE=.\TestMRF.cpp
# End Source File
# Begin Source File

SOURCE=.\TestNumeration.cpp
# End Source File
# Begin Source File

SOURCE=.\TestsDBN.cpp
# End Source File
# Begin Source File

SOURCE=.\TestSoftMax.cpp
# End Source File
# Begin Source File

SOURCE=..\..\trs\src\trsapi.c
# End Source File
# Begin Source File

SOURCE=..\..\trs\src\trserror.c
# End Source File
# Begin Source File

SOURCE=..\..\trs\src\trsread.c
# End Source File
# Begin Source File

SOURCE=.\tsys.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\test_conf.hpp
# End Source File
# Begin Source File

SOURCE=.\TestPnlHigh.hpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
