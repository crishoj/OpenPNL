/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      TBKInference.cpp                                            //
//                                                                         //
//  Purpose:   Timing Test of BK Inference                                 //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include "tUtil.h"

PNL_USING
static char func_name[]  = "timeBKInfEngine";
static char test_desc[]  = "Run timing test BK inference engine";
static char test_class[] = "Timing";


int CompareSmoothing( CBKInfEngine *pBKInf, C1_5SliceJtreeInfEngine *p1_5JTreeinf, 
                    pEvidencesVector &pEv, intVecVector &query, float eps);
int CompareFiltering( CBKInfEngine *pBKInf, C1_5SliceJtreeInfEngine *p1_5JTreeinf, 
                    pEvidencesVector &pEv, intVecVector &query, float eps);
int CompareFixLagSmoothing( CBKInfEngine *pBKInf, C1_5SliceJtreeInfEngine *p1_5JTreeinf, 
                          pEvidencesVector &pEv, int lag, intVecVector &query, float eps);

int timeBKInfEngine()
{
    int ret = TRS_OK;
    int seed = pnlTestRandSeed();
    std::cout<<"seed"<<seed<<std::endl;
    srand( seed );
    
    int nTimeSlices = -1;
    while(nTimeSlices <= 4)
    {
        trsiRead (&nTimeSlices, "10", "Number of slices");
    }
    
    float eps = -1.0f;
    while( eps <= 0 )
    {
        trssRead( &eps, "1e-2f", "accuracy in test");
    }
      
    
    CBNet *pBNet = pnlExCreateBatNetwork();
    
    CDBN *pDBN = CDBN::Create(pBNet);
        
    pEvidencesVecVector pEvid;
    intVector slices(1, nTimeSlices);
    pDBN->GenerateSamples( &pEvid, slices );
    
    int unobsNds[] = {0, 2, 3, 5, 6, 7, 9, 11, 12, 13, 
        15, 16, 18, 19, 20, 22, 24, 26};
    
    int nUnobsNds = 18;
    int slice;
    
    for( slice = 0; slice < nTimeSlices; slice++ )
    {
        pEvid[0][slice]->ToggleNodeState(nUnobsNds, unobsNds);
    }
    
    
    //////////////////////////////////////////////
    double freqCPU = trsClocksPerSec();
    
    //////////////////////////////////////////////////////////////////////////
    /*
     Compare fully factorized version of BK inference and jtree
     */
    trsTimerStart(1);
    CBKInfEngine *pBKInf = CBKInfEngine::Create(pDBN, true);
    trsTimerStop(1);
    
    double timeOfInfCreationBK = trsTimerSec(1);
    
    trsTimerStart(2);
    C1_5SliceJtreeInfEngine *p1_5JTreeInf = C1_5SliceJtreeInfEngine::Create(pDBN);
    trsTimerStop(2);
    
    double timeOfInfCreation1_5SliceJTree = trsTimerSec(2);

    int nnodesPerSlice = pDBN->GetNumberOfNodes();
    
    intVecVector queryes(nTimeSlices);
    
    int node = (int)(rand()%(nUnobsNds - 1));
    int chld = unobsNds[node];
    pBNet->GetFactor(chld)->GetDomain(&queryes[0]);
    int i;
    for( i = 1; i < nTimeSlices; i++ )
    {
       node = (int)(rand()%(nUnobsNds - 1));
       chld = unobsNds[node] + nnodesPerSlice;
       pBNet->GetFactor(chld)->GetDomain(&queryes[i]);
      
    }
    /////////////////////////////////////////////////////////////////
     trsCSVString8( "d", func_name,
        trsDouble(timeOfInfCreationBK),
        trsDouble(timeOfInfCreation1_5SliceJTree),
        trsDouble(freqCPU),
        "\n JTree inference creation ",
        "\n average time for entering evidence ",
        "\n CPU frequency " );

    trsWrite( TW_RUN | TW_CON,
        "%s performance measurement:\n\n", func_name );
    
    trsWrite( TW_RUN | TW_CON,
        "BK inference creation                            %g seconds\n"
        "1_5JTree inference creation                      %g seconds\n"        
        "CPU frequency                                    %g \n",
        timeOfInfCreationBK,
        timeOfInfCreation1_5SliceJTree,
        freqCPU );
    
    
    trsWrite( TW_RUN | TW_CON, "\n Fully factorized case:\n\n" );

    ret = CompareSmoothing(pBKInf, p1_5JTreeInf, pEvid[0], queryes, eps);
    ret = ret && CompareFiltering(pBKInf, p1_5JTreeInf, pEvid[0], queryes, eps);
    
    int lag = (int)(rand()%5)+1;
    ret = ret && CompareFixLagSmoothing(pBKInf, p1_5JTreeInf, pEvid[0], lag, queryes, eps);
    
    delete p1_5JTreeInf;
    delete pBKInf;
    //////////////////////////////////////////////////////////////////////////
    /*
     Compare exact version of BK inference and jtree
     */    
   
    pBKInf = CBKInfEngine::Create(pDBN, false);
    p1_5JTreeInf = C1_5SliceJtreeInfEngine::Create(pDBN);
  
    trsWrite( TW_RUN | TW_CON, "\n exact case BK inference :\n\n" );
    
    ret = CompareSmoothing(pBKInf, p1_5JTreeInf, pEvid[0], queryes, eps);
    ret = ret && CompareFiltering(pBKInf, p1_5JTreeInf, pEvid[0], queryes, eps);
    ret = ret && CompareFixLagSmoothing(pBKInf, p1_5JTreeInf, pEvid[0], lag, queryes, eps);
    
    delete p1_5JTreeInf;
    delete pBKInf;
    
    //////////////////////////////////////////////////////////////////////////
    
    for( slice = 0; slice < nTimeSlices; slice++ )
    {
        delete pEvid[0][slice];
    }
    
    delete pDBN;
    
    return trsResult( ret, ret == TRS_OK ? "No errors" : "BK timing FAILED");
}

int CompareFiltering( CBKInfEngine *pBKInf, C1_5SliceJtreeInfEngine *p1_5JTreeinf, 
                    pEvidencesVector &pEv, intVecVector &query, float eps)
{
    int ret = 1;  
    
    double timeOfDefFilteringBK, timeOfDefFiltering1_5JTree;
    double timeOfFilteringBK, timeOfFiltering1_5JTree;
    
    trsTimerStart(1);
    pBKInf->DefineProcedure(ptFiltering);
    trsTimerStop(1);
    timeOfDefFilteringBK = trsTimerSec(1);
    
    trsTimerStart(2);
    p1_5JTreeinf->DefineProcedure(ptFiltering);
    trsTimerStop(2);
    timeOfDefFiltering1_5JTree = trsTimerSec(2);
    
    
    int nTimeSlices = pEv.size();
    
    const CPotential *queryPotJTree, *queryPotBK;
    
    trsTimerStart(1);
    trsTimerStop(1);

    trsTimerStart(2);
    trsTimerStop(2);
    float err = 0.0f;
    int slice;
    for( slice = 0; slice < nTimeSlices; slice++ )
    {
        trsTimerContinue(1);
        pBKInf->EnterEvidence(&pEv[slice], 1);
        pBKInf->Filtering(slice);
        trsTimerStop(1);
        
        trsTimerContinue(2);
        p1_5JTreeinf->EnterEvidence(&pEv[slice], 1);
        p1_5JTreeinf->Filtering(slice);
        trsTimerStop(2);
                
        pBKInf->MarginalNodes( &query[slice].front(), query[slice].size() );
        queryPotBK = pBKInf->GetQueryJPD();
        
        p1_5JTreeinf->MarginalNodes( &query[slice].front(), query[slice].size() );
        queryPotJTree = p1_5JTreeinf->GetQueryJPD();
        
        if( !queryPotJTree->IsFactorsDistribFunEqual(queryPotBK, eps) )
        {
            ret = 0;
            break;
        }
         CNumericDenseMatrix<float> *pMat1 =  static_cast<CNumericDenseMatrix<float>*>
            (queryPotBK->GetMatrix(matTable));
        
        CNumericDenseMatrix<float> *pMat2 =  static_cast<CNumericDenseMatrix<float>*>
            (queryPotJTree->GetMatrix(matTable));
        pMat1->CombineInSelf(pMat2, 0);
        int nEl;
        const float *data;
        pMat1->GetRawData(&nEl, &data);
        floatVector diff(data, data+nEl);
        floatVector::iterator it = diff.begin();
        for(; it!= diff.end(); it++ )
        {
            (*it) = (float)fabs(*it);
        }
        float maxDiff = *std::max_element(diff.begin(), diff.end());
        err = err > maxDiff ? err : maxDiff;
              
        
    }
    timeOfFilteringBK = trsTimerSec(1);
    timeOfFiltering1_5JTree = trsTimerSec(2);
    trsWrite( TW_RUN | TW_CON,
        " BK filtering defining took                    %g seconds\n"
        " 1_5 Slice JTree filtering defining took       %g seconds\n"
        " BK inference engine took                       %g seconds\n"
        " 1_5 Slice JTree inference engine took         %g seconds\n"
        " error                                         %g \n",
       
        timeOfDefFilteringBK,
        timeOfDefFiltering1_5JTree,
        timeOfFilteringBK,
        timeOfFiltering1_5JTree,
        err);
    

    return ret;
    
}

int CompareSmoothing( CBKInfEngine *pBKInf, C1_5SliceJtreeInfEngine *p1_5JTreeinf, 
                    pEvidencesVector &pEv, intVecVector &query, float eps)
{
    int ret = 1;
    
    double timeOfDefSmoothingBK, timeOfDefSmoothing1_5JTree;
    double timeOfSmoothingBK, timeOfSmoothing1_5JTree;
    
    int nTimeSlices = pEv.size();
    trsTimerStart(1);
    pBKInf->DefineProcedure(ptSmoothing, nTimeSlices);
    trsTimerStop(1);
    timeOfDefSmoothingBK = trsTimerSec(1);
    
    trsTimerStart(2);
    p1_5JTreeinf->DefineProcedure(ptSmoothing, nTimeSlices);
    trsTimerStop(2);
    timeOfDefSmoothing1_5JTree = trsTimerSec(2);
    
    
    const CPotential *queryPotJTree, *queryPotBK;
    
   
    
    trsTimerStart(1); 
    pBKInf->EnterEvidence( &pEv.front(), nTimeSlices );
    pBKInf->Smoothing();
    trsTimerStop(1);
    timeOfSmoothingBK = trsTimerSec(1);
    
    
    trsTimerStart(2);
    p1_5JTreeinf->EnterEvidence( &pEv.front(), nTimeSlices );
    p1_5JTreeinf->Smoothing();
    trsTimerStop(2);
    timeOfSmoothing1_5JTree = trsTimerSec(2);

    float err =0.0f;
    int slice;
    for( slice = 0; slice < nTimeSlices; slice++ )
    {        
        pBKInf->MarginalNodes( &query[slice].front(), query[slice].size(), slice );
        queryPotBK = pBKInf->GetQueryJPD();
        
        p1_5JTreeinf->MarginalNodes( &query[slice].front(), query[slice].size(), slice  );
        queryPotJTree = p1_5JTreeinf->GetQueryJPD();
        
       
        
        if( !queryPotJTree->IsFactorsDistribFunEqual(queryPotBK, eps) )
        {
            ret = 0;
            break;
        }
        CNumericDenseMatrix<float> *pMat1 =  static_cast<CNumericDenseMatrix<float>*>
            (queryPotBK->GetMatrix(matTable));
        
        CNumericDenseMatrix<float> *pMat2 =  static_cast<CNumericDenseMatrix<float>*>
            (queryPotJTree->GetMatrix(matTable));
        pMat1->CombineInSelf(pMat2, 0);
        int nEl;
        const float *data;
        pMat1->GetRawData(&nEl, &data);
        floatVector diff(data, data+nEl);
        floatVector::iterator it = diff.begin();
        for(; it!= diff.end(); it++ )
        {
            (*it) = (float)fabs(*it);
        }
        float maxDiff = *std::max_element(diff.begin(), diff.end());
        err = err > maxDiff ? err : maxDiff;
              
            
    }
    
    
    trsWrite( TW_RUN | TW_CON,
        " BK smoothing defining took                    %g seconds\n"
        " 1_5 Slice JTree smoothing defining took       %g seconds\n"
        " BK inference engine took                       %g seconds\n"
        " 1_5 Slice JTree inference engine took         %g seconds\n"
        " error                                         %g \n",
        
        timeOfDefSmoothingBK,
        timeOfDefSmoothing1_5JTree,
        timeOfSmoothingBK,
        timeOfSmoothing1_5JTree,
        err);
    
        
    return ret;
    
}

int CompareFixLagSmoothing( CBKInfEngine *pBKInf, C1_5SliceJtreeInfEngine *p1_5JTreeinf, 
                          pEvidencesVector &pEv, int lag, intVecVector &query, float eps)
{
    
    double timeOfDefFLSmoothingBK, timeOfDefFLSmoothing1_5JTree;
    double timeOfFLSmoothingBK, timeOfFLSmoothing1_5JTree;
    int ret = 1;
    int nTimeSlices = pEv.size();
    
    const CPotential *queryPotJTree, *queryPotBK;
    
    trsTimerStart(1);
    pBKInf->DefineProcedure(ptFixLagSmoothing, lag);
    trsTimerStop(1);
    timeOfDefFLSmoothingBK = trsTimerSec(1);
    
    trsTimerStart(2);
    p1_5JTreeinf->DefineProcedure(ptFixLagSmoothing, lag);
    trsTimerStop(2);
    timeOfDefFLSmoothing1_5JTree = trsTimerSec(2);
    
    int slice;
    trsTimerStart(1);
    trsTimerStop(1);
    trsTimerStart(2);
    trsTimerStop(2);
    for( slice = 0; slice < lag; slice++ )
    {
        trsTimerContinue(1);
        pBKInf->EnterEvidence(&pEv[slice], 1);
        trsTimerStop(1);

        trsTimerContinue(2);
        p1_5JTreeinf->EnterEvidence(&pEv[slice], 1);
        trsTimerStop(2);
    }
    
    float err = 0.0f;
    for( ; slice < nTimeSlices; slice++ )
    {
        trsTimerContinue(1);
        pBKInf->EnterEvidence(&pEv[slice], 1);
        pBKInf->FixLagSmoothing(slice);
        trsTimerStop(1);

        trsTimerContinue(2);
        p1_5JTreeinf->EnterEvidence(&pEv[slice], 1);
        p1_5JTreeinf->FixLagSmoothing(slice);
        trsTimerStop(2);
               
        
        pBKInf->MarginalNodes( &query[slice-lag].front(), query[slice-lag].size() );
        queryPotBK = pBKInf->GetQueryJPD();
        
        p1_5JTreeinf->MarginalNodes( &query[slice-lag].front(), query[slice-lag].size() );
        queryPotJTree = p1_5JTreeinf->GetQueryJPD();
      
        if( !queryPotJTree->IsFactorsDistribFunEqual(queryPotBK, eps) )
        {
            ret = 0;
            break;
        }
        CNumericDenseMatrix<float> *pMat1 =  static_cast<CNumericDenseMatrix<float>*>
            (queryPotBK->GetMatrix(matTable));
        
        CNumericDenseMatrix<float> *pMat2 =  static_cast<CNumericDenseMatrix<float>*>
            (queryPotJTree->GetMatrix(matTable));
        pMat1->CombineInSelf(pMat2, 0);
        int nEl;
        const float *data;
        pMat1->GetRawData(&nEl, &data);
        floatVector diff(data, data+nEl);
        floatVector::iterator it = diff.begin();
        for(; it!= diff.end(); it++ )
        {
            (*it) = (float)fabs(*it);
        }
        float maxDiff = *std::max_element(diff.begin(), diff.end());
        err = err > maxDiff ? err : maxDiff;
        
    }
    timeOfFLSmoothingBK = trsTimerSec(1);
    timeOfFLSmoothing1_5JTree = trsTimerSec(2);
    trsWrite( TW_RUN | TW_CON,
        " BK fix lag smoothing defining took                     %g seconds\n"
        " 1_5 Slice JTree fix lag smoothing defining took        %g seconds\n"
        " BK inference engine took                                %g seconds\n"
        " 1_5 Slice JTree inference engine took                  %g seconds\n"
        " error                                                   %g \n",

        
        timeOfDefFLSmoothingBK,
        timeOfDefFLSmoothing1_5JTree,
        timeOfFLSmoothingBK,
        timeOfFLSmoothing1_5JTree,
        err );
    


    return ret;
    
    
}


void initTBKInfEngine()
{
    trsReg( func_name, test_desc, test_class, timeBKInfEngine );
}