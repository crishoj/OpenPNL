/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      ANormalizeFactor.cpp                                        //
//                                                                         //
//  Purpose:   Test on some functionality of CPotential object             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "trsapi.h"
#include "pnl_dll.hpp"
// this file must be last included file
#include "tUtil.h"

PNL_USING

static char func_name[] = "testNormalizePotential";

static char* test_desc = "Initialize a Factor &normalize";

static char* test_class = "Algorithm";

const float eps = 1e-6f;

int testNormalizePotential()
{
    int ret = TRS_OK;
    int seed1 = pnlTestRandSeed();
    /*create string to display the value*/
    char *value = new char[20];
#if 0
    value = _itoa(seed1, value, 10);
#else
    sprintf( value, "%d", seed1 );
#endif
    trsiRead(&seed1, value, "Seed for srand to define NodeTypes etc.");
    delete []value;
    trsWrite(TW_CON|TW_RUN|TW_DEBUG|TW_LST, "seed for rand = %d\n", seed1);

    srand ((unsigned int)seed1);
    int a1 = 2+rand()%((int) 20);

    const int nnodes = a1;
    const int numnt = 2;
    intVector nodeAssociation;
    nodeAssociation.assign( nnodes, 0 );

    nodeTypeVector nodeTypes;
    nodeTypes.assign( numnt, CNodeType() );

    nodeTypes[0].SetType(1, 1);
    nodeTypes[1].SetType(1, 2);

    int * domain = (int*)trsGuardcAlloc( nnodes, sizeof(int) );
    int Nlength = 1;
    for (int i2=0; i2 < nnodes; i2++)
    {	int n = rand()%((int) numnt);
    nodeAssociation[i2] = n;
    domain[i2] = i2;
    Nlength = Nlength *(n+1);
    }
    CModelDomain* pMD = CModelDomain::Create( nodeTypes,nodeAssociation);

    float * data = (float*)trsGuardcAlloc( Nlength, sizeof(float) );
    for (int i0=0; i0 < Nlength; data[i0]=1, i0++){};

    EMatrixType mType=matTable;

    CTabularPotential *pxFactor = CTabularPotential::Create( domain, nnodes, pMD);

    pxFactor->AllocMatrix(data, mType);

    CPotential *pxFactor_res=pxFactor->GetNormalized() ;

    const CNumericDenseMatrix<float> *pxMatrix=static_cast<
	CNumericDenseMatrix<float>*>(pxFactor_res->GetMatrix(mType));

    const float* dmatrix1 = (const float*)trsGuardcAlloc( Nlength, sizeof(float) );
    int n;
    pxMatrix->GetRawData(&n, &dmatrix1);

    float *testdata = new float[Nlength];
    for (int k1=0; k1 < Nlength; testdata[k1]=1.0f/Nlength,  k1++);

    for(int j = 0; j < Nlength; j++)
    {	// Test the values...
	//printf("%3d   %4.2f   %4.2f\n", j,  dmatrix1[j], testdata[j] );
	if(fabs(testdata[j] - dmatrix1[j]) > eps )
	{
	    return trsResult(TRS_FAIL, "data doesn't agree at max=0");
	}
    }


    int data_memory_flag = trsGuardCheck( data);
    int domain_memory_flag = trsGuardCheck( domain );

    trsGuardFree( data );
    trsGuardFree( domain );

    if(data_memory_flag || domain_memory_flag )
    {
	return trsResult( TRS_FAIL, "Dirty memory");
    }
    delete [] testdata;
    delete pxFactor;
    delete pxFactor_res;
    delete pMD;

    return trsResult( ret, ret == TRS_OK ? "No errors" : "Normalize FAILED");
}

void initANormalizePotential()
{
    trsReg(func_name, test_desc, test_class, testNormalizePotential);
}