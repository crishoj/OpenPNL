/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AReduceDims.cpp                                             //
//                                                                         //
//  Purpose:   Test on reduce operation algorithm on dense matrix          //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include "trsapi.h"
#include "pnl_dll.hpp"

PNL_USING

static char func_name[] = "testReduceDims";

static char* test_desc = "Initialize a reducing_dims";

static char* test_class = "Algorithm";

const float eps = 1e-6f;

int testReduceDims()
{
    int ret = TRS_OK;

	int i;
	int dim = 4;
//	int nnodes = dim;
	int *ranges = new int[dim];
	ranges[0] = 2;
	ranges[1] = 2;
	ranges[2] = 2;
	ranges[3] = 3;
	int data_length = 24;
	float * data = (float*)trsGuardcAlloc( data_length, sizeof(int) );;
	for (i = 0; i < data_length; data[i] = i * 0.01f, i++){};
	CNumericDenseMatrix<float> *pxMatrix = CNumericDenseMatrix<float>::Create(dim, ranges, data);

	int domSize = 2;
	int *pSmallDom = (int*)trsGuardcAlloc( domSize, sizeof(int) );
	pSmallDom[0] = 1;	pSmallDom[1] = 3;

	int *TabularValues = (int *)trsGuardcAlloc(domSize, sizeof(int));
	int **pOffsets = (int **)trsGuardcAlloc(domSize, sizeof(int*));
/*	for(i = 0; i < dim; i++)
	{
		pOffsets[i] = NULL;
	}*/
	for (i = 0; i < domSize; i++)
	{
		int valueOfNode = 1;
		TabularValues[i] = valueOfNode;
		pOffsets[i] = &TabularValues[i];
	}
	
	CNumericDenseMatrix<float> *pNewMatrix0 = static_cast<
        CNumericDenseMatrix<float>*>(pxMatrix->ReduceOp(pSmallDom, domSize, 0));
	
	CNumericDenseMatrix<float> *pNewMatrix1 = static_cast<
        CNumericDenseMatrix<float>*>(pxMatrix->ReduceOp(pSmallDom, domSize, 1));
	
	CNumericDenseMatrix<float> *pNewMatrix2 = static_cast<
        CNumericDenseMatrix<float>*>(pxMatrix->ReduceOp(pSmallDom, domSize, 2,
        TabularValues));

	const float* dmatrix2 = (const float*)trsGuardcAlloc(4, sizeof(float));
	int n;
	pNewMatrix2->GetRawData(&n, &dmatrix2);

	float *testdata2 = new float[4];
	testdata2[0]=0.07f;
	testdata2[1]=0.10f;
	testdata2[2]=0.19f;
	testdata2[3]=0.22f;
	for(i = 0; i < 4; i++)
    {	// Test the values...
       // printf("%3d  %4.2f  %4.2f\n", i1, dmatrix2[i1], testdata2[i1] );
		if(fabs(testdata2[i] - dmatrix2[i]) > eps)
        {
            return trsResult(TRS_FAIL, "data doesn't agree at max=0");
		}
	}
	const float* dmatrix0 = (const float*)trsGuardcAlloc( 6, sizeof(float) );
	pNewMatrix0->GetRawData(&n, &dmatrix0);
	
	float *testdata0=new float[6];
	testdata0[0]=0.30f; testdata0[1]=0.34f; testdata0[2]=0.38f;
	testdata0[3]=0.54f; testdata0[4]=0.58f; testdata0[5]=0.62f;
	
	for( i = 0; i < 6; i++)
    {	// Test the values...
        //printf("%d   %f %f", i1, dmatrix0[i1], testdata0[i1] );
		if(fabs(testdata0[i] - dmatrix0[i]) > eps)
        {
            return trsResult(TRS_FAIL, "data doesn't agree at max=0");
		}
	}
		
    const float *dmatrix1;
	pNewMatrix1->GetRawData(&n, &dmatrix1);
	
	float *testdata1=new float[6];
	testdata1[0]=0.15f;	testdata1[1]=0.16f; testdata1[2]=0.17f;
	testdata1[3]=0.21f; testdata1[4]=0.22f; testdata1[5]=0.23f;
	
	for(i = 0; i < 6; i++)
    {	// Test the values...
		//printf("%d   %f %f", i2, dmatrix1[i2], testdata1[i2]);
		if( fabs(dmatrix1[i] - testdata1[i]) > eps)
        {
            return trsResult(TRS_FAIL, "data doesn't agree at max=1");
		}
    }

	delete []testdata0;
	delete []testdata1;
	delete []testdata2;
	/*
	float dataSm[8] = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f};
	int rangesSm[3] = {2, 2, 2};
	CNumericDenseMatrix<float> *matrixSm = CNumericDenseMatrix<float>::Create(3, rangesSm, dataSm );
	int dimsKeep[2] = {2, 1};
	CNumericDenseMatrix<float> *matRed = matrixSm->ReduceOp( dimsKeep, 2, 0 );
	*/
    int data_memory_flag = trsGuardCheck( data );
	int Smalldomain_memory_flag = trsGuardCheck( pSmallDom );
		
	trsGuardFree( data );
	trsGuardFree( pSmallDom );

	
    if( data_memory_flag || Smalldomain_memory_flag )
    {
        return trsResult( TRS_FAIL, "Dirty memory");
    }
	delete pxMatrix;
	delete pNewMatrix0; delete pNewMatrix1; delete pNewMatrix2;
	delete[]ranges;
//	delete TabularValues;
	trsGuardFree(pOffsets);
    return trsResult( ret, ret == TRS_OK ? "No errors" : "Marginalize FAILED");
}

void initAReduceDims()
{
    trsReg(func_name, test_desc, test_class, testReduceDims);
}