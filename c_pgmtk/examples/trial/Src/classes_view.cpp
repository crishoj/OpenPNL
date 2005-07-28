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

inline static void DumpQueryJPDForNode( pnl::CInfEngine* pInfEngine,
                                        int queryNode, const std::string&
                                        logfilename = "" )
{
    if( logfilename.length() > 0 )
    {
	std::ofstream logFileStream( logfilename.c_str(), std::ios::app );
        pnl::LogDrvStream logDriver( &logFileStream, pnl::eLOG_ALL,
            pnl::eLOGSRV_ALL );

        pInfEngine->MarginalNodes( &queryNode, 1 );

        pInfEngine->GetQueryJPD()->Dump();
    }
    else
    {
        pInfEngine->MarginalNodes( &queryNode, 1 );

        pInfEngine->GetQueryJPD()->Dump();
    }
}
//////////////////////////////////////////////////////////////////////////

void CompareInferenceResults(const std::string& logfilename = "")
{
    // 1. create model to carry out inference for
    pnl::CBNet* pBNet = pnl::pnlExCreateBNetFormSMILE();

    // 2. create inference engines
    pnl::CPearlInfEngine *pPearlEng    = pnl::CPearlInfEngine::Create(pBNet);

    pnl::CJtreeInfEngine *pJTreeEng    = pnl::CJtreeInfEngine::Create(pBNet);
    
    pnl::CJtreeInfEngine *pJTreeEngCpy = pnl::CJtreeInfEngine::Copy(pJTreeEng);
    
    
    // 3. create appropriate evidence for inference
    // tabular Evidence for any tabular BNet:
    const int numOfObsNds  = 2;
    
    const int obsNds[]     = { 0, 3 };
    // different values for observed nodes (uncomment one)
    pnl::valueVector obsNdsVals(2);
    
    obsNdsVals[0].SetInt(1);
    obsNdsVals[1].SetInt(0);

    pnl::CEvidence *pEvidence = pnl::CEvidence::Create( pBNet, numOfObsNds,
        obsNds, obsNdsVals );
    

    // 4. start engines with entering evidence
    pPearlEng->EnterEvidence(pEvidence);
    
    pJTreeEng->EnterEvidence(pEvidence);
    
    pJTreeEngCpy->EnterEvidence(pEvidence);
    
    
    // 5. marginalize JPD to nodes in query and dump them to standard output
    for( int i = 0, numOfNds = pBNet->GetNumberOfNodes(); i < numOfNds; ++i )
    {
        std::cout << std::endl;

        std::cout << " JTree Inf Engine " << std::endl;
        DumpQueryJPDForNode( pJTreeEng,    i, logfilename );
        
        std::cout << " JTree Inf Engine Copy " << std::endl;
        DumpQueryJPDForNode( pJTreeEngCpy, i, logfilename );
        
        std::cout << " Pearl Inf Engine " << std::endl;
        DumpQueryJPDForNode( pPearlEng,    i, logfilename );
        
        std::cout << std::endl;
    }
    
    std::cin.get();

    // 6. destroy things
    delete pEvidence;
    
    //pnl::CJtreeInfEngine::Release(&pJTreeEng);
    delete pJTreeEng;
    
    //pnl::CJtreeInfEngine::Release(&pJTreeEngCpy);
    delete pJTreeEngCpy;
    
    //pnl::CPearlInfEngine::Release(&pPearlEng);    
    delete pPearlEng;
    
    delete pBNet;
}
//////////////////////////////////////////////////////////////////////////

int main()
{
//     pnl::CMRF2* pMRF2 = pnl::pnlExCreateBigMRF2( 1, 3, 3, 4, 2.5f, 2.5f );

    CompareInferenceResults("c:\\log");

//    pnl::CJtreeInfEngine* pJTreeInfEng = pnl::CJtreeInfEngine::Create(pMRF2);
//    pnl::CJtreeInfEngine* pJTreeInfEngCpy = pnl::CJtreeInfEngine::Copy(pJTreeInfEng);
//    pnl::CNaiveInfEngine* pNaiveInfEng = pnl::CNaiveInfEngine::Create(pMRF2);
//
//    pnl::intVector obsnds(2);
//    pnl::valueVector obsndsvals(2);
//
//    obsnds[0] = 0;
//    obsnds[1] = 1;
//
//    obsndsvals[0].SetInt(1);
//    obsndsvals[1].SetInt(2);
//
//
//    pnl::CEvidence* pEvidence = pnl::CEvidence::Create( pMRF2, obsnds,
//        obsndsvals );
//
//    pJTreeInfEng->EnterEvidence(pEvidence);
//    pJTreeInfEngCpy->EnterEvidence(pEvidence);
//    pNaiveInfEng->EnterEvidence(pEvidence);
//
//    for( int i = 0; i < 6; ++i )
//    {
//        pnl::intVector query(2);
//        query[0] = i;
//        query[1] = i + 3;
//
//        pJTreeInfEng->MarginalNodes( &query.front(), 2 );
//        const pnl::CPotential* pQPot1 = pJTreeInfEng->GetQueryJPD();
//        pJTreeInfEngCpy->MarginalNodes( &query.front(), 2 );
//        const pnl::CPotential* pQPot2 = pJTreeInfEng->GetQueryJPD();
//        pNaiveInfEng->MarginalNodes( &query.front(), 2 );
//        const pnl::CPotential* pQPot3 = pNaiveInfEng->GetQueryJPD();
//
//        std::cout << "--------------------------------------" << std::endl;
//        pQPot1->Dump();
//        pQPot2->Dump();
//        pQPot3->Dump();
//    }
//    
//
//    delete pJTreeInfEng;
//    delete pJTreeInfEngCpy;
//    delete pNaiveInfEng;
//    delete pEvidence;
//    delete pMRF2;
    return 0;
}

