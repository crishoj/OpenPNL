# Microsoft Developer Studio Project File - Name="pnl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=pnl - Win32 Debug Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pnl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pnl.mak" CFG="pnl - Win32 Debug Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pnl - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE "pnl - Win32 Release Static" (based on "Win32 (x86) Static Library")
!MESSAGE "pnl - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pnl - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pnl - Win32 Parallel Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pnl - Win32 Parallel Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "pnl - Win32 Debug Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "pnl___Win32_Debug_Static"
# PROP BASE Intermediate_Dir "pnl___Win32_Debug_Static"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\temp\pnl\DebugStatic"
# PROP Intermediate_Dir "..\..\temp\pnl\DebugStatic"
# PROP Target_Dir ""
F90=df.exe
LINK32=link.exe
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\src\include" /I "..\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W4 /Gm /GR /GX /Zi /Od /I "..\src\include" /I "..\include" /I "..\..\cxcore\cxcore\include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "PNL_STATIC" /D "WIN32" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
RSC=rc.exe
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\Lib\pnld.lib"
# ADD LIB32 /nologo /out:"..\..\lib\pnlds.lib" /nodefaultlib

!ELSEIF  "$(CFG)" == "pnl - Win32 Release Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pnl___Win32_Release_Static"
# PROP BASE Intermediate_Dir "pnl___Win32_Release_Static"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\temp\pnl\ReleaseStatic"
# PROP Intermediate_Dir "..\..\temp\pnl\ReleaseStatic"
# PROP Target_Dir ""
F90=df.exe
LINK32=link.exe
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Od /I "..\src\include" /I "..\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W4 /GR /GX /Zi /O2 /I "..\src\include" /I "..\include" /I "..\..\cxcore\cxcore\include" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "PNL_STATIC" /D "WIN32" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
RSC=rc.exe
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\Lib\pnl.lib"
# ADD LIB32 /nologo /out:"..\..\lib\pnls.lib" /nodefaultlib

!ELSEIF  "$(CFG)" == "pnl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\temp\pnl\Debug"
# PROP Intermediate_Dir "..\..\temp\pnl\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe -lib
CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /GR /GX /Zi /Od /I "..\src\include" /I "..\include" /I "..\..\cxcore\cxcore\include" /I "..\include\cart" /I "..\..\..\cxcore\cxcore\include\\" /D "_DEBUG" /D "_WINDOWS" /D "PNL_EXPORTS" /D "CV_DLL" /D "WIN32" /FR /FD /GZ /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
RSC=rc.exe
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcored.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:I386 /nodefaultlib:"libcmt.lib" /nodefaultlib:"msvcrt.lib" /out:"..\..\bin\pnld.dll" /implib:"..\..\lib\pnld.lib" /pdbtype:sept /libpath:"..\..\lib" /libpath:"..\..\cxcore\lib"
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "pnl - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\temp\pnl\Release"
# PROP Intermediate_Dir "..\..\temp\pnl\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe -lib
CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W4 /GR /GX /Zi /O2 /I "..\src\include" /I "..\include" /I "..\..\cxcore\cxcore\include" /D "NDEBUG" /D "_WINDOWS" /D "PNL_EXPORTS" /D "CV_DLL" /D "WIN32" /FD /c
# SUBTRACT CPP /Fr /YX /Yc /Yu
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
RSC=rc.exe
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcore.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\bin\pnl.dll" /implib:"..\..\lib\pnl.lib" /libpath:"..\..\lib" /libpath:"..\..\cxcore\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "pnl - Win32 Parallel Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "pnl___Win32_Parallel_Debug"
# PROP BASE Intermediate_Dir "pnl___Win32_Parallel_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\temp\pnl\ParallelDebug"
# PROP Intermediate_Dir "..\..\temp\pnl\ParallelDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe -lib
CPP=cl.exe
# ADD BASE CPP /nologo /MDd /W4 /GR /GX /Zi /Od /I "..\src\include" /I "..\include" /I "..\..\cxcore\cxcore\include" /I "..\include\cart" /I "..\..\..\cxcore\cxcore\include\\" /D "_DEBUG" /D "_WINDOWS" /D "PNL_EXPORTS" /D "CV_DLL" /D "WIN32" /FR /FD /GZ /c
# ADD CPP /nologo /MDd /w /W0 /GR /GX /Zi /Od /I "..\src\include" /I "..\include" /I "..\..\cxcore\cxcore\include" /I "..\include\cart" /I "..\..\..\cxcore\cxcore\include\\" /D "_DEBUG" /D "_WINDOWS" /D "PNL_EXPORTS" /D "CV_DLL" /D "WIN32" /D "BUILD_MPI" /D "BUILD_OMP" /FR /FD /GZ /Qopenmp /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
RSC=rc.exe
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcored.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:I386 /nodefaultlib:"libcmt.lib" /nodefaultlib:"msvcrt.lib" /out:"..\..\bin\pnld.dll" /implib:"..\..\lib\pnld.lib" /pdbtype:sept /libpath:"..\lib" /libpath:"..\..\cxcore\lib"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcored.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:I386 /nodefaultlib:"libcmt.lib" /nodefaultlib:"msvcrt.lib" /out:"..\..\bin\pnld.dll" /implib:"..\..\lib\pnld.lib" /pdbtype:sept /libpath:"..\lib" /libpath:"..\..\cxcore\lib"
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "pnl - Win32 Parallel Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pnl___Win32_Parallel_Release"
# PROP BASE Intermediate_Dir "pnl___Win32_Parallel_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\temp\pnl\ParallelRelease"
# PROP Intermediate_Dir "..\..\temp\pnl\ParallelRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe -lib
CPP=cl.exe
# ADD BASE CPP /nologo /MD /W4 /GR /GX /Zi /O2 /I "..\src\include" /I "..\include" /I "..\..\cxcore\cxcore\include" /D "NDEBUG" /D "_WINDOWS" /D "PNL_EXPORTS" /D "CV_DLL" /D "WIN32" /FD /c
# SUBTRACT BASE CPP /Fr /YX /Yc /Yu
# ADD CPP /nologo /MD /w /W0 /GR /GX /Zi /O2 /I "..\src\include" /I "..\include" /I "..\..\cxcore\cxcore\include" /D "NDEBUG" /D "_WINDOWS" /D "PNL_EXPORTS" /D "CV_DLL" /D "WIN32" /D "BUILD_MPI" /D "BUILD_OMP" /FD /Qopenmp /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
RSC=rc.exe
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcore.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\bin\pnl.dll" /implib:"..\..\lib\pnl.lib" /libpath:"..\lib" /libpath:"..\..\cxcore\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcore.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\bin\pnl.dll" /implib:"..\..\lib\pnl.lib" /libpath:"..\lib" /libpath:"..\..\cxcore\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "pnl - Win32 Debug Static"
# Name "pnl - Win32 Release Static"
# Name "pnl - Win32 Debug"
# Name "pnl - Win32 Release"
# Name "pnl - Win32 Parallel Debug"
# Name "pnl - Win32 Parallel Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "persistence"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=..\src\pnlContext.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlContextLoad.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlContextPersistence.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlContextSave.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlGroup.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlObjHandler.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlPersistCover.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlPersistCPD.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlPersistDistribFun.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlPersistence.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlPersistGraph.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlPersistModel.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlPersistModelDomain.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlPersistTypes.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlXMLRead.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlXMLWrite.cpp
# End Source File
# End Group
# Begin Group "matrices"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=..\src\pnl2DBitwiseMatrix.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnl2DNumericDenseMatrix.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlDenseMatrix.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnli2DNumericDenseMatrix.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnliNumericDenseMatrix.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnliNumericSparseMatrix.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlMatrix.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlMatrixIterator.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlNumericDenseMatrix.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlSparseMatrix.cpp
# End Source File
# End Group
# Begin Group "parallel"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=..\src\pnlParConfig.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\src\pnlParEmLearningEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlParGibbsSamplingInferenceEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlParJtreeInferenceEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlParNewDelete.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlParPearlInferenceEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# End Group
# Begin Source File

SOURCE=..\src\cvcart.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\src\CVSVD.C
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\src\dbginfo.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\src\memtrack.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\src\pnl1_5SliceInferenceEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnl1_5SliceJtreeInferenceEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnl2TBNInferenceEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnl2TPFInferenceEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlAllocator.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlBayesLearningEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlBicLearningEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlBKInferenceEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlBNet.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlCondGaussianDistribFun.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlCondSoftMaxDistribFun.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlConfig.cpp
# ADD CPP /Yc"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlCoverage.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlCPD.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlDAG.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlDBN.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlDiagnostics.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlDistribFun.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlDynamicGraphicalModel.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlDynamicInferenceEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlDynamicLearningEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlEmLearningEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlEmLearningEngineDBN.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlEvidence.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlExampleModels.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\SRC\pnlException.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlExInferenceEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlFactor.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlFactorGraph.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlFactors.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlFakePtr.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlFGSumMaxInferenceEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlGaussianCPD.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlGaussianDistribFun.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlGaussianPotential.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlGibbsSamplingInferenceEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlGibbsWithAnnealingInferenceEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlGraph.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlGraphicalModel.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlIDNet.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlIDPotential.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlIDTabularPotential.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlInferenceEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlJtreeInferenceEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlJunctionTree.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlLearningEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlLIMIDInfEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlLog.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\src\pnlLogDriver.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\src\pnlLogMultiplexor.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\src\pnlLogUsing.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\src\pnlLWSamplingInferenceEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlMixtureGaussianCPD.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlMlDynamicStructLearn.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlMlStaticStructLearn.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlMlStaticStructLearnHC.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlMNet.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlModelDomain.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlMRF2.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlNaiveInferenceEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlNodeType.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlNodeValues.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlObject.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlPearlInferenceEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlpnlType.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlPotential.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlReferenceCounter.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlRNG.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlSamplingInferenceEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlScalarDistribFun.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlScalarPotential.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlSoftMaxCPD.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlSoftMaxDistribFun.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlSpecPearlInferenceEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlStaticGraphicalModel.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlStaticLearningEngine.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlStaticStructLearnSEM.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlString.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pnlTabularCPD.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlTabularDistribFun.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlTabularPotential.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlTimer.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlTreeCPD.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# Begin Source File

SOURCE=..\src\pnlTreeDistribFun.cpp
# ADD CPP /Yu"pnlConfig.hpp"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "persistence.headers"

# PROP Default_Filter "hpp"
# Begin Source File

SOURCE=..\include\pnlContext.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlContextLoad.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlContextPersistence.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlContextSave.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlGroup.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlObjHandler.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlPersistArray.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlPersistCover.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlPersistCPD.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlPersistDistribFun.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlPersistence.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlPersistGraph.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlPersistModel.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlPersistModelDomain.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlPersistTypes.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlXMLContainer.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlXMLRead.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlXMLWrite.hpp
# End Source File
# End Group
# Begin Group "matrices.headers"

# PROP Default_Filter "hpp"
# Begin Source File

SOURCE=..\include\pnl2DBitwiseMatrix.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnl2DNumericDenseMatrix.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlDenseMatrix.hpp
# End Source File
# Begin Source File

SOURCE=..\src\include\pnli2DNumericDenseMatrix.hpp
# End Source File
# Begin Source File

SOURCE=..\src\include\pnliNumericDenseMatrix.hpp
# End Source File
# Begin Source File

SOURCE=..\src\include\pnliNumericSparseMatrix.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlMatrix.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlMatrixIterator.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlNumericDenseMatrix.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlNumericMatrix.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlNumericSparseMatrix.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlSparseMatrix.hpp
# End Source File
# End Group
# Begin Group "parallel.headers"

# PROP Default_Filter "hpp;h"
# Begin Source File

SOURCE=..\include\ParPNLCRTDBG.H
# End Source File
# Begin Source File

SOURCE=..\include\pnlParConfig.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlParEmLearningEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlParGibbsSamplingInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlParJtreeInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlParNewDelete.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlParPearlInferenceEngine.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\src\include\cart\cvcart.h
# End Source File
# Begin Source File

SOURCE=..\src\include\CVSVD.H
# End Source File
# Begin Source File

SOURCE=..\src\include\cart\datadefs.h
# End Source File
# Begin Source File

SOURCE=..\include\dbginfo.h
# End Source File
# Begin Source File

SOURCE=..\src\include\cart\inlines.h
# End Source File
# Begin Source File

SOURCE=..\include\memtrack.h
# End Source File
# Begin Source File

SOURCE=..\include\pnl1_5SliceInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnl1_5SliceJtreeInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnl2TBNInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnl2TPFInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnl_dll.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlAllocator.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlBayesLearningEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlBicLearningEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlBKInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlBNet.hpp
# End Source File
# Begin Source File

SOURCE=..\src\include\pnlcondgaussiandistribfun.hpp
# End Source File
# Begin Source File

SOURCE=..\src\include\pnlCondSoftMaxDistribFun.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlConfig.h
# End Source File
# Begin Source File

SOURCE=..\include\pnlConfig.hpp
# End Source File
# Begin Source File

SOURCE=..\src\include\pnlCoverage.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlCPD.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlDAG.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlDBN.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlDiagnostics.hpp
# End Source File
# Begin Source File

SOURCE=..\src\include\pnlDistribFun.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlDynamicGraphicalModel.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlDynamicInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlDynamicLearningEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlEmLearningEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlEmLearningEngineDBN.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlEvidence.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlExampleModels.hpp
# End Source File
# Begin Source File

SOURCE=..\INCLUDE\pnlException.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlExInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlFactor.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlFactorGraph.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlFactors.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlFakePtr.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlFGSumMaxInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlGaussianCPD.hpp
# End Source File
# Begin Source File

SOURCE=..\src\include\pnlGaussianDistribFun.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlGaussianPotential.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlGibbsSamplingInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlGibbsWithAnnealingInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlGraph.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlGraphicalModel.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlIDNet.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlIDPotential.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlIDTabularPotential.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlJtreeInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlJunctionTree.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlLearningEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlLIMIDInfEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlLog.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlLogDriver.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlLogMultiplexor.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlLogUsing.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlLWSamplingInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlMixtureGaussianCPD.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlMlDynamicStructLearn.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlMlStaticStructLearn.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlMlStaticStructLearnHC.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlMNet.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlModelDomain.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlModelTypes.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlMRF2.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlNaiveInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlNodeType.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlNodeValues.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlObject.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlPearlInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlpnlType.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlPotential.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlReferenceCounter.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlRing.hpp
# End Source File
# Begin Source File

SOURCE=..\src\include\pnlRNG.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlSamplingInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\src\include\pnlScalarDistribFun.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlScalarPotential.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlSoftMaxCPD.hpp
# End Source File
# Begin Source File

SOURCE=..\src\include\pnlSoftMaxDistribFun.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlSpecPearlInferenceEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlStaticGraphicalModel.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlStaticLearningEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlStaticStructLearnSEM.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlString.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlTabularCPD.hpp
# End Source File
# Begin Source File

SOURCE=..\src\include\pnlTabularDistribFun.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlTabularPotential.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlTimer.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlTreeCPD.hpp
# End Source File
# Begin Source File

SOURCE=..\src\include\pnlTreeDistribFun.hpp
# End Source File
# Begin Source File

SOURCE=..\include\pnlTypeDefs.hpp
# End Source File
# End Group
# End Target
# End Project
