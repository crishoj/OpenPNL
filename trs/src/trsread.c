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

#include <stdlib.h>
#include <string.h>
#include <time.h>

#if !defined (TRS_BUILD)
# define TRS_BUILD
#endif

#include "trsapi.h"
/* -------------------------------------------------------------------------*/
#if defined putchar
# undef putchar
#endif

#define putchar _putchar
#define printf  _printf
#define fprintf _fprintf
#define gets    _gets

/* --- Input ----------------------------------------------------------------*/

#if defined _TRSREAD_C

#define BUFLEN            400
#define COMMONLEN         INPBUF_LEN+BUFLEN

typedef  struct _InpValueType{
  struct _InpValueType   *next;                 /* next value in a list      */
  char                   *valname;              /* value name                */
  char                   *value;                /* value in string           */
  TRSfvr_t                 valtp;                /* value type                */
  void                   *extra;                /* size or items             */
  char                   *valhelp;              /* value size                */
  void                   *val_adr;              /* value address             */
} InpValueType;

typedef struct _InpListType{
  struct _InpListType     *next;
  char                    *inputdst;
  char                    *inputsrc;
  int                      lorgflg;
} InpListType;

typedef struct _TokenListType{
  struct _TokenListType   *next;
  char                    *token;
} TokenListType;

/* ------------------------------------------------------------------------- */

  static  InpListType           *InputList=NULL;
  static  InpValueType          *ValueList=NULL;
  static  TokenListType         *TokenList=NULL;
  static  int                    SrcFromFile=0;         /* src from file             */
  static  int                    DstToFile=0;           /* dst to file               */
  static  char  *TypeName[] =
          {"float","SCplx","double","DCplx","short","WCplx","int","ICplx","long",
           "text","",""};
  static  char  *TypePrompt[] =
          {"Enter ","Enter ","Enter ","Enter ","Enter ","Enter ","Enter ","Enter ","Enter ",
           "Enter ", "Select ","Set bits of "};
/* ------------------------------------------------------------------------- */

static void DeleteValueList   (void);
static void DeleteInpListEntry(InpListType *current);
static void DeleteTokenList   (void);
static void ErrorMess         (char *msg);
static int FillDst         (char *buf,void *dst,void *extra,TRSfvr_t dt);

static int ParseInput      (void *dst,  char *tname,char *buf,TRSfvr_t dsttype,
                            void *extra,char *help,int ifl);
static int ParseTokens     (char *buf,  char *delimiter);
static int ParseTokenAssign(char *name, char *cptr,void *dst,TRSfvr_t dsttype,
                            void *extra,char *help,int ifl);

static InpListType  *ScanInpList    (char *name);
static InpValueType *ScanValueList  (char *name);
static InpValueType *AddInpValueList(char *name,char *dst,TRSfvr_t dsttype,
                              void *d_adr,void *extra,char *help);
static int AssignValue(int valfromlist,InpValueType *currentvalue,char *value,
                char *valuename,void *dst,TRSfvr_t dsttype,
                void *extra,char *help,int inpmode);

/* ========================================================================= */

static void cmdAct(char *buf) {

  switch (buf[1]) {
    case 'q': Trs_FlowStatus=TRS_QUIT;break;
    case 'r': Trs_FlowStatus=TRS_RETRY;break;
    case 'p': Trs_FlowStatus=TRS_PREVIOUS;break;
    case 'e': Trs_FlowStatus=TRS_EXIT;break;
    case 't': Trs_FlowStatus=TRS_TOP;break;
    default : Trs_FlowStatus=TRS_CONTINUE;
  };
}

/* ------------------------------------------------------------------------- */

static void DeleteInpList(void) {

  InpListType   *ptr;

  while (InputList) {
    ptr=InputList;
    free(ptr->inputsrc);    free(ptr->inputdst);
    InputList=ptr->next;    free(ptr);
  };
  return;
}

static void DeleteValueList(void) {

  InpValueType      *ptr;

  while (ValueList) {
    ptr=ValueList;
    free(ptr->value);    free(ptr->valname);    free(ptr->valhelp);
    if ((ptr->valtp==case_fvr)||(ptr->valtp==bit_fvr)) free(ptr->extra);
    ValueList=ptr->next;
    free(ptr);
  };
  return;
}

TRSAPI(int, trs_arInit,(void)) {

  const char  *chptr;
  char         buf[INPBUF_LEN];


  chptr=trsGetKeyArg('D');
  if ((chptr==NULL) || (*chptr==0)) return TRUE;
  strcpy(buf,chptr);
  if(!ParseInput(NULL,"",buf,i_fvr,NULL,"",1)) return FALSE;
  return TRUE;
}

TRSAPI(void, trs_arTerminate,(void)) {

  DeleteValueList();
  DeleteInpList();
  DstToFile=FALSE;
  SrcFromFile=FALSE;
  return;
}

/* Find value entry with valuename                                           */

static InpValueType *ScanValueList(char *name){

  static InpValueType      *ptr=NULL;

  ptr=ValueList;
  while (ptr) {
    if (!strcmp(ptr->valname,name)) break;
    ptr=ptr->next;
  };
  return ptr;
}

/* Find input entry with inputdstname, if global and local take local        */

static InpListType *ScanInpList(char *name) {

  static InpListType     *ptr=NULL;
         InpListType     *keyinputvalue;

  ptr=InputList;
  while (ptr) {                                 /*scan InputList             */
    if (!strcmp(ptr->inputdst,name)) break;
    ptr=ptr->next;
  };
  if (!ptr) return ptr;                        /* no such entry              */
  if (!ptr->lorgflg) return ptr;               /* entry not global -  o'key  */
  keyinputvalue=ptr;                           /* if global - save ptr       */
  ptr=ptr->next;                               /* take next entry            */
  while (ptr) {                                /* scan for local entry       */
    if (!strcmp(ptr->inputdst,name)) break;    /* if local - return it ptr   */
    ptr=ptr->next;                             /* get next entry             */
  };
  if(!ptr)ptr=keyinputvalue;                   /* no local - get global ptr  */
  return ptr;
}

/* Delete local input entry with pointer                                     */

static void DeleteInpListEntry(InpListType *current){

  InpListType     *ptr,*prev;

  if((current==NULL) || (InputList==NULL)) return;
  ptr=InputList;prev=ptr;
  while (ptr!=current) {                       /* find entry                 */
    prev=ptr;                                  /* get previos                */
    ptr=ptr->next;
    if (!ptr) return;
  };
  if (ptr->lorgflg) return;                    /* if global - do nothing     */
  prev->next=ptr->next;                        /* if local - put next to prev*/
  if (ptr==InputList) InputList=ptr->next;
  free(ptr->inputsrc);                         /* delete entry               */
  free(ptr->inputdst);
  free(ptr);
  return;
}

static void AddInputListEntry(char *dst,char *src,int inplstflg) {

  InpListType     *iptr,*prev;

  iptr=ScanInpList(dst);
  if( (!iptr) || ((iptr!=NULL) && (iptr->lorgflg==1)) ) {
    iptr=(InpListType*)trsmAlloc(sizeof(InpListType));
    iptr->inputdst=StringDup(dst);
    iptr->inputsrc=StringDup(src);
    iptr->next=NULL;
    iptr->lorgflg=inplstflg;
    if (InputList==NULL)  InputList=iptr;
    else {
      prev=InputList;
      while ((prev->next) !=NULL)prev=prev->next;
      prev->next=iptr;
    };
  } else {
    iptr->inputdst=reStringDup(iptr->inputdst,dst);
    iptr->inputsrc=reStringDup(iptr->inputsrc,src);
    iptr->lorgflg =inplstflg;
  };
}

static InpValueType *AddInpValueList(char *name,char *dst,TRSfvr_t dsttype,
                              void *d_adr, void *extra,char *help) {
  static InpValueType  *ptr;
         InpValueType  *pp;

  ptr=ScanValueList(name);
  if(!ptr) {
    ptr=(InpValueType*)trsmAlloc(sizeof(InpValueType));
    if (ptr==NULL) return ptr;

    ptr->valname = StringDup(name);
    ptr->valhelp = StringDup(help);

    ptr->valtp    = dsttype;
    ptr->val_adr  = d_adr;
    ptr->value   = StringDup(dst);

    switch (dsttype) {
      case text_fvr: ptr->extra=extra;          break;
      case case_fvr:
      case bit_fvr: ptr->extra=(void*)StringDup((char*)extra);  break;
      default:    ptr->extra=NULL;           break;
    };
    ptr->next     = NULL;
    if (ValueList==NULL)
      ValueList=ptr;
    else {
      pp=ValueList;
      while ((pp->next) !=NULL)   pp=pp->next;
      pp->next=ptr;
    };
  } else {


    ptr->valname = reStringDup(ptr->valname,name);
    ptr->valhelp = reStringDup(ptr->valhelp,help);

    ptr->valtp    = dsttype;
    ptr->val_adr  = d_adr;
    ptr->value    = reStringDup(ptr->value,dst);

    switch (dsttype) {
      case text_fvr: ptr->extra=extra;                       break;
      case case_fvr:
      case bit_fvr: ptr->extra=reStringDup((char*)ptr->extra,(char*)extra);  break;
      default:    ptr->extra=NULL;                        break;
    };
  };
  return ptr;
}

/* ==========================================================================*/

static int FillDst(char *buf,void *dst,void *extra,TRSfvr_t dt) {

  int           res=0,bitno;
  long          i;
  float         re;
  double        dre;
  char          sep[2]=" ";
  long int      bitset=0;
  char          buftail[INPBUF_LEN],bufhead[INPBUF_LEN];
  char          head[INPBUF_LEN],tail[INPBUF_LEN];

  switch (dt) {
     case w_fvr: res=sscanf(buf,"%ld",&i);
                if (res>=1) {
                  *((short *) dst)=(short)i;
                  return TRUE;
                }
                break;
    case i_fvr: res=sscanf(buf,"%ld",&i);
                if (res>=1) {
                  *((int *) dst)=(int)i;
                  return TRUE;
                }
                break;
     case l_fvr: res=sscanf(buf,"%ld",&i);
/*    case l_fvr: res=sscanf(buf,"%lu",&l);*/
                if (res>=1) {
                  *((long *) dst)=i;
                  return TRUE;
                }
                break;
    case s_fvr: res=sscanf(buf,"%f",&re);
                if (res>=1) {
                  *((float *) dst)=re;
                  return TRUE;
                };
                break;
    case d_fvr: res=sscanf(buf,"%lf",&dre);
                if (res>=1) {
                  *((double *) dst)=dre;
                  return TRUE;
                };
                break;
    case text_fvr: StringnCpy((char *) dst,buf,(int )extra);
                return TRUE;
    case case_fvr: sep[0]=*(char *)extra;
                StrParse2((char*)extra,(char*)sep,(char*)head,(char*)tail);
                for (res=0;tail[0]!=0;res=res+1) {
                  StrParse2(tail,sep,head,tail);
                  if (!strcmp(buf,head)) {
                      *((int *) dst)=res;
                      return TRUE;
                  };
                };
                break;
    case bit_fvr: strcpy(buftail,buf);
                sep[0]=*(char *)extra;
                for (;buftail[0]!=0;) {
                  StrParse2(buftail,sep,bufhead,buftail);
                  if ((!buftail[0]) && (!bufhead[0])) {
                    res=TRUE; continue;
                  };
                  StrParse2((char*)extra,(char*)sep,(char*)head,(char*)tail);
                  res=FALSE;
                  for (bitno=0;tail[0]!=0;bitno=bitno+1) {
                    StrParse2(tail,sep,head,tail);
                    if (!strcmp(bufhead,head)) {
                      bitset=bitset | (1L<<bitno);
                      res=TRUE;break;
                    };
                  };
                  if (res) continue;
                  break;
                };
                if (res) {
                  *((long int *) dst)=bitset;
                  return TRUE;
                };
                break;
  };
  return FALSE;
}

/* ==========================================================================*/
static int ParseTokens( char *buf, char *delimiter) {

  TokenListType         *pcurtoken,*pprevtoken=NULL;
  char                  headbuf[256],tailbuf[256];
  int                   i=1;

  strcpy(tailbuf,buf);
  while (i!=0) {
    i=StrParse2(tailbuf,delimiter,headbuf,tailbuf);
    pcurtoken        = (TokenListType*)trsmAlloc(sizeof(TokenListType));
    pcurtoken->token = StringDup(headbuf);
    if (!pcurtoken->token) return FALSE;
    pcurtoken->next  = NULL;
    if (TokenList==NULL)
      TokenList=pcurtoken;
    else
      pprevtoken->next=pcurtoken;
    pprevtoken=pcurtoken;
  };
  return TRUE;
}

/* ==========================================================================*/

static void DeleteTokenList(void) {

  TokenListType      *ptr;

  while (TokenList) {
    ptr=TokenList;
    free(ptr->token);
    TokenList=ptr->next;
    free(ptr);
  };
  return;
}

/* ==========================================================================*/

static void ErrorMess(char *msg) {
  printf("?-f-TRS-%s",msg);
  DeleteTokenList();
}
static void Warning(char *str) {
  printf("?-w-TRS-%s\n",str);
}
/* ==========================================================================*/

/* --- token assignment ---------------------------------------------------- */

static   InpValueType  *CurrentEntryPtr=NULL;

static int AssignValue(int valfromlist,InpValueType *currentvalue,char *value,
                char *valuename,void *dst,TRSfvr_t dsttype,
                void *extra,char *help,int inpmode) {

  InpValueType  *ptr;
  int            res;

  if (valfromlist) {
    res=FillDst(value,currentvalue->val_adr,
                currentvalue->extra,currentvalue->valtp);
    currentvalue->value=reStringDup(currentvalue->value,value);
    if (!res) {
      Warning("Read: Illegal format");
      return FALSE;                    /*FALSE                            */
    };                                /*from while continue next or current*/
    ptr=currentvalue;
  } else {
    res=FillDst(value,dst,extra,dsttype);
    if (!res) {
        Warning("Read: Illegal format");
        Trs_ReadSource=TRS_None;
        return FALSE;                    /*   FALSE                        */
    };                                /*from while continue next or current*/
    ptr=AddInpValueList(valuename,value,dsttype,dst,extra,help);
    if (!ptr) {
        TRS_ERROR(TRS_StsNoMem,"AssignValue","No space for new value");
        Trs_FlowStatus=TRS_QUIT;
        return FALSE;
    };
    Trs_ReadSource=inpmode;
    CurrentEntryPtr=ptr;
  };
  if (DstToFile) PutToFile(ptr->valname,ptr->value,ptr->valhelp);
  return TRUE;
}

static int ParseTokenAssign(char *name,char *cptr,void *dst,TRSfvr_t dsttype,
                     void *extra,char *help,int inplstflg) {

  char          *iwrk;
  TRSfvr_t        valuetype;
  char          dstname[INPBUF_LEN],srcname[INPBUF_LEN];
  InpValueType  *ptr,*dptr=NULL;
  char          headbuf[INPBUF_LEN],tailbuf[INPBUF_LEN];
  int           valfromlist,res,i,dstfromfl=0;


  strcpy(dstname,name);
  strcpy(headbuf,cptr);
  valfromlist=0;
  valuetype=dsttype;
  i=StrParse2(headbuf,"=",headbuf,tailbuf);
  if (i==0) {
    res=StrParse2(headbuf,"<",headbuf,tailbuf);
    if (res==0) {                               /* "=" noexist               */
      strcpy(tailbuf,headbuf);
      headbuf[0]=0;
    } else {
      dstfromfl=1;
      if (headbuf[0]=='*') {
        headbuf[0]=0;
        SrcFromFile=TRUE;
      };
    };
  };
                                                /* parser destination        */
  if (headbuf[0]!=0) {
    strcpy(dstname,headbuf);
    if (strcmp(dstname,name)!=0) {              /* dstname != name           */
                                                /* search in ValueList       */
      dptr=ScanValueList(dstname);
      if (dptr!=NULL) {
        valfromlist=1;
        valuetype=dptr->valtp;
      } else  {                                 /* first occasion put string */
                                                /* into InputList            */
        if (dstfromfl) {
          strcpy(headbuf,"<");
          strcat(headbuf,tailbuf);
        } else strcpy(headbuf,tailbuf);
        AddInputListEntry(dstname,headbuf,inplstflg);
        return TRUE;
      };
    } else Trs_ReadSource=TRS_TTYInput;
  };
                                                /* Parser source             */
  if (dstfromfl) {
    strcpy(srcname,dstname);
    iwrk=GetDstFromFile(tailbuf,srcname);
    if (!iwrk) {
      if (!valfromlist) Trs_ReadSource=TRS_NoInFile;
      return TRUE;
    };
    printf("Read from file: %s=%s\n",srcname,iwrk);
    res=AssignValue(valfromlist,dptr,iwrk,name,dst,dsttype,
                    extra,help,TRS_FileInput);
    if (res==FALSE) {
     if (Trs_FlowStatus==TRS_QUIT) return FALSE;
     return TRUE;
    };
  } else {                                      /* letter or number        */
    if (((valfromlist==1) &&
        ((valuetype==text_fvr) || (valuetype==case_fvr) || (valuetype==bit_fvr))) ||
        ((valfromlist==0) &&
        ((dsttype==text_fvr) || (dsttype==case_fvr) || (dsttype==bit_fvr))))
    {
      res=AssignValue(valfromlist,dptr,tailbuf,name,dst,dsttype,
                      extra,help,TRS_TTYInput);
      if ((res==FALSE) && (Trs_FlowStatus==TRS_QUIT)) return FALSE;
      return TRUE;
    };
    if ((tailbuf[0]=='+') || (tailbuf[0]=='-') || (tailbuf[0]=='.')
    || ((tailbuf[0]>='0') && (tailbuf[0]<='9'))) { /* number               */
      res=AssignValue(valfromlist,dptr,tailbuf,name,dst,dsttype,
                      extra,help,TRS_TTYInput);
      if (res==FALSE) {
        if (Trs_FlowStatus==TRS_QUIT) return FALSE;
        return TRUE;
      };
    } else {                                    /* letter                  */
      strcpy(srcname,tailbuf);
      ptr=ScanValueList(srcname);
      if (ptr!=NULL) {
        if (ptr->valtp!=valuetype) {
          Warning("Read: Incompatible src and dst types");
          Trs_ReadSource=TRS_None;
          return TRUE;
        } else {
          res=AssignValue(valfromlist,dptr,ptr->value,name,dst,dsttype,
                          extra,help,TRS_TTYInput);
          if (res==FALSE) {
            if (Trs_FlowStatus==TRS_QUIT) return FALSE;
            return TRUE;
          };
        };
      } else {                                  /* src undefined         */
        Warning("Read: Source undefined");
        return TRUE;
      };
    }; /**/
  };
  return TRUE;
}

/*===========================================================================*/

static int ParseInput(void *dst,char *tname,char *buf,TRSfvr_t dsttype,
               void *extra,char *help,int inplstflg) {

  char          *cptr,*wptr;
  char          srcname[50]="";
  InpValueType  *ptr;
  TokenListType *ptoken;
  int            res;

    Trs_FlowStatus=TRS_CONTINUE;
    res=ParseTokens(buf,";");
    if (res==FALSE) {
      ErrorMess("ParseInput: No memory");
      Trs_FlowStatus=TRS_QUIT;
      return FALSE;
    };
    if (TokenList==NULL) {
      Trs_FlowStatus=TRS_CONTINUE;
      return TRUE;
    };

    for (ptoken=TokenList;ptoken!=NULL; ptoken=ptoken->next) {
      cptr=ptoken->token;                       /* get token                 */
      if((!cptr) || (*cptr==0)) continue;
      if ((ptoken==TokenList)  &&  ((dsttype==text_fvr) ||
                                    (dsttype==case_fvr) ||
                                    (dsttype==bit_fvr))) {
        res=AssignValue(0,NULL,cptr,tname,dst,dsttype,
                        extra,help,TRS_TTYInput);
        if ((res==FALSE) && (Trs_FlowStatus==TRS_QUIT)) return FALSE;
        continue;
      };
      while (*cptr==' ') cptr++;
      if (*cptr == '!') {                       /* command;                  */
        cmdAct(cptr);
        if (trsStatus()==TRS_CONTINUE) continue;
        DeleteTokenList();
        return FALSE;
      };
      if (*cptr=='?') {                         /* help                      */
        cptr++;
        trsPrintHelp(Trs_CurrentTest,*cptr,tname,"",help);
        continue;                              /* exit from while            */
      };

      wptr=cptr;

      cptr=strchr(wptr,'>');
      if (cptr){
        srcname[0]=0;
        ParseFileAppKey(cptr+1,srcname);

        if (*wptr=='*') {                       /* "*>a.dat                  */

          if (ValueList!=NULL) {
            ptr=ValueList;
            while (ptr!=NULL) {
              PutToFile(ptr->valname,ptr->value,ptr->valhelp);
              ptr=ptr->next;
            };
          };

        } else {
          if ((trsReadSource()!=TRS_None)&&(CurrentEntryPtr)) {
            if (srcname[0]==0)
              PutToFile(CurrentEntryPtr->valname,CurrentEntryPtr->value,CurrentEntryPtr->valhelp);
            else
              PutToFile(srcname,CurrentEntryPtr->value,CurrentEntryPtr->valhelp);
          };
        };
        continue;
      };
      if (!ParseTokenAssign(tname,wptr,dst,dsttype,extra,help,inplstflg)) return FALSE;
    };
    DeleteTokenList();
    return TRUE;
}

/* ==========================================================================*/

TRSAPI(int,trs_Read,(void *dst,char *name,void *extra,char *def,char *help,TRSfvr_t dsttype)) {

  char           workbuf[INPBUF_LEN],buf[INPBUF_LEN];
  char          *cptr;
  char           srcname[50];
  InpListType   *iptr;
  InpValueType  *ptr;
  int            res,ires;
  char*          tname=name+1;
  char           sep[2]=" ";
  char           head[INPBUF_LEN],tail[INPBUF_LEN],_typename[50];

  if (dsttype==text_fvr)tname=tname-1;
  Trs_ReadSource=TRS_None;

  if ((dsttype==case_fvr)||(dsttype==bit_fvr)) {
    sep[0]=*(char*)extra;
    StrParse2((char*)extra,(char*)sep,(char*)head,(char*)tail);
  };
  iptr=ScanInpList(tname);              /* test2. dst already defined (search*/
                                        /* in input list )                   */
  if (iptr)                             /* search in inputlist               */
  {                                     /* find                              */
    cptr=workbuf;
    cptr=strcpy(cptr,iptr->inputdst);
    cptr=strcat(cptr,"=");
    cptr=strcat(cptr,iptr->inputsrc);
    if (ParseTokenAssign(tname,cptr,dst,dsttype,extra,help,0)) {
       if (trsReadSource()!=TRS_None) {
         if (!iptr->lorgflg) Trs_ReadSource=TRS_IndirectTTY;
         else Trs_ReadSource=TRS_CmdLine;
         DeleteInpListEntry(iptr);
         return TRUE;
       } else DeleteInpListEntry(iptr);
    };
  };
  if (SrcFromFile) {                    /* test 3. dst from file             */
    strcpy(srcname,tname);
    cptr=GetDstFromFile(Trs_String,srcname);

    if (cptr) {
      printf("From file: %s=%s\n",srcname,cptr);
      res=FillDst(cptr,dst,extra,dsttype);
      if (res) {
        ptr=AddInpValueList(tname,cptr,dsttype,dst,extra,help);
        Trs_ReadSource=TRS_FileInput;
        if (DstToFile) PutToFile(ptr->valname,ptr->value,ptr->valhelp);
        return TRUE;
      };
    };
    Trs_ReadSource=TRS_NoInFile;
  };

  strcpy(_typename,TypeName[dsttype]);
  if ((dsttype==case_fvr)||(dsttype==bit_fvr)) strcpy(_typename,tail);

  for(;;) {
    res=CmdWithoutPrefix(buf,TypePrompt[dsttype],tname,_typename,def,help);
    switch (res) {
      case RETRY : continue;
      case FALSE : return FALSE;
      case TRUE  : ires=ParseInput(dst,tname,buf,dsttype,extra,help,0);
                   if (ires==FALSE) return FALSE;
                   if ((trsReadSource()==TRS_None) ||
                   (trsReadSource()==TRS_NoInFile) ||
                   (trsReadSource()==TRS_DefaultNull)  ||
                   (trsReadSource()==TRS_Default)) break;
                   Trs_FlowStatus=TRS_CONTINUE;
                   return TRUE;
    };
  };
}
TRSAPI(int, CmdWithoutPrefix,(char *buf,  char *prompt, char *dstname,
                     char *type,char *def,    char *help)) {

  char           bufwdefault[COMMONLEN];

  if (trsGetKeyArg('B')==NULL)
    trsMoreSize(Trs_PageSize);
  printf("%s%s, %s",prompt,dstname,type);
  if (def) printf(", [%s]",def);
  if (Trs_ReadSource==TRS_NoInFile)
     printf("?> ");
  else
     printf(" > ");

  CurrentEntryPtr=NULL;
  Trs_FlowStatus=TRS_CONTINUE;
  gets(buf);
  for (;;) {
    switch (buf[0]) {
      case  ';':
      case  0  : if (!def) {
                   Trs_ReadSource=TRS_DefaultNull;
                   Trs_FlowStatus=TRS_CONTINUE;
                   if (buf[0]==';') return TRUE;
                   return FALSE;
                 }
                 Trs_ReadSource=TRS_Default;
                 if (def[0]==0) {
                   if (buf[0]==';') return TRUE;
                   return RETRY;
                 };
                 strcpy(bufwdefault,def);
                 strcat(bufwdefault,buf);
                 strcpy(buf,bufwdefault);
                 continue;
      case '!' :switch (buf[1]) {
                  case 'q': Trs_FlowStatus=TRS_QUIT;break;
                  case 'r': Trs_FlowStatus=TRS_RETRY;break;
                  case 'p': Trs_FlowStatus=TRS_PREVIOUS;break;
                  case 'e': Trs_FlowStatus=TRS_EXIT;break;
                  case 't': Trs_FlowStatus=TRS_TOP;break;
                  default : return RETRY;
                };break;
      case HELP_PREFIX:
                  trsPrintHelp(Trs_CurrentTest,buf[1],dstname,def,help);return RETRY;
      default: return TRUE;
    }
    return FALSE;
  }
}

TRSAPI(int, trs_bRead,(void *dst,char *name,int n,char *def,char *help,
               TRSfvr_t dsttype)) {
  char   buf[INPBUF_LEN];
  int    i,res=0;

  if ((n<=0) || (!dst)) return FALSE;

  for (i=0;i<n;i++) {
    sprintf(buf,"&%s[%d]",name,i);
    switch (dsttype) {
      case l_fvr: res=trs_Read( &((long  *)dst)[i],buf,0,def,help,dsttype); break;
      case w_fvr: res=trs_Read( &((short *)dst)[i],buf,0,def,help,dsttype); break;
      case i_fvr: res=trs_Read( &((int   *)dst)[i],buf,0,def,help,dsttype); break;
      case s_fvr: res=trs_Read( &((float *)dst)[i],buf,0,def,help,dsttype); break;
      case d_fvr: res=trs_Read( &((double*)dst)[i],buf,0,def,help,dsttype); break;
    };
    if (!res) return res;
  };
  return res;
}

/* ==========================================================================*/
TRSAPI(int, trsPause,(char *def,char *help)) {

  char                   buf[INPBUF_LEN];
  int                    ires,res;

  for(;;) {
    res=CmdWithoutPrefix(buf,"","Pause",help,def,help);
    switch (res) {
      case RETRY : continue;
      case FALSE : if (Trs_ReadSource==TRS_Default) return TRUE;
                   return FALSE;
      case TRUE  : ires=ParseInput(NULL,"",buf,i_fvr,NULL,help,0);
                   if (ires==FALSE) return FALSE;
                   Trs_FlowStatus=TRS_CONTINUE;
                   return TRUE;
    };
  };
}

/* ========================================================================== */

TRSAPI(int, _trslRead,(long *dst,char *name,char *def,char *help)) {
  return trs_Read(dst,name,NULL,def,help,l_fvr);
}

TRSAPI(int, _trswRead,(short *dst,char *name,char *def,char *help)) {
  return trs_Read(dst,name,NULL,def,help,w_fvr);
}
TRSAPI(int, _trsiRead,(int *dst,char *name,char *def,char *help)) {
  return trs_Read(dst,name,NULL,def,help,i_fvr);
}

TRSAPI(int, _trssRead,(float *dst,char *name,char *def,char *help)) {
  return trs_Read(dst,name,NULL,def,help,s_fvr);
}
TRSAPI(int, _trsdRead,(double *dst,char *name,char *def,char *help)) {
  return trs_Read(dst,name,NULL,def,help,d_fvr);
}
TRSAPI(int, _trstRead,(char * str,int size,char *name,char *def,char *help)){
  return trs_Read(str,name,(void *)size,def,help,text_fvr);
}
TRSAPI(int, _trsCaseRead,(int *dst, char *name, char *items,char *def,char *help)){
  return trs_Read(dst,name,items,def,help,case_fvr);
}
TRSAPI(int, _trsBitRead,(long int *dst, char *name, char *items,char *def,char *help)){
  return trs_Read(dst,name,items,def,help,bit_fvr);
}

/* --------------------------------------------------------------------------*/

TRSAPI(int, _trslbRead,(long *dst,char *name,int n,char *def,char *help)) {
  return trs_bRead(dst,name,n,def,help,l_fvr);
}


TRSAPI(int, _trswbRead,(short *dst,char *name,int n,char *def,char *help)) {
  return trs_bRead(dst,name,n,def,help,w_fvr);
}
TRSAPI(int, _trsibRead,(int *dst,char *name,int n,char *def,char *help)) {
  return trs_bRead(dst,name,n,def,help,i_fvr);
}

TRSAPI(int, _trssbRead,(float *dst,char *name,int n,char *def,char *help)) {
  return trs_bRead(dst,name,n,def,help,s_fvr);
}
TRSAPI(int, _trsdbRead,(double *dst,char *name,int n,char *def,char *help)) {
  return trs_bRead(dst,name,n,def,help,d_fvr);
}

#endif

