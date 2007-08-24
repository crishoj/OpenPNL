/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlDenseMatrix.hpp                                          //
//                                                                         //
//  Purpose:   CDenseMatrix template class definition & implementation     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLDENSEMATRIX_HPP__
#define __PNLDENSEMATRIX_HPP__

#include "pnlMatrix.hpp"
#include "pnlMatrixIterator.hpp"
#include "pnlSparseMatrix.hpp"
#include "pnlFakePtr.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

template <class Type> class PNL_API CDenseMatrix : public /*virtual*/ CMatrix<Type>
{
public:
    static CDenseMatrix<Type> *Create( int dim, const int *range, const Type *data,
                int Clamp = 0 );
    static CDenseMatrix<Type> *Copy( CDenseMatrix<Type>*const  inputMat );
    virtual CMatrix<Type>* CreateEmptyMatrix( int dim, const int *range,
        int Clamp, Type defaultVal = Type(0) )const ;
    virtual CMatrix<Type>* Clone() const;
    //methods for conversion from other type
    //if requered type is the same as type of argument - return copy of object
    virtual CDenseMatrix<Type>* ConvertToDense() const;
    virtual CSparseMatrix<Type>* ConvertToSparse() const;
    virtual EMatrixClass GetMatrixClass() const;
    ~CDenseMatrix(){};
    CDenseMatrix<Type> & operator = ( const CDenseMatrix<Type>& inputMat );
    //partition matrix into blocks
    //blocks - according node sizes
    /*void GetBlocks( int *X, int xSize, int *blockSizes, int numBlocks,
        CDenseMatrix<Type> **matX, CDenseMatrix<Type> **matY,
        CDenseMatrix<Type> **matXY, CDenseMatrix<Type> **matYX )const;*/
    void GetRawData(int *data_length, const Type **data) const;
    //Returns the reference on the array
    inline int GetNumberDims() const;
    inline void GetRanges(int *numOfDims, const int **ranges) const;
    inline int GetRawDataLength() const;
    void SetData(const Type* NewData);
    virtual void SetDataFromOtherMatrix( const CMatrix<Type>* matInput );
    //auxillary methods to work with matrix
    inline int ConvertMultiDimIndex( const int* multidimindexes ) const;
    //methods for viewing/change data
    inline Type GetElementByIndexes(const int *multidimindexes) const;
    inline Type GetElementByOffset(int linearindex) const;
    //these functions return an element of matrix for
    //given multidimensional indexes or linear index (offset)
    inline void SetElementByOffset(Type value, int offset);
    inline void SetElementByIndexes(Type value, const int *multidimindexes);
    //these functions set matrix element equal value (for
    //given multidimensional indexes or linear index)
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

    CMatrix<Type> *FixDims( int const *pDimsToFix, int numDimsToFix, int const *pObservedValues,
                            CDenseMatrix< Type > *output = 0,
                            EAccumType = PNL_ACCUM_TYPE_STORE ) const;

    inline void ClearData();
    virtual inline void SetUnitData(); 

    const pnlVector<Type>* GetVector() const;

    //iterator
    virtual CMatrixIterator<Type>* InitIterator() const;
    virtual void Next(CMatrixIterator<Type>* current) const;
//    virtual Type* Value(CMatrixIterator<Type>* current);
    virtual const Type* Value(CMatrixIterator<Type>* current) const;
    virtual int IsValueHere( CMatrixIterator<Type>* current ) const;
    virtual void Index( CMatrixIterator<Type>* current, intVector* index ) const;

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CDenseMatrix< int >::GetStaticTypeInfo();
    }
#endif

#if GCC_VERSION >= 30400
    using CMatrix<Type>::GetClampValue;
    using CMatrix<Type>::SetClamp;
#endif

protected:
    CDenseMatrix(int dim, const int *range, const Type *data, int Clamp);
    CDenseMatrix( const CDenseMatrix<Type> & inputMat );
    int m_Dim;  //Contain number of dimentions of matrix
    // (number of nodes in cliques - for factor or number of nodes in family -
    // for CPD)
    intVector m_Range;
    //array of node ranges (nodes which are discribed by this Matrix)
    pnlVector <Type> m_Table;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
};

template <class Type>
static void SumVector( typename pnlVector<Type>::iterator i1,
                       typename pnlVector<Type>::const_iterator i2, int length,
                       int isPlus )
{
    int k;
    for( k = 0; k < length; k++ )
    {
        *(i1+k) = (*(i1+k)) + (isPlus)*(*(i2+k));
    }
}

template <class Type>
static void maxVector( typename pnlVector<Type>::iterator i1,
                       typename pnlVector<Type>::const_iterator i2, int length )
{
    int k;
    for( k = 0; k < length; k++ )
    {
        if( *(i1 + k) < *(i2 + k) )
        {
            *(i1 + k) = *(i2 + k);
        }
    }
}

template <class Type>
static void ChooseValue( typename pnlVector<Type>::iterator i1, int part_length, int value )
{
    int k;
    int number = value*part_length;
    for( k = 0; k < part_length; k++ )
    {
        *(i1+k) = *(i1+number+k);
    }
}

#if !defined(SWIG)

template <class Type>
CDenseMatrix<Type> * CDenseMatrix<Type>::Create( int dim, const int *range,
                                                 const Type *data, int Clamp)
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
        }
    }
    CDenseMatrix<Type> *pxMatrix = new CDenseMatrix<Type> ( dim,
            range, data, (Clamp>0));
    return pxMatrix;
}

template <class Type>
CMatrix<Type> * CDenseMatrix<Type>::CreateEmptyMatrix( int dim, const int *range,
                                      int Clamp, Type defaultVal) const
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
    CDenseMatrix<Type> *pxMatrix = CDenseMatrix<Type>::Create( dim, range,
        &data.front(), (Clamp>0));
    return pxMatrix;
}

template <class Type>
CDenseMatrix<Type>* CDenseMatrix<Type>::Copy( CDenseMatrix<Type>* const inputMat)
{
    PNL_CHECK_IS_NULL_POINTER( inputMat );
    CDenseMatrix<Type> *retMat = new CDenseMatrix<Type>( *inputMat );
    PNL_CHECK_IF_MEMORY_ALLOCATED( retMat );
    return retMat;
}

template <class Type>
CMatrix<Type>* CDenseMatrix<Type>::Clone() const
{
    CDenseMatrix<Type>* res = new CDenseMatrix<Type>( *this );
    PNL_CHECK_IF_MEMORY_ALLOCATED( res );
    return res;
}

//methods for conversion from other type
//if requered type is the same as type of argument - return copy of object
template <class Type>
CDenseMatrix<Type>* CDenseMatrix<Type>::ConvertToDense() const
{
    return static_cast<CDenseMatrix<Type>*>(Clone());
}

template <class Type>
CSparseMatrix<Type>* CDenseMatrix<Type>::ConvertToSparse() const
{
    CSparseMatrix<Type>* resMat = CSparseMatrix<Type>::Create( m_Dim,
        &m_Range.front(), Type(0), GetClampValue() );
    int dataSize = m_Table.size();

    int nlineBig = 1;
    //filling vector to convert from line index to multidimentional
    intVector pconvIndBig;
    pconvIndBig.resize( m_Dim );
    for( int i1 = m_Dim-1; i1 >= 0; i1--)
    {
        pconvIndBig[i1] = nlineBig;
        int bigNodeSize = m_Range[i1];
        nlineBig = nlineBig*bigNodeSize;
    }
    div_t result;
    intVector pInd;
    pInd.resize( m_Dim );
    for( int i = 0; i < dataSize; i++ )
    {
        int hres = i;
        for( int k = 0; k < m_Dim; k++ )
        {
            result = div( hres, pconvIndBig[k] );
            pInd[k] = result.quot;
            hres = result.rem;
        }
        Type val = m_Table[i];
        resMat->SetElementByIndexes( val, &pInd.front() );
    }
    return resMat;
}

template <class Type>
EMatrixClass CDenseMatrix<Type>::GetMatrixClass() const
{
    return mcDense;
}

template <class Type>
void CDenseMatrix<Type>::SetDataFromOtherMatrix( const CMatrix<Type>* matInput )
{
    PNL_CHECK_IS_NULL_POINTER( matInput );

    CDenseMatrix<Type>* self = this;
    *self = *(static_cast<const CDenseMatrix<Type>*>(matInput));
}

template <class Type>
CDenseMatrix<Type>& CDenseMatrix<Type>::operator = ( const CDenseMatrix<Type>& inputMat )
{
    if( this != &inputMat )
    {
        if( GetClampValue() )
        {
            PNL_THROW( CInvalidOperation, "we can't change clamped matrix" )
        }
        int numDims = inputMat.m_Dim;
        intVector ranges = inputMat.m_Range;
        int isTheSame = 1;
        if( numDims != m_Dim )
        {
            isTheSame = 0;
        }
        int i;
        for( i = 0; i < numDims; i++ )
        {
            if( ranges[i] != m_Range[i] )
            {
                isTheSame = 0;
                break;
            }
        }
        if( isTheSame )
        {
            m_Table.assign(inputMat.m_Table.begin(), inputMat.m_Table.end());
        }
        else
        {
            PNL_THROW( CInvalidOperation,
                "we have only quick operator= for objects of the same size" );
        }
    }
    return *this;
}

#if 0
template<class Type>
void CDenseMatrix<Type>::GetBlocks( int *X, int xSize, int *blockSizes, int numBlocks,
                        CDenseMatrix<Type> **matX, CDenseMatrix<Type> **matY,
                        CDenseMatrix<Type> **matXY, CDenseMatrix<Type> **matYX )const
{
    if( m_Dim != 2 )
    {
        PNL_THROW( CInvalidOperation, "only for palne matrices" )
    }
    if( m_Range[0]!= m_Range[1] )
    {
        PNL_THROW( CInvalidOperation, "we can divide in blocks only square matrix" );
    }
    if( xSize > numBlocks )
    {
        PNL_THROW( COutOfRange, "xSize > numBlocks" );
    }
    if( xSize == numBlocks )
    {
        //we need to create full copy of this matrix as matX
        (*matX) = CDenseMatrix<Type>::Create( 2, &m_Range.front(), &m_Table.front(),
            GetClampValue() );
        (*matY) = NULL;
        (*matXY) = NULL;
        (*matYX) = NULL;
        return;
    }
    if( xSize == 0 )
    {
        // we need to create full copy of this matrix as matY
        (*matX) = NULL;
        (*matY) = CDenseMatrix<Type>::Create( 2, &m_Range.front(), &m_Table.front(),
            GetClampValue() );
        (*matXY) = NULL;
        (*matYX) = NULL;
        return;
    }
    int i,j;
    int sumBlockSizes = 0;
    for( i = 0; i < numBlocks; i++ )
    {
        sumBlockSizes += blockSizes[i];
    }
    if( sumBlockSizes != m_Range[0] )
    {
        PNL_THROW( CInconsistentSize, "block sizes should sum to matrix range" )
    }
    int yCount = 0;
    int xCount = 0;
    int isInX=0;
    int xSizeInTypes = 0;
    int ySizeInTypes = 0;
    intVector mapX( numBlocks, 0 );
    intVector xBlockSizes( xSize );
    int ySize = numBlocks - xSize;
    intVector yBlockSizes( ySize );
    //we need to create backmaps to find indices
    intVector xBackMap( numBlocks, -1 );
    intVector yBackMap( numBlocks, -1 );
    for( i = 0; i < numBlocks; i++ )
    {
        isInX = 0;
        for( j = 0; j < xSize; j++)
        {
            if( i == X[j])
            {
                xBackMap[i] = xCount;
                mapX[i] = 1;
                xSizeInTypes += blockSizes[i];
                xBlockSizes[xCount] = blockSizes[i];
                isInX = 1;
                xCount++;
                break;
            }
        }
        if( !isInX )
        {
            yBackMap[i] = yCount;
            ySizeInTypes += blockSizes[i];
            yBlockSizes[yCount] = blockSizes[i];
            yCount++;
        }
    }
    if( xCount != xSize )
    {
        PNL_THROW( CInconsistentSize, "some values from X more than number of blocks" )
    }
    intVector bigOffsets( numBlocks  );
    bigOffsets[0] = 0;
    intVector xOffsets( xSize );
    xOffsets[0] = 0;
    intVector yOffsets( ySize );
    yOffsets[0] = 0;
    for( i = 1; i < numBlocks; i++ )
    {
        bigOffsets[i] = bigOffsets[i-1]+blockSizes[i-1];
    }
    for( i = 1; i < ySize; i++ )
    {
        yOffsets[i] = yOffsets[i-1]+yBlockSizes[i-1];
    }
    for( i = 1; i < xSize; i++ )
    {
        xOffsets[i] = xOffsets[i-1]+xBlockSizes[i-1];
    }
    //create vectors to store data
    pnlVector<Type> dataX( xSizeInTypes*xSizeInTypes );
    pnlVector<Type> dataY( ySizeInTypes*ySizeInTypes );
    pnlVector<Type> dataXY( xSizeInTypes*ySizeInTypes );
    pnlVector<Type> dataYX( ySizeInTypes*xSizeInTypes );

    //now we need to copy data at all vectors by blocks
    //dataX - we need to find beginning of blocks
    int k;
    int numColumns = m_Range[0];
    for( i = 0; i < numBlocks; i++ )
    {
        for( j = 0; j < numBlocks; j++ )
        {
            if( mapX[i] && mapX[j] )//we put it to the matX
            {
                for( k = 0; k < blockSizes[j]; k++ )
                {
                    if( blockSizes[i] > 0 )
                    {
                        memcpy( &dataX.front() +
                            (xOffsets[xBackMap[j]] * xSizeInTypes +
                            xOffsets[xBackMap[i]] + k * xSizeInTypes),
                            &m_Table.front() + (bigOffsets[j] * numColumns +
                            bigOffsets[i] + k * (numColumns)),
                            blockSizes[i] * sizeof(Type)/sizeof(char) );
                    }
                }
            }
            if( (!mapX[i])&&(!mapX[j]) )//we put this block to the matY
            {
                for( k = 0; k < blockSizes[j]; k++ )
                {
                    if( blockSizes[i] > 0 )
                    {
                        memcpy( &dataY.front() +
                            (yOffsets[yBackMap[j]] * ySizeInTypes +
                            yOffsets[yBackMap[i]] + k * ySizeInTypes),
                            &m_Table.front() + (bigOffsets[j] * numColumns +
                            bigOffsets[i] + k * (numColumns)),
                            blockSizes[i] * sizeof(Type)/sizeof(char) );
                    }
                }
            }
            if( (mapX[i])&&(!mapX[j]) )//we put this block to the matYX
            {
                for( k = 0; k < blockSizes[j]; k++ )
                {
                    if( blockSizes[i] > 0 )
                    {
                        memcpy( &dataYX.front() +
                            (yOffsets[yBackMap[j]] * xSizeInTypes +
                            xOffsets[xBackMap[i]] + k * xSizeInTypes),
                            &m_Table.front() + (bigOffsets[i] +
                            bigOffsets[j] * numColumns + k * (numColumns)),
                            blockSizes[i] * sizeof(Type)/sizeof(char) );
                    }
                }
            }
            if( (!mapX[i])&&(mapX[j]) )// we put this block to the matXY
            {
                for( k = 0; k < blockSizes[j]; k++ )
                {
                    if( blockSizes[i] > 0 )
                    {
                        memcpy( &dataXY.front() +
                            ( xOffsets[xBackMap[j]] * ySizeInTypes +
                            yOffsets[yBackMap[i]] + k * ySizeInTypes),
                            &m_Table.front() + (bigOffsets[i] +
                            bigOffsets[j] * numColumns + k * (numColumns)),
                            blockSizes[i] * sizeof(Type)/sizeof(char) );
                    }
                }
            }
        }
    }
    intVector ranges( 2 );
    ranges[0] = xSizeInTypes;
    ranges[1] = xSizeInTypes;
    CMatrix<Type>* self = this;
    (*matX) = self->CreateEmptyMatrix( 2, &ranges.front(), 0);
    matX->SetData( &dataX.front() );
    ranges[1] = ySizeInTypes;
    (*matXY) = CDenseMatrix<Type>::Create( 2, &ranges.front(), &dataXY.front());
    ranges[0] = ySizeInTypes;
    (*matY) = CDenseMatrix<Type>::Create( 2, &ranges.front(), &dataY.front());
    ranges[1] = xSizeInTypes;
    (*matYX) = CDenseMatrix<Type>::Create( 2, &ranges.front(), &dataYX.front());
}
#endif

template<class Type>
void CDenseMatrix<Type>::GetRawData( int *data_length, const Type **data ) const
{
    (*data_length) = m_Table.size();
    (*data) = &m_Table.front();
}

template<class Type>
inline int CDenseMatrix<Type>::GetNumberDims() const
{
    return m_Dim;
}

template <class Type>
inline void CDenseMatrix<Type>::GetRanges(int *numOfDims,
                                        const int **ranges ) const
{
    PNL_CHECK_IS_NULL_POINTER( numOfDims );
    PNL_CHECK_IS_NULL_POINTER( ranges );

    (*numOfDims) = m_Dim;
    (*ranges) = &m_Range.front();
}

template <class Type>
inline int CDenseMatrix<Type>::GetRawDataLength() const
{
    return m_Table.size();
}


template <class Type>
void CDenseMatrix<Type>::SetData( const Type *pNewData )
{
    if( !GetClampValue() )
    {
        PNL_CHECK_IS_NULL_POINTER( pNewData );

        int size = m_Table.size();
        m_Table.assign( pNewData, pNewData+size );
    }
    else
    {
        PNL_THROW( CInvalidOperation, "can't change clamped matrix" );
    }

}

template <class Type>
inline int CDenseMatrix<Type>::ConvertMultiDimIndex( const int* multidimindexes ) const
{
    int offset = 0;

    for( int i = 0; i < m_Dim; i++ )
    {
        offset = offset*m_Range[i] + multidimindexes[i];
    }
    return offset;
}


template <class Type>
inline Type CDenseMatrix<Type>::GetElementByIndexes(const int *multidimindexes) const
{
    int offset = ConvertMultiDimIndex( multidimindexes );
    Type result = GetElementByOffset(offset);
    return result;
}

template <class Type>
    inline Type CDenseMatrix<Type>::GetElementByOffset(int offset) const
{
    return m_Table[offset];
}


template <class Type>
inline void CDenseMatrix<Type>::SetElementByIndexes( Type value,
                                                const int *multidimindexes)
{
    if( !GetClampValue() )
    {
        int offset = ConvertMultiDimIndex( multidimindexes );
        SetElementByOffset(value, offset);
    }
    else
    {
        PNL_THROW( CInvalidOperation, "can't change clamped matrix" )
    }
}

template <class Type>
    inline void CDenseMatrix<Type>::SetElementByOffset(Type value, int offset)
{
    if(!GetClampValue())
    {
        m_Table[offset] = value;
    }
    else
    {
        PNL_THROW( CInvalidOperation, "can't change clamped matrix" )
    }
}

#if 1
template< typename T >
CMatrix< T > *CDenseMatrix< T >::ReduceOp( const int *dims_of_interest,
                                           int num_dims_of_interest,
                                           int action, const int *observed_values,
                                           CMatrix< T > *output,
                                           EAccumType accum_type ) const
{
    if ( action != 2 )
    {
        PNL_THROW( CInconsistentType, "ReduceOp: action not supported" );
    }
    return FixDims( dims_of_interest, num_dims_of_interest, observed_values,
                    (CDenseMatrix< T > *)output, accum_type );
}

#else

#define PNL_MULTIDMATRIX_MAX_DIM 64

#define PNL_REDUCEOP_CPY( dst, src ) dst = src

#define PNL_REDUCEOP_LOOP_STEP( k ) \
    ++stats[k];                     \
    j += steps[k];

#define PNL_REDUCEOP_LOOP_CHECK( RANGES, NUM_STATS )  \
    for ( k = NUM_STATS - 1; stats[k] == RANGES[k]; ) \
    {                                                 \
        stats[k] = 0;                                 \
        j -= backsteps[k--];                          \
        PNL_REDUCEOP_LOOP_STEP( k );                  \
    }


#define PNL_REDUCEOP_LOOP2( ACTION, PIVOT )                      \
    for ( i = 0, j = PIVOT; i < new_bulk_size; ++i )             \
    {                                                            \
        PNL_REDUCEOP_LOOP_CHECK( new_ranges, num_dims_to_keep ); \
        ACTION( new_bulk[i], bulk[j] );                          \
        PNL_REDUCEOP_LOOP_STEP( num_dims_to_keep - 1 );          \
    }

template <class Type>
CMatrix<Type>* CDenseMatrix<Type>::ReduceOp( const int *pDimsOfInterest,
                           int numDimsOfInterest,
                           int action, const int *p0bservedValues,
                           CMatrix< Type > *output,
                           EAccumType accum_type ) const
{
    int i, j, k;
    int num_dims;
    const int* ranges;
    const Type* bulk;
    int* new_ranges;
    int new_bulk_size, safe_bulk_size, bulk_size;
    Type* new_bulk;
    int num_dims_to_keep;
    const int *dims_to_keep;
    int stats[PNL_MULTIDMATRIX_MAX_DIM];
    int steps[PNL_MULTIDMATRIX_MAX_DIM];
    int backsteps[PNL_MULTIDMATRIX_MAX_DIM];
    int buf[PNL_MULTIDMATRIX_MAX_DIM];
    bool mask[PNL_MULTIDMATRIX_MAX_DIM];

    if ( action != 2 )
    {
        PNL_THROW( CInconsistentType, "ReduceOp: action not supported" );
    }
    if ( accum_type != PNL_ACCUM_TYPE_STORE )
    {
        PNL_THROW( CInconsistentType, "ReduceOp: accumulation type not supported here" );
    }
    GetRanges( &num_dims, &ranges );
    GetRawData( &safe_bulk_size, &bulk );
    if ( num_dims > PNL_MULTIDMATRIX_MAX_DIM )
    {
        PNL_THROW( CInconsistentSize, "ReduceOp: num_dims -- capacity exceeded" );
    }
    if ( (unsigned)numDimsOfInterest > num_dims )
    {
        PNL_THROW( CInconsistentSize, "ReduceOp: broken numDimsOfInterest" );
    }
    for ( i = num_dims; i--; )
    {
        mask[i] = false;
    }
    for ( i = numDimsOfInterest; i--; )
    {
        if ( (unsigned)pDimsOfInterest[i] > num_dims )
        {
            PNL_THROW( CInconsistentSize, "ReduceOp: broken pDimsOfInterest" );
        }
        if ( mask[pDimsOfInterest[i]] )
        {
            PNL_THROW( CInconsistentSize, "ReduceOp: pDimsOfInterest clash" );
        }
        mask[pDimsOfInterest[i]] = true;
    }
    bulk_size = new_bulk_size = 1;
    for ( i = num_dims; i--; )
    {
        bulk_size *= ranges[i];
    }
    if ( safe_bulk_size < bulk_size )
    {
        PNL_THROW( CInconsistentSize, "ReduceOp: broken raw data size or something" );
    }
    for ( i = 0, j = 0; i < num_dims; ++i )
    {
        if ( !mask[i] )
        {
            buf[j++] = i;
        }
    }
    dims_to_keep = buf;
    num_dims_to_keep = j;

    while ( j-- )
    {
        stats[j] = 0;
    }
    new_ranges = new( int[num_dims] );
    PNL_CHECK_IF_MEMORY_ALLOCATED( new_ranges );
    for ( i = num_dims_to_keep; i--; )
    {
        new_bulk_size *= new_ranges[i] = ranges[dims_to_keep[i]];
    }
    new_bulk = new( Type[new_bulk_size] );
    PNL_CHECK_IF_MEMORY_ALLOCATED( new_bulk );
    int pivot;
    for ( i = num_dims, j = 1; i--; j *= ranges[i] )
    {
        steps[i] = j;
    }
    for ( i = numDimsOfInterest, pivot = 0; i--; )
    {
        pivot += steps[pDimsOfInterest[i]] * p0bservedValues[i];
    }
    for ( i = 0; i < num_dims_to_keep; ++i )
    {
        steps[i] = steps[dims_to_keep[i]];
    }
    while ( i-- )
    {
        backsteps[i] = steps[i] * new_ranges[i];
    }
    PNL_REDUCEOP_LOOP2( PNL_REDUCEOP_CPY, pivot );
    for ( i = num_dims; i--; )
    {
        new_ranges[i] = ranges[i];
    }
    for ( i = numDimsOfInterest; i--; )
    {
        new_ranges[pDimsOfInterest[i]] = 1;
    }
    num_dims_to_keep = num_dims;
    CDenseMatrix<Type> *retval = CDenseMatrix<Type>::Create( num_dims_to_keep,
        new_ranges, new_bulk );
    delete[]( new_bulk );
    delete[]( new_ranges );

    return retval;
}
#undef PNL_REDUCEOP_LOOP2
#undef PNL_REDUCEOP_LOOP
#undef PNL_REDUCEOP_CPY
#undef PNL_REDUCEOP_MAX
#undef PNL_REDUCEOP_ADD

#endif

template <class Type>
CMatrix<Type>* CDenseMatrix<Type>::ExpandDims( const int *dimsToExpand,
                    const int *keepPosOfDims, const int *sizesOfExpandDims,
                    int numDimsToExpand) const
{
    int i,k;
    PNL_CHECK_RIGHT_BORDER( numDimsToExpand, m_Dim );
    if( numDimsToExpand > m_Dim)
    {
        PNL_THROW( CInconsistentSize, "numDimsToExpand > m_Dim" )
    }
    //check dimsToExpand be a subset of dims
    int location;
    intVector existingDims;
    existingDims.resize( m_Dim );
    for( i = 0; i < m_Dim; i++ )
    {
        existingDims[i] = i;
    }
    for( i = 0; i < numDimsToExpand; i++ )
    {
        location = std::find( existingDims.begin(),
                existingDims.end(), dimsToExpand[i] ) - existingDims.begin();
        if( location < m_Dim )
        {
            existingDims.erase(existingDims.begin() + location);
        }
    }
    if( int(existingDims.size()) != m_Dim - numDimsToExpand )
    {
        PNL_THROW( CInconsistentSize, "dims to expand isn't subset of dims" )
    }
    //no we begin to compute
    //create ranges for new matrix
    intVector rangesNew;
    rangesNew.assign( m_Range.begin(), m_Range.end() );
    for( i = 0; i < numDimsToExpand; i++ )
    {
        rangesNew[dimsToExpand[i]] = sizesOfExpandDims[i];
    }

    /*filling the arrays pconvIndBig & pconvIndSmall, used to convert
    linear indexes to multidim & backwards in this matrix and expanded matrix*/
    int nlineBig = 1;
    int nlineSmall = 1;
    int *pconvIndBig = new int[m_Dim];
    PNL_CHECK_IF_MEMORY_ALLOCATED( pconvIndBig );
    int *pconvIndSmall = new int[m_Dim];
    PNL_CHECK_IF_MEMORY_ALLOCATED( pconvIndSmall );
    for ( int i1 = m_Dim-1; i1 >= 0; i1--)
    {
        pconvIndBig[i1] = nlineBig;
        nlineBig = nlineBig*rangesNew[i1];
        pconvIndSmall[i1] = nlineSmall;
        nlineSmall = nlineSmall*m_Range[i1];
    }
    //create array to find positions to put zeros in new data
    int ExtDataSize = 1;
    int smallDataSize = 1;
    for( i = 0; i < m_Dim; i++ )
    {
        ExtDataSize = ExtDataSize * rangesNew[i];
        smallDataSize = smallDataSize * m_Range[i];
    }
    //new data is a result of expanding old data
    pnlVector<Type> newDataVector;
    newDataVector.assign( ExtDataSize, (Type)0 );
    //we can fill some positions in newDataVector by dara from this matrix
    div_t result;
    int hres;
    int *pIndex = new int[m_Dim];
    PNL_CHECK_IF_MEMORY_ALLOCATED( pIndex );
    /*the main multiplication cycle begins*/
    for( i = 0; i < smallDataSize ; i++ )
    {
        //we put data from small matrix to needed positions in bigData
        hres = i;
        for( k=0; k < m_Dim; k++ )
        {
            result = div(hres, pconvIndSmall[k]);
            pIndex[k] = result.quot;
            hres = result.rem;
        }
        for( k = 0; k < numDimsToExpand; k++ )
        {
            pIndex[dimsToExpand[k]] = keepPosOfDims[k];
        }
        hres = 0;
        for( k = 0; k < m_Dim; k++ )
        {
            hres += pIndex[k]*pconvIndBig[k];
        }
        newDataVector[hres] = m_Table[i];
    }
    //create new matrix
    const CDenseMatrix<Type> *self = this;
    CMatrix<Type> *resultMatr = self->CreateEmptyMatrix( m_Dim,
        &rangesNew.front(), 0);
    static_cast<CDenseMatrix<Type>*>(resultMatr)->SetData( &newDataVector.front() );
    delete []pIndex;
    delete []pconvIndBig;
    delete []pconvIndSmall;
    return resultMatr;
}

template <class Type>
inline void pnl::CDenseMatrix<Type> :: ClearData()
{
    for( int i = 0; i < m_Table.size(); i++ )
    {
        m_Table[i] = 0;
    }
}

template <class Type>
inline void pnl::CDenseMatrix<Type>::SetUnitData()
{
    PNL_THROW( CInvalidOperation,
        "can't set unit data for matrix of pointers, for example" );
}


template <class Type>
const pnlVector<Type>* CDenseMatrix<Type>::GetVector() const
{
    return &m_Table;
}

template <class Type>
CDenseMatrix<Type>::CDenseMatrix( int dim, const int *range, const Type *data,
                                  int Clamp)
                                  : CMatrix<Type>(Clamp), m_Dim(dim)
{
    m_Range.assign(range, range+dim ) ;
    SetClamp(Clamp);
    int N = 1;
    for (int i = 0; i < dim; N = N * range[i], i++ );
    if ( data != PNL_FAKEPTR( Type ) )
    {
        m_Table.assign( data, data+N );
    }
    else
    {
        m_Table.resize( N );
    }
}

template <class Type>
CDenseMatrix<Type>::CDenseMatrix( const CDenseMatrix<Type>& inputMat )
                                  : CMatrix<Type>(0),
                                  m_Dim(inputMat.m_Dim),
                                  m_Range(inputMat.m_Range),
                                  m_Table(inputMat.m_Table)
{
}

template <class Type>
CMatrixIterator<Type>* CDenseMatrix<Type>::InitIterator() const
{
    CDenseMatrixIterator<Type>* iter = CDenseMatrixIterator<Type>::Create(
        &m_Table.front());
    return iter;
}

template <class Type>
void CDenseMatrix<Type>::Next(CMatrixIterator<Type>* current) const
{
    current->Iteration();
}
/*
template <class Type>
Type* CDenseMatrix<Type>::Value(CMatrixIterator<Type>* current)
{
    return (current->GetCurrent());
}*/
template <class Type>
const Type* CDenseMatrix<Type>:: Value(CMatrixIterator<Type>* current) const
{
    return (current->GetCurrent());
}

template <class Type>
int CDenseMatrix<Type>::IsValueHere( CMatrixIterator<Type>* current ) const
{
    if(( current->GetCurrent() - (&m_Table.front() + m_Table.size()) )>= 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

template <class Type>
void CDenseMatrix<Type>::Index( CMatrixIterator<Type>* current,
                               intVector* index ) const
{
    PNL_CHECK_IS_NULL_POINTER( index );
    int lineNum = current->GetCurrent() - &m_Table.front();
    index->resize( m_Dim );

    intVector pconvIndBig;
    pconvIndBig.resize( m_Dim );
    int nlineBig = 1;
    int i1;
    for ( i1 = m_Dim-1; i1 >= 0; i1-- )
    {
        pconvIndBig[i1] = nlineBig;
        nlineBig = nlineBig*m_Range[i1];
    }
    div_t result;
    for( i1 = 0; i1 < m_Dim; i1++ )
    {
        result = div(lineNum, pconvIndBig[i1]);
        (*index)[i1] = result.quot;
        lineNum = result.rem;
    }
}

#define PNL_MULTIDMATRIX_EXPECTED_MAX_DIM 16

#define PNL_MARG_INIT_STEPS( T, RANGES, NUM_DIMS, STEPS, BACK_STEPS )\
    for ( STEPS[i = NUM_DIMS - 1] = sizeof( T ); i > 0; --i )    \
    {                                                                \
        BACK_STEPS[i] = STEPS[i - 1] = STEPS[i] * RANGES[i];         \
    }                                                                \
    BACK_STEPS[0] = STEPS[0] * RANGES[0]

#define PNL_MARG_INIT_OFFSETS( DIMS, RANGES, NUM_BLOCKED_DIMS, BLOCK_SIZE,\
                               BLOCK_OFFSETS, STEPS, BACK_STEPS )         \
    if ( NUM_BLOCKED_DIMS == 0 )                                          \
    {                                                                     \
        BLOCK_OFFSETS[0] = 0;                                             \
    }                                                                     \
    else                                                                  \
    {                                                                     \
        for ( i = BLOCK_SIZE; i--; )                                      \
        {                                                                 \
            BLOCK_OFFSETS[i] = STEPS[DIMS[0]];                            \
        }                                                                 \
        for ( i = 1, k = RANGES[DIMS[0]]; i < NUM_BLOCKED_DIMS; k *= RANGES[DIMS[i++]] ) \
        {                                                                 \
            step = STEPS[DIMS[i]] - BACK_STEPS[DIMS[i - 1]];              \
            for ( j = 0; j < BLOCK_SIZE; j += k )                         \
            {                                                             \
                BLOCK_OFFSETS[j] += step;                                 \
            }                                                             \
        }                                                                 \
        for ( i = 1, BLOCK_OFFSETS[0] = 0; i < BLOCK_SIZE; ++i )          \
        {                                                                 \
            BLOCK_OFFSETS[i] += BLOCK_OFFSETS[i - 1];                     \
        }                                                                 \
    }

#if 1
PNL_API int margBlockize(int *aDim, int nDim, const int *aRange, int &blockSize,
		 int desired, int limit);

#define PNL_MARG_BLOCKIZE_REV( DIMS, NUM_DIMS, RANGES, NUM_BLOCKED_DIMS, BLOCK_SIZE, DESIRED, LIM )             \
    NUM_BLOCKED_DIMS = pnl::blockize( compose_unaries( compose_unaries( std::bind1st( subscript< int, int *, int >(), RANGES ), \
                                                                        std::bind1st( subscript< int, int *, int >(), DIMS ) ), \
                                                       std::bind1st( std::minus< int >(), NUM_DIMS - 1 ) ),     \
                                      NUM_DIMS, mul_in_self< int, int, int >(), 1,                              \
                                      std::bind2nd( std::greater< int >(), DESIRED ),                           \
                                      std::bind2nd( std::greater< int >(), LIM ), true, &BLOCK_SIZE )

#else

#define PNL_MARG_BLOCKIZE( DIMS, NUM_DIMS, RANGES, NUM_BLOCKED_DIMS, BLOCK_SIZE, DESIRED, LIM )     \
    if ( NUM_DIMS == 0 || DESIRED == 1 )                                                            \
    {                                                                                               \
        NUM_BLOCKED_DIMS = 0;                                                                       \
        BLOCK_SIZE = 1;                                                                             \
    }                                                                                               \
    else                                                                                            \
    {                                                                                               \
        for ( i = 0, BLOCK_SIZE = 1; i < NUM_DIMS; ++i )                                            \
        {                                                                                           \
            BLOCK_SIZE *= RANGES[DIMS[i]];                                                          \
            if ( BLOCK_SIZE >= DESIRED )                                                            \
            {                                                                                       \
                break;                                                                              \
            }                                                                                       \
        }                                                                                           \
        if ( i == NUM_DIMS )                                                                        \
        {                                                                                           \
            --i;                                                                                    \
        }                                                                                           \
        if ( BLOCK_SIZE > LIM )                                                                     \
        {                                                                                           \
            if ( i > 0 )                                                                            \
            {                                                                                       \
                BLOCK_SIZE /= RANGES[DIMS[i--]];                                                    \
            }                                                                                       \
        }                                                                                           \
        NUM_BLOCKED_DIMS = i + 1;                                                                   \
    }

#define PNL_MARG_BLOCKIZE_REV( DIMS, NUM_DIMS, RANGES, NUM_BLOCKED_DIMS, BLOCK_SIZE, DESIRED, LIM ) \
    if ( NUM_DIMS == 0 || DESIRED == 1 )                                                            \
    {                                                                                               \
        NUM_BLOCKED_DIMS = 0;                                                                       \
        BLOCK_SIZE = 1;                                                                             \
    }                                                                                               \
    else                                                                                            \
    {                                                                                               \
        for ( i = NUM_DIMS, BLOCK_SIZE = 1; i--; )                                                  \
        {                                                                                           \
            BLOCK_SIZE *= RANGES[DIMS[i]];                                                          \
            if ( BLOCK_SIZE >= DESIRED )                                                            \
            {                                                                                       \
                break;                                                                              \
            }                                                                                       \
        }                                                                                           \
        if ( i < 0 )                                                                                \
        {                                                                                           \
            ++i;                                                                                    \
        }                                                                                           \
        if ( BLOCK_SIZE > LIM )                                                                     \
        {                                                                                           \
            if ( i < NUM_DIMS - 1 )                                                                 \
            {                                                                                       \
                BLOCK_SIZE /= RANGES[DIMS[i++]];                                                    \
            }                                                                                       \
        }                                                                                           \
        NUM_BLOCKED_DIMS = NUM_DIMS - i;                                                            \
    }
#endif

#define PNL_FIXDIMS_DESIRED_BLOCK_SIZE 200
#define PNL_FIXDIMS_MAX_BLOCK_SIZE 1000

template< typename T >
pnl::CMatrix<T> *pnl::CDenseMatrix<T>::FixDims( int const *dims_to_fix,
                                                int num_dims_to_fix,
                                                int const *observed_values,
                                                pnl::CDenseMatrix< T > *output,
                                                pnl::EAccumType accum_type ) const
{
    PNL_USING

    int i, j, k;
    int num_dims;
    int const *src_ranges;
    T const *src_bulk;
    int src_bulk_size, safe_bulk_size;
    int dst_bulk_size, safe_dst_bulk_size;
    T *dst_bulk;
    int safe_dst_num_dims;
    int const *safe_dst_ranges;
    int block_size, num_blocked_dims;
    int step;
    T const *src_base;
    T *dst_base;

    PNL_DEFINE_AUTOBUF( bool, mask, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOSTACK( int, dims_to_trail, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, src_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, src_back_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, dst_ranges, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, block_offsets, PNL_FIXDIMS_MAX_BLOCK_SIZE );
    PNL_DEFINE_AUTOBUF( int, idx, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );

    GetRanges( &num_dims, &src_ranges );
    GetRawData( &safe_bulk_size, &src_bulk );

    PNL_DEMAND_AUTOBUF( mask, num_dims );
    PNL_DEMAND_AUTOSTACK( dims_to_trail, num_dims - num_dims_to_fix );

    for ( i = num_dims; i--; )
    {
        mask[i] = false;
    }
    for ( i = num_dims_to_fix; i--; )
    {
        mask[dims_to_fix[i]] = true;
    }
    for ( i = num_dims; i--; )
    {
        if ( !mask[i] && src_ranges[i] > 1 )
        {
            PNL_AUTOSTACK_PUSH( dims_to_trail, i );
        }
    }

    PNL_DEMAND_AUTOBUF( src_steps, num_dims );
    PNL_DEMAND_AUTOBUF( src_back_steps, num_dims );

    PNL_MARG_INIT_STEPS( T, src_ranges, num_dims, src_steps, src_back_steps );

    src_bulk_size = src_back_steps[0] / sizeof( T );
    if ( safe_bulk_size < src_bulk_size )
    {
        PNL_THROW( CInconsistentSize, "FixDims: broken raw data size or something" );
    }

    PNL_DEMAND_AUTOBUF( dst_ranges, num_dims );
    for ( i = num_dims; i--; )
    {
        dst_ranges[i] = src_ranges[i];
    }
    for ( i = num_dims_to_fix; i--; )
    {
        dst_ranges[dims_to_fix[i]] = 1;
    }
    for ( i = PNL_AUTOSTACK_SIZE( dims_to_trail ), dst_bulk_size = 1; i--; )
    {
        dst_bulk_size *= dst_ranges[dims_to_trail[i]];
    }

    if ( output )
    {
        output->GetRanges( &safe_dst_num_dims, &safe_dst_ranges );
        output->GetRawData( &safe_dst_bulk_size, (T const **)&dst_bulk );
        if ( safe_dst_bulk_size < dst_bulk_size )
        {
            PNL_THROW( CInconsistentSize, "Marg: output matrix does not fit" );
        }
#if 0 /* it is not so bad to check only dst_bulk_size, though if you paranoid you may check ranges */
        for ( i = num_dims; i--; )
        {
            if ( safe_dst_ranges[i] != dst_ranges[i] )
            {
                PNL_THROW( CInconsistentSize, "Marg: output matrix does not fit" );
            }
        }
#endif
    }
    else
    {
        output = CDenseMatrix< T >::Create( num_dims, dst_ranges, PNL_FAKEPTR( T ) );
        output->GetRawData( &safe_dst_bulk_size, (T const **)&dst_bulk );
    }
    PNL_DEMAND_AUTOBUF( idx, num_dims );

    src_base = src_bulk;
    for ( i = num_dims_to_fix; i--; )
    {
        src_base = (T *)((char *)src_base + observed_values[i] * src_steps[dims_to_fix[i]]);
    }

    if ( PNL_IS_AUTOSTACK_EMPTY( dims_to_trail ) )
    {
        dst_bulk[0] = src_base[0];
        goto endup;
    }

    num_blocked_dims = margBlockize( dims_to_trail, PNL_AUTOSTACK_SIZE( dims_to_trail ), src_ranges, block_size,
                       PNL_FIXDIMS_DESIRED_BLOCK_SIZE, PNL_FIXDIMS_MAX_BLOCK_SIZE );

    PNL_DEMAND_AUTOBUF( block_offsets, block_size );

    PNL_MARG_INIT_OFFSETS( dims_to_trail, src_ranges, num_blocked_dims, block_size,
                           block_offsets, src_steps, src_back_steps );

    for ( i = PNL_AUTOSTACK_SIZE( dims_to_trail ); i--; )
    {
        idx[i] = 0;
    }
    dst_base = dst_bulk;
    
    if ( accum_type == PNL_ACCUM_TYPE_STORE )
    {
loop:
        for ( j = 0; j < block_size; )
        {
            *dst_base++ = *(T *)((char *)src_base + block_offsets[j++]);
        }
        for ( k = num_blocked_dims; k < PNL_AUTOSTACK_SIZE( dims_to_trail ); ++k )
        {
            j = dims_to_trail[k];
            src_base = (T *)((char *)src_base + src_steps[j]);
            if ( ++idx[k] < src_ranges[j] )
            {
                goto loop;
            }
            idx[k] = 0;
            src_base = (T *)((char *)src_base - src_back_steps[j]);
        }
    }
    else
    {
        PNL_THROW( CInvalidOperation, "unsupported accumulation type for that type of matrix" );
    }

 endup:
    PNL_RELEASE_AUTOBUF( idx );
    PNL_RELEASE_AUTOBUF( block_offsets );
    PNL_RELEASE_AUTOBUF( dst_ranges );
    PNL_RELEASE_AUTOBUF( src_back_steps );
    PNL_RELEASE_AUTOBUF( src_steps );
    PNL_RELEASE_AUTOSTACK( dims_to_trail );
    PNL_RELEASE_AUTOBUF( mask );

    return output;
}

#endif

#ifdef PNL_RTTI
template< typename T >
const CPNLType CDenseMatrix< T >::m_TypeInfo = CPNLType("CDenseMatrix", &(CMatrix< T >::m_TypeInfo));

#endif

PNL_END

#endif //__PNLDENSEMATRIX_HPP__
