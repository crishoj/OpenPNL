# Microsoft Developer Studio Project File - Name="tests_pnl_c" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=tests_pnl_c - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tests_pnl_c.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tests_pnl_c.mak" CFG="tests_pnl_c - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tests_pnl_c - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "tests_pnl_c - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tests_pnl_c - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\temp\tests_pnl_c\Release"
# PROP Intermediate_Dir "..\..\..\temp\tests_pnl_c\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W4 /GR /GX /Zi /O2 /I "..\include" /I "..\..\include" /I "..\..\..\trs\include" /I "..\..\src\include" /I "..\smile" /I "..\..\..\cxcore\cxcore\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcore.lib /nologo /subsystem:console /debug /machine:I386 /out:"..\..\..\bin\tests_pnl_c.exe" /libpath:"..\..\..\lib" /libpath:"..\..\..\cxcore\lib" /fixed:no"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build
TargetPath=\work\pnl.sf\PNL\bin\tests_pnl_c.exe
TargetName=tests_pnl_c
InputPath=\work\pnl.sf\PNL\bin\tests_pnl_c.exe
SOURCE="$(InputPath)"

"..\..\..\bin\$(TargetName).exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(TargetPath) ..\..\..\bin

# End Custom Build

!ELSEIF  "$(CFG)" == "tests_pnl_c - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\temp\tests_pnl_c\Debug"
# PROP Intermediate_Dir "..\..\..\temp\tests_pnl_c\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "..\include" /I "..\..\include" /I "..\..\..\trs\include" /I "..\..\src\include" /I "..\smile" /I "..\..\..\cxcore\cxcore\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcored.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /out:"..\..\..\bin\tests_pnl_cd.exe" /pdbtype:sept /libpath:"..\..\..\lib" /libpath:"..\..\..\cxcore\lib"
# Begin Custom Build
TargetPath=\work\pnl.sf\PNL\bin\tests_pnl_cd.exe
TargetName=tests_pnl_cd
InputPath=\work\pnl.sf\PNL\bin\tests_pnl_cd.exe
SOURCE="$(InputPath)"

"..\..\..\bin\$(TargetName).exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(TargetPath) ..\..\..\bin

# End Custom Build

!ENDIF 

# Begin Target

# Name "tests_pnl_c - Win32 Release"
# Name "tests_pnl_c - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\A1_5JTreeInfDBNCondGauss.cpp
# End Source File
# Begin Source File

SOURCE=..\src\A2TPFInfDBN.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AAnnealingGibbsInfEng.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ABayesParameterLearning.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ABKInfDBN.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ABKInfUsingClusters.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AComputeLikelihood.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ACondGaussian.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AConditionalGaussianLearning.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ACondSoftMaxAndSoftMax.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ADetermineDistrType.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ADSeparation.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AExInfEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AExLearnEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AFactorGraph.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AGaussianFactorOperations.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AGaussianInference.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AGaussianMRF2.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AGaussianParameterLearning.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AGeneralInference.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AGenerateEvidences.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AGetParametersTest.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AGibbsInfEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AGraph.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AInfForRndBNet.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AJTreeInfDBN.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AJtreeInfMixtureDBN.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ALearningCondGaussDBN.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ALIMIDInfEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ALog.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ALWInfEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AMarginalize.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AMarginalizeCompareMATLAB.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\src\AMixtureGaussainLearning.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AMixtureGaussainModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AMRF2PearlInf.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AMultiDMatrix.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AMultiDMatrix_exp.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AMultiply.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AMultiplyVolume.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ANaiveInfEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ANormalizeCPD.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ANormalizeFactor.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AParameterLearning.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AParameterLearningDBN.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AParameterTakeObsInto.cpp
# End Source File
# Begin Source File

SOURCE=..\src\APearlGibbsCompareMRF2.cpp
# End Source File
# Begin Source File

SOURCE=..\src\APearlInfEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\src\APearlInfEngineMRF2.cpp
# End Source File
# Begin Source File

SOURCE=..\src\APerformanceJTreeInf.cpp
# End Source File
# Begin Source File

SOURCE=..\src\APersistence.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ARandomFactors.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AReduceDims.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AReduceOp.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AReferenceCounter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ARockPaperScissors.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ASaveLoadEvidences.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ASetStatistics.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ASparseMatrixInfernceBNet.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ASparseReduceOp.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AStackAllocator.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AStructuralLearningWithJTreeInference.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AStructureLearning.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ATreeCPDLearn.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AUnrollDBN.cpp
# End Source File
# Begin Source File

SOURCE=..\src\SEEvidence.cpp
# End Source File
# Begin Source File

SOURCE=..\src\SEGaussian.cpp
# End Source File
# Begin Source File

SOURCE=..\src\SEParameter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TBKInference.cpp
# End Source File
# Begin Source File

SOURCE=..\src\tCreateArHMMwithGaussObs.cpp
# End Source File
# Begin Source File

SOURCE=..\src\tCreateIncineratorBNet.cpp
# End Source File
# Begin Source File

SOURCE=..\src\tCreateKjaerulffDBN.cpp
# End Source File
# Begin Source File

SOURCE=..\src\tCreateRandStructs.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TJTreeInfEngTiming.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TPearlInfEngTiming.cpp
# End Source File
# Begin Source File

SOURCE=..\src\tsys.cpp
# End Source File
# Begin Source File

SOURCE=..\src\tUtil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\AGraph.hpp
# End Source File
# Begin Source File

SOURCE=..\include\AMlLearningEngine.hpp
# End Source File
# Begin Source File

SOURCE=..\include\AReferenceCounter.h
# End Source File
# Begin Source File

SOURCE=..\include\tConfig.h
# End Source File
# Begin Source File

SOURCE=..\include\tCreateIncineratorBNet.hpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
