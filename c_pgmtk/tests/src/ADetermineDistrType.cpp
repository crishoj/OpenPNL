/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      ADetermineDistrType.cpp                                     //
//                                                                         //
//  Purpose:   Test on algorithm determine distribution type               //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <stdlib.h>
#include <algorithm>

static char func_name[] = "testDetermineDistributionType";

static char* test_desc = "Create some nodes, observed nodes and determine distribution type";

static char* test_class = "Algorithm";

PNL_USING

int testDetermineDistrType()
{
/*EDistributionType pnlDetermineDistributionType(int nAllNodes, 
int numberObsNodes, int *pObsNodesIndices,
	const CNodeType **pAllNodesTypes)*/
    

	int i;
	int ret = TRS_OK;
	EDistributionType dtGau, dtTab, dtMix;
	int numberOfNodes = 8;
	CNodeType *AllTypes = (CNodeType *)trsGuardcAlloc(2,
		sizeof(CNodeType));
	AllTypes[0] = CNodeType(1, 3);
	AllTypes[1] = CNodeType(0, 4);
	const CNodeType **AllNodeTypes = 
		(const CNodeType**)trsGuardcAlloc(8, sizeof(CNodeType*));
	for(i=0; i<4; i++)
	{
		/*nodes 0,2,4,6 - discrete, 1,3,5,7 - gaussian*/
		AllNodeTypes[2*i] = &AllTypes[0];
		AllNodeTypes[2*i+1] = &AllTypes[1];
	}
	/*try to create Gaussian  -  all hidden nodes are gaussian*/
	int NumberObsNodes = 4;
	int *pObsNodesForGauss = (int *)
		trsGuardcAlloc(NumberObsNodes, sizeof(int));
	int *pObsNodesForTabular = (int *)
		trsGuardcAlloc(NumberObsNodes, sizeof(int));
	int *pObsNodesForMixture = (int *)
		trsGuardcAlloc(NumberObsNodes, sizeof(int));
	for(i=0;i<NumberObsNodes; i++)
	{
	pObsNodesForGauss[i] = 2*i;/*all observed nodes are discrete 
	- all hidden are gaussian*/
	pObsNodesForTabular[i] = 2*i+1;/*all observed nodes are gaussian 
	- all hidden are discrete*/
	pObsNodesForMixture[i] = i;/*some of observed nodes are discrete, 
	some gaussian - some of hidden nodes are discrete, some gaussian*/
	}
	/*determine distribution type on corresponding evidence*/
	dtGau = pnlDetermineDistributionType( numberOfNodes, NumberObsNodes, 
								pObsNodesForGauss, AllNodeTypes);
	dtTab = pnlDetermineDistributionType( numberOfNodes, NumberObsNodes, 
								pObsNodesForTabular, AllNodeTypes);
	dtMix = pnlDetermineDistributionType( numberOfNodes, NumberObsNodes,
								pObsNodesForMixture, AllNodeTypes);
	/*check-up used memory*/
	int flagObsG = trsGuardCheck(pObsNodesForGauss);
	int flagObsT = trsGuardCheck(pObsNodesForTabular);
	int flagObsM = trsGuardCheck(pObsNodesForMixture);
	if((flagObsG)||(flagObsT)||(flagObsM))
	{
		ret = TRS_FAIL;
		return trsResult( ret, ret == TRS_OK ? "No errors" : 
		"DirtyMemory");
	}
	else
	{
		/*free used memory*/
		trsGuardFree(pObsNodesForGauss);
		trsGuardFree(pObsNodesForTabular);
		trsGuardFree(pObsNodesForMixture);
		if(dtGau!=dtGaussian)
		{
			ret = TRS_FAIL;
			return trsResult( ret, ret == TRS_OK ? "No errors" : 
			"Bad test on Gaussian");
		};
		if(dtTab!=dtTabular)
		{
			ret = TRS_FAIL;
			return trsResult( ret, ret == TRS_OK ? "No errors" : 
			"Bad test on Tabular");
		};
		if(dtMix!=dtCondGaussian)
		{
			ret = TRS_FAIL;
			return trsResult( ret, ret == TRS_OK ? "No errors" : 
			"Bad test on Conditional Gaussian");
		};
		return trsResult( ret, ret == TRS_OK ? "No errors" : 
		"Bad test on DetermineDistributionType");
	}
}
void initADetermineDistrType()
{
    trsReg(func_name, test_desc, test_class, testDetermineDistrType);
}
