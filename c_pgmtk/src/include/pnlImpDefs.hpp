/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlImpDefs.hpp                                              //
//                                                                         //
//  Purpose:   some macros aimed at the ease of implementation             //
//                                                                         //
//  Author(s): Denis Lagno                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLIMPDEFS_HPP__
#define __PNLIMPDEFS_HPP__

#include <assert.h>

// for VC6 compiler: disable warnings about not enough actual parameters for macro
#pragma warning( disable: 4003 )

PNL_BEGIN

#define PNL_CHECK_ALLOC( p ) PNL_CHECK_IF_MEMORY_ALLOCATED( p )

#define PNL_BIG_CACHELINE_BYTES 128
#define PNL_SMALL_CACHELINE_BYTES 64

#define PNL_SPAWN0( X )
#define PNL_SPAWN1( X )  X
#define PNL_SPAWN2( X )  PNL_SPAWN1( X ); X
#define PNL_SPAWN3( X )  PNL_SPAWN2( X ); X
#define PNL_SPAWN4( X )  PNL_SPAWN3( X ); X
#define PNL_SPAWN5( X )  PNL_SPAWN4( X ); X
#define PNL_SPAWN6( X )  PNL_SPAWN5( X ); X
#define PNL_SPAWN7( X )  PNL_SPAWN6( X ); X
#define PNL_SPAWN8( X )  PNL_SPAWN7( X ); X
#define PNL_SPAWN9( X )  PNL_SPAWN8( X ); X
#define PNL_SPAWN10( X ) PNL_SPAWN9( X ); X
#define PNL_SPAWN11( X ) PNL_SPAWN10( X ); X
#define PNL_SPAWN12( X ) PNL_SPAWN11( X ); X
#define PNL_SPAWN13( X ) PNL_SPAWN12( X ); X
#define PNL_SPAWN14( X ) PNL_SPAWN13( X ); X
#define PNL_SPAWN15( X ) PNL_SPAWN14( X ); X
#define PNL_SPAWN16( X ) PNL_SPAWN15( X ); X

#define PNL_MUL( dst, src ) (dst) *= (src)
#define PNL_ADD( dst, src ) (dst) += (src)
#define PNL_ASN( dst, src ) (dst) = (src)

#if 0
#define PNL_PASS_ARR( PTR, SIZE, ARG, FIX, POST, OP, ACT )          \
{                                                                   \
    int i;                                                          \
    for ( i = (SIZE); i--; )                                        \
    {                                                               \
        POST( (PTR)[ARG( i )] FIX ) OP ACT( i );                    \
    }                                                               \
}

#define PNL_SUBSCRIPT( X ) [X]
#define PNL_FUNC_VAL( VAL ) (VAL) PNL_VOID
#define PNL_FUNC_SUBSCRIPT( ARRAY ) (ARRAY)PNL_SUBSCRIPT
#define PNL_FUNC_ADD( ADDEND ) (ADDEND) +
#define PNL_FUNC_MULADD( MPLIER, ADDEND ) (ADDEND) + (MPLIER) *
#define PNLI_SUBSCRIPT_HELPER( X ) [X])
#define PNL_FTOR_VAL( VAL ) (VAL) PNL_VOID
#define PNL_FTOR_SUBSCRIPT( ARRAY, ARG_FTOR ) ((ARRAY)PNLI_SUBSCRIPT_HELPER
#define PNL_FTOR_SUBSCRIPT2( ARR1, ARR2 ) PNL_FTOR_SUBSCRIPT( ARR1 )PNL_FTOR_SUBSCRIPT( ARR2 )

#else

#define PNL_PASS_ARR( PTR, SIZE, ARG, FIX, POST, ACT )     \
{                                                          \
    int i;                                                 \
    for ( i = (SIZE); i--; )                               \
    {                                                      \
        POST( (PTR)[ARG] FIX ) ACT;                        \
    }                                                      \
}
#endif

#define PNL_ID( X ) (X)

#define PNL_VOID( X )
#define PNL_VOID2( X, Y )

#undef PNL_I_WANNA_CHECK_AUTOBUF

#if 1 /* former version is good for C, while latter is good for C++ environment */
#define PNL_DEFINE_AUTOBUF( T, PTR, PRELIM_SIZE ) \
    T *PTR;                                       \
    T PTR ## _buf[(PRELIM_SIZE) + 1];             \
    typedef T PTR ## _type;                       \
    int const PTR ## _prelim_size = (PRELIM_SIZE);\
    T *PTR ## _dyn = 0

#define PNL_DEMAND_AUTOBUF( PTR, DEMAND_SIZE )                \
    if ( (unsigned)(DEMAND_SIZE) > PTR ## _prelim_size )      \
    {                                                         \
        assert( PTR ## _dyn == 0 );                           \
        PTR = PTR ## _dyn = new( PTR ## _type[DEMAND_SIZE] ); \
        /* PNL_CHECK_ALLOC( PTR ); */                         \
    }                                                         \
    else                                                      \
    {                                                         \
        PTR = PTR ## _buf;                                    \
    }

#define PNL_INIT_AUTOBUF( PTR, DEMAND_SIZE, VAL )               \
    PNL_DEMAND_AUTOBUF( PTR, DEMAND_SIZE );                     \
    PNL_PASS_ARR( PTR, DEMAND_SIZE, i, /**/, /**/, = VAL )

#define PNL_RELEASE_AUTOBUF( PTR ) delete[]( PTR ## _dyn )

#else

template< typename T >
struct autobuf_wrapper_t
{
    T *dyn;
#ifdef PNL_I_WANNA_CHECK_AUTOBUF
    bool danger;
#endif

    autobuf_wrapper_t(): dyn( 0 )
    {
#ifdef PNL_I_WANNA_CHECK_AUTOBUF
        danger = true;
#endif
    }
    ~autobuf_wrapper_t()
    {
        delete[]( dyn );
#ifdef PNL_I_WANNA_CHECK_AUTOBUF
        if ( danger )
        {
            PNL_THROW( CInconsistentState, "no good: autobuf not released" );
        }
#endif
    }
};

#define PNL_DEFINE_AUTOBUF( T, PTR, PRELIM_SIZE )                   \
    T *PTR;                                                         \
    T PTR ## _buf[(PRELIM_SIZE) + 1];                               \
    typedef T PTR ## _type;                                         \
    int const PTR ## _prelim_size = (PRELIM_SIZE);                  \
    autobuf_wrapper_t< T > PTR ## _wrapper

#define PNL_DEMAND_AUTOBUF( PTR, DEMAND_SIZE )                          \
    if ( (unsigned)(DEMAND_SIZE) > PTR ## _prelim_size )                \
    {                                                                   \
        assert( PTR ## _wrapper.dyn == 0 );                             \
        PTR = PTR ## _wrapper.dyn = new( PTR ## _type[DEMAND_SIZE] );   \
        /* PNL_CHECK_ALLOC( PTR ); */                                   \
    }                                                                   \
    else                                                                \
    {                                                                   \
        PTR = PTR ## _buf;                                              \
    }

#define PNL_INIT_AUTOBUF( PTR, DEMAND_SIZE, VAL )               \
    PNL_DEMAND_AUTOBUF( PTR, DEMAND_SIZE );                     \
    PNL_PASS_ARR( PTR, DEMAND_SIZE, i, /**/, /**/, = VAL )

#ifndef PNL_I_WANNA_CHECK_AUTOBUF
#define PNL_RELEASE_AUTOBUF( PTR )
#else
#define PNL_RELEASE_AUTOBUF( PTR ) PTR ## _wrapper.danger = false
#endif
#endif

#define PNL_DEFINE_AUTOMULBUF( T, PTR, NUM_BUFS, PRELIM_SIZE )         \
    T *PTR[NUM_BUFS];                                                  \
    PNL_DEFINE_AUTOBUF( T, PTR ## _plane, (PRELIM_SIZE) * (NUM_BUFS) );\
    int const PTR ## _num_bufs = NUM_BUFS

#define PNL_DEMAND_AUTOMULBUF( PTR, DEMAND_SIZE )                               \
    PNL_DEMAND_AUTOBUF( PTR ## _plane, (DEMAND_SIZE) * PTR ## _num_bufs );      \
    PNL_ARRANGE_TOP_PTRS( PTR ## _type, PTR, PTR ## _num_bufs,                  \
                          PTR ## _plane, DEMAND_SIZE )

#define PNL_INIT_AUTOMULBUF( PTR, DEMAND_SIZE, VAL )                              \
    PNL_INIT_AUTOBUF( PTR ## _plane, (DEMAND_SIZE) * PTR ## _num_bufs, VAL );     \
    PNL_ARRANGE_TOP_PTRS( PTR ## _type, PTR, PTR ## _num_bufs,                    \
                          PTR ## _plane, DEMAND_SIZE )

#define PNL_RELEASE_AUTOMULBUF( PTR ) PNL_RELEASE_AUTOBUF( PTR ## _plane )

#define PNL_ARRANGE_TOP_PTRS( T, PTR, NUM_TOP_PTRS, PLANE, SLICE_SIZE )              \
    PNL_PASS_ARR( PTR, NUM_TOP_PTRS, i, /**/, /**/, = SLICE_SIZE * i + PLANE )

#define PNL_DEFINE_AUTOBUFBUF( T, PTR, PRELIM_NUM_BUFS, PRELIM_SIZE )      \
    PNL_DEFINE_AUTOBUF( T *, PTR, PRELIM_NUM_BUFS );                       \
    PNL_DEFINE_AUTOBUF( T, PTR ## _plane, PRELIM_SIZE * PRELIM_NUM_BUFS )

#define PNL_DEMAND_AUTOBUFBUF( PTR, NUM_BUFS, DEMAND_SIZE )      \
    PNL_DEMAND_AUTOBUF( PTR, NUM_BUFS );                         \
    PNL_DEMAND_AUTOBUF( PTR ## _plane, DEMAND_SIZE * NUM_BUFS ); \
    PNL_ARRANGE_TOP_PTRS( PTR ## _type, PTR, NUM_BUFS, PTR ## _plane, DEMAND_SIZE )

#define PNL_INIT_AUTOBUFBUF( PTR, NUM_BUFS, DEMAND_SIZE, VAL )      \
    PNL_DEMAND_AUTOBUF( PTR, NUM_BUFS );                            \
    PNL_INIT_AUTOBUF( PTR ## _plane, DEMAND_SIZE * NUM_BUFS, VAL ); \
    PNL_ARRANGE_TOP_PTRS( PTR ## _type, PTR, NUM_BUFS, PTR ## _plane, DEMAND_SIZE )

#define PNL_RELEASE_AUTOBUFBUF( PTR ) \
    PNL_RELEASE_AUTOBUF( PTR );       \
    PNL_RELEASE_AUTOBUF( PTR ## _plane )

#define PNL_DEFINE_AUTOSTACK( T, PTR, PRELIM_SIZE ) \
    PNL_DEFINE_AUTOBUF( T, PTR, PRELIM_SIZE );      \
    int PTR ## _top

#define PNL_DEMAND_AUTOSTACK( PTR, DEMAND_SIZE ) \
    PNL_DEMAND_AUTOBUF( PTR, DEMAND_SIZE );      \
    PTR ## _top = 0

#define PNL_AUTOSTACK_PUSH( PTR, VAL ) PTR[PTR ## _top++] = (VAL)

#define PNL_AUTOSTACK_POP( PTR ) (PTR[--PTR ## _top])

#define PNL_AUTOSTACK_TOP( PTR ) (PTR[PTR ## _top - 1])

#define PNL_AUTOSTACK_CLEAR( PTR ) PTR ## _top = 0
#define PNL_CLEAR_AUTOSTACK PNL_AUTOSTACK_CLEAR

#define PNL_AUTOSTACK_SIZE( PTR ) (PTR ## _top)

#define PNL_IS_AUTOSTACK_EMPTY( PTR ) (PTR ## _top == 0)

#define PNL_RELEASE_AUTOSTACK( PTR ) PNL_RELEASE_AUTOBUF( PTR )

#define PNL_DEFINE_AUTOMULSTACK( T, PTR, NUM_STACKS, PRELIM_SIZE ) \
    PNL_DEFINE_AUTOMULBUF( T, PTR, NUM_STACKS, PRELIM_SIZE );      \
    int PTR ## _top[NUM_STACKS]

#define PNL_DEMAND_AUTOMULSTACK( PTR, DEMAND_SIZE )                    \
    PNL_DEMAND_AUTOMULBUF( PTR, DEMAND_SIZE );                         \
    PNL_PASS_ARR( PTR ## _top, PTR ## _num_bufs, i, /**/, /**/, = 0 )

#define PNL_AUTOMULSTACK_PUSH( PTR, STACK, VAL ) \
    PTR[STACK][PTR ## _top[STACK]++] = (VAL)

#define PNL_AUTOMULSTACK_POP( PTR, STACK ) (PTR[STACK][--PTR ## _top[STACK]])

#define PNL_AUTOMULSTACK_TOP( PTR, STACK ) (PTR[STACK[PTR ## _top[STACK] - 1])

#define PNL_AUTOMULSTACK_CLEAR( PTR, STACK ) PTR ## _top[STACK] = 0

#define PNL_AUTOMULSTACK_SIZE( PTR, STACK ) (PTR ## _top[STACK])

#define PNL_AUTOMULSTACK_EMPTY( PTR, STACK ) (PTR ## _top[STACK] == 0)

#define PNL_RELEASE_AUTOMULSTACK( PTR ) PNL_RELEASE_AUTOMULBUF( PTR )

#define PNL_2DIDX( I, J, COLZ ) ((I) * (COLZ) + (J))

#define PNL_MAKE_LOCAL( T, VAR, OBJ, ALIEN ) T VAR = (T)(OBJ)->ALIEN
#define PNL_MKLOCAL( T, VAR ) PNL_MAKE_LOCAL( T, VAR, this, VAR )

#define PNL_RAWMEM_CAST( T, PTR ) ((T)(void *)(PTR))
#define PNL_ALIGN_CAPACITY_ABOVE( T, UNIT, SIZE ) (((SIZE) * sizeof( T ) - 1) / sizeof( UNIT ) + 1)
#define PNL_ALIGN_CAPACITY_BELOW( T, UNIT, SIZE ) (((SIZE) * sizeof( T )) / sizeof( UNIT ))
#define PNL_ALIGN_CAPACITY( T, UNIT, SIZE ) PNL_ALIGN_CAPACITY_ABOVE( T, UNIT, SIZE )

#define PNL_MAX( A, B ) ((A) >= (B) ? (A) : (B))
#define PNL_MIN( A, B ) ((A) <= (B) ? (A) : (B))
#define PNL_IMIN( A, B ) ((A) ^ (((A)^(B)) & (((A) < (B)) - 1)))
#define PNL_IMAX( A, B ) ((A) ^ (((A)^(B)) & (((A) > (B)) - 1)))

#define PNL_DEFINE_TACTER( TACTER, COUN ) \
    int TACTER ## _coun = COUN;           \
    int const TACTER ## _lim = COUN

#define PNL_RESET_TACTER( TACTER ) TACTER ## _coun = TACTER ## _lim

#define PNL_TACTER_STEP( TACTER, DAYLY_OP, MONTHLY_OP )   \
    if ( TACTER ## _coun-- )                              \
    {                                                     \
        DAYLY_OP;                                         \
    }                                                     \
    else                                                  \
    {                                                     \
        MONTHLY_OP;                                       \
        PNL_RESET_TACTER( TACTER );                       \
    }

#define PNL_RELEASE_TACTER( TACTER )

#if 0
inline void PNL_OMP_BARRIER()
{
    #if defined(_OPENMP) && !defined(PAR_OMP)
    #pragma omp barrier
    #endif
}

#define PNL_PRAGMA_OMP( DIRECTIVE, BLOCK )  \
{                                           \
    CException ompex;                       \
    bool ompex_flag = false;                \
                                            \
    _Pragma( "omp" #DIRECTIVE )             \
    {                                       \
        try                                 \
        {                                   \
            BLOCK;                          \
        }                                   \
        catch( CException &ex )             \
        {                                   \
            _Pragma( "omp critical" )       \
            {                               \
                ompex = ex;                 \
                ompex_flag = true;          \
            }                               \
        }                                   \
    }                                       \
    if ( ompex_flag )                       \
    {                                       \
        throw ompex;                        \
    }                                       \
}

#define PNL_OMP_THROW( TYPE, DESCR )            \
    _Pragma( "omp critical" )                   \
    {                                           \
        ompex = PNL_EXCEPTION( TYPE, DESCR );   \
        ompex_flag = true;                      \
    }
#endif

template< typename T, typename A, typename I >
struct subscript: public std::binary_function< A, I, T >
{
    T operator()( A const &arr, I const &i ) const
    {
        return arr[i];
    }
};

template< typename R, typename DST, typename SRC >
struct mul_in_self
{
    R operator()( DST &d, SRC const &s ) const
    {
        return d *= s;
    }
};

template< class UNARY_OP1, class UNARY_OP2 >
struct unary_compozitor: public std::unary_function< typename UNARY_OP2::argument_type, typename UNARY_OP1::result_type >
{
    UNARY_OP1 const &op1;
    UNARY_OP2 const &op2;

    unary_compozitor( UNARY_OP1 const &op1, UNARY_OP2 const &op2 ): op1( op1 ), op2( op2 )
    {}

    typename UNARY_OP1::result_type operator()( typename UNARY_OP2::argument_type const &x ) const
    {
        return op1( op2( x ) );
    }
};

template< class UNARY_OP1, class UNARY_OP2 >
inline unary_compozitor< UNARY_OP1, UNARY_OP2 > compose_unaries( UNARY_OP1 const &op1, UNARY_OP2 const &op2 )
{
    return unary_compozitor< UNARY_OP1, UNARY_OP2 >( op1, op2 );
}

template< typename T, class INDICATOR, class OP, class SOFTEXITCOND, class HARDEXITCOND >
int blockize( INDICATOR const &ind, int length, OP const &op, T const &neutral,
              SOFTEXITCOND const &soft_exit_cond, HARDEXITCOND const &hard_exit_cond,
              bool force_nonzero, T *block_size )
{
    int i;
    T prev_block_size = neutral;

    *block_size = neutral;
    if ( length > 0 && !soft_exit_cond( neutral ) )
    {
        for ( i = 0; i < length; ++i )
        {
            prev_block_size = *block_size;
            op( *block_size, ind( i ) );
            if ( soft_exit_cond( *block_size ) )
            {
                break;
            }
        }
        if ( i == length )
        {
            --i;
        }
        if ( hard_exit_cond( *block_size ) )
        {
            if ( !force_nonzero || i )
            {
                --i;
                *block_size = prev_block_size;
            }
        }
        return i + 1;
    }
    return 0;
}

template< int I >
class PNLI_FRAME
{};

// AUTOBUFLIST not tested yet
#if 0
#define PNL_DEFINE_AUTOBUFLIST( T, PTR, PRELIM_NUM_LISTS, PRELIM_TOTAL_SIZE )       \
    typedef struct PTR ## _elem_t                                                   \
    {                                                                               \
        T val;                                                                      \
        PTR ## _elem_t *next;                                                       \
    } PTR ## _elem_t;                                                               \
    PNL_DEFINE_AUTOMULBUF( PTR ## _elem_t *, PTR ## _head, 2, PRELIM_NUM_LISTS );   \
    PNL_DEFINE_AUTOBUF( PTR ## _elem_t, PTR ## _plane, PRELIM_TOTAL_SIZE );         \
    PTR ## _elem_t *PTR ## _free;                                                   \
    PTR ## _elem_t *PTR ## _temp

#define PNL_DEMAND_AUTOBUFLIST( PTR, DEMAND_TOTAL_SIZE )                                    \
    PNL_INIT_AUTOBUF( PTR ## _head, demand_num_lists, 0 );                                  \
    PNL_DEMAND_AUTOBUF( PTR ## _plane, DEMAND_TOTAL_SIZE );                                 \
    PTR ## _free = PTR ## _plane;                                                           \
    PNL_PASS_ARR( PTR ## _free, DEMAND_TOTAL_SIZE, i, .next, /**/, = PTR ## _free + i + 1 );\
    PTR ## _free[DEMAND_TOTAL_SIZE - 1].next = 0

#define PNL_AUTOBUFLIST_ELTYPE( PTR ) PTR ## _elem_t *

#define PNL_AUTOBUFLIST_HEAD( PTR, LIST ) (PTR ## _head[0][LIST])

#define PNL_AUTOBUFLIST_TAIL( PTR, LIST ) (PTR ## _head[1][LIST])

#define PNL_AUTOBUFLIST_VAL( ELEM ) ((ELEM)->val)

#define PNL_AUTOBUFLIST_NEXT( ELEM ) ((ELEM)->next)

#define PNL_AUTOBUFLIST_GET_FREE_ELEM( PTR, LIST ) \
    (PTR ## _temp = PTR ## _free,                  \
     PTR ## _free = PTR ## _temp->next,            \
     PTR ## _temp)

#define PNL_AUTOBUFLIST_HEAD_TURBO_APPEND( PTR, LIST, VAL )             \
{                                                                       \
    PTR ## _elem_t *node = PNL_AUTOBUFLIST_GET_FREE_ELEM( PTR, LIST );  \
    node->val = (VAL);                                                  \
    node->next = PNL_AUTOBUFLIST_HEAD( PTR, LIST );                     \
    PNL_AUTOBUFLIST_HEAD( PTR, LIST ) = node;                           \
}

#define PNL_AUTOBUFLIST_HEAD_APPEND( PTR, LIST, VAL )                          \
    PNL_AUTOBUFLIST_HEAD_TURBO_APPEND( PTR, LIST, VAL );                       \
    if ( PNL_AUTOBUFLIST_TAIL( PTR, LIST ) == 0 )                              \
    {                                                                          \
        PNL_AUTOBUFLIST_TAIL( PTR, LIST ) = PNL_AUTOBUFLIST_HEAD( PTR, LIST ); \
    }

#define PNL_AUTOBUFLIST_TAIL_TURBO_APPEND( PTR, LIST, VAL )             \
{                                                                       \
    PTR ## _elem_t *node = PNL_AUTOBUFLIST_GET_FREE_ELEM( PTR, LIST );  \
    node->val = (VAL);                                                  \
    node->next = 0;                                                     \
    PNL_AUTOBUFLIST_TAIL( PTR, LIST )->next = node;                     \
    PNL_AUTOBUFLIST_TAIL( PTR, LIST ) = node;                           \
}

#define PNL_AUTOBUFLIST_TAIL_APPEND( PTR, LIST, VAL )                          \
    PNL_AUTOBUFLIST_TAIL_TURBO_APPEND( PTR, LIST, VAL );                       \
    if ( PNL_AUTOBUFLIST_HEAD( PTR, LIST ) == 0 )                              \
    {                                                                          \
        PNL_AUTOBUFLIST_HEAD( PTR, LIST ) = PNL_AUTOBUFLIST_TAIL( PTR, LIST ); \
    }

#define PNL_AUTOBUFLIST_MERGE( PTR, LIST1, LIST2 )                                  \
    PNL_AUTOBUFLIST_TAIL( PTR, LIST1 )->next = PNL_AUTOBUFLIST_HEAD( PTR, LIST2 );  \
    PNL_AUTOBUFLIST_TAIL( PTR, LIST1 ) = PNL_AUTOBUFLIST_TAIL( PTR, LIST2 );        \
    PNL_AUTOBUFLIST_HEAD( PTR, LIST2 ) = 0

#define PNL_RELEASE_AUTOBUFLIST( PTR )    \
    PNL_RELEASE_AUTOBUF( PTR ## _plane ); \
    PNL_RELEASE_AUTOMULBUF( PTR ## _head )
#endif

PNL_END

#endif

/* end of file */
