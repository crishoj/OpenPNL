/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      classes_view.cpp                                            //
//                                                                         //
//  Purpose:   one of PNL applications (mainly for testing)                //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#define  PNL_DO_TIMING

#include "pnl_dll.hpp"


#include <sstream>
#include <fstream>
#include <time.h>

using namespace std;

PNL_USING


void GibbsForInceneratorBNet( int seed = (unsigned)time(0))
{

   
    CBNet *pBnet;
    pBnet = pnlExCreateAsiaBNet();
    
    const int numOfObsNds  = 2;
    const int obsNds[]     = { 0, 3 };
   
    valueVector obsNdsVals(2);
    obsNdsVals[0].SetInt(1);
    obsNdsVals[1].SetInt(0);
    CEvidence *pEvidence = CEvidence::Create( pBnet, numOfObsNds, obsNds,
        obsNdsVals );

    
    CGibbsSamplingInfEngine *pGibbsInf;
    pGibbsInf = CGibbsSamplingInfEngine::Create( pBnet );
    pGibbsInf->SetMaxTime( 100000);
    pGibbsInf->SetBurnIn( 10000 );


    intVecVector queries(2);
    queries[0].clear();
    
    queries[0].push_back( 2 );
    queries[0].push_back( 7 );
    
    queries[1].push_back( 1 );
    queries[1].push_back( 4 );
   
    
    pGibbsInf->SetQueries( queries );

    pGibbsInf->EnterEvidence( pEvidence );
    
    const int querySz1 = 1;
    const int query1[] = { 2 };
    pGibbsInf->MarginalNodes( query1, querySz1 );
    
    const CPotential *pQueryPot;
    pQueryPot = pGibbsInf->GetQueryJPD();
    
    std::cout<<"result of gibbs"<<std::endl<<std::endl;
    pQueryPot->Dump();
   
    delete pEvidence;
    delete pGibbsInf;
    delete pBnet;
    ///////////////////////////////////////////////////////////////////////////////

}
int main()
{
        
   GibbsForInceneratorBNet();
    return 0;
}