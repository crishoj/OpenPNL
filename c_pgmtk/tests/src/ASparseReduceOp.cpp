/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      ASparseReduceOp.cpp                                         //
//                                                                         //
//  Purpose:   Test on reduce operation algorithm for sparse matrices      //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"

PNL_USING

static char func_name[] = "testSparceReduceOp";
static char* test_desc = "testing ReduceOp for sparse matrices";
static char* test_class = "Algorithm";

int testSparseReduceOp()
{
    int rt = TRS_OK;
    int i;
    int num_dims;
    int const *ranges;
    int range1[] = { 3, 5, 4, 9, 2 };
    int dims_of_interest1[] = { 2, 3 };
    int idx[200];

    CNumericSparseMatrix< float > *mt1 = CNumericSparseMatrix< float >::Create(
        sizeof( range1 ) / sizeof( int ), range1, 0 );
    
    idx[0] = 1;
    idx[1] = 1;
    idx[2] = 1;
    idx[3] = 1;
    idx[4] = 1;
    mt1->SetElementByIndexes( 3.f, idx );

    CNumericSparseMatrix< float > *mt2 = (CNumericSparseMatrix< float > *)mt1->
        ReduceOp( dims_of_interest1, sizeof( dims_of_interest1 ) / sizeof( int ), 0 );
        
    mt2->GetRanges( &num_dims, &ranges );
    std::cout << num_dims << std::endl;
    for ( i = 0; i < num_dims; ++i )
    {
        std::cout << ranges[i] << " ";
    }
    std::cout << std::endl;

    return trsResult( rt, rt == TRS_OK ? "No errors" : "Sparse ReduceOp FAILED");
}

void initSparseReduceOp()
{
    trsReg( func_name, test_desc, test_class, &testSparseReduceOp );
}
