/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AReduceDims.cpp                                             //
//                                                                         //
//  Purpose:   Test on reduce operation algorithm on dense matrix          //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include <math.h>

#include "trsapi.h"
#include "pnl_dll.hpp"

#include "pnlExampleModels.hpp"
#include "pnlExInferenceEngine.hpp"
#include "pnlJtreeInferenceEngine.hpp"
#include "tCreateRandStructs.hpp"

PNL_USING

static char func_name[] = "testExInfEngine";

static char *test_desc = "comparing Naive engine and CExInfEngine atop JTree on disconnected models";

static char *test_class = "Algorithm";

const float eps = 1e-4f;

int testExInfEngine()
{
    int rv = TRS_OK;

    int i, j;
    int test_num;
    int num_nodes, degree;
    float edge_prob;
    CBNet *bn;
    CEvidence *ev;
    intVector query;
    intVector candidates;
    int perm[200];
    int maxi_flag;

    PNL_DEFINE_TACTER( TER, 15 );

    for ( test_num = 0; test_num < 96; ++test_num )
    {
#if 0
        maxi_flag = tTurboRand( 2 );
#else
        maxi_flag = 0;
#endif
        num_nodes = tTurboRand( 7, 12 );
        degree = 3;
        edge_prob = tTurboRand( 10, 90 ) / 100.f;
        i = tTurboRand( 2, 5 );
        j = tTurboRand( 2, 5 );
#if 0
        std::cout << std::endl << num_nodes << " " << degree << std::endl;
        std::cout << std::endl << i << " " << j << std::endl;
#endif
        bn = pnlExCreateRandomBNet( num_nodes, degree, 0, i, j, edge_prob, true );
        std::cout << "/o";
        std::cout.flush();

#if 0
        std::cout << "orig graph" << std::endl;
        bn->GetGraph()->Dump();
#endif

        CNaiveInfEngine *neng = CNaiveInfEngine::Create( bn );
#if 1
        CExInfEngine< CNaiveInfEngine, CBNet > *supereng1 = CExInfEngine< CNaiveInfEngine, CBNet >::Create( bn );
        CExInfEngine< CJtreeInfEngine, CBNet, PNL_EXINFENGINEFLAVOUR_ALL > *supereng2 = CExInfEngine< CJtreeInfEngine, CBNet, PNL_EXINFENGINEFLAVOUR_ALL >::Create( bn );
#endif
//        CJtreeInfEngine *jeng = CJtreeInfEngine::Create( bn );

        tCreateRandomPermutation( num_nodes, perm );
        query.resize( 0 );

//      std::cout << "orig query" << std::endl;
        for ( i = 0; i < tTurboRand( 1, num_nodes - 1 ); ++i )
        {
            query.push_back( perm[i] );
  //          std::cout << perm[i] << std::endl;
        }
        candidates.resize( 0 );
    //  std::cout << "orig cand" << std::endl;
        for ( ; i < num_nodes; ++i )
        {
            candidates.push_back( perm[i] );
//          std::cout << perm[i] << std::endl;
        }

        ev = tCreateRandomEvidence( bn, tTurboRand( 1, candidates.size() ), candidates );

#if 0
        std::cout << "orig ev" << std::endl;
        ev->Dump();
#endif

        neng->EnterEvidence( ev, maxi_flag );
#if 1
        supereng1->EnterEvidence( ev, maxi_flag );
        supereng2->EnterEvidence( ev, maxi_flag );
#endif
      //  jeng->EnterEvidence( ev, maxi_flag );

        tCreateRandomPermutation( num_nodes, perm );

//        std::cout << "doing MarginalNodes.." << std::endl;

        neng->MarginalNodes( &query.front(), query.size() );
  //      std::cout << "neng MarginalNodes done" << std::endl;
#if 1
        supereng1->MarginalNodes( &query.front(), query.size() );
//        std::cout << "supereng1 MarginalNodes done" << std::endl;
//        supereng2->MarginalNodes( &query.front(), 1 );
        supereng2->MarginalNodes( &query.front(), query.size() );
      //  std::cout << "supereng2 MarginalNodes done" << std::endl;
#endif
//        jeng->MarginalNodes( &query.front(), 1 );
        
        if ( maxi_flag )
        {
            neng->GetMPE();
#if 1
            supereng1->GetMPE();
            supereng2->GetMPE();
#endif
        }
        else
        {
            CPotential const *pot[4];
//            std::cout << "doing QueryJPD.." << std::endl;
            pot[0] = neng->GetQueryJPD();
  //          std::cout << "neng QueryJPD done" << std::endl;
//            std::cout << "naive gave us" << std::endl;
//            pot[0]->Dump();
#if 1
            pot[1] = supereng1->GetQueryJPD();
    //        std::cout << "supereng1 QueryJPD done" << std::endl;
  //          std::cout << "Ex atop naive gave us" << std::endl;
  //          pot[1]->Dump();
            pot[2] = supereng2->GetQueryJPD();
      //      std::cout << "supereng2 QueryJPD done" << std::endl;
  //          std::cout << "Ex atop jtree gave us" << std::endl;
    //        pot[2]->Dump();
#endif
      //      pot[3] = jeng->GetQueryJPD();
        //    std::cout << "jtree gave us" << std::endl;
          //  pot[3]->Dump();
        }

        delete( ev );
        delete( neng );
#if 1
        delete( supereng1 );
        delete( supereng2 );
#endif
  //      delete( jeng );

        PNL_TACTER_STEP( TER, std::cout << "k\\"; std::cout.flush(), std::cout << "k\\" << std::endl );
    }

    PNL_RELEASE_TACTER( TER );
    return trsResult( rv, rv == TRS_OK ? "No errors" : "ExInfEngine failed" );
}

void initAExInfEngine()
{
    trsReg( func_name, test_desc, test_class, &testExInfEngine );
}
