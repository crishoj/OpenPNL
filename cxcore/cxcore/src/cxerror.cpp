/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//            Intel License Agreement
//        For Open Source Computer Vision Library
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

#include "_cxcore.h"

#ifdef WIN32  
#include <windows.h>  
#else
#include <pthread.h>
#endif 

typedef struct CxContext
{
    CXStatus        CXLastStatus;
    int             CXErrMode;
    CXErrorCallBack CXErrorFunc;
    CxStackRecord*  CXStack;
    int             CXStackSize;
    int             CXStackCapacity;
    
} CxContext;

CxContext* icxCreateContext()
{
    CxContext* context = (CxContext*)malloc( sizeof( CxContext ) );

    context->CXErrMode    = CX_ErrModeLeaf;
    context->CXLastStatus = CX_StsOk;

#ifdef WIN32
   context->CXErrorFunc   = cxGuiBoxReport;
#else
   context->CXErrorFunc   = cxStdErrReport;
#endif

   /* below is stuff for profiling */
   context->CXStackCapacity = 100; /* let it be so*/
   context->CXStackSize = 0;
   context->CXStack =
       (CxStackRecord*)malloc( sizeof(CxStackRecord) * context->CXStackCapacity );
   return context;
}

void icxDestroyContext(CxContext* context)
{
    free(context->CXStack);
    free(context);
}

#ifdef WIN32
    DWORD g_TlsIndex = TLS_OUT_OF_INDEXES;
#else
    pthread_key_t g_TlsIndex;
#endif

CxContext* icxGetContext()
{
#ifdef CX_DLL
#ifdef WIN32
    CxContext* pContext = (CxContext*)TlsGetValue( g_TlsIndex );
    if( !pContext )
    {
    pContext = icxCreateContext();

    if( !pContext )
    {
        FatalAppExit( 0, "OpenCX. Problem to allocate memory for TLS OpenCX context." );
    }
    TlsSetValue( g_TlsIndex, pContext );
    }
    return pContext;
#else
    CxContext* pContext = (CxContext*)pthread_getspecific( g_TlsIndex );
    if( !pContext )
    {
    pContext = icxCreateContext();
    if( !pContext )
    {
            fprintf(stderr,"OpenCX. Problem to allocate memory for OpenCX context.");
        exit(1);
    }
    pthread_setspecific( g_TlsIndex, pContext );
    }
    return pContext;
#endif
#else /* CX_DLL */
    static CxContext* pContext = 0;

    if( !pContext )
    pContext = icxCreateContext();

    return pContext;
#endif
}


CX_IMPL CXStatus cxStdErrReport( CXStatus status, const char *funcName,
                                 const char *context, const char *file, int line )
{
    CxContext* cont = icxGetContext();

    if ( cxGetErrMode() == CX_ErrModeSilent )
    return ( status != CX_StsBackTrace ) ? ( cont->CXLastStatus = status ) : status;
    
    if (( status == CX_StsBackTrace ) || ( status == CX_StsAutoTrace ))
    fprintf(stderr, "\tcalled from ");
    else 
    {
    cont->CXLastStatus = status;
    fprintf(stderr, "OpenCX Error: %s \n\tin function ", cxErrorStr(status));
    }
    if ( line > 0 || file != NULL )
    fprintf(stderr,"[%s:%d]", file, line);
    fprintf(stderr,":%s", funcName ? funcName : "<unknown>");
    if ( context != NULL ) {
    if ( status != CX_StsAutoTrace )
        fprintf(stderr, "():\n%s", context);    /* Print context      */
    else
        fprintf(stderr, "(%s)", context);          /* Print arguments    */
    }
    fprintf(stderr, "\n");
    if ( cont->CXErrMode == CX_ErrModeLeaf ) {
    fprintf(stderr, "OpenCX: %s\n","terminating the application");
    exit(1);
    };

    return status;
}

CX_IMPL CXStatus cxGuiBoxReport( CXStatus status, const char *funcName, 
                 const char *context, const char *file, int line)
{

#ifdef WIN32

    char mess[1000];
    char title[100];
    char *choice = 0;
    const char* errText = cxErrorStr( status );


    if ( cxGetErrMode() != CX_ErrModeSilent )
    {
        if( !funcName ) funcName = "<unknown>";
        if( !context  ) context = "";
        if( !file     ) file = "";
        if(  line < 0 ) line = 0;

        if( cxGetErrMode() == CX_ErrModeLeaf )
            choice="\nErrMode=CX_ErrorModeLeaf\n"
                   "\nTerminate the application?";

        if( cxGetErrMode() == CX_ErrModeParent)
            choice="\nErrMode=CX_ErrorModeParent\n"
            "\nContinue?";

        if( status == CX_StsBackTrace)
            wsprintf( mess,"Called from %s(): [file %s, line %d]\n%s\n%s\n(status:%d)\n%s",
                      funcName, file,line,context, errText, status, choice);
        else if ( status == CX_StsAutoTrace )
            wsprintf( mess,"Called from %s(): [file %s, line %d]\n%s\n%s\n(status:%d)\n%s",
                      funcName, file, line, context, errText, status, choice);
        else
            wsprintf( mess,"In function %s(): [file %s, line %d]\n%s\n%s\n(status:%d)\n%s",
                      funcName, file, line, context,errText, status, choice);

        wsprintf(title,"OpenCX Beta 2: %s",cxErrorStr(cxGetErrStatus()));

        int answer = -1;

        if( (( cxGetErrMode()==CX_ErrModeParent) &&
            (IDCANCEL==MessageBox(NULL,mess,title,MB_ICONERROR|MB_OKCANCEL|MB_SYSTEMMODAL) ) ||
            ((cxGetErrMode() == CX_ErrModeLeaf) &&
            //(IDYES==MessageBox(NULL,mess,title,MB_ICONERROR|MB_YESNO|MB_SYSTEMMODAL))
            (IDABORT == (answer=MessageBox(NULL,mess,title,MB_ICONERROR|MB_ABORTRETRYIGNORE|MB_SYSTEMMODAL))||
            IDRETRY == answer)
            )))
        {
            if( answer == IDRETRY )
            {

    #if _MSC_VER >= 1200 || defined __ICL
                __asm int 3;
    #else
                assert(0);
    #endif
            }
            FatalAppExit(0,"OpenCX:\nterminating the application");
        }
    }

#else
    cxStdErrReport( status, funcName, context, file, line);
#endif

    return status;
}


CX_IMPL CXStatus cxNulDevReport( CXStatus status, const char *funcName,
                 const char *context, const char *file, int line)
{
  if( status||funcName||context||file||line )
  if ( cxGetErrMode() == CX_ErrModeLeaf )
      exit(1);
  return status;
}

CX_IMPL CXErrorCallBack cxRedirectError(CXErrorCallBack func)
{
    CxContext* context = icxGetContext();

    CXErrorCallBack old = context->CXErrorFunc;
    context->CXErrorFunc = func;
    return old;
} 
 
CX_IMPL const char* cxErrorStr(CXStatus status)
{
    static char buf[80];

    switch (status) 
    {
    case CX_StsOk :    return "No Error";
    case CX_StsBackTrace : return "Backtrace";
    case CX_StsError :     return "Unknown error";
    case CX_StsInternal :  return "Internal error";
    case CX_StsNoMem :     return "Insufficient memory";
    case CX_StsBadArg :    return "Bad argument";
    case CX_StsNoConv :    return "Iteration convergence failed";
    case CX_StsAutoTrace : return "Autotrace call";
    case CX_StsBadSize :   return "Bad/unsupported parameter of type CxSize";
    case CX_StsNullPtr :   return "Null pointer";
    case CX_StsDivByZero : return "Divizion by zero occured";
    case CX_BadStep :      return "Image step is wrong";
    case CX_StsInplaceNotSupported : return "Inplace operation is not supported";
    case CX_StsObjectNotFound :      return "Requested object was not found";
    case CX_BadDepth :     return "Input image depth is not supported by function";
    case CX_StsUnmatchedFormats : return "Formats of input arguments do not match"; 
    case CX_StsUnmatchedSizes :  return "Sizes of input arguments do not match";
    case CX_StsOutOfRange : return "One of arguments\' values is out of range";
    case CX_StsUnsupportedFormat : return "Unsupported format or combination of formats";
    case CX_BadCOI :      return "Input COI is not supported";
    case CX_BadNumChannels : return "Bad number of channels";
    case CX_StsBadFlag :   return "Bad flag (parameter or structure field)";
    case CX_StsBadPoint :  return "Bad parameter of type CxPoint";
    };

    sprintf(buf, "Unknown %s code %d", status >= 0 ? "status":"error", status);
    return buf;
}

CX_IMPL int cxGetErrMode(void)
{
    return icxGetContext()->CXErrMode;
}

CX_IMPL void cxSetErrMode( int mode )
{
    icxGetContext()->CXErrMode = mode;
}

CX_IMPL CXStatus cxGetErrStatus()
{
    return icxGetContext()->CXLastStatus;
}

CX_IMPL void cxSetErrStatus(CXStatus status)
{
    icxGetContext()->CXLastStatus = status;
}


/******************** Implementation of profiling stuff *********************/

/* initial assignment of profiling functions */
CxStartProfileFunc p_cxStartProfile = cxStartProfile;
CxEndProfileFunc p_cxEndProfile = cxEndProfile;


CX_IMPL void cxSetProfile( void (CX_CDECL *startprofile_f)(const char*, const char*, int),
               void (CX_CDECL *endprofile_f)(const char*, int))
{
    p_cxStartProfile = startprofile_f;
    p_cxEndProfile   = endprofile_f;
}

CX_IMPL void cxRemoveProfile()
{
    p_cxStartProfile = cxStartProfile;
    p_cxEndProfile   = cxEndProfile;
}

    

/* default implementation of cxStartProfile & cxEndProfile */
void CX_CDECL cxStartProfile(const char* call, const char* file, int line )
{   
#ifdef _CX_COMPILE_PROFILE_
    if( p_cxStartProfile != cxStartProfile )
    {
    p_cxStartProfile( call, file, line );
    }        
       
    /* default implementation */
    CxContext* context = icxGetContext();

    /* add record to stack */
    assert( context->CXStackCapacity >= context->CXStackSize ); 
    if( context->CXStackCapacity == context->CXStackSize )
    {
    /* increase stack */
    context->CXStackCapacity += 100;
    context->CXStack = (CxStackRecord*)realloc( context->CXStack, 
                      (context->CXStackCapacity) * sizeof(CxStackRecord) );
    }

    CxStackRecord* rec = &context->CXStack[context->CXStackSize];
    rec->file = file;
    rec->line = line;
    context->CXStackSize++;
#else 
    /* avoid warning "unreferenced value" */
    if( call||file||line) {}
    assert(0);
#endif
};

CX_IMPL void cxEndProfile( const char* file, int line )
{
#ifdef _CX_COMPILE_PROFILE_
    CxContext* context = icxGetContext();
    if( p_cxEndProfile != cxEndProfile )
    {
    p_cxEndProfile( file, line );
    }                
    /* default implementation */  
    context->CXStackSize--;

#else 
    /* avoid warning "unreferenced value" */
    if( file||line) {}
    assert(0);
#endif

};


CX_IMPL CXStatus cxError( CXStatus code, const char* funcName, 
              const char* msg, const char* file, int line )
{
    CxContext* context = icxGetContext();

    if ((code!=CX_StsBackTrace) && (code!=CX_StsAutoTrace))
    cxSetErrStatus(code);
    if (code == CX_StsOk)
    return CX_StsOk;
   
#ifdef _CX_COMPILE_PROFILE_

    int i;                                    
    char message[4096] = "";                              
    /* copy input message */                              
    strcpy( message, msg );                           
    /* append stack info */
    strcat( message, "\nStack\n{" );                               
    char* mes = message + strlen(message);

    for( i = 0; i < context->CXStackSize; i++ )                      
    {         
    i ? 0 : sprintf( mes,"\n" ), mes += strlen(mes); 
    CxStackRecord* rec = &(context->CXStack[i]);
    sprintf( mes, "   %s line %d\n", rec->file, rec->line ); 
    mes += strlen(mes);
    }
    strcat( message, "}\n" );

    context->CXErrorFunc( code, funcName, message, file, line );          
#else          
    context->CXErrorFunc( code, funcName, msg, file, line );

#endif
    return code;
};

CX_IMPL void cxGetCallStack(CxStackRecord** stack, int* size)
{
    CxContext* context = icxGetContext();
    *stack = context->CXStack;
    *size  = context->CXStackSize;
}

/******************** End of implementation of profiling stuff *********************/


/**********************DllMain********************************/

#ifdef CX_DLL

#ifdef WIN32
BOOL WINAPI DllMain( HINSTANCE /*hinstDLL*/,     /* DLL module handle        */
             DWORD     fdwReason,    /* reason called        */
             LPVOID    /*lpvReserved*/)  /* reserved             */
{
    CxContext *pContext;

    /// Note the actual size of the structure is larger.

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:

    g_TlsIndex = TlsAlloc();
    if( g_TlsIndex == TLS_OUT_OF_INDEXES ) return FALSE;

    /* No break: Initialize the index for first thread. */
    /* The attached process creates a new thread. */

    case DLL_THREAD_ATTACH:

    pContext = icxCreateContext();
    if( pContext == NULL)
        return FALSE;
    TlsSetValue( g_TlsIndex, (LPVOID)pContext );
    break;

    case DLL_THREAD_DETACH:

    if( g_TlsIndex != TLS_OUT_OF_INDEXES ) 
    {
        pContext = (CxContext*)TlsGetValue( g_TlsIndex );
        if( pContext != NULL ) 
        {
        icxDestroyContext( pContext );
        }
    }
    break;

    case DLL_PROCESS_DETACH:

    if( g_TlsIndex != TLS_OUT_OF_INDEXES ) {
        pContext = (CxContext*)TlsGetValue( g_TlsIndex );
        if( pContext != NULL ) 
        {
        icxDestroyContext( pContext );
        }
        TlsFree( g_TlsIndex );
    }
    break;

    default:
    break;
    }
    return TRUE;
}
#else
/* POSIX pthread */

/* function - destructor of thread */
void icxPthreadDestructor(void* key_val)
{
    CxContext* context = (CxContext*) key_val;
    icxDestroyContext( context );
}

int pthrerr = pthread_key_create( &g_TlsIndex, icxPthreadDestructor );

#endif

#endif

/* function, which converts CxStatus to CXStatus */
CX_IMPL CXStatus
cxErrorFromStatus( CxStatus status )
{
    switch (status) 
    {
    case CX_BADSIZE_ERR     : return CX_StsBadSize; //bad parameter of type CxSize
    case CX_NULLPTR_ERR     : return CX_StsNullPtr;
    case CX_DIV_BY_ZERO_ERR : return CX_StsDivByZero;
    case CX_BADSTEP_ERR     : return CX_BadStep ;
    case CX_OUTOFMEM_ERR    : return CX_StsNoMem;
    case CX_BADARG_ERR      : return CX_StsBadArg;
    case CX_NOTDEFINED_ERR  : return CX_StsError; //unknown/undefined err
    
    case CX_INPLACE_NOT_SUPPORTED_ERR: return CX_StsInplaceNotSupported;
    case CX_NOTFOUND_ERR : return CX_StsObjectNotFound;
    case CX_BADCONVERGENCE_ERR: return CX_StsNoConv;
    case CX_BADDEPTH_ERR     : return CX_BadDepth;
    case CX_UNMATCHED_FORMATS_ERR : return CX_StsUnmatchedFormats;

    case CX_UNSUPPORTED_COI_ERR      : return CX_BadCOI; 
    case CX_UNSUPPORTED_CHANNELS_ERR : return CX_BadNumChannels; 
    
    case CX_BADFLAG_ERR : return CX_StsBadFlag;//used when bad flag CX_ ..something
    
    case CX_BADRANGE_ERR    : return CX_StsBadArg; //used everywhere
    case CX_BADCOEF_ERR  :return CX_StsBadArg;     //used everywhere
    case CX_BADFACTOR_ERR:return CX_StsBadArg;     //used everywhere
    case CX_BADPOINT_ERR  :return CX_StsBadPoint;

    default: assert(0); return CX_StsError;
    }         
}         
/* End of file */


