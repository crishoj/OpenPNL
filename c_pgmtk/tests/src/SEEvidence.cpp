/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      SEEvidence.cpp                                              //
//                                                                         //
//  Purpose:   Test on side effects on creation/deleting CEvidence objects //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <stdlib.h>
#include "tCreateRandStructs.hpp"
// this file must be last included file
#include "tUtil.h"

PNL_USING

static char func_name[] = "testEvidence";

static char* test_desc = "Create an Evidence for the Model";

static char* test_class = "SideEffects";

int testEvidence()
{
    int ret = TRS_OK;
    int nnodes = 0;
    int nObsNodes = 0;
    int i,j;
    while(nnodes <= 0)
    {
	trsiRead( &nnodes, "10", "Number of nodes in Model" );
    }
    while((nObsNodes <= 0)||(nObsNodes>nnodes))
    {
        trsiRead( &nObsNodes, "2", "Number of Observed nodes from all nodes in model");
    }
    int seed1 = pnlTestRandSeed();
    /*create string to display the value*/
    char value[42];

    sprintf(value, "%i", seed1);
    trsiRead(&seed1, value, "Seed for srand to define NodeTypes etc.");
    trsWrite(TW_CON|TW_RUN|TW_DEBUG|TW_LST, "seed for rand = %d\n", seed1);
    CNodeType *modelNodeType = new CNodeType[2];
    modelNodeType[0] = CNodeType( 1, 4 );
    modelNodeType[1] = CNodeType( 0, 3 );
    int *NodeAssociat=new int [nnodes+1];
    for(i=0; i<(nnodes+1)/2; i++)
    {
	NodeAssociat[2*i]=0;
	NodeAssociat[2*i+1]=1;
    }
    //create random graph - number of nodes for every node is rand too
    int lowBorder = nnodes - 1;
    int upperBorder = int((nnodes * (nnodes - 1)) / 2);
    int numEdges = rand()%(upperBorder - lowBorder)+lowBorder;
    CGraph* theGraph = tCreateRandomDAG( nnodes, numEdges, 1 );

    CBNet *grModel = CBNet::Create(nnodes, 2, modelNodeType, NodeAssociat,theGraph);
    int *obsNodes = (int*)trsGuardcAlloc(nObsNodes, sizeof(int));
    srand ((unsigned int)seed1);
    intVector residuaryNodes;
    for (i=0; i<nnodes; i++)
    {
	residuaryNodes.push_back(i);
    }
    int num = 0;
    valueVector Values;
    Values.reserve(3*nnodes);
    Value val;
    for (i = 0; i<nObsNodes; i++)
    {
	num = rand()%(nnodes-i);
	obsNodes[i] = residuaryNodes[num];
	residuaryNodes.erase(residuaryNodes.begin()+num);
	CNodeType nt = modelNodeType[NodeAssociat[obsNodes[i]]];
	if(nt.IsDiscrete())
	{
            val.SetInt(1);
            Values.push_back(val);
	}
	else
	{
	    val.SetFlt(1.0f);
            Values.push_back(val);
            val.SetFlt(2.0f);
            Values.push_back(val);
            val.SetFlt(3.0f);
            Values.push_back(val);
	}

    }
    residuaryNodes.clear();
    CEvidence *pMyEvid = CEvidence::Create(grModel,
	nObsNodes, obsNodes, Values) ;
    int nObsNodesFromEv = pMyEvid->GetNumberObsNodes();
    const int *pObsNodesNow = pMyEvid->GetObsNodesFlags();
    //	const int *myOffset = pMyEvid->GetOffset();
    const int *myNumAllObsNodes = pMyEvid->GetAllObsNodes();
    valueVector ev;
    pMyEvid->GetRawData(&ev);
    const Value* vall = pMyEvid->GetValue(obsNodes[0]);
    if( NodeAssociat[obsNodes[0]] == 0 )
    {
	if( (vall)[0].GetInt() != 1 )
	{
	    ret = TRS_FAIL;
	}
    }
    else
    {
	for( j=0; j<3; j++)
	{
	    if( (vall)[j].GetFlt() != (j+1)*1.0f )
	    {
		ret = TRS_FAIL;
		break;
	    }
	}
    }
    if(nObsNodesFromEv == nObsNodes)
    {
	intVector numbersOfReallyObsNodes;
	int numReallyObsNodes=0;
	for ( i=0; i<nObsNodesFromEv; i++)
	{
	    if (pObsNodesNow[i])
	    {
		numbersOfReallyObsNodes.push_back(myNumAllObsNodes[i]);
		numReallyObsNodes++;
	    }
	}
#if 0
	const CNodeType ** AllNodeTypesFromModel= new const CNodeType*[nnodes];
	for (i=0; i<nnodes; i++)
	{
	    AllNodeTypesFromModel[i] = grModel->GetNodeType(i);
	}
	for (i=0; i<nObsNodesFromEv; i++)
	{
	    //Test the values which are keep in Evidence
	    CNodeType nt = *AllNodeTypesFromModel[myNumAllObsNodes[i]];
	    int IsDiscreteNode = nt.IsDiscrete();
	    if(IsDiscreteNode)
	    {
		int valFromEv = (ev[myOffset[i]].GetInt());
		if(!(Values[i].GetInt() == valFromEv))
		{
		    ret=TRS_FAIL;
		    break;
		}
	    }
	    else
	    {
		;
		for (j=0; j<3; j++)
		{
		    if(!((ev[myOffset[i]+j]).GetFlt() == Values[i+j].GetFlt()))
		    {
			ret=TRS_FAIL;
			break;
		    }
		}
	    }
	}
	delete []AllNodeTypesFromModel;
#endif
    }
    else
    {
	ret = TRS_FAIL;
    }
    //Toggle some Node
    int someNumber = (int)(rand()*nObsNodesFromEv/RAND_MAX);
    int *someOfNodes = new int[someNumber];
    intVector residuaryNums = intVector(myNumAllObsNodes,
	myNumAllObsNodes+nObsNodesFromEv);
    num=0;
    for(i=0; i<someNumber;i++)
    {
	num = (int)(rand()%(nObsNodes-i));
	someOfNodes[i] = residuaryNums[num];
	residuaryNums.erase(residuaryNums.begin()+num);
    }
    residuaryNums.clear();
    pMyEvid->ToggleNodeState(someNumber, someOfNodes);
    const int *pObsNodesAfterToggle = pMyEvid->GetObsNodesFlags();
    for (i=0; i<nObsNodesFromEv; i++)
    {
	//Test the ToggleNode method...
	if(pObsNodesAfterToggle[i])
	{
	    for(j=0; j<someNumber;j++)
	    {
		if(myNumAllObsNodes[i]==someOfNodes[j])
		{
		    ret=TRS_FAIL;
		    break;
		}
	    }
	}
    }

    delete grModel;
    delete pMyEvid;
    delete []modelNodeType;
    delete []NodeAssociat;
    delete []someOfNodes;
    int obsNodes_memory_flag = trsGuardCheck( obsNodes );
    if( obsNodes_memory_flag)
    {
	return trsResult( TRS_FAIL, "Dirty memory");
    }
    trsGuardFree( obsNodes );
    return trsResult( ret, ret == TRS_OK ? "No errors" : "Bad test on Values");
}

void initSEEvidence()
{
    trsReg(func_name, test_desc, test_class, testEvidence);
}
