# Microsoft Developer Studio Project File - Name="TrafficLight" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=TrafficLight - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TrafficLight.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TrafficLight.mak" CFG="TrafficLight - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TrafficLight - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "TrafficLight - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TrafficLight - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /I "..\include" /I "..\..\c_pgmtk\include" /I "..\..\c_pgmtk\src\include\\" /I "..\..\cxcore\cxcore\include\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /FR /YX /FD /O3 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x419 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 glaux.lib opengl32.lib glu32.lib pnl.lib pnlHigh.lib cxcore.lib /nologo /subsystem:windows /machine:I386 /libpath:"..\..\lib" /libpath:"..\..\high\bin" /libpath:"..\..\cxcore\lib"
# SUBTRACT LINK32 /incremental:yes

!ELSEIF  "$(CFG)" == "TrafficLight - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\include" /I "..\..\c_pgmtk\include" /I "..\..\c_pgmtk\src\include\\" /I "..\..\cxcore\cxcore\include\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x419 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 glaux.lib opengl32.lib glu32.lib pnld.lib pnlHighd.lib cxcored.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\lib" /libpath:"..\..\high\bin" /libpath:"..\..\cxcore\lib"

!ENDIF 

# Begin Target

# Name "TrafficLight - Win32 Release"
# Name "TrafficLight - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Car.cpp
# End Source File
# Begin Source File

SOURCE=.\CarStreamDisctrib.cpp
# End Source File
# Begin Source File

SOURCE=.\Field.cpp
# End Source File
# Begin Source File

SOURCE=.\Human.cpp
# End Source File
# Begin Source File

SOURCE=.\InfoPanel.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MovingObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Nature.cpp
# End Source File
# Begin Source File

SOURCE=.\OptimalTrafficLight.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\TrafficLight.cpp
# End Source File
# Begin Source File

SOURCE=.\TrafficLight.rc
# End Source File
# Begin Source File

SOURCE=.\TrafficLightDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\TrafficLightSimple.cpp
# End Source File
# Begin Source File

SOURCE=.\TrafficLightView.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Car.h
# End Source File
# Begin Source File

SOURCE=.\CarStreamDisctrib.hpp
# End Source File
# Begin Source File

SOURCE=.\Field.h
# End Source File
# Begin Source File

SOURCE=.\Human.h
# End Source File
# Begin Source File

SOURCE=.\InfoPanel.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MovingObject.h
# End Source File
# Begin Source File

SOURCE=.\Nature.h
# End Source File
# Begin Source File

SOURCE=.\OptimalTrafficLight.hpp
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SimpleTypes.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TrafficLight.h
# End Source File
# Begin Source File

SOURCE=.\TrafficLightDoc.h
# End Source File
# Begin Source File

SOURCE=.\TrafficLightSimple.hpp
# End Source File
# Begin Source File

SOURCE=.\TrafficLightView.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\formula.bmp
# End Source File
# Begin Source File

SOURCE=.\res\network.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Road.bmp
# End Source File
# Begin Source File

SOURCE=.\res\TLight.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\TrafficLight.ico
# End Source File
# Begin Source File

SOURCE=.\res\TrafficLight.rc2
# End Source File
# Begin Source File

SOURCE=.\res\TrafficLightDoc.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
