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

static char func_name[] = "testReduceOp";

static char *test_desc = "compare different versions of ReduceOp";

static char *test_class = "Algorithm";

const float eps = 1e-4f;

int testReduceOp()
{
    int rv = TRS_OK;

    int i;
    int num_tests;
    int action;
    int num_dims, small_num_dims;
    int ranges[64], small_ranges[64];
    int dims_of_interest[64];
    int ov[64];
    int num_dims_of_interest;
    int bs, small_bs;
    float *bulk, *small_bulk, *small_bulk2;

    PNL_DEFINE_TACTER( success_log, 15 );
    
    srand( 123 );
    try{
#if 1
    for ( action = 0; action <= 2; ++action )
    {
	std::cout << "testing ReduceOp action " << action << std::endl;
        PNL_RESET_TACTER( success_log );
	for ( num_tests = 1; num_tests <= 303; ++num_tests )
	{
//          std::cout << "action " << action << " test #" << num_tests << std::endl;
	    num_dims = tTurboRand( 1, 7 );
//          std::cout << "num_dims " << num_dims << std::endl;
	    tCreateRandomPermutation( num_dims, dims_of_interest );
	    num_dims_of_interest = tTurboRand( 1, num_dims );
#if 0
	    std::cout << "num_dims_of_interest " << num_dims_of_interest << std::endl;
	    std::cout << "dims_of_interest" << std::endl;
	    for ( i = 0; i < num_dims_of_interest; ++i )
	    {
		std::cout << ' ' << dims_of_interest[i];
	    }
	    std::cout << std::endl;
#endif
	    for ( i = num_dims, bs = 1; i--; )
	    {
		bs *= ranges[i] = tTurboRand( 1, 6 ) + tTurboRand( 1, 6 );
	    }
	    if ( bs > 20000000 )
	    {
		--num_tests;
		continue;
	    }
#if 0
	    std::cout << "ranges" << std::endl;
	    int iters = PNL_IMIN( 90000000 / bs, 100000 ) + 1;
            iters += 20;
	    for ( i = 0; i < num_dims; ++i )
	    {
		std::cout << ' ' << ranges[i];
	    }
	    std::cout << std::endl;
#endif
	    for ( i = num_dims_of_interest; i--; )
	    {
		ov[i] = tTurboRand( ranges[dims_of_interest[i]] );
	    }
	    bulk = new( float[bs] );
	    for ( i = bs; i--; )
	    {
		bulk[i] = (float)tTurboRand( 10 );
	    }
	    CNumericDenseMatrix< float > *mat = CNumericDenseMatrix< float >::Create( num_dims, ranges, bulk );

	    CMatrix< float > *r[2];

#if 0
	    clock_t cl = clock();
	    for ( j = iters; j--; )
	    {
		r[0] = mat->OldReduceOp( dims_of_interest, num_dims_of_interest, action, ov );
		delete r[0];
	    }
	    std::cout << "old clock " << (clock() - cl) / (float)CLOCKS_PER_SEC << std::endl;

	    cl = clock();
	    for ( j = iters; j--; )
	    {
		r[1] = mat->ReduceOp( dims_of_interest, num_dims_of_interest, action, ov );
		delete r[1];
	    }
	    std::cout << "new clock " << (clock() - cl) / (float)CLOCKS_PER_SEC << std::endl;
#endif

	    if ( false )
	    {
		r[0] = mat->ReduceOp( dims_of_interest, num_dims_of_interest, action, ov );
	    }
	    else
	    {
		r[0] = mat->OldReduceOp( dims_of_interest, num_dims_of_interest, action, ov );
	    }
	    r[1] = mat->ReduceOp( dims_of_interest, num_dims_of_interest, action, ov );

	    int r_num_dims[2];
	    int const *r_ranges[2];
	    int r_bs[2];
	    float const *r_bulk[2];

	    for ( i = 2; i--; )
	    {
		r[i]->GetRanges( r_num_dims + i, r_ranges + i );
		((CNumericDenseMatrix< float > *)r[i])->GetRawData( r_bs + i, r_bulk + i );
	    }

	    if ( r_num_dims[0] != r_num_dims[1] )
	    {
		std::cout << " failed" << std::endl;
		return trsResult( TRS_FAIL, "retvals different" );
	    }
	    for ( i = r_num_dims[0]; i--; )
	    {
		if ( r_ranges[0][i] != r_ranges[1][i] )
		{
		    std::cout << " failed" << std::endl;
		    return trsResult( TRS_FAIL, "retvals different" );
		}
	    }
	    if ( r_bs[0] != r_bs[1] )
	    {
		std::cout << " failed" << std::endl;
		return trsResult( TRS_FAIL, "retvals different" );
	    }
	    for ( i = r_bs[0]; i--; )
	    {
		if ( fabs( r_bulk[0][i] - r_bulk[1][i] ) > eps )
		{
		    std::cout << " failed" << std::endl;
		    return trsResult( TRS_FAIL, "retvals different" );
		}
	    }

            PNL_TACTER_STEP( success_log, std::cout << "/ok\\", std::cout << std::endl );

	    delete( bulk );
	    delete( r[1] );
	    delete( r[0] );
	    delete( mat );
	}
	std::cout << std::endl;
    }
#endif

    std::cout << "testing MultiplyInSelf" << std::endl;

    PNL_RESET_TACTER( success_log );

    for ( num_tests = 1; num_tests <= 300; ++num_tests )
    {
//      std::cout << "MultiplyInSelf " << " test #" << num_tests << std::endl;
	num_dims = tTurboRand( 1, 7 );
//      std::cout << "num_dims " << num_dims << std::endl;
	tCreateRandomPermutation( num_dims, dims_of_interest );
	small_num_dims = tTurboRand( 1, num_dims );
//      std::cout << "small_num_dims " << small_num_dims << std::endl;
	num_dims_of_interest = small_num_dims;
#if 0
	std::cout << "num_dims_of_interest " << num_dims_of_interest << std::endl;
	std::cout << "dims_of_interest" << std::endl;
	for ( i = 0; i < num_dims_of_interest; ++i )
	{
	    std::cout << ' ' << dims_of_interest[i];
	}
	std::cout << std::endl;
#endif
	for ( i = num_dims, bs = 1; i--; )
	{
	    bs *= ranges[i] = tTurboRand( 1, 6 ) + tTurboRand( 6 );
	}
	if ( bs > 20000000 )
	{
	    --num_tests;
	    continue;
	}
	for ( i = small_num_dims, small_bs = 1; i--; )
	{
	    small_bs *= small_ranges[i] = ranges[dims_of_interest[i]];
	}
#if 0
	int iters = PNL_IMIN( 10000000 / (bs), 100000 ) + 1;
        iters += 20;
	std::cout << "ranges" << std::endl;
	for ( i = 0; i < num_dims; ++i )
	{
	    std::cout << ' ' << ranges[i];
	}
	std::cout << std::endl;
	std::cout << "small_ranges" << std::endl;
	for ( i = 0; i < small_num_dims; ++i )
	{
	    std::cout << ' ' << small_ranges[i];
	}
	std::cout << std::endl;
#endif
	bulk = new( float[bs] );
	small_bulk = new( float[small_bs] );
	small_bulk2 = new( float[small_bs] );
	for ( i = bs; i--; )
	{
	    bulk[i] = (float)tTurboRand( 10 );
	}
	for ( i = small_bs; i--; )
	{
	    small_bulk[i] = (float)tTurboRand( 10 );
	    small_bulk2[i] = 1.f / small_bulk[i];
	}
	CNumericDenseMatrix< float > *r[2];
	r[0] = CNumericDenseMatrix< float >::Create( num_dims, ranges, bulk );
	r[1] = CNumericDenseMatrix< float >::Create( num_dims, ranges, bulk );
	CNumericDenseMatrix< float > *small_mat = CNumericDenseMatrix< float >::Create( small_num_dims, small_ranges, small_bulk );
	CNumericDenseMatrix< float > *small_mat2 = CNumericDenseMatrix< float >::Create( small_num_dims, small_ranges, small_bulk );

#if 0
	std::cout << "iters " << iters << std::endl;
	clock_t cl = clock();
	for ( j = iters; j--; )
	{
	    r[1]->MultiplyInSelf_omp( small_mat, num_dims_of_interest, dims_of_interest, 0, 0 );
	    r[1]->MultiplyInSelf_omp( small_mat2, num_dims_of_interest, dims_of_interest, 0, 0 );
	}
	std::cout << "omp clock " << (clock() - cl) / (float)CLOCKS_PER_SEC << std::endl;

	delete( small_mat2 );
	delete( small_mat );
	delete( r[1] );

        r[1] = CNumericDenseMatrix< float >::Create( num_dims, ranges, bulk );
	small_mat = CNumericDenseMatrix< float >::Create( small_num_dims, small_ranges, small_bulk );
	small_mat2 = CNumericDenseMatrix< float >::Create( small_num_dims, small_ranges, small_bulk );

        cl = clock();
	for ( j = iters; j--; )
	{
	    r[1]->MultiplyInSelf_omp2( small_mat, num_dims_of_interest, dims_of_interest, 0, 0 );
	    r[1]->MultiplyInSelf_omp2( small_mat2, num_dims_of_interest, dims_of_interest, 0, 0 );
	}
	std::cout << "omp2 clock " << (clock() - cl) / (float)CLOCKS_PER_SEC << std::endl;

	delete( small_mat2 );
	delete( small_mat );
	delete( r[1] );

        r[1] = CNumericDenseMatrix< float >::Create( num_dims, ranges, bulk );
	small_mat = CNumericDenseMatrix< float >::Create( small_num_dims, small_ranges, small_bulk );
	small_mat2 = CNumericDenseMatrix< float >::Create( small_num_dims, small_ranges, small_bulk );

        cl = clock();
	for ( j = iters; j--; )
	{
	    r[1]->MultiplyInSelf_plain( small_mat, num_dims_of_interest, dims_of_interest, 0, 0 );
	    r[1]->MultiplyInSelf_plain( small_mat2, num_dims_of_interest, dims_of_interest, 0, 0 );
	}
	std::cout << "plain clock " << (clock() - cl) / (float)CLOCKS_PER_SEC << std::endl;

	delete( small_mat2 );
	delete( small_mat );
	delete( r[1] );

        r[1] = CNumericDenseMatrix< float >::Create( num_dims, ranges, bulk );
	small_mat = CNumericDenseMatrix< float >::Create( small_num_dims, small_ranges, small_bulk );
	small_mat2 = CNumericDenseMatrix< float >::Create( small_num_dims, small_ranges, small_bulk );

        cl = clock();
	for ( j = iters; j--; )
	{
	    r[1]->MultiplyInSelf_omp( small_mat, num_dims_of_interest, dims_of_interest, 0, 0 );
	    r[1]->MultiplyInSelf_omp( small_mat2, num_dims_of_interest, dims_of_interest, 0, 0 );
	}
	std::cout << "omp_again clock " << (clock() - cl) / (float)CLOCKS_PER_SEC << std::endl;

        delete( r[1] );
        r[1] = CNumericDenseMatrix< float >::Create( num_dims, ranges, bulk );
#endif

        r[0]->OldMultiplyInSelf( small_mat, num_dims_of_interest, dims_of_interest, 0, 0 );
	r[1]->MultiplyInSelf( small_mat, num_dims_of_interest, dims_of_interest, 0, 0 );
	int r_num_dims[2];
	int const *r_ranges[2];
	int r_bs[2];
	float const *r_bulk[2];

	for ( i = 2; i--; )
	{
	    r[i]->GetRanges( r_num_dims + i, r_ranges + i );
	    ((CNumericDenseMatrix< float > *)r[i])->GetRawData( r_bs + i, r_bulk + i );
	}

	if ( r_num_dims[0] != r_num_dims[1] )
	{
	    std::cout << " failed" << std::endl;
	    return trsResult( TRS_FAIL, "retvals different" );
	}
	for ( i = r_num_dims[0]; i--; )
	{
	    if ( r_ranges[0][i] != r_ranges[1][i] )
	    {
		std::cout << " failed" << std::endl;
		return trsResult( TRS_FAIL, "retvals different" );
	    }
	}
	if ( r_bs[0] != r_bs[1] )
	{
	    std::cout << " failed" << std::endl;
	    return trsResult( TRS_FAIL, "retvals different" );
	}
	for ( i = r_bs[0]; i--; )
	{
	    if ( fabs( r_bulk[0][i] - r_bulk[1][i] ) > eps )
	    {
		std::cout << " failed " << r_bulk[0][i] << " " << r_bulk[1][i] << std::endl;
		return trsResult( TRS_FAIL, "retvals different" );
	    }
	}

        PNL_TACTER_STEP( success_log, std::cout << "/ok\\", std::cout << std::endl );

	std::cout.flush();
	delete( bulk );
	delete( small_bulk );
	delete( small_bulk2 );
	delete( r[1] );
	delete( r[0] );
	delete( small_mat );
	delete( small_mat2 );
    }
    std::cout << std::endl;

    return trsResult( rv, rv == TRS_OK ? "No errors" : "ReduceOp FAILED");
    }
    catch ( CException exc )
    {
	std::cout << exc.GetMessage() << std::endl;
	throw;
    }
}

void initAReduceOp()
{
    trsReg( func_name, test_desc, test_class, &testReduceOp );
}
