/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      SEParameter.cpp                                             //
//                                                                         //
//  Purpose:   Test on side effects on creation/deleting of CFactor objects//
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
// this file must be last included file
#include "tUtil.h"

PNL_USING

static char func_name[] = "testFactor";

static char* test_desc = "Create/delete Factor for the Model";

static char* test_class = "SideEffects";

int testFactor()
{
    int i;
    int ret = TRS_OK;
    EDistributionType dt;
    int disType = -1;
    EFactorType pt;
    int paramType = -1;
    while((disType<0)||(disType>0))//now we have only Tabulars
    {
	    trsiRead( &disType, "0", "DistributionType");
    }
    while((paramType<0)||(paramType>2)) //we have CPD & Potential only
    {
	    trsiRead( &paramType, "0", "FactorType");
    }
    dt = EDistributionType(disType);
    pt = EFactorType(paramType);
    int numberOfNodes = 0;
    //we can't create data for more than 15 nodes trsGuardcAlloc can't allocate memory
    while((numberOfNodes<=0)||(numberOfNodes>15))
    {
	    trsiRead( &numberOfNodes, "3", "Number of Nodes in domain");
    }
    int numNodeTypes = 0;
    while(numNodeTypes<=0)
    {
	    trsiRead( &numNodeTypes, "1", "Number of node types in Domain");
    }
    int seed1 = pnlTestRandSeed();
    /*create string to display the value*/
    char value[42];

    sprintf(value, "%i", seed1);
    trsiRead(&seed1, value, "Seed for srand to define NodeTypes etc.");
    trsWrite(TW_CON|TW_RUN|TW_DEBUG|TW_LST, "seed for rand = %d\n", seed1);
    intVector domain;
    domain.assign(numberOfNodes, 0);

    nodeTypeVector allNodeTypes;
    allNodeTypes.resize(numNodeTypes);
    //To generate the NodeTypes we use rand()% and creates only Tabular now
    for(i=0; i<numNodeTypes; i++)
    {
	    allNodeTypes[i] = CNodeType(1, 1+rand()%((int)(numNodeTypes+2)));
    }

    int numData = 1;
    int *Ranges = (int*)trsGuardcAlloc(numberOfNodes, sizeof(int));
    intVector nodeAssoc;
    nodeAssoc.resize(numberOfNodes);
    for(i=0; i<numberOfNodes; i++)
    {
	    domain[i] = i;
        nodeAssoc[i] = rand()%numNodeTypes;
	    Ranges[i] = allNodeTypes[nodeAssoc[i]].GetNodeSize();
	    numData=numData*Ranges[i];
    }

    CModelDomain* pMD = CModelDomain::Create( allNodeTypes, nodeAssoc );

    CFactor *pMyParam = NULL;
    CFactor *pOtherParam = NULL;
    if(( dt == dtTabular )&&( pt == ftPotential ))
    {
	    pMyParam = CTabularPotential::Create( &domain.front(), numberOfNodes, pMD );
	    pOtherParam = CTabularPotential::Create( &domain.front(), numberOfNodes, pMD );
    }
    else
    {
	    if(( dt == dtTabular )&&( pt == ftCPD ))
	    {
	        pMyParam = CTabularCPD::Create( &domain.front(), numberOfNodes, pMD );
	        pOtherParam = CTabularCPD::Create( &domain.front(), numberOfNodes, pMD );
	    }
    }
    float *data = (float *)trsGuardcAlloc(numData, sizeof(float));
    float *otherData = new float[numData];
    for( i = 0; i < numData; i++ )
    {
	data[i] = (float)rand()/1000;
	otherData[i] = (float)i;
    }
    pMyParam->AllocMatrix( data, matTable );
    pOtherParam->AllocMatrix( otherData, matTable );

    /*check methods GetDistributionType, GetFactorType, GetDomain, GetArgType*/
    EDistributionType myParamDt = pMyParam->GetDistributionType();
    if (myParamDt!=dt) ret = TRS_FAIL;
    EFactorType myParamPt = pMyParam->GetFactorType();
    if (myParamPt!=pt) ret = TRS_FAIL;
    int myParamDomSize;
    const int *myParamDomain;
    pMyParam->GetDomain(&myParamDomSize,&myParamDomain);
    const pConstNodeTypeVector *pMyParamArgType =
	pMyParam->GetArgType();
    if (myParamDomSize!=numberOfNodes) ret = TRS_FAIL;
    else
    {
	for (i=0; i<numberOfNodes; i++)
	{
	    if(myParamDomain[i]!=domain[i])
	    {
		    ret = TRS_FAIL;
		    break;
	    }
	    else
	    {
		    if((*(*pMyParamArgType)[i]) != allNodeTypes[nodeAssoc[i]])
		    {
		        ret = TRS_FAIL;
		        break;
		    }
	    }
	}
    }
    CNumericDenseMatrix<float> *myParamMatrix = static_cast<
        CNumericDenseMatrix<float>*>(pMyParam->GetMatrix(matTable));
    int n;
    const float *myParamData;
    myParamMatrix->GetRawData(&n, &myParamData);
    const pnl::floatVector* myParamVector = myParamMatrix->GetVector();
    int myNumParamDims;		// = myParamMatrix->GetNumberDims();
    const int *myParamRanges ;
    myParamMatrix->GetRanges(&myNumParamDims, &myParamRanges);
    /*check values of all data*/
    if(myNumParamDims!=numberOfNodes)
    {
	    ret=TRS_FAIL;
    }
    else
    {
	    int isAllTheSame=1;
	    for (i=0; i<numberOfNodes;i++)
	    {
	        if(myParamRanges[i]!=allNodeTypes[nodeAssoc[i]].GetNodeSize())
	        {
		        ret = TRS_FAIL;
		        isAllTheSame=0;
		        break;
	        }

	    }
	    if(isAllTheSame)
	    {
	        int numParamData = myParamVector->size();
	        if(numData!=numParamData)
	        {
		        ret=TRS_FAIL;
	        }
	        else
	        {
		        for(i=0; i<numData;i++)
		        {
		            if(data[i]!=myParamData[i])
		            {
			            ret = TRS_FAIL;
			            break;
		            }
		            else
		            {
			            if(data[i]!=(*myParamVector)[i])
			            {
			                ret = TRS_FAIL;
			                break;
			            }
		            }
		        }
	        }
	    }
    }
    float *newData = (float *)trsGuardcAlloc(numData, sizeof(float));
    for(i=0; i<numData; i++)
    {
	    newData[i] = (float)rand()/1000;
    }
    CNumericDenseMatrix<float> *myNewMatrix = 
        CNumericDenseMatrix<float>::Create(numberOfNodes,
	    Ranges, newData);
    pMyParam->AttachMatrix(myNewMatrix, matTable);
    /*check the values of Attach*/
    const CNumericDenseMatrix<float> *myNewParamMatrix = static_cast<
        CNumericDenseMatrix<float>*>(pMyParam->GetMatrix(matTable));
    const float *myNewParamData;
    myNewParamMatrix->GetRawData(&n, &myNewParamData);
    const pnl::floatVector* myNewParamVector =
	myNewParamMatrix->GetVector();
    int myNewNumParamDims;	// = myNewParamMatrix->GetNumberDims();
    const int *myNewParamRanges;
    myNewParamMatrix->GetRanges(&myNewNumParamDims, &myNewParamRanges);
    if(myNewNumParamDims!=numberOfNodes)
    {
	    ret=TRS_FAIL;
    }
    else
    {
	    int numParamData = myNewParamVector->size();
	    if(numData!=numParamData)
	    {
	        ret=TRS_FAIL;
	    }
	    else
	    {
	        for(i=0; i<numData;i++)
	        {
		        if(newData[i]!=myNewParamData[i])
		        {
		            ret = TRS_FAIL;
		            break;
		        }
		        else
		        {
		            if(newData[i]!=(*myNewParamVector)[i])
		            {
			        ret = TRS_FAIL;
			        break;
		            }
		        }
	        }
	    }
    }
    (*pMyParam) = (*pOtherParam);
    delete (pOtherParam);
    delete (pMyParam);
    delete []otherData;
    delete pMD;
    int ranges_memory_flag = trsGuardCheck(Ranges);
    int data_memory_flag = trsGuardCheck(data);
    int newData_memory_flag = trsGuardCheck(newData);
    if(((ranges_memory_flag)||(data_memory_flag)|| (newData_memory_flag)))
    {
	    ret = TRS_FAIL;
	    return trsResult( ret, ret == TRS_OK ? "No errors" :
	    "Bad test on Factor creation/destruction - memory");
    }
    else
    {
	    trsGuardFree(newData);
	    trsGuardFree(data);
	    trsGuardFree(Ranges);
	    return trsResult( ret, ret == TRS_OK ? "No errors" :
	    "Bad test on Factor creation/destruction");
    }
}

void initSEFactor()
{
    trsReg(func_name, test_desc, test_class, testFactor);
}
