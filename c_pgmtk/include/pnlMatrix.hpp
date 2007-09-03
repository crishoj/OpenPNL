/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlMatrix.hpp                                               //
//                                                                         //
//  Purpose:   CMatrix template class definition & implementation          //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//pnlMatrix.hpp

#ifndef __PNLMATRIX_HPP__
#define __PNLMATRIX_HPP__

#include "pnlReferenceCounter.hpp"
#include "pnlTypeDefs.hpp"
#include "pnlMatrixIterator.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

typedef PNL_API enum 
{
    matTable,
    matMean,
    matCovariance,
    matWeights,
    matH,
    matK,
    matDirichlet,
    matWishartMean,
    matWishartCov

} EMatrixType;

typedef PNL_API enum 
{
    stMatTable,
    stMatMu,
    stMatSigma,
    stMatCoeff,
    stMatWeight
} EStatisticalMatrix;

typedef PNL_API enum 
{
    mcBase,
    mcSparse,
    mcDense,
    mcNumericDense,
    mcNumericSparse,
    mc2DNumericDense,
    mc2DNumericSparse
} EMatrixClass;

template <class Type> class CDenseMatrix;
template <class Type> class CSparseMatrix;

template <class Type> class PNL_API CMatrix : public CReferenceCounter
{
public:
    virtual CMatrix<Type>* CreateEmptyMatrix( int dim, const int *range,
        int Clamp, Type defaultVal = Type(0) )const = 0;
    //method the same as operator = but virtual
    virtual void SetDataFromOtherMatrix( const CMatrix<Type>* matInput ) = 0;
    virtual CMatrix<Type>* Clone() const = 0;
    virtual int GetNumberDims() const = 0;
    virtual void GetRanges(int *numOfDimsOut, const int **rangesOut) const = 0;
    virtual EMatrixClass GetMatrixClass() const;
    //methods for conversion from other type
    //if requered type is the same as type of argument - return copy of object
    virtual CDenseMatrix<Type>* ConvertToDense() const = 0;
    virtual CSparseMatrix<Type>* ConvertToSparse() const = 0;
    //auxillary methods to work with matrix
   //methods for viewing/change data
    virtual Type GetElementByIndexes(const int *multidimindexes) const = 0;
    virtual void SetElementByIndexes(Type value, const int *multidimindexes) = 0;
    //these functions set matrix element equal value (for
    //given multidimensional indexes or linear index)
    virtual CMatrix<Type>* ReduceOp( const int *pDimsOfInterest,
                           int numDimsOfInterest, int action = 2,
                           const int *p0bservedValues = NULL,
                           CMatrix< Type > *output = NULL,
                           EAccumType = PNL_ACCUM_TYPE_STORE ) const = 0;
    /*this method is used for Factor::Marginalize (1) and
    Factor::ShrinkObservedNodes(2). It reduses matrix dimensions
    dependently of optimize:
    optimize = 0 - Sum of other(exept DimOfInterest) dimensions
    optimize = 1 - choosing Maximum of other dimensions
    optimize = 2 - choosing given value of node(dimension), values of nodes
    (DimOfKeep)
    are contained in pObsValue*/
    virtual CMatrix<Type> *ExpandDims(const int *dimsToExtend,
                        const int *keepPosOfDims,
                        const int *sizesOfExpandDims,
                        int numDimsToExpand) const = 0;
    /*this method extend dimensions dimsToExpand(their sizes should be 1
    before expand) to size sizesOfExpandDims by adding zeros
    at the matrix points in this dimsToExpand except valuesOfDims -
    for these matrix points values will be set from original matrix*/
    virtual void ClearData() = 0;
    virtual void SetUnitData() = 0;
    virtual ~CMatrix();
    inline int SetClamp( int Clamp);
    //to prevent changing Matrix (it can be shared by several factors)
    //return value of m_bClamped after the operation
    //(in case Clamp<0 m_bClamped doesn't change)
    inline int GetClampValue() const;
    //to set new data to the matrix (we can do it if matrix isn't clamped)

    //methods for numerical matrices
    //multiply matrix by dimensions
    virtual void MultiplyInSelf( const CMatrix<Type>* matToMult,
        int numDimsToMult, const int* indicesToMultInSelf, int isUnifrom = 0,
        const Type uniVal = Type(0) );
    //summarize matrix by dimensions
    virtual void SumInSelf(const CMatrix<Type>* matToMult,
        int numDimsToMult, const int* indicesToMultInSelf, int isUnifrom = 0,
        const Type uniVal = Type(0));
    //divide matrices by dimensions
    virtual void DivideInSelf( const CMatrix<Type>* matToDiv,
        int numDimsToDiv, const int* indicesToDivInSelf );
    //compute MPE
    virtual void GetIndicesOfMaxValue( intVector* indices )const ;
    //create new matrix - as result of normalization
    virtual CMatrix<Type>* NormalizeAll() const;
    virtual void Normalize();
    //compute sum of matrix elements
    virtual Type SumAll(int byAbsValue) const;
    
    //iterator
    virtual CMatrixIterator<Type>* InitIterator() const = 0;
    virtual void Next(CMatrixIterator<Type>* current) const = 0;
//    virtual Type* Value(CMatrixIterator<Type>* current) = 0;
    virtual const Type* Value(CMatrixIterator<Type>* current) const = 0;
    virtual int IsValueHere( CMatrixIterator<Type>* current ) const = 0;
    virtual void Index( CMatrixIterator<Type>* current, intVector* index ) const = 0;

    // dump to output
    virtual void Dump() const;  // defined in pnlMatrix_impl.hpp

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CMatrix< int >::GetStaticTypeInfo();
    }
#endif
protected:
    CMatrix( int Clamped );
    CMatrix();

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 

private:
    int m_bClamped;
    //If we can't change this Matrix during learning m_bClamped = 1
};

#if !defined(SWIG) && (!defined(PNL_VC7) || defined(PNL_EXPORTS))

template<class Type>
CMatrix<Type>::~CMatrix()
{

}

template <class Type>
pnl::EMatrixClass pnl::CMatrix<Type>::GetMatrixClass() const
{
    return mcBase;
}

template <class Type>
inline int pnl::CMatrix<Type>::SetClamp( int Clamp )
{
    m_bClamped = Clamp ? 1 : 0;

    return m_bClamped;
}

template <class Type>
inline int pnl::CMatrix<Type>::GetClampValue() const
{
    return m_bClamped;
}

template <class Type>
pnl::CMatrix<Type>::CMatrix( int Clamped ) : m_bClamped( Clamped ? 1 : 0 )
{
}

//multiply matrices by dimensions
template <class Type>
void pnl::CMatrix<Type>::MultiplyInSelf( const pnl::CMatrix<Type>* matToMult,
        int numDimsToMult, const int* indicesToMultInSelf, int isUnifrom,
        const Type uniVal )
{
    PNL_THROW( CInvalidOperation, "this operation is for numeric matrices only" );
}
//divide matrices by dimensions
template <class Type>
void pnl::CMatrix<Type>::DivideInSelf( const pnl::CMatrix<Type>* matToDiv,
        int numDimsToDiv, const int* indicesToDivInSelf )
{
    PNL_THROW( CInvalidOperation, "this operation is for numeric matrices only" );
}
//compute MPE
template <class Type>
void pnl::CMatrix<Type>::GetIndicesOfMaxValue( intVector* indices )const 
{
    PNL_THROW( CInvalidOperation, "this operation is for numeric matrices only" );
}
//create new matrix - as result of normalization
template <class Type>
pnl::CMatrix<Type>* pnl::CMatrix<Type>::NormalizeAll() const
{
    PNL_THROW( CInvalidOperation, "this operation is for numeric matrices only" );
    return NULL;
}

template <class Type>
void pnl::CMatrix<Type>::Normalize()
{
    PNL_THROW( CInvalidOperation, "this operation is for numeric matrices only" );
}

//compute sum of matrix elements
template <class Type>
Type pnl::CMatrix<Type>::SumAll(int byAbsValue) const
{
    PNL_THROW( CInvalidOperation, "this operation is for numeric matrices only" );
    return Type(0);
}

template <class Type>
void pnl::CMatrix<Type>::SumInSelf(const CMatrix<Type>* matToMult,
  int numDimsToMult, const int* indicesToMultInSelf, int isUnifrom,
  const Type uniVal)
{
  PNL_THROW(CInvalidOperation, "this operation is for numeric matrices only");
}

template <class Type>
pnl::CMatrix<Type>::CMatrix( )
{
    m_bClamped = 0;
    
}

#endif

#ifdef PNL_RTTI
template <class Type>
const CPNLType CMatrix<Type>::m_TypeInfo = CPNLType("CMatrix", &(CReferenceCounter::m_TypeInfo));

#endif

PNL_END

#endif //__PNLMATRIX_HPP__

