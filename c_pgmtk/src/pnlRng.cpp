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
#else //OpenCV's (PNL default)

#include "cxcore.h"
//initialization of global RNG

PNL_BEGIN

class CRNG
{
public:
    CvRNG m_cvRandState;
public:
    CRNG()
    {
        m_cvRandState = cvRNG(0 /*seed*/);
    }
    ~CRNG() {};
};

//global variable
CRNG g_RNG[NUMBER_OF_HEAPS];

void pnlSeed(int s)
{
    g_RNG[PAR_OMP_NUM_CURR_THREAD].m_cvRandState = cvRNG(s);
}

//generate uniform integer distribution in the range [left,right]
int pnlRand(int left, int right)
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
    
    PNL_CHECK_LEFT_BORDER( right, left );

    //create matrix wrapper for 1 integer (assuming 32-bit ints)
    int result = 0;
    CvMat mat = cvMat(1, 1, CV_32SC1, &result);

    //generate value
    cvRandArr(&g_RNG[myid].m_cvRandState, &mat, CV_RAND_UNI, 
	      cvRealScalar(left), cvRealScalar(right + 1));
    return result;
}

void pnlRand(int numElem, int* vec, int left, int right)
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
   
    PNL_CHECK_LEFT_BORDER(right, left);

    if (numElem > 0)
    {
	//create matrix wrapper for numElem integers (assuming 32-bit ints)
	CvMat mat = cvMat(1, numElem, CV_32SC1, vec);
	
	//generate values
	cvRandArr(&g_RNG[myid].m_cvRandState, &mat, CV_RAND_UNI,
		  cvRealScalar(left), cvRealScalar(right + 1));
    }
}

//generate uniform distribution (single precision) on [left, right) range
float pnlRand(float left, float right)
{
    int myid = PAR_OMP_NUM_CURR_THREAD;

    PNL_CHECK_LEFT_BORDER(right, left);

    //create matrix wrapper for 1 floating point value
    double val = 0;
    CvMat mat = cvMat( 1, 1, CV_64FC1, &val );

    //generate value
    cvRandArr( &g_RNG[myid].m_cvRandState, &mat, CV_RAND_UNI,
	       cvRealScalar(left), cvRealScalar(right) );

    return val;            
} 
           
void pnlRand(int numElem, float* vec, float left, float right)
{
    int myid = PAR_OMP_NUM_CURR_THREAD;

    PNL_CHECK_LEFT_BORDER(right, left);

    if (numElem > 0)
    {
	//create matrix wrapper for numElem floating point values
	CvMat mat = cvMat( 1, numElem, CV_32FC1, vec );
	
	//generate values
	cvRandArr( &g_RNG[myid].m_cvRandState, &mat, CV_RAND_UNI, 
		   cvRealScalar(left), cvRealScalar(right));
    }
}

void pnlRand(int numElem, double* vec, double left, double right)
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
    
    PNL_CHECK_LEFT_BORDER(right, left);

    if (numElem > 0)
    {
	//create matrix wrapper for numElem floating point values
	CvMat mat = cvMat( 1, numElem, CV_64FC1, vec );
	
	//generate values
	cvRandArr( &g_RNG[myid].m_cvRandState, &mat, CV_RAND_UNI, 
		   cvRealScalar(left), cvRealScalar(right));
    }
}

float pnlRandNormal( float mean, float variance )
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
    
    //create matrix wrapper for 1 floating point value
    float val = 0;
    CvMat mat = cvMat( 1, 1, CV_32FC1, &val );

    //generate value
    cvRandArr( &g_RNG[myid].m_cvRandState, &mat, CV_RAND_NORMAL, 
	    cvRealScalar(mean), cvRealScalar(sqrt(variance)));

    return val;
}

double pnlRandNormal( double mean, double variance )
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
    
    //create matrix wrapper for 1 floating point value
    double val = 0;
    CvMat mat = cvMat( 1, 1, CV_64FC1, &val );

    //generate value
    cvRandArr( &g_RNG[myid].m_cvRandState, &mat, CV_RAND_NORMAL, 
	    cvRealScalar(mean), cvRealScalar(sqrt(variance)));

    return val;           
}

void pnlRandNormal( int numElem, float* vec, float mean, float variance )
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
    
    if (numElem > 0)
    {
	//create matrix wrapper for 1 floating point value
	CvMat mat = cvMat( 1, numElem, CV_32FC1, vec );
	
	//generate value
	cvRandArr( &g_RNG[myid].m_cvRandState, &mat, CV_RAND_NORMAL, 
		   cvRealScalar(mean), cvRealScalar(sqrt(variance)));
    }
}

void pnlRandNormal( int numElem, double* vec, double mean, double variance )
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
    
    if (numElem > 0)
    {
	//create matrix wrapper for 1 floating point value
	CvMat mat = cvMat( 1, numElem, CV_64FC1, vec );
	
	//generate value
	cvRandArr( &g_RNG[myid].m_cvRandState, &mat, CV_RAND_NORMAL, 
		   cvRealScalar(mean), cvRealScalar(sqrt(variance)));
    }
}   

PNL_END
#endif

PNL_BEGIN

void pnlRandNormal(floatVector* vls, floatVector &mean, floatVector &cov )
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
    
    int nEl = mean.size();
    vls->resize(nEl);
    if( nEl > 1 )
    {
        floatVector normVls(nEl);
        //vsRngGaussian( VSL_METHOD_SGAUSSIAN_BOXMULLER2, g_RNG[myid].m_vslStream, nEl, &normVls.front(), 0, 1 );
        pnlRandNormal(nEl,&normVls.front(), 0, 1);
        
        CvMat matNormVls  = cvMat( nEl, 1, CV_32FC1, &normVls.front() ); 
        
        CvMat matMean = cvMat( nEl, 1, CV_32FC1, &mean.front() );
        CvMat matCov  = cvMat( nEl, nEl, CV_32FC1, &cov.front() );
        
        floatVector evecData( nEl*nEl );
        CvMat evecMat = cvMat( nEl, nEl, CV_32FC1, &evecData.front() );
        
        floatVector evalData( nEl*nEl );
        CvMat evalMat = cvMat( nEl, nEl,  CV_32FC1, &evalData.front() );
        
        
        cvSVD( &matCov, &evalMat, &evecMat,  NULL, CV_SVD_MODIFY_A   ); 
        
        int i;
        for( i = 0; i < nEl; i++)
        {
            evalData[i*(nEl+1)] = float( sqrt( evalData[i*(nEl+1)] ) );
        }
        
        
        CvMat *prodMat1 = cvCreateMat( nEl, nEl, CV_32FC1 );
        cvMatMul( &evecMat, &evalMat, prodMat1 );
        
        
        CvMat matResultVls  = cvMat( nEl, 1, CV_32FC1, &(vls->front()) ); 
        
        cvMatMulAdd( prodMat1, &matNormVls, &matMean, &matResultVls );
        cvReleaseMat( &prodMat1 );
        
    }
    else if ( nEl > 0 )
    {
        
        //vsRngGaussian( VSL_METHOD_SGAUSSIAN_BOXMULLER2, g_RNG[myid].m_vslStream, 1,
        //    &(vls->front()), mean.front(), cov.front() );
        vls->front() = pnlRandNormal( mean.front(), cov.front() );
    }
}

void pnlRandNormal(doubleVector* vls, doubleVector &mean, doubleVector &cov )
{
    int myid = PAR_OMP_NUM_CURR_THREAD;
    
    int nEl = mean.size();
    vls->resize(nEl);
    if( nEl > 1 )
    {
        doubleVector normVls(nEl);
        pnlRandNormal(nEl,&normVls.front(), 0, 1);
        
        CvMat matNormVls  = cvMat( nEl, 1, CV_64FC1, &normVls.front() ); 
        
        CvMat matMean = cvMat( nEl, 1, CV_64FC1, &mean.front() );
        CvMat matCov  = cvMat( nEl, nEl, CV_64FC1, &cov.front() );
        
        doubleVector evecData( nEl*nEl );
        CvMat evecMat = cvMat( nEl, nEl, CV_64FC1, &evecData.front() );
        
        doubleVector evalData( nEl*nEl );
        CvMat evalMat = cvMat( nEl, nEl,  CV_64FC1, &evalData.front() );
        
        
        cvSVD( &matCov, &evalMat, &evecMat,  NULL, CV_SVD_MODIFY_A   ); 
        
        int i;
        for( i = 0; i < nEl; i++)
        {
            evalData[i*(nEl+1)] =  sqrt( evalData[i*(nEl+1)] );
        }
        
        
        CvMat *prodMat1 = cvCreateMat( nEl, nEl, CV_64FC1 );
        cvMatMul( &evecMat, &evalMat, prodMat1 );
        
        
        CvMat matResultVls  = cvMat( nEl, 1, CV_64FC1, &(vls->front()) ); 
        
        cvMatMulAdd( prodMat1, &matNormVls, &matMean, &matResultVls );
        cvReleaseMat( &prodMat1 );
        
    }
    else if ( nEl > 0 )
    {
        
        //vsRngGaussian( VSL_METHOD_SGAUSSIAN_BOXMULLER2, g_RNG[myid].m_vslStream, 1,
        //    &(vls->front()), mean.front(), cov.front() );
        vls->front() = pnlRandNormal( mean.front(), cov.front() );
    }
}

PNL_END

