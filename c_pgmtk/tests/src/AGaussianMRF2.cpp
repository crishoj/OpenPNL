/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AGaussianMRF2.cpp                                           //
//                                                                         //
//  Purpose:   Test on inference for Gaussian MRF2 model                   //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <time.h>
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include <float.h>

PNL_USING

static char func_name[] = "testGaussianMRF2";

static char* test_desc = "Provide inference(all existing types) for Gaussian MRF2 model ";

static char* test_class = "Algorithm";

int testGaussianMRF2()
{
    float eps = 1e-4f;
    int ret = TRS_OK;
    int i=0, j=0;

    ofstream logFileStream( "logForPearlandFG.txt", ios::app );

    pnl::LogDrvStream logDriver( &logFileStream, eLOG_ALL, eLOGSRV_ALL  );
    CNodeType *nodeTypes = new CNodeType[3];

    for( i = 0; i < 3; i++ )
    {
        nodeTypes[i].SetType( 0, i + 1 );
    }
    int query;
    int isTheSame;

    const CPotential *jpdNSm = NULL;
    const CPotential *jpdPSm = NULL;
    const CPotential *jpdFGSm = NULL;
    const CPotential *jpdJSm = NULL;

    //different information below
    int smallNumNodes = 3;
    int smallNumClqs = 2;

    int smallClqSizes[] ={ 2, 2 };

    int SmallClqs0[2] = {0,1};
    int SmallClqs1[2] = { 1,2 };
    const int *SmallClqs[] = { SmallClqs0, SmallClqs1 };
    const CNodeType *domainNT[2];
    int obsNode;


    int simplestNodeAss[3] = { 0,0,0 };
    CMRF2 *simplestSmallModel = CMRF2::Create( smallNumNodes, 3, nodeTypes,
    simplestNodeAss,
    smallNumClqs, smallClqSizes, SmallClqs );
    CModelDomain* pMD = simplestSmallModel->GetModelDomain();

    simplestSmallModel->GetGraph()->Dump();
    CFactor **mySmallParams = new CFactor*[smallNumClqs];
    float simDataM0[] = { 1.0f, 2.0f };
    float simDataM1[] = { 4.0f, 3.0f };
    float *simDataM[] = { simDataM0, simDataM1 };

    float simDataCov0[] = { 3.0f, 3.0f, 3.0f, 4.0f };
    float simDataCov1[] = { 1.0f, 1.0f, 1.0f, 3.0f };
    float *simDataCov[] = { simDataCov0, simDataCov1};

    float simNorms[] = { 1.0f, 1.0f };

    simplestSmallModel->AllocFactors();

    CFactorGraph* pSimplestFG = CFactorGraph::Create( 
        simplestSmallModel->GetModelDomain(), smallNumClqs );

    for(i=0; i<smallNumClqs; i++)
    {
        for( j=0; j<2; j++ )
        {
            domainNT[j] = simplestSmallModel->GetNodeType(SmallClqs[i][j]);
        }
        mySmallParams[i] = CGaussianPotential::Create( SmallClqs[i], 2, pMD);
        static_cast<CGaussianPotential*>(mySmallParams[i])->SetCoefficient(
                simNorms[i], 1 );
        mySmallParams[i]->AllocMatrix(simDataM[i], matMean);
        mySmallParams[i]->AllocMatrix(simDataCov[i], matCovariance);
        simplestSmallModel->AttachFactor((CGaussianPotential*)mySmallParams[i]);
        pSimplestFG->AllocFactor(2,SmallClqs[i]);
        pFactorVector factors;
        pSimplestFG->GetFactors(2, SmallClqs[i], &factors);
        static_cast<CGaussianPotential*>(factors[0])->SetCoefficient(
                simNorms[i], 1 );
        factors[0]->AllocMatrix( simDataM[i], matMean );
        factors[0]->AllocMatrix( simDataCov[i], matCovariance );
    }

    //CFactorGraph* pSimplestFG = CFactorGraph::ConvertFromMNet(simplestSmallModel);

    CEvidence *simEmptyEv = CEvidence::Create( simplestSmallModel, 0,
    NULL, valueVector() );
    CNaiveInfEngine *iSimNaiveEmpt = CNaiveInfEngine::Create(
    simplestSmallModel );
    iSimNaiveEmpt->EnterEvidence( simEmptyEv );

    CFGSumMaxInfEngine* iSimFGInfEng = CFGSumMaxInfEngine::Create(pSimplestFG);
    iSimFGInfEng->EnterEvidence(simEmptyEv);

    CInfEngine *iSimJtreeEmpt = CJtreeInfEngine::Create(
          simplestSmallModel );
    iSimJtreeEmpt->EnterEvidence( simEmptyEv );

    CPearlInfEngine *iSimPearlEmpt = CPearlInfEngine::Create(
    simplestSmallModel );
    iSimPearlEmpt->EnterEvidence( simEmptyEv);
    query = 0;
    for( i = 0; i < smallNumNodes; i++ )
    {
        query = i;
        iSimNaiveEmpt->MarginalNodes( &query, 1 );
        jpdNSm = iSimNaiveEmpt->GetQueryJPD();

        iSimPearlEmpt->MarginalNodes( &query, 1);
        jpdPSm = iSimPearlEmpt->GetQueryJPD();

        iSimJtreeEmpt->MarginalNodes( &query, 1 );
        jpdJSm = iSimJtreeEmpt->GetQueryJPD();

        iSimFGInfEng->MarginalNodes(&query, 1);
        jpdFGSm = iSimFGInfEng->GetQueryJPD();

        isTheSame = jpdPSm->IsFactorsDistribFunEqual(
            static_cast<const CFactor*>(jpdNSm), eps, 0 );
        if( !isTheSame )
        {
            ret = TRS_FAIL;
        }

        isTheSame = jpdFGSm->IsFactorsDistribFunEqual( jpdPSm, eps, 0  );
        if( !isTheSame )
        {
            ret = TRS_FAIL;
        }

        isTheSame = jpdJSm->IsFactorsDistribFunEqual(
            static_cast<const CFactor*>(jpdNSm), eps, 0 );
        if( !isTheSame )
        {
            ret = TRS_FAIL;
        }
        isTheSame = jpdJSm->IsFactorsDistribFunEqual(
            static_cast<const CFactor*>(jpdPSm), eps, 0 );
        if( !isTheSame )
        {
            ret = TRS_FAIL;
        }
    }
    obsNode = 1;
    valueVector obsVal;
    obsVal.assign( 1, (Value)0 );
    obsVal[0].SetFlt(3.0f);
    CEvidence *simOneEv = CEvidence::Create( simplestSmallModel, 1,
    &obsNode, obsVal );

    CNaiveInfEngine *iSimNaiveOne = CNaiveInfEngine::Create(
    simplestSmallModel );
    iSimNaiveOne->EnterEvidence( simOneEv );

    CPearlInfEngine *iSimPearlOne = CPearlInfEngine::Create(
    simplestSmallModel );
    iSimPearlOne->SetMaxNumberOfIterations(10);
    iSimPearlOne->EnterEvidence( simOneEv);

    CFGSumMaxInfEngine* iSimFGOne = CFGSumMaxInfEngine::Create( pSimplestFG );
    iSimFGOne->EnterEvidence( simOneEv );

    CJtreeInfEngine *iSimJtreeOne = CJtreeInfEngine::Create(
        simplestSmallModel );
    iSimJtreeOne->EnterEvidence( simOneEv );


    query = 0;
    for( i = 0; i < smallNumNodes; i++ )
    {
        query = i;
        iSimNaiveOne->MarginalNodes( &query, 1 );
        jpdNSm = iSimNaiveOne->GetQueryJPD();

        iSimPearlOne->MarginalNodes( &query, 1);
        jpdPSm = iSimPearlOne->GetQueryJPD();

        iSimJtreeOne->MarginalNodes( &query, 1);
        jpdJSm = iSimJtreeOne->GetQueryJPD();

        iSimFGOne->MarginalNodes(&query, 1);
        jpdFGSm = iSimFGOne->GetQueryJPD();

        isTheSame = jpdPSm->IsFactorsDistribFunEqual(
            static_cast<const CFactor*>(jpdNSm), eps, 0 );
        if( !isTheSame )
        {
            ret = TRS_FAIL;
        }

        isTheSame = jpdFGSm->IsFactorsDistribFunEqual( jpdPSm, eps, 0  );
        if( !isTheSame )
        {
            ret = TRS_FAIL;
        }

        isTheSame = jpdFGSm->IsFactorsDistribFunEqual(
            static_cast<const CFactor*>(jpdNSm), eps, 0 );
        if( !isTheSame )
        {
            ret = TRS_FAIL;
        }
        isTheSame = jpdJSm->IsFactorsDistribFunEqual(
            static_cast<const CFactor*>(jpdNSm), eps, 0 );
        if( !isTheSame )
        {
            ret = TRS_FAIL;
        }
    }
    
    int smallNodeAssociation[3] = {  1, 2, 0 } ;


    CMRF2 *SmallModel = CMRF2::Create( smallNumNodes, 3, nodeTypes,
            smallNodeAssociation,
            smallNumClqs, smallClqSizes, SmallClqs );
    SmallModel->GetGraph()->Dump();
    CFactor **mySmallParams1 = new CFactor*[smallNumClqs];
    float smDataM0[] = { 0.6f, 0.4f, 1.3f, 1.7f, 1.9f };
    float smDataM1[] = { 1.6f, 1.7f, 1.8f, 2.1f };
    float *smDataM[] = { smDataM0, smDataM1 };

    float smDataCov0[] = { 7.4f, 7.5f, 7.6f, 7.4f, 7.3f, 7.5f, 7.2f, 7.3f, 7.3f, 7.5f, 7.6f, 7.3f, 7.8f, 7.1f, 7.1f, 7.4f, 7.3f, 7.1f, 7.1f, 7.6f, 7.3f, 7.5f, 7.1f, 7.6f, 7.3f  };//{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.3f  };
    float smDataCov1[] = { 3.0f, 4.0f, 5.0f, 6.0f, 4.0f, 8.0f, 9.0f, 1.0f, 5.0f, 9.0f, 3.0f, 4.0f, 6.0f, 1.0f, 4.0f, 8.0f };//{ 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 6.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 4.0f };
    float *smDataCov[] = { smDataCov0, smDataCov1};

    float smNorms[] = { 1.0f, 1.0f };

    SmallModel->AllocFactors();
    CModelDomain* pMD1 = SmallModel->GetModelDomain();

    CFactors* pFactors = CFactors::Create( smallNumClqs );

    for(i=0; i<smallNumClqs; i++)
    {
        for( j = 0; j < 2; j++ )
        {
            domainNT[j] = SmallModel->GetNodeType(SmallClqs[i][j]);
        }
        mySmallParams1[i] = CGaussianPotential::Create( SmallClqs[i], 2, pMD1);
        static_cast<CGaussianPotential*>(mySmallParams1[i])->SetCoefficient( smNorms[i], 0 );
        mySmallParams1[i]->AllocMatrix(smDataM[i], matMean);
        mySmallParams1[i]->AllocMatrix(smDataCov[i], matCovariance);

        SmallModel->AttachFactor((CGaussianPotential*)mySmallParams1[i]);
        pFactors->AddFactor( mySmallParams1[i]->Clone() );
    }

    //CFactorGraph* smallFG = CFactorGraph::ConvertFromMNet(SmallModel);
    //create the factor graph based on factors
    CFactorGraph* smallFG = CFactorGraph::Create( SmallModel->GetModelDomain(),
        pFactors );

    CEvidence *smallEmptyEv = CEvidence::Create( SmallModel, 0, NULL, valueVector() );

    CFGSumMaxInfEngine* iSmFGInfEmpt = CFGSumMaxInfEngine::Create( smallFG );
    iSmFGInfEmpt->EnterEvidence(smallEmptyEv);

    CInfEngine *iSmNaiveEmpt = CNaiveInfEngine::Create( SmallModel );
    iSmNaiveEmpt->EnterEvidence( smallEmptyEv );

    CInfEngine *iSmPearlEmpt = CPearlInfEngine::Create( SmallModel );
    iSmPearlEmpt->EnterEvidence( smallEmptyEv);


    query = 0;
    for( i = 0; i < smallNumNodes; i++ )
    {
        query = i;
        iSmFGInfEmpt->MarginalNodes( &query, 1 );
        jpdFGSm = iSmFGInfEmpt->GetQueryJPD();
        iSmNaiveEmpt->MarginalNodes( &query, 1 );
        jpdNSm = iSmNaiveEmpt->GetQueryJPD();
        iSmPearlEmpt->MarginalNodes( &query, 1);
        jpdPSm = iSmPearlEmpt->GetQueryJPD();
        isTheSame = jpdFGSm->IsFactorsDistribFunEqual(
            static_cast<const CFactor*>(jpdNSm), eps, 0 );
        if( !isTheSame )
        {
            ret = TRS_FAIL;
        }
        isTheSame = jpdPSm->IsFactorsDistribFunEqual(
            static_cast<const CFactor*>(jpdNSm), eps, 0 );
        if( !isTheSame )
        {
            ret = TRS_FAIL;
        }
    }

    obsNode = 1;
    valueVector vals;
    vals.assign( 3, (Value)0 );
    vals[0].SetFlt(1.0f);
    vals[1].SetFlt(2.0f);
    vals[2].SetFlt(3.0f);
    CEvidence *smallOneEv = CEvidence::Create( SmallModel, 1, &obsNode,
        vals );

    CFGSumMaxInfEngine* iSmFGInfOne = CFGSumMaxInfEngine::Create( smallFG );
    iSmFGInfOne->EnterEvidence(smallOneEv);

    CInfEngine *iSmNaiveOne = CNaiveInfEngine::Create( SmallModel );
    iSmNaiveOne->EnterEvidence( smallOneEv );

    CInfEngine *iSmPearlOne = CPearlInfEngine::Create( SmallModel );
    iSmPearlOne->EnterEvidence( smallOneEv);

    for( i = 0; i < smallNumNodes; i++ )
    {
        query = i;
        iSmFGInfOne->MarginalNodes( &query, 1 );
        jpdFGSm = iSmFGInfOne->GetQueryJPD();
        iSmNaiveOne->MarginalNodes( &query, 1 );
        jpdNSm = iSmNaiveOne->GetQueryJPD();
        iSmPearlOne->MarginalNodes( &query, 1);
        jpdPSm = iSmPearlOne->GetQueryJPD();
        isTheSame = jpdPSm->IsFactorsDistribFunEqual(
            static_cast<const CFactor*>(jpdNSm), eps, 0 );
        if( !isTheSame )
        {
            ret = TRS_FAIL;
        }
        isTheSame = jpdFGSm->IsFactorsDistribFunEqual(
            static_cast<const CFactor*>(jpdNSm), eps, 0 );
        if( !isTheSame )
        {
            ret = TRS_FAIL;
        }
    }


    int nnodes = 9;
    int numClqs =8;


    int nodeAssociation[9] = { 0, 0, 1, 2, 0, 1, 1, 2, 0 };

    //create graphical model by clqs;
    int *clqSizes  = new int[numClqs];
    for ( i = 0; i < numClqs; i++ )
    {
        clqSizes[i] = 2;
    }

    int clqs0[] = {0,1};
    int clqs1[] = {1,2};
    int clqs2[] = {1,3};
    int clqs3[] = {0,4};
    int clqs4[] = {4,5};
    int clqs5[] = {4,6};
    int clqs6[] = {6,7};
    int clqs7[] = {6,8};
    const int *clqs[] = { clqs0, clqs1, clqs2, clqs3, clqs4, clqs5, clqs6,
        clqs7};
    CMRF2 *myModel = CMRF2::Create( nnodes, 3, nodeTypes, nodeAssociation,
        numClqs, clqSizes, clqs );


    myModel->GetGraph()->Dump();
    //we creates every factor - it is factor

    //number of factors is the same as number of cliques - one per clique
    CFactor **myParams = new CFactor*[numClqs];


    //to create factors we need to create their tables

    //create container for Factors
    myModel->AllocFactors();
    //create array of data for every parameter
    float DataM0[] = { 0.6f, 0.4f };
    float DataM1[] = { 1.5f, 1.5f, 1.7f };
    float DataM2[] = { 2.1f, 2.9f, 2.3f, 2.7f };
    float DataM3[] = { 3.1f, 3.9f };
    float DataM4[] = { 4.2f, 4.8f, 4.3f };
    float DataM5[] = { 5.4f, 5.6f, 5.6f };
    float DataM6[] = { 6.8f, 6.2f, 6.9f, 6.1f, 6.7f };
    float DataM7[] = { 7.1f, 7.4f, 7.3f };
    float *DataM[] = { DataM0, DataM1, DataM2, DataM3, DataM4, DataM5, DataM6,
    DataM7};

    float DataC0[] = { 1.0f, 3.0f, 3.0f, 4.0f };
    //{ 1.0f, 0.0f, 0.0f, 4.0f };
    float DataC1[] = { 2.0f, 3.0f, 4.0f, 3.0f, 6.0f, 7.0f, 4.0f, 7.0f, 1.0f };
    //{ 2.0f, 0.0f, 0.0f, 0.0f, 6.0f, 0.0f, 0.0f, 0.0f, 1.0f };
    float DataC2[] ={ 3.0f, 4.0f, 5.0f, 6.0f, 4.0f, 8.0f, 9.0f, 1.0f, 5.0f, 9.0f, 3.0f, 4.0f, 6.0f, 1.0f, 4.0f, 8.0f };
    //{ 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 8.0f, 0.0f, 0.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 8.0f };
    float DataC3[] = { 4.0f, 6.0f, 6.0f, 7.0f };
    //{ 4.0f, 0.0f, 0.0f, 7.0f };
    float DataC4[] = { 5.0f, 6.0f, 7.0f, 6.0f, 9.0f, 1.0f, 7.0f, 1.0f, 4.0f };
    //{ 5.0f, 0.0f, 0.0f, 0.0f, 9.0f, 0.0f, 0.0f, 0.0f, 4.0f };
    float DataC5[] = { 6.0f, 7.0f, 8.0f, 7.0f, 1.0f, 2.0f, 8.0f, 2.0f, 5.0f };
    //{ 6.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 5.0f };
    float DataC6[] = { 7.4f, 7.5f, 7.6f, 7.4f, 7.3f, 7.5f, 7.2f, 7.3f, 7.3f, 7.5f, 7.6f, 7.3f, 7.8f, 7.1f, 7.1f, 7.4f, 7.3f, 7.1f, 7.1f, 7.6f, 7.3f, 7.5f, 7.1f, 7.6f, 7.3f  };
    //{ 7.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 7.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 7.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 7.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 7.3f  };
    float DataC7[] = { 8.0f, 9.0f, 1.0f, 9.0f, 3.0f, 4.0f, 1.0f, 4.0f, 7.0f };
    //{ 8.0f, 0.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 7.0f    };
    float *DataC[] = { DataC0, DataC1, DataC2, DataC3, DataC4, DataC5, DataC6,
    DataC7};

    float norms[] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

    CModelDomain* pMD2 = myModel->GetModelDomain();

    for(i=0; i<numClqs; i++)
    {
        myParams[i] = CGaussianPotential::Create( clqs[i], 2, pMD2);
        static_cast<CGaussianPotential*>(myParams[i])->SetCoefficient( norms[i], 1 );
        myParams[i]->AllocMatrix(DataM[i], matH);
        myParams[i]->AllocMatrix(DataC[i], matK);

        myModel->AttachFactor((CGaussianPotential*)myParams[i]);
    }

    CFactorGraph* myModelFG = CFactorGraph::ConvertFromMNet( myModel );

    CEvidence *emptyEv = CEvidence::Create( myModel, 0, NULL, valueVector() );

    //start NaiveInf
    CInfEngine *iNaiveEmpt = CNaiveInfEngine::Create(myModel );
    iNaiveEmpt->EnterEvidence( emptyEv );

    CInfEngine *iPearlEmpt = CPearlInfEngine::Create(myModel );
    iPearlEmpt->EnterEvidence(emptyEv);

    CInfEngine *iFGEmpt = CFGSumMaxInfEngine::Create( myModelFG );
    iFGEmpt->EnterEvidence( emptyEv );

    const CPotential *jpdN = NULL;
    const CPotential *jpdP = NULL;
    const CPotential* jpdFG = NULL;
    //    CNumericDenseMatrix<float> *matMeanN = NULL;
    //    CNumericDenseMatrix<float> *matCovN = NULL;
    //    CNumericDenseMatrix<float> *matMeanP = NULL;
    //    CNumericDenseMatrix<float> *matCovP = NULL;
    //    int flagSpecificP;
    //    int flagSpecificN;
    //    int flagMomN;
    //    int flagCanN;
    //    int flagMomP;
    //    int flagcanP;
    query = 0;
    eps = 1e-3f;
    for( i = 0; i < nnodes; i++ )
    {
        query = i;
        iNaiveEmpt->MarginalNodes( &query, 1 );
        jpdN = iNaiveEmpt->GetQueryJPD();
        iPearlEmpt->MarginalNodes( &query, 1);
        jpdP = iPearlEmpt->GetQueryJPD();
        iFGEmpt->MarginalNodes( &query, 1 );
        jpdFG = iFGEmpt->GetQueryJPD();
        isTheSame = jpdP->IsFactorsDistribFunEqual(
            static_cast<const CFactor*>(jpdFG), eps, 0 );
        if( !isTheSame )
        {
            ret = TRS_FAIL;
        }
        isTheSame = jpdN->IsFactorsDistribFunEqual(
            static_cast<const CFactor*>(jpdFG), eps, 0 );
        if( !isTheSame )
        {
            ret = TRS_FAIL;
        }
    }

    //check some methods of FactorGraph
    CFactorGraph* smallFGCopy = CFactorGraph::Copy(smallFG);
    int isValid = smallFGCopy->IsValid();
    if(!isValid)
    {
        ret = TRS_FAIL;
    }
    delete smallFGCopy;


    //release all memory
    delete clqSizes;
    delete emptyEv;
    delete iNaiveEmpt;
    delete iPearlEmpt;
    delete iFGEmpt;
    delete myModelFG;
    delete myModel;
    delete iSmFGInfOne;
    delete iSmNaiveOne;
    delete iSmPearlOne;
    delete smallOneEv;

    delete iSmFGInfEmpt;
    delete iSmNaiveEmpt;
    delete iSmPearlEmpt;

    delete iSimFGOne;
    //CPearlInfEngine::Release(&iSimPearlOne);
    delete iSimPearlOne;
    delete iSimJtreeOne;
    delete iSimNaiveOne;
    delete simOneEv;

    //CPearlInfEngine::Release(&iSimPearlEmpt);
    delete iSimPearlEmpt;
    delete iSimJtreeEmpt;
    delete iSimFGInfEng;
    delete iSimNaiveEmpt;
    delete smallEmptyEv;
    
    delete pFactors;
    delete simEmptyEv;
    delete pSimplestFG;
    delete simplestSmallModel;
    delete SmallModel;
    delete smallFG;

    delete []mySmallParams1;
    delete []mySmallParams;
    delete []myParams;
    delete []nodeTypes;
    return trsResult( ret, ret == TRS_OK ? "No errors"
    : "Bad test on GaussianMRF2");
}

void initAGaussianMRF2()
{
    trsReg(func_name, test_desc, test_class, testGaussianMRF2);
}

