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

/*

#if !defined (TRS_BUILD)

# define TRS_BUILD

#endif

*/

#include "trsapi.h"

/*#include "trserror.h"*/

/* --------------------------------------------------------------------------*/

#if defined putchar

# undef putchar

#endif



#define putchar _putchar

#define printf  _printf

#define fprintf _fprintf

#define gets    _gets

/* --------------------------------------------------------------------------*/



#include <stdlib.h>



/*------ returns function name after the last libError call -----------------*/



static char LastFuncName[40]="";



TRSFUN(char*, trsErrFuncName,(void)){return (*LastFuncName) ? LastFuncName : 0;}



/*==== The trsStdError implementation =======================================*/

/*                                                                           */

#ifdef ANYLIB

TRSFUN(IPLStatus, newStdError,(IPLStatus   status, const char *funcname,

                         const char *context,

                         const char *file, int line)) {



  StringnCpy(LastFuncName,funcname,sizeof(LastFuncName));



  if (iplGetErrMode()!=IPL_ErrModeSilent) {



      if ((status==IPL_StsBackTrace) || (status==IPL_StsAutoTrace))

        fprintf(stderr, "\tcalled from");

      else {

        fprintf(stderr, "IPL Error: %s in", iplErrorStr(status));

      };



      if (status!=IPL_StsAutoTrace)

        fprintf(stderr," function %s", funcname ? funcname: "<unknown>");



      if (line>0 || file != NULL)

        fprintf(stderr," [%s:%d]", file, line);



      if (status==IPL_StsAutoTrace)

          fprintf(stderr,":%s", funcname ? funcname: "<unknown>");



      if (context != NULL)

      {

        if (status!=IPL_StsAutoTrace)

          fprintf(stderr, ": %s", context);                /* Print context      */

        else

          fprintf(stderr, "(%s)", context);                /* Print arguments    */

      };

      fprintf(stderr, "\n");

      trs_FlushAll();

      if (iplGetErrMode()==IPL_ErrModeLeaf) {

         fprintf(stderr, "IPL: terminating the application\n");

         trs_FlushAll(); 

         exit(1);

      };

   }

   return status;

}

#endif



/*==== The trsErrrorStr implementation ======================================*/

/*                                                                           */



TRSVAR(TRSStatus) TstLastStatus = TRS_StsOk;

TRSVAR(int      ) TstStdErrMode = TRS_ErrModeParent;



TRSFUN(const char*, trsErrorStr,(TRSStatus status)) {

  static char buf[80];



  switch (status) {

    case TRS_StsOk:             return "No Error";

    case TRS_StsBackTrace:      return "Backtrace";

    case TRS_StsError:          return "Unknown error";

    case TRS_StsInternal:       return "Internal error";

    case TRS_StsNoMem:          return "Out of memory";

    case TRS_StsBadArg:         return "Bad argument";

    case TRS_StsBadFunc:        return "Unsupported function";

    case TRS_StsAutoTrace:      return "Autotrace call";

    case TRS_StsSyntaxError:    return "Syntax error";

    case TRS_StsFOpen:          return "No such file";

    case TRS_StsFCreate:        return "Unable to create file";

    case TRS_StsNoEntry:        return "No keyname in section key=";

    case TRS_StsNoSection:      return "No section [...] in file";

    case TRS_StsBadFormat:      return "Key value has illegal format";

    case TRS_StsBadMem:         return "Corrupted memory";

  };

  sprintf(buf, "Unknown %s code %d", status >= 0 ? "status":"error",status);

  return buf;

}



/*==== The trsStdError implementation =======================================*/

/*                                                                           */



TRSFUN(TRSStatus, trsStdError,(TRSStatus status, const char *funcname, const char *context,

                const char *file, int line)) {



  if (TstStdErrMode==TRS_ErrModeSilent)

     return (status!=TRS_StsBackTrace) ? (TstLastStatus=status) : status;



  if ((status==TRS_StsBackTrace) || (status==TRS_StsAutoTrace))

    fprintf(stderr, "\tcalled from");

  else {

    TstLastStatus = status;

    fprintf(stderr, "TRS Error: %s in", trsErrorStr(status));

  };



  if (status!=TRS_StsAutoTrace)

    fprintf(stderr," %s", funcname ? funcname: "<unknown>");



  if (line>0 || file != NULL)

    fprintf(stderr," [%s:%d]", file, line);



  if (status==TRS_StsAutoTrace)

      fprintf(stderr,":%s", funcname ? funcname: "<unknown>");



  if (context != NULL)

  {

    if (status!=TRS_StsAutoTrace)

      fprintf(stderr, ": %s", context);                /* Print context      */

    else

      fprintf(stderr, "(%s)", context);                /* Print arguments    */

  };

  fprintf(stderr, "\n");

  if (TstStdErrMode == TRS_ErrModeLeaf) {

     fprintf(stderr, "TRS System:\n"

                     "terminating the application");

    exit(1);

  };

  return status;

}



TRSFUN(TRSStatus, trsError,(TRSStatus status, const char *funcname, const char *context,

                const char *file, int line)) {

  return trsStdError(status, funcname, context, file, line);

}

