/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AGeneralInference.cpp                                       //
//                                                                         //
//  Purpose:   general test of inference with JTreeInfEngine               //
//             and other engines                                           //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include "trsapi.h"

#include "pnl_dll.hpp"
#include "tCreateRandStructs.hpp"

PNL_USING

static inline int MyRand( int k )
{
    int r = rand();
//    std::cout << "aaaa " << r << std::endl;
//    r = r / (RAND_MAX / k + 1);
//    std::cout << "rand " << r << std::endl;
//    std::cout << "rrrr " << RAND_MAX << " " << k << " " << RAND_MAX / k << std::endl;
    return r % k;
}

static inline int MyRand( int low, int high )
{
    return MyRand( high - low + 1 ) + low;
}

static char my_func_name[] = "testGeneralInference";
static char* my_test_desc = "test jtree vs naive on random dags";
static char* my_test_class = "Algorithm";

#define PNL_TEST_JTREE_GRAPH_MAX_SIZE 200
#define PNL_TEST_JTREE_MAX_STATES 3
#define PNL_TEST_JTREE_MAX_CONT_DIM 7

union PnlVar
{
    int i;
    float fl;
    void *ptr;
};

int testGeneralInference()
{
#if 0
    int i, j, k, a, s, n, m;
    int sizes[] = { 5, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 12, 13, 12, 11, 11, 11, 5 };
    const int num_tests = sizeof( sizes ) / sizeof( int );
    int num_edges[num_tests] = { 8, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 60, 20, 30, 40, 50, 50, 6 };
    int num_g[num_tests] = { 4, 1, 1, 4, 4, 4, 4, 4, 4, 4, 4, 3, 2, 1, 2, 1, 11, 5 };
    CNodeType node_types[PNL_TEST_JTREE_GRAPH_MAX_SIZE];
    int node_assoc[PNL_TEST_JTREE_GRAPH_MAX_SIZE];
    CGraph *grafik;
    CBNet *bn;
    CCPD *cpd;
    intVector pars;
    const CNodeType *nt[PNL_TEST_JTREE_GRAPH_MAX_SIZE];
    int dm[PNL_TEST_JTREE_GRAPH_MAX_SIZE];
    float *data;
    int vl[PNL_TEST_JTREE_GRAPH_MAX_SIZE];
    float bulk[PNL_TEST_JTREE_GRAPH_MAX_SIZE * 100];
    CJtreeInfEngine *jtie;
    CNaiveInfEngine *nbie;
    CEvidence *ev;
    char ev_vals[PNL_TEST_JTREE_GRAPH_MAX_SIZE * PNL_TEST_JTREE_MAX_CONT_DIM * sizeof( PnlVar )];
    char *polzunok;
    
    try{
    std::cout << "phase 1" << std::endl;
    int seed = pnlTestRandSeed();
    std::cout << "seed" << seed << std::endl;
    srand( seed );

    for ( i = num_tests; i--; )
    {
lx:
	std::cout << "gg" << std::endl;
	grafik = CreateRandomDAG( sizes[i], num_edges[i], true );
	if ( grafik->NumberOfConnectivityComponents() != 1 )
	{
	    delete grafik;
	    goto lx;
	}
	std::cout << "phase 2" << std::endl;
	for ( j = sizes[i]; j--; )
	{
#if 1
	    node_types[j].SetType( 1, MyRand( 2, PNL_TEST_JTREE_MAX_STATES ) );
#else
	    node_types[j].SetType( 1, 2 );
#endif

#if 0
	    node_assoc[j] = MyRand( sizes[i] );
#else
	    node_assoc[j] = j;
#endif
	}
	for ( j = num_g[i]; j--; )
	{
	    node_types[j].SetType( 0, MyRand( 1, PNL_TEST_JTREE_MAX_CONT_DIM ) );
	}
	for ( a = sizes[i] - 1; a--; )
	{
	    for ( j = sizes[i]; j--; )
	    {
		pars.clear();
		grafik->GetChildren( j, &pars );
		for ( k = pars.size(); k--; )
		{
		    node_types[pars[k]].SetType( 0, MyRand( 1, PNL_TEST_JTREE_MAX_CONT_DIM ) );
		}
	    }
	}
	std::cout << "node_types and assoc follow" << std::endl;
	for ( j = 0; j < sizes[i]; ++j )
	{
	    std::cout << node_types[j].IsDiscrete() << " " << node_types[j].GetNodeSize() << " " << node_assoc[j] << std::endl;
	}
	std::cout << "phase 3" << std::endl;
	std::cout << "i " << i << ", sizes[i] " << sizes[i] << std::endl;
	grafik->Dump();
	bn = CBNet::Create( sizes[i], sizes[i], node_types, node_assoc, grafik );
	CModelDomain* pMD = bn->GetModelDomain();
	bn->AllocFactors();
	std::cout << "phase 4" << std::endl;
	for ( k = sizes[i]; k--; )
	{
	    pars.clear();
	    grafik->GetParents( k, &pars );
	    std::cout << "ps " << pars.size() << std::endl;
	    for ( nt[j = pars.size()] = bn->GetNodeType( k ), dm[j] = k,
		  s = node_types[node_assoc[k]].GetNodeSize(); j--; )
	    {
		std::cout << "pp " << pars[j] << std::endl;
		nt[j] = bn->GetNodeType( pars[j] );
		std::cout << "ppdone" << std::endl;
		dm[j] = pars[j];
		s *= node_types[node_assoc[pars[j]]].GetNodeSize();
	    }
	    std::cout << "phase 5" << std::endl;
	    if ( node_types[k].IsDiscrete() )
	    {
#if 0
		cpd = CTabularCPD::CreateUnitFunctionCPD( dm, pars.size() + 1, pMD );
#else
		std::cout << "data allocated for " << s << " floats" << std::endl;
		data = new( float[s] );
		for ( j = s; j--; )
		{
		    data[j] = (float)MyRand( 10 ) / 10;
		}
		std::cout << "phase 6" << std::endl;
		std::cout << "ps " << pars.size() << std::endl;
		for ( j = 0; j < pars.size() + 1; ++j )
		{
		    std::cout << nt[j]->IsDiscrete() << " " << nt[j]->GetNodeSize() << " " << dm[j] << std::endl;
		}
		cpd = CTabularCPD::Create( dm, pars.size() + 1, pMD, data );
		std::cout << "phase 7" << std::endl;
		delete( data );
		std::cout << "phase 8" << std::endl;
#endif
	    }
	    else
	    {
		float mean[20];
		float cov[400];
		data = new( float[s] );
		std::cout << "data allocated for " << s << " floats" << std::endl;
		float *arr[PNL_TEST_JTREE_GRAPH_MAX_SIZE];
		for ( j = sizes[i]; j--; )
		{
		    arr[j] = data;
		}
		for ( j = 20; j--; )
		{
		    mean[j] = (float)MyRand( 10 ) / 10;
		}
		n = nt[pars.size()]->GetNodeSize();
		std::cout << "n " << n << std::endl;
		for ( j = n; j--; )
		{
		    for ( a = j + 1; a--; )
		    {
		        cov[a + j * n] = cov[j + a * n] = (float)MyRand( 10 ) / 10 + 2;
		    }
		}
		for ( j = n; j--; )
		{
		    for ( a = n; a--; )
		    {
		        std::cout << cov[a + j * n] << " ";
		    }
		    std::cout << std::endl;
		}
		for ( j = s; j--; )
		{
		    data[j] = (float)MyRand( 10 ) / 10;
		}
		std::cout << "wot" << std::endl;
		cpd = CGaussianCPD::Create( dm, pars.size() + 1, pMD );
		((CGaussianCPD *)cpd)->AllocDistribution( mean, cov, MyRand( 1, 1000 ), arr );
		delete( data );
	    }
	    std::cout << "phase 8.5" << std::endl;
	    cpd->NormalizeCPD();
	    std::cout << "phase 9" << std::endl;
	    bn->AttachFactor( cpd );
	    std::cout << "phase 10" << std::endl;
//            delete( cpd );
	}
	std::cout << "phase 10" << std::endl;
	jtie = CJtreeInfEngine::Create( bn );
	std::cout << "phase 11" << std::endl;
	nbie = CNaiveInfEngine::Create( bn );
	std::cout << "phase 12" << std::endl;
	for ( k = 3; k--; )
	{
	    std::cout << "test ber " << i << " " << k << std::endl;
	    j = MyRand( 1, sizes[i] );
//            std::cout << "ev: ";
	    for ( s = 0, polzunok = ev_vals; s < j; ++s )
	    {
lp:
		dm[s] = MyRand( sizes[i] );
//                std::cout << "probe " << dm[s] << std::endl;
		for ( a = s; a--; )
		{
//                    std::cout << "checking dm[" << a << "] " << dm[a] << std::endl;
		    if ( dm[a] == dm[s] )
		    {
		        goto lp;
		    }
		}
//                std::cout << dm[s] << ' ';
		if ( node_types[node_assoc[dm[s]]].IsDiscrete() )
		{
		    *(int *)polzunok = MyRand( node_types[node_assoc[dm[s]]].GetNodeSize() );
		    polzunok += sizeof( int );
		}
		else
		{
		    for ( n = node_types[node_assoc[dm[s]]].GetNodeSize(); n--; )
		    {
		        *(int *)polzunok = MyRand( node_types[node_assoc[dm[s]]].GetNodeSize() );
		        polzunok += sizeof( int );
		    }
		}

//                std::cout << vl[s] << ' ';
	    }
//            std::cout << std::endl;
	    std::cout << "ev: ";
	    for ( s = 0; s < j; ++s )
	    {
		std::cout << dm[s] << " " << vl[s] << ", ";
	    }
	    std::cout << std::endl;
	    std::cout << "phase 13" << std::endl;
	    ev = CEvidence::Create( bn, j, dm, ev_vals );
	    std::cout << "phase 14" << std::endl;

	    jtie->EnterEvidence( ev );
	    std::cout << "phase 15" << std::endl;
	    nbie->EnterEvidence( ev );
	    std::cout << "phase 16" << std::endl;

	    for ( a = 100; a--; )
	    {
#if 0
		j = MyRand( 1, sizes[i] );
		std::cout << "marg: ";
		for ( s = 0; s < j; ++s )
		{
lb:
		    dm[s] = MyRand( sizes[i] );
		    for ( a = s; a--; )
		    {
		        if ( dm[a] == dm[s] )
		        {
		            goto lb;
		        }
		    }
		    std::cout << dm[s] << ' ';
		}
#else
ll:
		j = MyRand( sizes[i] );
		std::cout << "marg: ";
		pars.clear();
		grafik->GetParents( j, &pars );
		std::cout << "ps " << pars.size() << std::endl;
		j = 0;
		for ( s = 0; s < pars.size(); ++s )
		{
		    if ( MyRand( 8 ) < 6 )
		    {
		        dm[j++] = pars[s];
		        std::cout << pars[s];
		    }
		}
		std::cout << std::endl;
		if ( j == 0 ) goto ll;
#endif
		jtie->MarginalNodes( dm, j );
		std::cout << "phase 17" << std::endl;
		jtie->GetQueryJPD()->Dump();
		std::cout << "phase 17" << std::endl;

       	        nbie->MarginalNodes( dm, j );
		std::cout << "phase 18" << std::endl;
		nbie->GetQueryJPD()->Dump();
		std::cout << "phase 19" << std::endl;
	    }
	    delete( ev );
	}
	std::cout << "phase 20" << std::endl;
	delete( jtie );
	std::cout << "phase 21" << std::endl;
	delete( nbie );
	std::cout << "phase 22" << std::endl;
	delete( bn );
	std::cout << "phase 23" << std::endl;
//        delete( grafik );
	std::cout << "phase 24" << std::endl;
    }
    std::cout << "done wrode" << std::endl;
    for ( ;; );
    }
    catch( CException &exc )
    {
	std::cout << exc.GetMessage() << std::endl;
    }
#endif
    return TRS_OK;

}

void initAGeneralInference()
{
    trsReg(my_func_name, my_test_desc, my_test_class, &testGeneralInference);
}
