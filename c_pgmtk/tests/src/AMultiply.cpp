/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AMultiply.cpp                                               //
//                                                                         //
//  Purpose:   Test on matrix multiplication                               //
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

static char func_name[] = "testMultiply";

static char* test_desc = "Initialize a multiply";

static char* test_class = "Algorithm";

const float eps = 1e-3f;

int testMultiply()
{
    int ret = TRS_OK;
    const int nnodes = 10;
    const int numnt = 2;

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

    int a1 = 1+rand()%((int) nnodes);
    int a2 = 1+rand()%((int) (a1));

    intVector nodeAssociation = intVector(nnodes) ;

    nodeTypeVector nodeTypes = nodeTypeVector(numnt);

    nodeTypes[0].SetType(1, 2);
    nodeTypes[1].SetType(1, 3);

    int i=0;
    for (i=0 ; i < nnodes;  i++)
    {
	    nodeAssociation[i] = rand()%((int) numnt);
    }

    CModelDomain* pMD = CModelDomain::Create( nodeTypes, nodeAssociation );

    int bigDomsize=a1;
    int *bigDom = (int*)trsGuardcAlloc( bigDomsize, sizeof(int) );
    int Nlengthbig=1;
    for (i = 0; i < bigDomsize; i++)
    {
	    int n1 = nodeAssociation[i];
	    bigDom[i] = i;
	    Nlengthbig = Nlengthbig *(n1+2);
    }
    int smallDomsize=a2;
    int *smallDom = (int*)trsGuardcAlloc( smallDomsize, sizeof(int) );
    int Nlengthsmall=1;
    for (i = 0; i < smallDomsize; i++)
    {
	    smallDom[i] = i;
	    int n2 = nodeAssociation[i];
	    Nlengthsmall = Nlengthsmall * (n2+2);
    }



    const CNodeType **pNodeTypesConstr1 = new const CNodeType*[bigDomsize];
    for (i = 0; i < bigDomsize; i++)
    {
	    pNodeTypesConstr1[i]=(pMD->GetVariableType(bigDom[i]));
    }
    const CNodeType* *pNodeTypesConstr2=new const CNodeType*[smallDomsize];
    for (i = 0; i < smallDomsize; i++)
    {
	    pNodeTypesConstr2[i] = ( pMD->GetVariableType(smallDom[i]) );
    }

    float* data_big = (float*)trsGuardcAlloc( Nlengthbig, sizeof(float) );

    int i1;
    for (i1 = 0; i1 < Nlengthbig; data_big[i1] = 1.0f, i1++);

    float* data_small = (float*)trsGuardcAlloc( Nlengthsmall, sizeof(float) );

    for ( i1 = 0; i1 < Nlengthsmall; i1++)
    {
	    data_small[i1] = (i1*1.0f+1.0f)*1.0f;
    }

    CTabularPotential *pxFactor1 = CTabularPotential::Create(
	bigDom, bigDomsize, pMD);
    CTabularPotential *pxFactor2 = CTabularPotential::Create(
	smallDom, smallDomsize, pMD);

    pxFactor1->AllocMatrix(data_big, matTable);
    pxFactor2->AllocMatrix(data_small, matTable);

    CPotential *pxFactor_res1=pxFactor1->Multiply(pxFactor2);
    //(*pxFactor1) *= (*pxFactor2);

    const CNumericDenseMatrix<float> *pxMatrix1=static_cast<
	CNumericDenseMatrix<float>*>(pxFactor_res1->GetMatrix(matTable));

    //const CNumericDenseMatrix<float> *pxMatrixInSelf = pxFactor1->GetMatrix(matTable);

    //CNumericDenseMatrix<float> *matT = pnlCombineMatrices( pxMatrix1, pxMatrixInSelf, 0 );
    //float dif = matT->SumAll(1);
    //if( dif > eps )
    //{
    //		ret = TRS_FAIL;
    //	}

    const float* dmatrix1;
    int n;
    pxMatrix1->GetRawData(&n, &dmatrix1);

    int posRepetition = Nlengthbig/Nlengthsmall;
    int posRepetition1 = 1;

    for (i = bigDomsize-1; i >= smallDomsize; i--)
    {
	    posRepetition1 = posRepetition1 * (nodeAssociation[i]+2);
    }
    if ( posRepetition != posRepetition1 )
    {
	    ret = TRS_FAIL;
	    printf("posRepetition is not correct,%d/%d=%d\n range product = %d\n",
	        Nlengthbig, Nlengthsmall, posRepetition, posRepetition1);
    }
    float *testdata1=new float[Nlengthbig];
    div_t b;
    for (i = 0; i < Nlengthbig; i++)
    {
	    b=div(i, posRepetition);
	    int b0 = b.quot;
	    testdata1[i]= data_small[b0];
    }

    for( i = 0; i < Nlengthbig; i++)
    {	// Test the values...
        //std::cout<<"current index: "<<i <<"\tmult result:"<<dmatrix1[i]<<"\tprecomputed:"<<testdata1[i]<<std::endl;
	    if(fabs(testdata1[i] - dmatrix1[i]) > eps )
	    {
	        return trsResult(TRS_FAIL, "data doesn't agree at max=0");
	    }
    }


    CPotential *pxFactor_res2=pxFactor2->Multiply(pxFactor1);

    const CNumericDenseMatrix<float> *pxMatrix2 = static_cast<
	CNumericDenseMatrix<float>*>(pxFactor_res2->GetMatrix(matTable));

    const float* dmatrix2;
    pxMatrix2->GetRawData(&n, &dmatrix2);

    for(i = 0; i < Nlengthbig; i++)
    {	// Test the values...
	    //        printf("%3d  %4.2f  %4.2f\n", i,  dmatrix2[i], testdata1[i] );
        //std::cout<<"current index: "<<i <<"\tmult result: "<<dmatrix2[i]<<"\tprecomputed: "<<testdata1[i]<<std::endl;
	    if(fabs(testdata1[i] - dmatrix2[i]) > eps)
	    {
	        return trsResult(TRS_FAIL, "data doesn't agree at max=1");
	    }
    }

    int data_big_memory_flag = trsGuardCheck( data_big);
    int bigDom_memory_flag = trsGuardCheck( bigDom );
    int data_small_memory_flag=trsGuardCheck(data_small);
    int smallDom_memory_flag=trsGuardCheck(smallDom);

    if( data_big_memory_flag ||
	bigDom_memory_flag|| data_small_memory_flag||smallDom_memory_flag )
    {
	    return trsResult( TRS_FAIL, "Dirty memory");
    }
    trsGuardFree( data_big );
    trsGuardFree( bigDom );
    trsGuardFree(smallDom);
    trsGuardFree(data_small);
    delete []pNodeTypesConstr1;
    delete []pNodeTypesConstr2;
    delete pxFactor1;
    delete pxFactor2;
    delete pxFactor_res1;
    delete pxFactor_res2;
    delete pMD;
    delete [] testdata1;

    return trsResult( ret, ret == TRS_OK ? "No errors" : "Multiply FAILED");
}

void initAMultiply()
{
    trsReg(func_name, test_desc, test_class, testMultiply);
}