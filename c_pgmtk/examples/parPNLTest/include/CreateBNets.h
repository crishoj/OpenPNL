/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  File:      CreateBNets.h                                               //
//                                                                         //
//  Purpose:   implementation of function to create random CBNet object    //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __CREATEBNETS_H__
#define __CREATEBNETS_H__

#include "pnl_dll.hpp"
#include "pnlBNet.hpp"

PNL_USING

// to generate CBNet object with any structure
CBNet* Create_BNet_Default(int num_nodes, int max_num_states, 
    int num_indep_nodes, int max_size_family, long& num_edges);

// to generate CBNet object with toyQMR structure
CBNet* Create_BNet_toyQMR(int num_nodes, int max_num_states, 
    int num_indep_nodes, int max_size_family, long& num_edges);

// to generate CBNet object with pyramid structure
CBNet* Create_BNet_Pyramid(int& num_nodes, int max_num_states, 
    int num_indep_nodes, int num_layers, long& num_edges);

// to generate CBNet object with complete graph structure
CBNet* Create_BNet_CompleteGraph(int num_nodes, int max_num_states, 
    long& num_edges);

// to generate CBNet object with multilayers grid topology
CBNet* Create_BNet_RegularGrid(int& num_nodes, int width, int height,
    int max_num_states, long& num_edges, int num_layers = 1);

#endif
