/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      tCreateRandStructs.hpp                                      //
//                                                                         //
//  Purpose:   Implementation of the algorithm                             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnl_dll.hpp"

PNL_USING

void tCreateRandomPermutation( int n, int buf[] );

CGraph *tCreateRandomDAG( int num_nodes, int num_edges, bool top_sorted );

CBNet *tCreateRandomBNet( int num_nodes, int num_edges,
                          int max_states, int max_dim,
                          int gaussian_seed,
                          bool no_gaussian_parent_with_discrete_child_please,
                          bool no_gaussian_child_with_discrete_parent_please );

inline int tTurboRand( int k )
{
    return rand() % k;
}

inline int tTurboRand( int lo, int hi )
{
    return tTurboRand( hi - lo + 1 ) + lo;
}

CEvidence *tCreateRandomEvidence( CGraphicalModel *model,
                                  int num_nodes,
                                  intVector const &candidates = intVector() );
