/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnliNumericSparseMatrix.hpp                                 //
//                                                                         //
//  Purpose:   CNumericSparseMatrix template class definition &            //  
//             implementation                                              //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLINUMERICSPARSEMATRIX_HPP__
#define __PNLINUMERICSPARSEMATRIX_HPP__

#include "pnlSparseMatrix.hpp"
//#include "pnlNumericMatrix.hpp"
#include "pnlNumericDenseMatrix.hpp"
//#include "pnlNumericSparseMatrix.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

template< typename T > class CNumericDenseMatrix;
template< typename T > class CNumericSparseMatrix;
template< typename T > class iCNumericSparseMatrix;

template< typename T >
CNumericSparseMatrix< T > *pnlCombineNumericMatrices( const iCNumericSparseMatrix< T > *mat1,
                                                      const iCNumericSparseMatrix< T > *mat2,
                                                      int combineFlag );

template< typename Type > class PNL_API iCNumericSparseMatrix
    : public CSparseMatrix<Type>//, public CMatrix<Type>
{
public:
    static CNumericSparseMatrix< Type >* Create( int dim, const int *range, int Clamp = 0 );
    static iCNumericSparseMatrix<Type> *Copy( const iCNumericSparseMatrix<Type> *inputMat );
    virtual CMatrix<Type>* CreateEmptyMatrix( int dim, const int *range, int Clamp,
                                              Type defaultVal = Type(0) ) const;
    virtual CMatrix<Type>* Clone() const ;
    //methods for conversion from other type
    //if requered type is the same as type of argument - return copy of object
    virtual CDenseMatrix<Type>* ConvertToDense() const;
    virtual CSparseMatrix<Type>* ConvertToSparse() const;

    EMatrixClass GetMatrixClass() const;

#ifndef PNL_VC6
    friend PNL_API
    CNumericSparseMatrix< Type > *pnlCombineNumericMatrices<>( const iCNumericSparseMatrix< Type > *mat1,
                                                               const iCNumericSparseMatrix< Type > *mat2,
                                                               int combineFlag );
#else
    friend PNL_API
    CNumericSparseMatrix< Type > *pnlCombineNumericMatrices( const iCNumericSparseMatrix< Type > *mat1,
                                                             const iCNumericSparseMatrix< Type > *mat2,
                                                             int combineFlag );
#endif

    void CombineInSelf( const iCNumericSparseMatrix* matAdd,
        int combineFlag = 1 );
    //combine flag = 1 if we want to sum matrices,
    //0 - substract second matrix from first, 
    //-1  -choose max values from matrices
    virtual CMatrix<Type>* ReduceOp( const int *pDimsOfInterest,
                           int numDimsOfInterest, int action = 2,
                           const int *p0bservedValues = NULL,
                           CMatrix< Type > *output = NULL,
                           EAccumType = PNL_ACCUM_TYPE_STORE ) const;
    /*this method is used for Factor::Marginalize (1) and
    Factor::ShrinkObservedNodes(2). It reduces matrix dimensions
    dependently of optimize:
    optimize = 0 - Sum of other(exept DimOfKeep) dimensions
    optimize = 1 - choosing Maximum of other dimensions
    optimize = 2 - choosing given value of node(dimension), values of nodes
    (DimOfKeep)
    are contained in pObsValue*/
    Type SumAll( int byAbsValue = 0 ) const;
    virtual CMatrix< Type > *NormalizeAll() const;
    virtual void Normalize();
    virtual void MultiplyInSelf( const CMatrix< Type > *matToMult, int numDimsToMult,
                                 const int *indicesToMultInSelf, int isUnifrom = 0,
                                 const Type uniVal = Type(0) );
    virtual void DivideInSelf( const CMatrix<Type>* matToDiv, int numDimsToDiv,
                               const int *indicesToDivInSelf );
    virtual void GetIndicesOfMaxValue( intVector* indicesOut ) const;
    //virtual methods from base class

    inline int GetNumberDims() const;
    inline void GetRanges(int *numOfDimsOut, const int **rangesOut) const;
    //methods for viewing/change data
    inline Type GetElementByIndexes(const int *multidimindexes) const;
    //these functions return an element of matrix for
    //given multidimensional indexes
    inline void SetElementByIndexes( Type value, const int *multidimindexes );
    //these functions set matrix element equal value (for
    //given multidimensional indexes )
    CMatrix<Type> *ExpandDims( const int *dimsToExtend, const int *keepPosOfDims,
                               const int *sizesOfExpandDims, int numDimsToExpand ) const;
    inline void ClearData();
    inline void SetUnitData();
    //inline void SetUnitValuesOneByOne();

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return iCNumericSparseMatrix< int >::GetStaticTypeInfo();
    }
#endif

#if GCC_VERSION >= 30400
    using CSparseMatrix<Type>::GetClampValue;
    using CSparseMatrix<Type>::GetDefaultValue;
#endif

protected:
    iCNumericSparseMatrix( int dim, const int *range, int Clamp );
    iCNumericSparseMatrix( const iCNumericSparseMatrix<Type> &inputMat );
    iCNumericSparseMatrix( CvSparseMat *p_sparse );

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 

#if GCC_VERSION >= 30400
    using CSparseMatrix<Type>::GetCvSparseMat;
#endif

private:
};

#if !defined(SWIG) && (!defined(PNL_VC7) || defined(PNL_EXPORTS))

template <class Type> CNumericSparseMatrix<Type>*
pnlCombineNumericMatrices(const iCNumericSparseMatrix<Type> *mat1, 
                               const iCNumericSparseMatrix<Type> *mat2,
                          int combineFlag = 1 )
{
    PNL_CHECK_IS_NULL_POINTER( mat1 );
    PNL_CHECK_IS_NULL_POINTER( mat2 );
    PNL_CHECK_RANGES( combineFlag, -1, 1 );

    int i;
    int dim1; const int * ranges1;
    mat1->GetRanges(&dim1, &ranges1);
    int dim2; const int * ranges2;
    mat2->GetRanges(&dim2, &ranges2);
    if( dim1 != dim2 )
    {
        PNL_THROW( CInconsistentSize, "matrix sizes");
    }
    for( i = 0; i < dim1; i++ )
    {
        if( ranges1[i] != ranges2[i] )
        {
            PNL_THROW( CInconsistentSize, "matrix sizes");
        }
    }
    iCNumericSparseMatrix<Type>* resMat = iCNumericSparseMatrix<Type>::Copy( mat1 );
    resMat->CombineInSelf( mat2, combineFlag );
    return resMat;

    /*CvSparseMat* cvSpMat = cvCreateSparseMat( dim1, ranges1, ConvertToIndex() );
    CvSparseMat* cvSpMat1 = mat1->GetCvSparseMat();
    CvSparseMat* cvSpMat2 = mat2->GetCvSparseMat();
    //iterate throw values from first matrix
    CvSparseMatIterator iterator1;
    CvSparseNode* node1;
    int* idx1;
    for( node1 = cvInitSparseMatIterator( cvSpMat1, &iterator );
                 node1 != 0; node1 = cvGetNextSparseNode( &iterator ))
    {
        idx1 = CV_NODE_IDX( cvSpMat1, node1 );
        void* val1 = CV_NODE_VAL(  m_pCvSparseMat, node );
        Type valT1 = *(Type*)val1;
        //set all values from the largest matrix
        //uchar* valInNew = cvPtrND( cvSpMat, idx1, NULL, 1 );//create node
        uchar* valInNew = cvPtrND( cvSpMat, idx1 );
        //set value for node
        *((Type*)valInNew) = *valT1;
    }
    CvSparseMatIterator iterator2;
    CvSparseNode* node2;
    int* idx2;
    for( node2 = cvInitSparseMatIterator( cvSpMat2, &iterator );
                 node2 != 0; node2 = cvGetNextSparseNode( &iterator ))
    {
        idx2 = CV_NODE_IDX( cvSpMat2, node2 );
        void* val2 = CV_NODE_VAL(  m_pCvSparseMat, node );
        Type valT2 = *(Type*)val2;
        //set all values from the largest matrix
        //uchar* valInNew = cvPtrND( cvSpMat, idx2, NULL, 2 );//create node
        uchar* valInNew = cvPtrND( cvSpMat, idx2 );
        //set value for node
        Type oldVal = *(Type*)(valInNew);
        Type resVal = Type(0);
        if( combineFlag == 1 )
        {
            resVal = valT2 + oldVal;
        }
        if( combineFlag == 0 )
        {
            resVal = oldVal - valT2;
        }
        if( combineFlag == -1 )
        {
            if( oldVal > valT2 )
            {
                resVal = oldVal;
            }
            else
            {
                resVal = valT2;
            }
        }
        *((Type*)valInNew) = resVal;
    }
    
	iCNumericSparseMatrix *mat = new iCNumericSparseMatrix( cvSpMat, 0);
	return mat;*/
}

template <class Type>
CNumericSparseMatrix<Type>* iCNumericSparseMatrix<Type>::Create(int dim,
                                    const int* range, int Clamp )
{
    PNL_THROW( CInvalidOperation,
        "Numeric matrices assume float or double types only!" )
    return NULL;
}


//combine flag = 1 if we want to sum matrices,
//0 - substract second matrix from first, 
//-1  -choose max values from matrices
template <class Type>
void iCNumericSparseMatrix<Type>::CombineInSelf( 
                              const iCNumericSparseMatrix<Type>* matAdd,
                                  int combineFlag)
{
    PNL_CHECK_IS_NULL_POINTER( matAdd );
    PNL_CHECK_RANGES( combineFlag, -1, 1 );

    int i;
	int dim1; const int * ranges1;
	GetRanges(&dim1, &ranges1);
    int dim2; const int * ranges2;
	matAdd->GetRanges(&dim2, &ranges2);
	if( dim1 != dim2 )
	{
		PNL_THROW( CInconsistentSize, "matrix sizes");
	}
	for( i = 0; i < dim1; i++ )
	{
		if( ranges1[i] != ranges2[i] )
		{
			PNL_THROW( CInconsistentSize, "matrix sizes");
		}
	}
    CvSparseMat* cvSpMat1 = GetCvSparseMat();
    CvSparseMat* cvSpMat2 = const_cast<CvSparseMat*>(matAdd->GetCvSparseMat());
    //iterate throw additional matrix
    CvSparseMatIterator iterator2;
    CvSparseNode* node2;
    if( combineFlag >= 0 )
    {
        int* idx2;
        for( node2 = cvInitSparseMatIterator( cvSpMat2, &iterator2 );
                 node2 != 0; node2 = cvGetNextSparseNode( &iterator2 ))
        {
            idx2 = CV_NODE_IDX( cvSpMat2, node2 );
            void* val2 = CV_NODE_VAL(  cvSpMat2, node2 );
            Type valT2 = *(Type*)val2;
            //set all values from the largest matrix
            uchar* valInNew = cvPtrND( cvSpMat1, idx2, NULL, 2, 0 );//create node
            //uchar* valInNew = cvPtrND( cvSpMat, idx2 );
            //set value for node
            Type res = Type(0);
            Type oldVal = *(Type*)(valInNew);
            if( combineFlag == 1 )
            {
                res = valT2 + oldVal;
            }
            if( combineFlag == 0 )
            {
                res = oldVal - valT2;
            }
            *((Type*)valInNew) = res;
        }
    }
    if( combineFlag == -1 )
    {
        PNL_THROW( CNotImplemented, 
            "combine of sparse matrices - max value for both them" )
    }
}

template <class Type>
CMatrix<Type>* iCNumericSparseMatrix<Type>::Clone() const
{
    iCNumericSparseMatrix<Type>* res = new iCNumericSparseMatrix<Type>( *this );
    PNL_CHECK_IF_MEMORY_ALLOCATED(res);
    return static_cast<CSparseMatrix<Type>*>(res);
}

template <class Type>
CDenseMatrix<Type>* iCNumericSparseMatrix<Type>::ConvertToDense() const
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
    dataVec.assign( dataSize, GetDefaultValue() );
    //iterate throw matrix and set non-default elements
    CvSparseMatIterator iterator;
    CvSparseNode* node;
    int* idx;
    int offset = 0;
    const CvSparseMat* mat = GetCvSparseMat();
    for( node = cvInitSparseMatIterator( mat, &iterator );
                 node != 0; node = cvGetNextSparseNode( &iterator ))
    {
        //we put data from small matrix to needed positions in bigData
        idx = CV_NODE_IDX( mat, node );
        void* val = CV_NODE_VAL(  mat, node );
        //convert from indices to line
        offset = 0;
        for( i = 0; i < dim; i++)
	    {
    		offset = offset * ranges[i] + idx[i];
    	}
        dataVec[offset] = *((Type*)val);
    }

    CNumericDenseMatrix< Type > *resMat
        = CNumericDenseMatrix< Type >::Create( dim, ranges, &dataVec.front(), GetClampValue() );
    return resMat;
}

template <class Type>
CSparseMatrix<Type>* iCNumericSparseMatrix<Type>::ConvertToSparse() const
{
    return static_cast<CSparseMatrix<Type>*>(Clone());
}

template  <class Type>
EMatrixClass iCNumericSparseMatrix<Type>::GetMatrixClass() const
{
    return mcNumericSparse;
}

template <class Type>
CMatrix<Type>* iCNumericSparseMatrix<Type>::CreateEmptyMatrix( int dim,
          const int *range, int Clamp, Type defaultVal) const
{
    PNL_CHECK_LEFT_BORDER( dim, 1 );
    PNL_CHECK_IS_NULL_POINTER( range );
    PNL_CHECK_LEFT_BORDER( Clamp, 0 );

    int size = 1;
    for( int i = 0; i < dim; i++ )
    {
        PNL_CHECK_LEFT_BORDER( range[i], 1 );
        size *= range[i];
    }
    CNumericSparseMatrix<Type>* mat = 
        CNumericSparseMatrix<Type>::Create( dim, range, Clamp );
    mat->SetDefaultVal(defaultVal);
    return mat;
}



template <class Type>
iCNumericSparseMatrix<Type>* iCNumericSparseMatrix<Type>::Copy( 
                            const iCNumericSparseMatrix<Type>* inputMat)
{
    PNL_CHECK_IS_NULL_POINTER( inputMat );
    iCNumericSparseMatrix<Type> *retMat = new iCNumericSparseMatrix<Type>( 
        *inputMat );
    PNL_CHECK_IF_MEMORY_ALLOCATED( retMat );
    return retMat;
}

template <class Type>
Type iCNumericSparseMatrix<Type>::SumAll( int byAbsValue /* = 0 */) const
{
    CvSparseMatIterator iterator;
    CvSparseNode* node;
    const iCNumericSparseMatrix<Type>* self = this;
    const CvSparseMat* mat = const_cast<CvSparseMat*>(self->GetCvSparseMat());
    Type sum = 0;
    for( node = cvInitSparseMatIterator( mat, &iterator );
                 node != 0; node = cvGetNextSparseNode( &iterator ))
    {
        //we put data from small matrix to needed positions in bigData
        void* val = CV_NODE_VAL(  mat, node );
        Type valT = *(Type*)val;
        if( byAbsValue )
        {
            sum += (Type)fabs( valT );
        }
        sum += valT;
    }
    return sum;
}



template <class Type>
void iCNumericSparseMatrix<Type>::Normalize()
{
    Type sum = SumAll();
    if(( sum > (2*FLT_MIN) )&&( fabs(sum - 1)> FLT_EPSILON ))
    {
        Type reciprocalSum = 1/sum;
        CvSparseMatIterator iterator;
        CvSparseNode* node;
        CvSparseMat* pCvSparseMat = this->GetCvSparseMat();
        for( node = cvInitSparseMatIterator( pCvSparseMat, &iterator );
                 node != 0; node = cvGetNextSparseNode( &iterator ))
        {
            //we put data from small matrix to needed positions in bigData
            void* val = CV_NODE_VAL( pCvSparseMat, node );
            Type valT = *(Type*)val;
            valT = valT*reciprocalSum;
            *((Type*)val) = valT;
        }
    }
}

template <class Type>
CMatrix<Type>*  iCNumericSparseMatrix<Type>::NormalizeAll() const
{
    Type sum = (static_cast<const CNumericSparseMatrix<Type>*>(this))->SumAll();
    const CvSparseMat* mat = GetCvSparseMat();
    CvSparseMat* resMat = cvCloneSparseMat(mat);
    if( sum != 0 )
    {
        CvSparseMatIterator iterator;
        CvSparseNode* node;
        for( node = cvInitSparseMatIterator( resMat, &iterator );
        node != 0; node = cvGetNextSparseNode( &iterator ))
        {
            //we put data from small matrix to needed positions in bigData
            void* val = CV_NODE_VAL(  resMat, node );
            Type valT = *(Type*)val;
            valT = valT/sum;
            *((Type*)val) = valT;
        }
    }
    iCNumericSparseMatrix<Type>* res = new iCNumericSparseMatrix<Type>(resMat);
    return static_cast<CNumericSparseMatrix<Type>*>(res);
}

template<class Type>
void iCNumericSparseMatrix<Type>::MultiplyInSelf( 
                            const CMatrix<Type>* matToMult,
                            int numDimsToMult, const int* indicesToMultInSelf,
                            int isUnifrom ,  const Type uniVal )
{
    EMatrixClass otherClass = matToMult->GetMatrixClass();
    if( otherClass != mcNumericSparse )
    {
        PNL_THROW( CInvalidOperation, "can't multiply dense in sparse" );
    }
    int i;
    Type zeroEpsilon = Type(0.0000000001);
    CvSparseMat* mat = GetCvSparseMat();
    const iCNumericSparseMatrix<Type>* matToMultSp = static_cast<const
        iCNumericSparseMatrix<Type>*>(matToMult);
    const CvSparseMat* cvMatMult = matToMultSp->GetCvSparseMat();
    int numSmDims = cvMatMult->dims;
    if( numSmDims != numDimsToMult )
    {
        PNL_THROW( CInconsistentSize, "numDimsToMult must be the same as " )
    }
    int* idx;
    Type newVal;
    if( isUnifrom )
    {
        if( mat->heap->active_count > 1 )
        {
            PNL_THROW( CInvalidOperation,
                "uniform matrices must haven't any data" );
        }
        if( uniVal )
        {
            if( numDimsToMult == mat->dims )
            {
                //create copy of matrix with multiplying value
                CvSparseMatIterator iterator;
                CvSparseNode* node;
                for( node = cvInitSparseMatIterator( cvMatMult, &iterator );
                         node != 0; node = cvGetNextSparseNode( &iterator ))
                {
                    idx = CV_NODE_IDX( cvMatMult, node );
                    newVal = *(Type*)CV_NODE_VAL( cvMatMult, node );
                    uchar* valInNew = cvPtrND( mat, idx, NULL, 1, 0 );
                    *((Type*)valInNew) = uniVal*newVal;
                }
                return;
            }
            //need to compute indices to replicate small matrix
            int numBigDims = GetNumberDims();
            int numDimsDif = numBigDims - numSmDims;
            //compute other indices
            intVector indicesOtherInSelf;
            indicesOtherInSelf.assign( numDimsDif, 0);
            int t = 0;
            int numDiffCombs = 1;
            for( i = 0; i < numBigDims; i++ )
            {
                int loc = std::find( indicesToMultInSelf, 
                    indicesToMultInSelf + numSmDims, i ) - indicesToMultInSelf;
                if( loc >= numSmDims )
                {
                    indicesOtherInSelf[t] = i;
                    numDiffCombs *= mat->size[i];
                    t++;
                }
            }
            if( t != numDimsDif )
            {
                PNL_THROW( CInvalidOperation, "number of dims must correspondse" );
            }
            intVector dimsProductsOther;
            dimsProductsOther.resize( numDimsDif );
            int line = 1;
            for( i = numDimsDif - 1; i >= 0; i-- )
            {
                dimsProductsOther[i] = line;
                int dimSize = mat->size[indicesOtherInSelf[i]];
                line = line*dimSize;
            }
            //we need to put new values in matrix on all positions corresponding 
            //non-zero result of multiplying of uniVal and other matrix elements
            div_t result;
            CvSparseMatIterator iterator;
            CvSparseNode* node;
            for( node = cvInitSparseMatIterator( cvMatMult, &iterator );
                         node != 0; node = cvGetNextSparseNode( &iterator ))
            {
                idx = CV_NODE_IDX( cvMatMult, node );
                intVector indexInBig;
                indexInBig.assign( numBigDims,0 );
                for( i = 0; i < numSmDims; i++ )
                {
                    indexInBig[indicesToMultInSelf[i]] = idx[i];
                }
                newVal = *(Type*)CV_NODE_VAL( cvMatMult, node );
                //need to set all other indices
                for( i = 0; i < numDiffCombs; i++ )
                {
                    int res = i;
                    //convert to multidimindices
                    for( int k = 0; k < numDimsDif; k++ )
                    {
                        int ind = dimsProductsOther[k];
                        result = div( res, ind );
                        indexInBig[indicesOtherInSelf[k]] = result.quot;
                        res = result.rem;
                    }
                    uchar* valInNew = cvPtrND( mat, &indexInBig.front(),
                                                    NULL, 1, 0 );
                    *((Type*)valInNew) = uniVal*newVal;
                }
            }
        }
        else
        {
            //the matrix is matrix of zeros as it was befor
            ClearData();
            SetDefaultVal(Type(0));
        }
    }
    else
    {
        //create storage for indices of deleting nodes
        //pnlVector<unsigned int> hasvalsToDelete;
        intVecVector indicesToDelete;
        int indexSize = mat->dims;
        intVector index;
        intVector indexInSmall;
        indexInSmall.resize( numDimsToMult );
        CvSparseMatIterator iterator;
        CvSparseNode* node;
        for( node = cvInitSparseMatIterator( mat, &iterator );
                     node != 0; node = cvGetNextSparseNode( &iterator ))
        {
            idx = CV_NODE_IDX( mat, node );
            for( i = 0; i < numDimsToMult; i++ )
            {
                indexInSmall[i] = idx[indicesToMultInSelf[i]];
            }
            uchar* valInMult = cvPtrND( cvMatMult, &indexInSmall.front() );
            //check if in small matrix there is such value - multiply them
            if( valInMult )
            {
                void* val = CV_NODE_VAL( mat, node );
                newVal = (*(Type*)valInMult)*(*(Type*)val);
                if( Type(fabs(newVal)) > zeroEpsilon )
                {
                    *((Type*)val) = newVal;
                }
                else
                {
                    //unsigned int hasval = node->hashval;
                    //hasvalsToDelete.push_back( hasval );
                    index.assign( idx, idx + indexSize );
                    indicesToDelete.push_back( index );
                }
            }
            else
            {
                //need to delete node from matrix
                //unsigned int hasval = node->hashval;
                //hasvalsToDelete.push_back( hasval );
                index.assign( idx, idx + indexSize );
                indicesToDelete.push_back( index );
            }
        }
        //delete nodes that need to be deleted
        int numDeleteNodes = indicesToDelete.size();
        for( i = numDeleteNodes - 1; i >= 0; i-- )
        {
            cvClearND( mat, &(indicesToDelete[i].front()) );
        }
    }
}

template <class Type>
void iCNumericSparseMatrix<Type>::DivideInSelf(
                            const CMatrix<Type>* matToDiv,
                            int numDimsToDiv, const int* indicesToDivInSelf)
{
    EMatrixClass otherClass = matToDiv->GetMatrixClass();
    if( otherClass != mcNumericSparse )
    {
        PNL_THROW( CInvalidOperation, "can't multiply dense in sparse" );
    }
    int i;
    const CvSparseMat* mat = GetCvSparseMat();
    const iCNumericSparseMatrix<Type>* matToDivSp = static_cast<const
        iCNumericSparseMatrix<Type>*>(matToDiv);
    const CvSparseMat* cvMatDiv = matToDivSp->GetCvSparseMat();
    int numSmDims = cvMatDiv->dims;
    if( numSmDims != numDimsToDiv )
    {
        PNL_THROW( CInconsistentSize, "numDimsToDiv must be the same as " )
    }
    int* idx;
    Type newVal;
    intVector indexInSmall;
    indexInSmall.resize( numDimsToDiv );
    CvSparseMatIterator iterator;
    CvSparseNode* node;
    for( node = cvInitSparseMatIterator( mat, &iterator );
                 node != 0; node = cvGetNextSparseNode( &iterator ))
    {
        idx = CV_NODE_IDX( mat, node );
        for( i = 0; i < numDimsToDiv; i++ )
        {
            indexInSmall[i] = idx[indicesToDivInSelf[i]];
        }
        uchar* valInDiv = cvPtrND( cvMatDiv, &indexInSmall.front() );
        //check if in small matrix there is such value - multiply them
        if( valInDiv )
        {
            Type divVal = *(Type*)valInDiv;
            //we needn't to divide if there is zero val
            if( divVal )
            {
                void* val = CV_NODE_VAL( mat, node );
                newVal = (*(Type*)val)/(*(Type*)valInDiv);
                //memcpy(val, &newVal, sizeof(Type));
                *((Type*)val) = newVal;
            }
        }
        //if thre isn't such value - no needn't to divide
    }
}

template <class Type>
void iCNumericSparseMatrix<Type>::GetIndicesOfMaxValue( intVector* indices)
                                                                 const
{
    PNL_CHECK_IS_NULL_POINTER( indices );

    const CvSparseMat* selfMat = GetCvSparseMat();
    int numDims = selfMat->dims;
    CvSparseMatIterator iterator;
    CvSparseNode* node  = cvInitSparseMatIterator( selfMat, &iterator );
    if( !node )
    {
        //no node - max indices are empty
        indices->assign( numDims, 0 );
        return;
    }
    CvSparseNode* maxNode = node;
    for( node = cvInitSparseMatIterator( selfMat, &iterator );
                 node != 0; node = cvGetNextSparseNode( &iterator ))
    {
        void* valCur = CV_NODE_VAL( selfMat, node );
        void* valMax = CV_NODE_VAL( selfMat, maxNode );
        if( (*(Type*)valCur) > (*(Type*)valMax ))
        {
            maxNode = node;
        }
    }
    int* idx = CV_NODE_IDX( selfMat, maxNode);
    indices->assign( idx, idx + numDims );
}

//implement virtual methods from base 
template <class Type>
inline int iCNumericSparseMatrix<Type>::GetNumberDims() const
{
    return CSparseMatrix<Type>::GetNumberDims();
}
template <class Type>
inline void iCNumericSparseMatrix<Type>::GetRanges(int *numOfDims,
                                         const int **ranges) const
{
    CSparseMatrix<Type>::GetRanges(numOfDims, ranges);
}
//methods for viewing/change data

//this function return an element of matrix for
//given multidimensional indexes
template <class Type>
inline Type iCNumericSparseMatrix<Type>::GetElementByIndexes(
                                           const int *multidimindexes) const
{
    return CSparseMatrix<Type>::GetElementByIndexes(multidimindexes);
}
//this function set matrix element equal value (for
//given multidimensional indexes )
template <class Type>
inline void iCNumericSparseMatrix<Type>::SetElementByIndexes(Type value,
                                               const int *multidimindexes)
{
    CSparseMatrix<Type>::SetElementByIndexes( value, multidimindexes );
}

template <class Type>
CMatrix<Type>* iCNumericSparseMatrix<Type>::ExpandDims(
                     const int *dimsToExtend,
                     const int *keepPosOfDims,
                     const int *sizesOfExpandDims,
                     int numDimsToExpand) const
{
    return CSparseMatrix<Type>::ExpandDims( dimsToExtend, keepPosOfDims,
        sizesOfExpandDims, numDimsToExpand);
}
template <class Type>
inline void iCNumericSparseMatrix<Type>::ClearData()
{
    CSparseMatrix<Type>::ClearData();
}

template <class Type>
inline void iCNumericSparseMatrix<Type>::SetUnitData() 
{
    //ClearData();
    //SetDefaultVal(static_cast<Type>(1));
}


/*
template <class Type>
inline void iCNumericSparseMatrix<Type>::SetUnitValuesOneByOne()
{
    Type val = static_cast<float>(1);

    int nlineBig = 1;
    //filling vector to convert from line index to multidimentional
    intVector pconvIndBig;
    pconvIndBig.resize( m_Dim );
    for( int i1 = m_Dim-1; i1 >= 0; i1--)
    {
        pconvIndBig[i1] = nlineBig;
        int bigNodeSize = m_pCvSparseMat->dims[i1];
        nlineBig = nlineBig*bigNodeSize;
    }
    div_t result;
    intVector pInd;
    pInd.resize( m_Dim );
    for( int i = 0; i < nlineBig; i++ )
    {
        int hres = i;
        for( int k = 0; k < m_Dim; k++ )
        {
            result = div( hres, pconvIndBig[k] );
            pInd[k] = result.quot;
            hres = result.rem;
        }
        resMat->SetElementByIndexes( val, &pInd.front() );
    }
    return resMat;
}
*/

template <class Type>
iCNumericSparseMatrix<Type>::iCNumericSparseMatrix(int dim, const int *range,
                                                 int Clamp  ):
CSparseMatrix<Type>( dim, range, Type(0), Clamp )//, CMatrix<Type>(Clamp)
{
}

template <class Type>
iCNumericSparseMatrix<Type>::iCNumericSparseMatrix( const 
                                       iCNumericSparseMatrix<Type>& inputMat)  
                                      :CSparseMatrix<Type>( inputMat )
                                      //,CMatrix<Type>(0)
{
}

template <class Type>
iCNumericSparseMatrix<Type>::iCNumericSparseMatrix( CvSparseMat* p_sparse)
:CSparseMatrix<Type>( p_sparse, Type(0) )// , CMatrix<Type>(0)
{
}

template< typename ELTYPE >
CMatrix< ELTYPE > *
iCNumericSparseMatrix< ELTYPE >::ReduceOp( const int *pDimsOfInterest,
					  int numDimsOfInterest,
					  int action,
					  const int *pObservedValues,
					  CMatrix< ELTYPE > *output,
					  EAccumType accum_type ) const
{
    int i;
    int num_dims;
    int *idx;
    const int *ranges;
    int num_dims_to_keep;
    CvSparseNode *nd;
    CvSparseMatIterator it;

    PNL_DEFINE_AUTOBUF( bool, mask, PNL_SPARSE_MATRIX_SOFT_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, dims_to_keep, PNL_SPARSE_MATRIX_SOFT_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, new_ranges, PNL_SPARSE_MATRIX_SOFT_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, stats, PNL_SPARSE_MATRIX_SOFT_MAX_DIM );

    PNL_MAKE_LOCAL( CvSparseMat *, mat, this, GetCvSparseMat() );
//    PNL_MAKE_LOCAL( ELTYPE, inival, this, GetDefaultValue() );

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
 
    if ( output )
    {
        if ( accum_type == PNL_ACCUM_TYPE_STORE )
        {
            output->ClearData();
        }
    }
    else
    {
        output = CNumericSparseMatrix< ELTYPE >::Create( action == 2 ? num_dims : num_dims_to_keep,
                                                         new_ranges, 0 );
    }

    PNL_MAKE_LOCAL( CvSparseMat *, tgt, (CNumericSparseMatrix< ELTYPE > *)output, GetCvSparseMat() );
    PNL_DEMAND_AUTOBUF( stats, num_dims );
    nd = cvInitSparseMatIterator( mat, &it );
    if ( action < 2 )
    {
        if ( action == 0 )
        {
            while ( nd )
            {
                idx = CV_NODE_IDX( mat, nd );
                for ( i = num_dims_to_keep; i--; )
                {
                    stats[i] = idx[dims_to_keep[i]];
                }
                //nnd = (CvSparseNode*)cvPtrND( tgt, stats );
                //nnd = (CvSparseNode *)cvPtrND( tgt, stats, 0, 1 );
                //*(ELTYPE *)CV_NODE_VAL( tgt, nnd ) += *(ELTYPE *)CV_NODE_VAL( mat, nd );

                *(ELTYPE *)cvPtrND( tgt, stats ) += *(ELTYPE *)CV_NODE_VAL( mat, nd );
                nd = cvGetNextSparseNode( &it );
            }
        }
        else
        {
            while ( nd )
            {
                idx = CV_NODE_IDX( mat, nd );
                for ( i = num_dims_to_keep; i--; )
                {
                    stats[i] = idx[dims_to_keep[i]];
                }
                //nnd = (CvSparseNode*)cvPtrND( tgt, stats );
                //nnd = (CvSparseNode *)cvPtrND( tgt, stats, 0, 1 );
                ELTYPE *el = (ELTYPE *)cvPtrND( tgt, stats );
                ELTYPE *ll = (ELTYPE *)CV_NODE_VAL( mat, nd );
                if ( *ll > *el ) *el = *ll;
                nd = cvGetNextSparseNode( &it );
            }
        }
    }
    else
    {
        for ( i = num_dims; i--; )
        {
            stats[i] = 0;
        }
        if ( accum_type == PNL_ACCUM_TYPE_STORE )
        {
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
#if 1

                *(ELTYPE *)cvPtrND( tgt, stats ) = *(ELTYPE *)CV_NODE_VAL( mat, nd );
#else
                CvSparseNode* nnd = (CvSparseNode *)cvPtrND( tgt, stats, 0, 1, 0 );
                *(ELTYPE *)CV_NODE_VAL( tgt, nnd ) = *(ELTYPE *)CV_NODE_VAL( mat, nd );
#endif
                cont:
                nd = cvGetNextSparseNode( &it );
            }
        }
        else if ( accum_type == PNL_ACCUM_TYPE_MUL )
        {
            while ( nd )
            {
                idx = CV_NODE_IDX( mat, nd );
                for ( i = numDimsOfInterest; i--; )
                {
                    if ( idx[pDimsOfInterest[i]] != pObservedValues[i] )
                    {
                        goto cont2;
                    }
                }
                for ( i = num_dims_to_keep; i--; )
                {
                    stats[dims_to_keep[i]] = idx[dims_to_keep[i]];
                }
                //nnd = (CvSparseNode*)cvPtrND( tgt, stats );
#if 1

                *(ELTYPE *)cvPtrND( tgt, stats ) *= *(ELTYPE *)CV_NODE_VAL( mat, nd );
#else
                CvSparseNode* nnd = (CvSparseNode *)cvPtrND( tgt, stats, 0, 1, 0 );
                *(ELTYPE *)CV_NODE_VAL( tgt, nnd ) = *(ELTYPE *)CV_NODE_VAL( mat, nd );
#endif
                cont2:
                nd = cvGetNextSparseNode( &it );
            }
        }
        else
        {
            PNL_THROW( CInvalidOperation, "unsupported accumulation type" );
        }
    }

    PNL_RELEASE_AUTOBUF( stats );
    PNL_RELEASE_AUTOBUF( new_ranges );
    PNL_RELEASE_AUTOBUF( dims_to_keep );
    PNL_RELEASE_AUTOBUF( mask );

    return (CMatrix< ELTYPE > *)(CSparseMatrix< ELTYPE > *)output;
}

#endif //SWIG


#ifdef PNL_RTTI
template <class Type>
const CPNLType iCNumericSparseMatrix< Type >::m_TypeInfo = CPNLType("iCNumericSparseMatrix", &(CSparseMatrix< Type >::m_TypeInfo));

#endif
PNL_END

#endif //__PNLINUMERICSPARSEMATRIX_HPP__
