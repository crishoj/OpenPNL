/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AMultiplyVolume.cpp                                         //
//                                                                         //
//  Purpose:   Test on matrix multiplication rows-by-columns               //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <stdlib.h>

PNL_USING

static char func_name[] = "testMultiply for Matrix";

static char* test_desc = "Initialize a Multiply";

static char* test_class = "Algorithm";

float eps=1e-4f;

int testMultiplyMatrix()
{

    int ret = TRS_OK;
    int i;
#if 0
    int range_max=5;

    int seed1 = pnlTestRandSeed();
    /*create string to display the value*/
    char *value = new char[20];
    value = _itoa(seed1, value, 10);
    trsiRead(&seed1, value, "Seed for srand to define NodeTypes etc.");
    delete []value;
    trsWrite(TW_CON|TW_RUN|TW_DEBUG|TW_LST, "seed for rand = %d\n", seed1);

    srand ((unsigned int)seed1);

    int Nrow1 = 1+rand()%((int) range_max);
    int Ncol1 = 1+rand()%((int) range_max);
    int Nrow2=Ncol1;
    int Ncol2=1+rand()%((int) range_max);
#endif
    int Nrow1 = 4;
    int Ncol1 = 6;
    int Nrow2=Ncol1;
    int Ncol2=5;


    int* ranges1 = (int*)trsGuardcAlloc( 2, sizeof(int) );
    int* ranges2 = (int*)trsGuardcAlloc( 2, sizeof(int) );

    ranges1[0]=Nrow1; ranges1[1]=Ncol1;
    ranges2[0]=Nrow2; ranges2[1]=Ncol2;

    int data_length1=ranges1[0]*ranges1[1];
    int data_length2=ranges2[0]*ranges2[1];
    float* data1 = (float*)trsGuardcAlloc( data_length1, sizeof(float) );
    float* data2 = (float*)trsGuardcAlloc( data_length2, sizeof(float) );
    for (i = 0; i < data_length1; data1[i] = (div(i,Ncol1).quot+1)*1.0f, i++);
    for (i = 0; i < data_length2; data2[i] = (div(i,Ncol2).rem+1)*0.1f, i++);

    C2DNumericDenseMatrix<float>* m1 = C2DNumericDenseMatrix<float>::Create( ranges1, data1);
    C2DNumericDenseMatrix<float>* m2 = C2DNumericDenseMatrix<float>::Create( ranges2, data2);
    C2DNumericDenseMatrix<float>* m3 = pnlMultiply(m1,m2,0);
    int data_length3;
    const float *m3data;
    m3->GetRawData(&data_length3, &m3data);

    float *testdata0=new float[data_length3];
    int currow;int curcol;
    for (i = 0; i < data_length3; i++)
    {
	currow=div(i,Ncol2).quot+1;
	curcol=div(i,Ncol2).rem+1;
	testdata0[i] =currow*curcol*Ncol1*0.1f;
    }
    for(i = 0; i < data_length3; i++)
    {
	// Test the values...
	//printf("%3d  %4.2f  %4.2f\n",i, testdata0[i], m3data[i]);
	if(m3data[i] - testdata0[i]>eps)
	{
	    return trsResult(TRS_FAIL, "data doesn't agree at max=0, preorder");
	}
    }
    delete m3;
    m3 = pnlMultiply(m2,m1,0);
    m3->GetRawData(&data_length3, &m3data);

    for(i = 0; i < data_length3; i++)
    {
	// Test the values...
	//printf("%3d  %4.2f  %4.2f\n",i, testdata0[i], m3data[i]);
	if(m3data[i] - testdata0[i]>eps)
	{
	    return trsResult(TRS_FAIL, "data doesn't agree at max=0, postorder");
	}
    }

#if 0
    float *data4 = new float[data_length2];
    for (i=0;i<data_length2;i++)
    {
	currow=div(i,Ncol2).quot+1;
	curcol=div(i,Ncol2).rem+1;
	data4[i]=currow*(1e+curcol-3f);
    }
    CNumericDenseMatrix<float> * m4=CNumericDenseMatrix<float>::Create(2,ranges2, data4);
#endif
    delete m3;
    m3 = pnlMultiply(m1,m2,1);
    m3->GetRawData(&data_length3, &m3data);

    float *testdata1=new float[data_length3];
    for (i = 0; i < data_length3; i++)
    {
	int currow=div(i,Ncol2).quot+1;
	int curcol=div(i,Ncol2).rem+1;
	testdata1[i] =currow*curcol*0.1f;
    }
    for(i = 0; i < data_length3; i++)
    {
	// Test the values...
	//printf("%3d  %4.2f  %4.2f\n",i, testdata1[i], m3data[i]);
	if(m3data[i] - testdata1[i] > eps)
	{
	    return trsResult(TRS_FAIL, "data doesn't agree at max=1, preorder");
	}
    }
    delete m3;
    m3 = pnlMultiply(m2,m1,1);
    m3->GetRawData(&data_length3, &m3data);

    for(i = 0; i < data_length3; i++)
    {
	// Test the values...
	//printf("%3d  %4.2f  %4.2f\n",i, testdata1[i], m3data[i]);
	if(m3data[i] - testdata1[i]>eps)
	{
	    return trsResult(TRS_FAIL, "data doesn't agree at max=1, postorder");
	}
    }

    int ranges1_memory_flag = trsGuardCheck( ranges1 );
    int ranges2_memory_flag = trsGuardCheck( ranges2 );
    int data1_memory_flag = trsGuardCheck( data1 );
    int data2_memory_flag = trsGuardCheck( data2 );

    trsGuardFree( ranges1 );
    trsGuardFree( ranges2 );
    trsGuardFree( data1 );
    trsGuardFree( data2 );

    if(ranges1_memory_flag || ranges2_memory_flag || data1_memory_flag||
	data2_memory_flag)
    {
	return trsResult( TRS_FAIL, "Dirty memory");
    }

    delete m1; delete m2; delete m3;
    delete []testdata1;
    delete []testdata0;
    return trsResult( ret, ret == TRS_OK ? "No errors" : "Bad matrix data");
}

void initAMultiplyMatrix()
{
    trsReg(func_name, test_desc, test_class, testMultiplyMatrix);
}
