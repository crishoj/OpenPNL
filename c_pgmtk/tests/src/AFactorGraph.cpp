/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AFactorGraph.cpp                                            //
//                                                                         //
//  Purpose:   Test on factor graph object and its functionality,          //
//         inference and so on                                             //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include "trsapi.h"
#include "pnl_dll.hpp"
#include "tUtil.h"

PNL_USING

static char func_name[]  = "testFactorGraph";

static char test_desc[]  = "Create a factor graph, start inference and compare results";

static char test_class[] = "Algorithm";


void SetRndObsNdsAndVals( const CModelDomain* pModelDomain, intVector* obsNds,
                          valueVector* obsNdsVals )
{
    assert( pModelDomain != NULL );
    assert( obsNds       != NULL );
    assert( obsNdsVals   != NULL );
    assert( obsNds->size() <= pModelDomain->GetNumberVariables() );
    assert( obsNds->size() == obsNdsVals->size() );
    
    srand( pnlTestRandSeed() );

    int i = 0;

    const int numOfObsNds = obsNds->size();
    
    int numVars = pModelDomain->GetNumberVariables();
    
    intVector residuaryNodesFor;

    residuaryNodesFor.assign( numVars, 0 );
    
    for( i = 0; i < numVars; ++i )
    {
        residuaryNodesFor[i] = i;
    }
    
    int j;

    for( i = 0; i < numOfObsNds; i++)
    {
        j = rand() % ( numVars - i );
        
        (*obsNds)[i] = residuaryNodesFor[j];

        residuaryNodesFor.erase(residuaryNodesFor.begin() + j);
        
        const CNodeType* nodeType = pModelDomain->GetVariableType((*obsNds)[i]);

        if( nodeType->IsDiscrete() )
        {
            (*obsNdsVals)[i].SetInt(rand() % nodeType->GetNodeSize());
        }
        else
        {
            (*obsNdsVals)[i].SetFlt((float)( rand() % 4 + 12.345 ));
        }
    }
}

int testFactorGraph()
{
    int    ret = TRS_OK;

    float eps = 1e-3f;

    float tolerance = 1e-6f;

    //create model as result of convert from BNet
    
    CBNet* pBNet = pnl::pnlExCreateAsiaBNet();//pnl::pnlExCreateBatNetwork();//
    
    const int numOfNds = pBNet->GetNumberOfNodes();
    
    const int numOfObsNds = 5;
    
    assert( numOfObsNds <= numOfNds );
    
    CModelDomain* pModelDomain = pBNet->GetModelDomain();
    
    CFactorGraph* pFactorGraph = CFactorGraph::ConvertFromBNet(pBNet);
    
    intVector   obsNds(numOfObsNds);
    
    valueVector obsNdsVals(numOfObsNds);
    
    SetRndObsNdsAndVals( pModelDomain, &obsNds, &obsNdsVals );
    
    CEvidence* pEvidence = CEvidence::Create( pModelDomain, obsNds,
        obsNdsVals );
    
    CJtreeInfEngine*    pPearlInf = CJtreeInfEngine::Create(pBNet);
    pPearlInf->EnterEvidence(pEvidence);
    
    CFGSumMaxInfEngine* pFGInf    = CFGSumMaxInfEngine::Create(pFactorGraph);
    pFGInf->SetMaxNumberOfIterations(10);
    pFGInf->SetTolerance(tolerance);
    pFGInf->EnterEvidence(pEvidence);
    
    const CPotential* pFGJPD = NULL;
    
    const CPotential* pBNJPD = NULL;
    //std::cout<<"The difference for one node"<<std::endl;
    float diff = 0;
    int i;
    for( i = 0; i < numOfNds; i++ )
    {
        pFGInf->MarginalNodes( &i, 1 );
	
        pPearlInf->MarginalNodes( &i, 1 );
	
        pFGJPD = pFGInf->GetQueryJPD();
	
        pBNJPD = pPearlInf->GetQueryJPD();

        if( !pFGJPD->IsFactorsDistribFunEqual(pBNJPD, eps, 0, &diff) )
        {
            ret = TRS_FAIL;
        }
        //std::cout<<"node "<<i<<"has the diff"<<diff<<std::endl;
        //pFGJPD->Dump();
	
        //pBNJPD->Dump();
    }
    
    //check common distributions on families - 
    //it can be different in values 
    //std::cout<<"The difference for the families"<<std::endl;
    eps = 1e-1f;
    int numParams = pBNet->GetNumberOfFactors();
    CFactor *tempParam = NULL;
    const int *domPar; int domPSize;
    for( i = 0; i < numParams; i++ )
    {
        tempParam = pBNet->GetFactor(i);
        tempParam->GetDomain(& domPSize, &domPar);
        pFGInf->MarginalNodes( domPar, domPSize );
        pFGJPD = pFGInf->GetQueryJPD();
        pPearlInf->MarginalNodes( domPar, domPSize );
        pBNJPD = pPearlInf->GetQueryJPD();
        if( !pFGJPD->IsFactorsDistribFunEqual(pBNJPD, eps, 0, &diff) )
        {
            ret = TRS_FAIL;
        }
        //std::cout<<"family "<<i<<"has the diff"<<diff<<std::endl;
        //pFGJPD->Dump();
        //pBNJPD->Dump();
    }
    /*//provide inference with maximization
    std::cout<<"The difference for one node in MPE"<<std::endl;
    pPearlInf->EnterEvidence(pEvidence, 1);
    pFGInf->EnterEvidence(pEvidence, 1);
    const CEvidence* pPMPE = NULL;
    const CEvidence* pFGMPE = NULL;
    for( i = 0; i < numOfNds; i++ )
    {
        pFGInf->MarginalNodes( &i, 1 );
        
        pPearlInf->MarginalNodes( &i, 1 );

        pPMPE = pPearlInf->GetMPE();

        pFGMPE = pFGInf->GetMPE();

        int mpeP = pPMPE->GetValue(i)->GetInt();
        int mpeFG = pFGMPE->GetValue(i)->GetInt();
        if( mpeP != mpeFG )
        {
            ret = TRS_FAIL;
        }
        std::cout<<"MPE for node "<<i<<std::endl;
        std::cout<<"Pearl: "<<mpeP<<std::endl;
        std::cout<<"FG:    "<<mpeFG<<std::endl;
    }
    std::cout<<"The difference for one node in MPE"<<std::endl;
    int j;
    for( i = 0; i < numParams; i++ )
    {
        tempParam = pBNet->GetFactor(i);
        tempParam->GetDomain(& domPSize, &domPar);
        pFGInf->MarginalNodes( domPar, domPSize );
        pPearlInf->MarginalNodes( domPar, domPSize );
        pPMPE = pPearlInf->GetMPE();
        pFGMPE = pFGInf->GetMPE();
        std::cout<<"MPE for node "<<i<<std::endl;
        for( j = 0; j < domPSize; j++ )
        {
            int mpeP = pPMPE->GetValue(domPar[j])->GetInt();
            int mpeFG = pFGMPE->GetValue(domPar[j])->GetInt();
            if( mpeP != mpeFG )
            {
                ret = TRS_FAIL;
            }
            std::cout<<"Pearl: "<<mpeP<<std::endl;
            std::cout<<"FG:    "<<mpeFG<<std::endl;
        }
    }*/
    delete pFGInf;
    
    //CPearlInfEngine::Release(&pPearlInf);
    //CJtreeInfEngine::Release(&pPearlInf);
    delete pPearlInf;
    
    delete pBNet;
    
    delete pFactorGraph;
    
    delete pEvidence;
    
    return trsResult( ret, ret == TRS_OK ? "No errors" : "Factor Graph FAILED");
}

void initAFactorGraph()
{
    trsReg(func_name, test_desc, test_class, testFactorGraph);
}