/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/


#if !defined (_TRSERROR_H)
# define _TRSERROR_H                    /* Prevent multiply SELF include     */

/*---------------------------------------------------------------------------*/
/*  This section describes the error handling framework.                     */
/*                                                                           */

#define TRS_BASE               -100

#define TRS_StsOk                 0     /* everithing is ok                  */

#define TRS_StsBackTrace   (TRS_BASE-1) /* pseudo error for back trace       */
#define TRS_StsError       (TRS_BASE-2) /* unknown /unspecified error        */
#define TRS_StsInternal    (TRS_BASE-3) /* internal error (bad state)        */
#define TRS_StsNoMem       (TRS_BASE-4) /* out of memory                     */
#define TRS_StsBadArg      (TRS_BASE-5) /* function arg/param is bad         */
#define TRS_StsBadFunc     (TRS_BASE-6) /* unsupported function              */
#define TRS_StsAutoTrace   (TRS_BASE-8) /* Tracing through trstrace.h        */

#define TRS_StsFOpen       (TRS_BASE-10)/* No such file                      */
#define TRS_StsFCreate     (TRS_BASE-11)/* Unable to create file             */
#define TRS_StsNoEntry     (TRS_BASE-12)/* No keyname in section key=        */
#define TRS_StsNoSection   (TRS_BASE-13)/* No section [...] in file          */
#define TRS_StsBadFormat   (TRS_BASE-14)/* Key value has illegal format      */
#define TRS_StsSyntaxError (TRS_BASE-15)/* Syntax error                      */
#define TRS_StsBadMem      (TRS_BASE-16)/* Corrupted memory guards           */

typedef int                 TRSStatus;

TRSREF(TRSStatus)           TstLastStatus;

#define TRS_ErrModeLeaf           0     /* Print error and exit program      */
#define TRS_ErrModeParent         1     /* Print error and continue          */
#define TRS_ErrModeSilent         2     /* Don't print and continue          */

TRSREF(int)                 TstStdErrMode;

/*------ returns function name after the last libError call -----------------*/

TRSAPI(int, newStdError,(int status, const char *funcname, const char *context,
                           const char *file, int line));
TRSAPI(char*, trsErrFuncName,(void));

TRSAPI(TRSStatus, trsError,(TRSStatus status, const char *func,
                   const char *context, const char *file,int line));

/*==== Original Macros : Unconditional - TRS_DEBUG independant ==============*/

#define TRS_FILE __FILE__

#define TRS_ERROR(status,func,context)\
                trsError((status),(func),(context),TRS_FILE,__LINE__)

#define TRS_ERRCHK(func,context)\
                ((TstLastStatus >= 0) ? TRS_StsOk \
                : TRS_ERROR(TRS_StsBackTrace,(func),(context)))

#define TRS_ASSERT(expr,func,context)\
                ((expr) ? TRS_StsOk \
                : TRS_ERROR(TRS_StsInternal,(func),(context)))

#define TRS_RSTERR() (TstLastStatus = TRS_StsOk)

#endif                                  /* _TRSERROR_H                        */
