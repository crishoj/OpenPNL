/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AMultiDMatrix.cpp                                           //
//                                                                         //
//  Purpose:   Test on matrix template classes functionality               //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"

PNL_USING

static char func_name[] = "testMatrix";

static char* test_desc = "Initialize a matrix";

static char* test_class = "Algorithm";


int testMatrix()
{
    int ret = TRS_OK;
    int dim = 0;
    int i;
    while(dim <= 0)
    {
        trsiRead( &dim, "128", "matrix dimension" );
    }
    
    int* range = (int*)trsGuardcAlloc( dim, sizeof(int) );
    float* data = (float*)trsGuardcAlloc( dim, sizeof(float) );

    for(i = 0; i < dim; i++)
    {
        range[i] = 1;
        data[i] = 1.0f*i;
    }
    range[0] = dim;

    CNumericDenseMatrix<float>* m = CNumericDenseMatrix<float>::Create(1, range, data);
	int length;
    const float *mdata;
	m->GetRawData(&length, &mdata);
    for(i = 0; i < dim; i++)
    {
        // Test the values...
        if(mdata[i] != data[i])
        {
            ret = TRS_FAIL;
            break;
        }
    }

    delete m;
    int range_memory_flag = trsGuardCheck( range );
    int data_memory_flag = trsGuardCheck( data );
    trsGuardFree( range );
    trsGuardFree( data );

    if(range_memory_flag || data_memory_flag)
    {
        return trsResult( TRS_FAIL, "Dirty memory");
    }

    return trsResult( ret, ret == TRS_OK ? "No errors" : "Bad matrix data");
}

void initAMultiDMatrix()
{
    trsReg(func_name, test_desc, test_class, testMatrix);
}