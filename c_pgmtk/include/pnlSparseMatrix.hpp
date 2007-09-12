/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlSparseMatrix.hpp                                         //
//                                                                         //
//  Purpose:   CSparseMatrix template class definition & implementation    //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLSPARSEMATRIX_HPP__
#define __PNLSPARSEMATRIX_HPP__

#include "cxcore.h"

#include "pnlMatrix.hpp"
#include "pnlImpDefs.hpp"
#include "pnlDenseMatrix.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

template <class Type> class CSparseMatrix : public /*virtual*/ CMatrix<Type>
{
public:
    static CSparseMatrix<Type>* Create( int dim, const int *range,
                const Type defaultValue, int Clamp = 0 );
    static CSparseMatrix<Type>* Copy( CSparseMatrix<Type>* const inputMat );
    virtual CMatrix<Type>* CreateEmptyMatrix( int dim, const int *range,
        int Clamp, Type defaultVal = Type(0) )const;
    virtual CMatrix<Type>* Clone() const;
    //methods for conversion from other type
    //if requered type is the same as type of argument - return copy of object
    virtual CDenseMatrix<Type>* ConvertToDense() const;
    virtual CSparseMatrix<Type>* ConvertToSparse() const;
    virtual EMatrixClass GetMatrixClass() const;
    virtual void SetDataFromOtherMatrix( const CMatrix<Type>* matInput );
    ~CSparseMatrix();
    CSparseMatrix<Type> & operator = ( const CSparseMatrix<Type>& inputMat );
    virtual inline int GetNumberDims() const;
    virtual inline void GetRanges(int *numOfDimsOut, const int **rangesOut) const;
    //methods for viewing/change data
    inline Type GetElementByIndexes(const int *multidimindexes) const;
    //these functions return an element of matrix for
    //given multidimensional indexes

    inline bool IsExistingElement(const int *multidimindexes) const;

    inline void SetElementByIndexes(Type value, const int *multidimindexes);
    //these functions set matrix element equal value (for
    //given multidimensional indexes )
    virtual CMatrix<Type>* ReduceOp( const int *pDimsOfInterest,
                           int numDimsOfInterest, int action = 2,
                           const int *p0bservedValues = NULL,
                           CMatrix< Type > *output = NULL,
                           EAccumType = PNL_ACCUM_TYPE_STORE ) const;
    /*this method is used for Factor::Marginalize (1) and
    Factor::ShrinkObservedNodes(2). It reduses matrix dimensions
    dependently of optimize:
    optimize = 0 - Sum of other(exept DimOfKeep) dimensions
    optimize = 1 - choosing Maximum of other dimensions
    optimize = 2 - choosing given value of node(dimension), values of nodes
    (DimOfKeep)
    are contained in pObsValue*/
    CMatrix<Type> *ExpandDims(const int *dimsToExtend,
                        const int *keepPosOfDims,
                        const int *sizesOfExpandDims,
                        int numDimsToExpand) const;
    /*this method extend dimensions dimsToExpand(their sizes should be 1
    before expand) to size sizesOfExpandDims by adding zeros
    at the matrix points in this dimsToExpand except valuesOfDims -
    for these matrix points values will be set from original matrix*/
    inline void ClearData();
    inline const Type GetDefaultValue() const;
    virtual inline void SetUnitData(); 

    //iterator
    virtual CMatrixIterator<Type>* InitIterator() const;
    virtual void Next(CMatrixIterator<Type>* current) const;
//    virtual Type* Value(CMatrixIterator<Type>* current);
    virtual const Type* Value(CMatrixIterator<Type>* current) const;
    virtual int IsValueHere(CMatrixIterator<Type>* current) const;
    virtual void Index( CMatrixIterator<Type>* current, intVector* index ) const;
    inline void SetDefaultVal( Type defVal );

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CSparseMatrix< int >::GetStaticTypeInfo();
    }
#endif

#if GCC_VERSION >= 30400
    using CMatrix<Type>::GetClampValue;
    using CMatrix<Type>::SetClamp;
#endif

protected:
    int ConvertToIndex() const;
    CSparseMatrix(int dim, const int *range, const Type defaultVal, int Clamp);
    CSparseMatrix( const CSparseMatrix<Type> & inputMat );
    CSparseMatrix( CvSparseMat* p_sparse, Type defaultVal );
    inline CvSparseMat* GetCvSparseMat();
    inline const CvSparseMat* GetCvSparseMat() const;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
    CvSparseMat* m_pCvSparseMat;
    Type m_defaultVal;

};

#if !defined(SWIG)

template <class Type>
CSparseMatrix<Type> * CSparseMatrix<Type>::Create( int dim, const int *range,
                                      const Type defaultVal, int Clamp)
{
    PNL_CHECK_LEFT_BORDER( dim, 1 );
    PNL_CHECK_IS_NULL_POINTER( range );
    CSparseMatrix<Type> *pxMatrix = NULL;
    int i = 0;
    for( i = 0; i < dim; i++ )
    {
        if( range[i] < 0 )
        {
            PNL_THROW( COutOfRange, "range is negative" )
        }
    }
    pxMatrix = new CSparseMatrix<Type> ( dim, range, defaultVal, (Clamp>0));
    return pxMatrix;
}

template <class Type>
CMatrix<Type>* CSparseMatrix<Type>::CreateEmptyMatrix( int dim,
                                                const int *range, int Clamp,
                                          Type defaultVal ) const
{
    return CSparseMatrix<Type>::Create( dim, range, defaultVal, Clamp );
}


template <class Type>
CSparseMatrix<Type>* CSparseMatrix<Type>::Copy( CSparseMatrix<Type>* const 
                                                                pInputMat)
{
    PNL_CHECK_IS_NULL_POINTER( pInputMat );
    CSparseMatrix<Type> *retMat = new CSparseMatrix<Type>( *pInputMat );
    PNL_CHECK_IF_MEMORY_ALLOCATED( retMat );
    return retMat;
}

template <class Type>
CMatrix<Type>* CSparseMatrix<Type>::Clone() const
{
    CSparseMatrix<Type>* res = new CSparseMatrix<Type>( *this );
    PNL_CHECK_IF_MEMORY_ALLOCATED(res);
    return res;
}

//methods for conversion from other type
//if requered type is the same as type of argument - return copy of object
template <class Type>
CDenseMatrix<Type>* CSparseMatrix<Type>::ConvertToDense() const
{
    int i;
    int dim;
    const int* ranges;
    GetRanges( &dim, &ranges );
    int dataSize = 1;
    for( i = 0; i < dim; i++ )
    {
        dataSize *= ranges[i];
    }
    pnlVector<Type> dataVec;
    dataVec.assign( dataSize, m_defaultVal );
    //iterate throw matrix and set non-default elements
    CvSparseMatIterator iterator;
    CvSparseNode* node;
    int* idx;
    int offset = 0;
    for( node = cvInitSparseMatIterator( m_pCvSparseMat, &iterator );
                 node != 0; node = cvGetNextSparseNode( &iterator ))
    {
        //we put data from small matrix to needed positions in bigData
        idx = CV_NODE_IDX( m_pCvSparseMat, node );
        void* val = CV_NODE_VAL(  m_pCvSparseMat, node );
        //convert from indices to line
        offset = 0;
        for( i = 0; i < dim; i++)
	    {
    		offset = offset * ranges[i] + idx[i];
    	}
        dataVec[offset] = *((Type*)val);
    }

    CDenseMatrix<Type>* resMat = CDenseMatrix<Type>::Create( dim, ranges,
        &dataVec.front(), GetClampValue() );
    return resMat;
}

template <class Type>
CSparseMatrix<Type>* CSparseMatrix<Type>::ConvertToSparse() const
{
    return static_cast<CSparseMatrix<Type>*>(Clone());
}

template <class Type>
EMatrixClass CSparseMatrix<Type>::GetMatrixClass() const
{
    return mcSparse;
}

template <class Type>
void CSparseMatrix<Type>::SetDataFromOtherMatrix( const CMatrix<Type>* matInput )
{
    PNL_CHECK_IS_NULL_POINTER( matInput );

    CSparseMatrix<Type>* self = this;
    *self = *(static_cast<const CSparseMatrix<Type>*>(matInput));
}


template <class Type>
CSparseMatrix<Type>::~CSparseMatrix<Type>()
{
    if( m_pCvSparseMat )
    {
        cvReleaseSparseMat( &m_pCvSparseMat );
    }
}


template <class Type>
CSparseMatrix<Type>& CSparseMatrix<Type>::operator = ( const 
                                              CSparseMatrix<Type>& inputMat )
{
    if( this != &inputMat )
    {
        if( GetClampValue() )
        {
            PNL_THROW( CInvalidOperation, "we can't change clamped matrix" )
        }
        if( m_defaultVal != inputMat.GetDefaultValue() )
        {
            PNL_THROW( CInvalidOperation, 
                "we can't set data to matrix with other default value" )
        }
        int numDimsInp;
        const int* rangesInp;
        inputMat.GetRanges( &numDimsInp, &rangesInp );
        int isTheSame = 1;
        int numDimsHere;
        const int* rangesHere;
        GetRanges( &numDimsHere, &rangesHere );
        if( numDimsInp != numDimsHere )
        {
            isTheSame = 0;
        }
        int i;
        for( i = 0; i < numDimsHere; i++ )
        {
            if( rangesInp[i] != rangesHere[i] )
            {
                isTheSame = 0;
                break;
            }
        }
        if( isTheSame )
        {
            if( m_pCvSparseMat )
            {
                cvReleaseSparseMat( &m_pCvSparseMat );
            }
            m_pCvSparseMat = cvCloneSparseMat( inputMat.GetCvSparseMat() );
        }
        else
        {
            PNL_THROW( CInvalidOperation,
                "we have only quick operator= for objects of the same size" );
        }
    }
    return *this;
}

template<class Type>
inline int CSparseMatrix<Type>::GetNumberDims() const
{
    return m_pCvSparseMat->dims;
}

template <class Type>
inline void CSparseMatrix<Type>::GetRanges(int *numOfDims,
                                        const int **ranges ) const
{
    *numOfDims = m_pCvSparseMat->dims;
    *ranges = m_pCvSparseMat->size;
}

template <class Type>
inline Type CSparseMatrix<Type>::GetElementByIndexes(const int *multidimindexes) const
{
    int* multInd = const_cast<int*>(multidimindexes);
    uchar* retVal = cvPtrND( m_pCvSparseMat, multInd, NULL, 0, 0 );
    Type retValT;
    if( retVal )
    {
        retValT = *((Type*)retVal);
    }
    else
    {
        retValT = m_defaultVal;
    }
    return retValT;
}

template <class Type>
inline bool CSparseMatrix<Type>::IsExistingElement(const int *multidimindexes) const
{
    int* multInd = const_cast<int*>(multidimindexes);
    uchar* retVal = cvPtrND( m_pCvSparseMat, multInd, NULL, 0, 0 );
    if( retVal )
    {
        return true;
    }
    else
    {
        return false;
    }
}

template <class Type>
inline void CSparseMatrix<Type>::SetElementByIndexes( Type value,
                                                const int *multidimindexes)
{
    if( !GetClampValue() )
    {
        int* multInd = const_cast<int*>(multidimindexes);
        uchar* retVal = cvPtrND( m_pCvSparseMat, multInd, NULL, 1, 0);
        //uchar* retVal = cvPtrND( m_pCvSparseMat, multInd );
        if( retVal )
        {
            //Type retValT = *((Type*)retVal) ;
            //retValT = value;
            memcpy( retVal, &value, sizeof(Type) );
        }
        else
        {
            PNL_THROW( CInvalidOperation, "can't set value" )
        }
    }
    else
    {
        PNL_THROW( CInvalidOperation, "can't change clamped matrix" )
    }
}

template <class Type>
CMatrix<Type>* CSparseMatrix<Type>::ExpandDims( const int *dimsToExpand,
                    const int *keepPosOfDims, const int *sizesOfExpandDims,
                    int numDimsToExpand) const
{
    int i;
    int dim = m_pCvSparseMat->dims;
    PNL_CHECK_RIGHT_BORDER( numDimsToExpand, dim );
    if( numDimsToExpand > dim)
    {
        PNL_THROW( CInconsistentSize, "numDimsToExpand > dim" )
    }
    //check dimsToExpand be a subset of dims
    int location;
    intVector existingDims;
    existingDims.resize( dim );
    for( i = 0; i < dim; i++ )
    {
        existingDims[i] = i;
    }
    for( i = 0; i < numDimsToExpand; i++ )
    {
        location = std::find( existingDims.begin(),
                existingDims.end(), dimsToExpand[i] ) - existingDims.begin();
        if( location < dim )
        {
            existingDims.erase(existingDims.begin() + location);
        }
    }
    if( int(existingDims.size()) != dim - numDimsToExpand )
    {
        PNL_THROW( CInconsistentSize, "dims to expand isn't subset of dims" )
    }
    //no we begin to compute
    //extract ranges of old matrix
    intVector rangesOld;
    rangesOld.assign( m_pCvSparseMat->size, m_pCvSparseMat->size+dim );
    //create ranges for new matrix
    intVector rangesNew;
    rangesNew.assign( m_pCvSparseMat->size, m_pCvSparseMat->size+dim );
    for( i = 0; i < numDimsToExpand; i++ )
    {
        rangesNew[dimsToExpand[i]] = sizesOfExpandDims[i];
    }
    //create new empty cv matrix 
    CSparseMatrix<Type>* resMat = static_cast<CSparseMatrix<Type>*>(
        CreateEmptyMatrix( dim, &rangesNew.front(),
        GetClampValue(), m_defaultVal ));
    CvSparseMat* retMat = resMat->GetCvSparseMat();
    //need to put information in this matrix
    //compute corrsponding map between old and new matrices
    CvSparseMatIterator iterator;
    CvSparseNode* node;
    int* idx;
    for( node = cvInitSparseMatIterator( m_pCvSparseMat, &iterator );
                 node != 0; node = cvGetNextSparseNode( &iterator ))
    {
        //we put data from small matrix to needed positions in bigData
        idx = CV_NODE_IDX( m_pCvSparseMat, node );
        intVector index = intVector( idx, idx + dim );
        for( i = 0; i < numDimsToExpand; i++ )
        {
            index[dimsToExpand[i]] = keepPosOfDims[i];
        }
        void* val = CV_NODE_VAL(  m_pCvSparseMat, node );
        Type* valT = (Type*)val;
        uchar* valInNew = cvPtrND( retMat, &index.front(), NULL, 1, 0 );
        //uchar* valInNew = cvPtrND( retMat, idx );
        *((Type*)valInNew) = *valT;
    }
    return resMat;
}

template <class Type>
inline void CSparseMatrix<Type> :: ClearData()
{
    //cvReleaseMemStorage( &m_pCvSparseMat->heap->storage );
    //cvFree( (void**)(m_pCvSparseMat->hashtable) );
    cvSetZero(m_pCvSparseMat);
}

template<class Type>
inline int CSparseMatrix<Type>::ConvertToIndex() const
{
    if (sizeof(Type) == sizeof(size_t))
    {
	return CV_USRTYPE1;
    }
    if (sizeof(Type) == 4)
    {
	return CV_32S;
    }
    if (sizeof(Type) == 2)
    {
	return CV_16U;
    }
    if (sizeof(Type) == 1)
    {
	return CV_8U;
    } 

    pnlString msg;
    msg << "Can't create sparse matrix with entries of size " << sizeof(Type);
    PNL_THROW(CInvalidOperation, msg);
}

template<>
inline int CSparseMatrix<float>::ConvertToIndex() const
{
    return int(CV_32F);
}

template<>
inline int CSparseMatrix<int>::ConvertToIndex() const
{
    return int(CV_32S);
}

template <class Type>
inline const Type CSparseMatrix<Type> ::GetDefaultValue() const
{
	return m_defaultVal;
}
template <class Type>
inline void pnl::CSparseMatrix<Type>::SetUnitData()
{
    PNL_THROW( CInvalidOperation,
        "can't set unit data for matrix of pointers, for example" );
}



template <class Type>
inline void CSparseMatrix<Type>::SetDefaultVal( Type defVal )
{
    m_defaultVal = defVal;
}

template <class Type>
CvSparseMat* CSparseMatrix<Type> ::GetCvSparseMat() 
{
	return m_pCvSparseMat;
}

template <class Type>
const CvSparseMat* CSparseMatrix<Type> ::GetCvSparseMat() const
{
	return m_pCvSparseMat;
}


template <class Type>
CSparseMatrix<Type>::CSparseMatrix( int dim,
                                 const int *range, const Type defaultVal,
                                 int Clamp):CMatrix<Type>(Clamp),
                                 m_defaultVal(defaultVal)
{
    m_pCvSparseMat = cvCreateSparseMat( dim, range, ConvertToIndex() );
}

template <class Type>
CSparseMatrix<Type>::CSparseMatrix( const CSparseMatrix<Type> & inputMat )
:CMatrix<Type>(0), m_defaultVal(0)
{
    m_defaultVal = inputMat.GetDefaultValue();
    m_pCvSparseMat = cvCloneSparseMat( inputMat.GetCvSparseMat() );
}


template <class Type>
CSparseMatrix<Type>::CSparseMatrix( CvSparseMat* p_sparse, Type defaultVal )
:CMatrix<Type>(0), m_defaultVal(defaultVal)
{
    m_pCvSparseMat = p_sparse;
}

#define PNL_SPARSE_MATRIX_SOFT_MAX_DIM 128

template< typename ELTYPE >
CMatrix< ELTYPE > *CSparseMatrix< ELTYPE >::ReduceOp( const int *pDimsOfInterest,
                                           int numDimsOfInterest,
                                           int action,
                                           const int *pObservedValues,
                                           CMatrix< ELTYPE > *output,
                                           EAccumType accum_type ) const
{ // output and accum_type not supported yet..
    CSparseMatrix< ELTYPE > *retval;
    int i;
    int num_dims;
    int *idx;
    const int *ranges;
    int num_dims_to_keep;
    CvSparseNode *nd, *nnd;
    CvSparseMatIterator it;

    PNL_DEFINE_AUTOBUF( bool, mask, PNL_SPARSE_MATRIX_SOFT_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, dims_to_keep, PNL_SPARSE_MATRIX_SOFT_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, new_ranges, PNL_SPARSE_MATRIX_SOFT_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, stats, PNL_SPARSE_MATRIX_SOFT_MAX_DIM );

    PNL_MAKE_LOCAL( CvSparseMat *, mat, this, GetCvSparseMat() );
    PNL_MAKE_LOCAL( ELTYPE, inival, this, GetDefaultValue() );

    if ( (unsigned)action > 2 )
    {
        PNL_THROW( CInconsistentType, "ReduceOp: action not supported" );
    }
    GetRanges( &num_dims, &ranges );
    if ( (unsigned)numDimsOfInterest > num_dims )
    {
        PNL_THROW( CInconsistentSize, "ReduceOp: broken numDimsOfInterest" );
    }
    PNL_DEMAND_AUTOBUF( mask, num_dims );
    for ( i = num_dims; i--; )
    {
        mask[i] = false;
    }
    for ( i = numDimsOfInterest; i--; )
    {
        if ( (unsigned)pDimsOfInterest[i] >= num_dims )
        {
            PNL_RELEASE_AUTOBUF( mask );
            PNL_THROW( CInconsistentSize, "ReduceOp: broken pDimsOfInterest" );
        }
        if ( mask[pDimsOfInterest[i]] )
        {
            PNL_RELEASE_AUTOBUF( mask );
            PNL_THROW( CInconsistentSize, "ReduceOp: pDimsOfInterest clash" );
        }
        mask[pDimsOfInterest[i]] = true;
    }
    if ( action == 2 )
    {
        for ( i = numDimsOfInterest; i--; )
        {
            if ( (unsigned)pObservedValues[i] >= ranges[pDimsOfInterest[i]] )
            {
                PNL_RELEASE_AUTOBUF( mask );
                PNL_THROW( CInconsistentSize, "ReduceOp: pObservedValues cranky" );
            }
        }
    }
    if ( action < 2 )
    {
        dims_to_keep = const_cast< int * >( pDimsOfInterest );
        num_dims_to_keep = numDimsOfInterest;
    }
    else
    {
        PNL_DEMAND_AUTOBUF( dims_to_keep, num_dims );
        for ( i = 0, num_dims_to_keep = 0; i < num_dims; ++i )
        {
            if ( !mask[i] )
            {
                dims_to_keep[num_dims_to_keep++] = i;
            }
        }
    }
    PNL_DEMAND_AUTOBUF( new_ranges, num_dims );
    if ( action < 2 )
    {
        for ( i = num_dims_to_keep; i--; )
        {
            new_ranges[i] = ranges[dims_to_keep[i]];
        }
    }
    else
    {
        for ( i = num_dims; i--; )
        {
            new_ranges[i] = ranges[i];
        }
        for ( i = numDimsOfInterest; i--; )
        {
            new_ranges[pDimsOfInterest[i]] = 1;
        }
    }
    retval = CSparseMatrix< ELTYPE >::Create( action == 2 ? num_dims : num_dims_to_keep,
                                              new_ranges, inival );
    PNL_MAKE_LOCAL( CvSparseMat *, tgt, retval, GetCvSparseMat() );
    PNL_DEMAND_AUTOBUF( stats, num_dims );
    nd = cvInitSparseMatIterator( mat, &it );
    if ( action < 2 )
    {
        PNL_THROW( CInvalidOperation, 
            "can call reduce with sum or max only for numeric matrices"  );
    }
    else
    {
        for ( i = num_dims; i--; )
        {
            stats[i] = 0;
        }
        while ( nd )
        {
            idx = CV_NODE_IDX( mat, nd );
            for ( i = numDimsOfInterest; i--; )
            {
                if ( idx[pDimsOfInterest[i]] != pObservedValues[i] )
                {
                    goto cont;
                }
            }
            for ( i = num_dims_to_keep; i--; )
            {
                stats[dims_to_keep[i]] = idx[dims_to_keep[i]];
            }
            //nnd = (CvSparseNode*)cvPtrND( tgt, stats );
            nnd = (CvSparseNode *)cvPtrND( tgt, stats, 0, 1, 0 );
            *(ELTYPE *)CV_NODE_VAL( tgt, nnd ) = *(ELTYPE *)CV_NODE_VAL( mat, nd );
cont:
            nd = cvGetNextSparseNode( &it );
        }
    }

    PNL_RELEASE_AUTOBUF( stats );
    PNL_RELEASE_AUTOBUF( new_ranges );
    PNL_RELEASE_AUTOBUF( dims_to_keep );
    PNL_RELEASE_AUTOBUF( mask );

    return retval;
}

template <class Type>
CMatrixIterator<Type>* CSparseMatrix<Type>::InitIterator() const
{   
    CSparseMatrixIterator<Type>* iter = CSparseMatrixIterator<Type>::Create(
        m_pCvSparseMat);
    return iter;
}

template <class Type>
void CSparseMatrix<Type>::Next(CMatrixIterator<Type>* current) const
{
    current->Iteration();
}
/*template <class Type>
Type* CSparseMatrix<Type>::Value(CMatrixIterator<Type>* current)
{
    return (current->GetCurrent());
}*/
template <class Type>
const Type* CSparseMatrix<Type>:: Value(CMatrixIterator<Type>* current) const
{
    return (current->GetCurrent());
}
template <class Type>
int CSparseMatrix<Type>::IsValueHere( CMatrixIterator<Type>* current ) const
{
    if( static_cast<CSparseMatrixIterator<Type>*>(current)->IsNodeHere() )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

template <class Type>
void CSparseMatrix<Type>::Index( CMatrixIterator<Type>*current, intVector* index) const
{
    CSparseMatrixIterator<Type>* curSparse = static_cast<
        CSparseMatrixIterator<Type>*>(current);
    int* idx = curSparse->Index();
    int numDims = GetNumberDims();
    index->assign( idx, idx+numDims );
}

#endif

#ifdef PNL_RTTI
template< typename T >
const CPNLType CSparseMatrix< T >::m_TypeInfo = CPNLType("CSparseMatrix", &(CMatrix< T >::m_TypeInfo));

#endif

PNL_END

#endif //__PNLSPARSEMATRIX_HPP__
