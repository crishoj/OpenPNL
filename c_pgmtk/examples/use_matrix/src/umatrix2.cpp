/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      umatrix2.cpp                                                //
//                                                                         //
//  Purpose:   Showing of multidimentional matrix functionality            //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnl_dll.hpp"
#include "cvsvd.h"
//#include "cv.h"
PNL_USING
int main()
{
    const int nnodes = 4;
    const int numnt = 2;

    int i, j;

    EModelTypes type;
    //CStaticGraphicalModel *mn;

    nodeTypeVector nodeTypes;
    nodeTypes.resize(numnt);
    intVector nodeAssociation;
    nodeAssociation.assign(nnodes, 0);

    nodeTypes[0].SetType(1, 2);
    nodeTypes[1].SetType(1, 3);

    nodeAssociation[3] = 1;
    /*	nodeAssociation[5] = 1;
    */
    CModelDomain* pMD = CModelDomain::Create( nodeTypes, nodeAssociation );

    type = mtMNet;

    //	mn = CMNet::Create( nnodes, numnt, nodeTypes, nodeAssociation, NULL);
    //modelType, numberOfNodes, numberOfNodeTypes, nodeTypes, nodeAssociation, pGraph );

    /*	pNodeTypeVector * pNodeTypesConstr = new pNodeTypeVector(nnodes);
    */

    int *domain=new int[nnodes];
    domain[0]=0; domain[1]=1; domain[2]=2; domain[3]=3;

    float *data=new float[24];
    for (int i1=0; i1<24; data[i1]=i1*0.01f, i1++){};


    EMatrixType mType=matTable;
    CTabularPotential *pxParam1= CTabularPotential::Create( domain, nnodes, pMD);

    pxParam1->AllocMatrix(data, mType);

    int domSize=2;
    int *pSmallDom=new int[domSize];
    pSmallDom[0] = 1;
    pSmallDom[1] = 3;
    int maximize=0;

    pxParam1->Dump();
    CFactor *pxParam2=pxParam1->Marginalize(pSmallDom, domSize, maximize);

    const CMatrix<float> *pxMatrix = pxParam2->GetMatrix(mType);

    const float *dmatrix1;
    int Nlength=0;
    static_cast<const CNumericDenseMatrix<float>*>(pxMatrix)->
	GetRawData(&Nlength, &dmatrix1);
    printf("the result of marginalization with maximize= %d \n", maximize);
    printf("\n");
    printf("%4.2f %4.2f %4.2f\n%4.2f %4.2f %4.2f \n", dmatrix1[0],dmatrix1[1],
	dmatrix1[2], dmatrix1[3], dmatrix1[4], dmatrix1[5]);
    printf("\n");

    maximize=1;
    CFactor *pxParam3=pxParam1->Marginalize(pSmallDom, domSize, maximize);

    const CMatrix<float> *pxMatrix1=pxParam3->GetMatrix(mType);

    const float *dmatrix2;
    int Nlength1;
    static_cast<const CNumericDenseMatrix<float>*>(pxMatrix1)->
	GetRawData(&Nlength1, &dmatrix2);
    printf("the result of marginalization with maximize= %d \n", maximize);
    printf("\n");
    printf("%4.2f %4.2f %4.2f\n%4.2f %4.2f %4.2f\n", dmatrix2[0],dmatrix2[1],
	dmatrix2[2], dmatrix2[3], dmatrix2[4], dmatrix2[5]);
	/*	float det = pxMatrix1->Determinant();
	CMultiDMatrix *matInv = pxMatrix1->Inverse();
	CMultiDMatrix *matTransp = pxMatrix1->Transpose();
    */
    delete []domain;
    delete []data;
    delete []pSmallDom;
    delete pxParam3;
    delete pxParam2;
    delete pxParam1;
    //	delete mn;
    //add some to test Inverse, Transpose, Determinant
    float *dataNew = new float[16];
    int *ranges = new int[2];
    ranges[0] = 4;
    ranges[1] = 4;
    for( i = 0; i< 16; dataNew[i] = (i+1)*1.0f, i++ );
    dataNew[7] = 0;
    dataNew[12] = 0;
    //temp C2DMatrix* mat = C2DMatrix::Create( ranges, dataNew );
    C2DNumericDenseMatrix<float>* mat = C2DNumericDenseMatrix<float>::Create(
	ranges, dataNew );
    dataNew[2] = 0;
    //temp C2DMatrix *mat1 = C2DMatrix::Create( ranges, dataNew );
    C2DNumericDenseMatrix<float>* mat1 = C2DNumericDenseMatrix<float>::Create(
	ranges, dataNew );
    (*mat1) = (*mat);
    float val;
    printf( "matrix:\n" );

    CMatrixIterator<float>* iter1 = mat1->InitIterator();
    for( iter1; mat1->IsValueHere( iter1 ); mat1->Next(iter1) )
    {
	val = *(mat1->Value( iter1 ));
	printf( "%2.5f ", val );
    }
    delete iter1;
    printf( "matrix is:\n" );
    for ( i = 0; i < 16; i++ )
    {
	printf( "%2.2f ", dataNew[i] );
    }
    float det = mat->Determinant();
    printf("\n Determinant of it is: %f \n ", det);
    //temp CMultiDMatrix *invMat = mat->Inverse();
    C2DNumericDenseMatrix<float>* invMat = mat->Inverse();
    printf( "inverse matrix for it is:\n" );
    for ( i = 0; i < 16; i++ )
    {
	printf( "%2.2f ", invMat->GetElementByOffset(i) );
    }
    printf("\n");
    //temp CMultiDMatrix *transpmat = mat->Transpose();
    C2DNumericDenseMatrix<float>* transpmat = mat->Transpose();
    printf( "transpose matrix for it is:\n" );
    for ( i = 0; i < 16; i++ )
    {
	printf( "%2.2f ", transpmat->GetElementByOffset(i) );
    }
    printf("\n");
    intVector blockSizes = intVector(4,1);
    intVector X = intVector(2);
    X[0] = 1;
    X[1] = 3;
    printf( "we partition matrix into blocks; sizes of blocks = 1; X = {1,3}  \n" );
    //temp C2DMatrix *matX = NULL;
    //temp C2DMatrix *matY = NULL;
    //temp C2DMatrix *matXY = NULL;
    //temp C2DMatrix *matYX = NULL;
    C2DNumericDenseMatrix<float> *matX = NULL;
    C2DNumericDenseMatrix<float> *matY = NULL;
    C2DNumericDenseMatrix<float> *matXY = NULL;
    C2DNumericDenseMatrix<float> *matYX = NULL;
    mat->GetBlocks(&X.front(), X.size(), &blockSizes.front(), blockSizes.size(),
	&matX, &matY, &matXY, &matYX);
    const floatVector *vecX = matX->GetVector();
    const floatVector *vecY = matY->GetVector();
    const floatVector *vecXY = matXY->GetVector();
    const floatVector *vecYX = matYX->GetVector();
    printf( "X matrix for it is:\n" );
    for ( i = 0; i < 4; i++ )
    {
	printf( "%2.2f ", (*vecX)[i] );
    }
    printf( "\n" );
    printf( "Y matrix for it is:\n" );
    for ( i = 0; i < 4; i++ )
    {
	printf( "%2.2f ", (*vecY)[i] );
    }
    printf( "\n" );
    printf( "XY matrix for it is:\n" );
    for ( i = 0; i < 4; i++ )
    {
	printf( "%2.2f ", (*vecXY)[i] );
    }
    printf( "\n" );
    printf( "YX matrix for it is:\n" );
    for ( i = 0; i < 4; i++ )
    {
	printf( "%2.2f ", (*vecYX)[i] );
    }
    printf( "\n" );
    //check result of multiplying in 2 different situations


    //create sparse matrix
    CNumericSparseMatrix<float>* matSPf = CNumericSparseMatrix<float>::Create(
	2, ranges );
    val = 0.0f;
    intVector indices;
    indices.resize( 2 );
    for( i = 0; i < 4; i++ )
    {
	indices[0] = i;
	for( j = 0; j < 4; j++ )
	{
	    indices[1] = j;
	    val = 0.01f*( i*4 + j );
	    if( fabs(val) > 0.0f )
	    {
		matSPf->SetElementByIndexes( val, &indices.front() );
	    }
	}
    }
    printf( "my sparse matrix is:\n" );
    for( i = 0; i < 4; i++ )
    {
	indices[0] = i;
	for( j = 0; j < 4; j++ )
	{
	    indices[1] = j;
	    val = matSPf->GetElementByIndexes( &indices.front() );
	    printf( "%2.2f ", (val) );
	}
    }
    //reduce matrix
    int dim = 1;
    int ranges1[] = { 4, 4, 2 };
    CNumericSparseMatrix<float>* matSPOfZeros = CNumericSparseMatrix<float>::Create(
	3, ranges1 );
    intVector indicesToMult;
    indicesToMult.assign(2,0);
    indicesToMult[1] = 1;
    matSPOfZeros->MultiplyInSelf(matSPf, 2, &indicesToMult.front(), 1, 1.0f);
    intVector index1;
    index1.assign( 3, 0 );
    printf( "my multiplied sparse matrix without any data is:\n" );
    for( i = 0; i < 4; i++ )
    {
	index1[0] = i;
	for( j = 0; j < 4; j++ )
	{
	    index1[1] = j;
	    for( int k = 0; k < 2; k++ )
	    {
		index1[2] = k;
		val = matSPf->GetElementByIndexes( &index1.front() );
		printf( "%2.2f ", (val) );
	    }
	}
    }


    CNumericSparseMatrix<float>* redMat = static_cast<
	CNumericSparseMatrix<float>*>(matSPf->ReduceOp( &dim, dim, 0 ));
    int numDims;
    const int* dimSizes;
    redMat->GetRanges( &numDims, &dimSizes );
    if( numDims != 1 )
    {
	printf( "something wrong in reduce:\n" );
    }
    printf( "my reduced sparse matrix is:\n" );
    for( i = 0; i < dimSizes[0]; i++ )
    {
	val = redMat->GetElementByIndexes( &i );
	printf( "%2.2f ", (val) );
    }


    //multiply in self
    ranges[0] = 4;
    CNumericSparseMatrix<float>* smallMat = CNumericSparseMatrix<float>::Create(
	1, ranges );
    int index = 0;
    for( i = 0; i < dimSizes[0]; i++ )
    {
	val = (i+1)* 1.0f;
	index = i;
	smallMat->SetElementByIndexes( val, &index );
    }
    int dimMult = 1;
    CNumericSparseMatrix<float>* cloneMat = static_cast<
	CNumericSparseMatrix<float>*>(matSPf->Clone());
    cloneMat->MultiplyInSelf( smallMat, 1, &dimMult );
    printf( "\n my multiplied sparse matrix is:\n" );
    for( i = 0; i < 4; i++ )
    {
	indices[0] = i;
	for( j = 0; j < 4; j++ )
	{
	    indices[1] = j;
	    val = cloneMat->GetElementByIndexes( &indices.front() );
	    printf( "%2.2f ", (val) );
	}
    }
    CNumericSparseMatrix<float>* cloneOther = static_cast<
	CNumericSparseMatrix<float>*>(matSPf->Clone());
    cloneOther->DivideInSelf( smallMat, 1, &dimMult );
    printf( "my divided sparse matrix is:\n" );
    for( i = 0; i < 4; i++ )
    {
	indices[0] = i;
	for( j = 0; j < 4; j++ )
	{
	    indices[1] = j;
	    val = cloneOther->GetElementByIndexes( &indices.front() );
	    printf( "%2.2f ", (val) );
	}
    }
    intVector indexMax;
    cloneMat->GetIndicesOfMaxValue( &indexMax );
    printf( "my max indices are:\n" );
    int maxIndexSize = indexMax.size();
    for( i = 0; i < maxIndexSize; i++ )
    {
	printf( "%d ", indexMax[i] );
    }
    CNumericSparseMatrix<float>* matNorm = static_cast<
	CNumericSparseMatrix<float>*>(matSPf->NormalizeAll());
    printf( "matrix: \n" );
    CMatrixIterator<float>* iter = matNorm->InitIterator();
    for( iter; matNorm->IsValueHere( iter ); matNorm->Next(iter) )
    {
	val = *(matNorm->Value( iter ));
	printf( "%2.5f ", val );

    }
    delete iter;
    printf( "\n my normalized matrix is:\n" );
    for( i = 0; i < 4; i++ )
    {
	indices[0] = i;
	for( j = 0; j < 4; j++ )
	{
	    indices[1] = j;
	    val = matNorm->GetElementByIndexes( &indices.front() );
	    printf( "%2.5f ", (val) );
	}
    }

    matNorm->ClearData();
    getchar();

    delete matX;
    delete matY;
    delete matXY;
    delete matYX;
    delete []dataNew;
    delete mat;
    delete invMat;
    delete transpmat;
    delete []ranges;
    return 0;
}
