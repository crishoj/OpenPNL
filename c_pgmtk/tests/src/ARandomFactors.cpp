/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AGaussianInference.cpp                                      //
//                                                                         //
//  Purpose:   Test on inference for Gaussian BNet model                   //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <math.h>
#include <float.h>
#include "tCreateRandStructs.hpp"
#include "tUtil.h"
PNL_USING

static char func_name[] = "testRandomFactors";

static char* test_desc = "Create random model with random factors and start inference for it ";

static char* test_class = "Algorithm";

int testRandomFactors()
{
    int ret = TRS_OK;
    
    int nnodes = 0;
    int i;
    while(nnodes <= 0)
    {
        trsiRead( &nnodes, "5", "Number of nodes in Model" );
    }
    //create node types
    int seed1 = pnlTestRandSeed();
    //create string to display the value
    char *value = new char[20];
#if 0
    value = _itoa(seed1, value, 10);
#else
    sprintf( value, "%d", seed1 );
#endif
    trsiRead(&seed1, value, "Seed for srand to define NodeTypes etc.");
    delete []value;
    trsWrite(TW_CON|TW_RUN|TW_DEBUG|TW_LST, "seed for rand = %d\n", seed1);
    //create 2 node types and model domain for them
    nodeTypeVector modelNodeType;
    modelNodeType.resize(2);
    modelNodeType[0] = CNodeType( 1, 4 );
    modelNodeType[1] = CNodeType( 1, 3 );
    intVector NodeAssociat;
    NodeAssociat.assign(nnodes, 0);
    for( i = 0; i < nnodes; i++ )
    {
        float rand = pnlRand( 0.0f, 1.0f );
        if( rand < 0.5f )
        {
            NodeAssociat[i] = 1;
        }
    }
    CModelDomain* pMDDiscr = CModelDomain::Create( modelNodeType,
        NodeAssociat );
    //create random graph - number of nodes for every node is rand too
    int lowBorder = nnodes - 1;
    int upperBorder = int((nnodes * (nnodes - 1))/2);
    int numEdges = pnlRand( lowBorder, upperBorder );
mark: 
    CGraph* pGraph = tCreateRandomDAG( nnodes, numEdges, 1 );
    if ( pGraph->NumberOfConnectivityComponents() != 1 )
    {
        delete pGraph;
        goto mark;
    }
    CBNet* pDiscrBNet = CBNet::CreateWithRandomMatrices( pGraph, pMDDiscr );
    //start jtree inference just for checking 
    //the model is valid for inference and all operations can be made
    CEvidence* pDiscrEmptyEvid = CEvidence::Create( pMDDiscr, 0, NULL, valueVector() );
    CJtreeInfEngine* pDiscrInf = CJtreeInfEngine::Create( pDiscrBNet );
    pDiscrInf->EnterEvidence( pDiscrEmptyEvid );
    const CPotential* pot = NULL;
    for( i = 0; i < nnodes; i++ )
    {
        intVector domain;
        pDiscrBNet->GetFactor(i)->GetDomain( &domain );
        pDiscrInf->MarginalNodes( &domain.front(), domain.size() );
        pot = pDiscrInf->GetQueryJPD();
    }
    //make copy of Graph for using with other models
    pGraph = CGraph::Copy( pDiscrBNet->GetGraph() );
    delete pDiscrInf;
    delete pDiscrBNet;
    delete pDiscrEmptyEvid;
    delete pMDDiscr;  

    //create gaussian model domain
    modelNodeType[0] = CNodeType( 0, 4 );
    modelNodeType[1] = CNodeType( 0, 2 );
    CModelDomain* pMDCont = CModelDomain::Create( modelNodeType,
        NodeAssociat );
    CBNet* pContBNet = CBNet::CreateWithRandomMatrices( pGraph, pMDCont );
    CEvidence* pContEmptyEvid = CEvidence::Create( pMDCont, 0, NULL, valueVector() );
    CNaiveInfEngine* pContInf = CNaiveInfEngine::Create( pContBNet );
    pContInf->EnterEvidence( pContEmptyEvid );
    for( i = 0; i < nnodes; i++ )
    {
        intVector domain;
        pContBNet->GetFactor(i)->GetDomain( &domain );
        pContInf->MarginalNodes( &domain.front(), domain.size() );
        pot = pContInf->GetQueryJPD();
    }

    pGraph = CGraph::Copy(pContBNet->GetGraph());
    delete pContInf;
    delete pContBNet;
    delete pContEmptyEvid;
    delete pMDCont;
    //find the node that haven't any parents 
    //and change its node type for it to create Conditional Gaussian CPD
    int numOfNodeWithoutParents = -1;
    intVector parents;
    parents.reserve(nnodes);
    for( i = 0; i < nnodes; i++ )
    {
        pGraph->GetParents( i, &parents );
        if( parents.size() == 0 )
        {
            numOfNodeWithoutParents = i;
            break;
        }
    }
    //change node type of this node, make it discrete
    CNodeType ntTab = CNodeType( 1,4 );
    modelNodeType.push_back( ntTab );
    NodeAssociat[numOfNodeWithoutParents] = 2;
    //need to change this model domain
    CModelDomain* pMDCondGau = CModelDomain::Create( modelNodeType, NodeAssociat );
    CBNet* pCondGauBNet = CBNet::CreateWithRandomMatrices( pGraph, pMDCondGau );
    //need to create evidence for all gaussian nodes
    intVector obsNodes;
    obsNodes.reserve(nnodes);
    int numGauVals = 0;
    for( i = 0; i < numOfNodeWithoutParents; i++ )
    {
        int GauSize = pMDCondGau->GetVariableType(i)->GetNodeSize();
        numGauVals += GauSize;
        obsNodes.push_back( i );
    }
    for( i = numOfNodeWithoutParents + 1; i < nnodes; i++ )
    {
        int GauSize = pMDCondGau->GetVariableType(i)->GetNodeSize();
        numGauVals += GauSize;
        obsNodes.push_back( i );
    }
    valueVector obsGauVals;
    obsGauVals.resize( numGauVals );
    floatVector obsGauValsFl;
    obsGauValsFl.resize( numGauVals);
    pnlRand( numGauVals, &obsGauValsFl.front(), -3.0f, 3.0f);
    //fill the valueVector
    for( i = 0; i < numGauVals; i++ )
    {
        obsGauVals[i].SetFlt(obsGauValsFl[i]);
    }
    CEvidence* pCondGauEvid = CEvidence::Create( pMDCondGau, obsNodes, obsGauVals );
    CJtreeInfEngine* pCondGauInf = CJtreeInfEngine::Create( pCondGauBNet );
    pCondGauInf->EnterEvidence( pCondGauEvid );
    pCondGauInf->MarginalNodes( &numOfNodeWithoutParents, 1 );
    pot = pCondGauInf->GetQueryJPD();
    pot->Dump();

    delete pCondGauInf;
    delete pCondGauBNet;
    delete pCondGauEvid;
    delete pMDCondGau;

    return trsResult( ret, ret == TRS_OK ? "No errors" : 
    "Bad test on RandomFactors");
}

void initARandomFactors()
{
    trsReg(func_name, test_desc, test_class, testRandomFactors);
}
