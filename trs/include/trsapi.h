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


#if !defined _TRSAPI_H
#define _TRSAPI_H

#include <stdio.h>
/* =========================================================================*/
#ifdef __cplusplus
extern "C"{
#endif

#ifndef WIN32
    #define __cdecl
#endif

#if !defined(TRS_W32DLL) || !defined(WIN32)
# define   TRSAPI(type,func,arg)              extern type __cdecl func arg
#elif defined __BORLANDC__
# define TRSAPI(type,func,arg)               type _export __cdecl func arg
#else
# define TRSAPI(type,func,arg) __declspec(dllexport) type __cdecl func arg
#endif


typedef struct _trsProcessorInfo
{
    int     family;
    int     frequency;  /* MHz */
    double  time_scale; /* 1e-6/frequency */
} trsProcessorInfo;


/* === Get internal testsystem info ========================================*/

TRSAPI(void *,trsGet,(int item));

/* === Test registration and starting ======================================*/

typedef int (*TRSProc_t)        (void);     /* Test's body() ptr            */
typedef int (*TRSProcArg_t)     (void*);    /* Test's body(arg) ptr         */
typedef int (*TRSProcTerm_t)    (int);      /* Termination procedure        */

TRSAPI(int,trsReg,(char     *FuncName,      /* Lib Function name            */
                   char     *TestName,      /* Brief test explanation       */
                   char     *TestClass,     /* Name of test class           */
                   TRSProc_t TestProc));    /* Function test body           */

TRSAPI(int,trsRegArg,(
                   char     *FuncName,      /* Lib Function name            */
                   char     *TestName,      /* Brief test explanation       */
                   char     *TestClass,     /* Name of test class           */
                   TRSProc_t TestProc,      /* Function test body           */
                   void     *TestArg));     /* Pointer to any needed data   */

#define TG_REG_FUNCNAME        1            /* "FuncName"                   */
#define TG_REG_TESTNAME        2            /* "TestName"                   */
#define TG_REG_TESTCLASS       3            /* "TestClass"                  */
#define TG_REG_TESTRPOC        4            /* "TestProc"                   */
#define TG_REG_FILE            5            /* __FILE__                     */
#define TG_REG_TESTPROC        6            /* TestProc                     */
#define TG_REG_TESTARG         7            /* TestArg                      */
#define TG_REG_LINE            8            /* __LINE__                     */

/* Test classes:                                                            */
/*   Algorithm   - does the function do what we need ?                      */
/*   SideEffects - does the function do what we don't need ?                */
/*   BadArgs     - attempts to break the function                           */
/*   Timing      - how long we are waiting the function's results           */
/*   Relative    - mutual check with another lib functios                   */
/*   Application - the function usage examples                              */
/*   Tuning      - optimize internal parameters for best performance        */
/*   Any other...                                                           */

/* --- Test Result Status --------------------------------------------------*/
/* This value should be set by the test according to results of test.       */

#define TRS_OK                 TRUE     /* Test is OK, ->.sum file: "OK"    */
#define TRS_FAIL              FALSE     /* Test failed,->.sum file: "FAIL"  */
#define TRS_UNDEF                -1     /* Result undefine, no write to .sum*/


TRSAPI(int,trsResult,(int result, const char *format,...));

/* if trsResult(TRS_FAIL,"Accuracy is %e", 1.24); then in .sum:             */
/*                                                                          */
/* <r>esult: Accuracy is 1.24                                         FAIL  */  

/* --- Terminating function ------------------------------------------------*/

TRSAPI(TRSProcTerm_t, trsRegTerm,(TRSProcTerm_t Term));

/*  Term can free memory, tables, do some completion work, will
    invoked after test ends and get "result" equal to return value
    of test.

    If no any trsRegTerm(Term) - then as usual.

  int Term(int result) {

      switch (result) {
        case TRS_UNDEF:    ...; break;
        case TRS_OK:       ...; break;
        case TRS_FAIL:     ...; break;
        case MyErrorCode1: ...; return TRS_FAIL;
        case MyErrorCode2: ...; return TRS_UNDEF;
        case MyErrorCode3: ...; return TRS_OK;
        default:           ...; break;
      }
      return result;
   }

      if in test                          will be called in trsRun, 

    if (error1) return TRS_FAIL;         trsResult=Term(TRS_FAIL);    
    if (error2) return TRS_UNDEF;        trsResult=Term(TRS_UNDEF);
    if (case1)  return MyErrorCode1;     trsResult=Term(MyErrorCode1);
    if (case2)  return MyErrorCode2;     trsResult=Term(MyErrorCode2);
    if (case3)  return MyErrorCode3;     trsResult=Term(MyErrorCode3);
    return TRS_OK;                       trsResult=Term(TRS_OK);      

   MyErrorCode may be any if out of range [-16..15]
*/
/* -------------------------------------------------------------------------*/

TRSAPI(int,trsRun,(int  argC, char *argV[]));   /* Start test system        */

#define TG_RUN_ARGC            9
#define TG_RUN_ARGV           10

/* --- Test write ----------------------------------------------------------*/

TRSAPI(int, trsWrite,(int twflag,const char *format,...));
                                         /*    When Write?                  */
#define TW_RUN     0x8000                /* in batch mode                   */
#define TW_RERUN   0x4000                /* at restart when error or "-r"   */
#define TW_DEBUG   0x2000                /* in debug mode            "-d"   */
#define TW_EMPTY   0x0000                /* to comment out trsWrite         */

                                         /*    Where Write?                 */
#define TW_CON     0x0800                /* To console                      */
#define TW_LST     0x0400                /* To .lst file             "-l"   */
#define TW_SUM     0x0200                /* To .sum file             "-s"   */

#define TO_CON     TW_RUN | TW_CON
#define TO_LST     TW_RUN | TW_LST
#define TO_SUM     TW_RUN | TW_SUM

/* --- Benchmark functions -------------------------------------------------*/

#define TRS_MAX_TIMER 9                 /* Timers [0..TRS_MAX_TIMER]        */
                                        /* High Priority Timers:            */
                                        /*     0..TRS_MAX_TIMER/2           */

TRSAPI(void,  trsTimerStart,   (int TimerNo)); /* Start counting ticks fr 0 */
TRSAPI(void,  trsTimerStop,    (int TimerNo)); /* Stop counting ticks       */
TRSAPI(void,  trsTimerContinue,(int TimerNo)); /* Continue counting ticks   */

TRSAPI(double,trsTimerClock,   (int TimerNo)); /* Interval in CLOCKS_PER_SEC*/
TRSAPI(double,trsTimerSec,     (int TimerNo)); /* Timer interval in seconds */
                                               /* Getting is available also */
                                               /* on fly (when timer run)   */
TRSAPI(double,trsClocksPerSec,(void));         /* Clock frequency           */

/* --- Loops organization functions ----------------------------------------*/

#define TRS_STEP_ADD 0                          /* Additive step type       */
#define TRS_STEP_MPY 1                          /* Mult step type           */
#define TRS_STEP_DIV 2                          /* Div step type            */

typedef struct _TRSScale_t {
  int start;
  int end;
  int step;
  int steptype;                                 /*TRS_STEP_ADD or TRS_STEP_MPY*/
  int current;                                  /* current point            */
  int init;                                     /* 0 - init, 1 - run        */
} TRSScale_t;

TRSAPI(int, trsScaleInit,(int start,int end,int step,int steptype,TRSScale_t *scale));
/* returns:
     0 - error in parameters
     1 - init OK
*/

TRSAPI(int, trsScaleNextPoint,(TRSScale_t *scale, int *next));
/* returns:
     0 - if no more points (out of range)
     1 - next point value was returned to (*next)
*/

/* --- Guarded memory manage functions -------------------------------------*/
/* This functions may be used in test class "SideEffects".                  */
/* They set low and high guards, and can check their integrity.             */

TRSAPI(void, trsGuardPattern,(long pattern, int number));
/* Set pattern for guard, and number of guards (0<number<=16).              */
/* Default: trsGuardPattern(0xA55A,1), is set before each test.             */

TRSAPI(long int, trsGuardAllocated,(long int base));
/* Returns total memory allocated, relative to base. If base==0, return     */
/* absolute value:                                                          */
/*   base=trsGuardAllocated(0);                                             */
/*   ptr=trsGuardcAlloc(10,20);                                             */
/*   trsGuardAllocated(base) => 200                                         */
/*   trsGuardFree(ptr);                                                     */
/*   trsGuardAllocated(base) => 0                                           */

TRSAPI(void *, trsGuardcAlloc,(size_t number, size_t size));
/* calloc(number,size) and put low and high guard                           */

TRSAPI(void *, trsGuardFree,(void * ptr));
/* Check guarded memory, and if is OK, then free it and returns NULL.       */
/* If guards corrupted, returns ptr.                                        */

TRSAPI(int,    trsGuardCheck,(void * ptr));
/* Check guarded memory, allocated by guardcAlloc. Returns:                 */
/*   0 - if memory OK                                                       */
/*  -1 - if low guard is corrupted                                          */
/*   1 - if high guard is corrupted                                         */

/* --- Allocate / Deallocate -----------------------------------------------*/

TRSAPI(void   *, trsmAlloc,    (size_t size));
TRSAPI(void   *, trsreAlloc,   (void *buf,size_t size));

TRSAPI(float  *, trssbAllocate,(int n));
TRSAPI(double *, trsdbAllocate,(int n));
TRSAPI(short  *, trswbAllocate,(int n));
TRSAPI(int    *, trsibAllocate,(int n));
TRSAPI(long   *, trslbAllocate,(int n));

TRSAPI(void   *, trsFree,      (void * ptr));

/* --- Input ----------------------------------------------------------------*/
/* Read data from console, command line or .ini file.                        */
/* dst  - variable address, where to place entered data.                     */
/* def  - default value, is assigned if <CR> was entered.                    */
/*        if def=="", then  trs<?>Reads will wait for any valid input.       */
/*        if def==NULL, then, if there is no valid input, function returns 0,*/
/*        trsReadSource()=>TRS_None, and trsStatus()=>TRS_CONTINUE           */
/* help - help line, printed if '?' was entered                              */
/* All this functions return value:                                          */
/*   1 - if read was successful, i.e. some value was assigned to dst,        */
/*   0 - if entered command was  !n,!q,!p,!r, or (<cr> with def==NULL), and  */
/*       no value was  assigned to dst.                                      */
/* Extra available input for all ?Read functions:                            */
/*  ?  - help on value                  !e - exit current test               */
/*  ?? - test header                    !q - quit                            */
/*  ?h - help on test                   !r - retry current test              */
/*  ?l - test listing                   !p - go to previous test             */
/*  ?L - whole source file listing      !t - go to top of test list          */
/*  ;  - token separator                                                     */
/*  <  - get current value from ini file                                     */
/*  *< - get current and rest values from ini file                           */
/*  > -  put current value to ini file                                       */
/*  *> - put all defined values to ini file                                  */
/*  Case,Bit and tRead need first token to be their valid value:             */
/*  ...,text [abc]=> new;>                                                   */
/*  or                                                                       */
/*  ...,text [abc]=> ;>                                                      */
/*  but not extra input:                                                     */
/*  ...,text [abc]=> >                                                       */

TRSAPI(int, trsCaseRead,(int *dst, char *items, char *def, char *helps));

/* Function trsCaseRead assignes to dst item's number, if answer string was  */
/* found in items list. Items are counted from 0 by step 1.                  */
/* First char in items is separator.                                         */
/* Returns:                                                                  */
/*  1 - if item was recognized                                               */
/*  0 - if command was !n,!q,...                                             */
/*  First symbol in items is separator.                                      */
/*  Example:                                                                 */
/*   if (!trsCaseRead(Mode,"/C/Exit","Exit","Continue/Exit program"))        */
/*      return TRS_UNDEF;                                                    */
/*   if (trsReadSource()==TRS_Default) ...                                   */
/*  or                                                                       */
/*    if (!trsCaseRead(Mode,"/C/Exit","C","Continue/Exit program"))          */
/*      if (trsStatus()==TRS_QUIT) ...                                       */
/*                                                                           */

TRSAPI(int, trsBitRead, (long int *dst, char *items, char *def, char *helps));

/* Function trsBitRead sets bits into dst if respective item from list was   */
/* found. It returns:                                                        */
/*  1 - if item was recognized                                               */
/*  0 - if command was !n,!q,...                                             */
/*                                                                           */
/* Format of items and helps is as in function trsCaseRead                   */

TRSAPI(int, trstRead,(char   *dst, int size, char *def, char *help));
/* size = sizeof(dst[])                                                      */

typedef enum { s_fvr,c_fvr,d_fvr,z_fvr,
               w_fvr,v_fvr,i_fvr,j_fvr,l_fvr,
               text_fvr,case_fvr,bit_fvr} TRSfvr_t;

TRSAPI(int, trslRead,(long    *dst, char *def, char *help));

TRSAPI(int, trswRead,(short   *dst, char *def, char *help));
TRSAPI(int, trsiRead,(int     *dst, char *def, char *help));
TRSAPI(int, trssRead,(float   *dst, char *def, char *help));
TRSAPI(int, trsdRead,(double  *dst, char *def, char *help));

TRSAPI(int, trs_Read,(void    *dst, char *name,void *extra, char *def,
                      char    *help,TRSfvr_t dsttype));

TRSAPI(int, trslbRead,(long   *dst, int n, char *def, char *help));
TRSAPI(int, trswbRead,(short  *dst, int n, char *def, char *help));
TRSAPI(int, trsibRead,(int    *dst, int n, char *def, char *help));
TRSAPI(int, trssbRead,(float  *dst, int n, char *def, char *help));
TRSAPI(int, trsdbRead,(double *dst, int n, char *def, char *help));

TRSAPI(int, trs_bRead,(void   *dst, char *name,int n,char *def,char *help,
                       TRSfvr_t dsttype));

TRSAPI(int, trsPause, (char *def, char *help));

/* --- Returned Flow Status after ?Reads ------------------------------------*/

#define TRS_CONTINUE              0     /* Normal flow                       */
#define TRS_RETRY              -101     /* Restart current test          !r  */
#define TRS_PREVIOUS           -102     /* Start previous test           !p  */
#define TRS_EXIT               -103     /* Exit test with trsResult(0)   !e  */
#define TRS_TOP                -104     /* Go to top of list             !t  */
#define TRS_QUIT               -105     /* Quit test system tsResult(0)  !q  */

TRSAPI(int, trsStatus,(void));           /* Return flow status                */

/* --- Returned source type from where was last Read ------------------------*/

#define TRS_NoInFile             -3     /* No value in .ini file             */
#define TRS_None                 -2     /* No any read                       */
#define TRS_DefaultNull          -1     /* <CR> was entered and default==NULL*/
#define TRS_Default               0     /* <CR> was entered, default accepted*/
#define TRS_TTYInput              1     /* direct answer: Enter var> 1,1     */
#define TRS_IndirectTTY           2     /* previously entered "var=src"      */
#define TRS_FileInput             3     /* found in file .ini                */
#define TRS_CmdLine               4     /* through -D "var=src"              */

TRSAPI(int, trsReadSource,(void));      /* Return read source                */

/*****************************************************************/
/* --- Generate special benchmark output file -------------------*/
/*****************************************************************/

TRSAPI(void, trsCSVString5,(char *flavour,char *function,char *value,
                            char *units,  char *comments));

TRSAPI(void, trsCSVString6,(char *flavour,char *function,char *value,
                            char *units,  char *comments,char *param1));

TRSAPI(void, trsCSVString7,(char *flavour,char *function,char *value,
                            char *units,  char *comments,
                            char *param1, char *param2));

TRSAPI(void, trsCSVString8,(char *flavour,char *function,char *value,
                            char *units,  char *comments,
                            char *param1, char *param2,  char *param3));

TRSAPI(void, trsCSVString9,(char *flavour,char *function,char *value,
                            char *units,  char *comments,char *param1,
                            char *param2, char *param3,  char *param4));

TRSAPI(void, trsCSVString10,(char *flavour,char *function,char *value,
                             char *units,  char *comments,char *param1,
                             char *param2, char *param3,  char *param4,
                             char *param5));
TRSAPI(void, trsCSVString11,(char *flavour,char *function,char *value,
                             char *units,  char *comments,char *param1,
                             char *param2, char *param3,  char *param4,
                             char *param5, char *param6));
TRSAPI(void, trsCSVString12,(char *flavour,char *function,char *value,
                             char *units,  char *comments,char *param1,
                             char *param2, char *param3,  char *param4,
                             char *param5, char *param6,  char *param7));

TRSAPI(char *, trsFloat ,(float  src));
TRSAPI(char *, trsShort ,(short  src));
TRSAPI(char *, trsDouble,(double src));
TRSAPI(char *, trsInt   ,(int    src));

trsProcessorInfo InitProcessorInfo();

/* Only for compatibility with timapi.c                                      */

TRSAPI(void, trsCommandLine,(int argc, char *argv[]));

/* ==========================================================================*/
                                        /* Private section of test system    */
#include "trsipi.h"                     /* Redefinitions and common objects  */

#ifdef __cplusplus
}
#endif

#endif                                  /* _TRSAPI_H                         */
