# Microsoft Developer Studio Project File - Name="PNLGeNIe" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PNLGeNIe - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pnlGeNIe.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pnlGeNIe.mak" CFG="PNLGeNIe - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PNLGeNIe - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PNLGeNIe - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PNLGeNIe - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\bin"
# PROP Intermediate_Dir "..\..\..\temp\pnlgenie\release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PNLGeNIe_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W4 /GX /Zi /O2 /I "..\..\GeNIe" /I "..\..\..\c_pgmtk\include" /I "..\..\..\c_pgmtk\src\include" /I "..\..\..\cxcore\cxcore\include" /I "..\..\include" /I "..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PNLGeNIe_EXPORTS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib pnl.lib cxcore.lib /nologo /dll /machine:I386 /out:"..\..\bin/engine1.dll" /libpath:"..\..\..\lib" /libpath:"..\..\..\cxcore\lib"

!ELSEIF  "$(CFG)" == "PNLGeNIe - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\bin"
# PROP Intermediate_Dir "..\..\..\temp\pnlgenie\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PNLGeNIe_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /w /W0 /GR /GX /Zi /Od /I "..\..\GeNIe" /I "..\..\..\c_pgmtk\include" /I "..\..\..\c_pgmtk\src\include" /I "..\..\..\cxcore\cxcore\include" /I "..\..\include" /I "..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PNLGeNIe_EXPORTS" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib pnld.lib /nologo /dll /incremental:no /debug /machine:I386 /out:"..\..\bin\engine1.dll" /pdbtype:sept /libpath:"..\..\..\lib" /libpath:"..\..\..\cxcore\lib"

!ENDIF 

# Begin Target

# Name "PNLGeNIe - Win32 Release"
# Name "PNLGeNIe - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\source\diagnetworkPNLH.cpp
# End Source File
# Begin Source File

SOURCE=..\source\networkPNLH.cpp
# End Source File
# Begin Source File

SOURCE=..\source\pnlGeNIeDLL.cpp
# End Source File
# Begin Source File

SOURCE=..\source\XMLBindingPNLH.cpp
# End Source File
# Begin Source File

SOURCE=..\source\XMLReaderPNLH.cpp
# End Source File
# Begin Source File

SOURCE=..\source\XMLWriterPNLH.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\diagnetworkPNLH.hpp
# End Source File
# Begin Source File

SOURCE=..\include\networkPNLH.hpp
# End Source File
# Begin Source File

SOURCE=..\include\typesPNLH.hpp
# End Source File
# Begin Source File

SOURCE=..\include\XMLBindingPNLH.hpp
# End Source File
# Begin Source File

SOURCE=..\include\XMLContainer.hpp
# End Source File
# Begin Source File

SOURCE=..\include\XMLReaderPNLH.hpp
# End Source File
# Begin Source File

SOURCE=..\include\XMLWriterPNLH.hpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
