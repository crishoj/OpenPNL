/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      tCreateRandStructs.cpp                                      //
//                                                                         //
//  Purpose:   Creation of some random structures, currently permutations, //
//             DAGs and static BNets                                       //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <iostream>

#include "tCreateRandStructs.hpp"
#include "pnlImpDefs.hpp"

PNL_USING

void tCreateRandomPermutation( int n, int buf[] )
{
    int i, j, k;

    for ( i = n; i--; )
    {
        buf[i] = i;
    }
    for ( i = tTurboRand( n ) + tTurboRand( n ) + tTurboRand( 10 ), k = 0; i--; )
    {
        k += tTurboRand( n );
    }
    while ( k-- )
    {
        i = tTurboRand( n );
        j = tTurboRand( n );
        std::swap( buf[i], buf[j] );
    }
}

CGraph *tCreateRandomDAG( int num_nodes, int num_edges, bool top_sorted )
{
    CGraph *retval;
    int i, j, k;
    int est;

    PNL_DEFINE_AUTOBUF( int, perm, 64 );
    PNL_DEFINE_AUTOBUF( int, nnbrs, 64 );
    PNL_DEFINE_AUTOBUF( int *, lst1, 64 );
    PNL_DEFINE_AUTOBUF( ENeighborType *, lst2, 64 );
    PNL_DEFINE_AUTOBUF( int, elst, 2048 );
    PNL_DEFINE_AUTOBUF( int, vtx1, 2048 );
    PNL_DEFINE_AUTOBUF( int, vtx2, 2048 );

    if ( num_nodes <= 0
         || (unsigned)num_edges > (est = (num_nodes * (num_nodes - 1)) / 2) )
    {
        PNL_THROW( CInconsistentSize, "tCreateRandomDAG: params no good" );
    }
    PNL_DEMAND_AUTOBUF( perm, num_nodes );
    PNL_DEMAND_AUTOBUF( nnbrs, num_nodes );
    PNL_DEMAND_AUTOBUF( lst1, num_nodes );
    PNL_DEMAND_AUTOBUF( lst2, num_nodes );

    if ( top_sorted )
    {
        for ( i = num_nodes; i--; )
        {
            perm[i] = i;
        }
    }
    else
    {
        tCreateRandomPermutation( num_nodes, perm );
    }
    for ( i = num_nodes; i--; )
    {
        nnbrs[i] = 0;
        lst1[i] = 0;
        lst2[i] = 0;
    }
    retval = CGraph::Create( num_nodes, nnbrs, lst1, lst2 );
    if ( num_edges <= est * 3 / 4 )
    {
        for ( i = num_edges; i--; )
        {
            do
            {
                j = tTurboRand( num_nodes );
                k = tTurboRand( num_nodes );
            } while ( k <= j || retval->IsExistingEdge( perm[j], perm[k] ) );
            retval->AddEdge( perm[j], perm[k], 1 );
        }
    }
    else
    {
        PNL_DEMAND_AUTOBUF( elst, est );
        PNL_DEMAND_AUTOBUF( vtx1, est );
        PNL_DEMAND_AUTOBUF( vtx2, est );

        for ( i = num_nodes, k = 0; --i; )
        {
            for ( j = i; j--; )
            {
                vtx1[k] = j;
                vtx2[k++] = i;
            }
        }
        tCreateRandomPermutation( est, elst );
        for ( i = num_edges; i--; )
        {
            retval->AddEdge( perm[vtx1[elst[i]]], perm[vtx2[elst[i]]], 1 );
        }
    }
    PNL_RELEASE_AUTOBUF( vtx2 );
    PNL_RELEASE_AUTOBUF( vtx1 );
    PNL_RELEASE_AUTOBUF( elst );
    PNL_RELEASE_AUTOBUF( lst2 );
    PNL_RELEASE_AUTOBUF( lst1 );
    PNL_RELEASE_AUTOBUF( nnbrs );
    PNL_RELEASE_AUTOBUF( perm );

    return retval;
}

CBNet *tCreateRandomBNet( int num_nodes, int num_edges,
                          int max_states, int max_dim,
                          int gaussian_seed,
                          bool no_gaussian_parent_with_discrete_child_please,
                          bool no_gaussian_child_with_discrete_parent_please )
{
    int j, k, s, n, a;
    CGraph *gr;
    intVector pars;
    CBNet *bn;
    CCPD *cpd;
    float *data;

    PNL_DEFINE_AUTOBUF( CNodeType, node_types, 32 );
    PNL_DEFINE_AUTOBUF( int, node_assoc, 32 );
    PNL_DEFINE_AUTOBUF( CNodeType const *, nt, 32 );
    PNL_DEFINE_AUTOBUF( int, dm, 32 );
    PNL_DEFINE_AUTOBUF( float *, arr, 32 );
    PNL_DEFINE_AUTOBUF( float, mean, 20 );
    PNL_DEFINE_AUTOBUF( float, cov, 20 * 20 );

    if ( max_states <= 0 || max_dim <= 0 || gaussian_seed < 0 )
    {
        PNL_THROW( CInconsistentSize, "tCreatePeculiarRandomBNet: params no good" );
    }
    if ( gaussian_seed )
    {
        PNL_DEMAND_AUTOBUF( mean, max_dim );
        PNL_DEMAND_AUTOBUF( cov, max_dim * max_dim );
    }

lx:
    gr = tCreateRandomDAG( num_nodes, num_edges, true );
    if ( gr->NumberOfConnectivityComponents() != 1 )
    {
        delete( gr );
        goto lx;
    }
    PNL_DEMAND_AUTOBUF( node_types, num_nodes );
    PNL_DEMAND_AUTOBUF( node_assoc, num_nodes );
    PNL_DEMAND_AUTOBUF( nt, num_nodes );
    PNL_DEMAND_AUTOBUF( dm, num_nodes );
    PNL_DEMAND_AUTOBUF( arr, num_nodes );
    for ( j = num_nodes; j--; )
    {
        node_types[j].SetType( 1, tTurboRand( 2, max_states ) );
        node_assoc[j] = tTurboRand( num_nodes );
    }
    for ( j = gaussian_seed; j--; )
    {
        node_types[j].SetType( 0, tTurboRand( 1, max_dim ) );
    }

    while ( no_gaussian_parent_with_discrete_child_please )
    {
        no_gaussian_parent_with_discrete_child_please = false;
        for ( j = num_nodes; j--; )
        {
            if ( node_types[node_assoc[j]].IsDiscrete() )
            {
                continue;
            }
            pars.clear();
            gr->GetChildren( j, &pars );
            for ( k = pars.size(); k--; )
            {
                if ( node_types[node_assoc[pars[k]]].IsDiscrete() )
                {
                    no_gaussian_parent_with_discrete_child_please = true;
                    node_types[node_assoc[pars[k]]].SetType( 0, tTurboRand( 1, max_dim ) );
                }
            }
        }
    }

    while ( no_gaussian_child_with_discrete_parent_please )
    {
        no_gaussian_child_with_discrete_parent_please = false;
        for ( j = num_nodes; j--; )
        {
            if ( node_types[node_assoc[j]].IsDiscrete() )
            {
                continue;
            }
            pars.clear();
            gr->GetParents( j, &pars );
            for ( k = pars.size(); k--; )
            {
                if ( node_types[node_assoc[pars[k]]].IsDiscrete() )
                {
                    no_gaussian_child_with_discrete_parent_please = true;
                    node_types[node_assoc[pars[k]]].SetType( 0, tTurboRand( 1, max_dim ) );
                }
            }
        }
    }
    bn = CBNet::Create( num_nodes, num_nodes, node_types, node_assoc, gr );
    CModelDomain* pMD = bn->GetModelDomain();
    bn->AllocFactors();
    for ( k = num_nodes; k--; )
    {
        pars.clear();
        gr->GetParents( k, &pars );
        for ( nt[j = pars.size()] = bn->GetNodeType( k ), dm[j] = k,
              s = node_types[node_assoc[k]].GetNodeSize(); j--; )
        {
            nt[j] = bn->GetNodeType( pars[j] );
            dm[j] = pars[j];
            s *= node_types[node_assoc[pars[j]]].GetNodeSize();
        }
        if ( node_types[node_assoc[k]].IsDiscrete() )
        {
#if 0
            cpd = CTabularCPD::CreateUnitFunctionCPD(  dm, pars.size() + 1, pMD );
#else
            data = new( float[s] );
            for ( j = s; j--; )
            {
                data[j] = (float)tTurboRand( 1000 ) / 567;
#if 1
                data[j] -= tTurboRand( 1000 ) / 5678e3f; // to test robustness
#endif
            }
            cpd = CTabularCPD::Create( dm, pars.size() + 1, pMD,  data );
            delete( data );
#endif
        }
        else
        {
            PNL_DEMAND_AUTOBUF( mean, max_dim );
            PNL_DEMAND_AUTOBUF( cov, max_dim * max_dim );
            data = new( float[s] );
            for ( j = num_nodes; j--; )
            {
                arr[j] = data;
            }
            for ( j = 20; j--; )
            {
                mean[j] = (float)tTurboRand( 10 ) / 10;
            }
            n = nt[pars.size()]->GetNodeSize();
            for ( j = n; j--; )
            {
                for ( a = j + 1; a--; )
                {
                    cov[a + j * n] = cov[j + a * n] = (float)tTurboRand( 10 ) / 10 + 2;
                }
            }
            for ( j = s; j--; )
            {
                data[j] = (float)tTurboRand( 10 ) / 10;
            }
            cpd = CGaussianCPD::Create( dm, pars.size() + 1, pMD );
            ((CGaussianCPD *)cpd)->AllocDistribution( mean, cov, (float)tTurboRand( 1, 1000 ), arr );
            delete( data );
        }
        cpd->NormalizeCPD();
        bn->AttachFactor( cpd );
        delete( cpd );
    }
    return bn;
}

CEvidence *tCreateRandomEvidence( CGraphicalModel *model,
                                  int num_nodes,
                                  intVector const &candidates )
{
    int i, j, k;
    intVector tmp;
    intVector const *cand;
    intVector nodes;
    valueVector values;
    CNodeType const *nt;

    PNL_MAKE_LOCAL( int, sz, model, GetNumberOfNodes() );

    PNL_DEFINE_AUTOBUF( int, perm, 32 );
    PNL_DEMAND_AUTOBUF( perm, sz );

    if ( candidates.empty() )
    {
        for ( i = model->GetNumberOfNodes(); i--; )
        {
            tmp.push_back( i );
        }
        cand = &tmp;
    }
    else
    {
        cand = &candidates;
    }

    if ( num_nodes > cand->size() )
    {
        PNL_RELEASE_AUTOBUF( perm );
        PNL_THROW( CInconsistentSize, "num_nodes is greater than number of possible candidates" );
    }

    tCreateRandomPermutation( cand->size(), perm );
    for ( i = num_nodes; i--; )
    {
        k = (*cand)[perm[i]];
        nt = model->GetNodeType( (*cand)[perm[i]] );
        nodes.push_back( k );
        if ( nt->IsDiscrete() )
        {
            values.push_back( (Value)(int)tTurboRand( nt->GetNodeSize() ) );
        }
        else
        {
            for ( j = nt->GetNodeSize(); j--; )
            {
                values.push_back( (Value)(float)tTurboRand( 20, 90 ) );
            }
        }
    }

    PNL_RELEASE_AUTOBUF( perm );

    return CEvidence::Create( model, nodes, values );
}
