/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      inf_learn_dbn.cpp                                           //
//                                                                         //
//  Purpose:   Example on creation DBN, inference and learning             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnl_dll.hpp"
#include "time.h"
#include <fstream>

PNL_USING

void ShowResultsForInference(const CPotential * pQueryPot, int slice);
void ShowCPD(const CCPD* pCPD);
int main()
{
    CBNet *pBNetForArHMM = pnlExCreateRndArHMM();
    CDBN *pArHMM = CDBN::Create( pBNetForArHMM );

     //Create an inference engine
    C1_5SliceJtreeInfEngine* pInfEng;
    pInfEng = C1_5SliceJtreeInfEngine::Create(pArHMM);
    
    //Number of time slices for unrolling 
    int nTimeSlices = 5;
    const CPotential* pQueryJPD;
    
    //Crate evidence for every slice
    CEvidence** pEvidences;
    pEvidences = new CEvidence*[nTimeSlices];
    
    //Let node 1 is always observed 
    const int obsNodesNums[] = { 1 };
    valueVector obsNodesVals(1);
    
    int i;    
    for( i = 0; i < nTimeSlices; i++ )
    {
        // Generate random value 
	obsNodesVals[0].SetInt(rand()%2);
        pEvidences[i] = CEvidence::Create( pArHMM, 1, obsNodesNums,
            obsNodesVals );
    }
    
    // Create smoothing procedure
    pInfEng->DefineProcedure(ptSmoothing, nTimeSlices);
    // Enter created evidences
    pInfEng->EnterEvidence(pEvidences, nTimeSlices);
    // Start smoothing process
    pInfEng->Smoothing();
    
    // Choose query set of nodes for every slice
    int queryPrior[] = { 0 };
    int queryPriorSize = 1;
    int query[] = { 0, 2 };
    int querySize = 2;
    
    std::cout << " Results of smoothing " << std::endl;
    
    int slice = 0;
    pInfEng->MarginalNodes( queryPrior, queryPriorSize, slice );
    pQueryJPD = pInfEng->GetQueryJPD();
    
    ShowResultsForInference(pQueryJPD, slice);
    //pQueryJPD->Dump();
   
    
    std::cout << std::endl;
    
    for( slice = 1; slice < nTimeSlices; slice++ )
    {
        pInfEng->MarginalNodes( query, querySize, slice );
        pQueryJPD = pInfEng->GetQueryJPD();
        ShowResultsForInference(pQueryJPD, slice);
	//pQueryJPD->Dump();
    }
    
    slice = 0;
    
    //Create filtering procedure
    pInfEng->DefineProcedure( ptFiltering );
    pInfEng->EnterEvidence( &(pEvidences[slice]), 1 );
    pInfEng->Filtering( slice );
    pInfEng->MarginalNodes( queryPrior, queryPriorSize );
    pQueryJPD = pInfEng->GetQueryJPD();

    std::cout << " Results of filtering " << std::endl;
    ShowResultsForInference(pQueryJPD, slice);
    //pQueryJPD->Dump();
    

    for( slice = 1; slice < nTimeSlices; slice++ )
    {
        pInfEng->EnterEvidence( &(pEvidences[slice]), 1 );
        pInfEng->Filtering( slice );
        pInfEng->MarginalNodes( query, querySize );
        pQueryJPD = pInfEng->GetQueryJPD();
        ShowResultsForInference(pQueryJPD, slice);
	//pQueryJPD->Dump();
    }
    
    //Create fixed-lag smoothing (online)
    int lag = 2;
    pInfEng->DefineProcedure( ptFixLagSmoothing, lag );

    for (slice = 0; slice < lag + 1; slice++)
    {
        pInfEng->EnterEvidence( &(pEvidences[slice]), 1 );
    }
    std::cout << " Results of fixed-lag smoothing " << std::endl;
    
    pInfEng->FixLagSmoothing( slice );
    pInfEng->MarginalNodes( queryPrior, queryPriorSize );
    pQueryJPD = pInfEng->GetQueryJPD();
    ShowResultsForInference(pQueryJPD, slice);
    //pQueryJPD->Dump();
    
    std::cout << std::endl;

    for( ; slice < nTimeSlices; slice++ )
    {
        pInfEng->EnterEvidence( &(pEvidences[slice]), 1 );
        pInfEng->FixLagSmoothing( slice );
        pInfEng->MarginalNodes( query, querySize );
        pQueryJPD = pInfEng->GetQueryJPD();
        ShowResultsForInference(pQueryJPD, slice);
	//pQueryJPD->Dump();
    }
    
    delete pInfEng;
    
    for( slice = 0; slice < nTimeSlices; slice++)
    {
        delete pEvidences[slice];
    }
    
    //Create learning procedure for DBN
    pEvidencesVecVector evidencesOut;
    
    
    const int nTimeSeries = 500;
    intVector nSlices(nTimeSeries);
    //define number of slices in the every time series
    pnlRand(nTimeSeries, &nSlices.front(), 3, 20);
    // Generate evidences in a random way
    pArHMM->GenerateSamples( &evidencesOut, nSlices);
        
    // Create DBN for learning
    CDBN *pDBN = CDBN::Create(pnlExCreateRndArHMM());
    
    // Create learning engine
    CEMLearningEngineDBN *pLearn = CEMLearningEngineDBN::Create( pDBN );
    
    // Set data for learning
    pLearn->SetData( evidencesOut );
    
    // Start learning
    try
    {
        pLearn->Learn();
    }
    catch(CAlgorithmicException except)
    {
        std::cout << except.GetMessage() << std::endl;
    }
    
    std::cout<<"Leraning procedure"<<std::endl;
    
    const CCPD *pCPD1, *pCPD2;
    for( i = 0; i < 4; i++ )
    {
        std::cout<<" initial model"<<std::endl;
        pCPD1 = static_cast<const CCPD*>( pArHMM->GetFactor(i) );
        ShowCPD( pCPD1 );
	
        
	std::cout<<" model after learning"<<std::endl;
        pCPD2 = static_cast<const CCPD*>( pDBN->GetFactor(i) );
	ShowCPD( pCPD2 );
        
    }
    
    for( i = 0; i < evidencesOut.size(); i++ )
    {
        int j;
        for( j = 0; j < evidencesOut[i].size(); j++ )
        {
            delete evidencesOut[i][j];
        }
    }
    delete pDBN;
    delete pArHMM;
    delete pLearn;

    return 0;
}

void ShowResultsForInference(const CPotential * pQueryPot, int slice)
{
    int nnodes;
    const int* domain;
    pQueryPot->GetDomain( &nnodes, &domain );
    std::cout<<" probability distribution for nodes [ ";
    
    int i;	
    for( i = 0; i < nnodes; i++ )
    {
	std::cout<<domain[i]<<" ";
    }
    
    
    std::cout<<"] at slice "<<slice<<std::endl;
    CMatrix<float>* pMat = pQueryPot->GetMatrix(matTable);
    
    // graphical model hase been created using dense matrix
    // so, the marginal is also dense
    EMatrixClass type = pMat->GetMatrixClass();
    if( ! ( type == mcDense || type == mcNumericDense || type == mc2DNumericDense ) )
    {
	assert(0);
    }
    
    int nEl;
    const float* data;
    static_cast<CNumericDenseMatrix<float>*>(pMat)->GetRawData(&nEl, &data);
    for( i = 0; i < nEl; i++ )
    {
	std::cout<<" "<<data[i];
    }
    std::cout<<std::endl;
}

void ShowCPD( const CCPD* pCPD )
{
    int nnodes;
    const int* domain;
    pCPD->GetDomain( &nnodes, &domain );
    std::cout<<" node "<<domain[nnodes -1]<<" hase the parents ";
    
    int i;	
    for( i = 0; i < nnodes - 1; i++ )
    {
	std::cout<<domain[i]<<" ";
    }
    
    std::cout<<std::endl;
    CMatrix<float>* pMat = pCPD->GetMatrix(matTable);
    
    // graphical model hase been created using dense matrix
    
    EMatrixClass type = pMat->GetMatrixClass();
    if( ! ( type == mcDense || type == mcNumericDense || type == mc2DNumericDense ) )
    {
	assert(0);
    }
    
    std::cout<<" conditional probability distribution \n";
    int nEl;
    const float* data;
    static_cast<CNumericDenseMatrix<float>*>(pMat)->GetRawData(&nEl, &data);
    for( i = 0; i < nEl; i++ )
    {
	std::cout<<" "<<data[i];
    }

    std::cout<<std::endl<<std::endl;
}

