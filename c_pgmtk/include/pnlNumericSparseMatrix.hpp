/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlNumericSparseMatrix.hpp                                  //
//                                                                         //
//  Purpose:   CNumericSparseMatrix template class definition &            //
//             implementation                                              //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLNUMERICSparseMATRIX_HPP__
#define __PNLNUMERICSparseMATRIX_HPP__

#include "pnliNumericSparseMatrix.hpp"
#include <float.h>
#include <math.h>

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

template <class Type> class PNL_API CNumericSparseMatrix : 
            public iCNumericSparseMatrix<Type>
{
public:
    static CNumericSparseMatrix<Type>* Create( int dim, const int *range,
            int Clamp = 0 );
    virtual CMatrix<Type>* CreateEmptyMatrix( int dim, const int *range,
                int Clamp, Type defaultVal = Type(0) ) const;
    ~CNumericSparseMatrix(){};

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CNumericSparseMatrix< int >::GetStaticTypeInfo();
    }
#endif
protected:
    CNumericSparseMatrix( int dim, const int *range, int Clamp );
    CNumericSparseMatrix( const CNumericSparseMatrix<Type> & inputMat );
    CNumericSparseMatrix( CxSparseMat* p_sparse );

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
};

#ifndef SWIG

template <class Type>
CNumericSparseMatrix<Type>* CNumericSparseMatrix<Type>::Create( int dim,
                          const int *range, int Clamp )
{
     PNL_THROW( CInvalidOperation,
        "Numeric matrices assume float or double types only!" )
     return NULL;
}

template <class Type>
CMatrix<Type>* CNumericSparseMatrix<Type>::CreateEmptyMatrix( int dim,
                            const int* range, int Clamp, Type defaultVal)const
{
    return CNumericSparseMatrix<Type>::Create( dim, range, Clamp);
}

template <class Type>
CNumericSparseMatrix<Type>::CNumericSparseMatrix( int dim, const int *range,
                                                 int Clamp )
:iCNumericSparseMatrix<Type>( dim, range, Clamp )
{
}

template <class Type>
CNumericSparseMatrix<Type>::CNumericSparseMatrix( 
                               const CNumericSparseMatrix<Type> & inputMat )
                               :iCNumericSparseMatrix<Type>( inputMat )
{
}

template <class Type>
CNumericSparseMatrix<Type>::CNumericSparseMatrix( CxSparseMat* p_sparse)
:iCNumericSparseMatrix<Type>( p_sparse )
{
}
#endif
///////////////////////////////////////////////////////////////////////////
//specialization for float
template <> class PNL_API CNumericSparseMatrix<float>:public iCNumericSparseMatrix<float>
{
public:
    
    float SumAll( int byAbsValue = 0 ) const;
    static CNumericSparseMatrix<float>* Create( int dim, const int *range,
        int Clamp = 0 );
    ~CNumericSparseMatrix(){};

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CNumericSparseMatrix< int >::GetStaticTypeInfo();
    }
#endif
protected:
    CNumericSparseMatrix(int dim, const int *range, int Clamp);

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
};

#ifdef PNL_RTTI
template <class Type> 
const CPNLType CNumericSparseMatrix< Type > ::m_TypeInfo = CPNLType("CNumericSparseMatrix", &(iCNumericSparseMatrix< Type >::m_TypeInfo));

#endif  
            
            PNL_END
                
#endif //__PNLNUMERICSparseMATRIX_HPP__
