/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlNumericDenseMatrix.hpp                                   //
//                                                                         //
//  Purpose:   CNumericDenseMatrix template class definition &             //
//             implementation                                              //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLNUMERICDENSEMATRIX_HPP__
#define __PNLNUMERICDENSEMATRIX_HPP__

#include "pnliNumericDenseMatrix.hpp"
//#include "pnlNumericSparseMatrix.hpp"
#include <float.h>
#include <math.h>

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN
template <class Type> class PNL_API CNumericDenseMatrix : 
            public iCNumericDenseMatrix<Type>
{
public:
    static CNumericDenseMatrix<Type>* Create( int dim, const int *range,
            const Type *data, int Clamp = 0 );
    virtual CMatrix<Type>* CreateEmptyMatrix( int dim, const int *range,
     int Clamp, Type defaultVal = Type(0) ) const;
    ~CNumericDenseMatrix(){};
#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CNumericDenseMatrix< int >::GetStaticTypeInfo();
    }
#endif
protected:
    CNumericDenseMatrix(int dim, const int *range, const Type *data, int Clamp);
    CNumericDenseMatrix( const CNumericDenseMatrix<Type> & inputMat );
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
};

#ifndef SWIG

template <class Type>
CNumericDenseMatrix<Type>* CNumericDenseMatrix<Type>::Create( int dim,
                          const int *range, const Type *data, int Clamp )
{
     PNL_THROW( CInvalidOperation,
        "Numeric matrices assume float or double types only!" )
     return NULL;
}

template <class Type>
CMatrix<Type>* CNumericDenseMatrix<Type>::CreateEmptyMatrix( int dim,
                                              const int* range, int Clamp, 
                                              Type defaultVal )const
{
    PNL_CHECK_LEFT_BORDER( dim, 1 );
    PNL_CHECK_IS_NULL_POINTER( range );
    PNL_CHECK_LEFT_BORDER( Clamp, 0 );

    int i = 0;
    int size = 1;
    for( i = 0; i < dim; i++ )
    {
        if( range[i] <= 0 )
        {
            PNL_THROW( COutOfRange, "range is negative" );
            size *= range[i];
        }
    }
    pnlVector<Type> data;
    data.assign( size, defaultVal );
    CDenseMatrix<Type> *pxMatrix = CDenseMatrix<Type>::Create( dim,
            range, &data.front(), (Clamp>0));
    return pxMatrix;
}

template <class Type>
CNumericDenseMatrix<Type>::CNumericDenseMatrix( int dim,
                                 const int *range, const Type *data, int Clamp)
      :iCNumericDenseMatrix<Type>( dim, range, data, Clamp )
{
}

template <class Type>
CNumericDenseMatrix<Type>::CNumericDenseMatrix( 
                               const CNumericDenseMatrix<Type> & inputMat )
:iCNumericDenseMatrix<Type>( inputMat )
{
}

#endif


template <> class PNL_API CNumericDenseMatrix<float>:public iCNumericDenseMatrix<float> 
{
public:
    static CNumericDenseMatrix<float>* Create( int dim, const int *range,
            const float *data, int Clamp = 0 );
    ~CNumericDenseMatrix(){};

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CNumericDenseMatrix< int >::GetStaticTypeInfo();
    }
#endif
protected:
    CNumericDenseMatrix(int dim, const int *range, const float *data, int Clamp);
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
};

#ifdef PNL_RTTI
template <class Type>
const CPNLType CNumericDenseMatrix< Type >::m_TypeInfo = CPNLType("CNumericDenseMatrix", &(iCNumericDenseMatrix< Type >::m_TypeInfo));

#endif

PNL_END

#endif //__PNLNUMERICDENSEMATRIX_HPP__
