/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnli2dnumericdensematrix.hpp                                //
//                                                                         //
//  Purpose:   C2DNumericDenseMatrix class definition & implementation     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLI2DNUMERICDENSEMATRIX_HPP__
#define __PNLI2DNUMERICDENSEMATRIX_HPP__

#include "pnlNumericDenseMatrix.hpp"
#include <math.h>
#include "cvsvd.h"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

template< typename T > class C2DNumericDenseMatrix;

template< typename T >
C2DNumericDenseMatrix< T > *pnlMultiply( const C2DNumericDenseMatrix< T > *pxMatrix1,
                                         const C2DNumericDenseMatrix< T > *pxMatrix2,
                                         int maximize );

template <class Type> class PNL_API iC2DNumericDenseMatrix :
                             public CNumericDenseMatrix<Type>
{
public:
    static C2DNumericDenseMatrix<Type> *Create( const int* lineSizes,
		const Type *data, int Clamp = 0 );
    static C2DNumericDenseMatrix<Type>* Copy( 
                            const iC2DNumericDenseMatrix* pInpMat );
    virtual CMatrix<Type>* CreateEmptyMatrix( int dim, const int *range,
         int Clamp, Type defaultVal = Type(0) ) const;

	static C2DNumericDenseMatrix<Type>* CreateIdentityMatrix( int lineSize );
    virtual CMatrix<Type>* Clone() const ;
    //methods for conversion from other type
    //if requered type is the same as type of argument - return copy of object
    virtual CDenseMatrix<Type>* ConvertToDense() const;
    virtual CSparseMatrix<Type>* ConvertToSparse() const;
    
    virtual EMatrixClass GetMatrixClass() const;
    int IsSymmetric( Type epsilon ) const;
    Type Trace() const;
    int IsIllConditioned( Type conditionRatio = Type(1e+8), bool asRatio = 1 ) const;
    Type Determinant() const;
    C2DNumericDenseMatrix<Type>* Inverse() const;
    C2DNumericDenseMatrix<Type>* Transpose()const;
    void GetLinearBlocks( const int *X, int xSize,const int *blockSizes,
            int numBlocks, C2DNumericDenseMatrix<Type> **matXOut, 
            C2DNumericDenseMatrix<Type> **matYOut ) const;
    void GetBlocks( const int *X, int xSize,const int *blockSizes, int numBlocks,
			C2DNumericDenseMatrix<Type> **matXOut, 
            C2DNumericDenseMatrix<Type> **matYOut,
			C2DNumericDenseMatrix<Type> **matXYOut,
            C2DNumericDenseMatrix<Type> **matYXOut )const;

#ifndef PNL_VC6
    friend PNL_API C2DNumericDenseMatrix<Type>* pnlMultiply<>( const C2DNumericDenseMatrix<Type>* pxMatrix1,
                                                               const C2DNumericDenseMatrix<Type>* pxMatrix2,
                                                               int maximize );
#else
    friend PNL_API C2DNumericDenseMatrix<Type>* pnlMultiply( const C2DNumericDenseMatrix<Type>* pxMatrix1,
                                                             const C2DNumericDenseMatrix<Type>* pxMatrix2,
                                                             int maximize );
#endif

    void icvSVBkSb(int m, int n, const Type* w,
                    const Type* uT, int lduT, int nu,
                    const Type* vT, int ldvT,
                    const Type* b, int ldb, int nb,
                    Type* xOut, Type* bufferIn) const;
    void icvSVD( Type* a, int lda, int m, int n,
                 Type* w,
                 Type* uT, int lduT, int nu,
                 Type* vT, int ldvT,
                 Type* bufferIn ) const;
    ~iC2DNumericDenseMatrix(){};

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return iC2DNumericDenseMatrix< float >::GetStaticTypeInfo();
    }
#endif

#if GCC_VERSION >= 30400
    using CDenseMatrix<Type>::GetClampValue;
    using CDenseMatrix<Type>::GetRanges;
    using CDenseMatrix<Type>::GetElementByIndexes;
#endif

protected:
    iC2DNumericDenseMatrix( int dim, const int *range, const Type *data,
                                                                int Clamp);
    iC2DNumericDenseMatrix( const iCNumericDenseMatrix<Type> & inputMat );

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 

#if GCC_VERSION >= 30400
    using CDenseMatrix<Type>::m_Dim;
    using CDenseMatrix<Type>::m_Range;
    using CDenseMatrix<Type>::m_Table;
#endif

};


#if !defined(SWIG) && (!defined(PNL_VC7) || defined(PNL_EXPORTS))

template <class Type>
C2DNumericDenseMatrix<Type>* iC2DNumericDenseMatrix<Type>::Create( 
                           const int* lineSizes, const Type *data, int Clamp )
{
    PNL_THROW( CInvalidOperation,
        "Numeric matrices assume float or double types only!" )
    return NULL;
}

template <class Type>
CMatrix<Type>* iC2DNumericDenseMatrix<Type>::CreateEmptyMatrix( int dim,
                          const int* range, int Clamp, Type defaultVal ) const
{
    if( dim != 2 )
    { 
        PNL_THROW( COutOfRange, "number of dims must be 2" );
    }
    int size = range[0]*range[1];
    pnlVector<Type> data;
    data.assign( size, defaultVal );
    return C2DNumericDenseMatrix<Type>::Create( range, &data.front(), Clamp );
}


template <class Type>
C2DNumericDenseMatrix<Type>* iC2DNumericDenseMatrix<Type>
::CreateIdentityMatrix(int lineSize)
{
    PNL_THROW( CInvalidOperation,
        "Numeric matrices assume float or double types only!" )
    return NULL;
	
}

template <class Type>
C2DNumericDenseMatrix<Type>* iC2DNumericDenseMatrix<Type>::Copy( 
                                   const iC2DNumericDenseMatrix<Type>* inputMat)
{
    PNL_CHECK_IS_NULL_POINTER( inputMat );
    iC2DNumericDenseMatrix<Type> *retMat = new iC2DNumericDenseMatrix<Type>( 
                                                                *inputMat );
    PNL_CHECK_IF_MEMORY_ALLOCATED( retMat );
    return static_cast<C2DNumericDenseMatrix<Type>*>(retMat);
}

template <class Type>
CMatrix<Type>* iC2DNumericDenseMatrix<Type>::Clone() const
{
    iC2DNumericDenseMatrix<Type>* res = new iC2DNumericDenseMatrix<Type>(*this);
    PNL_CHECK_IF_MEMORY_ALLOCATED(res);
    return static_cast<CDenseMatrix<Type>*>(res);
}

template <class Type>
CDenseMatrix<Type>* iC2DNumericDenseMatrix<Type>::ConvertToDense() const
{
    return static_cast<CDenseMatrix<Type>*>(Clone());
}

template <class Type>
CSparseMatrix<Type>* iC2DNumericDenseMatrix<Type>::ConvertToSparse() const
{
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
EMatrixClass iC2DNumericDenseMatrix<Type>::GetMatrixClass() const
{
    return mc2DNumericDense;
}

template<class Type>
C2DNumericDenseMatrix<Type>* pnlMultiply(
    const C2DNumericDenseMatrix<Type>* pxMatrix1,
    const C2DNumericDenseMatrix<Type>* pxMatrix2, 
    int maximize = 0 )
{
    if(( !pxMatrix1 )||( !pxMatrix2 ))
    {
	PNL_THROW( CNULLPointer, "one of input matrices" );
    }
    int dim1;
    const int * ranges1;
    pxMatrix1->GetRanges( &dim1, &ranges1 );
    int dim2; const int * ranges2;
    pxMatrix2->GetRanges( &dim2, &ranges2 );
    int Nrow1=0;
    int Ncol1=0;
    int Ncol2=0;
    const Type *data1;
    int N1;
    const Type *data2;
    int N2;
    if( ranges1[1] == ranges2[0] )
    {
	Nrow1 = ranges1[0]; 
	Ncol1 = ranges1[1];
	Ncol2 = ranges2[1];
	pxMatrix1->GetRawData(&N1, &data1);
	pxMatrix2->GetRawData(&N2, &data2);
    }
    else
    {
	if ( ranges1[0] == ranges2[1] )
	{
	    Nrow1 = ranges2[0];
	    Ncol1 = ranges2[1];
	    Ncol2 = ranges1[1];
	    pxMatrix2->GetRawData(&N1, &data1);
	    pxMatrix1->GetRawData(&N2, &data2);
	}
	else
	{
	    data1 = NULL;
	    data2 = NULL;
	    PNL_THROW( CInconsistentSize,
		"number of rows and columns input matrixes doesn't match" )
		//order=0 
	}
    }

    int *ranges3 = new int[2];
    PNL_CHECK_IF_MEMORY_ALLOCATED( ranges3 );
    ranges3[0] = Nrow1;
    ranges3[1] = Ncol2;
    int datasize = ranges3[0]*ranges3[1];
    Type *data3 = new Type[datasize];
    PNL_CHECK_IF_MEMORY_ALLOCATED( data3 );
    int i, j;
    div_t division;
    if(maximize)
    {
	for( i = 0; i < datasize; i++ )
	{
	    Type maxim = FLT_MIN;
	    division = div(i,Ncol2);
	    int currentrow = division.quot + 1;
	    int currentcol = division.rem + 1;
	    Type h1 = 0;
	    for(j = 0; j < Ncol1; j++)
	    {
		int a = (currentrow-1)*Ncol1 + j;
		int b = (currentcol-1) + j*Ncol2;
		h1 = data1[a]*data2[b];
		if ( h1 > maxim)
		    maxim = h1;
	    }
	    data3[i] = h1;
	}
    }
    else //maximaze=0 - Sum
    {
	for( i = 0; i < datasize; i++ )
	{
	    division = div(i,Ncol2);
	    int currentrow = division.quot + 1;
	    int currentcol = division.rem + 1;
	    Type h1 = 0;
	    for( j = 0; j < Ncol1; j++ )
	    {
		int a = (currentrow - 1)*Ncol1 + j;
		int b = (currentcol-1) + j*Ncol2;
		h1 = h1 + data1[a]*data2[b];
	    }
	    data3[i] = h1;
	}
    }
    C2DNumericDenseMatrix<Type>* pxMatrix3 = C2DNumericDenseMatrix<Type>::
	Create( ranges3, data3);
    delete []ranges3;
    delete []data3;
    return (pxMatrix3);
}

template <class Type>
int iC2DNumericDenseMatrix<Type>::IsSymmetric( Type epsilon )const
{
    PNL_THROW( CInvalidOperation, 
	"can work only with matrices of float or double" );
    return 0;
}

template <class Type>
int iC2DNumericDenseMatrix<Type>::IsIllConditioned( Type conditionRatio, bool asRatio )const
{
    int n = m_Range[0];
    Type *w = new Type[n];
    PNL_CHECK_IF_MEMORY_ALLOCATED( w );
    Type *uT = new Type[n*n];
    PNL_CHECK_IF_MEMORY_ALLOCATED( uT );
    Type *vT = new Type[n*n];
    PNL_CHECK_IF_MEMORY_ALLOCATED( vT );
    Type * buff = new Type[2*n];
    PNL_CHECK_IF_MEMORY_ALLOCATED( buff );
    Type *dataCopy = new Type[n*n];
    PNL_CHECK_IF_MEMORY_ALLOCATED( dataCopy );
    memcpy(dataCopy, &m_Table.front(), n*n*sizeof(Type));
    const C2DNumericDenseMatrix<Type>* mat = static_cast<
    const C2DNumericDenseMatrix<Type>*>(this);
    mat->icvSVD( dataCopy, n, n, n, w, uT, n, n, vT, n, buff );
    int ret = 0;
    if( asRatio )
    {
        Type ratio = w[0]/w[n-1];
        if( ratio >= conditionRatio )
	{
	    ret = 1;
	}
	else
	{
	    ret = 0;
	}
    }
    else
    {
        if( w[n-1] < conditionRatio ) 
        {
            ret = 0;
        }
        else
        {
            ret = 1;
        }
    }
    delete []dataCopy;
    delete []buff;
    delete []vT;
    delete []uT;
    delete []w;
    return ret;
}

template <class Type>
Type iC2DNumericDenseMatrix<Type>::Determinant() const
{
    Type det = 1.0f;
    int m = m_Range[0];
    int n = m_Range[1];
    if( m != n )
    {
	PNL_THROW( CInvalidOperation, "determinant - for square matrix only" );
    }
    Type *buffer = new Type[m+n];
    PNL_CHECK_IF_MEMORY_ALLOCATED( buffer );
    //fixme - is it data modified by the function? - yes?
    Type *dataCopy = new Type[m*n];
    PNL_CHECK_IF_MEMORY_ALLOCATED( dataCopy );
    Type *w = new Type[n];
    PNL_CHECK_IF_MEMORY_ALLOCATED( w );
    memcpy(dataCopy, &m_Table.front(), m*n*sizeof(Type));
    const C2DNumericDenseMatrix<Type>* mat = static_cast<
	const C2DNumericDenseMatrix<Type>*>(this);
    mat->icvSVD( dataCopy, n, n, n, w, NULL, n, n, NULL, n, buffer );
    for( int i = 0; i < n; i++ )
    {
	det = det*w[i];
    }
    delete []buffer;
    delete []dataCopy;
    delete []w;
    return det;
}

template <class Type>
C2DNumericDenseMatrix<Type> *iC2DNumericDenseMatrix<Type>::Inverse() const
{
    if( m_Range[0] != m_Range[1] )
    {
        PNL_THROW( CInvalidOperation, "we can inverse only square plane matrix" );
        return NULL;
    }
    else
    {
        if( m_Range[0] == 1 )
        {
            float reciprocal = 1/m_Table[0];
            C2DNumericDenseMatrix<Type> *InvMat = C2DNumericDenseMatrix<Type>::
                Create( &m_Range.front(), &reciprocal );
            return InvMat;
        }
        else
        {
            //provide all calculations in double
            int i;
			int n = m_Range[0];
            //find singular value decomposition
            double *w = new double[n];
            PNL_CHECK_IF_MEMORY_ALLOCATED( w );
            double *uT = new double[n*n];
            PNL_CHECK_IF_MEMORY_ALLOCATED( uT );
            double *vT = new double[n*n];
            PNL_CHECK_IF_MEMORY_ALLOCATED( vT );
            double * buff = new double[2*n];
            PNL_CHECK_IF_MEMORY_ALLOCATED( buff );
            double *dataCopy = new double[n*n];
            PNL_CHECK_IF_MEMORY_ALLOCATED( dataCopy );
            
            for( i = 0; i < n*n; i++ )
            {
                dataCopy[i] = static_cast<double>( m_Table[i] );
            }
            //const C2DNumericDenseMatrix<Type>* mat = static_cast<
            //    const C2DNumericDenseMatrix<Type>*>(this);
            icvSVD_64f( dataCopy, n, n, n, w, uT, n, n, vT, n, buff );
            /*
            Type con = w[n-1]/w[0];
            if( fabs(con) < (1/FLT_MAX) )
            {
				PNL_THROW( CInvalidOperation, 
				"matrix is bad conditioned - no inverse matrix" );
				return NULL;
            }
			*/
            double *IMat = new double[n*n];
            for( i = 0; i < n; i++ )
            {
                for( int j = 0; j < n; j++ )
                {
                    if( i==j )
                    {
                        IMat[i*n+j] = 1.0;
                    }
                    else
                    {
                        IMat[i*n+j] = 0.0;
                    }
                }
            }
            double *buffer = new double[n];
            PNL_CHECK_IF_MEMORY_ALLOCATED( buffer );
            double *x = new double [n*n];
            PNL_CHECK_IF_MEMORY_ALLOCATED( x );
            icvSVBkSb_64f( n, n, w, uT, n, n, vT, n, IMat, n, n, x, buffer);
            delete []dataCopy;
            delete []buff;
            delete []IMat;
            delete []buffer;
            delete []w;
            delete []uT;
            delete []vT;
            //create new array of Type of size x
            Type* xType = new Type[n*n];
            for( i = 0; i < n*n; i++ )
            {
                xType[i] = static_cast<Type>( x[i] );
            }
            C2DNumericDenseMatrix<Type> *InvMat = C2DNumericDenseMatrix<Type>::
                Create( &m_Range.front(), xType );
            delete[]x;
            delete[]xType;
            return static_cast<C2DNumericDenseMatrix<Type>*>(InvMat);
        }
    }
}

template <class Type>
C2DNumericDenseMatrix<Type>* iC2DNumericDenseMatrix<Type>::Transpose()const
{
    int i,j;
    int m = m_Range[0];
    int n = m_Range[1];
    Type *Table = new Type[m*n];
    PNL_CHECK_IF_MEMORY_ALLOCATED( Table );
    for( i = 0; i < m; i++ )
    {
	for( j = 0; j < n; j++ )
	{
	    Table[j*m+i] = m_Table[i*n+j];
	}
    }
    int *Ranges = new int[2];
    PNL_CHECK_IF_MEMORY_ALLOCATED( Ranges );
    Ranges[0] = n;
    Ranges[1] = m;
    C2DNumericDenseMatrix<Type> *matTransp = C2DNumericDenseMatrix<Type>::
	Create( Ranges, Table );
    delete []Ranges;
    delete []Table;
    return static_cast<C2DNumericDenseMatrix<Type>*>(matTransp);			
}

	
template <class Type>
void iC2DNumericDenseMatrix<Type>::GetBlocks( const int *X, int xSize,
            const int *blockSizes, int numBlocks,
            C2DNumericDenseMatrix<Type> **matX, 
            C2DNumericDenseMatrix<Type> **matY,
            C2DNumericDenseMatrix<Type> **matXY,
            C2DNumericDenseMatrix<Type> **matYX )const
{
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
        (*matX) = C2DNumericDenseMatrix<Type>::Create( &m_Range.front(), &m_Table.front(),
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
        (*matY) = C2DNumericDenseMatrix<Type>::Create( &m_Range.front(), &m_Table.front(),
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
                xBackMap[i] = j;
                mapX[i] = 1;
                xSizeInTypes += blockSizes[i];
                xBlockSizes[j] = blockSizes[i];
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
    if( xSizeInTypes == 0 )
    {
        //we have only Y matrices
        (*matX) = NULL;
        (*matY) = C2DNumericDenseMatrix<Type>::Create( &m_Range.front(), &m_Table.front(),
                                                    GetClampValue() );
        (*matXY) = NULL;
        (*matYX) = NULL;
        return;
    }
    if( ySizeInTypes == 0 )
    {
        (*matX) = C2DNumericDenseMatrix<Type>::Create( &m_Range.front(), &m_Table.front(),
                                                    GetClampValue() );
        (*matY) = NULL;
        (*matXY) = NULL;
        (*matYX) = NULL;
        return;
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
    (*matX) = C2DNumericDenseMatrix<Type>::Create( &ranges.front(), &dataX.front());
    ranges[1] = ySizeInTypes;
    (*matXY) = C2DNumericDenseMatrix<Type>::Create( &ranges.front(), &dataXY.front());
    ranges[0] = ySizeInTypes;
    (*matY) = C2DNumericDenseMatrix<Type>::Create( &ranges.front(), &dataY.front());
    ranges[1] = xSizeInTypes;
    (*matYX) = C2DNumericDenseMatrix<Type>::Create( &ranges.front(), &dataYX.front());
}

template <class Type>
void iC2DNumericDenseMatrix<Type>::GetLinearBlocks(const int *X,
                             int xSize, const int *blockSizes, int numBlocks,
                             C2DNumericDenseMatrix<Type> **matX,
                             C2DNumericDenseMatrix<Type> **matY )const
{
    if( m_Range[1] != 1 )
    {
	PNL_THROW( CInvalidOperation, "we can divide in blocks only line matrices" );
    }
    if( xSize > numBlocks )
    {
    	PNL_THROW( COutOfRange, "xSize > numBlocks" );
    }
    if( xSize == numBlocks )
    {
	//we need to create full copy of this matrix as matX
	(*matX) = C2DNumericDenseMatrix<Type>::Create( &m_Range.front(), &m_Table.front(),
                                                       GetClampValue() );
	(*matY) = NULL;
	return;
    }
    if( xSize == 0 )
    {
	// we need to create full copy of this matrix as matY
	(*matX) = NULL;
	(*matY) = C2DNumericDenseMatrix<Type>::Create( &m_Range.front(), &m_Table.front(),
                                                      GetClampValue() );
	return;
    }
    int i;
    int sumBlockSizes = 0;
    intVector offsets;
    offsets.resize(numBlocks + 1);
    offsets[0] = 0;
    for( i = 0; i < numBlocks; i++ )
    {
        offsets[i+1] = offsets[i] + blockSizes[i];
    }
    if( offsets[numBlocks] != m_Range[0] )
    {
    	PNL_THROW( CInconsistentSize, "block sizes should sum to matrix range" )
    }
    pnlVector<Type> dataX;
    dataX.reserve(sumBlockSizes);
    pnlVector<Type> dataY;
    dataY.reserve(sumBlockSizes);
    for( i = 0; i < xSize; i++ )
    {
		dataX.insert(dataX.end(), m_Table.begin() + offsets[X[i]],
			m_Table.begin() + offsets[X[i] + 1]);
	}
    int loc;
    for( i = 0; i < numBlocks; i++ )
    {
        loc = std::find( X, X + xSize, i ) - X;
        if( loc == xSize )
        {
            dataY.insert(dataY.end(), m_Table.begin() + offsets[i],
                m_Table.begin() + offsets[i + 1]);
        }
    }
    int xSizeInTypes = dataX.size();
    int ySizeInTypes = dataY.size();
    if( xSizeInTypes == 0 )
    {
        //we have only Y matrices
        (*matX) = NULL;
        (*matY) = C2DNumericDenseMatrix<Type>::Create( &m_Range.front(), &m_Table.front(),
                                                    GetClampValue() );
        return;
    }
    if( ySizeInTypes == 0 )
    {
        (*matX) = C2DNumericDenseMatrix<Type>::Create( &m_Range.front(), &m_Table.front(),
                                                    GetClampValue() );
        (*matY) = NULL;
        return;
    }
    intVector ranges( 2 );
    ranges[0] = xSizeInTypes;
//    if( xSizeInTypes )
//    {
//        dataX.push_back(0);//add data to avoid exception for NULL pointer
//    }
    ranges[1] = 1;
    (*matX) = C2DNumericDenseMatrix<Type>::Create( &ranges.front(), &dataX.front());
    ranges[0] = ySizeInTypes;
//    if( ySizeInTypes == 0 )
//    {
//        dataY.push_back(0);//add data to avoid exception for NULL pointer
//    }
    (*matY) = C2DNumericDenseMatrix<Type>::Create( &ranges.front(), &dataY.front());
}

template <class Type>
void iC2DNumericDenseMatrix<Type>::icvSVBkSb(int m, int n, const Type* w,
                    const Type* uT, int lduT, int nu,
                    const Type* vT, int ldvT,
                    const Type* b, int ldb, int nb,
                    Type* x, Type* buffer)const
{
    PNL_THROW( CInvalidOperation, "we haven't any other types except float & double" )
}

template<class Type>
void iC2DNumericDenseMatrix<Type>::icvSVD( Type* a, int lda, int m, int n,
                 Type* w,
                 Type* uT, int lduT, int nu,
                 Type* vT, int ldvT,
                 Type* buffer )const
{
    PNL_THROW( CInvalidOperation, "we haven't any other types except float & double" )
    /*//provide all computations in double and convert the result to Type
    double *wD = new double[n];
    PNL_CHECK_IF_MEMORY_ALLOCATED( w );
    double *uTD = new double[m*n];
    PNL_CHECK_IF_MEMORY_ALLOCATED( uT );
    double *vTD = new double[m*n];
    PNL_CHECK_IF_MEMORY_ALLOCATED( vT );
    double * buffD = new double[2*n];
    PNL_CHECK_IF_MEMORY_ALLOCATED( buff );
    double *dataCopyD = new double[m*n];
    PNL_CHECK_IF_MEMORY_ALLOCATED( dataCopy );
    int i;
    for( i = 0; i < m*n; i++ )
    {
        dataCopyD[i] = static_cast<double>(dataCopy[i]);
    }
    icvSVD_64f( dataCopyD, n, n, n, wD, uTD, n, n, vTD, n, buffD );
    //fill resulting arrays
    */
}



template <class Type>
iC2DNumericDenseMatrix<Type>::iC2DNumericDenseMatrix( int dim,
                                 const int *range, const Type *data, int Clamp)
:CNumericDenseMatrix<Type>( dim, range, data, Clamp )
{
}

template <class Type>
iC2DNumericDenseMatrix<Type>::iC2DNumericDenseMatrix( 
                               const iCNumericDenseMatrix<Type> & inputMat )
                               :CNumericDenseMatrix<Type>( inputMat )
{
}

template <class Type>
Type iC2DNumericDenseMatrix<Type>::Trace() const
{    
    int ndims;
    const int *ranges;
    GetRanges( &ndims, &ranges);
    int indices[2] ={0, 0};
    Type trace = GetElementByIndexes(indices);
    
    int i;
    for( i = 1; i < ranges[0]; i++ )
    {
        indices[0] = i;
        indices[1] = i;
        trace += GetElementByIndexes(indices);
    }
    
    return trace;
}

#endif // !defined(SWIG) && (!defined(PNL_VC7) || defined(PNL_EXPORTS))

#ifdef PNL_RTTI
template <class Type>
const CPNLType iC2DNumericDenseMatrix< Type >::m_TypeInfo = CPNLType("iC2DNumericDenseMatrix", &(CNumericDenseMatrix< Type >::m_TypeInfo));

#endif

PNL_END

#endif //__PNLI2DNUMERICDENSEMATRIX_HPP__
