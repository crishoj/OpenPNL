/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AParameterTakeObsInto.cpp                                   //
//                                                                         //
//  Purpose:   Implementation of the algorithm                             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <stdlib.h>
#include <algorithm>

PNL_USING

static char func_name[] = "testShrinkObservedNodes";

static char* test_desc = "Create Factor and ShrinkObservedNodes for it.";

static char* test_class = "Algorithm";

int testShrinkObservedNodes()
{
    int i/*,j*/;
    int ret = TRS_OK;
    /*prepare to read the values from console*/
    EDistributionType dt;
    int disType = -1;
    EFactorType pt;
    int paramType = -1;
    /*read int disType corresponding DistributionType*/
    while((disType<0)||(disType>0))/*now we have only Tabulars&Gaussian*/
    {
	trsiRead( &disType, "0", "DistributionType");
    }
    /*read int paramType corresponding FactorType*/
    while((paramType<0)||(paramType>2))
    {
	trsiRead( &paramType, "0", "FactorType");
    }
    dt = EDistributionType(disType);
    pt = EFactorType(paramType);
    int numberOfNodes = 0;
    /*read number of nodes in Factor domain*/
    while(numberOfNodes<=0)
    {
	trsiRead( &numberOfNodes, "1", "Number of Nodes in domain");
    }
    int numNodeTypes = 0;
    /*read number of node types in model*/
    while(numNodeTypes<=0)
    {
	trsiRead( &numNodeTypes, "1", "Number of node types in Domain");
    }
    //int seed1 = pnlTestRandSeed()/*%100000*/;
    /*create string to display the value*/
    /*	char *value = new char[20];
    value = _itoa(seed1, value, 10);
    trsiRead(&seed1, value, "Seed for srand to define NodeTypes etc.");
    delete []value;
    trsWrite(TW_CON|TW_RUN|TW_DEBUG|TW_LST, "seed for rand = %d\n", seed1);
    int *domain = (int *)trsGuardcAlloc(numberOfNodes, sizeof(int));
    CNodeType * allNodeTypes = (CNodeType*)trsGuardcAlloc(numNodeTypes,
    sizeof(CNodeType));
    //To generate the NodeTypes we use rand()% and creates only Tabular now
    for(i=0; i<numNodeTypes; i++)
    {
    allNodeTypes[i] = CNodeType(1, 1+rand()%(numNodeTypes+3));
    }
    */	
    
    /*load data for parameter::ShrinkObservedNodes from console*/
    intVector domain;
    domain.assign( numberOfNodes, 0 );
    nodeTypeVector allNodeTypes;
    allNodeTypes.assign( numNodeTypes, CNodeType() );
    /*read node types*/
    for(i=0; i < numNodeTypes; i++)
    {
	int IsDiscrete = -1;
	int NodeSize = -1;
	while((IsDiscrete<0)||(IsDiscrete>1))
	    /*now we have tabular & Gaussian nodes!! */
	    trsiRead(&IsDiscrete, "1", "Is the node discrete?");
	while(NodeSize<0)
	    trsiRead(&NodeSize, "2", "NodeSize of node");
	allNodeTypes[i] = CNodeType( IsDiscrete != 0, NodeSize );
    }
    const CNodeType **nodeTypesOfDomain = (const CNodeType**)
	trsGuardcAlloc(numberOfNodes, sizeof(CNodeType*));
    int numData = 1;
    int *Ranges = (int*)trsGuardcAlloc(numberOfNodes, sizeof(int));
    /*associate nodes to node types*/
    for(i=0; i<numberOfNodes; i++)
    {
	domain[i] = i;
	int nodeAssociationToNodeType = -1;
	while((nodeAssociationToNodeType<0)||(nodeAssociationToNodeType>=
	    numNodeTypes))
	    trsiRead(&nodeAssociationToNodeType, "0", 
	    "node i has type nodeAssociationToNodeType");
	nodeTypesOfDomain[i] = &allNodeTypes[nodeAssociationToNodeType];
	//	nodeTypesOfDomain[i] = &allNodeTypes[rand()%numNodeTypes];
	Ranges[i] = nodeTypesOfDomain[i]->GetNodeSize();
	numData=numData*Ranges[i];
    }
    
    CModelDomain* pMD = CModelDomain::Create( allNodeTypes, domain );
    
    /*create factor according all information*/
    CFactor *pMyParam = NULL;
    float *data = (float *)trsGuardcAlloc(numData, sizeof(float));
    char *stringVal;/* = (char*)trsGuardcAlloc(50, sizeof(char));*/
    double val=0;
    /*read the values from console*/
    if(pt == ftPotential)
    {
	pMyParam = CTabularPotential::Create( &domain.front(), numberOfNodes, pMD );
	/*here we can create data by multiply on 0.1 - numbers are nonnormalized*/
	for(i=0; i<numData; i++)
	{
	    val = 0.1*i;
	    stringVal = trsDouble(val);
	    trsdRead(&val, stringVal, "value of i's data position");
	    data[i] = (float)val;
	    //data[i] = (float)rand()/1000;
	}
    }
    else
    {
    /*we can only read data from console - it must be normalized!!
	(according their dimensions) - or we can normalize it by function!*/
	if(pt == ftCPD)
	    pMyParam = CTabularCPD::Create( &domain.front(), numberOfNodes, pMD );
	for(i=0; i<numData; i++)
	{
	    val = -1;
	    while((val<0)||(val>1))
	    {
		trsdRead(&val, "-1", "value of (2*i)'s data position");
	    }
	    data[i] = (float)val;
	}
    }
    //trsWrite(TW_CON|TW_RUN|TW_DEBUG|TW_LST, "data for Factor = %d\n", data[i]);
    pMyParam->AllocMatrix(data,matTable);
    int nObsNodes = 0;	/*rand()%numberOfNodes;*/
    while((nObsNodes<=0)||(nObsNodes>numberOfNodes))
    {
	trsiRead(&nObsNodes, "1", "Number of Observed Nodes");
    }
    intVector myHelpForEvidence = intVector(domain.begin(), domain.end() );
    int *ObsNodes = (int *)trsGuardcAlloc(nObsNodes, sizeof(int));
    valueVector TabularValues;
    TabularValues.assign( nObsNodes, (Value)0 );
    char *strVal;
    for(i=0; i<nObsNodes; i++)
    {
	//fixme - we need to have noncopy only different ObsNodes
	/*		j = rand()%(numberOfNodes-i);*/
	int numberOfObsNode = -1;
	strVal = trsInt(i);
        intVector::iterator j = std::find( myHelpForEvidence.begin(), myHelpForEvidence.end(), numberOfObsNode );
	while((numberOfObsNode<0)||(numberOfObsNode>numberOfNodes)||
	    (j==myHelpForEvidence.end()))
	{
	    trsiRead(&numberOfObsNode, strVal,"Number of i's observed node");
	    j = std::find(myHelpForEvidence.begin(), myHelpForEvidence.end(),
		numberOfObsNode);
	}
	//ObsNodes[i] = myHelpForEvidence[j];
	myHelpForEvidence.erase( j );
	ObsNodes[i] = numberOfObsNode;
	int valueOfNode = -1;
	int maxValue = (*nodeTypesOfDomain[ObsNodes[i]]).GetNodeSize();
	while((valueOfNode<0)||(valueOfNode>=maxValue))
	{
	    trsiRead(&valueOfNode,"0","this is i's observed node value");
	}
	TabularValues[i].SetInt(valueOfNode);
	/*rand()%((*nodeTypesOfDomain[ObsNodes[i]]).pgmGetNodeSize());*/
    }
    CEvidence* pEvidence = CEvidence::Create( pMD, nObsNodes, ObsNodes, TabularValues );
    myHelpForEvidence.clear();
    CNodeType *ObservedNodeType = (CNodeType*)trsGuardcAlloc(1, 
	sizeof(CNodeType));
    *ObservedNodeType = CNodeType(1,1);
    CPotential *myTakedInFactor = static_cast<CPotential*>(pMyParam)->ShrinkObservedNodes(pEvidence);
    const int *myfactorDomain;
    int factorDomSize ;
    myTakedInFactor->GetDomain(&factorDomSize, &myfactorDomain);
#if 0
    CNumericDenseMatrix<float> *mySmallMatrix = static_cast<
        CNumericDenseMatrix<float>*>(myTakedInFactor->GetMatrix(matTable));
    int n;
    const float* mySmallData;
    mySmallMatrix->GetRawData(&n, &mySmallData);
    int nDims; // = mySmallMatrix->GetNumberDims();
    const int * mySmallRanges;
    mySmallMatrix->GetRanges(&nDims, &mySmallRanges);
    
    if(nDims!=numberOfNodes)
    {
	ret = TRS_FAIL;
	trsWrite(TW_CON|TW_RUN|TW_DEBUG|TW_LST, "nDims = %d\n", nDims);
    }
    else
    {
	int numSmallData = 1;
	for(i=0; i<nDims; i++)
	{
	    numSmallData = numSmallData*mySmallRanges[i];
	    trsWrite(TW_CON|TW_RUN|TW_DEBUG|TW_LST, "Range[%d] = %d\n", i, 
		mySmallRanges[i]);
	}
	for(i=0; i<numSmallData; i++)
	{	
	    trsWrite(TW_CON|TW_RUN|TW_DEBUG|TW_LST, "mySmallData[%d] = %f ",
		i, mySmallData[i]);
	}
    }
#endif
    //getchar();
    delete(myTakedInFactor);
    delete (pMyParam);
    delete pMD;
    //test gaussian parameter
    nodeTypeVector nTypes;
    nTypes.assign( 2, CNodeType() );
    nTypes[0] = CNodeType( 0, 2 );
    nTypes[1] = CNodeType( 0,1 );
    intVector domn = intVector(3,0);
    domn[1] = 1;
    domn[2] = 1;
    
    CModelDomain* pMD1 = CModelDomain::Create( nTypes, domn );
    
    domn[2] = 2;
    
    CPotential *BigFactor = CGaussianPotential::CreateUnitFunctionDistribution( 
	&domn.front(), domn.size(), pMD1,0 );
    float mean[] = { 1.0f, 3.2f};
    CPotential *SmallDelta = CGaussianPotential::CreateDeltaFunction( &domn.front(), 1, pMD1, mean, 1 );
    domn.resize( 2 );
    domn[0] = 1;
    domn[1] = 2;
    CPotential *SmallFunct = CGaussianPotential::Create( &domn.front(),
	domn.size(),  pMD1);
    float datH[] = { 1.1f, 2.2f, 3.3f };
    float datK[] = { 1.2f, 2.3f, 2.3f, 3.4f, 5.6f, 6.7f, 3.4f, 6.7f, 9.0f };
    SmallFunct->AllocMatrix( datH, matH );
    SmallFunct->AllocMatrix( datK, matK );
    static_cast<CGaussianPotential*>(SmallFunct)->SetCoefficient( 0.2f, 1 );
    CPotential* multFact = BigFactor->Multiply( SmallDelta );
    CPotential* nextMultFact = multFact->Multiply( SmallFunct );
    domn[0] = 0;
    domn[1] = 1;
    CPotential *marginalized = static_cast<CPotential*>(nextMultFact->Marginalize( &domn.front(), domn.size() ));
    int isSpecific = marginalized->IsDistributionSpecific();
    if( isSpecific )
    {
	trsWrite(TW_CON|TW_RUN|TW_DEBUG|TW_LST, "\nGaussian Distribution is specific");
    }
    delete BigFactor;
    delete SmallFunct;
    delete SmallDelta;
    delete pMD1;
    
    int ranges_memory_flag = trsGuardCheck(Ranges);
    int data_memory_flag = trsGuardCheck(data);
    int nodeTypesOfDomain_mem_b = trsGuardCheck(nodeTypesOfDomain);
    int ObsNodes_mem_b = trsGuardCheck(ObsNodes);
    int ObsNodeType_mem_b = trsGuardCheck(ObservedNodeType);
    if(((ranges_memory_flag)||(data_memory_flag)||
	(nodeTypesOfDomain_mem_b)||
	(ObsNodes_mem_b)||(ObsNodeType_mem_b)))
    {
	ret = TRS_FAIL;
	return trsResult( ret, ret == TRS_OK ? "No errors" : 
	"Bad test on ShrinkObservedNodes Method - memory");
    }
    else
    {
	trsGuardFree(ObservedNodeType);
	trsGuardFree(ObsNodes);
	trsGuardFree(nodeTypesOfDomain);
	trsGuardFree(data);
	trsGuardFree(Ranges);
    }			
    return trsResult( ret, ret == TRS_OK ? "No errors" : 
    "Bad test on ShrinkObservedNodes Method");
}

void initAShrinkObservedNodes()
{
    trsReg(func_name, test_desc, test_class, testShrinkObservedNodes);
}
