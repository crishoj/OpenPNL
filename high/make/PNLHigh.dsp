# Microsoft Developer Studio Project File - Name="PNLHigh" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PNLHigh - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PNLHigh.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PNLHigh.mak" CFG="PNLHigh - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PNLHigh - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PNLHigh - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PNLHigh - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\..\temp\pnlhigh\release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "HIGHLEVELAPIFORPNL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W4 /GR /GX /Zi /O2 /I "..\include" /I "..\..\c_pgmtk\include" /I "..\..\c_pgmtk\src\include" /I "..\..\cxcore\cxcore\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PNLHIGH_EXPORTS" /FR /FD /c
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib pnl.lib cxcore.lib /nologo /dll /debug /machine:I386 /libpath:"..\..\lib" /libpath:"..\..\cxcore\lib"
# SUBTRACT LINK32 /incremental:yes

!ELSEIF  "$(CFG)" == "PNLHigh - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\..\temp\pnlhigh\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "HIGHLEVELAPIFORPNL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /w /W0 /GR /GX /Zi /Od /I "..\include" /I "..\..\c_pgmtk\include" /I "..\..\c_pgmtk\src\include" /I "..\..\cxcore\cxcore\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PNLHIGH_EXPORTS" /FR /FD /GZ /c
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib pnld.lib cxcored.lib /nologo /dll /incremental:no /debug /machine:I386 /out:"..\bin\pnlhighd.dll" /pdbtype:sept /libpath:"..\..\lib" /libpath:"..\..\cxcore\lib"

!ENDIF 

# Begin Target

# Name "PNLHigh - Win32 Release"
# Name "PNLHigh - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\source\BNet.cpp
# End Source File
# Begin Source File

SOURCE=..\source\BNetCB.cpp
# End Source File
# Begin Source File

SOURCE=..\source\DBN.cpp
# End Source File
# Begin Source File

SOURCE=..\source\DBNCB.cpp
# End Source File
# Begin Source File

SOURCE=..\source\LIMID.cpp
# End Source File
# Begin Source File

SOURCE=..\source\LIMIDCB.cpp
# End Source File
# Begin Source File

SOURCE=..\source\ModelEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\source\MRF.cpp
# End Source File
# Begin Source File

SOURCE=..\source\MRFCB.cpp
# End Source File
# Begin Source File

SOURCE=..\source\NetCallBack.cpp
# End Source File
# Begin Source File

SOURCE=..\source\PersistNodeDiagInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\source\PersistProbNet.cpp
# End Source File
# Begin Source File

SOURCE=..\source\PersistWNodeInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\source\PersistWTypes.cpp
# End Source File
# Begin Source File

SOURCE=..\source\pnlTok.cpp
# End Source File
# Begin Source File

SOURCE=..\source\pnlWDistributions.cpp
# End Source File
# Begin Source File

SOURCE=..\source\pnlWEvidence.cpp
# End Source File
# Begin Source File

SOURCE=..\source\pnlWGraph.cpp
# End Source File
# Begin Source File

SOURCE=..\source\pnlWProbabilisticNet.cpp
# End Source File
# Begin Source File

SOURCE=..\source\TokenCover.cpp
# End Source File
# Begin Source File

SOURCE=..\source\Tokens.cpp
# End Source File
# Begin Source File

SOURCE=..\source\WCliques.cpp
# End Source File
# Begin Source File

SOURCE=..\source\WCover.cpp
# End Source File
# Begin Source File

SOURCE=..\source\Wcsv.cpp
# End Source File
# Begin Source File

SOURCE=..\source\WDistribFun.cpp
# End Source File
# Begin Source File

SOURCE=..\source\WInner.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\BNet.hpp
# End Source File
# Begin Source File

SOURCE=..\include\BNetCB.hpp
# End Source File
# Begin Source File

SOURCE=..\include\DBN.hpp
# End Source File
# Begin Source File

SOURCE=..\include\DBNCB.hpp
# End Source File
# Begin Source File

SOURCE=..\include\LIMID.hpp
# End Source File
# Begin Source File

SOURCE=..\include\LIMIDCB.hpp
# End Source File
# Begin Source File

SOURCE=..\include\ModelEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\MRF.hpp
# End Source File
# Begin Source File

SOURCE=..\include\MRFCB.hpp
# End Source File
# Begin Source File

SOURCE=..\include\NetCallBack.hpp
# End Source File
# Begin Source File

SOURCE=..\include\PersistNodeDiagInfo.hpp
# End Source File
# Begin Source File

SOURCE=..\include\PersistProbNet.hpp
# End Source File
# Begin Source File

SOURCE=..\include\PersistWNodeInfo.hpp
# End Source File
# Begin Source File

SOURCE=..\include\PersistWTypes.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlHigh.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlHighConf.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlTok.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlWDistributions.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlWEvidence.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlWGraph.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlWProbabilisticNet.hpp
# End Source File
# Begin Source File

SOURCE=..\include\TokenCover.hpp
# End Source File
# Begin Source File

SOURCE=..\include\Tokens.hpp
# End Source File
# Begin Source File

SOURCE=..\include\WCliques.hpp
# End Source File
# Begin Source File

SOURCE=..\include\WCover.hpp
# End Source File
# Begin Source File

SOURCE=..\include\Wcsv.hpp
# End Source File
# Begin Source File

SOURCE=..\include\WDistribFun.hpp
# End Source File
# Begin Source File

SOURCE=..\include\WInner.hpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=..\tokenology.txt
# End Source File
# End Target
# End Project
