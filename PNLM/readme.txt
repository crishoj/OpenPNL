/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//     File:      PGMT\ifMatLab\readme.txt                                 //
//                                                                         //
//  Changes:      9-Sep-03 jma                                             //
/////////////////////////////////////////////////////////////////////////////


MatLab interface to PNL.

This directory contains current version of PNL's MatLab interface. Each PNL 
function can be called from Matlab using these wrapper functions that are
generated automatically from PNL code. This interface has been tested with 
Matlab version 6, Release 12.1 and Matlab 6.5, release 13.

The scripts subdirectory contains tools to re-compile this interface. You
don't need to do this to install the Matlab interface. See scripts\readme.txt.


TO INSTALL
----------

In addition to the wrapper files in this directory, MatLab calls pnl_full.dll. 
Wrapper files (.m files), method directories (prefaced by @) and pnl_full.dll 
must be in the MatLab command path, so it makes sense to place them together 
in this directory.  Although its not called directly, the interface also depends 
on pnl.dll and cxcore.dll -- if its not found by the system you'll get this
error:

        Unable to load mex file: d:\pnl\pnlm\pnl_full.dll.
        The specified module could not be found.

        ??? Invalid MEX-file

To keep the distribution size manageable, a compiled version of pnl.dll may 
not be included in it. 


1) If pnl.dll doesn't exist, compile it (see below) and place it in your
pnl\bin directory (the same place as cxcore.dll). 

2) Set your system path variable to include your pnl\bin directory:
for Windows 2000:
     Go to System control panel, 
     Choose the "Advanced" tab, 
     Select the "Environment Variables" button
     Choose "Edit to add this directoy to the "Path" variable.

3) In MatLab use the PathTool (using menu 'File/Set Path...') to add your
pnl\pnlm directory to the Matlab command path. Choose "save" then
"close" to make this permanent. 



TO COMPILE pnl.dll
------------------


1) In MS Visual Studio open workspace PNL\c_pgmtk\make\pnl.dsw

2) Set "pnl" as the active project (Either go to the project menu, or right click 
on the pnl item in the Class View list)

3) Check under "active configuration"; It should be set to pnl-Win32 Release.

4) Compile with the menu command Build => Build pnl.dll (function F7). Relax - this
takes awhile! 

5) The result should show up in PNL\bin\pnl.dll

