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
#include "tCreateRandStructs.hpp"

PNL_USING

static char func_name[] = "testDSeparation";

static char *test_desc = "stress test of DSeparation related stuff";

static char *test_class = "Algorithm";

const float eps = 1e-4f;

int testDSeparation()
{
    int rv = TRS_OK;

    int i, j;
    int test_num;
    int num_nodes, num_edges;
    int perm[10000];
    bool mark[10000];
    intVector sep, lst;
    CGraph *gr;
    intVecVector tbl;
//  clock_t cl;

    PNL_DEFINE_TACTER( TER, 15 );

    for ( test_num = 0; test_num < 48; ++test_num )
    {
        num_nodes = tTurboRand( 1, 150 );
        num_edges = tTurboRand( 0, num_nodes * (num_nodes - 1) / 2 );
#if 0
        std::cout << std::endl << num_nodes << " " << num_edges << std::endl;
#endif
        gr = tCreateRandomDAG( num_nodes, num_edges, false );
        std::cout << "/o";
        std::cout.flush();
#if 0
        gr->Dump();
#endif

        tCreateRandomPermutation( num_nodes, perm );
        sep.clear();
#if 0
        std::cout << "sep";
        for ( i = tTurboRand( num_nodes ); i--; )
        {
            sep.push_back( perm[i] );
            std::cout << " " << perm[i];
        }
        std::cout << std::endl;
#endif

        tbl.clear();
//      cl = clock();
        gr->GetDConnectionTable( sep, &tbl );
//      std::cout << "clock " << (clock() - cl) / (float)CLOCKS_PER_SEC << std::endl;

//      cl = clock();
        for ( i = 0; i < num_nodes; ++i )
        {
            lst.clear();
            gr->GetDConnectionList( i, sep, &lst );

            if ( lst.size() != tbl[i].size() )
            {
                std::cout << "size mismatch" << std::endl;
                return TRS_FAIL;
            }
            for ( j = num_nodes; j--; )
            {
                mark[j] = true;
            }
            for ( j = 0; j < lst.size(); ++j )
            {
                mark[lst[j]] = false;
            }
            for ( j = 0; j < lst.size(); ++j )
            {
                if ( mark[tbl[i][j]] )
                {
                    std::cout << "lists mismatch" << std::endl;
                    return TRS_FAIL;
                }
            }
#if 0
            std::cout << "i" << i;
            for ( j = 0; j < lst.size(); ++j )
            {
                std::cout << " " << lst[j];
            }
            std::cout << std::endl;
#endif
        }
//      std::cout << "clock " << (clock() - cl) / (float)CLOCKS_PER_SEC << std::endl;

        PNL_TACTER_STEP( TER, std::cout << "k\\"; std::cout.flush(), std::cout << "k\\" << std::endl );
    }

    PNL_RELEASE_TACTER( TER );
    return trsResult( rv, rv == TRS_OK ? "No errors" : "DSeparation FAILED" );
}

void initADSeparation()
{
    trsReg( func_name, test_desc, test_class, &testDSeparation );
}
