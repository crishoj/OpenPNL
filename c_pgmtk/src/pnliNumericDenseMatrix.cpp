/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnliNumericDenseMatrix.cpp                                  //
//                                                                         //
//  Purpose:   CnumericDenseMatrix class member functions implementation   //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlNumericDenseMatrix.hpp"

PNL_BEGIN

int margBlockize(int *aDim, int nDim, const int *aRange, int &blockSize,
		 int desired, int limit)
{
    return pnl::blockize( compose_unaries(
	    std::bind1st( subscript< const int, const int *, int >(), aRange ),
	    std::bind1st( subscript< int, int *, int >(), aDim ) ),
	nDim, mul_in_self< int, int, int >(), 1,
	std::bind2nd( std::greater< int >(), desired),
	std::bind2nd( std::greater< int >(), limit), true, &blockSize );
}

CNumericDenseMatrix<float> * CNumericDenseMatrix<float>::Create( int dim, 
                                      const int *range,
                                      const float *data, int Clamp)
{
    PNL_CHECK_LEFT_BORDER( dim, 1 );
    PNL_CHECK_IS_NULL_POINTER( range );
    PNL_CHECK_LEFT_BORDER( Clamp, 0 );
    PNL_CHECK_IS_NULL_POINTER( data );

    int i = 0;
    for( i = 0; i < dim; i++ )
    {
        if( range[i] < 0 )
        {
            PNL_THROW( COutOfRange, "range is negative" )
            break;
        }
    }
    PNL_CHECK_IS_NULL_POINTER( data );
    CNumericDenseMatrix<float> *pxMatrix = new CNumericDenseMatrix( dim,
        range, data, (Clamp>0));
    return pxMatrix;
}

CNumericDenseMatrix<float>::CNumericDenseMatrix( int dim,
                                 const int *range, const float *data, int Clamp)
      :iCNumericDenseMatrix<float>( dim, range, data, Clamp )
{
}

/*
template <>
CNumericDenseMatrix<double>* CNumericDenseMatrix<double>::Create( int dim, 
                                      const int *range,
                                      const double *data, int Clamp)
{
    PNL_CHECK_LEFT_BORDER( dim, 1 );
    PNL_CHECK_IS_NULL_POINTER( range );
    PNL_CHECK_LEFT_BORDER( Clamp, 0 );
    PNL_CHECK_IS_NULL_POINTER( data );

    int i = 0;
    for( i = 0; i < dim; i++ )
    {
        if( range[i] < 0 )
        {
            PNL_THROW( COutOfRange, "range is negative" )
            break;
        }
    }
    PNL_CHECK_IS_NULL_POINTER( data );
    CNumericDenseMatrix<double> *pxMatrix = 
           static_cast<CNumericDenseMatrix<double>*>(CDenseMatrix<double>::
           Create( dim, range, data, (Clamp>0)));
    return pxMatrix;
}
*/

#ifdef PNL_RTII
template<>
const CPNLType &iCNumericDenseMatrix< int >::GetStaticTypeInfo()
{
  return iCNumericDenseMatrix< int >::m_TypeInfo;
}
#endif
PNL_END
