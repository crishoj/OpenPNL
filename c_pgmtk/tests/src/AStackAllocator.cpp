/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AStackAllocator.cpp                                         //
//                                                                         //
//  Purpose:   stress testing stack allocator                              //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <time.h>
#include <stdlib.h>

#include <new>
#if 0
#include <vector>
#endif

#include "trsapi.h"

#include "pnl_dll.hpp"
#include "tCreateRandStructs.hpp"
#include "pnlAllocator.hpp"
#include "pnlVector.hpp"

PNL_USING

static char const my_func_name[] = "testStackAllocator";
static char const my_test_desc[] = "test stack allocator";
static char const my_test_class[] = "Algorithm";

struct Bag1
{
    int i;

    Bag1( int i ): i( i )
    {}

    operator int()
    {
        return i;
    }
};

struct Bag2
{
    int i;
    float f;
    char a[33];
    int j;

    Bag2( int j ): j( j )
    {}

    operator int()
    {
        return j;
    }
};

typedef std::vector< int, CFusedAllocator< int > > vtor0_type;
typedef std::vector< struct Bag1, CFusedAllocator< Bag1 > > vtor1_type;
typedef std::vector< struct Bag2, CFusedAllocator< Bag2 > > vtor2_type;

typedef CFusedAllocator< int > alloc_type0;
typedef CFusedAllocator< Bag1 > alloc_type1;
typedef CFusedAllocator< Bag1 > alloc_type2;

int testStackAllocatorTimings()
{
    int i, j, k, t;

    PNL_DEFINE_AUTOALLOCATOR_SCOPE( stack_ator, 0 );
    PNL_INSTANTIATE_AUTOALLOCATOR( stack_ator_int, stack_ator, int );

    CFusedAllocator< int > std_ator_int;
    
    std::vector< int, CFusedAllocator< int > > *vtor[100];

    for ( i = 100; i--; )
    {
        vtor[i] = new( std::vector< int, CFusedAllocator< int > > )( stack_ator_int );
    }

    srand( 12 );
    
    clock_t cl = clock();
    
    for ( t = 100; t--; )
    {
        for ( i = 100; i--; )
        {
            j = tTurboRand( 100 );
            k = tTurboRand( 10 );// + (int)!(bool)tTurboRand( 1000 ) * 50000;
//          std::cout << "vtor[" << j << "] pushing " << k << " ints" << std::endl;
            delete( vtor[j] );
            vtor[j] = new( std::vector< int, CFusedAllocator< int > > )( stack_ator_int );
//            vtor[j]->empty();
            vtor[j]->reserve( k );
//            for ( m = 0; m < k; ++m )
//            {
//                vtor[j]->push_back( 2 * m - 123 );
//            }
        }
        for ( i = 100; i--; )
        {
            delete vtor[i];
        }
        stack_ator_stor->Empty();
        stack_ator_stor->Abjure();
        for ( i = 100; i--; )
        {
            vtor[i] = new( std::vector< int, CFusedAllocator< int > > )( stack_ator_int );
        }
    }


    std::cout << "stack clock " << (clock() - cl) / (float)CLOCKS_PER_SEC << std::endl;

    for ( i = 100; i--; )
    {
        delete vtor[i];
        vtor[i] = new( std::vector< int, CFusedAllocator< int > > )( std_ator_int );
    }

    srand( 12 );

    cl = clock();
    
    for ( t = 100; t--; )
    {
        for ( i = 100; i--; )
        {
            j = tTurboRand( 100 );
            k = tTurboRand( 10 );// + (int)!(bool)tTurboRand( 1000 ) * 50000;
            delete( vtor[j] );
            vtor[j] = new( std::vector< int, CFusedAllocator< int > > )( std_ator_int );
//            vtor[j]->empty();
            vtor[j]->reserve( k );
//            for ( m = 0; m < k; ++m )
//            {
//                vtor[j]->push_back( 2 * m - 123 );
//            }
        }
        for ( i = 100; i--; )
        {
            delete vtor[i];
        }
        for ( i = 100; i--; )
        {
            vtor[i] = new( std::vector< int, CFusedAllocator< int > > )( std_ator_int );
        }
    }

    std::cout << "std clock " << (clock() - cl) / (float)CLOCKS_PER_SEC << std::endl;

////////////////////////
    
    for ( i = 100; i--; )
    {
        delete vtor[i];
        vtor[i] = new( std::vector< int, CFusedAllocator< int > > )( stack_ator_int );
    }

    srand( 12 );
    
    cl = clock();
    
    for ( t = 100; t--; )
    {
        for ( i = 100; i--; )
        {
            j = tTurboRand( 100 );
            k = tTurboRand( 10 );// + (int)!(bool)tTurboRand( 1000 ) * 50000;
//          std::cout << "vtor[" << j << "] pushing " << k << " ints" << std::endl;
            delete( vtor[j] );
            vtor[j] = new( std::vector< int, CFusedAllocator< int > > )( stack_ator_int );
//            vtor[j]->empty();
            vtor[j]->reserve( k );
//            for ( m = 0; m < k; ++m )
//            {
//                vtor[j]->push_back( 2 * m - 123 );
//            }
        }
        for ( i = 100; i--; )
        {
            delete vtor[i];
        }
        stack_ator_stor->Empty();
        stack_ator_stor->Abjure();
        for ( i = 100; i--; )
        {
            vtor[i] = new( std::vector< int, CFusedAllocator< int > > )( stack_ator_int );
        }
    }

    std::cout << "stack clock " << (clock() - cl) / (float)CLOCKS_PER_SEC << std::endl;

    for ( i = 100; i--; )
    {
        delete vtor[i];
        vtor[i] = new( std::vector< int, CFusedAllocator< int > > )( std_ator_int );
    }

    srand( 12 );

    cl = clock();
    
    for ( t = 100; t--; )
    {
        for ( i = 100; i--; )
        {
            j = tTurboRand( 100 );
            k = tTurboRand( 10 );// + (int)!(bool)tTurboRand( 1000 ) * 50000;
            delete( vtor[j] );
            vtor[j] = new( std::vector< int, CFusedAllocator< int > > )( std_ator_int );
//            vtor[j]->empty();
            vtor[j]->reserve( k );
//            for ( m = 0; m < k; ++m )
//            {
//                vtor[j]->push_back( 2 * m - 123 );
//            }
        }
        for ( i = 100; i--; )
        {
            delete vtor[i];
        }
        for ( i = 100; i--; )
        {
            vtor[i] = new( std::vector< int, CFusedAllocator< int > > )( std_ator_int );
        }
    }

    std::cout << "std clock " << (clock() - cl) / (float)CLOCKS_PER_SEC << std::endl;

    PNL_RELEASE_AUTOALLOCATOR_SCOPE( stack_ator );
    
    return TRS_OK;
}

static void foo( CVecVector< int > &vv )
{
    int i, j;

    CVecVector< int >::allocator_type ator = vv.get_allocator();
    CFusedAllocator< Bag1 > bator = PNL_REBIND_FUSED_ALLOCATOR( ator, Bag1 );
    
    vv.resize( 200 );
    for ( i = 200; i--; )
    {
        for ( j = tTurboRand( 100 ); j--; )
        {
            vv[i].push_back( tTurboRand( 100 ) );
        }
    }

    vv.resize( 500 );
}

static void goo( CVecVector< int > vv )
{
    int i, j;

    vv.resize( 200 );
    for ( i = 200; i--; )
    {
        for ( j = tTurboRand( 100 ); j--; )
        {
            vv[i].push_back( tTurboRand( 100 ) );
        }
    }

    vv.resize( 500 );
}

static void bar( CVecVector< int > &vv )
{
    int i, j;

    vv.resize( 200 );
    for ( i = 200; i--; )
    {
        for ( j = tTurboRand( 100 ); j--; )
        {
            vv[i].push_back( tTurboRand( 100 ) );
        }
    }

    vv.resize( 50 );
}

int testStackAllocator()
{
    int i, j, k, m;

    CStackMemStor<> *stor[100];

    CFusedAllocator< int > ator0[100];
    CFusedAllocator< Bag1 > ator1[100];
    CFusedAllocator< Bag2 > ator2[100];

    vtor0_type *vtor0[100];
    vtor1_type *vtor1[100];
    vtor2_type *vtor2[100];

    for ( i = 100; i--; )
    {
        stor[i] = 0;
    }

    for ( i = 100; i--; )
    {
        vtor0[i] = 0;
    }
    for ( i = 100; i--; )
    {
        vtor1[i] = 0;
    }
    for ( i = 100; i--; )
    {
        vtor2[i] = 0;
    }

    for ( i = 100; i--; )
    {
//        std::cout << "tact " << i << std::endl;
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            if ( stor[j] == 0 )
            {
//                std::cout << "stor[" << j << "] Create" << std::endl;
                stor[j] = CStackMemStor<>::Create();
            }
        }
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            k = tTurboRand( 100 );
            if ( stor[j] && stor[k] == 0 )
            {
//                std::cout << "stor[" << k << "] spawned from stor[" << j << "]" << std::endl;
                stor[k] = stor[j]->Spawn();
            }
        }
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            if ( stor[j] )
            {
//                std::cout << "stor[" << j << "] Empty" << std::endl;
                stor[j]->Empty();
            }
        }
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            if ( stor[j] )
            {
//                std::cout << "stor[" << j << "] Abjure" << std::endl;
                stor[j]->Abjure();
            }
        }
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            if ( stor[j] && stor[j]->GetNumChildren() == 0 )
            {
//                std::cout << "stor[" << j << "] Release" << std::endl;
                stor[j]->Release();
                stor[j] = 0;
            }
        }
    }

    for ( i = 100; i--; )
    {
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            k = tTurboRand( 100 );
            if ( stor[j] )
            {
//                std::cout << "ator0[" << k << "] dtor" << std::endl;
                (ator0 + k)->~CFusedAllocator< int >();
//                std::cout << "ator0[" << k << "] init from stor[" << j << "]" << std::endl;
                new( ator0 + k )( CFusedAllocator< int > )( stor[j] );
//                new( ator0 + k )( CFusedAllocator< int > )();
            }
        }
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            k = tTurboRand( 100 );
            if ( stor[j] )
            {
//                std::cout << "ator1[" << k << "] dtor" << std::endl;
                (ator1 + k)->~CFusedAllocator< Bag1 >();
//                std::cout << "ator1[" << k << "] init from stor[" << j << "]" << std::endl;
                new( ator1 + k )( CFusedAllocator< Bag1 > )( stor[j] );
//                new( ator1 + k )( CFusedAllocator< Bag1 > )();
            }
        }
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            k = tTurboRand( 100 );
            if ( stor[j] )
            {
//                std::cout << "ator2[" << k << "] dtor" << std::endl;
                (ator2 + k)->~CFusedAllocator< Bag2 >();
//                std::cout << "ator2[" << k << "] init from stor[" << j << "]" << std::endl;
                new( ator2 + k )( CFusedAllocator< Bag2 > )( stor[j] );
//                new( ator2 + k )( CFusedAllocator< Bag2 > )();
            }
        }
    }

    PNL_DEFINE_TACTER( success_log, 15 );
    for ( i = 100; i--; )
    {
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            if ( vtor0[j] )
            {
                k = tTurboRand( 2000 ) + (tTurboRand( 100 ) ? 0:1) * 300000;
//                std::cout << "vtor0[" << j << "] pushing " << k << " ints" << std::endl;
                vtor0[j]->clear();
                for ( m = 0; m < k; ++m )
                {
                    vtor0[j]->push_back( 3 * m - 123 );
//                    std::cout << "* " << (*vtor0[j])[0] << std::endl;
                }
            }
        }
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            if ( vtor0[j] )
            {
//                std::cout << "vtor0[" << j << "] checking" << std::endl;
//                std::cout << "size " << vtor0[j]->size() << std::endl;
                for ( k = vtor0[j]->size(); k--; )
                {
                    if ( (*vtor0[j])[k] != 3 * k - 123 )
                    {
                        std::cout << "allocators have problems.." << std::endl;
                        return TRS_FAIL;
                    }
                }
#ifdef _DEBUG
                if ( vtor0[j]->size() )
                {
                    PNL_TACTER_STEP( success_log, std::cout << "/OK\\", std::cout << std::endl );
                }
#endif
            }
        }
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            if ( vtor1[j] )
            {
                k = tTurboRand( 2000 ) + (tTurboRand( 100 ) ? 0:1)* 300000;
//                std::cout << "vtor1[" << j << "] pushing " << k << " ints" << std::endl;
                vtor1[j]->clear();
                for ( m = 0; m < k; ++m )
                {
                    vtor1[j]->push_back( Bag1( 2 * m - 321 ) );
                }
            }
        }
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            if ( vtor1[j] )
            {
//                std::cout << "vtor1[" << j << "] checking" << std::endl;
//                std::cout << "size " << vtor1[j]->size() << std::endl;
                for ( k = vtor1[j]->size(); k--; )
                {
                    if ( (int)(*vtor1[j])[k] != 2 * k - 321 )
                    {
                        std::cout << "allocators have problems.." << std::endl;
                        return TRS_FAIL;
                    }
                }
#ifdef _DEBUG
                if ( vtor1[j]->size() )
                {
                    PNL_TACTER_STEP( success_log, std::cout << "/OK\\", std::cout << std::endl );
                }
#endif
            }
        }
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            if ( vtor2[j] )
            {
                k = tTurboRand( 2000 ) + (tTurboRand( 100 ) ? 0:1)* 300000;
//                std::cout << "vtor2[" << j << "] pushing " << k << " ints" << std::endl;
                vtor2[j]->clear();
                for ( m = 0; m < k; ++m )
                {
                    vtor2[j]->push_back( Bag2( -2 * m + 80 ) );
                }
            }
        }
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            if ( vtor2[j] )
            {
//                std::cout << "vtor2[" << j << "] checking" << std::endl;
//                std::cout << "size " << vtor2[j]->size() << std::endl;
                for ( k = vtor2[j]->size(); k--; )
                {
                    if ( (int)(*vtor2[j])[k] != -2 * k + 80 )
                    {
                        std::cout << "allocators have problems.." << std::endl;
                        return TRS_FAIL;
                    }
                }
#ifdef _DEBUG
                if ( vtor2[j]->size() )
                {
                    PNL_TACTER_STEP( success_log, std::cout << "/OK\\", std::cout << std::endl );
                }
#endif
            }
        }

        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            k = tTurboRand( 100 );
            if ( vtor0[k] )
            {
//                std::cout << "vtor0[" << k << "] dtor" << std::endl;
//              vtor0[k]->~vector< int, CFusedAllocator< int > >();
                delete vtor0[k];
                vtor0[k] = 0;
            }
        }
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            k = tTurboRand( 100 );
            if ( vtor1[k] )
            {
//                std::cout << "vtor1[" << k << "] dtor" << std::endl;
//              vtor1[k]->~vector< int, CFusedAllocator< Bag1 > >();
                delete vtor1[k];
                vtor1[k] = 0;
            }
        }
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            k = tTurboRand( 100 );
            if ( vtor2[k] )
            {
//                std::cout << "vtor2[" << k << "] dtor" << std::endl;
//              vtor2[k]->~vector< int, CFusedAllocator< Bag2 > >();
                delete vtor2[k];
                vtor2[k] = 0;
            }
        }

        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            k = tTurboRand( 100 );
            if ( vtor0[k] == 0 )
            {
//                std::cout << "vtor0[" << k << "] create from ator[" << j << "]" << std::endl;
                vtor0[k] = new( vector< int, CFusedAllocator< int > > )( ator0[j] );
            }
        }
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            k = tTurboRand( 100 );
            if ( vtor1[k] == 0 )
            {
//                std::cout << "vtor1[" << k << "] create from ator[" << j << "]" << std::endl;
                vtor1[k] = new( vector< Bag1, CFusedAllocator< Bag1 > > )( ator1[j] );
            }
        }
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            k = tTurboRand( 100 );
            if ( vtor2[k] == 0 )
            {
//                std::cout << "vtor2[" << k << "] create from ator[" << j << "]" << std::endl;
                vtor2[k] = new( vector< Bag2, CFusedAllocator< Bag2 > > )( ator2[j] );
            }
        }

#if 0        
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            k = tTurboRand( 100 );
            if ( ator[j] )
            {
                if ( vtor1[k] )
                {
                    vtor1[k]->~vtor1_type();
                }

                alloc_type1 ab = *ator[j];

                new( vtor1[k] )
                    
                    ( vtor1_type )
                    
                    ( ab );
            }
        }
        if ( tTurboRand( 10 ) == 0 )
        {
            j = tTurboRand( 100 );
            k = tTurboRand( 100 );
            if ( ator[j] )
            {
                if ( vtor2[k] )
                {
                    vtor2[k]->~vtor2_type();
                }
                new( vtor2[k] )( vtor2_type )( (CStackAllocator< Bag2 >)*ator[j] );
            }
        }

#endif
    }
    PNL_RELEASE_TACTER( success_log );

    for ( i = 100; i--; )
    {
        delete( vtor0[i] );
        delete( vtor1[i] );
        delete( vtor2[i] );
    }

    do
    {
        for ( i = 100, j = 0; i--; )
        {
            if ( stor[i] && stor[i]->GetNumChildren() == 0 )
            {
                stor[i]->Release();
                stor[i] = 0;
                ++j;
            }
        }
    } while ( j );

    PNL_DEFINE_AUTOALLOCATOR_SCOPE( ator, 50000 );
    PNL_INSTANTIATE_AUTOALLOCATOR( ator_int, ator, int );

    CVecVector< int > vv;

    vv.resize( 100 );

    for ( i = 100; i--; )
    {
        for ( j = tTurboRand( 100 ); j--; )
        {
            vv[i].push_back( tTurboRand( 100 ) );
        }
    }

    foo( vv );
    goo( vv );
    bar( vv );

    PNL_RELEASE_AUTOALLOCATOR_SCOPE( ator );

    return TRS_OK;
}

void initAStackAllocator()
{
    trsReg( (char *)my_func_name, (char *)my_test_desc, (char *)my_test_class, &testStackAllocator );
}
