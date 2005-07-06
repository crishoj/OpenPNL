/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlMatrixIterator.hpp                                       //
//                                                                         //
//  Purpose:   CMatrixIterator template class and its derived subclasses   //
//             definitions & implementations                               //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLMATRIXITERATOR_HPP__
#define __PNLMATRIXITERATOR_HPP__

#include "cxcore.h"

//#include "pnlSparseMatrix.hpp"
//#include "pnlDenseMatrix.hpp"
#include "pnlObject.hpp"
#include "pnlException.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN
template <class Type> class PNL_API CMatrixIterator : public CPNLBase
{
    public:
//        inline Type* GetCurrent();
        inline const Type* GetCurrent() const;
        virtual void Iteration() = 0;
        virtual ~CMatrixIterator(){}

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CMatrixIterator< int >::GetStaticTypeInfo();
    }
#endif
    protected:
        const Type* m_Current;
        CMatrixIterator(const Type* current);

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
};

template <class Type>
CMatrixIterator<Type>::CMatrixIterator(const Type* current):m_Current(current)
{
}
/*template <class Type>
inline Type* CMatrixIterator<Type>::GetCurrent()
{
    return m_Current;
}
*/
template <class Type>
inline const Type* CMatrixIterator<Type>::GetCurrent() const
{
    return m_Current;
}

#ifdef PNL_RTTI
template <class Type> 
const CPNLType CMatrixIterator<Type> ::m_TypeInfo = CPNLType("CMatrixIterator", &(CPNLBase::m_TypeInfo));

#endif

template <class Type> class PNL_API CDenseMatrixIterator : public CMatrixIterator<Type>
{
    public:
        static CDenseMatrixIterator<Type>* Create(const Type* first );
        void Iteration();
        ~CDenseMatrixIterator(){}

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CDenseMatrixIterator< int >::GetStaticTypeInfo();
    }
#endif
    protected:
        CDenseMatrixIterator(const Type* first);

#if GCC_VERSION >= 30400
	using CMatrixIterator<Type>::m_Current;
#endif

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
    private:
        Type* m_endOfData;
};

template <class Type>
CDenseMatrixIterator<Type>* CDenseMatrixIterator<Type>::Create(const Type* first)
{
    PNL_CHECK_IS_NULL_POINTER(first);
    CDenseMatrixIterator<Type>* res = new CDenseMatrixIterator<Type>(first);
    PNL_CHECK_IF_MEMORY_ALLOCATED(res);
    return res;
}

template <class Type>
CDenseMatrixIterator<Type>::CDenseMatrixIterator(const Type* first)
:CMatrixIterator<Type>(first)
{
}

template <class Type>
inline void CDenseMatrixIterator<Type>::Iteration()
{
    m_Current++;
}

#ifdef PNL_RTTI
template <class Type> 
const CPNLType CDenseMatrixIterator<Type> ::m_TypeInfo = CPNLType("CDenseMatrixIterator", &(CMatrixIterator<Type>::m_TypeInfo));

#endif

template <class Type> class PNL_API CSparseMatrixIterator : public CMatrixIterator<Type>
{
public:
    static CSparseMatrixIterator<Type>* Create( CxSparseMat* matrix );
    void Iteration();
    inline int IsNodeHere() const;
    inline int* Index() const;
    ~CSparseMatrixIterator(){}

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CSparseMatrixIterator< int >::GetStaticTypeInfo();
    }
#endif
protected:
    CSparseMatrixIterator( CxSparseMat* matrix );

#if GCC_VERSION >= 30400
    using CMatrixIterator<Type>::m_Current;
#endif

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
    CxSparseNode* m_currNode;
    CxSparseMatIterator m_cvSparseIter;
    CxSparseMat* m_CvMatrix;
};

template <class Type>
CSparseMatrixIterator<Type>* CSparseMatrixIterator<Type>::Create(
                                                       CxSparseMat* matrix)
{
    PNL_CHECK_IS_NULL_POINTER( matrix );
    CSparseMatrixIterator<Type>* res = new CSparseMatrixIterator<Type>(matrix);
    PNL_CHECK_IF_MEMORY_ALLOCATED(res);
    return res;
}

template <class Type>
inline void CSparseMatrixIterator<Type>::Iteration()
{
    m_currNode = cxGetNextSparseNode( &m_cvSparseIter );
    m_Current = (Type*)CX_NODE_VAL( m_CvMatrix, m_currNode );
}

template <class Type>
int CSparseMatrixIterator<Type>::IsNodeHere() const
{
    if( m_currNode )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

template <class Type>
int* CSparseMatrixIterator<Type>::Index() const
{
    return CX_NODE_IDX( m_CvMatrix, m_currNode );
}

template <class Type>
CSparseMatrixIterator<Type>::CSparseMatrixIterator( CxSparseMat* matrix)
                    :CMatrixIterator<Type>(NULL)
{
    m_CvMatrix = matrix;
    m_currNode = cxInitSparseMatIterator( m_CvMatrix, &m_cvSparseIter );    
    m_Current = (Type*)CX_NODE_VAL( matrix, m_currNode );
}

#ifdef PNL_RTTI
template <class Type> 
const CPNLType CSparseMatrixIterator<Type> ::m_TypeInfo = CPNLType("CSparseMatrixIterator", &(CMatrixIterator<Type>::m_TypeInfo));

#endif

PNL_END

#endif //__PNLMATRIXITERATOR_HPP__
