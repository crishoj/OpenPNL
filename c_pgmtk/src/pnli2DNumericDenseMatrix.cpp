/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnli2DNumericDenseMatrix.cpp                                //
//                                                                         //
//  Purpose:   Implementation of the algorithm                             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnli2DNumericDenseMatrix.hpp"

PNL_USING
/*
PNL_BEGIN


template <>
iC2DNumericDenseMatrix<float>* iC2DNumericDenseMatrix<float>::Create( 
                       const int* lineSizes, const float *data, int Clamp )
{
    PNL_CHECK_IS_NULL_POINTER( lineSizes );
    PNL_CHECK_LEFT_BORDER( Clamp, 0 );
    PNL_CHECK_IS_NULL_POINTER( data );

    int i = 0;
    for( i = 0; i < 2; i++ )
    {
        if( lineSizes[i] < 0 )
        {
            PNL_THROW( COutOfRange, "range is negative" )
        }
    }
    PNL_CHECK_IS_NULL_POINTER( data );
    iC2DNumericDenseMatrix<float> *pxMatrix = 
                new iC2DNumericDenseMatrix<float>( 2, lineSizes, data, (Clamp>0));
    return pxMatrix;
}

template <>
iC2DNumericDenseMatrix<double>* iC2DNumericDenseMatrix<double>::Create( 
                       const int* lineSizes, const double *data, int Clamp )
{
    PNL_CHECK_IS_NULL_POINTER( lineSizes );
    PNL_CHECK_LEFT_BORDER( Clamp, 0 );
    PNL_CHECK_IS_NULL_POINTER( data );

    int i = 0;
    for( i = 0; i < 2; i++ )
    {
        if( lineSizes[i] < 0 )
        {
            PNL_THROW( COutOfRange, "range is negative" )
        }
    }
    PNL_CHECK_IS_NULL_POINTER( data );
    iC2DNumericDenseMatrix<double> *pxMatrix = 
                new iC2DNumericDenseMatrix<double>( 2, lineSizes, data, (Clamp>0));
    return pxMatrix;
}



template <>
iC2DNumericDenseMatrix<float>* iC2DNumericDenseMatrix<float>
::CreateIdentityMatrix(int lineSize)
{
    PNL_CHECK_LEFT_BORDER( lineSize, 1 )
    int i;
    floatVector data( lineSize*lineSize, 0.0f );
    for( i = 0; i < lineSize; i++ )
    {
        data[i+i*lineSize] = 1.0f;
    }
    intVector ranges( 2, lineSize );
    iC2DNumericDenseMatrix<float>* resMat = iC2DNumericDenseMatrix<float>::Create( 
        &ranges.front(), &data.front(), 0 );
    PNL_CHECK_IF_MEMORY_ALLOCATED( resMat );
    return resMat;
}

template <>
iC2DNumericDenseMatrix<double>* iC2DNumericDenseMatrix<double>
::CreateIdentityMatrix(int lineSize)
{
    PNL_CHECK_LEFT_BORDER( lineSize, 1 )
    int i;
    doubleVector data( lineSize*lineSize, 0.0 );
    for( i = 0; i < lineSize; i++ )
    {
        data[i+i*lineSize] = 1.0;
    }
    intVector ranges( 2, lineSize );
    iC2DNumericDenseMatrix<double>* resMat = iC2DNumericDenseMatrix<double>::Create( 
        &ranges.front(), &data.front(), 0 );
    PNL_CHECK_IF_MEMORY_ALLOCATED( resMat );
    return resMat;
}

PNL_END

PNL_USING

template <>
int iC2DNumericDenseMatrix<float>::IsSymmetric(float epsilon) const
{
    PNL_CHECK_LEFT_BORDER( epsilon, 0.0f );
    if( m_Range[0] != m_Range[1] )
    {
        return 0;
    }
    int i,j;
    int size1 = m_Range[0];
    int size2 = m_Range[1];
    for( i = 0; i < size1 ; i++)
    {
        for( j = 0; j <size2; j++ )
        {
            if( fabs(m_Table[i*size1+j] - m_Table[j*size1+i] ) >= epsilon)
            {
                return 0;
            }
        }
    }
    return 1;
}

template <>
int iC2DNumericDenseMatrix<double>::IsSymmetric(double epsilon) const
{
    PNL_CHECK_LEFT_BORDER( epsilon, 0.0f );
    if( m_Range[0] != m_Range[1] )
    {
        return 0;
    }
    int i,j;
    int size1 = m_Range[0];
    int size2 = m_Range[1];
    for( i = 0; i < size1 ; i++)
    {
        for( j = 0; j <size2; j++ )
        {
            if( fabs(m_Table[i*size1+j] - m_Table[j*size1+i] )>=epsilon)
            {
                return 0;
            }
        }
    }
    return 1;
}


template <>
void iC2DNumericDenseMatrix<float>::icvSVBkSb(int m, int n, const float* w,
                    const float* uT, int lduT, int nu,
                    const float* vT, int ldvT,
                    const float* b, int ldb, int nb,
                    float* x, float* buffer)const
{
    icvSVBkSb_32f( m, n, w, uT, lduT, nu, vT, ldvT, b, ldb, nb, x, buffer );
}

template <>
void iC2DNumericDenseMatrix<double>::icvSVBkSb(int m, int n, const double* w,
                    const double* uT, int lduT, int nu,
                    const double* vT, int ldvT,
                    const double* b, int ldb, int nb,
                    double* x, double* buffer)const
{
    icvSVBkSb_64f( m, n, w, uT, lduT, nu, vT, ldvT, b, ldb, nb, x, buffer );
}

template<>
void iC2DNumericDenseMatrix<double>::icvSVD( double* a, int lda, int m, int n,
                 double* w,
                 double* uT, int lduT, int nu,
                 double* vT, int ldvT,
                 double* buffer )const
{
    icvSVD_64f( a, lda, m, n, w, uT, lduT, nu, vT, ldvT, buffer );
}

template<>
void iC2DNumericDenseMatrix<float>::icvSVD( float* a, int lda, int m, int n,
                 float* w,
                 float* uT, int lduT, int nu,
                 float* vT, int ldvT,
                 float* buffer )const
{
    icvSVD_32f( a, lda, m, n, w, uT, lduT, nu, vT, ldvT, buffer );
}*/

#ifdef PNL_RTTI
template<>
const CPNLType &iC2DNumericDenseMatrix< float >::GetStaticTypeInfo()
{
  return iC2DNumericDenseMatrix< float >::m_TypeInfo;
}
#endif