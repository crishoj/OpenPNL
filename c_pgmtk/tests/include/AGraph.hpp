/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AGraph.hpp                                                  //
//                                                                         //
//  Purpose:   TestGraph class definition                                  //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __AGRAPH_HPP__
#define __AGRAPH_HPP__

#include "pnlGraph.hpp"

PNL_USING

class TestGraph : public CGraph
/* need for testing the memory allocation */
{
public:
	TestGraph( int numberOfNodes, const int* numberOfNeighbors,
		       int **neighborList, ENeighborType **edgeOrientation )
		       : CGraph( numberOfNodes, numberOfNeighbors, neighborList, 
		                    edgeOrientation )
	{}
};

#endif // AGraph.hpp