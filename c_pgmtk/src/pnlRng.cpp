/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:                                                                  //
//                                                                         //
//  Purpose:                                                               //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlParConfig.hpp"
#include "pnlConfig.hpp"
#include "pnlRng.hpp"
#include "pnlException.hpp"

#ifdef PAR_OMP
#include <omp.h>
#define NUMBER_OF_HEAPS 256
#define PAR_OMP_NUM_CURR_THREAD omp_get_thread_num()
#else
#define NUMBER_OF_HEAPS 1
#define PAR_OMP_NUM_CURR_THREAD 0
#endif

//#define USE_VSL

#ifdef USE_VSL
#include "mkl_vsl.h"
#pragma comment(lib,"mkl_c_dll")

#define _VSL_UNI_METHOD_  VSL_BRNG_MCG31

//
class CRNG
{
public:
    VSLStreamStatePtr m_vslStream;
public:
    CRNG()
    {
        m_vslStream = 0;
        vslNewStream(&m_vslStream, _VSL_UNI_METHOD_, 0 );
    }
    ~CRNG()
    {
        if(m_vslStream) vslDeleteStream(&m_vslStream);
    }         
};

//global variable
CRNG g_RNG;

PNL_BEGIN
void pnlSeed(int s)
{

    vslDeleteStream(&g_RNG.m_vslStream);
    vslNewStream(&g_RNG.m_vslStream, _VSL_UNI_METHOD_, s); 
}

//generate uniform integer distribution in the range [left,right]
int pnlRand(int left, int right)
{
    //TODO: check argument range
    int retVal = 0;
    viRngUniform( VSL_METHOD_IUNIFORM_STD, g_RNG.m_vslStream, 1, &retVal, left, right+1 );
    return retVal;
}

void pnlRand(int numElem, int* vec, int left, int right )
{
   viRngUniform( VSL_METHOD_IUNIFORM_STD, g_RNG.m_vslStream, numElem, vec, left, right+1 );
}

//generate uniform distribution (single precision) on (0,1) range
float pnlRand(float left, float right)
{
    //TODO: check argument range
    float retVal = 0;
    vsRngUniform( VSL_METHOD_SUNIFORM_STD, g_RNG.m_vslStream, 1, &retVal, left, right );
    return retVal;           
}

double pnlRand(double left, double right)
{
    //TODO: check argument range
    double retVal = 0;
    vdRngUniform( VSL_METHOD_DUNIFORM_STD, g_RNG.m_vslStream, 1, &retVal, left, right );
    return retVal;           
} 
           
void pnlRand(int numElem, float* vec, float left, float right)
{
   vsRngUniform( VSL_METHOD_SUNIFORM_STD, g_RNG.m_vslStream, numElem, vec, left, right );
}

void pnlRand(int numElem, double* vec, double left, double right)
{
   vdRngUniform( VSL_METHOD_DUNIFORM_STD, g_RNG.m_vslStream, numElem, vec, left, right );
}

float pnlRandNormal( float mean, float sigma )
{
    float val = 0;
    vsRngGaussian( VSL_METHOD_SGAUSSIAN_BOXMULLER2, g_RNG.m_vslStream, 1, &val, mean, (float)sqrt(sigma) );
    return val;
}

double pnlRandNormal( double mean, double sigma )
{
    double val = 0;
    vdRngGaussian( VSL_METHOD_DGAUSSIAN_BOXMULLER2, g_RNG.m_vslStream, 1, &val, mean, sqrt(sigma) );
    return val;
}

void pnlRandNormal( int numElem, float* vec, float mean, float sigma )
{
    vsRngGaussian( VSL_METHOD_SGAUSSIAN_BOXMULLER2, g_RNG.m_vslStream, numElem, vec, mean, (float)sqrt(sigma) );
}

void pnlRandNormal( int numElem, double* vec, double mean, double sigma )
{
    vdRngGaussian( VSL_METHOD_DGAUSSIAN_BOXMULLER2, g_RNG.m_vslStream, numElem, vec, mean, sqrt(sigma) );
}   

PNL_END
#else //OpenCX's (PNL default)

#include "cxcore.h"
//initialization of global RNG

PNL_BEGIN

class CRNG
{
public:
    CxRandState m_cxRandState;
public:
    CRNG()
    {
        cxRandInit( &m_cxRandState, 0, 1, 0/*seed*/, CX_RAND_UNI );
    }
    ~CRNG() {};
};

//global variable
CRNG g_RNG[NUMBER_OF_HEAPS];

void pnlSeed(int s)
{
    cxRandInit( &g_RNG[PAR_OMP_NUM_CURR_THREAD].m_cxRandState, 0, 1, s, CX_RAND_UNI );
}

//generate uniform integer distribution in the range [left,right]
int pnlRand(int left, int right)
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
    
    PNL_CHECK_LEFT_BORDER( right, left );
    g_RNG[myid].m_cxRandState.disttype = CX_RAND_UNI;
    //set range
    cxRandSetRange( &g_RNG[myid].m_cxRandState, left, right, -1 );

    //create matrix wrapper for 1 floating point value
    float val = 0;
    CxMat mat = cxMat( 1, 1, CX_32FC1, &val );

    //generate value
    cxRand( &g_RNG[myid].m_cxRandState, &mat );
    double x1;
   
    double ip = modf(val, &x1);
    return (int)(x1 >= 0 ? ( ip > 0.5 ? ++x1 : x1 ) : ( ip > -0.5 ? --x1 : x1 )); 
    
}

void pnlRand(int numElem, int* vec, int left, int right)
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
   
    //set distribution type
    g_RNG[myid].m_cxRandState.disttype = CX_RAND_UNI;
    //set range
    cxRandSetRange( &g_RNG[myid].m_cxRandState, left, right, -1 );
    
    //create matrix wrapper for 1 floating point value
    float *array = new float[numElem];
    CxMat mat = cxMat( 1, numElem, CX_32FC1, array );
    
    //generate value
    cxRand( &g_RNG[myid].m_cxRandState, &mat );
   
    double ip;
    double x1;

    int i = 0;
    for( i = 0; i < numElem; ++i )
    {
	ip = modf(array[i], &x1);
	vec[i] =  (int)(x1 >= 0 ? ( ip > 0.5 ? ++x1 : x1 ) : ( ip > -0.5 ? --x1 : x1 )); 

    }
    delete []array;
    
}

//generate uniform distribution (single precision) on (0,1) range
float pnlRand(float left, float right)
{
    int myid = PAR_OMP_NUM_CURR_THREAD;

    //set distribution type
    g_RNG[myid].m_cxRandState.disttype = CX_RAND_UNI;
    //set range
    cxRandSetRange( &g_RNG[myid].m_cxRandState, left, right, -1 );

    //create matrix wrapper for 1 floating point value
    float val = 0;
    CxMat mat = cxMat( 1, 1, CX_32FC1, &val );

    //generate value
    cxRand( &g_RNG[myid].m_cxRandState, &mat );

    return val;           
}

double pnlRand(double left, double right)
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
    
    //set distribution type
    g_RNG[myid].m_cxRandState.disttype = CX_RAND_UNI;
    //set range
    cxRandSetRange( &g_RNG[myid].m_cxRandState, left, right, -1 );

    //create matrix wrapper for 1 floating point value
    double val = 0;
    CxMat mat = cxMat( 1, 1, CX_64FC1, &val );

    //generate value
    cxRand( &g_RNG[myid].m_cxRandState, &mat );

    return val;            
} 
           
void pnlRand(int numElem, float* vec, float left, float right)
{
    int myid = PAR_OMP_NUM_CURR_THREAD;

    //set distribution type
    g_RNG[myid].m_cxRandState.disttype = CX_RAND_UNI;
    //set range
    cxRandSetRange( &g_RNG[myid].m_cxRandState, left, right, -1 );

    //create matrix wrapper for 1 floating point value
    CxMat mat = cxMat( 1, numElem, CX_32FC1, vec );

    //generate value
    cxRand( &g_RNG[myid].m_cxRandState, &mat );
}

void pnlRand(int numElem, double* vec, double left, double right)
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
    
    //set distribution type
    g_RNG[myid].m_cxRandState.disttype = CX_RAND_UNI;
    //set range
    cxRandSetRange( &g_RNG[myid].m_cxRandState, left, right, -1 );

    //create matrix wrapper for 1 floating point value
    CxMat mat = cxMat( 1, numElem, CX_64FC1, vec );

    //generate value
    cxRand( &g_RNG[myid].m_cxRandState, &mat );
}

float pnlRandNormal( float mean, float sigma )
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
    
    //set distribution type
    g_RNG[myid].m_cxRandState.disttype = CX_RAND_NORMAL;
    //set range
    cxRandSetRange( &g_RNG[myid].m_cxRandState, (float)sqrt(sigma), mean, -1 );

    //create matrix wrapper for 1 floating point value
    float val = 0;
    CxMat mat = cxMat( 1, 1, CX_32FC1, &val );

    //generate value
    cxRand( &g_RNG[myid].m_cxRandState, &mat );

    return val;
}

double pnlRandNormal( double mean, double sigma )
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
    
    //set distribution type
    g_RNG[myid].m_cxRandState.disttype = CX_RAND_NORMAL;
    //set range
    cxRandSetRange( &g_RNG[myid].m_cxRandState, sqrt(sigma), mean, -1 );

    //create matrix wrapper for 1 floating point value
    double val = 0;
    CxMat mat = cxMat( 1, 1, CX_32FC1, &val );

    //generate value
    cxRand( &g_RNG[myid].m_cxRandState, &mat );

    return val;           
}

void pnlRandNormal( int numElem, float* vec, float mean, float sigma )
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
    
    //set distribution type
    g_RNG[myid].m_cxRandState.disttype = CX_RAND_NORMAL;
    //set range
    cxRandSetRange( &g_RNG[myid].m_cxRandState, (float)sqrt(sigma), mean, -1 );

    //create matrix wrapper for 1 floating point value
    CxMat mat = cxMat( 1, numElem, CX_32FC1, vec );

    //generate value
    cxRand( &g_RNG[myid].m_cxRandState, &mat );
}
void pnlRandNormal( int numElem, double* vec, double mean, double sigma )
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
    
    //set distribution type
    g_RNG[myid].m_cxRandState.disttype = CX_RAND_NORMAL;
    //set range
    cxRandSetRange( &g_RNG[myid].m_cxRandState, sqrt(sigma), mean, -1 );

    //create matrix wrapper for 1 floating point value
    CxMat mat = cxMat( 1, numElem, CX_64FC1, vec );

    //generate value
    cxRand( &g_RNG[myid].m_cxRandState, &mat );
}   

PNL_END
#endif

PNL_BEGIN

void pnlRandNormal(floatVector* vls, floatVector &mean, floatVector &sigma )
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
    
    int nEl = mean.size();
    vls->resize(nEl);
    if( nEl > 1 )
    {
        floatVector normVls(nEl);
        //vsRngGaussian( VSL_METHOD_SGAUSSIAN_BOXMULLER2, g_RNG[myid].m_vslStream, nEl, &normVls.front(), 0, 1 );
        pnlRandNormal(nEl,&normVls.front(), 0, 1);
        
        CxMat matNormVls  = cxMat( nEl, 1, CX_32FC1, &normVls.front() ); 
        
        CxMat matMean = cxMat( nEl, 1, CX_32FC1, &mean.front() );
        CxMat matCov  = cxMat( nEl, nEl, CX_32FC1, &sigma.front() );
        
        floatVector evecData( nEl*nEl );
        CxMat evecMat = cxMat( nEl, nEl, CX_32FC1, &evecData.front() );
        
        floatVector evalData( nEl*nEl );
        CxMat evalMat = cxMat( nEl, nEl,  CX_32FC1, &evalData.front() );
        
        
        cxSVD( &matCov, &evalMat, &evecMat,  NULL, CX_SVD_MODIFY_A   ); 
        
        int i;
        for( i = 0; i < nEl; i++)
        {
            evalData[i*(nEl+1)] = float( sqrt( evalData[i*(nEl+1)] ) );
        }
        
        
        CxMat *prodMat1 = cxCreateMat( nEl, nEl, CX_32FC1 );
        cxMatMul( &evecMat, &evalMat, prodMat1 );
        
        
        CxMat matResultVls  = cxMat( nEl, 1, CX_32FC1, &(vls->front()) ); 
        
        cxMatMulAdd( prodMat1, &matNormVls, &matMean, &matResultVls );
        cxReleaseMat( &prodMat1 );
        
    }
    else
    {
        
        //vsRngGaussian( VSL_METHOD_SGAUSSIAN_BOXMULLER2, g_RNG[myid].m_vslStream, 1,
        //    &(vls->front()), mean.front(), sigma.front() );
        vls->front() = pnlRandNormal( mean.front(), sigma.front() );
    }
}

void pnlRandNormal(doubleVector* vls, doubleVector &mean, doubleVector &sigma )
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
    
    int nEl = mean.size();
    vls->resize(nEl);
    if( nEl > 1 )
    {
        doubleVector normVls(nEl);
        pnlRandNormal(nEl,&normVls.front(), 0, 1);
        
        CxMat matNormVls  = cxMat( nEl, 1, CX_64FC1, &normVls.front() ); 
        
        CxMat matMean = cxMat( nEl, 1, CX_64FC1, &mean.front() );
        CxMat matCov  = cxMat( nEl, nEl, CX_64FC1, &sigma.front() );
        
        doubleVector evecData( nEl*nEl );
        CxMat evecMat = cxMat( nEl, nEl, CX_64FC1, &evecData.front() );
        
        doubleVector evalData( nEl*nEl );
        CxMat evalMat = cxMat( nEl, nEl,  CX_64FC1, &evalData.front() );
        
        
        cxSVD( &matCov, &evalMat, &evecMat,  NULL, CX_SVD_MODIFY_A   ); 
        
        int i;
        for( i = 0; i < nEl; i++)
        {
            evalData[i*(nEl+1)] =  sqrt( evalData[i*(nEl+1)] );
        }
        
        
        CxMat *prodMat1 = cxCreateMat( nEl, nEl, CX_64FC1 );
        cxMatMul( &evecMat, &evalMat, prodMat1 );
        
        
        CxMat matResultVls  = cxMat( nEl, 1, CX_64FC1, &(vls->front()) ); 
        
        cxMatMulAdd( prodMat1, &matNormVls, &matMean, &matResultVls );
        cxReleaseMat( &prodMat1 );
        
    }
    else
    {
        
        //vsRngGaussian( VSL_METHOD_SGAUSSIAN_BOXMULLER2, g_RNG[myid].m_vslStream, 1,
        //    &(vls->front()), mean.front(), sigma.front() );
        vls->front() = pnlRandNormal( mean.front(), sigma.front() );
    }
}

PNL_END

