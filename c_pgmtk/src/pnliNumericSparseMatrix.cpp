/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnliNumericSparseMatrix.cpp                                 //
//                                                                         //
//  Purpose:   CNumericSparseMatrix class member functions implementation  //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlNumericSparseMatrix.hpp"
#include <math.h>

PNL_BEGIN
/////////////////////////////////////////////////////////////////////////////
//float CNumericSparseMatrix

CNumericSparseMatrix<float>* CNumericSparseMatrix<float>::Create( int dim,
                                const int *range, int Clamp)
{
    PNL_CHECK_LEFT_BORDER( dim, 1 );
    PNL_CHECK_IS_NULL_POINTER( range );
    PNL_CHECK_LEFT_BORDER( Clamp, 0 );

    int i = 0;
    for( i = 0; i < dim; i++ )
    {
        if( range[i] <= 0 )
        {
            PNL_THROW( COutOfRange, "range is negative" )
        }
    }
    CNumericSparseMatrix<float> *pxMatrix = new CNumericSparseMatrix<float>(
                    dim, range, Clamp );
    return pxMatrix;
}


/*
template <>
CNumericSparseMatrix<double>* CNumericSparseMatrix<double>::Create( int dim,
                                const int *range, int Clamp)
{
    PNL_CHECK_LEFT_BORDER( dim, 1 );
    PNL_CHECK_IS_NULL_POINTER( range );
    PNL_CHECK_LEFT_BORDER( Clamp, 0 );

    int i = 0;
    for( i = 0; i < dim; i++ )
    {
        if( range[i] < 0 )
        {
            PNL_THROW( COutOfRange, "range is negative" )
        }
    }
    iCNumericSparseMatrix<double> *pxMatrix = new iCNumericSparseMatrix<double>(
                    dim, range, Clamp );
    return pxMatrix;
}
*/



/*
template<>
double iCNumericSparseMatrix<double>::SumAll( int byAbsValue )const
{
    CxSparseMatIterator iterator;
    CxSparseNode* node;
    const iCNumericSparseMatrix<double>* self = this;
    CxSparseMat* mat = const_cast<CxSparseMat*>(self->GetCxSparseMat());
    double sum = 0;
    for( node = cxInitSparseMatIterator( mat, &iterator );
                 node != 0; node = cxGetNextSparseNode( &iterator ))
    {
        //we put data from small matrix to needed positions in bigData
        void* val = CX_NODE_VAL(  mat, node );
        double valT = *(double*)val;
        if( byAbsValue )
        {
            sum += fabs( valT );
        }
        sum += valT;
    }
    return sum;
}
*/
float CNumericSparseMatrix<float>::SumAll( int byAbsValue )const
{
    CxSparseMatIterator iterator;
    CxSparseNode* node;
    const CNumericSparseMatrix<float>* self = this;
    const CxSparseMat* mat = const_cast<CxSparseMat*>(self->GetCxSparseMat());
    float sum = 0;
    for( node = cxInitSparseMatIterator( mat, &iterator );
                 node != 0; node = cxGetNextSparseNode( &iterator ))
    {
        //we put data from small matrix to needed positions in bigData
        void* val = CX_NODE_VAL(  mat, node );
        float valT = *(float*)val;
        if( byAbsValue )
        {
            sum += (float)fabs( valT );
        }
        sum += valT;
    }
    return sum;
}

CNumericSparseMatrix<float>::CNumericSparseMatrix(int dim, const int *range,
                                                 int Clamp  ):
                    iCNumericSparseMatrix<float>(dim, range, Clamp)
{
}

#ifdef PNL_RTTI
template<>
const CPNLType &iCNumericSparseMatrix< int >::GetStaticTypeInfo()
{
  return iCNumericSparseMatrix< int >::m_TypeInfo;
}

template<>
const CPNLType &CNumericSparseMatrix< int >::GetStaticTypeInfo()
{
  return CNumericSparseMatrix< int >::m_TypeInfo;
}

template <> 
const CPNLType CNumericSparseMatrix< float > ::m_TypeInfo = CPNLType("CNumericSparseMatrix", &(iCNumericSparseMatrix< Type >::m_TypeInfo));

#endif  
PNL_END
