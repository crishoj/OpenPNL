/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AUnrollDBN.cpp                                              //
//                                                                         //
//  Purpose:   Test on unrolling Dynamic bayesian network                  //
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

PNL_USING

static char func_name[] = "testUnrollDBN";

static char* test_desc = "Provide Unroll DBN for sparse and dense models and compare";

static char* test_class = "Algorithm";

int testUnrollDBN()
{

/*/////////////////////////////////////////////////////////////////////
 An example of Dynamic Bayesian Network
 Kjaerulff "dHugin: A computational system for dynamic time-sliced Bayesian networks",
 Intl. J. Forecasting 11:89-111, 1995.

 The intra structure is (all arcs point downwards)

  0 -> 1
   \  /
     2
     |
     3
    / \
   4   5
   \  /
     6
     |
     7

 The inter structure is 0->0, 3->3, 7->7

*//////////////////////////////////////////////////////////////////////	
	
	int ret = TRS_OK;
	int i, j;
	const int nnodes = 16;//Number of nodes
	int numNt = 1;//number of Node Types
	
	
	CNodeType *nodeTypes = new CNodeType [numNt];
		for( i=0; i < numNt; i++ )
	{
		nodeTypes[i] = CNodeType(1,2);//all nodes are discrete and binary
	}
	

	
	int nodeAssociation[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


	int numOfNeigh[] = { 3, 2, 3, 4, 2, 2, 3, 2, 3, 2, 3, 4, 2, 2, 3, 2};
	
    int neigh0[] = { 1, 2, 8 };
	int neigh1[] = { 0, 2 };
	int neigh2[] = { 0, 1, 3 };
	int neigh3[] = { 2, 4, 5, 11 };
	int neigh4[] = { 3, 6 };
	int neigh5[] = { 3, 6 };
	int neigh6[] = { 4, 5, 7 };
	int neigh7[] = { 6, 15 };

	int neigh8[] =  { 0, 9, 10 };
	int neigh9[] =  { 8, 10};
	int neigh10[] = { 8, 9, 11 };
	int neigh11[] = { 3, 10, 12, 13 };
	int neigh12[] = { 11, 14 };
	int neigh13[] = { 11, 14 };
	int neigh14[] = { 12, 13, 15 };
	int neigh15[] = { 7, 14 };
	
	ENeighborType orient0[] = { ntChild, ntChild, ntChild };
	ENeighborType orient1[] = { ntParent, ntChild };
	ENeighborType orient2[] = { ntParent, ntParent, ntChild };
	ENeighborType orient3[] = { ntParent, ntChild, ntChild, ntChild };
	ENeighborType orient4[] = { ntParent, ntChild };
	ENeighborType orient5[] = { ntParent, ntChild };
	ENeighborType orient6[] = { ntParent, ntParent, ntChild };
	ENeighborType orient7[] = { ntParent, ntChild };

	ENeighborType orient8[] = {  ntParent, ntChild, ntChild };
	ENeighborType orient9[] = {  ntParent, ntChild };
	ENeighborType orient10[] = { ntParent, ntParent, ntChild };
	ENeighborType orient11[] = { ntParent, ntParent, ntChild, ntChild };
	ENeighborType orient12[] = { ntParent, ntChild };
	ENeighborType orient13[] = { ntParent, ntChild };
	ENeighborType orient14[] = { ntParent, ntParent, ntChild };
	ENeighborType orient15[] = { ntParent, ntParent };
	
	int *neigh[] = { neigh0, neigh1, neigh2, neigh3, neigh4, neigh5, neigh6, neigh7,
		neigh8, neigh9, neigh10, neigh11, neigh12, neigh13, neigh14, neigh15};
	ENeighborType *orient[] = { orient0, orient1, orient2, orient3, orient4, orient5, orient6, orient7,
		orient8, orient9, orient10, orient11, orient12, orient13, orient14, orient15};

	 
	
	CGraph* Graph = CGraph::Create( nnodes, numOfNeigh, neigh,
		orient);
	
	CBNet *myBNet = CBNet::Create(nnodes, numNt, nodeTypes, 
		nodeAssociation, Graph );

    CModelDomain* pMD = myBNet->GetModelDomain();
	///////////////////////////////////////////////////////////////////////////
	
	//we creates every factor - it is CPD
	
	//number of factors is the same as number of nodes - one CPD per node
	CFactor **myParams = new CFactor*[nnodes];
	//to create factors we need to create their domains and CPD
	int **domains = (int **)trsGuardcAlloc(nnodes, sizeof(int *));
	int *domainsSizes = (int *)trsGuardcAlloc(nnodes, sizeof(int));
	int num = 0;
	int counter1 = 0;
	for(i=0; i < nnodes; i++)
	{
		num = 1;// to add the node - it is child in this domain
		for(j=0; j < numOfNeigh[i]; j++)
		{
			if((orient[i])[j]==0) 
			{
				num++;
			}
		}
		domainsSizes[i] = num;
		//alloc memory for every domain
		domains[i] = (int *)trsGuardcAlloc(num, sizeof(int));
		counter1 = 0;
		for(j=0; j< numOfNeigh[i]; j++)
		{
			if((orient[i])[j]==0) 
			{
				(domains[i])[counter1] = neigh[i][j];
				counter1++;
			}
		}
		domains[i][num-1] = i;
	}
	//compute Data sizes for every Factor
	int *dataSizes = (int *)trsGuardcAlloc(nnodes, sizeof(int));
	//create array of node sizes for Matlab
	double *NodeSizes = (double*)trsGuardcAlloc(nnodes, sizeof(double));
	for(i=0; i<nnodes; i++)
	{
		NodeSizes[i] = (double)(myBNet->GetNodeType(i))->GetNodeSize();
		dataSizes[i] = 1;
		for(j=0; j<domainsSizes[i]; j++)
		{
			dataSizes[i] = ((int)dataSizes[i])*(myBNet->GetNodeType((domains[i])
				[j]))->GetNodeSize();
		}
	}
	//create container for Factors
	myBNet->AllocFactors();
	//create CPD 
	//create array of data for every parameter
	float **Datas = (float**)trsGuardcAlloc(nnodes, sizeof(float*));
	for(i=0; i<nnodes; i++)
	{
		Datas[i] = (float *)trsGuardcAlloc(dataSizes[i], sizeof(float));
	}
	int domSize=0;
	//create temporary storage for normalized CPD
	//CTabularCPD* tempCPD = NULL;
	for(i=0; i<nnodes; i++)
	{
		domSize = domainsSizes[i];
		myParams[i] = CTabularCPD::Create( domains[i], domSize, pMD);
		int childSize = pMD->GetVariableType((domains[i])[domSize-1])->GetNodeSize();
		for(j=0; j<dataSizes[i]; j++)
		{
			if( !(j%childSize) )
			{
				Datas[i][j] = 1.0f;
			}
			else
			{
				Datas[i][j] = (float)(rand()%10);
			}
		}
		myParams[i]->AllocMatrix(Datas[i], matTable);
		((CTabularCPD*)myParams[i])->NormalizeCPD();
		
		myBNet->AttachFactor((CTabularCPD*)myParams[i]);
	}
	
	////////////////////////////////////////////////////////////////////////////
	
	
	int numberOfSlices = -1;
	while( numberOfSlices <= 0 )
	{
		trsiRead( &numberOfSlices, "3", "number of slices");
	}	

	int equal = 0;

    //create sparse bnet
    CBNet* pSparseBnet = myBNet->ConvertToSparse();

	CDynamicGraphicalModel *pDBN = CDBN::Create( myBNet);
	//create dynamic sparse bnet

    CDBN* pSparseDBN = CDBN::Create(pSparseBnet);
    	
	
	trsTimerStart(0);
	CBNet *pItogBnet = static_cast<CBNet*>(pDBN->UnrollDynamicModel(numberOfSlices));
	CBNet* pSparseUnroll = static_cast<CBNet*>(
        pSparseDBN->UnrollDynamicModel(numberOfSlices));
    double time = trsTimerSec(0);

	printf( "time of unrolling %f\n", time);
	int domainSizeDBN, domainSizeUnrollDBN;
	const int *domainDBN, *domainUnrollDBN;

    int sparseDomainSizeDBN, sparseDomainSizeUnrollDBN;
	const int *sparseDomainDBN, *sparseDomainUnrollDBN;

	CFactor *paramDBN, *paramUnrollDBN;
    CFactor *sparseParamDBN, *sparseParamUnrollDBN;
	const int *itogNodeAss;
	itogNodeAss = pItogBnet->GetNodeAssociations();
	std::cout<<"node associations for unrolled network\n";
	for(i = 0; i< pItogBnet->GetNumberOfNodes(); i++)
	{
		std::cout<<itogNodeAss[i]<<" ";
	}
	for(i = 0; i < nnodes/2; i++)
	{
        //compare dense
		paramDBN = myBNet-> GetFactor(i);
		paramUnrollDBN = pItogBnet->GetFactor(i);
        equal = paramDBN->IsFactorsDistribFunEqual(paramUnrollDBN,
            FLT_EPSILON);
		if(!equal)
		{
				ret = TRS_FAIL;
				break;
		}
        //compare sparse
        sparseParamDBN = pSparseBnet->GetFactor(i);
        sparseParamUnrollDBN = pSparseUnroll->GetFactor(i);
        equal = sparseParamDBN->IsFactorsDistribFunEqual(sparseParamUnrollDBN,
            FLT_EPSILON);
		if(!equal)
		{
				ret = TRS_FAIL;
				break;
		}
        //compare sparse with dense
        equal = paramDBN->IsFactorsDistribFunEqual(sparseParamDBN,
            FLT_EPSILON);
        if(!equal)
		{
				ret = TRS_FAIL;
				break;
		}
        equal = paramUnrollDBN->IsFactorsDistribFunEqual(sparseParamUnrollDBN,
            FLT_EPSILON);
        if(!equal)
		{
				ret = TRS_FAIL;
				break;
		}
        //compare dense domains
		paramDBN->GetDomain(&domainSizeDBN, &domainDBN);
		paramUnrollDBN->GetDomain(&domainSizeUnrollDBN, &domainUnrollDBN);
		
		if(( domainSizeDBN - domainSizeUnrollDBN ) )
		{
			ret = TRS_FAIL;
			break;
		}
		else
		{
			
			if( memcmp( domainDBN, domainUnrollDBN, sizeof(int)*domainSizeDBN ) )
			{
				ret = TRS_FAIL;
				break;
				
			}
			
		}
        //compare sparse domains
        sparseParamDBN->GetDomain(&domainSizeDBN, &domainDBN);
		sparseParamUnrollDBN->GetDomain(&domainSizeUnrollDBN, &domainUnrollDBN);
		
		if(( domainSizeDBN - domainSizeUnrollDBN ) )
		{
			ret = TRS_FAIL;
			break;
		}
		else
		{
			
			if( memcmp( domainDBN, domainUnrollDBN, sizeof(int)*domainSizeDBN ) )
			{
				ret = TRS_FAIL;
				break;
				
			}
			
		}
	}
	
	if( numberOfSlices > 1)
	{
	
		for(i = nnodes/2; i < nnodes; i++)
		{
			
			for( j = 0; j < numberOfSlices - 1; j++)
			{
				
				int number = i + nnodes/2*j;
			
                //compare dense
				paramDBN = pSparseBnet-> GetFactor(i);
				paramUnrollDBN = pSparseUnroll->GetFactor(number);
			

				paramDBN->GetDomain(&domainSizeDBN, &domainDBN);
				paramUnrollDBN->GetDomain(&domainSizeUnrollDBN,
                    &domainUnrollDBN);
				if( domainSizeDBN - domainSizeUnrollDBN  )
					{
					ret = TRS_FAIL;
					break;
				}
				else
				{
					for( int node = 0; node < domainSizeDBN; node++)
					{
						if( domainDBN[node] != (domainUnrollDBN[node] - nnodes/2*j) )
						{
							ret = TRS_FAIL;
							break;
							
						}
					}
				}
				equal = paramDBN-> IsFactorsDistribFunEqual(paramUnrollDBN, FLT_EPSILON);
				if(!equal)
				{
					ret = TRS_FAIL;
					break;
				}

                //compare sparse
                sparseParamDBN = pSparseBnet-> GetFactor(i);
				sparseParamUnrollDBN = pSparseUnroll->GetFactor(number);
			

				sparseParamDBN->GetDomain(&sparseDomainSizeDBN, &sparseDomainDBN);
				sparseParamUnrollDBN->GetDomain(&sparseDomainSizeUnrollDBN,
                    &sparseDomainUnrollDBN);
				if( sparseDomainSizeDBN - sparseDomainSizeUnrollDBN  )
					{
					ret = TRS_FAIL;
					break;
				}
				else
				{
					for( int node = 0; node < sparseDomainSizeDBN; node++)
					{
						if( sparseDomainDBN[node] != (
                            sparseDomainUnrollDBN[node] - nnodes/2*j) )
						{
							ret = TRS_FAIL;
							break;
							
						}
					}
				}
				equal = sparseParamDBN-> IsFactorsDistribFunEqual(
                    sparseParamUnrollDBN, FLT_EPSILON);
				if(!equal)
				{
					ret = TRS_FAIL;
					break;
				}
                //compare sparse and dense
                equal = sparseParamDBN-> IsFactorsDistribFunEqual(paramDBN,
                    FLT_EPSILON);
                if(!equal)
				{
					ret = TRS_FAIL;
					break;
				}
                equal = sparseParamUnrollDBN->IsFactorsDistribFunEqual(
                    paramUnrollDBN, FLT_EPSILON);
				if(!equal)
				{
					ret = TRS_FAIL;
					break;
				}


			}
			
		}
		
	}
	
	int flag[7];
	flag[0] = 0;
	for(i=0; i < nnodes; i++)
	{
		if(trsGuardCheck(Datas[i]))
		{
			flag[0] = trsGuardCheck(Datas[i]);
			break;
		}
	}
	flag[1] = trsGuardCheck(Datas);
	flag[2] = trsGuardCheck(NodeSizes);
	flag[3] = trsGuardCheck(dataSizes);
	flag[4] = 0;
	for( i=0; i < nnodes; i++ )
	{
		if(trsGuardCheck(domains[i]))
		{
			flag[4] = trsGuardCheck(domains[i]);
			break;
		}
	}
	flag[5] = trsGuardCheck(domains);
	flag[6] = trsGuardCheck(domainsSizes);

	int fl=0;
	for (i = 0; i < 7; i++)
	{
		fl = fl + ((flag[i] < 0) ? -flag[i]:flag[i]);
	}
	if(fl)
	{
		return trsResult( TRS_FAIL, "Dirty memory");
	}
	

	for(i=0; i < nnodes; i++)
	{
		trsGuardFree(Datas[i]);
	}
	trsGuardFree(Datas);
	trsGuardFree(NodeSizes);
	trsGuardFree(dataSizes);
	for( i=0; i < nnodes; i++)
	{
		trsGuardFree(domains[i]);
	}
	trsGuardFree(domains);
	trsGuardFree(domainsSizes);

	delete (myParams);
	delete []nodeTypes;

	delete (pDBN);
	delete (pItogBnet);

    delete pSparseUnroll;
    delete pSparseDBN;
	
	return trsResult( ret, ret == TRS_OK ? "No errors" : 
	"Bad test on Unroll DBN");
}

void initAUnrollDBN()
{
    trsReg(func_name, test_desc, test_class, testUnrollDBN);
}
