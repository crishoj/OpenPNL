/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      ASetStatistics.cpp                                          //
//                                                                         //
//  Purpose:   Test on factor estimation using safficient statistics       //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "trsapi.h"
#include "pnl_dll.hpp"
#include <stdlib.h>

#include <float.h>
#include <assert.h>

#include "tUtil.h"

PNL_USING
static char func_name[] = "testInfForRndBNet";

static char* test_desc = "Provide inference and learning for bnet";

static char* test_class = "Algorithm";



int testInfForRndBNet()
{
   
    int ret = TRS_OK;
    int seed = pnlTestRandSeed();
    pnlSeed( seed );
    std::cout<<"seed"<<seed<<std::endl;
    
    
    int nnodes = pnlRand(5, 30);
    int ndsType = pnlRand(0, 3);
    std::cout<<ndsType;
    int nContNds;
    switch(ndsType)
    {
    case 0:
	nContNds = 0;
    	break;
    case 1:
    	nContNds = nnodes;
	break;
    default:
	nContNds = pnlRand(0, nnodes);
    }
    
    std::cout<<"\n number of nodes nodes = "<<nnodes<<std::endl;
    std::cout<<"\n number of cont nodes = "<<nContNds<<std::endl;
    
    int discrSz = pnlRand(2, 3);
    int contSz = pnlRand(1, 2);
    
    std::cout<<"\n size of discrete nodes = "<<discrSz<<std::endl;
    std::cout<<"\n size of cont nodes = "<<contSz<<std::endl;

    CBNet *pBNet = NULL;
    while (!pBNet) 
    {
        pBNet= pnlExCreateRandomBNet( nnodes, 3, nContNds, discrSz, contSz );
    }
    const CModelDomain *pMD = pBNet->GetModelDomain();
    pBNet->GetGraph()->Dump();

    pEvidencesVector pEv;
    pBNet->GenerateSamples( &pEv, 1 );
    int i;
    for( i = 0; i < nnodes; i++ )
    {
	if( pMD->GetVariableType( i )->IsDiscrete())
	{
	    if( pnlRand(0, 3) )
	    {
		pEv[0]->MakeNodeHiddenBySerialNum(i);
	    }
	}
    }

    CJtreeInfEngine *pJTreeInf = CJtreeInfEngine::Create(pBNet);
    pJTreeInf->EnterEvidence( pEv.front() );

    intVector domain;
    int node = pnlRand( 0, nnodes - 1 );
    pBNet->GetGraph()->GetParents(node, &domain );
    domain.push_back(node);

    pJTreeInf->MarginalNodes( &domain.front(), domain.size(), 1 );

    

    delete pJTreeInf;
    delete pEv.front();
    delete pBNet;


    CDBN *pDBN = NULL;
    while(!pDBN)
    {    
	int nInterfNds = pnlRand(1, nnodes);
	pDBN = pnlExCreateRandomDBN( nnodes, 2, nInterfNds, nContNds, discrSz, contSz );
    }
    pMD = pDBN->GetModelDomain();
    
    
    
    CBKInfEngine *pBKInf;
    int type = pnlRand(0, 3 );
    switch(type) {
    case 0:
	pBKInf = CBKInfEngine::Create( pDBN, false );
	break;
    case 1:
	pBKInf = CBKInfEngine::Create( pDBN, true );
	break;
    default:
	{
	    intVector intNds;
	    pDBN->GetInterfaceNodes(&intNds);
	    std::random_shuffle(intNds.begin(), intNds.end());
	    intVecVector clusters;
	    int nClusters = pnlRand(1, intNds.size());
	    clusters.resize(nClusters);
	    for( i = 0; i < intNds.size(); i++ )
	    {
		(clusters[pnlRand(0, nClusters -1 )]).push_back(intNds[i]);
	    }
	    intVecVector resClust;
	    for( i = 0; i < nClusters; i++ )
	    {
		if( clusters[i].size())
		{
		    resClust.push_back(clusters[i]);
		}
	    }
	    pBKInf = CBKInfEngine::Create( pDBN, resClust );
	    
	}
    }
    int nSlices = pnlRand(1, 20);
    
    pEvidencesVecVector evidences;
    intVector slices(1, nSlices);
    
    pDBN->GenerateSamples( &evidences, slices );
    for( i = 0; i < nnodes; i++ )
    {
	if( pMD->GetVariableType( i )->IsDiscrete())
	{
	    int j;
	    for( j = 0; j <nSlices; j++ )
	    {
		if( pnlRand(0, 3) )
		{
		    (evidences[0])[j]->MakeNodeHiddenBySerialNum(i);
		}
	    }
	}
    }

    pBKInf->DefineProcedure(ptSmoothing, nSlices);
    pBKInf->EnterEvidence(&evidences[0].front(), nSlices);
    pBKInf->Smoothing();

    for( i = 0; i < nSlices; i++ )
    {
	delete (evidences[0])[i];
    }
    
    delete pBKInf;
    delete pDBN;

    //////////////////////////////////////////////////////////////////////////
   
    nnodes = 5;
    const int nEdges = 2;
    const int nCNds = 2;
    const int dSz = 2;
    const int cSz = 1;
    const int nSamples = 5000;
    
    pBNet = pnlExCreateRandomBNet( nnodes, nEdges, nCNds, dSz, cSz );
    pBNet->GetGraph()->Dump();
    for( i = 0; i < pBNet->GetNumberOfNodes(); i++ )
    {
	pBNet->GetFactor(i)->ConvertToSparse();
    }
    
    
    pEv.clear();
    pBNet->GenerateSamples( &pEv, nSamples );
    
    CEMLearningEngine *pLearn = CEMLearningEngine::Create(pBNet);
    pLearn->SetData(pEv.size(), &pEv.front());
    pLearn->Learn();
    
    
    delete pLearn;
    for( i = 0; i < nSamples - 1; i ++ )
    {
	delete pEv[i];
    }
    
    CEvidence *ev = pEv[nSamples-1];
    pMD = pBNet->GetModelDomain();
    float ll0 = pBNet->ComputeLogLik(ev);
    intVector query(nnodes, 0);
    for( i = 0; i < nnodes; i++ )
    {
	if( pMD->GetVariableType(i)->IsDiscrete() )
	{
	    ev->MakeNodeHidden(i);
	}
	query[i] = i;
    }

    ev->Dump();
    CGibbsWithAnnealingInfEngine *pGibbs;
    pGibbs = CGibbsWithAnnealingInfEngine::Create(pBNet);
    pGibbs->EnterEvidence(ev);
    pGibbs->MarginalNodes(query);
    const CEvidence * resEv = pGibbs->GetMPE();
    resEv->Dump();
    float ll1 = pBNet->ComputeLogLik(resEv);
    std::cout<<"\n logLik initial"<< ll0<<"; logLik current "<< ll1<<std::endl;
    delete ev;
    delete pGibbs;
    delete pBNet;


    
    return trsResult( ret, ret == TRS_OK ? "No errors" : 
    "Bad test inference for Rnd BNet");
    
    
}

void initAInfForRndBNet()
{
    trsReg(func_name, test_desc, test_class, testInfForRndBNet);
}