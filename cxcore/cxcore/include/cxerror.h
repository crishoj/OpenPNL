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

#ifndef _CXCORE_ERROR_H_
#define _CXCORE_ERROR_H_

#if defined _CX_ALWAYS_PROFILE_ || defined _DEBUG
#define _CX_COMPILE_PROFILE_
#endif

#define _CX_ALWAYS_NO_PROFILE_

#ifdef _CX_ALWAYS_NO_PROFILE_
#undef _CX_COMPILE_PROFILE_
#endif

#ifdef _CX_COMPILE_PROFILE_
   #define CX_START_CALL( func, file, line )  cxStartProfile( func, file, line )
   #define CX_END_CALL( file, line )    cxEndProfile( file, line )
#else
   #define CX_START_CALL( func, file, line )
   #define CX_END_CALL( file, line )
#endif 

/************Below is declaration of error handling stuff in PLSuite manner**/

typedef int CXStatus;

/* this part of CXStatus is compatible with IPLStatus 
  Some of below symbols are not [yet] used in OpenCX
*/
#define CX_StsOk                    0  /* everithing is ok                */
#define CX_StsBackTrace            -1  /* pseudo error for back trace     */
#define CX_StsError                -2  /* unknown /unspecified error      */
#define CX_StsInternal             -3  /* internal error (bad state)      */
#define CX_StsNoMem                -4  /* Insufficient memory             */
#define CX_StsBadArg               -5  /* function arg/param is bad       */
#define CX_StsBadFunc              -6  /* unsupported function            */
#define CX_StsNoConv               -7  /* iter. didn't converge           */
#define CX_StsAutoTrace            -8  /* Tracing                         */

#define CX_HeaderIsNull            -9  /* image header is NULL            */
#define CX_BadImageSize            -10 /* Image size is invalid           */
#define CX_BadOffset               -11 /* Offset is invalid               */
#define CX_BadDataPtr              -12 /**/
#define CX_BadStep                 -13 /**/
#define CX_BadModelOrChSeq         -14 /**/
#define CX_BadNumChannels          -15 /**/
#define CX_BadNumChannel1U         -16 /**/
#define CX_BadDepth                -17 /**/
#define CX_BadAlphaChannel         -18 /**/
#define CX_BadOrder                -19 /**/
#define CX_BadOrigin               -20 /**/
#define CX_BadAlign                -21 /**/
#define CX_BadCallBack             -22 /**/
#define CX_BadTileSize             -23 /**/
#define CX_BadCOI                  -24 /**/
#define CX_BadROISize              -25 /**/

#define CX_MaskIsTiled             -26 /**/

#define CX_StsNullPtr                -27 /* Null pointer */
#define CX_StsVecLengthErr           -28 /* Incorrect vector length */
#define CX_StsFilterStructContentErr -29 /* Incorr. filter structure content */
#define CX_StsKernelStructContentErr -30 /* Incorr. transform kernel content */
#define CX_StsFilterOffsetErr        -31 /* Incorrect filter ofset value */

/*extra for CX */
#define CX_StsBadSize                -201 /* bad CxSize */
#define CX_StsDivByZero              -202 /* division by zero */
#define CX_StsInplaceNotSupported    -203 /* inplace operation is not supported */
#define CX_StsObjectNotFound         -204 /* request can't be completed */
#define CX_StsUnmatchedFormats       -205 /* formats of input/output arrays differ */
#define CX_StsBadFlag                -206 /* flag is wrong or not supported */  
#define CX_StsBadPoint               -207 /* bad CxPoint */ 
#define CX_StsBadMask                -208 /* bad format of mask (neither 8uC1 nor 8sC1)*/
#define CX_StsUnmatchedSizes         -209 /* ROI sizes of arrays differ */
#define CX_StsUnsupportedFormat      -210 /* the format is not supported by the function*/
#define CX_StsOutOfRange             -211 /* Some of parameters is out of range */

/********************************* Error handling Macros ********************************/

#define OPENCX_ERROR(status,func,context)                           \
                cxError((status),(func),(context),__FILE__,__LINE__)

#define OPENCX_ERRCHK(func,context)                                 \
                ((cxGetErrStatus() >= 0) ? CX_StsOk                 \
                : OPENCX_ERROR(CX_StsBackTrace,(func),(context)))

#define OPENCX_ASSERT(expr,func,context)                            \
                ((expr) ? CX_StsOk                                  \
                : OPENCX_ERROR(CX_StsInternal,(func),(context)))

#define OPENCX_RSTERR() (cxSetErrStatus(CX_StsOk))

#define OPENCX_CALL( Func )                                         \
{                                                                   \
    CX_START_CALL( #Func, __FILE__, __LINE__ );                     \
    Func;                                                           \
    CX_END_CALL( __FILE__, __LINE__ );                              \
} 


/**************************** OpenCX-style error handling *******************************/

/* CX_FUNCNAME macro defines icxFuncName constant which is used by CX_ERROR macro */
#ifdef CX_NO_FUNC_NAMES
    #define CX_FUNCNAME( Name )
    #define cxFuncName ""
#else    
    #define CX_FUNCNAME( Name )  \
    static char cxFuncName[] = Name
#endif


/*
  CX_ERROR macro unconditionally raises error with passed code and message.
  After raising error, control will be transferred to the exit label.
*/
#define CX_ERROR( Code, Msg )                                       \
{                                                                   \
     cxError( (Code), cxFuncName, Msg, __FILE__, __LINE__ );        \
     EXIT;                                                          \
}

/* Simplified form of CX_ERROR */
#define CX_ERROR_FROM_CODE( code )   \
    CX_ERROR( code, "" )

/*
 CX_CHECK macro checks error status after CX (or IPL)
 function call. If error detected, control will be transferred to the exit
 label.
*/
#define CX_CHECK()                                                  \
{                                                                   \
    if( cxGetErrStatus() < 0 )                                      \
        CX_ERROR( CX_StsBackTrace, "Inner function failed." );      \
}


/*
 CX_CALL macro calls CX (or IPL) function, checks error status and
 signals a error if the function failed. Useful in "parent node"
 error procesing mode
*/
#define CX_CALL( Func )                                             \
{                                                                   \
    /* start profile */                                             \
    CX_START_CALL( #Func, __FILE__, __LINE__ );                     \
    Func;                                                           \
    CX_END_CALL( __FILE__, __LINE__ );                              \
    CX_CHECK();                                                     \
}


/* Runtime assertion macro */
#define CX_ASSERT( Condition )                                          \
{                                                                       \
    if( !(Condition) )                                                  \
        CX_ERROR( CX_StsInternal, "Assertion: " #Condition " failed" ); \
}

#define __BEGIN__       {
#define __END__         goto exit; exit: ; }
#define __CLEANUP__
#define EXIT            goto exit

#define CX_ERROR_FROM_STATUS( result )                \
    CX_ERROR( cxErrorFromStatus( result ), "OpenCX function failed" )

#define IPPI_CALL( Func )                                              \
{                                                                      \
      CxStatus  ippi_call_result;                                      \
      CX_START_CALL( #Func, __FILE__, __LINE__ );                      \
      ippi_call_result = Func;                                         \
      CX_END_CALL( __FILE__, __LINE__ );                               \
                                                                       \
      if( ippi_call_result < 0 )                                       \
            CX_ERROR_FROM_STATUS( (ippi_call_result));                 \
}

#endif /* _CXCORE_ERROR_H_ */

/* End of file. */
