/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      ASparseMatrixInfernceBNet.cpp                               //
//                                                                         //
//  Purpose:   Test on inference on sparse BNet model                      //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"

PNL_USING

static char func_name[] = "testSparseMatrixInferenceBNet";

static char* test_desc = "Create model with sparse matrices and provide inference on it";

static char* test_class = "Algorithm";

int testSparseMatrixInferenceBNet()
{
    int i;
    float eps = 1e-5f;
    int ret = TRS_OK;
    //create sparse model
    CBNet* pSpBNet = pnlExCreateWaterSprinklerSparseBNet();
    CNaiveInfEngine *pSpNaiveEng = CNaiveInfEngine::Create(pSpBNet);
    CJtreeInfEngine *pSpJtreeEng = CJtreeInfEngine::Create( pSpBNet );
    //create dense model
    CBNet* pBNet = pnlExCreateWaterSprinklerBNet();
    CNaiveInfEngine *pNaiveEng = CNaiveInfEngine::Create(pBNet);
    CJtreeInfEngine *pJtreeEng = CJtreeInfEngine::Create(pBNet);

    //create empty evidence
    CEvidence* pSpEmptyEvid = CEvidence::Create(pSpBNet, 0, NULL, valueVector());
    CEvidence* pEmptyEvid = CEvidence::Create( pBNet, 0, NULL, valueVector() );
    //enter empty evidences
    pSpNaiveEng->EnterEvidence(pSpEmptyEvid);
    pSpJtreeEng->EnterEvidence(pSpEmptyEvid);

    pJtreeEng->EnterEvidence(pEmptyEvid);
    pNaiveEng->EnterEvidence(pEmptyEvid);

     //get & view marginals
    int compareRes = 1;

    const CPotential* pSpNaivePot = NULL;
    const CPotential* pSpJtreePot = NULL;
    const CPotential* pNaivePot = NULL;
    const CPotential* pJtreePot = NULL;
    for( i = 0; i < 4; i++ )
    {
        pSpNaiveEng->MarginalNodes( &i, 1 );
	    pSpNaivePot = pSpNaiveEng->GetQueryJPD();//->Dump();
        //std::cin.get();

        pSpJtreeEng->MarginalNodes( &i, 1 );
	    pSpJtreePot =  pSpJtreeEng->GetQueryJPD();//->Dump();
        //std::cin.get();

        pNaiveEng->MarginalNodes( &i, 1 );
	    pNaivePot = pNaiveEng->GetQueryJPD();//->Dump();
        //std::cin.get();

        pJtreeEng->MarginalNodes( &i, 1 );
	    pJtreePot = pJtreeEng->GetQueryJPD();//->Dump();
        //std::cin.get();

        compareRes = pSpNaivePot->IsFactorsDistribFunEqual( pSpJtreePot, eps );
        if( !compareRes )
        {
            ret = TRS_FAIL;
        }
        compareRes = pNaivePot->IsFactorsDistribFunEqual( pJtreePot, eps );
        if( !compareRes )
        {
            ret = TRS_FAIL;
        }
        compareRes = pSpNaivePot->IsFactorsDistribFunEqual( pNaivePot, eps );
        if( !compareRes )
        {
            ret = TRS_FAIL;
        }
        compareRes = pSpJtreePot->IsFactorsDistribFunEqual( pJtreePot, eps );
        if( !compareRes )
        {
            ret = TRS_FAIL;
        }
    }

    //create evidences
    const int numOfObsNds = 2;
	int obsNds[] = { 0, 1 };
    valueVector obsNdsVals;
    obsNdsVals.assign( 2, (Value)0 );
    obsNdsVals[0].SetInt(0);
    obsNdsVals[1].SetInt(0);
    CEvidence *pSpEvidence = CEvidence::Create( pSpBNet, numOfObsNds, obsNds,
		obsNdsVals );
    CEvidence *pEvidence = CEvidence::Create( pBNet, numOfObsNds, obsNds,
		obsNdsVals );
    //enter evidences
    pSpNaiveEng->EnterEvidence(pSpEvidence);
    pSpJtreeEng->EnterEvidence(pSpEvidence);

    pNaiveEng->EnterEvidence(pEvidence);
    pJtreeEng->EnterEvidence(pEvidence);
    //get & view marginals

    for( i = 0; i < 4; i++ )
    {
        pSpNaiveEng->MarginalNodes( &i, 1 );
	    pSpNaivePot = pSpNaiveEng->GetQueryJPD();//->Dump();
        //std::cin.get();

        pSpJtreeEng->MarginalNodes( &i, 1 );
	    pSpJtreePot =  pSpJtreeEng->GetQueryJPD();//->Dump();
        //std::cin.get();

        pNaiveEng->MarginalNodes( &i, 1 );
	    pNaivePot = pNaiveEng->GetQueryJPD();//->Dump();
        //std::cin.get();

        pJtreeEng->MarginalNodes( &i, 1 );
	    pJtreePot = pJtreeEng->GetQueryJPD();//->Dump();
        //std::cin.get();

        compareRes = pSpNaivePot->IsFactorsDistribFunEqual( pSpJtreePot, eps );
        if( !compareRes )
        {
            ret = TRS_FAIL;
        }
        compareRes = pNaivePot->IsFactorsDistribFunEqual( pJtreePot, eps );
        if( !compareRes )
        {
            ret = TRS_FAIL;
        }
        compareRes = pSpNaivePot->IsFactorsDistribFunEqual( pNaivePot, eps );
        if( !compareRes )
        {
            ret = TRS_FAIL;
        }
        compareRes = pSpJtreePot->IsFactorsDistribFunEqual( pJtreePot, eps );
        if( !compareRes )
        {
            ret = TRS_FAIL;
        }
    }

     
    delete pSpEmptyEvid;
    delete pEmptyEvid;
    delete pNaiveEng;
    delete pSpNaiveEng;
    //CJtreeInfEngine::Release(&pSpJtreeEng);
    delete pSpJtreeEng;
    delete pBNet;
    delete pSpBNet;

    //create other model
    pBNet = pnlExCreateAsiaBNet();
    //convert to sparse
    pSpBNet = pBNet->ConvertToSparse();
    pSpNaiveEng = CNaiveInfEngine::Create( pSpBNet );
    pSpJtreeEng = CJtreeInfEngine::Create( pSpBNet );

    pNaiveEng = CNaiveInfEngine::Create(pBNet);
    pJtreeEng = CJtreeInfEngine::Create(pBNet);

    pSpEmptyEvid = CEvidence::Create(pSpBNet, 0, NULL, valueVector());
    pEmptyEvid = CEvidence::Create( pBNet, 0, NULL, valueVector() );

    pSpEvidence = CEvidence::Create( pSpBNet, numOfObsNds, obsNds, obsNdsVals );
    pEvidence = CEvidence::Create( pBNet, numOfObsNds, obsNds, obsNdsVals );
    
    //enter empty evidences
    pSpNaiveEng->EnterEvidence(pSpEmptyEvid);
    pSpJtreeEng->EnterEvidence(pSpEmptyEvid);

    pNaiveEng->EnterEvidence(pEmptyEvid);
    pJtreeEng->EnterEvidence(pEmptyEvid);

    int nnodes = pBNet->GetNumberOfNodes();
    //check full matrices
    intVector allNodes;
    allNodes.assign( nnodes, 0 );
    for( i = 0; i < nnodes; i++ )
    {
        allNodes[i] = i;
    }
    pSpNaiveEng->MarginalNodes( &allNodes.front(), nnodes );
	pSpNaivePot = pSpNaiveEng->GetQueryJPD();
    pNaiveEng->MarginalNodes( &allNodes.front(), nnodes );
	pNaivePot = pNaiveEng->GetQueryJPD();
    compareRes = pSpNaivePot->IsFactorsDistribFunEqual( pNaivePot, eps );
    if( !compareRes )
    {
        ret = TRS_FAIL;
    }
    for( i = 0; i < nnodes; i++ )
    {

        pSpNaiveEng->MarginalNodes( &i, 1 );
	    pSpNaivePot = pSpNaiveEng->GetQueryJPD();//->Dump();
        //std::cin.get();

        pSpJtreeEng->MarginalNodes( &i, 1 );
	    pSpJtreePot =  pSpJtreeEng->GetQueryJPD();//->Dump();
        //std::cin.get();

        pNaiveEng->MarginalNodes( &i, 1 );
	    pNaivePot = pNaiveEng->GetQueryJPD();//->Dump();
        //std::cin.get();

        pJtreeEng->MarginalNodes( &i, 1 );
	    pJtreePot = pJtreeEng->GetQueryJPD();//->Dump();
        //std::cin.get();

        compareRes = pSpNaivePot->IsFactorsDistribFunEqual( pSpJtreePot, eps );
        if( !compareRes )
        {
            ret = TRS_FAIL;
        }
        compareRes = pNaivePot->IsFactorsDistribFunEqual( pJtreePot, eps );
        if( !compareRes )
        {
            ret = TRS_FAIL;
        }
        compareRes = pSpNaivePot->IsFactorsDistribFunEqual( pNaivePot, eps );
        if( !compareRes )
        {
            ret = TRS_FAIL;
        }
        compareRes = pSpJtreePot->IsFactorsDistribFunEqual( pJtreePot, eps );
        if( !compareRes )
        {
            ret = TRS_FAIL;
        }

    }

    //enter evidences - non empty
    pSpNaiveEng->EnterEvidence(pSpEvidence);
    pSpJtreeEng->EnterEvidence(pSpEvidence);

    pNaiveEng->EnterEvidence(pEvidence);
    pJtreeEng->EnterEvidence(pEvidence);
    for( i = 0; i < nnodes; i++ )
    {
        pSpNaiveEng->MarginalNodes( &i, 1 );
	    pSpNaivePot = pSpNaiveEng->GetQueryJPD();
        //pSpNaivePot->Dump();
        //std::cin.get();

        pSpJtreeEng->MarginalNodes( &i, 1 );
	    pSpJtreePot =  pSpJtreeEng->GetQueryJPD();
        //pSpNaivePot->Dump();
        //std::cin.get();

        pNaiveEng->MarginalNodes( &i, 1 );
	    pNaivePot = pNaiveEng->GetQueryJPD();
        //pSpNaivePot->Dump();
        //std::cin.get();

        pJtreeEng->MarginalNodes( &i, 1 );
	    pJtreePot = pJtreeEng->GetQueryJPD();
        //pSpNaivePot->Dump();
        //std::cin.get();

        compareRes = pSpNaivePot->IsFactorsDistribFunEqual( pSpJtreePot, eps );
        if( !compareRes )
        {
            ret = TRS_FAIL;
        }
        compareRes = pNaivePot->IsFactorsDistribFunEqual( pJtreePot, eps );
        if( !compareRes )
        {
            ret = TRS_FAIL;
        }
        compareRes = pSpNaivePot->IsFactorsDistribFunEqual( pJtreePot, eps );
        if( !compareRes )
        {
            ret = TRS_FAIL;
        }
    }
    //asking about distributions on families
    const CGraph* graph = pBNet->GetGraph();
    intVector parents;
    for( i = 0; i < nnodes; i++ )
    {
        graph->GetParents( i, &parents );
        parents.push_back(i);

        pSpNaiveEng->MarginalNodes( &parents.front(), parents.size() );
	    pSpNaivePot = pSpNaiveEng->GetQueryJPD();
        //pSpNaivePot->Dump();
        //std::cin.get();

        pSpJtreeEng->MarginalNodes( &parents.front(), parents.size() );
	    pSpJtreePot =  pSpJtreeEng->GetQueryJPD();
        //pSpNaivePot->Dump();
        //std::cin.get();

        pNaiveEng->MarginalNodes( &parents.front(), parents.size());
	    pNaivePot = pNaiveEng->GetQueryJPD();
        //pSpNaivePot->Dump();
        //std::cin.get();

        pJtreeEng->MarginalNodes( &parents.front(), parents.size() );
	    pJtreePot = pJtreeEng->GetQueryJPD();
        //pSpNaivePot->Dump();
        //std::cin.get();

        parents.clear();
        compareRes = pSpNaivePot->IsFactorsDistribFunEqual( pSpJtreePot, eps );
        if( !compareRes )
        {
            ret = TRS_FAIL;
        }
        compareRes = pNaivePot->IsFactorsDistribFunEqual( pJtreePot, eps );
        if( !compareRes )
        {
            ret = TRS_FAIL;
        }
        compareRes = pSpNaivePot->IsFactorsDistribFunEqual( pJtreePot, eps );
        if( !compareRes )
        {
            ret = TRS_FAIL;
        }
    }


    delete pNaiveEng;
    delete pSpNaiveEng;
    //CJtreeInfEngine::Release(&pSpJtreeEng);
    delete pSpJtreeEng;
    delete pBNet;
    delete pSpBNet;
    delete pEmptyEvid;
    delete pSpEmptyEvid;
    delete pEvidence;
    delete pSpEvidence;
    
   	return trsResult( ret, ret == TRS_OK ? "No errors" : 
    	"Bad test on sparse matrix inference");
}

void initASparseMatrixInferenceBnet()
{
    trsReg(func_name, test_desc, test_class, testSparseMatrixInferenceBNet);
}
