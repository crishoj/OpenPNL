/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AJTreeInfDBN.cpp                                            //
//                                                                         //
//  Purpose:   Test on 1.5 Slice Junction tree inference for DBN           //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include <float.h>
#include <assert.h>
#include "tCreateKjaerulffDBN.hpp"

// this file must be last included file
#include "tUtil.h"

PNL_USING

static char func_name[] = "testJTreeInfMixtureDBN";

static char* test_desc = "Provide 1.5 Slice Inf";

static char* test_class = "Algorithm";


int testJTreeInfMixtureDBN()
{
    int ret = TRS_OK;
    int seed = pnlTestRandSeed();

    std::cout<<"seed"<<seed<<std::endl;

    srand( seed );
    CDBN *pDBN = tCreateKjaerulffDBN();
    //create sparse copy ofDBN
    const CBNet* pBnet = static_cast<const CBNet*>(pDBN->GetStaticModel());
    CBNet* pSpBnet =  pBnet->ConvertToSparse();
    CDBN *pSpDBN = CDBN::Create(pSpBnet);

    /////////////////////////////////////////////////////////////////////////////
    int nTimeSlices = -1;
    while(nTimeSlices <= 0)
    {
	trsiRead (&nTimeSlices, "4", "Number of slices");
    }

    float eps = -1.0f;
    while( eps <= 0 )
    {
	trssRead( &eps, "1e-2f", "accuracy in test");
    }

    //the model is

    //               W(0) --->W(4)

    //                |        |

    //                |        |

    //               X(1) --> X(5)

    //              / |      / |

    //             /  |     /  |

    //         (2)Y-->Z(3)(6)Y-->Z(7)

    //  X - Tabular nodes, Z - gaussain mixture node

    // Y is mixture node

    const int numOfNds = 8; //number of nodes;

    int numOfNdTypes = 2;

    CNodeType *nodeTypes = new CNodeType[numOfNdTypes];

    nodeTypes[0] = CNodeType(1, 2);

    nodeTypes[1] = CNodeType(0, 1);

    int *nodeAssoc = new int [numOfNds];

    //int numOfNeigh[] = {2, 4, 2, 2, 2, 4, 2, 2};

     int numOfNeigh[] = {2, 3, 1, 2, 2, 3, 1, 2};
    int neigh0[] = {1, 4};

    //int neigh1[] = {0, 2, 3, 5};

     int neigh1[] = {0, 3, 5};

    //int neigh2[] = {1, 3};

    int neigh2[] = { 3};

    int neigh3[] = {1, 2};

    int neigh4[] = {0, 5};

    //int neigh5[] = {1, 4, 6, 7};
    int neigh5[] = {1, 4, 7};

    //int neigh6[] = {5, 7};

    int neigh6[] = { 7};

    int neigh7[] = {5, 6};

    ENeighborType orient0[] = { ntChild, ntChild};

    //ENeighborType orient1[] = { ntParent, ntChild, ntChild, ntChild };

    ENeighborType orient1[] = { ntParent, ntChild, ntChild };

//    ENeighborType orient2[] = { ntParent, ntChild };

    ENeighborType orient2[] = { ntChild };

    ENeighborType orient3[] = { ntParent, ntParent };

    ENeighborType orient4[] = { ntParent, ntChild };

    //ENeighborType orient5[] = { ntParent, ntParent, ntChild, ntChild };

    ENeighborType orient5[] = { ntParent, ntParent,  ntChild };

    //ENeighborType orient6[] = { ntParent, ntChild };

    ENeighborType orient6[] = { ntChild };

    ENeighborType orient7[] = { ntParent, ntParent };

    nodeAssoc[0] = 0;

    nodeAssoc[1] = 0;

    nodeAssoc[2] = 0;

    nodeAssoc[3] = 1;

    nodeAssoc[4] = 0;

    nodeAssoc[5] = 0;

    nodeAssoc[6] = 0;

    nodeAssoc[7] = 1;

    int *neigh[] = { neigh0, neigh1, neigh2, neigh3, neigh4, neigh5, neigh6, neigh7 };

    ENeighborType *orient[] = { orient0, orient1, orient2, orient3, orient4, orient5, orient6, orient7 };



    CGraph* pGraph = CGraph::Create( numOfNds, numOfNeigh, neigh, orient);

    CBNet *pBNet = CBNet::Create( numOfNds, numOfNdTypes, nodeTypes, nodeAssoc, pGraph );



    float table0[] = { 0.7f, 0.3f };//node X

    float table1[] = {0.79f, 0.21f, 0.65f, 0.35f};

    float table2[] = {0.5f, 0.5f, 0.5f, 0.5f};

    float table3[] = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};

    //float table1[] = { 0.1f, 0.9f };//node Y

    float mean100 = 1.0f, cov100 = 0.5f;//node Z for X = 0, Y = 0

    float mean110 = -5.0f, cov110 = 4.0f;//node Z for X = 1, Y = 0

    float mean101 = -3.0f, cov101 = 1.5f;//node Z for X = 0, Y = 1

    float mean111 = 2.0f, cov111 = 1.0f;//node Z for X = 1, Y = 1

    //create domains

    int *nodeNumbers = new int [numOfNds];

    int domain0[] = { 0 };

    int domain1[] = { 0, 1 };

    //int domain2[] = { 1, 2 };
    int domain2[] = { 2 };

    int domain3[] = { 1, 2, 3 };

    int domain4[] = { 0, 4 };

    int domain5[] = { 1, 4, 5 };

    //int domain6[] = { 5, 6 };
    int domain6[] = { 6 };
    int domain7[] = { 5, 6, 7 };

    int *domains[] = { domain0, domain1, domain2, domain3, domain4, domain5, domain6, domain7 };

    nodeNumbers[0] = 1;

    nodeNumbers[1] = 2;

    //nodeNumbers[2] = 2;

    nodeNumbers[2] = 1;

    nodeNumbers[3] = 3;

    nodeNumbers[4] = 2;

    nodeNumbers[5] = 3;

    //nodeNumbers[6] = 2;

    nodeNumbers[6] = 1;

    nodeNumbers[7] = 3;

    pnlVector<pConstNodeTypeVector > nt;

    nt.assign( numOfNds, pConstNodeTypeVector() );

    int i;
    for( i = 0; i < numOfNds; i++ )

    {

	int size = nodeNumbers[i];

	for( int j = 0; j < size; j++ )

	    nt[i].push_back(pBNet->GetNodeType(domains[i][j]));

    }

    //               W(0) --->W(4)

    //                |        |

    //                |        |

    //               X(1) --> X(5)

    //              / |      / |

    //             /  |     /  |

    //         (2)Y-->Z(3)(6)Y-->Z(7)



    pBNet->AllocFactors();

    CTabularCPD* CPD0 = CTabularCPD::Create( domains[0], nodeNumbers[0], pBNet->GetModelDomain(), table0 );

    pBNet->AttachFactor( CPD0 );

    CTabularCPD* CPD1 = CTabularCPD::Create(domains[1], nodeNumbers[1], pBNet->GetModelDomain(), table1 );

    pBNet->AttachFactor( CPD1 );

    CTabularCPD* CPD2 = CTabularCPD::Create( domains[2], nodeNumbers[2], pBNet->GetModelDomain(), table1 );

    pBNet->AttachFactor( CPD2 );

    CMixtureGaussianCPD* CPD3 = CMixtureGaussianCPD::Create( domains[3], nodeNumbers[3],pBNet->GetModelDomain(), table1 );

    intVector parVal;

    parVal.assign( 2, 0 );

    CPD3->AllocDistribution( &mean100, &cov100, 2.0f, NULL, &parVal.front() );

    parVal[1] = 1;

    CPD3->AllocDistribution( &mean101, &cov101, 1.0f, NULL, &parVal.front());

    parVal[0] = 1;

    CPD3->AllocDistribution( &mean111, &cov111, 1.0f, NULL, &parVal.front() );

    parVal[1] = 0;

    CPD3->AllocDistribution( &mean110, &cov110, 1.0f, NULL, &parVal.front() );

    pBNet->AttachFactor( CPD3 );

    CTabularCPD* CPD4 = CTabularCPD::Create( domains[4], nodeNumbers[4], pBNet->GetModelDomain(), table2 );

    pBNet->AttachFactor( CPD4 );

    CTabularCPD* CPD5 = CTabularCPD::Create( domains[5], nodeNumbers[5], pBNet->GetModelDomain(), table3 );

    pBNet->AttachFactor( CPD5 );

    CTabularCPD* CPD6 = CTabularCPD::Create( domains[6], nodeNumbers[6], pBNet->GetModelDomain(), table1);

    pBNet->AttachFactor( CPD6 );

    CMixtureGaussianCPD* CPD7 = CMixtureGaussianCPD::Create( domains[7], nodeNumbers[7],
	pBNet->GetModelDomain(), table1 );

    parVal.assign( 2, 0 );

    CPD7->AllocDistribution( &mean100, &cov100, 2.0f, NULL, &parVal.front() );

    parVal[1] = 1;

    CPD7->AllocDistribution( &mean101, &cov101, 1.0f, NULL, &parVal.front());

    parVal[0] = 1;

    CPD7->AllocDistribution( &mean111, &cov111, 1.0f, NULL, &parVal.front() );

    parVal[1] = 0;

    CPD7->AllocDistribution( &mean110, &cov110, 1.0f, NULL, &parVal.front() );

    pBNet->AttachFactor( CPD7 );



    CDBN *pArHMM = CDBN::Create( pBNet );
    CBNet *pUnrollBNet = static_cast<CBNet*>(pArHMM->UnrollDynamicModel(nTimeSlices));

    C1_5SliceJtreeInfEngine* pInfEng;

    pInfEng = C1_5SliceJtreeInfEngine::Create(pArHMM);

    CNaiveInfEngine* pUnrollInfEng;

     pUnrollInfEng = CNaiveInfEngine::Create(pUnrollBNet);

    //Number of time slices for unrolling


    //Crate evidence for every slice

    CEvidence** pEvidences;
    CEvidence* pUnrollEvidence;
    pEvidences = new CEvidence*[nTimeSlices];


    //Let node 1 is always observed


    intVector obsNodesUnrollNums(nTimeSlices);
    intVector obsNodesNums(1,3);
    valueVector obsVals(1);
    valueVector obsValsUnroll(nTimeSlices);
    for( i = 0; i < nTimeSlices; i++ )
    {
	float ft = rand()/10.0f;
	obsVals[0].SetFlt(ft);

	pEvidences[i] = CEvidence::Create( pArHMM->GetModelDomain(), obsNodesNums, obsVals );

	obsValsUnroll[i].SetFlt(ft);
	obsNodesUnrollNums[i] = obsNodesNums[0] + numOfNds/2*i;
    }


    pUnrollEvidence = CEvidence::Create( pUnrollBNet->GetModelDomain(),
	obsNodesUnrollNums, obsValsUnroll );

    pInfEng->DefineProcedure(ptSmoothing, nTimeSlices);

    pInfEng->EnterEvidence(pEvidences, nTimeSlices);

    pInfEng->Smoothing();


    pUnrollInfEng->EnterEvidence(pUnrollEvidence);

    intVector query(2, 0);
    intVector queryUnroll(2, 0);
    query[1] = 1;
    queryUnroll[1] = 1;
    pInfEng->MarginalNodes(&query.front(), query.size(), 0);
    pInfEng->GetQueryJPD()->Dump();


    pUnrollInfEng->MarginalNodes(&queryUnroll.front(), queryUnroll.size());
    pUnrollInfEng->GetQueryJPD()->Dump();

    query[0] = numOfNds/2;
    query[1] = numOfNds/2 + 1;
    int t;
    for( t = 1 ; t < nTimeSlices; t++)
    {
	pInfEng->MarginalNodes(&query.front(), query.size(), t);
	pInfEng->GetQueryJPD()->Dump();

	queryUnroll[0] += numOfNds/2;
	queryUnroll[1] += numOfNds/2;
	pUnrollInfEng->MarginalNodes(&queryUnroll.front(), queryUnroll.size());
	pUnrollInfEng->GetQueryJPD()->Dump();

    }

    int result = 1;


    if(!result)
    {
	ret = TRS_FAIL;
    }

    delete pInfEng;
    delete pUnrollInfEng;
    delete (pDBN);
    delete (pSpDBN);
    delete (pUnrollBNet);
    delete pUnrollEvidence;
    for( i = 0; i < nTimeSlices; i++ )
    {
        delete pEvidences[i];
    }
    delete pArHMM;
    delete []pEvidences;
    delete []nodeTypes;
    delete []nodeAssoc;
    delete []nodeNumbers;
    //delete pBNet;

    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    return trsResult( ret, ret == TRS_OK ? "No errors" :
    "Bad test on Unroll DBN");

}

void initAJTreeInfMixtureDBN()
{
    trsReg(func_name, test_desc, test_class, testJTreeInfMixtureDBN);
}
