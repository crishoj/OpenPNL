/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AStructureLearning.cpp                                      //
//                                                                         //
//  Purpose:   Test on structure learning of BNet                          //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnl_dll.hpp"
#include "pnlStaticStructLearnSEM.hpp"

PNL_USING

CBNet* Create1DMixBNet()
{
    const int nnodes = 11;
    const int numberOfNodeTypes = 2;
    
    int i;
    
    CGraph *pGraph = CGraph::Create(nnodes, NULL, NULL, NULL);
    pGraph->AddEdge(0,1,1);
    pGraph->AddEdge(0,2,1);
    pGraph->AddEdge(0,3,1);

    pGraph->AddEdge(1,4,1);
    pGraph->AddEdge(2,5,1);
    pGraph->AddEdge(3,6,1);

    pGraph->AddEdge(4,8,1);
    pGraph->AddEdge(5,8,1);
	pGraph->AddEdge(7,8,1);

    pGraph->AddEdge(5,10,1);
    pGraph->AddEdge(6,10,1);
	pGraph->AddEdge(9,10,1);

    CNodeType *nodeTypes = new CNodeType [2];
    
    nodeTypes[0].SetType(1, 2);
    nodeTypes[1].SetType(0, 1);

    int *nodeAssociation = new int[nnodes];
    
    for ( i = 0; i < 6; i++ )
    {
		nodeAssociation[i] = 0;
    }
	nodeAssociation[6] = 1;
	nodeAssociation[7] = 0;
	nodeAssociation[8] = 1;
	nodeAssociation[9] = 0;
	nodeAssociation[10] = 1;

    CBNet *pBNet = CBNet::Create( nnodes, numberOfNodeTypes, nodeTypes,
	nodeAssociation, pGraph );
    
    CModelDomain* pMD = pBNet->GetModelDomain();
    
    //number of parameters is the same as number of nodes - one CPD per node
    CFactor **myParams = new CFactor*[nnodes];
    int *nodeNumbers = new int [nnodes];
    
    int domain0[] = { 0 };
    nodeNumbers[0] =  1;
    int domain1[] = { 0, 1 };
    nodeNumbers[1] =  2;
    int domain2[] = { 0, 2 };
    nodeNumbers[2] =  2;
    int domain3[] = { 0, 3 };
    nodeNumbers[3] =  2;
    int domain4[] = { 1, 4 };
    nodeNumbers[4] =  2;
    int domain5[] = { 2, 5 };
    nodeNumbers[5] =  2;
    int domain6[] = { 3, 6 };
    nodeNumbers[6] =  2;
    int domain7[] = { 7 };
    nodeNumbers[7] =  1;
    int domain8[] = { 4, 5, 7, 8 };
    nodeNumbers[8] =  4;
    int domain9[] = { 9 };
    nodeNumbers[9] =  1;
    int domain10[] = { 5, 6, 9, 10 };
    nodeNumbers[10] = 4;


    int *domains[] = { domain0, domain1, domain2, domain3, domain4,
	domain5, domain6, domain7, domain8, domain9, domain10 };
   
    pBNet->AllocFactors();
    
    for( i = 0; i < 6; i++ )
    {
		myParams[i] = CTabularCPD::Create( domains[i], nodeNumbers[i], pMD);
    }

	float data7[] = {0.76f, 0.24f};
	float data9[] = {0.31f, 0.69f};
	pnlSeed(0);

	myParams[6] = CGaussianCPD::Create( domains[6], nodeNumbers[6], pMD );
	myParams[7] = CTabularCPD::Create( domains[7], nodeNumbers[7], pMD);
	myParams[8] = CMixtureGaussianCPD::Create( domains[8], nodeNumbers[8], pMD, data7 );
	myParams[9] = CTabularCPD::Create( domains[9], nodeNumbers[9], pMD);
	myParams[10] = CMixtureGaussianCPD::Create( domains[10], nodeNumbers[10], pMD, data9 );

    // data creation for all CPDs of the model
    float data0[] = {0.3f, 0.7f};
    float data1[] = {0.8f, 0.2f, 0.25f, 0.75f};
    float data2[] = {0.2f, 0.8f, 0.45f, 0.55f};
    float data3[] = {0.9f, 0.1f, 0.64f, 0.36f};
    float data4[] = {0.75f, 0.25f, 0.57f, 0.43f};
    float data5[] = {0.08f, 0.92f, 0.41f, 0.59f};
    float *data[] = { data0, data1, data2, data3, data4, data5};
    
    for( i = 0; i < 6; i++ )
    {
		myParams[i]->AllocMatrix(data[i], matTable);
		pBNet->AttachFactor(myParams[i]);
    }

	myParams[6]->CreateAllNecessaryMatrices(1);
	pBNet->AttachFactor(myParams[6]);

	myParams[7]->AllocMatrix(data7, matTable);
	pBNet->AttachFactor(myParams[7]);

	static_cast<CCondGaussianDistribFun*>(myParams[8]->GetDistribFun()) -> CreateDefaultMatrices(1);
	pBNet->AttachFactor(myParams[8]);

	myParams[9]->AllocMatrix(data9, matTable);
	pBNet->AttachFactor(myParams[9]);

	static_cast<CCondGaussianDistribFun*>(myParams[10]->GetDistribFun()) -> CreateDefaultMatrices(1);
	pBNet->AttachFactor(myParams[10]);

    delete[] nodeTypes;
    delete[] nodeAssociation;
	delete[] nodeNumbers;
    
    return pBNet;
}

CBNet* CreateInit1DMixBNet()
{
    const int nnodes = 11;
    const int numberOfNodeTypes = 2;
    
    int i;
    
    CGraph *pGraph = CGraph::Create(nnodes, NULL, NULL, NULL);
    pGraph->AddEdge(0,1,1);
    pGraph->AddEdge(0,2,1);
//    pGraph->AddEdge(0,3,1);

    pGraph->AddEdge(1,4,1);
    pGraph->AddEdge(2,5,1);
    pGraph->AddEdge(3,6,1);

    pGraph->AddEdge(4,8,1);
	pGraph->AddEdge(7,8,1);

    pGraph->AddEdge(6,10,1);
	pGraph->AddEdge(9,10,1);

    CNodeType *nodeTypes = new CNodeType [2];
    
    nodeTypes[0].SetType(1, 2);
    nodeTypes[1].SetType(0, 1);

    int *nodeAssociation = new int[nnodes];
    
    for ( i = 0; i < 6; i++ )
    {
		nodeAssociation[i] = 0;
    }
	nodeAssociation[6] = 1;
	nodeAssociation[7] = 0;
	nodeAssociation[8] = 1;
	nodeAssociation[9] = 0;
	nodeAssociation[10] = 1;

    CBNet *pBNet = CBNet::Create( nnodes, numberOfNodeTypes, nodeTypes,
	nodeAssociation, pGraph );
    
    CModelDomain* pMD = pBNet->GetModelDomain();
    
    //number of parameters is the same as number of nodes - one CPD per node
    CFactor **myParams = new CFactor*[nnodes];
    int *nodeNumbers = new int [nnodes];
    
    int domain0[] = { 0 };
    nodeNumbers[0] =  1;
    int domain1[] = { 0, 1 };
    nodeNumbers[1] =  2;
    int domain2[] = { 0, 2 };
    nodeNumbers[2] =  2;
    int domain3[] = { 3 };
    nodeNumbers[3] =  1;
    int domain4[] = { 1, 4 };
    nodeNumbers[4] =  2;
    int domain5[] = { 2, 5 };
    nodeNumbers[5] =  2;
    int domain6[] = { 3, 6 };
    nodeNumbers[6] =  2;
    int domain7[] = { 7 };
    nodeNumbers[7] =  1;
    int domain8[] = { 4, 7, 8 };
    nodeNumbers[8] =  3;
    int domain9[] = { 9 };
    nodeNumbers[9] =  1;
    int domain10[] = { 6, 9, 10 };
    nodeNumbers[10] = 3;


    int *domains[] = { domain0, domain1, domain2, domain3, domain4,
	domain5, domain6, domain7, domain8, domain9, domain10 };
   
    pBNet->AllocFactors();
    
    for( i = 0; i < 6; i++ )
    {
		myParams[i] = CTabularCPD::Create( domains[i], nodeNumbers[i], pMD);
    }

	float data7[] = {0.76f, 0.24f};
	float data9[] = {0.31f, 0.69f};
	//pnlSeed(0);

	myParams[6] = CGaussianCPD::Create( domains[6], nodeNumbers[6], pMD );
	myParams[7] = CTabularCPD::Create( domains[7], nodeNumbers[7], pMD);
	myParams[8] = CMixtureGaussianCPD::Create( domains[8], nodeNumbers[8], pMD, data7 );
	myParams[9] = CTabularCPD::Create( domains[9], nodeNumbers[9], pMD);
	myParams[10] = CMixtureGaussianCPD::Create( domains[10], nodeNumbers[10], pMD, data9 );

    // data creation for all CPDs of the model
    float data0[] = {0.3f, 0.7f};
    float data1[] = {0.8f, 0.2f, 0.25f, 0.75f};
    float data2[] = {0.2f, 0.8f, 0.45f, 0.55f};
    float data3[] = {0.9f, 0.1f/*, 0.64f, 0.36f*/};
    float data4[] = {0.75f, 0.25f, 0.57f, 0.43f};
    float data5[] = {0.08f, 0.92f, 0.41f, 0.59f};
    float *data[] = { data0, data1, data2, data3, data4, data5};
    
    for( i = 0; i < 6; i++ )
    {
		myParams[i]->AllocMatrix(data[i], matTable);
		pBNet->AttachFactor(myParams[i]);
    }

	myParams[6]->CreateAllNecessaryMatrices(1);
	pBNet->AttachFactor(myParams[6]);

	myParams[7]->AllocMatrix(data7, matTable);
	pBNet->AttachFactor(myParams[7]);

	static_cast<CCondGaussianDistribFun*>(myParams[8]->GetDistribFun()) -> CreateDefaultMatrices(1);
	pBNet->AttachFactor(myParams[8]);

	myParams[9]->AllocMatrix(data9, matTable);
	pBNet->AttachFactor(myParams[9]);

	static_cast<CCondGaussianDistribFun*>(myParams[10]->GetDistribFun()) -> CreateDefaultMatrices(1);
	pBNet->AttachFactor(myParams[10]);

    delete[] nodeTypes;
    delete[] nodeAssociation;
	delete[] nodeNumbers;
    
    return pBNet;
}

CBNet* CreateInitSimpleBNet()
{
    const int nnodes = 11;
    const int numberOfNodeTypes = 2;
    
    int i;
    
    CGraph *pGraph = CGraph::Create(nnodes, NULL, NULL, NULL);

	pGraph->AddEdge(7,8,1);
	pGraph->AddEdge(9,10,1);

    CNodeType *nodeTypes = new CNodeType [2];
    
    nodeTypes[0].SetType(1, 2);
    nodeTypes[1].SetType(0, 1);

    int *nodeAssociation = new int[nnodes];
    
    for ( i = 0; i < 6; i++ )
    {
		nodeAssociation[i] = 0;
    }
	nodeAssociation[6] = 1;
	nodeAssociation[7] = 0;
	nodeAssociation[8] = 1;
	nodeAssociation[9] = 0;
	nodeAssociation[10] = 1;

    CBNet *pBNet = CBNet::Create( nnodes, numberOfNodeTypes, nodeTypes,
	nodeAssociation, pGraph );
    
    CModelDomain* pMD = pBNet->GetModelDomain();
    
    //number of parameters is the same as number of nodes - one CPD per node
    CFactor **myParams = new CFactor*[nnodes];
    int *nodeNumbers = new int [nnodes];
    
    int domain0[] = { 0 };
    nodeNumbers[0] =  1;
    int domain1[] = { 1 };
    nodeNumbers[1] =  1;
    int domain2[] = { 2 };
    nodeNumbers[2] =  1;
    int domain3[] = { 3 };
    nodeNumbers[3] =  1;
    int domain4[] = { 4 };
    nodeNumbers[4] =  1;
    int domain5[] = { 5 };
    nodeNumbers[5] =  1;
    int domain6[] = { 6 };
    nodeNumbers[6] =  1;
    int domain7[] = { 7 };
    nodeNumbers[7] =  1;
    int domain8[] = { 7, 8 };
    nodeNumbers[8] =  2;
    int domain9[] = { 9 };
    nodeNumbers[9] =  1;
    int domain10[] = { 9, 10 };
    nodeNumbers[10] = 2;


    int *domains[] = { domain0, domain1, domain2, domain3, domain4,
	domain5, domain6, domain7, domain8, domain9, domain10 };
   
    pBNet->AllocFactors();
    
    for( i = 0; i < 6; i++ )
    {
		myParams[i] = CTabularCPD::Create( domains[i], nodeNumbers[i], pMD);
    }

	float data7[] = {0.76f, 0.24f};
	float data9[] = {0.31f, 0.69f};
	//pnlSeed(0);

	myParams[6] = CGaussianCPD::Create( domains[6], nodeNumbers[6], pMD );
	myParams[7] = CTabularCPD::Create( domains[7], nodeNumbers[7], pMD);
	myParams[8] = CMixtureGaussianCPD::Create( domains[8], nodeNumbers[8], pMD, data7 );
	myParams[9] = CTabularCPD::Create( domains[9], nodeNumbers[9], pMD);
	myParams[10] = CMixtureGaussianCPD::Create( domains[10], nodeNumbers[10], pMD, data9 );

    // data creation for all CPDs of the model
    float data0[] = {0.3f, 0.7f};
    float data1[] = {0.8f, 0.2f, 0.25f, 0.75f};
    float data2[] = {0.2f, 0.8f, 0.45f, 0.55f};
    float data3[] = {0.9f, 0.1f/*, 0.64f, 0.36f*/};
    float data4[] = {0.75f, 0.25f, 0.57f, 0.43f};
    float data5[] = {0.08f, 0.92f, 0.41f, 0.59f};
    float *data[] = { data0, data1, data2, data3, data4, data5};
    
    for( i = 0; i < 6; i++ )
    {
		myParams[i]->AllocMatrix(data[i], matTable);
		pBNet->AttachFactor(myParams[i]);
    }

	myParams[6]->CreateAllNecessaryMatrices(1);
	pBNet->AttachFactor(myParams[6]);

	myParams[7]->AllocMatrix(data7, matTable);
	pBNet->AttachFactor(myParams[7]);

	static_cast<CCondGaussianDistribFun*>(myParams[8]->GetDistribFun()) -> CreateDefaultMatrices(1);
	pBNet->AttachFactor(myParams[8]);

	myParams[9]->AllocMatrix(data9, matTable);
	pBNet->AttachFactor(myParams[9]);

	static_cast<CCondGaussianDistribFun*>(myParams[10]->GetDistribFun()) -> CreateDefaultMatrices(1);
	pBNet->AttachFactor(myParams[10]);

    delete[] nodeTypes;
    delete[] nodeAssociation;
	delete[] nodeNumbers;
    
    return pBNet;
}

int main()
{
    int nodes = 11;
	CBNet* pMix = Create1DMixBNet();
    pEvidencesVector evidences;
	int nEv = 1000;
	pnlSeed(0);
	pMix->GenerateSamples(&evidences, nEv);

	int i,k;
/*	for(i=0; i<nEv; i++)
	{
		k = (int)pnlRand(0, 5);
		(evidences[i])->MakeNodeHidden(k);
	}*/
	CBNet* pInit = CreateInitSimpleBNet();
	CStaticStructLearnSEM* pLearn = CStaticStructLearnSEM::Create(pInit, itStructLearnSEM, 3);
    pLearn -> SetData(nEv, &evidences.front());

    pLearn->Learn();
    CBNet *resBNet;
    resBNet = pLearn->GetCurrBNet();
    
    resBNet->GetGraph()->Dump();
    
    printf("\n Result Factors\n");
    for (i = 0; i < resBNet->GetGraph()->GetNumberOfNodes(); i++)
        resBNet->GetFactor(i)->GetDistribFun()->Dump();
    
	delete pLearn;
	delete pInit;
	delete pMix;
	for(i=0; i<nEv; i++)
		delete evidences[i];
	evidences.clear();
}
