/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AMarginalize.cpp                                            //
//                                                                         //
//  Purpose:   Test on marginalization                                     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include "trsapi.h"
#include "pnl_dll.hpp"

PNL_USING

static char func_name[] = "testMarginalize";

static char* test_desc = "Initialize a marginalize";

static char* test_class = "Algorithm";

const float eps = 1e-6f;

int testMarginalize()
{
    int ret = TRS_OK;
    
    const int nnodes = 4;
    const int numnt = 2;
    
    intVector nodeAssociation = intVector( nnodes );
    
    nodeTypeVector nodeTypes;
    nodeTypes.assign( numnt, CNodeType() );
    
    nodeTypes[0].SetType(1, 2);
    nodeTypes[1].SetType(1, 3);
    
    nodeAssociation[0] = 0;
    nodeAssociation[1] = 0;
    nodeAssociation[2] = 0;
    nodeAssociation[3] = 1;
    
    CModelDomain* pMD = CModelDomain::Create( nodeTypes, nodeAssociation );
    
    
    int *domain = (int*)trsGuardcAlloc( nnodes, sizeof(int) );
    domain[0]=0; domain[1]=1; domain[2]=2; domain[3]=3;
    
    float* data = (float*)trsGuardcAlloc( 24, sizeof(float) );
    for (int i0=0; i0<24; data[i0]=i0*0.01f, i0++){};
    
    EMatrixType mType=matTable;
    
    CTabularPotential *pxParam1=CTabularPotential::Create( domain, nnodes, pMD); 
    
    pxParam1->AllocMatrix(data, mType);
    pxParam1->Dump();
    
    int domSize=2;
    int *pSmallDom = (int*)trsGuardcAlloc( domSize, sizeof(int) );
    pSmallDom[0] = 1;	pSmallDom[1] = 3;
    
    CFactor *pxParam2=pxParam1->Marginalize(pSmallDom, domSize, 0);
    //	CFactor *pxParam2=pxParam1->Marginalize(pSmallDom, 0, 0);	
    
    const CNumericDenseMatrix<float> *pxMatrix = static_cast<
        CNumericDenseMatrix<float>*>(pxParam2->GetMatrix(mType));
    
    const float* dmatrix1;
    //	const float* dmatrix1 = (const float*)trsGuardcAlloc( 1, sizeof(float) );
    int n;
    pxMatrix->GetRawData(&n, &dmatrix1);
    
    float *testdata1=new float[6];
    testdata1[0]=0.30f; testdata1[1]=0.34f; testdata1[2]=0.38f;
    testdata1[3]=0.54f; testdata1[4]=0.58f; testdata1[5]=0.62f;
    
    for(int i1 = 0; i1 < n; i1++)
    {	// Test the values...
        //printf("%d   %f %f", i1, dmatrix1[i1], testdata1[i1] );
        if(fabs(testdata1[i1] - dmatrix1[i1]) > eps)
        {
            return trsResult(TRS_FAIL, "data doesn't agree at max=0");
        }
    }
    
    CFactor *pxParam3=pxParam1->Marginalize(pSmallDom, domSize, 1);
    
    const CNumericDenseMatrix<float> *pxMatrix1 = static_cast<
        CNumericDenseMatrix<float>*>(pxParam3->GetMatrix(mType));
    
    const float *dmatrix2;
    pxMatrix1->GetRawData(&n, &dmatrix2);
    float *testdata2 = new float[6];
    testdata2[0]=0.15f;
    testdata2[1]=0.16f; testdata2[2]=0.17f;
    testdata2[3]=0.21f; testdata2[4]=0.22f; testdata2[5]=0.23f;
    
    for(int i2 = 0; i2 < 6; i2++)
    {	// Test the values...
        //	printf("%d   %f %f", i2, dmatrix2[i2], testdata2[i2]);
        if( fabs(dmatrix2[i2] - testdata2[i2]) > eps)
        {
            return trsResult(TRS_FAIL, "data doesn't agree at max=1");
        }
    }

    //we can check some methods of Tabular
    CTabularPotential* pUniPot = 
        CTabularPotential::CreateUnitFunctionDistribution( domain, nnodes,
        pMD, 1 );
    CTabularPotential* pCopyUniPot = static_cast<CTabularPotential*>(
        pUniPot->CloneWithSharedMatrices());
    CTabularPotential* pNormUniPot = static_cast<CTabularPotential*>(
        pUniPot->GetNormalized());
    pUniPot->Dump();
    pUniPot->ConvertToDense();
    pUniPot->ConvertToSparse();
    (*pCopyUniPot) = (*pCopyUniPot);
    pxParam1->AllocMatrix(data, matTable);
    intVector indices;
    indices.assign(4,0);
    pxParam1->GetMatrix(matTable)->SetElementByIndexes(-1.0f,&indices.front());
    //we've just damaged the potential
    std::string s;
    if( pxParam1->IsValid(&s) )
    {
        ret = TRS_FAIL;
    }
    else
    {
        std::cout<<s<<std::endl;
    }
    intVector pos;
    floatVector vals;
    intVector offsets;
    pUniPot->GetMultipliedDelta(&pos, &vals, &offsets);

    delete pNormUniPot;
    delete pCopyUniPot;
    delete pUniPot;
    
    delete pxParam1; 
    delete pxParam2; 
    delete pxParam3;
    delete testdata1; 
    delete testdata2;
    delete pMD;
    
    int data_memory_flag = trsGuardCheck( data );
    int domain_memory_flag = trsGuardCheck( domain );
    int Smalldomain_memory_flag = trsGuardCheck( pSmallDom );
    
    trsGuardFree( data );
    trsGuardFree( domain );
    trsGuardFree( pSmallDom );
    
    if( data_memory_flag || domain_memory_flag || Smalldomain_memory_flag )
    {
        return trsResult( TRS_FAIL, "Dirty memory");
    }
    
    return trsResult( ret, ret == TRS_OK ? "No errors" : "Marginalize FAILED");
}

void initAMarginalize()
{
    trsReg(func_name, test_desc, test_class, testMarginalize);
}