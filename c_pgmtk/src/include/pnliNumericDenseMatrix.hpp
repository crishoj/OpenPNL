/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlinumericdensematrix.hpp                                  //
//                                                                         //
//  Purpose:   CNumericDenseMatrix template class definition &             //
//             implementation                                              //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLINUMERICDENSEMATRIX_HPP__
#define __PNLINUMERICDENSEMATRIX_HPP__

#include <float.h>
#include <math.h>

#if defined(_OPENMP) && !defined(PAR_OMP)
#include <omp.h>
#endif

#include "pnlDenseMatrix.hpp"
//#include "pnlNumericMatrix.hpp"
#include "pnlNumericSparseMatrix.hpp"
#include "pnlImpDefs.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

template< typename Type > class CNumericDenseMatrix;
template< typename Type > class iCNumericDenseMatrix;

template< typename Type >
CNumericDenseMatrix<Type> *pnlCombineNumericMatrices( const iCNumericDenseMatrix<Type> *mat1,
                                                      const iCNumericDenseMatrix<Type> *mat2,
                                                      int combineFlag );

template< typename Type > class iCNumericDenseMatrix :public CDenseMatrix<Type>//, public CMatrix<Type>
{
public:

    static iCNumericDenseMatrix<Type> *Copy( iCNumericDenseMatrix<Type> const *inputMat );
    virtual CMatrix<Type>* CreateEmptyMatrix( int dim, const int *range,
                                              int Clamp, Type defaultVal = Type(0) ) const;
    virtual CMatrix<Type>* Clone() const;

    //methods for conversion from other type
    //if requered type is the same as type of argument - return copy of object
    virtual CDenseMatrix<Type>* ConvertToDense() const;
    virtual CSparseMatrix<Type>* ConvertToSparse() const;

    virtual EMatrixClass GetMatrixClass() const;

#ifndef PNL_VC6
    friend PNL_API CNumericDenseMatrix<Type> *pnlCombineNumericMatrices<>( const iCNumericDenseMatrix<Type> *mat1,
                                                                           const iCNumericDenseMatrix<Type> *mat2,
                                                                           int combineFlag );
#else
    friend PNL_API CNumericDenseMatrix<Type> *pnlCombineNumericMatrices( const iCNumericDenseMatrix<Type> *mat1,
                                                                         const iCNumericDenseMatrix<Type> *mat2,
                                                                         int combineFlag );
#endif

    void CombineInSelf( const CNumericDenseMatrix<Type>* matAdd, int combineFlag = 1 );

    //combine flag = 1 if we want to sum matrices,
    //0 - substract second matrix from first,
    //-1  -choose max values from matrices
    virtual CMatrix<Type>* ReduceOp( int const *pDimsOfInterest, int numDimsOfInterest,
                                     int action = 2,
                                     int const *p0bservedValues = NULL,
                                     CMatrix< Type > *output = NULL,
                                     EAccumType = PNL_ACCUM_TYPE_STORE ) const;
    CMatrix<Type>* _ReduceOp( int const *pDimsOfInterest, int numDimsOfInterest,
                                     int action = 2,
                                     int const *p0bservedValues = NULL,
                                     CMatrix< Type > *output = NULL,
                                     EAccumType = PNL_ACCUM_TYPE_STORE ) const;

        virtual CMatrix<Type>* OldReduceOp( const int *pDimsOfInterest,
                           int numDimsOfInterest, int action = 2,
                           const int *p0bservedValues = NULL) const;
    CMatrix<Type>* _OldReduceOp( const int *pDimsOfInterest,
                           int numDimsOfInterest, int action = 2,
                           const int *p0bservedValues = NULL) const;
    /* action == 0 -- sum Marg
       action == 1 -- max Marg
       action == 2 -- FixDims */
    inline CMatrix<Type> *Marg( int const *pDimsToKeep, int numDimsToKeep, EMargType,
                                CNumericDenseMatrix< Type > *output = 0,
                                EAccumType = PNL_ACCUM_TYPE_STORE ) const;
    CMatrix<Type> *Marg2D( int const *pDimsToKeep, int numDimsToKeep, EMargType,
                                CNumericDenseMatrix< Type > *output = 0,
                                EAccumType = PNL_ACCUM_TYPE_STORE ) const;
 
    CMatrix<Type> *FixDims( int const *pDimsToFix, int numDimsToFix, int const *pObservedValues,
                            CNumericDenseMatrix< Type > *output = 0,
                            EAccumType = PNL_ACCUM_TYPE_STORE ) const;
    Type SumAll(int byAbsValue = 0) const;
    virtual CMatrix<Type>* NormalizeAll() const;
    virtual void Normalize();
    void MultiplyInSelf( CMatrix< Type > const *matToMult,
                         int numDimsToMult, int const *indicesToMultInSelf, int isUnifrom = 0,
                         Type const uniVal = Type(0) );
    void OldMultiplyInSelf( CMatrix< Type > const *matToMult,
                            int numDimsToMult, int const *indicesToMultInSelf, int isUnifrom = 0,
                            Type const uniVal = Type(0) );
    void DivideInSelf( const CMatrix<Type>* matToDiv,
                       int numDimsToDiv, const int* indicesToDivInSelf );
    void GetIndicesOfMaxValue( intVector* indices )const;

    //virtual methods from base class
    inline int GetNumberDims() const;
    inline void GetRanges(int *numOfDimsOut, const int **rangesOut) const;
    //methods for viewing/change data
    inline Type GetElementByIndexes(const int *multidimindexes) const;
    //these functions return an element of matrix for
    //given multidimensional indexes
    inline void SetElementByIndexes(Type value, const int *multidimindexes);
    //these functions set matrix element equal value (for
    //given multidimensional indexes )
    CMatrix<Type> *ExpandDims(const int *dimsToExtend,
                        const int *keepPosOfDims,
                        const int *sizesOfExpandDims,
                        int numDimsToExpand) const;
    inline void ClearData();
    inline void SetUnitData();
    ~iCNumericDenseMatrix(){};
//protected:
    iCNumericDenseMatrix(int dim, const int *range, const Type *data, int Clamp);
    iCNumericDenseMatrix( const iCNumericDenseMatrix<Type> & inputMat );
    CMatrix<Type> *Marg_plain( int const *pDimsToKeep, int numDimsToKeep, EMargType,
                               CNumericDenseMatrix< Type > *output = 0,
                               EAccumType = PNL_ACCUM_TYPE_STORE ) const;
    CMatrix<Type> *Marg_omp( int const *pDimsToKeep, int numDimsToKeep, EMargType,
                             CNumericDenseMatrix< Type > *output = 0,
                             EAccumType = PNL_ACCUM_TYPE_STORE ) const;
    void MultiplyInSelf_plain( const CMatrix<Type>* matToMult,
                               int numDimsToMult, const int* indicesToMultInSelf, int isUnifrom = 0,
                               const Type uniVal = Type(0) );
    void MultiplyInSelf2D_plain( const CMatrix<Type>* matToMult,
                               int numDimsToMult, const int* indicesToMultInSelf, int isUnifrom = 0,
                               const Type uniVal = Type(0) );
    void MultiplyInSelf_omp( const CMatrix<Type>* matToMult,
                             int numDimsToMult, const int* indicesToMultInSelf, int isUnifrom = 0,
                             const Type uniVal = Type(0) );
    /*CMatrix<Type> *_Marg2D( int const *pDimsToKeep, int numDimsToKeep, EMargType,
                                CNumericDenseMatrix< Type > *output = 0,
                                EAccumType = PNL_ACCUM_TYPE_STORE ) const;
    void _MultiplyInSelf2D_plain( const CMatrix<Type>* matToMult,
                               int numDimsToMult, const int* indicesToMultInSelf, int isUnifrom = 0,
                               const Type uniVal = Type(0) );*/

    void SumInSelf(const CMatrix<Type>* matToSum,
      int numDimsToSum, const int* indicesToSumInSelf, int isUnifrom = 0,
      const Type uniVal = Type(0));

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return iCNumericDenseMatrix< int >::GetStaticTypeInfo();
    }
#endif

#if GCC_VERSION >= 30400
    using CDenseMatrix<Type>::GetClampValue;
#endif

protected:

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 

#if GCC_VERSION >= 30400
    using CDenseMatrix<Type>::m_Dim;
    using CDenseMatrix<Type>::m_Range;
    using CDenseMatrix<Type>::m_Table;
#endif

private:
};

//this method sum(1), substract(0) and choose max values(-1) from 2 matrices of the same size

template <class Type>
CNumericDenseMatrix<Type>*
pnlCombineNumericMatrices( const iCNumericDenseMatrix<Type> *mat1,
                           const iCNumericDenseMatrix<Type> *mat2, int combineFlag = 1 )
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

    const pnlVector<Type> *data1 = mat1->GetVector();
    const pnlVector<Type> *data2 = mat2->GetVector();
    typename pnlVector<Type>::const_iterator it1 = data2->begin();
    pnlVector<Type> data;
    data.assign( data1->begin(), data1->end() );

    if( combineFlag > 0 )
    {
        SumVector<Type>( data.begin(), it1, data2->size(), combineFlag );
    }
    else
    {
        if( combineFlag == 0 )
        {
            SumVector<Type>( data.begin(), it1, data2->size(), -1 );
        }
        else
        {
            maxVector<Type>( data.begin(), it1, data2->size() );
        }
    }
    CNumericDenseMatrix<Type> *mat = CNumericDenseMatrix<Type>::Create( dim1, ranges1, &data.front() );
    return mat;
}

#if !defined(SWIG)

template <class Type>
void iCNumericDenseMatrix<Type>::CombineInSelf(
                                  const CNumericDenseMatrix<Type>* matAdd,
                                  int combineFlag)
{
    PNL_USING
    PNL_CHECK_IS_NULL_POINTER( matAdd );
    PNL_CHECK_RANGES( combineFlag, -1, 1 );


    int i;

    int dim; const int * ranges;
    matAdd->GetRanges(&dim, &ranges);
    if( m_Dim != dim )
    {
        PNL_THROW( CInconsistentSize, "matrix sizes");
    }
    for( i = 0; i < m_Dim; i++ )
    {
        if( m_Range[i] != ranges[i] )
        {
            PNL_THROW( CInconsistentSize, "matrix sizes");
        }
    }
    const pnlVector<Type>* data = matAdd->GetVector();
    typename pnlVector<Type>::const_iterator it1 = data->begin();
    typename pnlVector<Type>::iterator it = m_Table.begin();
    if( combineFlag > 0 )
    {
        SumVector<Type>( it, it1, data->size(), combineFlag );
    }
    else
    {
        if( combineFlag == 0 )
        {
            SumVector<Type>( m_Table.begin(), it1, data->size(), -1 );
        }
        else
        {
            maxVector<Type>( m_Table.begin(), it1, data->size() );
        }
    }
}

template <class Type>
CMatrix<Type>* iCNumericDenseMatrix<Type>::CreateEmptyMatrix( int dim,
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
    pnlVector<Type> data;
    data.assign( size, defaultVal );
    return CNumericDenseMatrix<Type>::Create( dim, range, &data.front(),Clamp );
}

template <class Type>
CDenseMatrix<Type>* iCNumericDenseMatrix<Type>::ConvertToDense() const
{
    PNL_USING
    return static_cast<CDenseMatrix<Type>*>(CDenseMatrix<Type>::Clone());
}

template <class Type>
CSparseMatrix<Type>* iCNumericDenseMatrix<Type>::ConvertToSparse() const
{
    PNL_USING
    CNumericSparseMatrix<Type>* resMat = CNumericSparseMatrix<Type>::Create(
        m_Dim, &m_Range.front(), GetClampValue() );
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
        if( Type(fabs(val)) )
        {
            resMat->SetElementByIndexes( val, &pInd.front() );
        }
    }
    return resMat;
}

template <class Type>
EMatrixClass iCNumericDenseMatrix<Type>::GetMatrixClass()const
{
    return mcNumericDense;
}

template <class Type>
iCNumericDenseMatrix<Type>* iCNumericDenseMatrix<Type>::Copy(
                                   const iCNumericDenseMatrix<Type>* inputMat)
{
    PNL_CHECK_IS_NULL_POINTER( inputMat );
    iCNumericDenseMatrix<Type> *retMat =
        new iCNumericDenseMatrix<Type>( *inputMat );
    PNL_CHECK_IF_MEMORY_ALLOCATED( retMat );
    return retMat;
}

template <class Type>
CMatrix<Type>* iCNumericDenseMatrix<Type>::Clone() const
{
    PNL_USING
    iCNumericDenseMatrix<Type>* res = new iCNumericDenseMatrix<Type>( *this );
    PNL_CHECK_IF_MEMORY_ALLOCATED( res );
    return static_cast<CDenseMatrix<Type>*>(res);
}

PNL_API int margBlockize(int *aDim, int nDim, const int *aRange, int &blockSize,
                 int desired, int limit);

#define PNL_MARG_DESIRED_BLOCK1_SIZE 3
#define PNL_MARG_MAX_BLOCK1_SIZE 7
#define PNL_MARG_DESIRED_BLOCK2_SIZE 30
#define PNL_MARG_MAX_BLOCK2_SIZE 300

#define PNL_MARG_MAX( dst, src ) \
{                                \
    T t = (src);                 \
    if ( (dst) < t ) (dst) = t;  \
}

#define PNL_MARG_REVMUL( src, dst ) PNL_MUL( dst, src )

#define PNL_MARG_LOAD_ZERO( op, src ) (op) = 0
#define PNL_MARG_LOAD_LOWLIM( op, src ) (op) = -FLT_MAX
#define PNL_MARG_LOAD_ONE( op, src ) (op) = 1

#define PNL_MARG_MAIN_LOOP_PROLOG( MASTER, SLAVE )                                         \
    for ( i = num_blocked_trail_dims; i < PNL_AUTOSTACK_SIZE( final_dims_to_trail ); ++i ) \
    {                                                                                      \
        idx[i] = 0;                                                                        \
    }                                                                                      \
    MASTER ## _base = (T *)((char *)MASTER ## _bulk + MASTER ## _shift);                   \
    SLAVE ## _base = (T *)((char *)SLAVE ## _bulk + SLAVE ## _shift)

#define PNL_MARG_SPECPROLOG( MASTER, SLAVE )                                                    \
    PNL_DEFINE_AUTOBUF( int, idx, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );                          \
    PNL_DEMAND_AUTOBUF( idx, MASTER ## _num_dims );                                             \
                                                                                                \
    T *SLAVE ## _base, *SLAVE ## _base2;                                                        \
    T *MASTER ## _base, *MASTER ## _base2;                                                      \
    T op;                                                                                       \
                                                                                                \
    int MASTER ## _shift = 0;                                                                   \
    int SLAVE ## _shift = 0;                                                                    \
    div_t dv;                                                                                   \
    PNL_MARG_MAIN_LOOP_PROLOG( MASTER, SLAVE );                                                 \
    dv = div( block_size, 8 )

#define PNL_MARG_SPECPROLOG_OMP( MASTER, SLAVE, PAR_DIMS, NUM_PAR_DIMS )                        \
    PNL_DEFINE_AUTOBUF( int, idx, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM * 2 );                      \
    PNL_DEMAND_AUTOBUF( idx, MASTER ## _num_dims );                                             \
                                                                                                \
    T *SLAVE ## _base, *SLAVE ## _base2;                                                        \
    T *MASTER ## _base, *MASTER ## _base2;                                                      \
    T op;                                                                                       \
                                                                                                \
    int thr = channel;                                                                          \
                                                                                                \
    int MASTER ## _shift = 0;                                                                   \
    int SLAVE ## _shift = 0;                                                                    \
    div_t dv;                                                                                   \
    for ( i = 0; i < NUM_PAR_DIMS; ++i )                                                        \
    {                                                                                           \
        dv = div( thr, MASTER ## _ranges[PAR_DIMS[i]] );                                        \
        MASTER ## _shift += dv.rem * MASTER ## _steps[PAR_DIMS[i]];                             \
        SLAVE ## _shift += dv.rem * corr_steps[PAR_DIMS[i]];                                    \
        thr = dv.quot;                                                                          \
    }                                                                                           \
    PNL_MARG_MAIN_LOOP_PROLOG( MASTER, SLAVE );                                                 \
    dv = div( block_size, 8 )

#define PNL_MARG_SPECEPILOG() \
    PNL_RELEASE_AUTOBUF( idx )

#define PNL_MARG_SPECEPILOG_OMP() \
    PNL_RELEASE_AUTOBUF( idx )

#define PNL_MARG_MAIN_LOOP_BULK2( MASTER, SLAVE, ACTION, INNER_LOAD, INNER_STORE, INNER_FOR, INNER_LENGTH, INNER_SPAWN, EXTRA_SPAWN ) \
{                                                                                             \
 loop_ ## ACTION ## INNER_LOAD ## INNER_FOR ## INNER_SPAWN ## EXTRA_SPAWN:                    \
    for ( j = 0; j < trail_block_size; ++j )                                                  \
    {                                                                                         \
        MASTER ## _base2 = (T *)((char *)MASTER ## _base + MASTER ## _trail_block_offsets[j]);\
        SLAVE ## _base2 = (T *)((char *)SLAVE ## _base + SLAVE ## _trail_block_offsets[j]);   \
        INNER_LOAD( op, *SLAVE ## _base2 );                                                   \
        INNER_FOR( i = 0; i < INNER_LENGTH; )                                                 \
        {                                                                                     \
            INNER_SPAWN                                                                       \
            (                                                                                 \
                ACTION( op, *(T *)((char *)MASTER ## _base2 + block_offsets[i++]) );          \
            );                                                                                \
        }                                                                                     \
        EXTRA_SPAWN                                                                           \
        (                                                                                     \
            ACTION( op, *(T *)((char *)MASTER ## _base2 + block_offsets[i++]) );              \
        )                                                                                     \
        INNER_STORE( *SLAVE ## _base2, op );                                                  \
    }                                                                                         \
    for ( k = num_blocked_trail_dims; k < PNL_AUTOSTACK_SIZE( final_dims_to_trail ); ++k )    \
    {                                                                                         \
        j = final_dims_to_trail[k];                                                           \
        MASTER ## _base = (T *)((char *)MASTER ## _base + MASTER ## _steps[j]);               \
        SLAVE ## _base = (T *)((char *)SLAVE ## _base + corr_steps[j]);                       \
        if ( ++idx[k] < MASTER ## _ranges[j] )                                                \
        {                                                                                     \
            goto loop_ ## ACTION ## INNER_LOAD ## INNER_FOR ## INNER_SPAWN ## EXTRA_SPAWN;    \
        }                                                                                     \
        idx[k] = 0;                                                                           \
        MASTER ## _base = (T *)((char *)MASTER ## _base - MASTER ## _back_steps[j]);          \
        SLAVE ## _base = (T *)((char *)SLAVE ## _base - corr_back_steps[j]);                  \
    }                                                                                         \
}

#define PNL_MARG_MAIN_LOOP2( MASTER, SLAVE, ACTION, INNER_LOAD, INNER_STORE, QUOT, REM ) \
    if ( QUOT == 0 )                                                                     \
    {                                                                                    \
        PNL_MARG_MAIN_LOOP_BULK2( MASTER, SLAVE, ACTION, INNER_LOAD, INNER_STORE, PNL_SPAWN1, block_size, PNL_VOID, PNL_SPAWN ## REM ); \
    }                                                                                    \
    else if ( QUOT == 1 )                                                                \
    {                                                                                    \
        PNL_MARG_MAIN_LOOP_BULK2( MASTER, SLAVE, ACTION, INNER_LOAD, INNER_STORE, PNL_SPAWN1, block_size - REM, PNL_SPAWN8, PNL_SPAWN ## REM ); \
    }                                                                                    \
    else                                                                                 \
    {                                                                                    \
        PNL_MARG_MAIN_LOOP_BULK2( MASTER, SLAVE, ACTION, INNER_LOAD, INNER_STORE, for, block_size - REM, PNL_SPAWN8, PNL_SPAWN ## REM ); \
    }

#define PNL_MARG_SPECBODY( MASTER, SLAVE, ACTION, INNER_LOAD, INNER_STORE )                             \
    if ( dv.rem < 4 )                                                                                   \
    {                                                                                                   \
        if ( dv.rem < 2 )                                                                               \
        {                                                                                               \
            if ( dv.rem < 1 )                                                                           \
            {                                                                                           \
                PNL_MARG_MAIN_LOOP2( MASTER, SLAVE, ACTION, INNER_LOAD, INNER_STORE, dv.quot, 0 );      \
            }                                                                                           \
            else                                                                                        \
            {                                                                                           \
                PNL_MARG_MAIN_LOOP2( MASTER, SLAVE, ACTION, INNER_LOAD, INNER_STORE, dv.quot, 1 );      \
            }                                                                                           \
        }                                                                                               \
        else                                                                                            \
        {                                                                                               \
            if ( dv.rem < 3 )                                                                           \
            {                                                                                           \
                PNL_MARG_MAIN_LOOP2( MASTER, SLAVE, ACTION, INNER_LOAD, INNER_STORE, dv.quot, 2 );      \
            }                                                                                           \
            else                                                                                        \
            {                                                                                           \
                PNL_MARG_MAIN_LOOP2( MASTER, SLAVE, ACTION, INNER_LOAD, INNER_STORE, dv.quot, 3 );      \
            }                                                                                           \
        }                                                                                               \
    }                                                                                                   \
    else                                                                                                \
    {                                                                                                   \
        if ( dv.rem < 6 )                                                                               \
        {                                                                                               \
            if ( dv.rem < 5 )                                                                           \
            {                                                                                           \
                PNL_MARG_MAIN_LOOP2( MASTER, SLAVE, ACTION, INNER_LOAD, INNER_STORE, dv.quot, 4 );      \
            }                                                                                           \
            else                                                                                        \
            {                                                                                           \
                PNL_MARG_MAIN_LOOP2( MASTER, SLAVE, ACTION, INNER_LOAD, INNER_STORE, dv.quot, 5 );      \
            }                                                                                           \
        }                                                                                               \
        else                                                                                            \
        {                                                                                               \
            if ( dv.rem < 7 )                                                                           \
            {                                                                                           \
                PNL_MARG_MAIN_LOOP2( MASTER, SLAVE, ACTION, INNER_LOAD, INNER_STORE, dv.quot, 6 );      \
            }                                                                                           \
            else                                                                                        \
            {                                                                                           \
                PNL_MARG_MAIN_LOOP2( MASTER, SLAVE, ACTION, INNER_LOAD, INNER_STORE, dv.quot, 7 );      \
            }                                                                                           \
        }                                                                                               \
    }


#define PNL_MULINSELF_SPECMPLEXOR( MASTER, SLAVE, ACTION, INNER_LOAD, INNER_STORE )                     \
{                                                                                                       \
    PNL_MARG_SPECPROLOG( MASTER, SLAVE );                                                               \
    PNL_MARG_SPECBODY( MASTER, SLAVE, ACTION, INNER_LOAD, INNER_STORE );                                \
    PNL_MARG_SPECEPILOG();                                                                              \
}

#define PNL_MULINSELF_SPECMPLEXOR_OMP( MASTER, SLAVE, ACTION, INNER_LOAD, INNER_STORE, PAR_DIMS, NUM_PAR_DIMS ) \
{                                                                                                               \
    PNL_MARG_SPECPROLOG_OMP( MASTER, SLAVE, PAR_DIMS, NUM_PAR_DIMS );                                           \
    PNL_MARG_SPECBODY( MASTER, SLAVE, ACTION, INNER_LOAD, INNER_STORE );                                        \
    PNL_MARG_SPECEPILOG();                                                                                      \
}

#define PNL_MARG_SPECSPECBODY( MASTER, SLAVE )                                                          \
        switch ( marg_type )                                                                            \
        {                                                                                               \
        case PNL_MARG_TYPE_SUM:                                                                         \
            if ( !need_ini )                                                                            \
            {                                                                                           \
                PNL_MARG_SPECBODY( MASTER, SLAVE, PNL_ADD, PNL_ASN, PNL_ASN );                          \
            }                                                                                           \
            else                                                                                        \
            {                                                                                           \
                PNL_MARG_SPECBODY( MASTER, SLAVE, PNL_ADD, PNL_MARG_LOAD_ZERO, PNL_ASN );               \
            }                                                                                           \
            break;                                                                                      \
        case PNL_MARG_TYPE_MAX:                                                                         \
            if ( !need_ini )                                                                            \
            {                                                                                           \
                PNL_MARG_SPECBODY( MASTER, SLAVE, PNL_MARG_MAX, PNL_ASN, PNL_ASN );                     \
            }                                                                                           \
            else                                                                                        \
            {                                                                                           \
                PNL_MARG_SPECBODY( MASTER, SLAVE, PNL_MARG_MAX, PNL_MARG_LOAD_LOWLIM, PNL_ASN );        \
            }                                                                                           \
        }                                                                                               \

#define PNL_MARG_SPECMPLEXOR( MASTER, SLAVE )                         \
{                                                                     \
    PNL_MARG_SPECPROLOG( MASTER, SLAVE );                             \
    PNL_MARG_SPECSPECBODY( MASTER, SLAVE );                           \
    PNL_MARG_SPECEPILOG();                                            \
}

#define PNL_MARG_SPECMPLEXOR_OMP( MASTER, SLAVE, PAR_DIMS, NUM_PAR_DIMS )  \
{                                                                          \
    PNL_MARG_SPECPROLOG_OMP( MASTER, SLAVE, PAR_DIMS, NUM_PAR_DIMS );      \
    PNL_MARG_SPECSPECBODY( MASTER, SLAVE )                                 \
    PNL_MARG_SPECEPILOG();                                                 \
}
/*
template< typename  T >
void iCNumericDenseMatrix< T >::MultiplyInSelf2D_plain( CMatrix< T > const *mat,
                                                           int num_dims_to_mul,
                                                           int const *dims_to_mul,
                                                           int, T const )
{
    PNL_USING
        assert(m_Dim <= 2);

        T*  mult_mat;
    const int* mult_ranges;
        int src_size = m_Table.size();
    int* ranges = &m_Range.front();
        T*  src_mat = m_Table.begin();
        assert( (unsigned)num_dims_to_mul < 3);
    int mult_mat_size = 1;
        

    ((iCNumericDenseMatrix< T >*)mat)->GetRawData( &mult_mat_size, (T const **)&mult_mat );
        assert(mult_mat_size <= src_size);

        mat->GetRanges(&mult_mat_size, &mult_ranges);
        
        assert(num_dims_to_mul == mult_mat_size);
        
        if ( num_dims_to_mul == m_Range.size() )
        {
                T* _src_mat = src_mat;
                T* _mult_mat = mult_mat;
                if ( dims_to_mul[0] == 0 || num_dims_to_mul == 1)
                {
                        for (int i = src_size ; i-- ; _src_mat++, _mult_mat++)
                                *_src_mat *= (*_mult_mat);
                }
                else
                {
                        for (int i = ranges[1]; i--; _mult_mat++)
                        {
                                T* __mult_mat = _mult_mat; 
                                for (int i = ranges[0]; i--; _src_mat ++, __mult_mat += ranges[1])
                                        *_src_mat *= *__mult_mat;
                        }
                }
        }
        else if ( num_dims_to_mul == 1 )
        {
                T* _src_mat = src_mat;
                T* _mult_mat = mult_mat;
                if (dims_to_mul[0] == 1)
                {
                        for (int i = ranges[1]; i--; _mult_mat++)
                        {
                                T mul = *_mult_mat; 
                                for (int i = ranges[0]; i--; _src_mat ++)
                                        *_src_mat *= mul;
                        }
                }
                else
                {
                        for (int i = ranges[0]; i--; _mult_mat++,_src_mat++)
                        {
                                T* __src_mat = _src_mat;
                                T mul = (*_mult_mat);
                                for (int i = ranges[1]; i--; __src_mat += ranges[0])
                                        *__src_mat *= mul;
                        }
                }
        }
        else
        {
                assert( num_dims_to_mul == 0);
                T mul = mult_mat[0];
                T* _src_mat = src_mat;
                for (int i = src_size ; i-- ; _src_mat++)
                        *_src_mat *= mul;
        }

}


template< typename T >
CMatrix<T> *iCNumericDenseMatrix<T>::Marg2D( int const *dims_to_keep,
                                                       int num_dims_to_keep,
                                                       EMargType marg_type,
                                                       CNumericDenseMatrix< T >*output,
                                                       EAccumType accum_type ) const
{
PNL_USING
    assert(m_Dim <= 2);
        const int* dest_ranges;
        T*  dest_mat;
        int safe_dst_bulk_size;
    int new_ranges[2];
        int src_size = m_Table.size();
    const int* ranges = &m_Range.front();
        const T*  src_mat = m_Table.begin();
        assert( (unsigned)num_dims_to_keep < 3);
    int dest_size = 1;
        
        if (num_dims_to_keep == 1)
        {
                dest_size = new_ranges[0] =  ranges[dims_to_keep[0]];
        }
        else if (num_dims_to_keep == 2)
        {
                dest_size = src_size;
                new_ranges[0] = ranges[dims_to_keep[0]];
                new_ranges[1] = ranges[dims_to_keep[1]];
        }
    if ( output )
    {
                int safe_dst_num_dims;
        output->GetRanges( &safe_dst_num_dims, &dest_ranges );
        output->GetRawData( &safe_dst_bulk_size, (T const **)&dest_mat );
                assert(safe_dst_bulk_size <= m_Table.size() );
                assert(dest_size == safe_dst_bulk_size);
                assert(num_dims_to_keep == safe_dst_num_dims);
    }
    else
    {
                int safe_dst_size= -1;
        output = CNumericDenseMatrix< T >::Create( num_dims_to_keep, new_ranges, PNL_FAKEPTR( T ) );
        output->GetRawData( &safe_dst_size, (T const **)&dest_mat );
                assert(safe_dst_size == dest_size);
    }
    switch ( marg_type )
    {
                case PNL_MARG_TYPE_SUM:
                {
                        if ( num_dims_to_keep == m_Range.size() )
                        {
                                const T* _src_mat = src_mat;
                                T* _dest_mat = dest_mat;
                
                        
                //              if (accum_type == PNL_ACCUM_TYPE_STORE)
                                {
                                        if (dims_to_keep[0] == 0)
                                        {
                                                memcpy(dest_mat, src_mat, src_size * sizeof(T) );
                                        //      for (int i = src_size; i--; _dest_mat++, _src_mat++)
                                        //              *_dest_mat = *_src_mat;
                                        }
                                        else
                                        {
                                                for (int i = ranges[1]; i--; _src_mat++)
                                                {
                                                        const T* __src_mat = _src_mat; 
                                                        for (int i = ranges[0]; i--; _dest_mat ++, __src_mat += ranges[1])
                                                                *_dest_mat = *__src_mat;
                                                }
                                        }
                                }
                //              else if (accum_type == PNL_ACCUM_TYPE_STORE)
                        

                        }
                        else if ( num_dims_to_keep == 1 )
                        {
                                if (accum_type == PNL_ACCUM_TYPE_STORE || accum_type == PNL_ACCUM_TYPE_ACCUMULATE)
                                        memset(dest_mat, 0, dest_size * sizeof (T));
                                const T* _src_mat = src_mat;
                                T* _dest_mat = dest_mat;
                                if (dims_to_keep[0] == 0)
                                {
                                        for (int i = ranges[0]; i--; _dest_mat++, _src_mat++)
                                        {
                                                const T* __src_mat = _src_mat; 
                                                T sum = 0;
                                                for (int i = ranges[1]; i--; __src_mat += ranges[0])
                                                        sum += *__src_mat;
                                                *_dest_mat += sum;
                                        }
                                }
                                else
                                {
                                        for (int i = ranges[1]; i--; _dest_mat++)
                                        {
                                                T sum = 0;
                                                for (int i = ranges[0]; i--; _src_mat ++)
                                                         sum += *_src_mat;
                                                *_dest_mat += sum;
                                        }
                                }
                        }
                        else
                        {
                                assert( num_dims_to_keep == 0);
                                if (accum_type == PNL_ACCUM_TYPE_STORE || accum_type == PNL_ACCUM_TYPE_ACCUMULATE)
                                        dest_mat[0] = 0;
                                T sum = 0;
                                const T* _src_mat = src_mat;
                                for (int i = src_size ; i-- ; _src_mat++)
                                        sum += *_src_mat;
                                dest_mat[0] += sum;
                        }
                }
        break;
                case PNL_MARG_TYPE_MAX:
                {
                        if ( num_dims_to_keep == 2 )
                        {
                                if (accum_type == PNL_ACCUM_TYPE_STORE || accum_type == PNL_ACCUM_TYPE_ACCUMULATE)
                                        memcpy( dest_mat, src_mat, dest_size * sizeof (T));
                                else if (accum_type == PNL_MARG_TYPE_MAX)
                                {
                                        const T* _src_mat = src_mat;
                                        T* _dest_mat = dest_mat;
                                        for (int i = dest_size; i--; _dest_mat++, _src_mat++)
                                        {
                                                T src = *_src_mat;
                                                *_dest_mat = __max(src , *_dest_mat );
                                        }
                                }
                                else
                                        assert(0);
                        }
                        else if ( num_dims_to_keep == 1 )
                        {
                                const T* _src_mat = src_mat;
                                T* _dest_mat = dest_mat;
                                if (dims_to_keep[0] == 0)
                                {
                                        for (int i = ranges[0]; i--; _dest_mat++, _src_mat++)
                                        {
                                                const T* __src_mat = _src_mat; 
                                                T max = -FLT_MAX;
                                                for (int i = ranges[1]; i--; __src_mat += ranges[0])
                                                        max = __max( *__src_mat, max);
                                                *_dest_mat = max;
                                        }
                                }
                                else
                                {
                                        for (int i = ranges[1]; i--; _dest_mat++)
                                        {
                                                T max = -FLT_MAX;
                                                for (int i = ranges[0]; i--; _src_mat ++)
                                                        max     = __max( *_src_mat , max);
                                                *_dest_mat = max;
                                        }
                                }
                        }
                        else
                        {
                                assert( num_dims_to_keep == 0);
                        //      if (accum_type == PNL_ACCUM_TYPE_STORE || accum_type == PNL_ACCUM_TYPE_ACCUMULATE)
                        //              dest_mat[0] = 0;
                                T max = 0;
                                const T* _src_mat = src_mat;
                                for (int i = src_size ; i-- ; _src_mat++)
                                        max = __max(max, *_src_mat);
                                dest_mat[0] = max;//__max ( max, dest_mat[0] );
                        }
                }
                break;
                default:
                        PNL_THROW( CInvalidOperation, "unsupported marg type" );
    }

  
    return output;
}
*/


template< typename  T >
void iCNumericDenseMatrix< T >::MultiplyInSelf2D_plain( CMatrix< T > const *mat,
                                                           int num_dims_to_mul,
                                                           int const *dims_to_mul,
                                                           int, T const )
{
    PNL_USING
        assert(m_Dim <= 2);

        T*  mult_mat;
    const int* mult_ranges;
        int src_size = m_Table.size();
    int* ranges = &m_Range.front();
        T*  src_mat = &m_Table.front();
        assert( (unsigned)num_dims_to_mul < 3);
    int mult_mat_size = 1;
        

    ((iCNumericDenseMatrix< T >*)mat)->GetRawData( &mult_mat_size, (T const **)&mult_mat );
        assert(mult_mat_size <= src_size);

        mat->GetRanges(&mult_mat_size, &mult_ranges);
        
        assert(num_dims_to_mul == mult_mat_size);

        T* _src_mat = src_mat;
        T* _mult_mat = mult_mat;
        
        if ( num_dims_to_mul == int(m_Range.size()) )
        {
                if ( dims_to_mul[0] == 0 )
                {
                        for (int i = src_size ; i-- ; _src_mat++, _mult_mat++)
                                *_src_mat *= (*_mult_mat);
                }
                else
                {
                        for (int i = ranges[0]; i--; _mult_mat++)
                        {
                                T* __mult_mat = _mult_mat; 
                                for (int j = ranges[1]; j--; _src_mat ++, __mult_mat += ranges[0])
                                        *_src_mat *= *__mult_mat;
                        }
                }
        }
        else if ( num_dims_to_mul == 1 )
        {
                if (dims_to_mul[0] == 0)
                {
                        for (int i = ranges[0]; i--; _mult_mat++)
                        {
                                T mul = *_mult_mat; 
                                for (int j = ranges[1]; j--; _src_mat ++)
                                        *_src_mat *= mul;
                        }
                }
                else
                {
                        for (int i = ranges[1]; i--; _mult_mat++,_src_mat++)
                        {
                                T* __src_mat = _src_mat;
                                T mul = (*_mult_mat);
                                for (int j = ranges[0]; j--; __src_mat += ranges[1])
                                        *__src_mat *= mul;
                        }
                }
        }
        else
        {
                assert( num_dims_to_mul == 0);
                T mul = mult_mat[0];
                for (int i = src_size ; i-- ; _src_mat++)
                        *_src_mat *= mul;
        }

}


template< typename T >
CMatrix<T> *iCNumericDenseMatrix<T>::Marg2D( int const *dims_to_keep,
                                                       int num_dims_to_keep,
                                                       EMargType marg_type,
                                                       CNumericDenseMatrix< T >*output,
                                                       EAccumType accum_type ) const
{
PNL_USING
    assert(m_Dim <= 2);
        const int* dest_ranges;
        T*  dest_mat;
        int safe_dst_bulk_size;
    int new_ranges[2];
        int src_size = m_Table.size();
    const int* ranges = &m_Range.front();
        const T *src_mat = &m_Table.front();
        assert( (unsigned)num_dims_to_keep < 3);
    int dest_size = 1;
        
        if (num_dims_to_keep == 1)
        {
                dest_size = new_ranges[0] =  ranges[dims_to_keep[0]];
        }
        else if (num_dims_to_keep == 2)
        {
                dest_size = src_size;
                new_ranges[0] = ranges[dims_to_keep[0]];
                new_ranges[1] = ranges[dims_to_keep[1]];
        }
    if ( output )
    {
                int safe_dst_num_dims;
        output->GetRanges( &safe_dst_num_dims, &dest_ranges );
        output->GetRawData( &safe_dst_bulk_size, (T const **)&dest_mat );
                assert(safe_dst_bulk_size <= m_Table.size() );
                assert(dest_size == safe_dst_bulk_size);
                assert(num_dims_to_keep == safe_dst_num_dims);
    }
    else
    {
                int safe_dst_size= -1;
        output = CNumericDenseMatrix< T >::Create( num_dims_to_keep, new_ranges, PNL_FAKEPTR( T ) );
        output->GetRawData( &safe_dst_size, (T const **)&dest_mat );
                assert(safe_dst_size == dest_size);
                assert(accum_type == PNL_ACCUM_TYPE_STORE);
    }
        const T* _src_mat = src_mat;
        T* _dest_mat = dest_mat;
                                
//// copy
        if ( num_dims_to_keep == int(m_Range.size()) && accum_type == PNL_ACCUM_TYPE_STORE )
        {
                if (dims_to_keep[0] == 0)
                {
                        memcpy(dest_mat, src_mat, src_size * sizeof(T) );
                }
                else
                {
                        for (int i = ranges[1]; i--; _src_mat++)
                        {
                                const T* __src_mat = _src_mat; 
                                for (int j = ranges[0]; j--; _dest_mat ++, __src_mat += ranges[1])
                                        *_dest_mat = *__src_mat;
                        }
                }
            return output;
        }
    switch( marg_type )
    {
                case PNL_MARG_TYPE_SUM:
                {
                        assert ( accum_type == PNL_ACCUM_TYPE_SUM ||
                                         accum_type == PNL_ACCUM_TYPE_ADD ||
                                         accum_type == PNL_ACCUM_TYPE_ACCUMULATE ||
                                         accum_type == PNL_ACCUM_TYPE_STORE );
                        
                        if ( num_dims_to_keep == int(m_Range.size()) )
                        {
                                if (dims_to_keep[0] == 1)
                                {
                                        for (int i = dest_size; i--; _dest_mat++, _src_mat++)
                                                *_dest_mat += *_src_mat;
                                }
                                else
                                {
                                        for (int i = ranges[0]; i--; _src_mat++)
                                        {
                                                const T* __src_mat = _src_mat; 
                                                for (int j = ranges[1]; j--; _dest_mat ++, __src_mat += ranges[0])
                                                        *_dest_mat += *__src_mat;
                                        }
                                }
                        }
                        else if ( num_dims_to_keep == 1 )
                        {
                                if (accum_type == PNL_ACCUM_TYPE_STORE)
                                        memset(dest_mat, 0, dest_size * sizeof (T));
                                if (dims_to_keep[0] == 1)
                                {
                                        for (int i = ranges[1]; i--; _dest_mat++, _src_mat++)
                                        {
                                                const T* __src_mat = _src_mat; 
                                                T sum = 0;
                                                for (int j = ranges[0]; j--; __src_mat += ranges[1])
                                                        sum += *__src_mat;
                                                *_dest_mat += sum;
                                        }
                                }
                                else
                                {
                                        for (int i = ranges[0]; i--; _dest_mat++)
                                        {
                                                T sum = 0;
                                                for (int j = ranges[1]; j--; _src_mat ++)
                                                         sum += *_src_mat;
                                                *_dest_mat += sum;
                                        }
                                }
                        }
                        else
                        {
                                assert( num_dims_to_keep == 0);
                                if (accum_type == PNL_ACCUM_TYPE_STORE)
                                        dest_mat[0] = 0;
                                T sum = 0;
                                for (int i = src_size ; i-- ; _src_mat++)
                                        sum += *_src_mat;
                                dest_mat[0] += sum;
                        }
                }
        break;
                case PNL_MARG_TYPE_MAX:
                {
                        assert ( accum_type == PNL_ACCUM_TYPE_ACCUMULATE ||
                                         accum_type == PNL_ACCUM_TYPE_MAX ||
                                         accum_type == PNL_ACCUM_TYPE_STORE );
                
                        if ( num_dims_to_keep == int(m_Range.size()))
                        {
                                if (dims_to_keep[0] == 1)
                                {
                                        for (int i = dest_size; i--; _dest_mat++, _src_mat++)
                                                *_dest_mat = PNL_MAX( *_src_mat, *_dest_mat);
                                }
                                else
                                {
                                        for (int i = ranges[0]; i--; _src_mat++)
                                        {
                                                const T* __src_mat = _src_mat; 
                                                for (int j = ranges[1]; j--; _dest_mat ++, __src_mat += ranges[0])
                                                        *_dest_mat = PNL_MAX( *__src_mat, *_dest_mat);
        
                                        }
                                }
                        }
                        else if ( num_dims_to_keep == 1 )
                        {
                                if (dims_to_keep[0] == 1)
                                {
                                        for (int i = ranges[1]; i--; _dest_mat++, _src_mat++)
                                        {
                                                const T* __src_mat = _src_mat; 
                                                T max = (accum_type == PNL_ACCUM_TYPE_STORE) ?
                                                                -FLT_MAX : *_dest_mat;
                                                for (int j = ranges[0]; j--; __src_mat += ranges[1])
                                                        max = PNL_MAX( *__src_mat, max);
                                                *_dest_mat = max;
                                        }
                                }
                                else
                                {
                                        for (int i = ranges[0]; i--; _dest_mat++)
                                        {
                                                T max = (accum_type == PNL_ACCUM_TYPE_STORE) ?
                                                        -FLT_MAX : *_dest_mat;
                                
                                                for (int j = ranges[1]; j--; _src_mat ++)
                                                        max     = PNL_MAX( *_src_mat , max);
                                                *_dest_mat = max;
                                        }
                                }
                        }
                        else
                        {
                                assert( num_dims_to_keep == 0);
                                T max = (accum_type == PNL_ACCUM_TYPE_STORE) ?
                                                -FLT_MAX : dest_mat[0];
                                for (int i = src_size ; i-- ; _src_mat++)
                                        max = PNL_MAX(max, *_src_mat);
                                dest_mat[0] = max;//__max ( max, dest_mat[0] );
                        }
                }
                break;
                default:
                        PNL_THROW( CInvalidOperation, "unsupported marg type" );
    }

  
    return output;
}

template< typename T >
CMatrix<T> *iCNumericDenseMatrix<T>::Marg_plain( int const *dims_to_keep,
                                                           int num_dims_to_keep,
                                                           EMargType marg_type,
                                                           CNumericDenseMatrix< T > *output,
                                                           EAccumType accum_type ) const
{
    PNL_USING

    int i, j, k;
    int src_num_dims;
    int const *src_ranges;
    T const *src_bulk;
    int src_bulk_size, safe_bulk_size;
    T *dst_bulk;
    int dst_bulk_size, safe_dst_bulk_size;
    int safe_dst_num_dims;
    int const *safe_dst_ranges;
    int block_size, trail_block_size;
    int num_blocked_dims, num_blocked_trail_dims;
    int step;
    T inival;
    bool need_ini;

    PNL_DEFINE_AUTOSTACK( int, dims_to_margout, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOSTACK( int, dims_to_trail, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOSTACK( int, final_dims_to_trail, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, src_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, src_back_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, dst_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, dst_back_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, corr, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, dst_ranges, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, corr_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, corr_back_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, block_offsets, PNL_MARG_MAX_BLOCK1_SIZE );
    PNL_DEFINE_AUTOBUF( int, src_trail_block_offsets, PNL_MARG_MAX_BLOCK2_SIZE );
    PNL_DEFINE_AUTOBUF( int, dst_trail_block_offsets, PNL_MARG_MAX_BLOCK2_SIZE );
    PNL_DEFINE_AUTOBUF( bool, mask, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );

    GetRanges( &src_num_dims, &src_ranges );
    GetRawData( &safe_bulk_size, &src_bulk );

    switch ( marg_type )
    {
    case PNL_MARG_TYPE_SUM:
        inival = 0;
        break;
    case PNL_MARG_TYPE_MAX:
        inival = (T)-FLT_MAX;
        break;
    default:
        PNL_THROW( CInvalidOperation, "unsupported marg type" );
    }
    if ( accum_type != PNL_ACCUM_TYPE_STORE )
    {
        inival = src_bulk[0];
    }

    PNL_DEMAND_AUTOBUF( dst_ranges, num_dims_to_keep );
    for ( i = num_dims_to_keep, dst_bulk_size = 1; i--; )
    {
        dst_bulk_size *= dst_ranges[i] = src_ranges[dims_to_keep[i]];
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
        for ( i = num_dims_to_keep; i--; )
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
        output = CNumericDenseMatrix< T >::Create( num_dims_to_keep, dst_ranges, PNL_FAKEPTR( T ) );
        output->GetRawData( &safe_dst_bulk_size, (T const **)&dst_bulk );
    }

    PNL_DEMAND_AUTOBUF( corr, src_num_dims );
    for ( i = src_num_dims; i--; )
    {
        corr[i] = -1;
    }
    for ( i = num_dims_to_keep; i--; )
    {
        if ( corr[dims_to_keep[i]] >= 0 )
        {
            PNL_THROW( CInvalidOperation, "Marg: dims_to_keep clash" );
        }
        corr[dims_to_keep[i]] = i;
    }

    PNL_DEMAND_AUTOBUF( src_steps, src_num_dims );
    PNL_DEMAND_AUTOBUF( src_back_steps, src_num_dims );

    PNL_MARG_INIT_STEPS( T, src_ranges, src_num_dims, src_steps, 
			 src_back_steps );

    src_bulk_size = src_back_steps[0] / sizeof( T );
    if ( safe_bulk_size < src_bulk_size )
    {
        PNL_THROW( CInconsistentSize, "Marg: broken raw data size or something" );
    }

    PNL_DEMAND_AUTOSTACK( dims_to_margout, src_num_dims );
    PNL_DEMAND_AUTOSTACK( dims_to_trail, src_num_dims );
    PNL_DEMAND_AUTOSTACK( final_dims_to_trail, src_num_dims );
    PNL_DEMAND_AUTOBUF( dst_steps, num_dims_to_keep );
    PNL_DEMAND_AUTOBUF( dst_back_steps, num_dims_to_keep );
    PNL_DEMAND_AUTOBUF( corr_steps, src_num_dims );
    PNL_DEMAND_AUTOBUF( corr_back_steps, src_num_dims );
    PNL_DEMAND_AUTOBUF( mask, src_num_dims );

    for ( i = src_num_dims; i--; )
    {
        if ( src_ranges[i] > 1 )
        {
            if ( corr[i] < 0 )
            {
                PNL_AUTOSTACK_PUSH( dims_to_margout, i );
            }
            else
            {
                PNL_AUTOSTACK_PUSH( dims_to_trail, i );
            }
        }
    }

    PNL_MARG_INIT_STEPS( T, dst_ranges, num_dims_to_keep, dst_steps, 
			 dst_back_steps );

#if 1
    num_blocked_dims = margBlockize( dims_to_margout, PNL_AUTOSTACK_SIZE( dims_to_margout ), src_ranges, block_size,
                       PNL_MARG_DESIRED_BLOCK1_SIZE, PNL_MARG_MAX_BLOCK1_SIZE );
#else
    PNL_MARG_BLOCKIZE( dims_to_margout, PNL_AUTOSTACK_SIZE( dims_to_margout ), src_ranges, num_blocked_dims, block_size,
                       PNL_MARG_DESIRED_BLOCK1_SIZE, PNL_MARG_MAX_BLOCK1_SIZE );
#endif

    PNL_DEMAND_AUTOBUF( block_offsets, block_size );

    PNL_MARG_INIT_OFFSETS( dims_to_margout, src_ranges, num_blocked_dims, block_size,
                           block_offsets, src_steps, src_back_steps );

    for ( i = src_num_dims; i--; )
    {
        mask[i] = false;
    }
    for ( i = num_blocked_dims; i < PNL_AUTOSTACK_SIZE( dims_to_margout ); ++i )
    {
        mask[dims_to_margout[i]] = true;
    }
    for ( i = PNL_AUTOSTACK_SIZE( dims_to_trail ); i--; )
    {
        mask[dims_to_trail[i]] = true;
    }
    for ( i = src_num_dims; i--; )
    {
        if ( mask[i] )
        {
            PNL_AUTOSTACK_PUSH( final_dims_to_trail, i );
        }
    }

    if ( PNL_IS_AUTOSTACK_EMPTY( final_dims_to_trail ) )
    {
        T op = inival;
        switch ( marg_type )
        {
        case PNL_MARG_TYPE_SUM:
            for ( i = src_bulk_size; i--; )
            {
                PNL_ADD( op, src_bulk[i] );
            }
            dst_bulk[0] = op;
            break;
        case PNL_MARG_TYPE_MAX:
            for ( i = src_bulk_size; i--; )
            {
                PNL_MARG_MAX( op, src_bulk[i] );
            }
            dst_bulk[0] = op;
        }
        goto endup;
    }

#if 1
    num_blocked_trail_dims = margBlockize( final_dims_to_trail,
        PNL_AUTOSTACK_SIZE( final_dims_to_trail ), src_ranges, trail_block_size,
        PNL_MARG_DESIRED_BLOCK2_SIZE, PNL_MARG_MAX_BLOCK2_SIZE );
#else
    PNL_MARG_BLOCKIZE( final_dims_to_trail, PNL_AUTOSTACK_SIZE( final_dims_to_trail ), src_ranges, num_blocked_trail_dims,
                       trail_block_size, PNL_MARG_DESIRED_BLOCK2_SIZE, PNL_MARG_MAX_BLOCK2_SIZE );
#endif

    PNL_DEMAND_AUTOBUF( src_trail_block_offsets, trail_block_size );
    PNL_DEMAND_AUTOBUF( dst_trail_block_offsets, trail_block_size );

    PNL_MARG_INIT_OFFSETS( final_dims_to_trail, src_ranges, num_blocked_trail_dims, trail_block_size,
                           src_trail_block_offsets, src_steps, src_back_steps );

    for ( i = src_num_dims; i--; )
    {
        corr_steps[i] = corr_back_steps[i] = 0;
    }
    for ( i = PNL_AUTOSTACK_SIZE( dims_to_trail ); i--; )
    {
        corr_steps[dims_to_trail[i]] = dst_steps[corr[dims_to_trail[i]]];
        corr_back_steps[dims_to_trail[i]] = dst_back_steps[corr[dims_to_trail[i]]];
    }

    PNL_MARG_INIT_OFFSETS( final_dims_to_trail, src_ranges, num_blocked_trail_dims, trail_block_size,
                           dst_trail_block_offsets, corr_steps, corr_back_steps );

    if ( num_blocked_dims < PNL_AUTOSTACK_SIZE( dims_to_margout ) )
    {
        if ( accum_type == PNL_ACCUM_TYPE_STORE )
        {
            for ( int i = 0; i < dst_bulk_size; ++i )
            {
                dst_bulk[i] = inival;
            }
        }
        need_ini = false;
    }
    else
    {
        need_ini = true;
    }

    PNL_MARG_SPECMPLEXOR( src, dst );

 endup:
    PNL_RELEASE_AUTOBUF( dst_ranges );
    PNL_RELEASE_AUTOBUF( mask );
    PNL_RELEASE_AUTOBUF( dst_trail_block_offsets );
    PNL_RELEASE_AUTOBUF( src_trail_block_offsets );
    PNL_RELEASE_AUTOBUF( block_offsets );
    PNL_RELEASE_AUTOBUF( corr_back_steps );
    PNL_RELEASE_AUTOBUF( corr_steps );
    PNL_RELEASE_AUTOBUF( corr );
    PNL_RELEASE_AUTOBUF( dst_back_steps );
    PNL_RELEASE_AUTOBUF( dst_steps );
    PNL_RELEASE_AUTOBUF( src_back_steps );
    PNL_RELEASE_AUTOBUF( src_steps );
    PNL_RELEASE_AUTOSTACK( final_dims_to_trail );
    PNL_RELEASE_AUTOSTACK( dims_to_trail );
    PNL_RELEASE_AUTOSTACK( dims_to_margout );

    return output;
}

#define PNL_MARG_DESIRED_PARBLOCK_SIZE ((omp_get_num_procs() - 1) * 3 + 1)
#define PNL_MARG_MAX_PARBLOCK_SIZE (omp_get_num_procs() * 30)

template< typename T >
CMatrix<T> *iCNumericDenseMatrix<T>::Marg_omp( int const *dims_to_keep,
                                                         int num_dims_to_keep,
                                                         EMargType marg_type,
                                                         CNumericDenseMatrix< T > *output,
                                                         EAccumType accum_type ) const
{
#if defined(_OPENMP) && !defined(PAR_OMP)
    PNL_USING

    int i, j, k;
    int src_num_dims;
    int const *src_ranges;
    T const *src_bulk;
    int src_bulk_size, safe_bulk_size;
    T *dst_bulk;
    int dst_bulk_size, safe_dst_bulk_size;
    int safe_dst_num_dims;
    int const *safe_dst_ranges;
    int block_size, trail_block_size;
    int num_blocked_dims, num_blocked_trail_dims;
    T const *src_base, *src_base2;
    T *dst_base, *dst_base2;
    int step;
    int num_par_dims, par_block_size;
    int false_sharing_lim;
    T inival;
    bool need_ini;

    PNL_DEFINE_AUTOSTACK( int, dims_to_margout, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOSTACK( int, dims_to_trail, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOSTACK( int, final_dims_to_trail, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, src_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, src_back_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, dst_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, dst_back_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, corr, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, dst_ranges, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, corr_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, corr_back_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, block_offsets, PNL_MARG_MAX_BLOCK1_SIZE );
    PNL_DEFINE_AUTOBUF( int, src_trail_block_offsets, PNL_MARG_MAX_BLOCK2_SIZE );
    PNL_DEFINE_AUTOBUF( int, dst_trail_block_offsets, PNL_MARG_MAX_BLOCK2_SIZE );
    PNL_DEFINE_AUTOBUF( bool, mask, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );

    GetRanges( &src_num_dims, &src_ranges );
    GetRawData( &safe_bulk_size, &src_bulk );

    switch ( marg_type )
    {
    case PNL_MARG_TYPE_SUM:
        inival = 0;
        break;
    case PNL_MARG_TYPE_MAX:
        inival = (T)-FLT_MAX;
        break;
    default:
        PNL_THROW( CInvalidOperation, "unsupported marg type" );
    }
    if ( accum_type != PNL_ACCUM_TYPE_STORE )
    {
        inival = src_bulk[0];
    }

    PNL_DEMAND_AUTOBUF( dst_ranges, num_dims_to_keep );
    for ( i = num_dims_to_keep, dst_bulk_size = 1; i--; )
    {
        dst_bulk_size *= dst_ranges[i] = src_ranges[dims_to_keep[i]];
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
        for ( i = num_dims_to_keep; i--; )
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
        output = CNumericDenseMatrix< T >::Create( num_dims_to_keep, dst_ranges, PNL_FAKEPTR( T ) );
        output->GetRawData( &safe_dst_bulk_size, (T const **)&dst_bulk );
    }

    PNL_DEMAND_AUTOBUF( corr, src_num_dims );
    for ( i = src_num_dims; i--; )
    {
        corr[i] = -1;
    }
    for ( i = num_dims_to_keep; i--; )
    {
        if ( corr[dims_to_keep[i]] >= 0 )
        {
            PNL_THROW( CInvalidOperation, "Marg: dims_to_keep clash" );
        }
        corr[dims_to_keep[i]] = i;
    }

    PNL_DEMAND_AUTOBUF( src_steps, src_num_dims );
    PNL_DEMAND_AUTOBUF( src_back_steps, src_num_dims );

    PNL_MARG_INIT_STEPS( T, src_ranges, src_num_dims, src_steps, 
			 src_back_steps );

    src_bulk_size = src_back_steps[0] / sizeof( T );
    if ( safe_bulk_size < src_bulk_size )
    {
        PNL_THROW( CInconsistentSize, "Marg: broken raw data size or something" );
    }

    PNL_DEMAND_AUTOSTACK( dims_to_margout, src_num_dims );
    PNL_DEMAND_AUTOSTACK( dims_to_trail, src_num_dims );
    PNL_DEMAND_AUTOSTACK( final_dims_to_trail, src_num_dims );
    PNL_DEMAND_AUTOBUF( dst_steps, num_dims_to_keep );
    PNL_DEMAND_AUTOBUF( dst_back_steps, num_dims_to_keep );
    PNL_DEMAND_AUTOBUF( corr_steps, src_num_dims );
    PNL_DEMAND_AUTOBUF( corr_back_steps, src_num_dims );
    PNL_DEMAND_AUTOBUF( mask, src_num_dims );

    if ( src_bulk_size > 200 )
    {
        PNL_MARG_BLOCKIZE_REV( dims_to_keep, num_dims_to_keep, src_ranges, false_sharing_lim, par_block_size,
                               PNL_BIG_CACHELINE_BYTES, 32768 );
        false_sharing_lim = num_dims_to_keep - false_sharing_lim;

#if 1
        num_par_dims = margBlockize( dims_to_keep, false_sharing_lim, src_ranges,
            par_block_size, PNL_MARG_DESIRED_PARBLOCK_SIZE, PNL_MARG_MAX_PARBLOCK_SIZE);
#else
        PNL_MARG_BLOCKIZE( dims_to_keep, false_sharing_lim, src_ranges, num_par_dims, par_block_size,
                           PNL_MARG_DESIRED_PARBLOCK_SIZE, PNL_MARG_MAX_PARBLOCK_SIZE );
#endif
    }
    else
    {
        // going sequential
        num_par_dims = 0;
        par_block_size = 1;
    }

    for ( i = num_par_dims; i--; )
    {
        corr[dims_to_keep[i]] = -2;
    }

    for ( i = src_num_dims; i--; )
    {
        if ( src_ranges[i] > 1 )
        {
            if ( corr[i] == -1 )
            {
                PNL_AUTOSTACK_PUSH( dims_to_margout, i );
            }
            else if ( corr[i] >= 0 )
            {
                PNL_AUTOSTACK_PUSH( dims_to_trail, i );
            }
        }
    }

    for ( i = num_par_dims; i--; )
    {
        corr[dims_to_keep[i]] = i;
    }

    PNL_MARG_INIT_STEPS( T, dst_ranges, num_dims_to_keep, dst_steps, 
			 dst_back_steps );

#if 1
    num_blocked_dims = margBlockize(dims_to_margout, PNL_AUTOSTACK_SIZE( dims_to_margout ), src_ranges, block_size,
                       PNL_MARG_DESIRED_BLOCK1_SIZE, PNL_MARG_MAX_BLOCK1_SIZE );
#else
    PNL_MARG_BLOCKIZE( dims_to_margout, PNL_AUTOSTACK_SIZE( dims_to_margout ), src_ranges, block_size,
                       PNL_MARG_DESIRED_BLOCK1_SIZE, PNL_MARG_MAX_BLOCK1_SIZE );
#endif

    PNL_DEMAND_AUTOBUF( block_offsets, block_size );

    PNL_MARG_INIT_OFFSETS( dims_to_margout, src_ranges, num_blocked_dims, block_size,
                           block_offsets, src_steps, src_back_steps );

    for ( i = src_num_dims; i--; )
    {
        mask[i] = false;
    }
    for ( i = num_blocked_dims; i < PNL_AUTOSTACK_SIZE( dims_to_margout ); ++i )
    {
        mask[dims_to_margout[i]] = true;
    }
    for ( i = 0; i < PNL_AUTOSTACK_SIZE( dims_to_trail ); ++i )
    {
        mask[dims_to_trail[i]] = true;
    }
    for ( i = src_num_dims; i--; )
    {
        if ( mask[i] )
        {
            PNL_AUTOSTACK_PUSH( final_dims_to_trail, i );
        }
    }

    if ( PNL_IS_AUTOSTACK_EMPTY( final_dims_to_trail ) && num_par_dims == 0 )
    {
        T op = inival;
        switch ( marg_type )
        {
        case PNL_MARG_TYPE_SUM:
            for ( i = src_bulk_size; i--; )
            {
                PNL_ADD( op, src_bulk[i] );
            }
            dst_bulk[0] = op;
            break;
        case PNL_MARG_TYPE_MAX:
            for ( i = src_bulk_size; i--; )
            {
                PNL_MARG_MAX( op, src_bulk[i] );
            }
            dst_bulk[0] = op;
        }
        goto endup;
    }

#if 1
    num_blocked_trail_dims = margBlockize( final_dims_to_trail, PNL_AUTOSTACK_SIZE( final_dims_to_trail ), src_ranges,
        trail_block_size, PNL_MARG_DESIRED_BLOCK2_SIZE, PNL_MARG_MAX_BLOCK2_SIZE );
#else
    PNL_MARG_BLOCKIZE( final_dims_to_trail, PNL_AUTOSTACK_SIZE( final_dims_to_trail ), src_ranges, num_blocked_trail_dims,
                       trail_block_size, PNL_MARG_DESIRED_BLOCK2_SIZE, PNL_MARG_MAX_BLOCK2_SIZE );
#endif

    PNL_DEMAND_AUTOBUF( src_trail_block_offsets, trail_block_size );
    PNL_DEMAND_AUTOBUF( dst_trail_block_offsets, trail_block_size );

    PNL_MARG_INIT_OFFSETS( final_dims_to_trail, src_ranges, num_blocked_trail_dims, trail_block_size,
                           src_trail_block_offsets, src_steps, src_back_steps );

    for ( i = src_num_dims; i--; )
    {
        corr_steps[i] = corr_back_steps[i] = 0;
    }
    for ( i = PNL_AUTOSTACK_SIZE( dims_to_trail ); i--; )
    {
        corr_steps[dims_to_trail[i]] = dst_steps[corr[dims_to_trail[i]]];
        corr_back_steps[dims_to_trail[i]] = dst_back_steps[corr[dims_to_trail[i]]];
    }
    for ( i = num_par_dims; i--; )
    {
        corr_steps[dims_to_keep[i]] = dst_steps[corr[dims_to_keep[i]]];
        corr_back_steps[dims_to_keep[i]] = dst_back_steps[corr[dims_to_keep[i]]];
    }

    PNL_MARG_INIT_OFFSETS( final_dims_to_trail, src_ranges, num_blocked_trail_dims, trail_block_size,
                           dst_trail_block_offsets, corr_steps, corr_back_steps );

    if ( num_blocked_dims < PNL_AUTOSTACK_SIZE( dims_to_margout ) )
    {
        if ( accum_type == PNL_ACCUM_TYPE_STORE )
        {
            for ( int i = 0; i < dst_bulk_size; ++i )
            {
                dst_bulk[i] = inival;
            }
        }
        need_ini = false;
    }
    else
    {
        need_ini = true;
    }

    #pragma omp parallel for schedule( guided, 1 ), private( i, j, k )
    for ( int channel = 0; channel < par_block_size; ++channel )
    {
        PNL_MARG_SPECMPLEXOR_OMP( src, dst, dims_to_keep, num_par_dims );
    }

 endup:
    PNL_RELEASE_AUTOBUF( dst_ranges );
    PNL_RELEASE_AUTOBUF( mask );
    PNL_RELEASE_AUTOBUF( dst_trail_block_offsets );
    PNL_RELEASE_AUTOBUF( src_trail_block_offsets );
    PNL_RELEASE_AUTOBUF( block_offsets );
    PNL_RELEASE_AUTOBUF( corr_back_steps );
    PNL_RELEASE_AUTOBUF( corr_steps );
    PNL_RELEASE_AUTOBUF( corr );
    PNL_RELEASE_AUTOBUF( dst_back_steps );
    PNL_RELEASE_AUTOBUF( dst_steps );
    PNL_RELEASE_AUTOBUF( src_back_steps );
    PNL_RELEASE_AUTOBUF( src_steps );
    PNL_RELEASE_AUTOSTACK( final_dims_to_trail );
    PNL_RELEASE_AUTOSTACK( dims_to_trail );
    PNL_RELEASE_AUTOSTACK( dims_to_margout );

    return output;
#else
    return 0;
#endif
}

template< typename T >
CMatrix<T> *iCNumericDenseMatrix<T>::Marg( int const *dims_to_keep,
                                                     int num_dims_to_keep,
                                                     EMargType marg_type,
                                                     CNumericDenseMatrix< T > *output,
                                                     EAccumType accum_type ) const
{
#if defined(_OPENMP) && !defined(PAR_OMP)
    return Marg_omp( dims_to_keep, num_dims_to_keep, marg_type, output, accum_type );
#else
    return Marg_plain( dims_to_keep, num_dims_to_keep, marg_type, output, accum_type );
#endif
}

template< typename T >
CMatrix<T> *iCNumericDenseMatrix<T>::FixDims( int const *dims_to_fix,
                                                        int num_dims_to_fix,
                                                        int const *observed_values,
                                                        CNumericDenseMatrix< T > *output,
                                                        EAccumType accum_type ) const
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
        output = CNumericDenseMatrix< T >::Create( num_dims, dst_ranges, PNL_FAKEPTR( T ) );
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

#if 1
    num_blocked_dims =
        margBlockize(dims_to_trail, PNL_AUTOSTACK_SIZE( dims_to_trail ),
        src_ranges, block_size, PNL_FIXDIMS_DESIRED_BLOCK_SIZE, PNL_FIXDIMS_MAX_BLOCK_SIZE);
#else
    PNL_MARG_BLOCKIZE( dims_to_trail, PNL_AUTOSTACK_SIZE( dims_to_trail ), src_ranges, num_blocked_dims, block_size,
                       PNL_FIXDIMS_DESIRED_BLOCK_SIZE, PNL_FIXDIMS_MAX_BLOCK_SIZE );
#endif

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
    else if ( accum_type == PNL_ACCUM_TYPE_MUL )
    {
loop_mul:
        for ( j = 0; j < block_size; )
        {
            *dst_base++ *= *(T *)((char *)src_base + block_offsets[j++]);
        }
        for ( k = num_blocked_dims; k < PNL_AUTOSTACK_SIZE( dims_to_trail ); ++k )
        {
            j = dims_to_trail[k];
            src_base = (T *)((char *)src_base + src_steps[j]);
            if ( ++idx[k] < src_ranges[j] )
            {
                goto loop_mul;
            }
            idx[k] = 0;
            src_base = (T *)((char *)src_base - src_back_steps[j]);
        }
    }
    else
    {
        PNL_THROW( CInvalidOperation, "unsupported accumulation type" );
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

template< typename T >
CMatrix<T> *iCNumericDenseMatrix<T>::ReduceOp( int const *dims_of_interest,
                                               int num_dims_of_interest, int action,
                                               int const *observed_values,
                                               CMatrix< T > *output,
                                               EAccumType accum_type ) const
{
    PNL_USING
    switch ( action )
    {
    case 0:
                if (m_Dim <= 2)
                {
                        return Marg2D( dims_of_interest, num_dims_of_interest, PNL_MARG_TYPE_SUM,
                   (CNumericDenseMatrix< T > *)output, accum_type );
                }
                else
                {
                        return Marg( dims_of_interest, num_dims_of_interest, PNL_MARG_TYPE_SUM,
                   (CNumericDenseMatrix< T > *)output, accum_type );
                }
    case 1:
                if (m_Dim <= 2)
                {
                        return Marg2D( dims_of_interest, num_dims_of_interest, PNL_MARG_TYPE_MAX,
                     (CNumericDenseMatrix< T > *)output, accum_type );
                }
                else
                {
                        return Marg( dims_of_interest, num_dims_of_interest, PNL_MARG_TYPE_MAX,
                     (CNumericDenseMatrix< T > *)output, accum_type );
                }

    case 2:
        return FixDims( dims_of_interest, num_dims_of_interest, observed_values,
                        (CNumericDenseMatrix< T > *)output, accum_type );
    }
    PNL_THROW( CInconsistentType, "ReduceOp: action not supported" );
}

#define PNL_MULTIDMATRIX_MAX_DIM 64

#define PNL_REDUCEOP_ADD( dst, src ) dst += src
#define PNL_REDUCEOP_MAX( dst, src ) if ( src > dst ) dst = src
#define PNL_REDUCEOP_CPY( dst, src ) dst = src

#define PNL_REDUCEOP_LOOP_STEP( k ) \
    ++stats[k];                     \
    j += steps[k];

#define PNL_REDUCEOP_LOOP_CHECK( RANGES, NUM_STATS )  \
    for ( k = NUM_STATS - 1; k >= 0 && stats[k] == RANGES[k]; ) \
    {                                                 \
        stats[k] = 0;                                 \
        j -= backsteps[k--];                          \
        PNL_REDUCEOP_LOOP_STEP( k );                  \
    }

#define PNL_REDUCEOP_LOOP( ACTION, INITVAL )              \
    for ( i = new_bulk_size; i--; )                       \
    {                                                     \
        new_bulk[i] = INITVAL;                            \
    }                                                     \
    for ( i = 0, j = 0; i < bulk_size; ++i )              \
    {                                                     \
        PNL_REDUCEOP_LOOP_CHECK( ranges, num_dims );      \
        ACTION( new_bulk[j], bulk[i] );                   \
        PNL_REDUCEOP_LOOP_STEP( num_dims - 1 );           \
    }

#define PNL_REDUCEOP_LOOP2( ACTION, PIVOT )                      \
    for ( i = 0, j = PIVOT; i < new_bulk_size; ++i )             \
    {                                                            \
        PNL_REDUCEOP_LOOP_CHECK( new_ranges, num_dims_to_keep ); \
        ACTION( new_bulk[i], bulk[j] );                          \
        PNL_REDUCEOP_LOOP_STEP( num_dims_to_keep - 1 );          \
    }


#pragma optimize( "", off )

template <class Type>
CMatrix<Type>* iCNumericDenseMatrix<Type>::OldReduceOp( const int *pDimsOfInterest,
                                                                  int numDimsOfInterest,
                                                                  int action,
                                                                  const int *pObservedValues) const
{
    PNL_USING

    int i, j, k;
    int num_dims;
    const int* ranges;
    const Type* bulk;
    int *new_ranges;
    int new_bulk_size, safe_bulk_size, bulk_size;
    Type* new_bulk;
    int num_dims_to_keep;
    const int *dims_to_keep;
    int stats[PNL_MULTIDMATRIX_MAX_DIM];
    int steps[PNL_MULTIDMATRIX_MAX_DIM];
    int backsteps[PNL_MULTIDMATRIX_MAX_DIM];
    int buf[PNL_MULTIDMATRIX_MAX_DIM];
    bool mask[PNL_MULTIDMATRIX_MAX_DIM];

    if ( (unsigned)action > 2 )
    {
        PNL_THROW( CInconsistentType, "ReduceOp: action not supported" );
    }
    static_cast<const CDenseMatrix<Type>*>(this)->GetRanges( &num_dims, &ranges );
    static_cast<const CDenseMatrix<Type>*>(this)->GetRawData( &safe_bulk_size, &bulk );
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
        if ( (unsigned)pDimsOfInterest[i] >= num_dims )
        {
            PNL_THROW( CInconsistentSize, "ReduceOp: broken pDimsOfInterest" );
        }
        if ( mask[pDimsOfInterest[i]] )
        {
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
                PNL_THROW( CInconsistentSize, "ReduceOp: pObservedValues cranky" );
            }
        }
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
    if ( action < 2 )
    {
        dims_to_keep = pDimsOfInterest;
        num_dims_to_keep = numDimsOfInterest;
        j = num_dims;
    }
    else
    {
        for ( i = 0, j = 0; i < num_dims; ++i )
        {
            if ( !mask[i] )
            {
                buf[j++] = i;
            }
        }
        dims_to_keep = buf;
        num_dims_to_keep = j;
    }
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
    if ( action < 2 )
    {
        for ( i = num_dims; i--; )
        {
            steps[i] = 0;
        }
        for ( i = numDimsOfInterest, j = 1; i--; )
        {
            steps[pDimsOfInterest[i]] = j;
            j *= ranges[pDimsOfInterest[i]];
        }
        for ( i = num_dims; i--; )
        {
            backsteps[i] = steps[i] * ranges[i];
        }
        if ( action == 0 )
        {
            PNL_REDUCEOP_LOOP( PNL_REDUCEOP_ADD, 0.f );
        }
        else
        {
            PNL_REDUCEOP_LOOP( PNL_REDUCEOP_MAX, -FLT_MAX );
        }
    }
    else
    {
        int pivot;
        for ( i = num_dims, j = 1; i--; j *= ranges[i] )
        {
            steps[i] = j;
        }
        for ( i = numDimsOfInterest, pivot = 0; i--; )
        {
            pivot += steps[pDimsOfInterest[i]] * pObservedValues[i];
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
    }
    CNumericDenseMatrix<Type> *retval = CNumericDenseMatrix<Type>::Create( num_dims_to_keep, new_ranges, new_bulk );
    delete[]( new_bulk );
    delete[]( new_ranges );

    return retval;
}
#undef PNL_REDUCEOP_LOOP2
#undef PNL_REDUCEOP_LOOP
#undef PNL_REDUCEOP_CPY
#undef PNL_REDUCEOP_MAX
#undef PNL_REDUCEOP_ADD

#pragma optimize( "", on )

template <class Type>
CMatrix<Type>* iCNumericDenseMatrix<Type>::NormalizeAll() const
{
    PNL_USING
        iCNumericDenseMatrix<Type> *retMatr = new iCNumericDenseMatrix<Type>( m_Dim,
        &m_Range.front(), &m_Table.front(), GetClampValue() );
    Type sum = SumAll();
    if( sum > FLT_MIN*10.f )
    {
        if( fabs(sum - 1) > 0.00001f )
        {
            Type reciprocalSum = 1/sum;
            typename pnlVector<Type>::iterator it = retMatr->m_Table.begin();
            typename pnlVector<Type>::iterator itE = retMatr->m_Table.end();
            for ( ; it != itE; it++ )
            {
                (*it) *= reciprocalSum;
            }
        }
    }
    else
    {
        sum = (Type)m_Table.size();
        Type reciprocalSum = 1/sum;
        typename pnlVector<Type>::iterator it = retMatr->m_Table.begin();
        typename pnlVector<Type>::iterator itE = retMatr->m_Table.end();
        for ( ; it != itE; it++ )
        {
            (*it) = reciprocalSum;
        }
    }
    return retMatr;
}

template <class Type>
void iCNumericDenseMatrix<Type>::Normalize() 
{
    PNL_USING
    Type sum = SumAll();
    if(( sum > (2*FLT_MIN) )&&( fabs(sum - 1)>= FLT_EPSILON ))
    {
        Type reciprocalSum = 1/sum;
        typename pnlVector<Type>::iterator it = m_Table.begin();
        typename pnlVector<Type>::iterator itE = m_Table.end();
        for ( ; it != itE; it++ )
        {
            (*it) *= reciprocalSum;
        }
    }
}


template <class Type>
Type iCNumericDenseMatrix<Type>::SumAll(int byAbsValue) const
{
    PNL_USING
        double sum = 0;
    typename pnlVector<Type>::const_iterator it = m_Table.begin();
    typename pnlVector<Type>::const_iterator itE = m_Table.end();
    if( byAbsValue )
    {
        for( ; it != itE ; sum += fabs(*it), it++ );
    }
    else
    {
        for( ; it != itE ; sum += (*it), it++ );
    }
    return (Type)sum;
}

#define PNL_MULINSELF_DESIRED_BLOCK1_SIZE 3
#define PNL_MULINSELF_MAX_BLOCK1_SIZE 50
#define PNL_MULINSELF_DESIRED_BLOCK2_SIZE 70
#define PNL_MULINSELF_MAX_BLOCK2_SIZE 1000

template< typename  T >
void iCNumericDenseMatrix< T >::MultiplyInSelf_plain( CMatrix< T > const *mat,
                                                           int num_dims_to_mul,
                                                           int const *dims_to_mul,
                                                           int, T const )
{
    PNL_USING

    int i, j, k;
    int big_num_dims;
    int const *big_ranges;
    T *big_bulk;
    int big_bulk_size;
    /* I spell small as smal here and there because dumb MFC defines global small *
     * and we can encounter some problems with macros                             */
    int smal_num_dims;
    int const *smal_ranges;
    T const *smal_bulk;
    int smal_bulk_size;
    int block_size, trail_block_size, num_blocked_dims, num_blocked_trail_dims;
    int step;

    PNL_DEFINE_AUTOSTACK( int, final_dims_to_mul, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOSTACK( int, swamp_dims, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOSTACK( int, final_dims_to_trail, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, smal_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, smal_back_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, big_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, big_back_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, corr, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, corr_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, corr_back_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, block_offsets, PNL_MARG_MAX_BLOCK1_SIZE );
    PNL_DEFINE_AUTOBUF( int, big_trail_block_offsets, PNL_MARG_MAX_BLOCK2_SIZE );
    PNL_DEFINE_AUTOBUF( int, smal_trail_block_offsets, PNL_MARG_MAX_BLOCK2_SIZE );
    PNL_DEFINE_AUTOBUF( bool, mask, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );

    EMatrixClass other_class = mat->GetMatrixClass();
    if( other_class != mcNumericDense )
    {
        PNL_THROW( CInvalidOperation, "can't multiply dense in sparse" );
    }

    GetRanges( &big_num_dims, &big_ranges );
    ((iCNumericDenseMatrix< T > *)mat)->GetRanges( &smal_num_dims, &smal_ranges );
    if( smal_num_dims != num_dims_to_mul )
    {
        PNL_THROW( CInvalidOperation, "can multiply only by all dims of small matrix" );
    }
    GetRawData( &big_bulk_size, (T const **)&big_bulk );
    ((iCNumericDenseMatrix< T > *)mat)->GetRawData( &smal_bulk_size, &smal_bulk );

    PNL_DEMAND_AUTOBUF( corr, big_num_dims );
    for ( i = big_num_dims; i--; )
    {
        corr[i] = -1;
    }
    for ( i = num_dims_to_mul; i--; )
    {
        if ( corr[dims_to_mul[i]] >= 0 )
        {
            PNL_THROW( CInvalidOperation, "dims_to_mul clash" );
        }
        corr[dims_to_mul[i]] = i;
    }

    PNL_DEMAND_AUTOSTACK( final_dims_to_mul, smal_num_dims );
    PNL_DEMAND_AUTOSTACK( swamp_dims, big_num_dims );
    PNL_DEMAND_AUTOSTACK( final_dims_to_trail, big_num_dims );
    PNL_DEMAND_AUTOBUF( smal_steps, smal_num_dims );
    PNL_DEMAND_AUTOBUF( smal_back_steps, smal_num_dims );
    PNL_DEMAND_AUTOBUF( big_steps, big_num_dims );
    PNL_DEMAND_AUTOBUF( big_back_steps, big_num_dims );
    PNL_DEMAND_AUTOBUF( corr_steps, big_num_dims );
    PNL_DEMAND_AUTOBUF( corr_back_steps, big_num_dims );
    PNL_DEMAND_AUTOBUF( mask, big_num_dims );

    PNL_MARG_INIT_STEPS( T, big_ranges, big_num_dims, big_steps, 
			 big_back_steps );
    PNL_MARG_INIT_STEPS( T, smal_ranges, smal_num_dims, smal_steps, 
			 smal_back_steps );

    big_bulk_size = big_back_steps[0] / sizeof( T );

    for ( i = big_num_dims; i--; )
    {
        if ( big_ranges[i] > 1 )
        {
            if ( corr[i] < 0 )
            {
                PNL_AUTOSTACK_PUSH( swamp_dims, i );
            }
            else
            {
                PNL_AUTOSTACK_PUSH( final_dims_to_mul, i );
            }
        }
    }

#if 1
    num_blocked_dims = margBlockize(swamp_dims, PNL_AUTOSTACK_SIZE( swamp_dims ),
        big_ranges, block_size, PNL_MULINSELF_DESIRED_BLOCK1_SIZE, PNL_MULINSELF_MAX_BLOCK1_SIZE);
#else
    PNL_MARG_BLOCKIZE( swamp_dims, PNL_AUTOSTACK_SIZE( swamp_dims ), big_ranges, num_blocked_dims, block_size,
                       PNL_MULINSELF_DESIRED_BLOCK1_SIZE, PNL_MULINSELF_MAX_BLOCK1_SIZE );
#endif

    PNL_DEMAND_AUTOBUF( block_offsets, block_size );

    PNL_MARG_INIT_OFFSETS( swamp_dims, big_ranges, num_blocked_dims, block_size,
                           block_offsets, big_steps, big_back_steps );

    for ( i = big_num_dims; i--; )
    {
        mask[i] = false;
    }
    for ( i = num_blocked_dims; i < PNL_AUTOSTACK_SIZE( swamp_dims ); ++i )
    {
        mask[swamp_dims[i]] = true;
    }
    for ( i = PNL_AUTOSTACK_SIZE( final_dims_to_mul ); i--; )
    {
        mask[final_dims_to_mul[i]] = true;
    }
    for ( i = big_num_dims; i--; )
    {
        if ( mask[i] )
        {
            PNL_AUTOSTACK_PUSH( final_dims_to_trail, i );
        }
    }

#if 1
    num_blocked_trail_dims = margBlockize(final_dims_to_trail,
        PNL_AUTOSTACK_SIZE( final_dims_to_trail ), big_ranges, trail_block_size,
        PNL_MULINSELF_DESIRED_BLOCK2_SIZE, PNL_MULINSELF_MAX_BLOCK2_SIZE );
#else
    PNL_MARG_BLOCKIZE( final_dims_to_trail, PNL_AUTOSTACK_SIZE( final_dims_to_trail ), big_ranges, num_blocked_trail_dims,
                       trail_block_size, PNL_MULINSELF_DESIRED_BLOCK2_SIZE, PNL_MULINSELF_MAX_BLOCK2_SIZE );
#endif

    PNL_DEMAND_AUTOBUF( big_trail_block_offsets, trail_block_size );
    PNL_DEMAND_AUTOBUF( smal_trail_block_offsets, trail_block_size );

    PNL_MARG_INIT_OFFSETS( final_dims_to_trail, big_ranges, num_blocked_trail_dims, trail_block_size,
                           big_trail_block_offsets, big_steps, big_back_steps );

    for ( i = big_num_dims; i--; )
    {
        corr_steps[i] = corr_back_steps[i] = 0;
    }
    for ( i = PNL_AUTOSTACK_SIZE( final_dims_to_mul ); i--; )
    {
        corr_steps[final_dims_to_mul[i]] = smal_steps[corr[final_dims_to_mul[i]]];
        corr_back_steps[final_dims_to_mul[i]] = smal_back_steps[corr[final_dims_to_mul[i]]];
    }

    PNL_MARG_INIT_OFFSETS( final_dims_to_trail, big_ranges, num_blocked_trail_dims, trail_block_size,
                           smal_trail_block_offsets, corr_steps, corr_back_steps );

    PNL_MULINSELF_SPECMPLEXOR( big, smal, PNL_MARG_REVMUL, PNL_ASN, PNL_VOID2 );

    PNL_RELEASE_AUTOBUF( mask );
    PNL_RELEASE_AUTOBUF( smal_trail_block_offsets );
    PNL_RELEASE_AUTOBUF( big_trail_block_offsets );
    PNL_RELEASE_AUTOBUF( block_offsets );
    PNL_RELEASE_AUTOBUF( corr_back_steps );
    PNL_RELEASE_AUTOBUF( corr_steps );
    PNL_RELEASE_AUTOBUF( corr );
    PNL_RELEASE_AUTOBUF( big_back_steps );
    PNL_RELEASE_AUTOBUF( big_steps );
    PNL_RELEASE_AUTOBUF( smal_back_steps );
    PNL_RELEASE_AUTOBUF( smal_steps );
    PNL_RELEASE_AUTOSTACK( final_dims_to_trail );
    PNL_RELEASE_AUTOSTACK( swamp_dims );
    PNL_RELEASE_AUTOSTACK( final_dims_to_mul );
}

#define PNL_MULINSELF_DESIRED_PARBLOCK_SIZE PNL_MARG_DESIRED_PARBLOCK_SIZE
#define PNL_MULINSELF_MAX_PARBLOCK_SIZE PNL_MARG_MAX_PARBLOCK_SIZE

template< typename  T >
void iCNumericDenseMatrix< T >::MultiplyInSelf_omp( CMatrix< T > const *mat,
                                                         int num_dims_to_mul,
                                                         int const *dims_to_mul,
                                                         int, T const )
{
#if defined(_OPENMP) && !defined(PAR_OMP)
    PNL_USING

    int i, j, k;
    int big_num_dims;
    int const *big_ranges;
    T *big_bulk;
    int big_bulk_size;
    /* I spell small as smal here and there because dumb MFC defines global small *
     * and we can encounter some problems with macros                             */
    int smal_num_dims;
    int const *smal_ranges;
    T const *smal_bulk;
    int smal_bulk_size;
    int block_size, trail_block_size, num_blocked_dims, num_blocked_trail_dims;
    int step;
    int num_par_dims, par_block_size;
    int false_sharing_lim;

    PNL_DEFINE_AUTOSTACK( int, final_dims_to_mul, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOSTACK( int, swamp_dims, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOSTACK( int, final_dims_to_trail, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, smal_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, smal_back_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, big_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, big_back_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, corr, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, corr_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, corr_back_steps, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOBUF( int, block_offsets, PNL_MARG_MAX_BLOCK1_SIZE );
    PNL_DEFINE_AUTOBUF( int, big_trail_block_offsets, PNL_MARG_MAX_BLOCK2_SIZE );
    PNL_DEFINE_AUTOBUF( int, smal_trail_block_offsets, PNL_MARG_MAX_BLOCK2_SIZE );
    PNL_DEFINE_AUTOBUF( bool, mask, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );
    PNL_DEFINE_AUTOSTACK( int, par_dims, PNL_MULTIDMATRIX_EXPECTED_MAX_DIM );

    EMatrixClass other_class = mat->GetMatrixClass();
    if( other_class != mcNumericDense )
    {
        PNL_THROW( CInvalidOperation, "can't multiply dense in sparse" );
    }

    GetRanges( &big_num_dims, &big_ranges );
    ((iCNumericDenseMatrix< T > *)mat)->GetRanges( &smal_num_dims, &smal_ranges );
    if( smal_num_dims != num_dims_to_mul )
    {
        PNL_THROW( CInvalidOperation, "can multiply only by all dims of small matrix" );
    }
    GetRawData( &big_bulk_size, (T const **)&big_bulk );
    ((iCNumericDenseMatrix< T > *)mat)->GetRawData( &smal_bulk_size, &smal_bulk );

    PNL_DEMAND_AUTOBUF( corr, big_num_dims );
    for ( i = big_num_dims; i--; )
    {
        corr[i] = -1;
    }
    for ( i = num_dims_to_mul; i--; )
    {
        if ( corr[dims_to_mul[i]] >= 0 )
        {
            PNL_THROW( CInvalidOperation, "dims_to_mul clash" );
        }
        corr[dims_to_mul[i]] = i;
    }

    PNL_DEMAND_AUTOSTACK( final_dims_to_mul, smal_num_dims );
    PNL_DEMAND_AUTOSTACK( swamp_dims, big_num_dims );
    PNL_DEMAND_AUTOSTACK( final_dims_to_trail, big_num_dims );
    PNL_DEMAND_AUTOBUF( smal_steps, smal_num_dims );
    PNL_DEMAND_AUTOBUF( smal_back_steps, smal_num_dims );
    PNL_DEMAND_AUTOBUF( big_steps, big_num_dims );
    PNL_DEMAND_AUTOBUF( big_back_steps, big_num_dims );
    PNL_DEMAND_AUTOBUF( corr_steps, big_num_dims );
    PNL_DEMAND_AUTOBUF( corr_back_steps, big_num_dims );
    PNL_DEMAND_AUTOBUF( mask, big_num_dims );
    PNL_DEMAND_AUTOSTACK( par_dims, smal_num_dims );

    PNL_MARG_INIT_STEPS( T, big_ranges, big_num_dims, big_steps, 
			 big_back_steps );
    PNL_MARG_INIT_STEPS( T, smal_ranges, smal_num_dims, smal_steps, 
			 smal_back_steps );

    big_bulk_size = big_back_steps[0] / sizeof( T );

    if ( big_bulk_size > 200 )
    {
        for ( false_sharing_lim = big_num_dims; false_sharing_lim--; )
        {
            if ( big_steps[false_sharing_lim] >= PNL_BIG_CACHELINE_BYTES )
            {
                break;
            }
        }

#if 1
        num_par_dims = margBlockize( dims_to_mul, num_dims_to_mul, big_ranges, par_block_size,
                           PNL_MULINSELF_DESIRED_PARBLOCK_SIZE, PNL_MULINSELF_MAX_PARBLOCK_SIZE );
#else
        PNL_MARG_BLOCKIZE( dims_to_mul, num_dims_to_mul, big_ranges, num_par_dims, par_block_size,
                           PNL_MULINSELF_DESIRED_PARBLOCK_SIZE, PNL_MULINSELF_MAX_PARBLOCK_SIZE );
#endif

        for ( i = num_par_dims; i--; )
        {
            if ( dims_to_mul[i] > false_sharing_lim )
            {
                PNL_AUTOSTACK_PUSH( final_dims_to_mul, dims_to_mul[i] );
                par_block_size /= smal_ranges[i];
            }
            else
            {
                PNL_AUTOSTACK_PUSH( par_dims, dims_to_mul[i] );
            }
        }

        while ( par_block_size < PNL_MULINSELF_DESIRED_PARBLOCK_SIZE
                && num_par_dims < num_dims_to_mul )
        {
            if ( dims_to_mul[num_par_dims] <= false_sharing_lim
                 && par_block_size * smal_ranges[num_par_dims] <= PNL_MULINSELF_MAX_PARBLOCK_SIZE )
            {
                par_block_size *= smal_ranges[num_par_dims];
                PNL_AUTOSTACK_PUSH( par_dims, dims_to_mul[num_par_dims] );
            }
            else if ( smal_ranges[num_par_dims] > 1 )
            {
                PNL_AUTOSTACK_PUSH( final_dims_to_mul, dims_to_mul[num_par_dims] );
            }
            ++num_par_dims;
        }

        for ( i = 0; par_block_size < PNL_MULINSELF_DESIRED_PARBLOCK_SIZE && i <= false_sharing_lim; ++i )
        {
            if ( corr[i] < 0 )
            {
                if ( par_block_size * big_ranges[i] <= PNL_MULINSELF_MAX_PARBLOCK_SIZE )
                {
                    par_block_size *= big_ranges[i];
                    PNL_AUTOSTACK_PUSH( par_dims, i );
                }
            }
        }
    }
    else
    {
        // going sequential
        num_par_dims = 0;
        par_block_size = 1;
    }

    for ( i = PNL_AUTOSTACK_SIZE( par_dims ); i--; )
    {
        corr[par_dims[i]] = -2;
    }

    for ( i = big_num_dims; i--; )
    {
        if ( big_ranges[i] > 1 )
        {
            if ( corr[i] == -1 )
            {
                PNL_AUTOSTACK_PUSH( swamp_dims, i );
            }
            else if ( corr[i] >= 0 )
            {
                PNL_AUTOSTACK_PUSH( final_dims_to_mul, i );
            }
        }
    }

    for ( i = num_par_dims; i--; )
    {
        corr[dims_to_mul[i]] = i;
    }

#if 1
    num_blocked_dims = margBlockize(swamp_dims, PNL_AUTOSTACK_SIZE( swamp_dims ),
        big_ranges, block_size, PNL_MULINSELF_DESIRED_BLOCK1_SIZE, PNL_MULINSELF_MAX_BLOCK1_SIZE);
#else
    PNL_MARG_BLOCKIZE( swamp_dims, PNL_AUTOSTACK_SIZE( swamp_dims ), big_ranges, num_blocked_dims, block_size,
                       PNL_MULINSELF_DESIRED_BLOCK1_SIZE, PNL_MULINSELF_MAX_BLOCK1_SIZE );
#endif

    PNL_DEMAND_AUTOBUF( block_offsets, block_size );

    PNL_MARG_INIT_OFFSETS( swamp_dims, big_ranges, num_blocked_dims, block_size,
                           block_offsets, big_steps, big_back_steps );

    for ( i = big_num_dims; i--; )
    {
        mask[i] = false;
    }
    for ( i = num_blocked_dims; i < PNL_AUTOSTACK_SIZE( swamp_dims ); ++i )
    {
        mask[swamp_dims[i]] = true;
    }
    for ( i = 0; i < PNL_AUTOSTACK_SIZE( final_dims_to_mul ); ++i )
    {
        mask[final_dims_to_mul[i]] = true;
    }
    for ( i = big_num_dims; i--; )
    {
        if ( mask[i] )
        {
            PNL_AUTOSTACK_PUSH( final_dims_to_trail, i );
        }
    }

    PNL_MARG_BLOCKIZE( final_dims_to_trail, PNL_AUTOSTACK_SIZE( final_dims_to_trail ), big_ranges, num_blocked_trail_dims,
                       trail_block_size, PNL_MULINSELF_DESIRED_BLOCK2_SIZE, PNL_MULINSELF_MAX_BLOCK2_SIZE );

    PNL_DEMAND_AUTOBUF( big_trail_block_offsets, trail_block_size );
    PNL_DEMAND_AUTOBUF( smal_trail_block_offsets, trail_block_size );

    PNL_MARG_INIT_OFFSETS( final_dims_to_trail, big_ranges, num_blocked_trail_dims, trail_block_size,
                           big_trail_block_offsets, big_steps, big_back_steps );

    for ( i = big_num_dims; i--; )
    {
        corr_steps[i] = corr_back_steps[i] = 0;
    }
    for ( i = num_dims_to_mul; i--; )
    {
        corr_steps[dims_to_mul[i]] = smal_steps[corr[dims_to_mul[i]]];
        corr_back_steps[dims_to_mul[i]] = smal_back_steps[corr[dims_to_mul[i]]];
    }

    PNL_MARG_INIT_OFFSETS( final_dims_to_trail, big_ranges, num_blocked_trail_dims, trail_block_size,
                           smal_trail_block_offsets, corr_steps, corr_back_steps );

    #pragma omp parallel for schedule( guided, 1 ), private( i, j, k )
    for ( int channel = 0; channel < par_block_size; ++channel )
    {
        PNL_MULINSELF_SPECMPLEXOR_OMP( big, smal, PNL_MARG_REVMUL, PNL_ASN, PNL_VOID2,
                                       par_dims, PNL_AUTOSTACK_SIZE( par_dims ) );
    }

    PNL_RELEASE_AUTOSTACK( par_dims );
    PNL_RELEASE_AUTOBUF( mask );
    PNL_RELEASE_AUTOBUF( smal_trail_block_offsets );
    PNL_RELEASE_AUTOBUF( big_trail_block_offsets );
    PNL_RELEASE_AUTOBUF( block_offsets );
    PNL_RELEASE_AUTOBUF( corr_back_steps );
    PNL_RELEASE_AUTOBUF( corr_steps );
    PNL_RELEASE_AUTOBUF( corr );
    PNL_RELEASE_AUTOBUF( big_back_steps );
    PNL_RELEASE_AUTOBUF( big_steps );
    PNL_RELEASE_AUTOBUF( smal_back_steps );
    PNL_RELEASE_AUTOBUF( smal_steps );
    PNL_RELEASE_AUTOSTACK( final_dims_to_trail );
    PNL_RELEASE_AUTOSTACK( swamp_dims );
    PNL_RELEASE_AUTOSTACK( final_dims_to_mul );
#endif
}

template< typename  T >
void iCNumericDenseMatrix< T >::MultiplyInSelf( CMatrix< T > const *mat,
                                                     int num_dims_to_mul,
                                                     int const *dims_to_mul,
                                                     int, T const )
{
        if (m_Dim > 2)
        {
#if defined(_OPENMP) && !defined(PAR_OMP)
                MultiplyInSelf_omp( mat, num_dims_to_mul, dims_to_mul );
#else
                MultiplyInSelf_plain( mat, num_dims_to_mul, dims_to_mul );
#endif
        }
        else
        {
                MultiplyInSelf2D_plain( mat, num_dims_to_mul, dims_to_mul );
        }
}

template<class Type>
void iCNumericDenseMatrix<Type>::OldMultiplyInSelf(
                            const CMatrix<Type>* matToMult,
                            int numDimsToMult, const int* indicesToMultInSelf,
                            int isUnifrom, const Type uniVal )
{
    PNL_USING
    EMatrixClass otherClass = matToMult->GetMatrixClass();
    if( otherClass != mcNumericDense )
    {
        PNL_THROW( CInvalidOperation, "can't multiply dense in sparse" );
    }
    int BigNumDims;
    const int *pBigNodeSizes;
        GetRanges( &BigNumDims, &pBigNodeSizes );
    int SmallNumDims;
        const int *pSmallNodeSizes;
    const iCNumericDenseMatrix<Type>* matToMultD = static_cast<const
        iCNumericDenseMatrix<Type>*>(matToMult);
        matToMultD->GetRanges( &SmallNumDims, &pSmallNodeSizes );
    if( SmallNumDims != numDimsToMult )
    {
        PNL_THROW( CInvalidOperation,
            "can multiply only by all dimensions of small matrix" )
    }
    intVector pconvIndBig;
    pconvIndBig.resize( BigNumDims );
    intVector pconvIndSmall;
    pconvIndSmall.resize( SmallNumDims );

    //create storage to keep result
    int ResultSize = 0;
    const Type *pBigData;
    GetRawData( &ResultSize, &pBigData );

    int NSmallLength = 0;
    const Type *pSmallData;
    matToMultD->GetRawData( &NSmallLength, &pSmallData );

    pnlVector<Type> pResultData;
    pResultData.resize(ResultSize);
    int nlineBig = 1;
    int nlineSmall = 1;
    /*filling the arrays pconvIndBig & pconvIndSmall, used to convert
    linear indexes to multidim & backwards in BigData and SmallData*/
    div_t result;

    for( int i1 = BigNumDims-1; i1 >= 0; i1--)
    {
        pconvIndBig[i1] = nlineBig;
        int bigNodeSize = pBigNodeSizes[i1];
        nlineBig = nlineBig*bigNodeSize;
    }

    for( int i2 = SmallNumDims-1; i2 >= 0; i2--)
    {
        pconvIndSmall[i2] = nlineSmall;
        int smallNodeSize = pSmallNodeSizes[i2];
        nlineSmall = nlineSmall*smallNodeSize;
    }
    /*the main multiplication cycle begins*/
    intVector pindBig;
    pindBig.resize(BigNumDims);
    intVector pindSmall;
    pindSmall.resize(SmallNumDims);

    for (int ind_res = 0; ind_res < ResultSize; ind_res++ )
    {
        //counting multi indexes from ind_res
        int hres = ind_res;
        for( int k = 0; k < BigNumDims; k++ )
        {
            int pConvInd = pconvIndBig[k];
            result = div( hres, pConvInd );
            pindBig[k] = result.quot;
            hres = result.rem;
        }
        //getting multi indexes for SmallData
        for( int k1 = 0; k1 < SmallNumDims; k1++ )
        {
            pindSmall[k1] = pindBig[indicesToMultInSelf[k1]];
        }

        //counting linear ind_small for SmallData
        int ind_small = 0;
        int hlind = 1;
        for(int k2 = 0; k2 < SmallNumDims; k2++ )
        {
            hlind = pindSmall[k2] * pconvIndSmall[k2];
            ind_small += hlind;
        }

        pResultData[ind_res] = pBigData[ind_res] * pSmallData[ind_small];
    }
    SetData( &pResultData.front());
}

template <class Type>
void iCNumericDenseMatrix<Type>::DivideInSelf(
                            const CMatrix<Type>* matToDiv,
                            int numDimsToDiv, const int* indicesToDivInSelf)
{
    PNL_USING
    EMatrixClass otherClass = matToDiv->GetMatrixClass();
    if( otherClass != mcNumericDense )
    {
        PNL_THROW( CInvalidOperation, "can't divide dense in sparse" );
    }
    int BigNumDims;
    const int *pBigNodeSizes;
    GetRanges( &BigNumDims, &pBigNodeSizes );
    int SmallNumDims;
    const int *pSmallNodeSizes;
    const iCNumericDenseMatrix<Type>* matToDivD = static_cast<const
        iCNumericDenseMatrix<Type>*>(matToDiv);
    matToDivD->GetRanges( &SmallNumDims, &pSmallNodeSizes );
    if( SmallNumDims != numDimsToDiv )
    {
        PNL_THROW( CInvalidOperation,
            "can multiply only by all dimensions of small matrix" )
    }
    intVector pconvIndBig;
    pconvIndBig.resize( BigNumDims );
    intVector pconvIndSmall;
    pconvIndSmall.resize( SmallNumDims );

    //create storage to keep result
    int ResultSize = 0;
    const Type *pBigData;
    GetRawData( &ResultSize, &pBigData );

    int NSmallLength = 0;
    const Type *pSmallData;
    matToDivD->GetRawData( &NSmallLength, &pSmallData );

    pnlVector<Type> pResultData;
    pResultData.resize(ResultSize);
    int nlineBig = 1;
    int nlineSmall = 1;
    /*filling the arrays pconvIndBig & pconvIndSmall, used to convert
    linear indexes to multidim & backwards in BigData and SmallData*/
    div_t result;

    for( int i1 = BigNumDims-1; i1 >= 0; i1--)
    {
        pconvIndBig[i1] = nlineBig;
        int bigNodeSize = pBigNodeSizes[i1];
        nlineBig = nlineBig*bigNodeSize;
    }

    for( int i2 = SmallNumDims - 1; i2 >= 0; i2--)
    {
        pconvIndSmall[i2] = nlineSmall;
        int smallNodeSize = pSmallNodeSizes[i2];
        nlineSmall = nlineSmall*smallNodeSize;
    }
    /*the main multiplication cycle begins*/
    intVector pindBig;
    pindBig.resize(BigNumDims);
    intVector pindSmall;
    pindSmall.resize(SmallNumDims);

    for (int ind_res = 0; ind_res < ResultSize; ind_res++ )
    {
        //counting multi indexes from ind_res
        int hres = ind_res;
        for( int k = 0; k < BigNumDims; k++ )
        {
            int pConvInd = pconvIndBig[k];
            result = div( hres, pConvInd );
            pindBig[k] = result.quot;
            hres = result.rem;
        }
        //getting multi indexes for SmallData
        for( int k1 = 0; k1 < SmallNumDims; k1++ )
        {
            pindSmall[k1] = pindBig[indicesToDivInSelf[k1]];
        }

        //counting linear ind_small for SmallData
        int ind_small = 0;
        int hlind = 1;
        for(int k2 = 0; k2 < SmallNumDims; k2++ )
        {
            hlind = pindSmall[k2] * pconvIndSmall[k2];
            ind_small += hlind;
        }
        Type divVal =  pSmallData[ind_small];
        if( divVal )
        {
            pResultData[ind_res] = pBigData[ind_res] / divVal;
        }
    }

    SetData( &pResultData.front() );
}

template <class Type>
void iCNumericDenseMatrix<Type>::GetIndicesOfMaxValue( intVector* indices)
                                                                 const
{
    PNL_USING
    PNL_CHECK_IS_NULL_POINTER( indices );

    int length; const Type * data;
    GetRawData( &length, &data);
    int nDims; const int *ranges;
    GetRanges( &nDims, & ranges);
    indices->assign( nDims, 0 );
    const Type* location = std::max_element( data, data + length);
    int offset = location - data;
    int residue = offset;
    div_t division;
    for ( int i = nDims- 1; i >= 0 ; i-- )
    {
        division = div( residue, ranges[i] );
        residue = division.quot;
        (*indices)[i] = division.rem;
    }
}

//implement virtual methods from base
template <class Type>
inline int iCNumericDenseMatrix<Type>::GetNumberDims() const
{
    PNL_USING
    return CDenseMatrix<Type>::GetNumberDims();
}
template <class Type>
inline void iCNumericDenseMatrix<Type>::GetRanges(int *numOfDims,
                                         const int **ranges) const
{
    PNL_USING
    CDenseMatrix<Type>::GetRanges(numOfDims, ranges);
}
//methods for viewing/change data

//this function return an element of matrix for
//given multidimensional indexes
template <class Type>
inline Type iCNumericDenseMatrix<Type>::GetElementByIndexes(
                                           const int *multidimindexes) const
{
    PNL_USING
    return CDenseMatrix<Type>::GetElementByIndexes(multidimindexes);
}
//this function set matrix element equal value (for
//given multidimensional indexes )
template <class Type>
inline void iCNumericDenseMatrix<Type>::SetElementByIndexes(Type value,
                                               const int *multidimindexes)
{
    PNL_USING
    CDenseMatrix<Type>::SetElementByIndexes( value, multidimindexes );
}

template <class Type>
CMatrix<Type> *
iCNumericDenseMatrix<Type>::ExpandDims(const int *dimsToExtend,
                                            const int *keepPosOfDims,
                                            const int *sizesOfExpandDims,
                                            int numDimsToExpand) const
{
    PNL_USING
    return CDenseMatrix<Type>::ExpandDims( dimsToExtend, keepPosOfDims,
                                           sizesOfExpandDims, numDimsToExpand );
}

template <class Type>
inline void iCNumericDenseMatrix<Type>::ClearData()
{
    PNL_USING
    CDenseMatrix<Type>::ClearData();
}


template <class Type>
inline void iCNumericDenseMatrix<Type>::SetUnitData() 
{
    for( int i = 0; i < m_Table.size(); i++ )
    {
        m_Table[i] = static_cast<Type>(1);
    }
}


template <class Type>
iCNumericDenseMatrix<Type>::iCNumericDenseMatrix( int dim,
                                 const int *range, const Type *data, int Clamp)
:CDenseMatrix<Type>( dim, range, data, Clamp )//, CMatrix<Type>(Clamp)
{
}

template <class Type>
iCNumericDenseMatrix<Type>::iCNumericDenseMatrix(
                               const iCNumericDenseMatrix<Type> & inputMat )
                               :CDenseMatrix<Type>( inputMat )//, CMatrix<Type>(0)
{
}

// ----------------------------------------------------------------------------
template< typename  T >
void pnl::iCNumericDenseMatrix< T >::SumInSelf(
  pnl::CMatrix< T > const *mat, int num_dims_to_sum,
  int const *dims_to_sum, int, T const)
{
  int big_num_dims;
  int const *big_ranges;
  T *big_bulk;
  int big_bulk_size;

  const T *dst_bulk;
  int dst_bulk_size;
  int safe_dst_num_dims;
  int const *safe_dst_ranges;

  EMatrixClass other_class = mat->GetMatrixClass();
  if (other_class != mcNumericDense)
  {
    PNL_THROW(CInvalidOperation, "can't multiply dense in sparse");
  }
  
  GetRanges(&big_num_dims, &big_ranges);
  ((iCNumericDenseMatrix< T > *)mat)->
    GetRanges(&safe_dst_num_dims, &safe_dst_ranges);
  if (safe_dst_num_dims != num_dims_to_sum)
  {
    PNL_THROW(CInvalidOperation, 
      "can multiply only by all dims of small matrix");
  }
  GetRawData(&big_bulk_size, (T const **)&big_bulk);
  ((iCNumericDenseMatrix< T > *)mat)->GetRawData(&dst_bulk_size, &dst_bulk);

  const int MAX_SIZE = 100;
  int i, j;

//  int dst_mult_count = big_bulk_size / dst_bulk_size;
  int num_dims_unkeep_mult = big_num_dims - safe_dst_num_dims;

  int keep_steps_mult[MAX_SIZE];
  int step = 1;
  for (i = safe_dst_num_dims - 1; i >=0; i--)
  {
    keep_steps_mult[i] = step;
    step *= big_ranges[dims_to_sum[i]];
  }

  int dims_unkeep_mult[MAX_SIZE];
  int pos = 0, loc;
  for (i = 0; i < big_num_dims; i++)
  {
    loc = 0;
    for (j = 0; j < safe_dst_num_dims; j++)
      if (i == dims_to_sum[j])
        loc = 1;
    if (!loc)
      dims_unkeep_mult[pos++] = i;
  }

  int smal_steps_mult[MAX_SIZE];
  for (i = safe_dst_num_dims - 1; i >=0; i--)
  {
    smal_steps_mult[dims_to_sum[i]] = keep_steps_mult[i];
  }
  for (i = num_dims_unkeep_mult - 1; i >=0; i--)
  {
    smal_steps_mult[dims_unkeep_mult[i]] = 0;
  }
  int dbsteps_mult[MAX_SIZE];
  for (i = big_num_dims - 1; i >=0; i--)
  {
    dbsteps_mult[i] = smal_steps_mult[i] * big_ranges[i];
  }
  for (i = 0; i < big_num_dims - 1; i++)
  {
    dbsteps_mult[i] = smal_steps_mult[i] - dbsteps_mult[i + 1];
  }

// --- Main Loop -------------------------------------------------------
  int counts_mult[MAX_SIZE];
  memset(counts_mult, 0, big_num_dims * sizeof(int));
  int dst_pos = 0;
  int p = big_num_dims - 1;
  for (i = 0; i < big_bulk_size; i++)
  {
    while (counts_mult[p] == big_ranges[p])
    {
      counts_mult[p] = 0;
      p--;
      counts_mult[p]++;
      dst_pos += dbsteps_mult[p];
    }

    big_bulk[i] += dst_bulk[dst_pos];

    p = big_num_dims - 1;
    counts_mult[p]++;
    dst_pos += smal_steps_mult[p];
  }
}

#endif //!defined(SWIG)

#ifdef PNL_RTTI
template< typename  T >
const CPNLType iCNumericDenseMatrix< T >::m_TypeInfo = CPNLType("iCNumericDenseMatrix", &(CDenseMatrix< T >::m_TypeInfo));

#endif
PNL_END

#endif //__PNLINUMERICDENSEMATRIX_HPP__
