/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      ANormalizeCPD.cpp                                           //
//                                                                         //
//  Purpose:   Test on some functionality of CCPD object                   //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include <stdlib.h>
#include "trsapi.h"
#include "pnl_dll.hpp"
// this file must be last included file
#include "tUtil.h"

PNL_USING

using namespace std;

static char func_name[] = "testNormalizeCPD";

static char* test_desc = "Initialize a CPDnormalize";

static char* test_class = "Algorithm";

const float eps = 1e-6f;

int testNormalizeCPD()
{
    int ret = TRS_OK;
    int seed1 = pnlTestRandSeed();
    int i = 0;

    if(!trsiRead(&seed1, "1", "Seed for srand to define NodeTypes etc."))
    {
        seed1 = pnlTestRandSeed();
    }

    trsWrite(TW_CON|TW_RUN|TW_DEBUG|TW_LST, "seed for rand = %d\n", seed1);

    srand ((unsigned int)seed1);
    int a1 = 3 + (rand() % 9);

    const int nnodes = a1;

    intVecVector neighbors(nnodes) ;
    neighborTypeVecVector orientation(nnodes);

    int **neighb = ((int **)trsGuardcAlloc(nnodes, sizeof(const int*)));
    ENeighborType **orient = ((ENeighborType **)trsGuardcAlloc(nnodes,
        sizeof(const ENeighborType*)));

    int *numOfNeighb = (int *)trsGuardcAlloc(nnodes, sizeof(int));

    int child = nnodes - 1;

    for( i = 0; i < nnodes - 1; i++ )
    {
        //fill neighbors and orientation
        neighbors[i].push_back(child);
        orientation[i].push_back(ntChild);
        neighbors[child].push_back(i);
        orientation[child].push_back(ntParent);
    }

    for( i = 0; i < nnodes; i++ )
    {
        neighb[i] = &neighbors[i].front();
        orient[i] = &orientation[i].front();
        numOfNeighb[i] = neighbors[i].size();
    }

    CGraph* pGraph = CGraph::Create( nnodes, numOfNeighb, neighb,
        orient);

    //CGraph::CGraph( int numberOfNodes, const int *numberOfNeighbors,
    //                                    const int **neighborList, const int **edgeOrientation )

    const int numnt = 3;

    CNodeType *nodeTypes = new CNodeType [numnt];
    nodeTypes[0].SetType(1, 1);
    nodeTypes[1].SetType(1, 2);
    nodeTypes[2].SetType(1, 3);

    int *nodeAssociation = (int*)trsGuardcAlloc( nnodes, sizeof(int) );
    int i2;
    for (i2=0; i2 < nnodes; i2++)
    {   int n = rand()%((int) numnt);
    nodeAssociation[i2] = n;
    //          printf("nodeType[%d] = %d\n", i2, n + 1);
    }

    CBNet * bn1 = CBNet::Create( nnodes, numnt, nodeTypes, nodeAssociation,
        pGraph);
    bn1->GetGraph()->Dump();

    CModelDomain* pMD = bn1->GetModelDomain();

    int * domain = (int*)trsGuardcAlloc( nnodes , sizeof(int) );
    int Nlength = 1;
    int *ranges = new int[nnodes];
    for (i2 = 0; i2 < nnodes; i2++)
    {
        domain[i2] = i2;
        ranges[i2] = bn1->GetNodeType(i2)->GetNodeSize();
        Nlength = Nlength * ranges[i2];
        //              printf("range[%2d] =  %3d\n", i2, ranges[i2]);
    }

    float * data = (float*)trsGuardcAlloc( Nlength, sizeof(float) );
    for ( i=0; i < Nlength; data[i] = 1, i++){};

    CTabularCPD *pxCPD = CTabularCPD::Create( domain, nnodes, pMD) ;

    pxCPD->AllocMatrix(data, matTable);

    pxCPD->NormalizeCPD() ;

    //CCPD *pxCPD_res = pxCPD;

    const CNumericDenseMatrix<float> *pxMatrix=static_cast<CNumericDenseMatrix<
        float>*>(pxCPD->GetMatrix(matTable));

    const float* dmatrix1 = (const float*)trsGuardcAlloc( Nlength, sizeof(float) );
    int n;
    pxMatrix->GetRawData(&n, &dmatrix1);

    int child_range = ranges[nnodes - 1];
    float *testdata = new float[Nlength];
    int repeat = Nlength/child_range;
    for (i = 0; i < repeat ; i++)
    {
        for (int k1 = 0; k1 < ranges[nnodes - 1]; k1++ )
        {
            testdata[child_range*i + k1]=1.0f/child_range;
        }
    }
    for(int j = 0; j < Nlength; j++)
    {   // Test the values...
        //      printf("%3d   dmatrix = %4.2f   testdata =  %4.2f\n", j,  dmatrix1[j], testdata[j] );
        if(fabs(testdata[j] - dmatrix1[j]) > eps )
        {
            //return trsResult(TRS_FAIL, "data doesn't agree at max=0");
            ret = TRS_FAIL;
            printf("data doesn't match\n");
            //break;
        }
    }

    //add test for Tabular Unit Function
    CTabularCPD* unitF = CTabularCPD::CreateUnitFunctionCPD( domain, nnodes, pMD );
    CPotential * unitFPot = unitF->ConvertToPotential();
    //create random data to multiply

    float * dataRand = (float*)trsGuardcAlloc( Nlength, sizeof(float) );
    for ( i=0; i < Nlength; dataRand[i] = (float)(rand()%5), i++){};
    CPotential *randPot = CTabularPotential::Create(domain, nnodes - 1, pMD);
    randPot->AllocMatrix( dataRand, matTable );
    CPotential *multRes1 = unitFPot->Multiply( randPot );
    CPotential *divRes = unitFPot->Divide(randPot);
    (*unitFPot) *= (*randPot);
    (*unitFPot) /= (*randPot);

    int nodeAssociation_memory_flag = trsGuardCheck( nodeAssociation );
    int data_memory_flag = trsGuardCheck( data );
    int dataRand_memory_flag = trsGuardCheck( dataRand );
    int domain_memory_flag = trsGuardCheck( domain );
    int neiFlag = trsGuardCheck( neighb );
    int oriFlag = trsGuardCheck( orient );

    trsGuardFree( dataRand );
    trsGuardFree( nodeAssociation);
    trsGuardFree( data );
    trsGuardFree( domain );
    trsGuardFree(neighb);
    trsGuardFree(orient);

    if(nodeAssociation_memory_flag|| data_memory_flag || domain_memory_flag||
        neiFlag||oriFlag|| dataRand_memory_flag )
    {
        return trsResult( TRS_FAIL, "Dirty memory");
    }
    delete divRes;
    delete multRes1;
    delete unitFPot;
    delete unitF;
    delete pxCPD;
    delete bn1;
    //delete pGraph;
    delete [] ranges;
    delete []testdata;
    delete []nodeTypes;
    return trsResult( ret, ret == TRS_OK ? "No errors" : "NormalizeCPD FAILED");
}

void initANormalizeCPD()
{
    trsReg(func_name, test_desc, test_class, testNormalizeCPD);
}
