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


#define USE_WIN32_TIMER
#define HIGH_PRIORITY
#define WIN32_LEAN_AND_MEAN

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <assert.h>


#if (defined USE_WIN32_TIMER || defined HIGH_PRIORITY) && defined WIN32
# include <windows.h>
#endif

#include "trsapi.h"                     /* <stdio.h> inside                  */

/* --------------------------------------------------------------------------*/

#if (defined USE_WIN32_TIMER || defined HIGH_PRIORITY) && defined WIN32
#include "trswind.h"
#endif

/* --------------------------------------------------------------------------*/
#if defined putchar
# undef putchar
#endif

#define putchar _putchar
#define printf  _printf
#define fprintf _fprintf
#define gets    _gets

/* === For High Priority Timers 0..4 ========================================*/
#if defined HIGH_PRIORITY && defined WIN32
  void trsHighPriority(void) {
    SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);
  }
  void trsNormalPriority(void) {
    SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);
  }
#else
#define trsHighPriority()
#define trsNormalPriority()
#endif

#define TRS_MIN_HP_TIMER 0
#define TRS_MAX_HP_TIMER TRS_MAX_TIMER/2

/* ==========================================================================*/

#define COMMENT_EMPTY     0
#define COMMENT_STORING   1
#define COMMENT_READY    -1
#define ____(str)                       /* No action                         */

/* ==========================================================================*/

typedef struct _CommentsType {          /* Test Comment List                 */
  struct _CommentsType  *next;          /* Next test in a list               */
  char                  *comment;       /* Testing function name             */
} CommentsType;

/* ==========================================================================*/

TRSVAR(TRSid_t   *) Trs_CurrentTest = NULL;
TRSVAR(int        ) Trs_FlowStatus  = TRS_CONTINUE;
TRSVAR(int        ) Trs_PageSize    = 0;
TRSVAR(int        ) Trs_ReadSource  = TRS_None;
TRSVAR(char       ) Trs_String[INPBUF_LEN] = {0};

/* --- previous TRSVAR variables ------------------------------------------*/
static char          CatalogSep  = '\\';
static char          Cmd[1024]   = "";
static char          cmdprefix   = '!';
static char          ExePath[512]= "";
static char *        helpprompt  = ";!e;!q;!p;!r;!t;\?;\?\?;?h;?l;?L;<;*<;>;*>"
                                ";*<file[sec];*>file[sec]";
static FILE *        IniFile     = NULL;
static char *        IniFileName = NULL;
static FILE *        LstFile     = NULL;
static char *        LstFileName = NULL;
static char *        NullStr     ="";   /* ==NullStr - always length = 0     */
static char *        prompt      = "Enter ";
static FILE *        SumFile     = NULL;
static char *        SumFileName = NULL;
static char *        TestAppName = NULL;
static char *        TestID      = "TestSystem";
static TRSid_t *     TestList    = NULL;;
static FILE *        CsvFile     = NULL;
static char *        CsvFileName = NULL;

/* ------------------------------------------------------------------------- */

static int           TestResult      = FALSE;
static time_t        TotalTime;
static time_t        TestTime;
static CommentsType *CommentList     = NULL;
static int           CommentStatus   = COMMENT_EMPTY;
static int           CurrLine;
static int           FullHelp        = FALSE;
static char         *TmpIniFileName= "TRSFUN.tmp";
static char         *LoadIniFileName;

static int           RunMode         = FALSE;
static int           DebugMode       = FALSE;
static int           ReRunMode       = FALSE;
/*static int           LoopMode        = FALSE;*/

static int           ReRunCount      = 0;
static int           LoopMax         = 1;
static int           LoopCount;
static char         *ResultString    = NULL;
static TRSProcTerm_t Terminate       = NULL;

/* ==========================================================================*/

#if !defined _TRSREAD_C
TRSFUN(int,  trs_arInit,(void)){return 0;}
TRSFUN(void, trs_arTerminate,(void)){}
#endif

/* ==========================================================================*/
TRSFUN(int, trsStatus,(void)){                    /* Return flow status        */
  return Trs_FlowStatus;
}

TRSFUN(int, trsReadSource,(void)){                        /* Return read source        */
  return Trs_ReadSource;
}

TRSFUN(TRSProcTerm_t,trsRegTerm,(TRSProcTerm_t Term)) {
  TRSProcTerm_t  oldTerm = Terminate;

  Terminate=Term;
  return oldTerm;
}

TRSFUN(void *,trsGet,(int item)){
  if (item) return NULL;
  return NULL;
}

/* --- Allocate / Deallocate ------------------------------------------------*/

/*typedef long int       INT32;*/
#ifndef WIN32
    typedef int INT32;
#endif

static INT32     Pattern=0xFFFFFFFFL;
static INT32     Patterns=3;
static INT32     MemAllocated=0;              /* through guardAlloc        */
static INT32    *FullBlock;                   /* returned by calloc        */
static INT32     FullPatterns;                /* pat no in releasing block */
static INT32     OriginalSize;                /* called size to alloc,bytes*/

TRSFUN(void,  trsGuardPattern,(long pattern, int number)){
  if ((number>0) && (number<=16)) {
    Patterns=number;                            /* No of check patterns      */
    Pattern=pattern;
  }
}

TRSFUN(long int,  trsGuardAllocated,(long int base)) {
  return (MemAllocated-base);
}

TRSFUN(void *, _trsGuardcAlloc,(size_t number, size_t size,char *file,int line)) {
  size_t     newnumber;                        /* + extra space for guards  */
  INT32     *res;
  INT32     *block;
  INT32      i;

  newnumber=number+((size_t)(Patterns*4+2)*sizeof(Pattern) - 1 + size)/size;
  res=(INT32*)calloc(newnumber,size);

  if (!res)
    trsError(TRS_StsNoMem,"trsGuardcAlloc","",file,line);
  else {
    block=res;
    i=Patterns-1;
    while (i--) {                               /* if more, than 1 pattern   */
      (*block++) =Pattern^0xAAAAAAAAL;          /* pattern^10101010          */
      (*block++) =Pattern;
    }

    (*block++) =Patterns;                       /* Number of check patterns  */
    (*block++) =(INT32)number*(INT32)size;      /* Orig size of block, bytes */
    (*block++) =Pattern^0xAAAAAAAAL;            /* pattern^10101010          */
    (*block++) =Pattern;

    res   = block;                              /* start of allocated block  */
    block = (INT32*)((INT32)res+
                     (INT32)number*(INT32)size);/* end of allocated block    */

    i=Patterns;
    while (i--) {
      (*block++) = Pattern^0x55555555L;         /* pattern^01010101          */
      (*block++) =~Pattern;
    }
    MemAllocated = MemAllocated + (INT32) number * (INT32) size;
  }
  return res;
}

TRSFUN(int, _trsGuardCheck,(void * ptr,char *file,int line)){
  INT32  *block;
  INT32   pattern;
  INT32   i;

  if (ptr) {
    block=(INT32*)ptr;
    pattern=*(--block);
    if ((INT32)(pattern^0xAAAAAAAAL) != *(--block)) {
      trsError(TRS_StsBadMem,"trsGuardCheck","Base(low) guard",file,line);
      return -1;
    }
    OriginalSize =      *(--block);             /* get block size in bytes   */
    FullPatterns = (int)*(--block);             /* get patterns number       */
    i=FullPatterns-1;
    while (i--) {
      if ( ((INT32)(pattern            ) != *(--block)) ||
           ((INT32)(pattern^0xAAAAAAAAL) != *(--block))) {
        trsError(TRS_StsBadMem,"trsGuardCheck","Low guard",file,line);
        return -1;
      }
    }
    FullBlock=block;                            /* original start of mem block*/
    block = (INT32*)((INT32)ptr+
                     (INT32)OriginalSize);      /* end of allocated block    */
    i=FullPatterns;
    while (i--) {
      if ( (( pattern^0x55555555L) != (*block++)) ||
           ((~pattern            ) != (*block++)) ) {
        trsError(TRS_StsBadMem,"trsGuardCheck","High guard",file,line);
        return 1;
      }
    }
  }
  return 0;
}

TRSFUN(void *, _trsGuardFree,(void * ptr,char *file, int line)){
  INT32 i;
  if (ptr) {
    if (!_trsGuardCheck(ptr,file,line)) {
      for (i=0; i<(FullPatterns*2+2); i=i+1)    /* +Patt no+size             */
        FullBlock[i]=0L;                        /* Clear Low Guard           */
      free(FullBlock);                          /* FullBlock,FullSize        */
      MemAllocated = MemAllocated-OriginalSize; /* were set in _guardCheck   */
      ptr=NULL;
    } else trsError(TRS_StsInternal,"trsGuardFree",
                    "Unable to free corrupted memory",file,line);
  }
  return ptr;
}
/* ------------------------------------------------------------------------- */
TRSFUN(void *, _trsmAlloc,(size_t size,char *file, int line)) {
/*  static void * res;*/
  void * res;
  res=malloc(size);
  if (!res)
    trsError(TRS_StsNoMem,"trsmAlloc","",file,line);
/*  else
    MemAllocated=MemAllocated+(unsigned long int)size;
*/
  return res;
}

TRSFUN(void *, trsFree, (void * ptr)) {
  if (ptr)  free(ptr);
  return NULL;
}

TRSFUN(void *, _trsreAlloc,(void *buf,size_t size, char *file,int line)) {
/*  static void * res;*/
  void * res;

  res=realloc(buf,size);
  if (!res) trsError(TRS_StsNoMem,"trsreAlloc","",file,line);
  return res;
}

TRSFUN(float  *, trssbAllocate,(int n)) {
  float *ptr;
  ptr=(float*)calloc(n,sizeof(float));
  if (!ptr) trsError(TRS_StsNoMem,"trssbAllocate","",TRS_FILE,__LINE__);
  return ptr;
}

TRSFUN(double *, trsdbAllocate,(int n)){
  double *ptr;

  ptr=(double*)calloc(n,sizeof(double));
  if (!ptr) trsError(TRS_StsNoMem,"trsdbAllocate","",TRS_FILE,__LINE__);
  return ptr;
}

TRSFUN(short  *, trswbAllocate,(int n)){
  short *ptr;

  ptr=(short*)calloc(n,sizeof(short));
  if (!ptr) trsError(TRS_StsNoMem,"trswbAllocate","",TRS_FILE,__LINE__);
  return ptr;
}

TRSFUN(int  *, trsibAllocate,(int n)){
  int *ptr;

  ptr=(int*)calloc(n,sizeof(int));
  if (!ptr) trsError(TRS_StsNoMem,"trsibAllocate","",TRS_FILE,__LINE__);
  return ptr;
}


TRSFUN(long  *, trslbAllocate,(int n)){
  long *ptr;

  ptr=(long*)calloc(n,sizeof(long));
  if (!ptr) trsError(TRS_StsNoMem,"trslbAllocate","",TRS_FILE,__LINE__);
  return ptr;
}

/* ==========================================================================*/

TRSFUN(char *, StringDup,(const char *str)){
  char * res;
  int    len=1;

  if (str) len=len+strlen(str);
  res=(char*)trsmAlloc(len);
  if (!res) printf("No Memory, StringDup(%s)\n",str);
  else {
    if (str) strcpy(res,str);
    else *res=0;
  }
  return res;
}

TRSFUN(char *, reStringDup,(char *oldptr,const char *str)){
  char * res;
  trsFree(oldptr);
  res=StringDup(str);
  if (!res) printf("No Memory, reStringDup(%s)\n",str);
  return res;
}

TRSFUN(char *, StringCat,(char *str,const char *suffix)) {
  char *res;
  int   len=1;

  if (str)    len=len+strlen(str);
  if (suffix) len=len+strlen(suffix);
  res=(char*)realloc(str,len);
  if (!res) {
    printf("?StringCat-No Memory for \"%s%s\"\n",str,suffix);
  } else
    if (suffix) strcat(res,suffix);
  return res;
}

TRSFUN(int, StringnCpy,(char *dst,const char *src,int size)) {
  int i=0;

  if (!dst) return 0;
  if (src)
    for (;i<size-1;i=i+1) {
      if (!src[i]) break;
      dst[i]=src[i];
    }
  dst[i]=0;
  return i;
}

TRSFUN(int, SpaceCut,(char *str)) {
  int i=0,j=0,zeropos=0;
  int copy=FALSE;

  if (!str) return 0;

  while (str[i]!=0) {
    if (str[i]!=' ') {
      copy=copy||TRUE;                          /* Set after leading spaces  */
      zeropos=j+1;                              /* First trailing space      */
    }
    if (copy) {                                 /* start copying             */
      str[j]=str[i];
      j=j+1;
    }
    i=i+1;
  }
  str[zeropos]=0;
  return zeropos;
}

TRSFUN(int, StrParse,(char *buf,char *seps,int n,...)){
  va_list   argptr;
  int       k=1,sepno=0;
  char     *dst;
  char      sep;

  va_start(argptr,n);

  if ((buf) && (seps)) {
    sep=seps[0];

    while (n>0) {
      dst=va_arg(argptr,char*);                 /* new dst                   */
      while ((*buf)!=0) {                       /* until end of string       */
        if (((*buf)==sep) && (n>1)) {           /* not last dst              */
          sepno=sepno+1;                        /* number of used separators */
          if (seps[k]!=0) {sep=seps[k];k=k+1;}  /* new separator             */
          buf++;                                /* skip sep in buf           */
          break;
        }
        (*dst)=(*buf);                          /* fill dst                  */
        buf++;
        dst++;
      }
      (*dst)=0;                                 /* terminate dst             */
      n=n-1;
    }
  }
  va_end(argptr);
  return sepno;                                 /* no of used seps           */
}

TRSFUN(int, BracketsParse,(char *buf,char br0,char br1,char *prefix,char *expr,char * suffix)){
  int       i,j;
  int       res=-1;                             /* -1 if any NULL pointer    */
  int       level;

  if ((buf) && (prefix) && (expr) && (suffix)) {

    for (i=0;((buf[i]!=0) && (buf[i]!=br0));) { /* Copy prefix until br0     */
      prefix[i]=buf[i];i=i+1;
    }
    prefix[i]=0;

    for (j=0,level=0;buf[i]!=0;) {              /* Copy expr until pair br1  */
      if (buf[i]==br1) level=level-1;
      if (level>0) {expr[j]=buf[i];j=j+1;}
      if (buf[i]==br0) level=level+1;
      i=i+1;
      if (level==0) break;
    }
    expr[j]=0;  res=level;

    for (j=0;buf[i]!=0;i=i+1) {                /* copy rest of buf to suffix*/
      suffix[j]=buf[i];
      j=j+1;
    }
    suffix[j]=0;
  }
  return res;
}

static void* Reverse(void* Start){
  typedef struct _listtype { struct _listtype *next;} listtype;

  listtype *oldlist=(listtype*)Start;
  listtype *newlist=NULL,  *elem;

  while(oldlist) {
    elem=oldlist;
    oldlist=oldlist->next;
    elem->next=newlist;
    newlist=elem;
  }
  return newlist;
}
/* ==========================================================================*/

TRSFUN(int, _trsReg,(char *FuncName, char *TestName,
            char *TestClass,TRSProc_t Test,   char *FuncCall,
            char *File,     int  Line)) {

  return _trsRegArg(FuncName,TestName,TestClass,
                    (TRSProcArg_t)Test,NULL, FuncCall,File,Line,0);
}

TRSFUN(int, _trsRegArg,(char *FuncName,  /* lib Function name                 */
              char        *TestName,     /* Brief test explanation            */
              char        *TestClass,    /* Name of test class                */

              TRSProcArg_t Test,         /* t_ Function test                  */
              void        *Arg,          /* Pointer to test data              */
              char        *FuncCall,     /* Test ascii name                   */
              char        *File,         /* File, where is the test           */
              int          Line,         /* Start line of the test            */
              int          Status))       /* Test reg status                   */
{
  TRSid_t       *ptr=NULL;            /* =NULL for bcpro only */
  TRSid_t       *scan;

  if (!RunMode && !ptr) {
    ptr =(TRSid_t*)malloc(sizeof(TRSid_t));

    ptr->funcname     = FuncName;
    ptr->testname     = TestName;
    ptr->testclass    = TestClass;
    ptr->proctext     = FuncCall;
    ptr->file         = File;
    ptr->line         = Line;
    ptr->test         = Test;
    ptr->arg          = Arg;
    ptr->status       = Status;
    ptr->infotext     = NULL;
    ptr->next         = NULL;

    if (TestList==NULL)
      TestList=ptr;
    else {
      scan=TestList;
      while ((scan->next) !=NULL)   scan=scan->next;
      scan->next=ptr;
    };
  };
  return RunMode;
}

/* ==========================================================================*/

TRSFUN(void, trsMoreSize,(int i)) {

  Trs_PageSize=i;
  CurrLine=1;
}

static void DeleteComments(void) {
  CommentsType *ptr;

  while (CommentList) {                         /* Release test reg memory   */
    ptr=CommentList;
    CommentList=CommentList->next;
    if (ptr->comment) free(ptr->comment);
    free(ptr);
  }
  CommentStatus=COMMENT_EMPTY;
}

/* Registrate some additional text                                           */

static const char* trsComment(const char *format,...){
  va_list       argptr;
  int           res;
  char          buf[1024];
  CommentsType *ptr;

  va_start(argptr,format);
  res=vsprintf(buf,format,argptr);
  va_end(argptr);

  if (res>sizeof(buf)) exit(0);
  if (CommentStatus==COMMENT_READY) DeleteComments();

  CommentStatus=COMMENT_STORING;
  ptr =(CommentsType*)trsmAlloc(sizeof(CommentsType));
  ptr->comment=StringDup(buf);
  ptr->next   = CommentList;
  CommentList=ptr;

  return ptr->comment;
}

static int putnchar(int ch,int n) {
  for (;n>0;n=n-1) putchar(ch);
  return ch;
}

static char * TimeString(double dTime) {
  time_t ltime,htime,mtime,stime;
  static char timestr[]="00:00:00";

  ltime=(time_t) (dTime+0.5);
  htime=ltime/3600;
  if (htime>99) htime=99;
  mtime=(ltime % 3600) /60;
  stime=(ltime % 60);
  sprintf(timestr,"%ld%ld:%ld%ld:%ld%ld",htime/10,htime%10,
          mtime/10,mtime%10,stime/10,stime%10);
  return timestr;
}

static void trsPrintHeader(TRSid_t *ptr,int header) {
  int lt=201,rt=187,lb=200,rb=188,v=186,h=205;

  if (header) {
    lt=218;rt=191;lb=192;rb=217;v=179;h=196;
  }
  putchar(lt);putnchar(h,77);printf("%c\n",rt);
  printf("%c<f>unction  : %-63s%c\n",v,ptr->funcname,v);
  printf("%c<t>est      : %-63s%c\n",v,ptr->testname,v);
  if (header)
     printf("%c<c>lass     : %-63s%c\n",v,ptr->testclass,v);
  else {
     printf("%c<c>lass     : %-43s  %s/",v,ptr->testclass,
            TimeString(difftime(time(NULL),TestTime)));
     printf("%s %c\n",
            TimeString(difftime(time(NULL),TotalTime)),v);
  }
  printf("%c<T>est body : %-44s line: %-12d%c\n",v,ptr->proctext,ptr->line,v);
  printf("%c<S>ource    : %-63s%c\n",v,ptr->file,v);

  if (header) {
    putchar(lb);putnchar(h,77);printf("%c\n",rb);
    fflush(stdout);
  } else {
    printf("%c<R>esult    : ",v);
    if (ResultString) {
      printf("%-55s",ResultString);
      ResultString=(char*)trsFree(ResultString);
    } else
      printf("%-55s"," ");
    switch (TestResult) {
      case TRS_UNDEF: printf("       ");break;
      case TRUE     : printf("     OK");break;
      case FALSE    : printf("   FAIL");break;
      default       : printf("%-7d",TestResult);break;
    }
    printf(" %c\n",v);
  }
}

static void trsPrintFooter(TRSid_t *ptr) {
  CommentsType  *cptr;
  FILE          *oldLstFile=LstFile;
  char           buf[1024],comment[1024];
  int            newline;
  int            trailspaces=77;
  const char    *summary;
  int            printsum;

  summary=trsGetKeyArg('R');
  if (summary)
    printsum=((!strcmp(summary,"OK"  )) && (TestResult==TRS_OK  )) ||
             ((!strcmp(summary,"FAIL")) && (TestResult==TRS_FAIL));
  else
    printsum=TRUE;

  if (printsum) {

    if (SumFile) {
      LstFile=SumFile;
/*      trs_FlushAll();*/
    }

    trsPrintHeader(ptr,0);
    if (CommentStatus!=COMMENT_EMPTY) {

      if (CommentStatus==COMMENT_STORING)
         CommentList=(CommentsType*)Reverse(CommentList);
      CommentStatus=COMMENT_READY;
      cptr=CommentList;

      putchar(199);putnchar(196,77);putchar(182);putchar(LF);

      newline=1;

      while (cptr) {
        if (cptr->comment) {
          StringnCpy(buf,cptr->comment,sizeof(buf));
          while (buf[0]!=0) {
            if (newline) { putchar(186);trailspaces=77;}

            newline=StrParse2(buf,"\n",comment,buf);
            trailspaces=trailspaces-strlen(comment);
            printf("%s",comment);
            if (newline) {
              if (trailspaces>0) putnchar(' ',trailspaces);
              putchar(186);putchar(LF);
            }
          }
        }
        cptr=cptr->next;
      }
    }
    putchar(200);putnchar(205,77);putchar(188);putchar(LF);
/*    trs_FlushAll();*/

    LstFile=oldLstFile;
  }
  trs_FlushAll();
}

static void WrLn(char *str) {
  printf("%s\n",str);
}
static void FHlp(char *str) {
  if (FullHelp) WrLn(str);
}

static int CmdLineHelp(int fullhelp){

  trsMoreSize(Trs_PageSize);
  FullHelp=fullhelp;

  ____("-a name       - ini file with answers");
  ____("");
  WrLn("-B string     - batch mode, substitute all reads from CON by 'string'");
  FHlp("   -B                  - accept default (simulate <CR>)");
  FHlp("   -B\";>\"              - accept default and write to default sections");
  FHlp("   -B\";>[.i486];>[.p5]\"- accept default, write to extended sections");
  FHlp("   -B\";>[lib]\"         - accept default, write to absolute section");
  FHlp("   -B     -D\"*<\"       - data from file, if no data, simulate <CR>");
  FHlp("   -B\"!e\" -D\"*<\"       - data from file, if no data, then exit");
  FHlp("");
  WrLn("-c name       - class, apply command to tests of class 'name'");
  FHlp("");
  FHlp("   test -c ?         - list all classes of tests");
  FHlp("   test -c Algorithm - run all tests with class 'Algorithm'");
  ____("   test -c a%b*f     - all test classes with pattern 'a%b*f'");
  FHlp("");
  WrLn("-C name       - name of .csv file for performance numbers");
  FHlp("");
  WrLn("-d            - DEBUG mode, activate trsWrite(TW_DEBUG,...)");
  FHlp("");
  WrLn("-D string     - define symbols through 'string'");
  FHlp("   -D \"a=1;b=4.3;c=abcd.ext\"");
  FHlp("   -D \"?h;?l;!e\"     - print test help, listing and exit");
  FHlp("   -D \"*<\"           - ?Read from section [funcname.testclass.testbody]");
  FHlp("   -D \"*<[]\"         - ?Read from section [funcname.testclass.testbody]");
  FHlp("   -D \"*<[.p5]\"      - ?Read from ext section [funcname.testclass.testbody.p5]");
  FHlp("   -D \"*<a.ini[lib]\" - ?Read from a.ini, absolute section [lib]");
  FHlp("");
  ____("-e name       - error file is 'name'");
  ____("");
  WrLn("-f name       - apply command to function with name 'name'");
  FHlp("");
  FHlp("   test -f ?       - show functions list");
  FHlp("   test -f libsAdd - run all tests for function 'libsAdd'");
  ____("   test -f %%%Add* - run all tests of function Add");
  FHlp("");
  ____("-g name       - generate answer file");
  ____("   test -f %%%Add* -g %%%Add*    - run all tests of funct Add and generate");
  ____("                                   answer files");
  ____("   test -f %%%Add* -g %%%Add* -d \"*=?\" - run all tests of funct Add,");
  ____("                                   reinitialize all values from console and");
  ____("                                   generate answer files");
  ____("   test -f %%%Add* -g %%%Add* -b - run all tests of funct Add and generate");
  ____("                                   answer files in BLAS style");
  ____("");
  WrLn("-h            - short help");
  FHlp("");
  WrLn("-H            - full help");
  FHlp("");
  WrLn("-i name       - set current ini file name");
  FHlp("");
  ____("-I            - terminate interactive mode");
  ____("-j");
  ____("-k");
  WrLn("-l name       - listing(TW_LST), output to file 'name', default *.lst");
  FHlp("");
  WrLn("-L n          - loop, repeat each test 'n' times");
  FHlp("");
  WrLn("-m n          - more mode, n lines per page, if no 'n', more off,");
  FHlp("                more prompt '--More--' can accept '!q'");
  FHlp("");
  ____("-o name       - output file is 'name'");
  ____("-q level      - quiet mode, report only errors");
  WrLn("-R result     - print to .sum (-s) only tests with result=OK/FAIL");
  FHlp("");
  WrLn("-r n          - RERUN Mode");
  FHlp("");
  FHlp("   test -r    - force RERUN mode, activate trsWrite(TW_RERUN,...)");
  FHlp("   test -r 2  - if test FAIL repeat test 2 times in RERUN Mode");
  FHlp("");
  WrLn("-s name       - summary (TW_SUM), only test results, default *.sum");
  FHlp("");
  WrLn("-S name       - source, apply command to tests in file 'name'");
  FHlp("");
  FHlp("   test -S ?  - show all source files in test system");
  FHlp("   test -S abcd.c  - run all tests in source file 'abcd.c'");
  FHlp("");
  WrLn("-t name       - testname, apply command to functions with testname 'name'");
  FHlp("");
  FHlp("   test -t ?       - show testnames list");
  FHlp("   test -t Test1   - run all tests with name 'Test1'");
  ____("   test -t %%%Add  - test all implementations of Add function");
  FHlp("");
  WrLn("-T name       - testbody, apply command to testbody 'name'");
  FHlp("");
  FHlp("   test -T ?       - show testbodies list");
  FHlp("   test -T trsbody - run all tests with bodyname 'trsbody'");
  FHlp("");
  ____("-u");
  ____("");
  ____("-w");
  ____("-X name       - exec file, apply command to tests in bin file 'name'");
  ____("");
  ____("   test -X ?        - show all bin files in test system");
  ____("   test -X abcd.exe - run all tests in bin file 'abcd.exe'");
  ____("");
  ____("-y");
  ____("-z");
  WrLn("-- string     - any user defined string, key -- is equiv to -Z");
  WrLn("");
  return FALSE;
}

/* ==========================================================================*/
#define KEYARRSIZE 'z'-'?'+1

  static char  *KeyStr[KEYARRSIZE];     /* ==NullStr - set, but no arg       */
                                        /* !=NullStr - set, has arg          */
                                        /* ==NULL    - key is not set        */
  static Bool_t   KeyFlg[KEYARRSIZE];   /* FALSE - value is not available    */
                                        /* TRUE  - value is available        */
  static char   Ch;
  static char  *tmpBuf;
  static char  *cmdBuf;
  static int    tmpBufIdx,        tmpBufLen;
  static int    cmdBufIdx,        cmdBufLen;

/* --- Command line key values ----------------------------------------------*/

TRSFUN(const char *, trsGetKeyArg,(char key)) {

  if (key=='-') key='Z';                        /* convert -- to -Z          */
  if (((key>='a') && (key<='z')) || ((key>='?') && (key<='Z')))
    return KeyStr[key-'?'];
  return NULL;
}

static void KeysWithArg(char *s){
  int    i,len;
  char   ch;

  len=strlen(s);
  for (i=0;i<KEYARRSIZE;i=i+1) {
    KeyFlg[i]=FALSE;
    KeyStr[i]=NULL;
  };
  for (i=0;i<len;i=i+1) {
    ch=s[i];
    if (((ch>='a') && (ch<='z')) || ((ch>='?') && (ch<='Z')))
       KeyFlg[ch-'?']=TRUE;
  };
}

static int  MayHasArg(char key) {
  if ( (((key>='a') && (key<='z')) || ((key>='?') && (key<='Z'))) &&
       (KeyFlg[key-'?']) ) return TRUE;
  return FALSE;
}

static void InvalidKey(char key) {
  switch (key) {
    case ' ': printf("?-%s-Invalid command\n",TestID);break;
    default : printf("?-%s-Invalid key -%c\n",TestID,key);
  };
}

static void ResetKeyArgBuf(char *keyargbuf,int keybuflen){
  tmpBuf=keyargbuf;
  tmpBuf[0]=0;
  tmpBufIdx=0;
  tmpBufLen=keybuflen;
}

static void StoreCh(char ch){

  if (tmpBufIdx<tmpBufLen-1) {
    tmpBuf[tmpBufIdx]=ch;
    tmpBufIdx++;
    if (tmpBufIdx<tmpBufLen)  tmpBuf[tmpBufIdx]=0;
  }
}

static void StoreArg(char key){
  int        idx=key-'?';

  if ((KeyStr[idx]!=NULL) && (KeyStr[idx]!=NullStr))
    free(KeyStr[idx]);
  if (strlen(tmpBuf)==0)
    KeyStr[idx]=NullStr;
  else
    KeyStr[idx]=StringDup(tmpBuf);
}

static void AssignCmd(char *cmd) {
  cmdBufLen=strlen(cmd);
  cmdBuf=cmd;
  cmdBufIdx=0;
}

static void NextCh(void) {
  if (cmdBufIdx<cmdBufLen) {
    cmdBufIdx=cmdBufIdx+1;
    Ch=cmdBuf[cmdBufIdx];
  }
}

static void ParseSwitches(char *cmd){
  enum {PS_INIT,PS_RD_KEY,PS_SKIP_SPACE,PS_RD_QUOTED,PS_RD_SPACED,PS_END}
       state;
  char key=0;
  char keyargbuf[INPBUF_LEN];
  char qmark=0;

  AssignCmd(cmd);
  KeysWithArg("?aBcdDfFghHilLmoqrRsStTxZ");
  state=PS_INIT;
  NextCh();

  while (state!=PS_END) {
    switch (state) {
      case PS_INIT:
        switch (Ch) {
          case ' ': NextCh();break;
          case '-': state=PS_RD_KEY;NextCh();break;
          case EOL: state=PS_END;break;
          default : state=PS_END;InvalidKey(' ');break;
        };
        break;
      case PS_RD_KEY:
        switch (Ch) {
          case ' ': state=PS_INIT;NextCh();break;
          case EOL: /*state=PS_INIT;AssignCmd(cmd);*/
                    state=PS_END;
                    break;
          case '-': Ch='Z';                     /* convert -- to -Z          */
          default : if (((Ch>='a') && (Ch<='z')) ||
                        ((Ch>='?') && (Ch<='Z')))
                    {
                      key=Ch;
                      NextCh();
                      ResetKeyArgBuf(keyargbuf,INPBUF_LEN);
                      if (MayHasArg(key))
                        state=PS_SKIP_SPACE;
                      else
                        StoreArg(key);
                    } else {
                      state=PS_END;InvalidKey(Ch);
                    };
        };
        break;
      case PS_SKIP_SPACE:
        switch (Ch) {
          case ' ' : NextCh();
                     if (Ch=='-') {
                       state=PS_RD_KEY;StoreArg(key);NextCh();
                     };
                     break;
          case EOL : state=PS_END;StoreArg(key);break;
          case SQ  :
          case DQ  : state=PS_RD_QUOTED;qmark=Ch;NextCh();break;
          default  : state=PS_RD_SPACED;
        };
        break;
      case PS_RD_QUOTED:
        if (Ch==qmark) {
          state=PS_SKIP_SPACE;NextCh();
        } else {
          switch (Ch) {
            case EOL : state=PS_END;StoreArg(key);break;
            default  : StoreCh(Ch);NextCh();
          };
        };
        break;
      case PS_RD_SPACED:
        switch (Ch) {
          case EOL : state=PS_END;StoreArg(key);break;
          case SQ  :
          case DQ  : state=PS_RD_QUOTED;qmark=Ch;NextCh();break;
          case ' ' : NextCh();
                     if (Ch=='-') {
                       state=PS_RD_KEY;StoreArg(key);NextCh();
                     } else {
                       state=PS_SKIP_SPACE;StoreCh(' ');
                     };
                     break;
          default  : StoreCh(Ch);NextCh();
        };
        break;
      default:
        state=PS_END;break;
    };
  };
}

static void ParseCmdLine(int argC, char *argV[]){
  int     i;
  char   *p;
  char    ch;
/*  char   *path,*filename

  printf("argC=%d\n",argC);for(i=0;i<argC;i=i+1) printf("%d. '%s'\n",i,argV[i]);
*/
  strcpy(ExePath,argV[0]);
  p=strrchr(ExePath,'.');
  if (p) (*p)=0;
/*  
  path=StringDup(ExePath);
  filename=StringDup(ExePath);

  StrParse(path,"/",2,path,filename);
*/
  strcpy(Cmd,ExePath);
  strcat(Cmd,".sum");
  SumFileName=StringDup(Cmd);

  strcpy(Cmd,ExePath);
  strcat(Cmd,".ini");
  IniFileName=StringDup(Cmd);

  strcpy(Cmd,ExePath);
  strcat(Cmd,".lst");
  LstFileName=StringDup(Cmd);

  strcpy(Cmd,ExePath);
  strcat(Cmd,".csv");
  CsvFileName=StringDup(Cmd);

  strcpy(ExePath,argV[0]);
  p=strrchr(ExePath,CatalogSep);
  if (p) *(p+1)=0;
    else ExePath[0]=0;

  Cmd[0]=0;
  for (i=1; i<argC; i=i+1) {            /* Restore command line              */
    strcat(Cmd," ");
    strcat(Cmd,argV[i]);
  };
  ParseSwitches(Cmd);
  for (ch='?';ch<='z';ch++)
    if (trsGetKeyArg(ch)!=NULL) {
      printf("<%c>:[%s]\n",ch,trsGetKeyArg(ch));
    };
}
/* ==========================================================================*/
static void NL(int i) {if (i) printf("\n");}

static int Mismatch(char ch, char* str) {
  const char *name=trsGetKeyArg(ch);

  if (name) {
    if (name[0]=='?') return FALSE;
    if (strcmp(name, str)) return TRUE;
  }
  return FALSE;
}

static int MismatchFileName(char ch, char* str) {
  const char *name=trsGetKeyArg(ch);
  char       *fname=strrchr(str,'\\');

  if (name) {
    if (name[0]=='?') return FALSE;
    if (!fname) fname=str; else fname++;
#ifdef __BORLANDC__
	 if (stricmp(name, fname)) return TRUE;
#elif defined WIN32
	 if (_stricmp(name, fname)) return TRUE;
#else
	 if (strcasecmp(name, fname)) return TRUE;
#endif
  }
  return FALSE;
}

static int PrintFieldInfo(char ch,char *str){
  const char *name=trsGetKeyArg(ch);
  if ((name) &&(name[0] =='?')) {printf(" [%s]",str); return 1;}
  return 0;
}

static int CompareFields(TRSid_t *ptr){
  int info=0;

  if (Mismatch('f',ptr->funcname)) return FALSE;
  if (Mismatch('t',ptr->testname)) return FALSE;
  if (Mismatch('c',ptr->testclass)) return FALSE;
  if ((Mismatch('S',ptr->file)) && (MismatchFileName('S',ptr->file))) return FALSE;
  if (Mismatch('T',ptr->proctext )) return FALSE;

  info+=PrintFieldInfo('S',ptr->file);
  info+=PrintFieldInfo('c',ptr->testclass);
  info+=PrintFieldInfo('f',ptr->funcname);
  info+=PrintFieldInfo('t',ptr->testname);
  info+=PrintFieldInfo('T',ptr->proctext);

  NL(info);

  if (info) return FALSE;

  return TRUE;
}

static FILE * OpenTstFile(FILE ** file, char **fname, char key,char *mode) {
  char    *keyval;
/*  const char    *keyval;*/

  keyval=(char*)trsGetKeyArg(key);
  if (keyval) {
    if (keyval!=NullStr) {
      if (*fname) free(*fname);
      *fname=StringDup(keyval);
    }
    if (*file) fclose(*file);
    (*file)=fopen(*fname,mode);
  }
  return (*file);
}

static void trsCSVHeader(void);

static int ProcessKeys(void){
  const char    *keyval;
  int            intval;
  int            res;

  keyval=trsGetKeyArg('m');
  if (!keyval) trsMoreSize(DEFAULT_PAGE_SIZE);
  else {
    res=sscanf(keyval,"%u",&intval);
    if (res!=1) intval=0;
    trsMoreSize(intval);
  }

  keyval=trsGetKeyArg('L');
  if (keyval) {
/*    LoopMode=TRUE;*/
    res=sscanf(keyval,"%u",&LoopMax);
    if (res!=1) LoopMax=1;
  }

  keyval=trsGetKeyArg('r');
  if (keyval) {
    ReRunMode=TW_RERUN;
    res=sscanf(keyval,"%u",&ReRunCount);
    if (res!=1) ReRunCount=0;
  }

  if (trsGetKeyArg('d')) DebugMode=TW_DEBUG;

  OpenTstFile(&LstFile,&LstFileName,'l',"w");
  OpenTstFile(&SumFile,&SumFileName,'s',"w");
  OpenTstFile(&CsvFile,&CsvFileName,'C',"w");
  trsCSVHeader();

  keyval=trsGetKeyArg('i');

  if ((keyval) && (keyval[0]))
    IniFileName=reStringDup(IniFileName,keyval);

  if (trsGetKeyArg('?')) return CmdLineHelp(FALSE);
  if (trsGetKeyArg('h')) return CmdLineHelp(FALSE);
  if (trsGetKeyArg('H')) return CmdLineHelp(TRUE);

  return TRUE;
}

static TRSid_t *FindPrev(TRSid_t *current){
  TRSid_t       *ptr,*prev;

  ptr=TestList;prev=TestList;
  while (ptr!=current) {
    prev=ptr;
    ptr=ptr->next;
  };
  return prev;
}

static void trsInit(TRSid_t *ptr) {
  int appnamelen;

  appnamelen=strlen(ptr->funcname)+strlen(ptr->testclass)+
             strlen(ptr->proctext)+3;           /* +.+.+\0                   */
  TestAppName=(char*)trsreAlloc(TestAppName,appnamelen);
  strcpy(TestAppName,ptr->funcname);  strcat(TestAppName,".");
  strcat(TestAppName,ptr->testclass); strcat(TestAppName,".");
  strcat(TestAppName,ptr->proctext);
  trsGuardPattern(0xA55A,4);                       /* Set guardAlloc pattern    */

#ifdef ANYLIB
  iplSetErrStatus(IPL_StsOk);                    /* Set context               */
  iplSetErrMode(IPL_ErrModeParent);
#endif

  TstLastStatus=TRS_StsOk;
  TstStdErrMode=TRS_ErrModeParent;
  srand(1);                                      /* remove tests influence */
}

static void DeleteAppList(void);
static int trsCSVClose(void);

static void trsTerminateAll(void){
  TRSid_t  *ptr;
  int         i;

  DeleteAppList();
  LoadIniFileName=(char*)trsFree(LoadIniFileName);
  DeleteComments();

  if (LstFile) fclose(LstFile);                 /* Close Lst file       */
  if (SumFile) fclose(SumFile);                 /* Close Lst file       */
  if (CsvFile) trsCSVClose();

  while (TestList) {                            /* Release test reg memory   */
    ptr=TestList;
    TestList=TestList->next;
    free(ptr);
  }
  for (i=0;i<KEYARRSIZE;i=i+1)
    if (KeyStr[i]!=NullStr)
      trsFree(KeyStr[i]);

  trsFree(ResultString);
  trsFree(TestAppName);
  trsFree(SumFileName);
  trsFree(IniFileName);
  trsFree(LstFileName);
  trsFree(CsvFileName);
}
/* ==========================================================================*/
TRSFUN(void, trsCommandLine,(int argc, char *argv[])) {
  if (argc||argv) return;               /* for compatibility with timapi.c   */
}


TRSFUN(int, trsRun,(int argC, char *argV[]))
{
  TRSid_t               *ptr;
  int                    FailsFound=0;

#ifdef ANYLIB
  IPLErrorCallBack       oldStdError;
  const IPLLibVersion   *lib;
#endif

  TotalTime=time(NULL);

  ParseCmdLine(argC,argV);

  if (!ProcessKeys()) return TRS_QUIT;

  RunMode=TW_RUN;
  ptr=TestList;

#ifdef ANYLIB
  oldStdError=iplRedirectError(newStdError);
  lib=iplGetLibVersion();

  printf("--- Testing %s,%s,%s,%s,%s ---\n",
         lib->Name,lib->Version,lib->InternalVersion,lib->CallConv,lib->BuildDate);
  if (SumFile) fprintf(SumFile,"--- Testing %s,%s,%s,%s,%s ---\n",
         lib->Name,lib->Version,lib->InternalVersion,lib->CallConv,lib->BuildDate);
#endif

  while (ptr != NULL ) {

    Trs_CurrentTest=ptr;

    if (CompareFields(ptr)) {
      TestTime=time(NULL);
      TestResult=TRS_UNDEF;
      Trs_ReadSource=TRS_None;

      DeleteComments();
      trsPrintHeader(ptr,TRUE);
      Trs_FlowStatus=TRS_CONTINUE;
      trsInit(ptr);
      trs_arInit();
      Terminate=NULL;

      for (LoopCount=0;LoopCount<LoopMax;LoopCount++) {
        if (Trs_FlowStatus==TRS_CONTINUE) {
          if (!ptr->status)                       /* Invoking the test         */
            TestResult=((TRSProc_t)    ptr->test)();
          else
            TestResult=((TRSProcArg_t) ptr->test)(ptr->arg);
          if (Terminate)
             TestResult=Terminate(TestResult);
          if (TestResult==TRS_FAIL)
             FailsFound=1;
        }
      }
      trs_arTerminate();

      WrLn("");
      if (TestResult!=TRS_UNDEF)
         trsPrintFooter(Trs_CurrentTest);

      switch (Trs_FlowStatus) {          /* Some actions with return value     */
        case TRS_RETRY:
                WrLn("-Retrying test...\n");continue;
        case TRS_PREVIOUS:
                WrLn("-Previous test...\n");
                ptr=FindPrev(ptr);
                continue;
        case TRS_TOP:
                WrLn("-Top of list\n");
                ptr=TestList;continue;
        case TRS_QUIT:
                WrLn("-Cancel all tests.\n");break;
        case TRS_CONTINUE:
                WrLn("");
        case TRS_EXIT:
                WrLn("-Next test.\n");break;
        default:
                printf("-Unknown test return status: %d\n",TestResult);
                break;
      };
      if (Trs_FlowStatus==TRS_QUIT) break;
    };
    ptr=ptr->next;                      /* Next test in a list                */
  };

  WrLn("-End testing.\n");

#ifdef ANYLIB
  iplRedirectError(oldStdError);
#endif

  trsTerminateAll();
  return FailsFound;
}

/* --- Return Test Phase Status ---------------------------------------------*/
/*
void Error(const char *str, int level){
  printf("%s\n",str);
  exit(level);
}
*/
/*---------------------------------------------------------------*/
/* --- Timing routines : use Win32 high performance Counter -----*/
/* ---               or  use clock function ---------------------*/
/*---------------------------------------------------------------*/

#define TIMER_STOPPED 0
#define TIMER_RUNNING 1
#define USECS 1000000.0

#if TRUE

/* #define DBG_PRINT(s) printf s*/

#define DBG_PRINT(s)

typedef struct _Timer_t {
   int    state;
   double total;
   double start;
} Timer_t;

static Timer_t       Timer[TRS_MAX_TIMER+1]={{0}};
static int           HighPriorityTimerCount=0;

#if defined USE_WIN32_TIMER && defined WIN32

#ifndef WIN32
    //typedef long long LARGE_INTEGER;
#endif

static double getCounter(void){
	static LARGE_INTEGER tick_counts = {{0,0}};

   if (!QueryPerformanceCounter(&tick_counts)) 
      return (double)clock();
	return ((double)tick_counts.u.HighPart * 65536.0 * 65536.0 +
		(double)tick_counts.u.LowPart);
}

static double getFrequency(void)
{
   static int        first_time = TRUE;
   static double     tick_frequency;
   LARGE_INTEGER     cps;

   if (!first_time) return tick_frequency;

   if (!QueryPerformanceFrequency(&cps))  {
      tick_frequency = (double)CLOCKS_PER_SEC;
   } else {
		tick_frequency = (double)cps.u.HighPart * 65536.0 * 65536.0 +
         (double)cps.u.LowPart;
   }
   first_time = FALSE;
   return tick_frequency;
}

TRSFUN(double, trsClocksPerSec,(void)) {return (double)getFrequency()/100.0;}

#else /* use clock() function */

#define getCounter()        (double)clock()
#define getFrequency()      (double)CLOCKS_PER_SEC

TRSFUN(double, trsClocksPerSec,(void)) {return (double)getFrequency();}

#endif


/*****************************************************************/
/* --- Timer routines -------------------------------------------*/
/*****************************************************************/

TRSFUN(void, trsTimerStart,(int No))
{
   if ((No<0) || (No>TRS_MAX_TIMER)) return;

  if ((No>=TRS_MIN_HP_TIMER) &&                 /* Priority Timers Range    */
      (No<=TRS_MAX_HP_TIMER) &&
      (Timer[No].state !=TIMER_RUNNING))        /* Exclude double start     */
   {
      if (HighPriorityTimerCount==0) trsHighPriority();
      HighPriorityTimerCount++;
   }

   Timer[No].state=TIMER_RUNNING;
   Timer[No].total=0.0;
   Timer[No].start=getCounter();
   DBG_PRINT(("\nIn TimerStart/start: %10.2f ", Timer[No].start));
   return;
}

TRSFUN(void, trsTimerContinue,(int No)) {
   
   if ((No<0) || (No>TRS_MAX_TIMER) || 
      (Timer[No].state ==TIMER_RUNNING)) return;

   /* Priority Timers will raise priority */
   if ((No>=TRS_MIN_HP_TIMER) && (No<=TRS_MAX_HP_TIMER)) {
      if (HighPriorityTimerCount==0) trsHighPriority();
      HighPriorityTimerCount++;
   }

   Timer[No].state=TIMER_RUNNING;
   Timer[No].start=getCounter();
   DBG_PRINT(("\nIn TimerStart/start: %10.2f ", Timer[No].start));
   return;
}

TRSFUN(void, trsTimerStop,(int No))
{
   double tempClock = getCounter();
   DBG_PRINT(("\nIn TimerStop/stop : %10.2f ", tempClock));

   if ((No<0) || (No>TRS_MAX_TIMER) || 
      (Timer[No].state==TIMER_STOPPED)) return;

   /* Priority Timers will raise priority */
   if ((No>=TRS_MIN_HP_TIMER) && (No<=TRS_MAX_HP_TIMER)) {
      HighPriorityTimerCount--;
      if (HighPriorityTimerCount==0) trsNormalPriority();
   }

   Timer[No].state=TIMER_STOPPED;
   Timer[No].total=Timer[No].total+tempClock-Timer[No].start;
   DBG_PRINT(("\nIn TimerStop/total: %10.2f ", Timer[No].total));
   return;
}

TRSFUN(double, trsTimerClock,(int No))
{
   double tempClock = getCounter();

   if ((No<0) || (No>TRS_MAX_TIMER)) return 0;
   if (Timer[No].state==TIMER_STOPPED)
      return Timer[No].total;
   else
      return Timer[No].total-Timer[No].start+tempClock;
}

TRSFUN(double, trsTimerSec,(int No))
{
   return trsTimerClock(No) / getFrequency();
}

TRSFUN(double, trsTimerUSec,(int No))
{
   return trsTimerClock(No) / getFrequency() * USECS;
}

#else

typedef struct _Timer_t {
  clock_t       total;
  clock_t       start;
  int           state;
} Timer_t;

  static Timer_t     Timer[TRS_MAX_TIMER+1]={{0}};
  static int         HighPriorityTimerCount=0;

TRSFUN(double, trsClocksPerSec,(void)) {return (double)CLOCKS_PER_SEC;}

TRSFUN(void,  trsTimerStart,   (int No)){

  if ((No<0) || (No>TRS_MAX_TIMER)) return;
  if ((No>=TRS_MIN_HP_TIMER) &&                 /* Priority Timers Range    */
      (No<=TRS_MAX_HP_TIMER) &&
      (Timer[No].state !=TIMER_RUNNING))        /* Exclude double start     */
  {
    if (HighPriorityTimerCount==0)              /* Current Low Priority     */
      trsHighPriority();
    HighPriorityTimerCount++;
  }
  Timer[No].state=TIMER_RUNNING;
  Timer[No].total=0;
  Timer[No].start=clock();
}

TRSFUN(void,  trsTimerStop,   (int No)){
  clock_t     tempClock;

  tempClock=clock();
  if ((No<0) || (No>TRS_MAX_TIMER)) return;
  if (Timer[No].state==TIMER_STOPPED)           /* Exclude double stop      */
    return;
  else {
    if ((No>=TRS_MIN_HP_TIMER) &&               /* Priority Timers Range    */
        (No<=TRS_MAX_HP_TIMER))                  
    {
      HighPriorityTimerCount--;
      if (HighPriorityTimerCount==0)           /* Last used HP Timer        */
        trsNormalPriority();
    }
    Timer[No].state=TIMER_STOPPED;
    Timer[No].total=Timer[No].total+tempClock-Timer[No].start;
  };
}

TRSFUN(void,  trsTimerContinue,(int No)){

   if ((No<0) || (No>TRS_MAX_TIMER) ||
       (Timer[No].state==TIMER_RUNNING)) return;
  
   if ((No>=TRS_MIN_HP_TIMER) && (No<=TRS_MAX_HP_TIMER))  {
      if (HighPriorityTimerCount==0)          /* Low Priority             */
        trsHighPriority();
      HighPriorityTimerCount++;
    }
   Timer[No].state=TIMER_RUNNING;
   Timer[No].start=clock();
}

TRSFUN(double, trsTimerClock, (int No)){
  clock_t     tempClock;

  tempClock=clock();
  if ((No<0) || (No>TRS_MAX_TIMER)) return 0;
  if (Timer[No].state==TIMER_STOPPED)
    return (double)(Timer[No].total);
  else
    return (double)(Timer[No].total-Timer[No].start+tempClock);
}

TRSFUN(double, trsTimerSec,(int No)) {

  return (double) trsTimerClock(No) / (double) CLOCKS_PER_SEC;
}
#endif

/*****************************************************************/
/* --- End of timer routines ------------------------------------*/
/*****************************************************************/


/* ==========================================================================*/

TRSFUN(int, trsScaleInit,(int start,int end,int step,int steptype,TRSScale_t *scale)){

  switch (steptype) {
    case TRS_STEP_MPY: if  (step==-1) return 0;break;
    case TRS_STEP_DIV: if ((step==-1) || (step==0)) return 0; break;
    case TRS_STEP_ADD: if ((end>start) && (step<0)) return 0;
                       if ((end<start) && (step>0)) return 0;
                       break;
  };
  scale->start    = start;
  scale->end      = end;
  scale->step     = step;
  scale->steptype = steptype;
  scale->current  = start;
  scale->init     = 0;
  return 1;
}

TRSFUN(int, trsScaleNextPoint,(TRSScale_t *scale, int *next)) {

  (*next) =scale->current;

  if (scale->init) {
    switch (scale->steptype) {
      case TRS_STEP_MPY:  (*next) *= scale->step;break;  /* Multiplicative step  */
      case TRS_STEP_ADD:  (*next) += scale->step;break;  /* Additive step        */
      case TRS_STEP_DIV:  (*next) /= scale->step;break;  /* Div step             */
    }

    if ((*next)==scale->current) return 0;           /* Abort infinite Loops */
  } else
    scale->init=1;

  if ((scale->end >= scale->start) && 
      (((*next) > scale->end) || ((*next) < scale->start)))
     return 0;                                       /* Out of range         */
  if ((scale->end <= scale->start) && 
      (((*next) < scale->end) || ((*next) > scale->start)))
     return 0;                                       /* Out of range         */

  scale->current=(*next);
  return 1;
}
#if FALSE
void testScale(void) {
   Scale_t VecOrderScale;
   int start, end, step, steptype,n;

   for (steptype=0;steptype<=2;steptype++) {
     for (step=-10;step<10;step++) {
       for (start=-10;start< 10;start++) {
         for (end=-10;end<10;end++) {
           printf("start=%d; end=%d; step=%d; steptype=%d\n", start,end, step, steptype);
           if (trsScaleInit(start,end,step,steptype,&VecOrderScale)) {
             while (trsScaleNextPoint(&VecOrderScale,&n)) {
               printf("%d ",n);
             }
             printf("\n");
           }
         }
       }
     }
   }
}
#endif

/* ==========================================================================*/

TRSFUN(void, trsPrintHelp,(TRSid_t * ptr,char mode,char *name,char *def,char *help))
{
  FILE       *helpfile;
  int        linecnt=0;
  int        ch,och;
  int        blocklev;
  char       helpname[256];

  if (mode)
  {
    if (mode=='?') trsPrintHeader(ptr,TRUE);
    if ((mode=='h') || (mode=='l') || (mode=='L'))
    {
/*      strcpy(helpname,ExePath);*/
      strcpy(helpname,ptr->file);

      helpfile=fopen(helpname,"r");
      if (!helpfile)
        printf("File %s not found",helpname);
      else
      {
        och=0; ch=0;
        if (mode == 'L') {
          while ( (!feof(helpfile)) )  {
            ch=fgetc(helpfile);
            putchar(ch);
          };
        } else {
          linecnt=1;
          while ( (linecnt<=ptr->line) && (!feof(helpfile)) )  {
            och=ch;
            ch=fgetc(helpfile);
            if (ch==LF) linecnt=linecnt+1;
          };
        };

        if (mode=='h') {
          while ( ((och!='/') || (ch!='*')) && (!feof(helpfile)) ) {
            och=ch;
            ch=fgetc(helpfile);
            if (ch==LF) linecnt=linecnt+1;
          };
          och=fgetc(helpfile);ch=fgetc(helpfile);
          while ( ((och!='*')||(ch!='/')) && (!feof(helpfile)) ) {
            putchar(och);
            och=ch;
            ch=fgetc(helpfile);
            if (ch==LF) {
              linecnt=linecnt+1;
            };
          };
        };

        if (mode=='l') {
          while ( (ch!='{') && (!feof(helpfile)) ) {
            ch=fgetc(helpfile);
            if (ch==LF)  linecnt=linecnt+1;
          };
          blocklev=1;

          while ( !feof(helpfile) ) {
            ch=fgetc(helpfile);
            if (ch=='{') blocklev=blocklev+1;
            if (ch=='}') blocklev=blocklev-1;
            if (blocklev==0) break;
            putchar(ch);
            if (ch==LF) {
              linecnt=linecnt+1;
              printf("%3d ",linecnt);
            };
          };
        };
        fclose(helpfile);
      };
    };
    WrLn("");
  } else {
    printf("  %s - %s\n",name,help);
    if (def)
      printf("  default = [%s]\n",def);
    else
      printf("  default = NONE\n");
    printf("  cmd=[%s]\n",helpprompt);
  }
}

/* --- Input ----------------------------------------------------------------*/

#if !defined _TRSREAD_C

TRSFUN(int, CmdWithoutPrefix,(char *buf, char *prompt, char *dstname,
                             char *type,char *def,    char *help)) {

  if (trsGetKeyArg('B')==NULL)
     trsMoreSize(PageSize);
  printf("%s%s, %s",prompt,dstname,type);
  if (def) printf(", [%s]",def);
  if (ReadSource==TRS_NoInFile)
     printf("?> ");
  else
     printf(" > ");

  ReadSource=TRS_TTYInput;
  FlowStatus=TRS_CONTINUE;
  gets(buf);
  for (;;) {
    switch (buf[0]) {
      case  0  : if (!def) {
                   ReadSource=TRS_DefaultNull;
                   FlowStatus=TRS_CONTINUE;
                   return FALSE;
                 }
                 ReadSource=TRS_Default;
                 if (def[0]==0) return RETRY;
                 strcpy(buf,def);
                 continue;
      case '!' :switch (buf[1]) {
                  case 'q': FlowStatus=TRS_QUIT;break;
                  case 'r': FlowStatus=TRS_RETRY;break;
                  case 'p': FlowStatus=TRS_PREVIOUS;break;
                  case 'e': FlowStatus=TRS_EXIT;break;
                  case 't': FlowStatus=TRS_TOP;break;
                  default : return RETRY;
                };break;
      case HELP_PREFIX:
                  trsPrintHelp(CurrentTest,buf[1],dstname,def,help);return RETRY;
      default: return TRUE;
    }
    return FALSE;
  }
}

TRSFUN(int, _trsCaseRead,(int *dst, char *name, char *items,char *def,char *help)){
  char      sep[2]=" ";
  char      buf[INPBUF_LEN];
  char      head[INPBUF_LEN],tail[INPBUF_LEN];
  int       res;
  char*     tname=name+1;

  if ((!dst) || (!items) || (!def) || (!help)) return -1;
  for(;;) {
    sep[0]=items[0];

    StrParse(items,sep,2,head,tail);
    res=CmdWithoutPrefix(buf,"Select ",tname,tail,def,help);
    switch (res) {
      case RETRY : continue;
      case FALSE : return FALSE;
      case TRUE  :
                   for (res=0;tail[0]!=0;res=res+1) {
                     StrParse(tail,sep,2,head,tail);
                     if (!strcmp(buf,head)) {
                       (*dst)=res;FlowStatus=TRS_CONTINUE;
                       return TRUE;
                     }
                   };
                   continue;
    };
  };
}

TRSFUN(int, _trsBitRead,(long int *dst, char *name, char *items,char *def, char *help)){
  char      sep[2]=" ";
  char      buf[INPBUF_LEN],bufhead[INPBUF_LEN];
  char      head[INPBUF_LEN],tail[INPBUF_LEN];
  int       res,bitno;
  long int  bitset=0;
  char*     tname=name+1;

  if ((!dst) || (!items) || (!def) || (!help)) return -1;
  for(;;) {
    sep[0]=items[0];
    StrParse(items,sep,2,head,tail);
    res=CmdWithoutPrefix(buf,"Bits of ",tname,tail,def,help);
    switch (res) {
      case RETRY : continue;
      case FALSE : return FALSE;
      case TRUE  :
                   for (;buf[0]!=0;) {
                     StrParse(buf,sep,2,bufhead,buf);
                     StrParse(items,sep,2,head,tail);
                     res=FALSE;
                     for (bitno=0;tail[0]!=0;bitno=bitno+1) {
                       StrParse(tail,sep,2,head,tail);
                       if (!strcmp(bufhead,head)) {
                         bitset=bitset | (1L<<bitno);
                         res=TRUE;break;
                       }
                     };
                     if (res) continue;
                     break;
                   }
                   if (res) {FlowStatus=TRS_CONTINUE;*dst=bitset;return TRUE;}
                   continue;
    };
  };
}

TRSFUN(int,  _trstRead,(char * str,int size,char *name,char *def,char *help))
{
  char      buf[INPBUF_LEN];
  int       res;

  for(;;) {
    res=CmdWithoutPrefix(buf,prompt,name,"text",def,help);
    switch (res) {
      case RETRY : continue;
      case FALSE : return FALSE;
      case TRUE  : StringnCpy(str,buf,size);
                   FlowStatus=TRS_CONTINUE;
                   return TRUE;
    };
  };
}

TRSFUN(int,  trsPause,(char *def,char *help))
{
  char      buf[INPBUF_LEN];
  int       res;

  for(;;) {
    res=CmdWithoutPrefix(buf,"","Pause",help,def,help);
    switch (res) {
      case RETRY : continue;
      case TRUE  : return TRUE;
      case FALSE : if (ReadSource==TRS_Default) return TRUE;
                   return FALSE;
    };
  };
}

TRSFUN(int,  _trsiRead,(int * n,char *name,char *def,char *help))
{
  char      buf[INPBUF_LEN];
  int       res;
  char*     tname=name+1;

  for(;;) {
    res=CmdWithoutPrefix(buf,prompt,tname,"int",def,help);
    switch (res) {
      case RETRY : continue;
      case FALSE : return FALSE;
      case TRUE  : res=sscanf(buf,"%ld",n);
                   if (res!=1) continue;
                   FlowStatus=TRS_CONTINUE;
                   return TRUE;
    };
  };
}


TRSFUN(int,  _trssRead,(float *A,char *name,char *def,char *help))
{
  char       buf[INPBUF_LEN];
  float      a;
  int        res;
  char*      tname=name+1;

  for(;;) {
    res=CmdWithoutPrefix(buf,prompt,tname,"float",def,help);
    switch (res) {
      case RETRY : continue;
      case FALSE : return FALSE;
      case TRUE  : res=sscanf(buf,"%f",&a);
                   if (res<1) continue;
                   *A=a;
                   FlowStatus=TRS_CONTINUE;
                   return TRUE;
    };
  };
}

TRSFUN(int,  _trsdRead,(double *A,char *name,char *def,char *help))
{
  char       buf[INPBUF_LEN];
  double     a;
  int        res;
  char*      tname=name+1;

  for(;;) {
    res=CmdWithoutPrefix(buf,prompt,tname,"double",def,help);
    switch (res) {
      case RETRY : continue;
      case FALSE : return FALSE;
      case TRUE  : res=sscanf(buf,"%lf",&a);
                   if (res<1) continue;
                   *A=a;
                   FlowStatus=TRS_CONTINUE;
                   return TRUE;
    };
  };
}


TRSFUN(int,  _trssbRead,(float *A,char *name, int n,char *def,char *help))
{
  char             buf[INPBUF_LEN];
  int              res,i;

  if ((n<=0) || (!A)) return FALSE;

  for (i=0;i<n;i=i+1) {
   sprintf(buf,"&%s[%d]",name,i);
   res=_trssRead(&A[i],buf,def,help);
   if (!res) return res;
  };
  return res;
}


TRSFUN(int,  _trsdbRead,(double *A,char *name, int n,char *def,char *help))
{
  char             buf[INPBUF_LEN];
  int              res,i;

  if ((n<=0) || (!A)) return FALSE;

  for (i=0;i<n;i=i+1) {
   sprintf(buf,"&%s[%d]",name,i);
   res=_trsdRead(&A[i],buf,def,help);
   if (!res) return res;
  };
  return res;
}

#endif

/* --- Output ---------------------------------------------------------------*/

TRSFUN(void, _trssPrint,(char *str,char *name,float src)){
  printf("%s%s=(%+.8e)\n",str,name,src);
}
TRSFUN(void, _trsdPrint,(char *str,char *name,double src)){
  printf("%s%s=(%+.17e)\n",str,name,src);
}

TRSFUN(void, _trssbPrint,(char *str, char *name,float *src, int n)){
  int       i;

  if (!src) return;
  for (i=0;i<n; i=i+1) {
    printf("%s%s[%d]=(%+.8e)\n",str,name,i,src[i]);
  };
}

TRSFUN(void, _trsdbPrint,(char *str, char *name,double *src, int n))
{
  int       i;

  if (!src) return;
  for (i=0;i<n; i=i+1) {
    printf("%s%s[%d]=(%+.17e)\n",str,name,i,src[i]);
  };
}

/*****************************************************************/
/* --- Generate special benchmark output file -------------------*/
/*****************************************************************/

static time_t StartTime;

#ifndef WIN32
typedef struct SYSTEM_INFO
{
    int dwProcessorType;
}SYSTEM_INFO;

typedef struct OSVERSIONINFO
{
    int dwPlatformId;
    int dwMajorVersion;
    int dwMinorVersion;
    int dwBuildNumber;
    int dwOSVersionInfoSize;
}OSVERSIONINFO;

#define GetSystemInfo(a)
#define GetVersionEx(a)

#define PROCESSOR_INTEL_386 1
#define PROCESSOR_INTEL_486 2
#define PROCESSOR_INTEL_PENTIUM 3

#define VER_PLATFORM_WIN32s 1
#define VER_PLATFORM_WIN32_NT 2

#endif

static void trsCSVHeader(void) {
  SYSTEM_INFO          SystemInfo;
  OSVERSIONINFO        OsVersion;

#ifdef ANYLIB
  const IPLLibVersion *lib;
#endif

  if (CsvFile) {
    GetSystemInfo(&SystemInfo);

    fprintf(CsvFile,"Header_begin\n");

    fprintf(CsvFile,"  Processor,");
    switch (SystemInfo.dwProcessorType) {
      case PROCESSOR_INTEL_386:     fprintf(CsvFile,"i386\n");break;
      case PROCESSOR_INTEL_486:     fprintf(CsvFile,"i486\n");break;
      case PROCESSOR_INTEL_PENTIUM: fprintf(CsvFile,"Pentium\n");break;
      default:                      fprintf(CsvFile,"Other\n");break;
    }

    OsVersion.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
    GetVersionEx(&OsVersion);

    fprintf(CsvFile,"  OS,Windows");
    switch (OsVersion.dwPlatformId) {
      case VER_PLATFORM_WIN32s:  fprintf(CsvFile,"32s on Windows");break;
      case VER_PLATFORM_WIN32_NT:fprintf(CsvFile,"NT");break;
      default:                   fprintf(CsvFile,"95");break;
    }

    fprintf(CsvFile," v%1ld.%1ld build %1ld\n",
                    OsVersion.dwMajorVersion,OsVersion.dwMinorVersion,
                    OsVersion.dwBuildNumber);
#ifdef ANYLIB
    lib=iplGetLibVersion();
    fprintf(CsvFile,"  Library,\"%s,%s,%s,%s,build %s\"\n",
                            lib->Name,lib->Version,lib->InternalVersion,
                            lib->CallConv,lib->BuildDate);
#endif

    fprintf(CsvFile,"  Start_Date,\n");
    time(&StartTime);
    fprintf(CsvFile,"  Start_Time,%s",ctime(&StartTime));
    fprintf(CsvFile,"Header_end\n");
    fprintf(CsvFile,"Data_begin\n");
    fprintf(CsvFile,"  Flavour,Function,Value,Units,Comments,Param1,Param2,Param3,Param4\n");
  }
}

static void trsCSVFooter(void) {
  time_t endTime;
  double elapsedTime;

  if (CsvFile) {
    fprintf(CsvFile,"Data_end\n");
    fprintf(CsvFile,"Footer_begin\n");
    fprintf(CsvFile,"  End_Date,\n");
    time(&endTime);
    elapsedTime=difftime(endTime,StartTime);
    fprintf(CsvFile,"  End_Time,%s",ctime(&endTime));
    fprintf(CsvFile,"  Elapsed_Time,%.1f\n",elapsedTime);
    fprintf(CsvFile,"Footer_end\n");
  }
}

TRSFUN(void, trsCSVString9,(char *flavour, char *function,char *value,
                           char *units,   char *comments,char *param1,
                           char *param2,  char *param3,  char *param4)) {
  if (CsvFile) {
    if (!units)    units="us";
    if (!comments) comments="";
    if (!param1)   param1="";
    if (!param2)   param2="";
    if (!param3)   param3="";
    if (!param4)   param4="";

    fprintf(CsvFile,"  %s,%s,%s,%s,%s,%s,%s,%s,%s\n",
            flavour,function,value,units,comments,
            param1,param2,param3,param4);
    fflush(CsvFile);
  }
}

TRSFUN(void, trsCSVString10,(char *flavour, char *function,char *value,
                           char *units,   char *comments,char *param1,
                           char *param2,  char *param3,  char *param4,
                           char *param5)) {
  if (CsvFile) {
    if (!units)    units="us";
    if (!comments) comments="";
    if (!param1)   param1="";
    if (!param2)   param2="";
    if (!param3)   param3="";
    if (!param4)   param4="";
    if (!param5)   param5="";

    fprintf(CsvFile,"  %s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
            flavour,function,value,units,comments,
            param1,param2,param3,param4,param5);
    fflush(CsvFile);
  }
}

TRSFUN(void, trsCSVString11,(char *flavour, char *function,char *value,
                           char *units,   char *comments,char *param1,
                           char *param2,  char *param3,  char *param4,
                           char *param5,  char *param6)) {
  if (CsvFile) {
    if (!units)    units="us";
    if (!comments) comments="";
    if (!param1)   param1="";
    if (!param2)   param2="";
    if (!param3)   param3="";
    if (!param4)   param4="";
    if (!param5)   param5="";
    if (!param6)   param6="";

    fprintf(CsvFile,"  %s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
            flavour,function,value,units,comments,
            param1,param2,param3,param4,param5,param6);
    fflush(CsvFile);
  }
}


TRSFUN(void, trsCSVString5,(char *flavour,char *function,char *value,
                           char *units,  char *comments)) {
  trsCSVString9(flavour,function,value,units,comments,
               NULL,NULL,NULL,NULL);
}
TRSFUN(void, trsCSVString6,(char *flavour,char *function,char *value,
                           char *units,  char *comments,char *param1) {
  trsCSVString9(flavour,function,value,units,comments,
               param1,NULL,NULL,NULL));
}
TRSFUN(void, trsCSVString7,(char *flavour,char *function,char *value,
                           char *units,  char *comments,
                           char *param1, char *param2)) {
  trsCSVString9(flavour,function,value,units,comments,
               param1,param2,NULL,NULL);
}
TRSFUN(void, trsCSVString8,(char *flavour,char *function,char *value,
                           char *units,  char *comments,
                           char *param1, char *param2,  char *param3)) {
  trsCSVString9(flavour,function,value,units, comments,
               param1,param2,param3,NULL);
}

#define BUF_NO 9

static char buf[BUF_NO][64];
static int  buf_idx=0;

static void Next_buf_idx(void) {
  buf_idx=(buf_idx+1)%BUF_NO;
}

TRSFUN(char *, trsFloat,(float src)) {
  Next_buf_idx();
  sprintf(buf[buf_idx],"%e",src);
  return buf[buf_idx];
}

TRSFUN(char *, trsShort,(short src)) {
  Next_buf_idx();
  sprintf(buf[buf_idx],"%hd",src);
  return buf[buf_idx];
}

TRSFUN(char *, trsDouble,(double src)) {
  Next_buf_idx();
  sprintf(buf[buf_idx],"%e",src);
  return buf[buf_idx];
}

TRSFUN(char *, trsInt,(int src)) {
  Next_buf_idx();
  sprintf(buf[buf_idx],"%ld",src);
  return buf[buf_idx];
}

/* ------------------------------------------------------------------------*/
static int trsCSVClose(void)
{
  trsCSVFooter();
  if (CsvFile == NULL) return 0;
  fclose(CsvFile);
  CsvFile = NULL;
  return 0;
}

/* ==========================================================================*/

TRSFUN(void, trs_FlushAll,(void)) {

  if (LstFile) fflush(LstFile);
  if (SumFile) fflush(SumFile);
  fflush(stderr);
  fflush(stdout);

}

#undef putchar
#undef printf
#undef fprintf
#undef gets

static void More(void);

static void Printf(char * string) {
  int i;

  if (string)
    for (i=0;string[i]!=0;i=i+1) {
      if (string[i]=='\n')
        CurrLine=CurrLine+1;
      putchar(string[i]);
      More();
    }
}

static void More(void) {
  char        buf[80];

  if ((Trs_PageSize) && (CurrLine>Trs_PageSize)) {

    trs_FlushAll(); 
    printf("%s","--More-- ");

    gets(buf);
    if (!strcmp(buf,"!q")) exit(0);
    CurrLine=1;

  }
}

TRSFUN(int, putchar_Prot,(int ch)){

  if (LstFile)
    return fputc(ch,LstFile);
  return 0;
}

TRSFUN(int, printf_Prot,(const char *format,...)) {
  va_list   argptr;
  int       res;

  if (LstFile) {
    va_start(argptr,format);
    res=vfprintf(LstFile,format,argptr);
    va_end(argptr);
    return res;
  }
  return 0;
}

TRSFUN(int, _putchar,(int ch)){
  int res;
  putchar_Prot(ch);
  if (ch=='\n')  CurrLine=CurrLine+1;
  res=fputc(ch,stdout);
  More();
  return res;
}

TRSFUN(int, _printf,(const char *format,...)) {
  va_list   argptr;
  int       res;
  char      buff[1024];

  va_start(argptr,format);
  res=vsprintf(buff,format,argptr);
  va_end(argptr);

  Printf(buff);
  if (LstFile)
    fprintf(LstFile,"%s",buff);
  return res;
}

#define TW_SUMMASK 0x000F

TRSFUN(int, trsWrite,(int flag,const char *format,...)) {
  va_list   argptr;
  int       res;
  char      buff[1024];

  va_start(argptr,format);
  res=vsprintf(buff,format,argptr);
  va_end(argptr);

  if ((TW_RUN  | TW_RERUN | TW_DEBUG )  &
      (RunMode | ReRunMode| DebugMode)  &
      (            flag             )) {

    if (flag&TW_CON) 
      Printf(buff);

    if ((LstFile) && (flag&TW_LST))
      fprintf(LstFile,"%s",buff);

    if (flag&TW_SUM)
      trsComment("%s",buff);
  }

  return res;
}

TRSFUN(int, _fprintf,(FILE *st,...)) {
  va_list     argptr;
  char       *format;
  int         res;
  char        buff[1024];

  va_start(argptr,st);
  format=va_arg(argptr,char *);
  res=vsprintf(buff,format,argptr);
  va_end(argptr);

  if ((st==stdout) || (st==stderr)) {
    Printf(buff);
    if (LstFile)
      fprintf(LstFile,"%s",buff);
  } else
    fprintf(st,"%s",buff);

  return res;
}

TRSFUN(char *, _gets,(char* buf)){
  char       *res=NULL;                 /* =NULL for bcpro only */
  const char *batchcmd;

  if (!res) trs_FlushAll();
  batchcmd=trsGetKeyArg('B');
  if (batchcmd) {
     strcpy(buf,batchcmd);
     res=buf;
     CurrLine=CurrLine+1;
     More();
     printf("%s\n",buf);
  } else
     res=gets(buf);
  if (res) printf_Prot("%s\n",buf);
  return res;
}


TRSFUN(int, trsResult,(int result, const char *format,...)){
                                        /*return OK with printf(format,...)  */
  va_list   argptr;
  char      buff[1024];

  if (format) {
    va_start(argptr,format);
    vsprintf(buff,format,argptr);
    va_end(argptr);

    ResultString=reStringDup(ResultString,buff);
  } else 
    ResultString=(char*)trsFree(ResultString);

  return result;
}

/* ==========================================================================*/

typedef struct _KeyListType {
  struct _KeyListType  *next;
  char                 *keyname;
  char                 *keyvalue;
} KeyListType;

typedef struct _AppListType {
  struct _AppListType *next;
  char                *appname;
  KeyListType         *keylist;
} AppListType;

  static AppListType * AppList=NULL;

/* ========================================================================= */
/* return                                                                    */
/*   (*scan) == NULL, search failed, scan - where to insert new App          */
/*   (*scan) != NULL, success, (*scan) points to struct found                */

static AppListType **FindApp(AppListType **scan, char * AppName) {

  if (!AppName) return scan;
  while (*scan) {
    if (!strcmp((*scan)->appname,AppName)) break;
    scan=&((*scan)->next);
  }
  return scan;
}

/* ------------------------------------------------------------------------- */
/* return                                                                    */
/*   (*scan) == NULL, search failed, scan - where to insert new Line         */
/*   (*scan) != NULL, success, (*scan) points to struct found                */

static KeyListType **FindKey(KeyListType **scan, char * KeyName) {

  if (!KeyName) return scan;
  while (*scan) {
    if (!strcmp((*scan)->keyname,KeyName)) break;
    scan=&((*scan)->next);
  }
  return scan;
}

/* ========================================================================= */

static KeyListType **DeleteKey(KeyListType **scan) {
  KeyListType *del;

  del=(*scan);
  if (del) {
    (*scan)=del->next;
    trsFree(del->keyname);
    trsFree(del->keyvalue);
    trsFree(del);
  }
  return scan;
}

static AppListType **DeleteApp(AppListType **scan) {
  AppListType  *del;
  KeyListType **dellist;

  del=(*scan);
  if (del) {
    (*scan)=del->next;
    trsFree(del->appname);
    dellist=&(del->keylist);
    while (*dellist)
      dellist=DeleteKey(dellist);
    trsFree(del);
  }
  return scan;
}

static void DeleteAppList(void) {
  AppListType  **delapp;

  delapp=&AppList;
  while (*delapp)
    delapp=DeleteApp(delapp);
}

/* ========================================================================= */

static void NewKey(KeyListType ** scan, char * KeyName, char * KeyValue) {
  KeyListType *newline;

  scan=FindKey(scan, KeyName);

  if (!(*scan) && (KeyValue)) {                 /* Create new key entry      */
    newline=(KeyListType*)trsmAlloc(sizeof(KeyListType));
    newline->next=NULL;
    newline->keyname=StringDup(KeyName);
    newline->keyvalue=StringDup(KeyValue);
    (*scan)=newline;
  } else {
    if (KeyValue) {                             /* Substitute old value      */
      if (*KeyValue) {
        (*scan)->keyvalue=(char*)trsreAlloc((*scan)->keyvalue,strlen(KeyValue)+1);
        strcpy((*scan)->keyvalue,KeyValue);
      }
    } else                                      /* Delete entry              */
      DeleteKey(scan);
  }
}

static void NewApp(AppListType **scan, char *AppName, char *KeyName, char *KeyValue) {
  AppListType *newapp;

  scan=FindApp(scan, AppName);

  if (!(*scan)) {                               /* Create new app entry      */
    newapp=(AppListType*)trsmAlloc(sizeof(AppListType));
    newapp->next=NULL;
    newapp->appname=StringDup(AppName);
    newapp->keylist=NULL;
    (*scan)=newapp;
  }
  NewKey(&((*scan)->keylist),KeyName,KeyValue);
}
/* ========================================================================= */

static int LoadAppList(char * FileName) {
  FILE         *inifile;
  char         *buf;
  char         *temp, *name, *value;
  char         *appname=NULL;
  int           res;
  int           bufsize=64;
  int           i,nsep;
  int           cr=0;
  AppListType **appscan;

  if (LoadIniFileName) {
    if (!strcmp(LoadIniFileName,FileName))
      return TRUE;
    else {
      DeleteAppList();
      LoadIniFileName=(char*)trsFree(LoadIniFileName);
    };
  };

  inifile=fopen(FileName,"rt");

  if (inifile) {
    LoadIniFileName=StringDup(FileName);

    buf    = (char*)trsmAlloc(bufsize);
    temp   = (char*)trsmAlloc(bufsize);
    name   = (char*)trsmAlloc(bufsize);
    value  = (char*)trsmAlloc(bufsize);

    appscan=&AppList;
    for (;!feof(inifile);) {
      for (i=0;;)  {
        res=fgetc(inifile);
        if ((res==LF) || (res==EOF)) {          /* end of line               */
          if (cr) i=i-1;                        /* if sequential <cr><lf>    */
          buf[i]=0;break;                       /* EOLN                      */
        }
        buf[i]=(char)res;
        cr=(res==CR);
        i=i+1;
        if (i<bufsize) continue;
        bufsize=bufsize*2;
        buf    = (char*)trsreAlloc(buf,   bufsize);    /* duplicate buf              */
        if (!buf) exit(0);                      /* not implemented yet        */
        temp   = (char*)trsreAlloc(temp,  bufsize);
        name   = (char*)trsreAlloc(name,  bufsize);
        value  = (char*)trsreAlloc(value, bufsize);
      }
      SpaceCut(buf);
      nsep=StrParse2(buf,"[",temp,name);
      i=SpaceCut(name);
      if ((nsep==1) && (temp[0]==0) &&          /* [appname] found           */
          (name[0]!=0) && name[i-1]==']') {     /* last sym in name is ']'   */
        trsFree(appname);
        name[i-1]=0;                            /* remove ']'                */
        appname=StringDup(name);
      } else {
        if (strlen(buf) >0 ) {
          StrParse2(buf,"=",name,value);
          NewApp(appscan, appname, name, value);
/*          i=strlen(buf);*/
        }
      }
    }
    trsFree(buf);
    trsFree(value);
    trsFree(name);
    trsFree(temp);
    trsFree(appname);
    fclose(inifile);
    return TRUE;
  }

  return FALSE;
}

static int SaveAppList(char * FileName) {
  FILE        *inifile;
  AppListType *appscan=AppList;
  KeyListType *keyscan;

  inifile=fopen(FileName,"wt");
  if (inifile) {
    while (appscan) {
      if ((appscan->appname) && (appscan->appname[0]!=0))
        fprintf(inifile,"\n[%s]\n\n",appscan->appname);
      keyscan=appscan->keylist;
      while (keyscan) {
        fprintf(inifile,"%s",keyscan->keyname);
        if (keyscan->keyname[0]!=';') fprintf(inifile,"=");
        fprintf(inifile,"%s\n",keyscan->keyvalue);
        keyscan=keyscan->next;
      }
      appscan=appscan->next;
    }
    fclose(inifile);
    return TRUE;
  }
  return FALSE;
}
/* ========================================================================= */

static int trsGetPrivateProfileString(char *AppName,char *KeyName,char *Default,
                            char *Return, int   nSize,  char *FileName){
  int             res = 0;
  AppListType   **findapp;
  KeyListType   **findline;

  if (LoadAppList(FileName)) {
    findapp=FindApp(&AppList,AppName);
    if (*findapp) {
      findline=FindKey(&((*findapp)->keylist),KeyName);
      if (*findline)
        res=StringnCpy(Return,(*findline)->keyvalue,nSize);
    }
#if FALSE
    DeleteAppList();
#endif
  }

  if (!res)
    res=StringnCpy(Return,Default,nSize);
  return res;
}

static int trsWritePrivateProfileString(char *AppName,char *KeyName,
                              char *String, char *FileName){
  int             res = 0;
  AppListType   **findapp;

  LoadAppList(FileName);

  findapp=FindApp(&AppList,AppName);
  if (*findapp) {
    if (KeyName)                              /* Create or delete key entry*/
      NewKey(&((*findapp)->keylist),KeyName,String);
    else                                      /* Delete App entry          */
      DeleteApp(findapp);
  } else
    NewApp(findapp,AppName,KeyName,String);

  if (SaveAppList(TmpIniFileName)) {
    remove(FileName);
    rename(TmpIniFileName,FileName);
  }
#if FALSE
  DeleteAppList();
#endif
  return res;
}

/* ==========================================================================*/
#if FALSE
static int GetProfileString(char *AppName,char *KeyName,char *Default,
                     char *Return, int   nSize) {
  return GetPrivateProfileString(AppName,KeyName,Default,
                                 Return,nSize,IniFileName);
}

static int WriteProfileString(char *AppName,char *KeyName,char *String) {
  return WritePrivateProfileString(AppName,KeyName,String,IniFileName);
}
#endif

/* ==========================================================================*/

static char  File[INPBUF_LEN];
static char  App[INPBUF_LEN];
static char  Key[INPBUF_LEN];

TRSFUN(int, ParseFileAppKey,(char *string,char *srcname)) {
   char  extraapp[INPBUF_LEN]="";
   int   i;

   i=BracketsParse(string,'[',']',File,extraapp,Key);
   if (i) return FALSE;
   StringnCpy(Trs_String,string,sizeof(Trs_String));

   if (File[0]==0) StringnCpy(File,IniFileName,sizeof(File));
   if (Key[0]==0)
     StringnCpy(Key,srcname,sizeof(Key));
   else
     strcpy(srcname,Key);

   StringnCpy(App,TestAppName,sizeof(App));
   if (extraapp[0]=='.')
     strncat(App,extraapp,sizeof(App));
   else
    if (extraapp[0]!=0) StringnCpy(App,extraapp,sizeof(App));

   return TRUE;
}
TRSFUN(char*, GetDstFromFile,(char *string,char *srcname)) {
  static char value[1024];
  char *helpptr;

  if (!ParseFileAppKey(string,srcname)) return NULL;
  trsGetPrivateProfileString(App,Key,"",value,sizeof(value),File);
  if (value[0]==0) return NULL;

  helpptr=strstr(value,"//");
  if (helpptr) (*helpptr)=0;
  SpaceCut(value);
  return value;
}

TRSFUN(void, PutToFile,(char *key,char *value, char *help)) {
  char valueandhelp[1024];
  int   len,spaces;
  char *charptr;

  StringnCpy(valueandhelp,value,sizeof(valueandhelp));
  len=strlen(key)+strlen(valueandhelp)+1;
  if (len<24) spaces=(24-len);
    else spaces=TABSIZE-(len % TABSIZE);

  while (spaces--) strcat(valueandhelp," ");
  strcat(valueandhelp,"//");

  if (help) {
    len=strlen(valueandhelp);
    StringnCpy(&valueandhelp[len],help,sizeof(valueandhelp)-len);
    charptr=strchr(valueandhelp,'\n');          /* Get only fist line of help*/
    if (charptr) (*charptr)=0;
  }

  trsWritePrivateProfileString(App,key,valueandhelp,File);
}


/* ************************************************************************* */
#ifdef WIN32
#define RDTSC( addr ) __asm                       \
{                                                 \
    __asm _emit 0x0f __asm _emit 0x31 /* RDTSC */ \
    __asm mov dword ptr addr, eax                 \
    __asm mov dword ptr addr + 4, edx             \
}
#define Int64 __int64
#else
#define RDTSC(addr)
    typedef long long Int64;
#endif

double GetTime( trsProcessorInfo* proc_info )
{
    Int64 val;
    RDTSC( val );
    return val * proc_info->time_scale;
}

/* determine type of processor, frequency and memory size */
trsProcessorInfo InitProcessorInfo()
{
    int bCPUID = 0;
    int features, val;

    trsProcessorInfo proc_info;

    proc_info.time_scale = proc_info.family = proc_info.frequency = 0;
#ifdef WIN32
    __asm
    {
        /* check CPUID support */
        pushfd
        pushfd
        pop     eax
        mov     edx, eax
        xor     eax, (1<<21)
        push    eax
        popfd
        pushfd
        pop     eax
        xor     ecx, ecx
        cmp     eax, edx
        setne   cl
        mov     bCPUID, ecx
        popfd
    }

    assert( bCPUID );

    __asm push  ebx
    __asm mov   eax, 1
    __asm _emit 0x0f __asm _emit 0xa2 /* CPUID */
    __asm mov   val, eax
    __asm mov   features, edx
    __asm pop   ebx
#endif
    proc_info.family = ((val >> 8)&15) | ((features & (1<<23))<<8);

    if( features & (1<<4) ) /* RDTSC support */
    {
        Int64 clocks1, clocks2;
        volatile int t;
        int dt = 500;
        int frequency = 0, old_frequency;

        do
        {
            old_frequency = frequency;
            t = clock();
            while( t==clock() );
            t = clock();

            RDTSC( clocks1 );
            while( dt+t>clock() );
            RDTSC( clocks2 );

            frequency = (int)(((double)(clocks2 - clocks1))/(1e3*dt)+.5) + 10;
            if( frequency % 50 <= 16 )
            {
                frequency = (frequency/50)*50;
            }
            else
            {
                frequency = (frequency/100)*100 + ((frequency % 100)/33)*33;
            }
        }
        while( frequency != old_frequency );
        proc_info.frequency  = frequency;
        proc_info.time_scale = 1.e-6/frequency;
    }

    return proc_info;
}

/* ==========================================================================*/
