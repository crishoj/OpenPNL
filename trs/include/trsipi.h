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


#if !defined _TRSIPI_H
# define _TRSIPI_H

#if !defined(TRS_W32DLL)
# define   TRSFUN(type,func,arg)                     extern type __cdecl func arg
#elif  !defined __BORLANDC__
# define TRSFUN(type,func,arg) extern __declspec(dllexport) type __cdecl func arg
#else
# define TRSFUN(type,func,arg)                       extern type _import __cdecl func arg
#endif

#define   TRSVAR(type)                                      type
#define   TRSREF(type)                               extern type

/* ==========================================================================*/

#include "trserror.h"

#define _TRSREAD_C

#ifndef FALSE
#  define FALSE 0
#endif

#ifndef TRUE
#  define TRUE  1
#endif
/* === Test System Private objects ==========================================*/

#define RETRY                    -1     /* Retry input                       */

#define DEFAULT_PAGE_SIZE 24
#define INPBUF_LEN        1024
#define HELP_PREFIX       '?'
#define LF                0x0A
#define CR                0x0D
#define HT                0x09
#define SQ                0x27          /* '                                 */
#define DQ                0x22          /* "                                 */
#define EOL               0
#define Bool_t            unsigned char
#define TABSIZE           8             /* for ini files                     */

/* === Types =============================================================== */

typedef struct _TRSid_t {              /* Test descriptor                   */
  struct _TRSid_t *next;                /* Next test in a list               */
  char              *funcname;          /* Testing function name             */
  char              *testname;          /*                                   */
  char              *testclass;         /* For test classification           */
  TRSProcArg_t       test;              /* t_ Function test                  */
  void              *arg;               /* Pointer to data                   */
  char              *proctext;          /* Test ascii name                   */
  char              *file;              /* File, containing test             */
  int                line;              /* Start line of test                */
  char              *infotext;          /* Optional info                     */
  int                status;            /* Status bits                       */
} TRSid_t;

/* === External variables =================================================*/

TRSREF(int      ) Trs_FlowStatus;           /* Used to control test flow       */
TRSREF(int      ) Trs_ReadSource;
TRSREF(TRSid_t *) Trs_CurrentTest;
TRSREF(char     ) Trs_String[INPBUF_LEN];
TRSREF(int      ) Trs_PageSize;

#if 0
TRSREF(char     ) CatalogSep;
TRSREF(char     ) Cmd[1024];
TRSREF(char     ) cmdprefix;
TRSREF(char     ) ExePath[512];
TRSREF(char *   ) helpprompt;
TRSREF(FILE *   ) IniFile;
TRSREF(char *   ) IniFileName;
TRSREF(FILE *   ) LstFile;
TRSREF(char *   ) LstFileName;
TRSREF(char *   ) Nullstr;              /* ==NullStr - always length = 0   */
TRSREF(char *   ) prompt;
TRSREF(FILE *   ) SumFile;
TRSREF(char *   ) SumFileName;
TRSREF(char *   ) TestAppName;
TRSREF(char *   ) TestID;
TRSREF(TRSid_t *) TestList;

#endif

/* ==========================================================================*/

/* --- Command line key values ----------------------------------------------*/

/* Return pointer to value of the 'key' from OS command line                 */

TRSAPI(const char *, trsGetKeyArg,(char key)); /* ptr==NULL - key is not set   */
                                        /* (*ptr)==0 - key is set with no arg*/
                                        /* (*ptr)!=0 - key is set with arg   */
/* --- String parsing function ----------------------------------------------*/

TRSAPI(char *, StringDup,(const char *str));
/* Only because strdup is not ANSI C function                                */

TRSAPI(char *, reStringDup,(char *oldptr, const char *str));
/* free(oldptr); res=StrDup(str)                                             */

TRSAPI(char *, StringCat,(char *str,const char *suffix));
/* Reallocate extra space for str, append suffix and return resulting string */
/* str    - may be NULL or pointer, returned by any mem alloc function       */
/* suffix - may be NULL or any string                                        */

TRSAPI(int, StringnCpy,(char *dst,const char *src,int size));
/* StrnCpy copy src to dst while not 0, and number of char to copy is less   */
/* than size-1, dst will be terminated by 0                                  */
/* Returns number of chars copied                                            */

TRSAPI(int, SpaceCut,(char *str));
/* Remove leading and trailing spaces from str                               */
/* Returns length of new string                                              */

TRSAPI(int, StrParse,(char *buf,char *seps,int n,...));

/* StrParse use separators sequentially to copy parts to destinations        */
/* If too few separators, the last separator is used.                        */
/* Destinations must be large enough to contain parts.                       */
/* n - number of destinations 1..                                            */
/* StrParse returns number of times the separators were used.                */

#define StrParse2(buf,seps,d1,d2)             StrParse(buf,seps,2,d1,d2)
#define StrParse3(buf,seps,d1,d2,d3)          StrParse(buf,seps,3,d1,d2,d3)
#define StrParse4(buf,seps,d1,d2,d3,d4)       StrParse(buf,seps,4,d1,d2,d3,d4)
#define StrParse5(buf,seps,d1,d2,d3,d4,d5)    StrParse(buf,seps,5,d1,d2,d3,d4,d5)
#define StrParse6(buf,seps,d1,d2,d3,d4,d5,d6) StrParse(buf,seps,6,d1,d2,d3,d4,d5,d6)

TRSAPI(int, BracketsParse,(char *buf,char br0,char br1,char *pref,char *expr,char *suff));

/* BracketsParse use pair of brackets (br0,br1) to parse buf to:             */
/* pref   - before first br0                                                 */
/* expr   - between pair (br0,balanced br1), may contain balanced (br0,br1), */
/*          outermoust brackets are removed                                  */
/* suff   - after br1,balanced to first br0, may contain any combination of  */
/*          br0,br1                                                          */
/* Return: -1, if any ptr==NULL                                              */
/*          0, if brackets are balanced in expr, (but may be not in suff)    */
/*          n, level of unbalanced brackets in expr, suff[0] assigned to 0   */


/* ==========================================================================*/
/*           Redefinitions                                                   */
/* There is no need to examine this part of header. It is need to simplify   */
/* usage of first part of header.                                            */
/* ==========================================================================*/
/* Redefine trsReg to get __FILE__,__LINE__ values and ascii name of Test:   */

#define trsReg(FuncName,TestName,TestClass,Test) \
  _trsReg(FuncName,TestName,TestClass,(Test),\
   #Test,__FILE__,__LINE__)

#define trsRegArg(FuncName,TestName,TestClass,Test,Arg) \
  _trsRegArg(FuncName,TestName,TestClass,(Test),(void*)(Arg),\
  #Test"("#Arg")",__FILE__,__LINE__,1)

/* === Real   function:  ====================================================*/
TRSAPI(int, _trsReg,(
              char       *FuncName,     /* Lib Function name                 */
              char       *TestName,     /* Test Name                         */
              char       *TestClass,    /* Name of test class                */
              TRSProc_t   Test,         /* t_ Function test                  */
              char       *FuncCall,     /* Test ascii name                   */
              char       *File,         /* File, where is the test           */
              int         Line));       /* Start line of the test            */

TRSAPI(int, _trsRegArg,(
              char        *FuncName,    /* Lib Function name                 */
              char        *TestName,    /* Test Name                         */
              char        *TestClass,   /* Name of test class                */
              TRSProcArg_t Test,        /* t_ Function test                  */
              void        *Arg,         /* Pointer to any data structure     */
              char        *FuncCall,    /* Test ascii name                   */
              char        *File,        /* File, where is the test           */
              int          Line,        /* Start line of the test            */
              int          Status));    /* Test reg status                   */

#define trsCaseRead(dst,items,def,helps) _trsCaseRead(dst,#dst,items,def,helps)
#define trsBitRead(dst,items,def,helps)  _trsBitRead(dst,#dst,items,def,helps)
#define trstRead(dst,size,def,help)      _trstRead (dst,size,#dst,def,help)

#define trslRead(dst,def,help)    _trslRead (dst,#dst,def,help)
#define trswRead(dst,def,help)    _trswRead (dst,#dst,def,help)
#define trsiRead(dst,def,help)    _trsiRead (dst,#dst,def,help)
#define trssRead(dst,def,help)    _trssRead (dst,#dst,def,help)
#define trsdRead(dst,def,help)    _trsdRead (dst,#dst,def,help)

#define trslbRead(dst,n,def,help) _trslbRead( dst,#dst,n,def,help)

#define trswbRead(dst,n,def,help) _trswbRead( dst,#dst,n,def,help)
#define trsibRead(dst,n,def,help) _trsibRead( dst,#dst,n,def,help)
#define trssbRead(dst,n,def,help) _trssbRead( dst,#dst,n,def,help)
#define trscbRead(dst,n,def,help) _trscbRead( dst,#dst,n,def,help)

/* --- Input ----------------------------------------------------------------*/

TRSAPI(int, _trsCaseRead,(int      *dst,char *name,char *items, char *def,char *help));
TRSAPI(int, _trsBitRead, (long int *dst,char *name,char *items, char *def,char *help));
TRSAPI(int, _trstRead,   (char     *dst,int size,char *name,char *def,char *help));

TRSAPI(int, _trslRead,   (long   *dst,char *name,char *def,char *help));
TRSAPI(int, _trswRead,   (short  *dst,char *name,char *def,char *help));
TRSAPI(int, _trsiRead,   (int    *dst,char *name,char *def,char *help));
TRSAPI(int, _trssRead,   (float  *dst,  char *name,char *def,char *help));
TRSAPI(int, _trsdRead,   (double *dst,  char *name,char *def,char *help));

TRSAPI(int, _trslbRead,  (long   *dst,  char *name, int n,char *def,char *help));
TRSAPI(int, _trswbRead,  (short  *dst,  char *name, int n,char *def,char *help));
TRSAPI(int, _trsibRead,  (int    *dst,  char *name, int n,char *def,char *help));
TRSAPI(int, _trssbRead,  (float  *dst,  char *name, int n,char *def,char *help));
TRSAPI(int, _trsdbRead,  (double *dst,  char *name, int n,char *def,char *help));

/* --------------------------------------------------------------------------*/

#define trsmAlloc(size)             _trsmAlloc(size,__FILE__,__LINE__)
#define trsreAlloc(buf,size)        _trsreAlloc(buf,size,__FILE__,__LINE__)
#define trsGuardcAlloc(number,size) _trsGuardcAlloc(number,size,__FILE__,__LINE__)
#define trsGuardFree(buf)           _trsGuardFree(buf,__FILE__,__LINE__)
#define trsGuardCheck(buf)          _trsGuardCheck(buf,__FILE__,__LINE__)

TRSAPI(void *,   _trsmAlloc  ,(size_t size,char *file,int line));
TRSAPI(void *,   _trsreAlloc ,(void *buf,size_t size,char *file,int line));
TRSAPI(void *,   _trsGuardcAlloc,(size_t number, size_t size,char *file,int line));
TRSAPI(void *,   _trsGuardFree  ,(void * ptr,char *file, int line));
TRSAPI(int   ,   _trsGuardCheck ,(void * ptr,char *file, int line));

/* === End of Redefinitions =================================================*/

TRSAPI(void,   trs_FlushAll,(void));

TRSAPI(void,   trsPrintHelp,(TRSid_t * ptr,char mode,char *name,char *def,char *help));
TRSAPI(void,   trsMoreSize,(int i));

TRSAPI(int,    trs_arInit,(void));
TRSAPI(void,   trs_arTerminate,(void);)

TRSAPI(int,    CmdWithoutPrefix,(char *buf,  char *prompt, char *dstname,
                                char *type, char *def,    char *help));

TRSAPI(int,    ParseFileAppKey,(char *string,char *srcname));
TRSAPI(char *, GetDstFromFile,(char *string,char *srcname));
TRSAPI(void,   PutToFile,(char *key,char *value, char *help));

/* ==========================================================================*/
/* Redefinition of standard functions to gather output to .lst & .sum files  */
/* when -p [file] or -P [file] or -p [file] -P [file] command line keys used */

#if FALSE
#if defined putchar
# undef putchar
#endif

#define putchar _putchar
#define printf  _printf
#define fprintf _fprintf
#define gets    _gets
#endif

TRSAPI(int,    _putchar,(int ch));
TRSAPI(int,    _printf,(const char *format,...));
TRSAPI(int,    _fprintf,(FILE *st,...));
TRSAPI(char *, _gets,(char *buf));

TRSAPI(int,    putchar_Prot,(int ch));
TRSAPI(int,    printf_Prot,(const char *format,...));

/* ==========================================================================*/

#endif                                  /* _TRSIPI_H                         */
