/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlVector.hpp                                               //
//                                                                         //
//  Purpose:   Vector, Vector of Vectors and Vector of mixed Vectors       //
//                                                                         //
//  Author(s): Denis Lagno                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLVECTOR_HPP__
#define __PNLVECTOR_HPP__

#include <vector>
#include <iterator>

#include "pnlAllocator.hpp"

PNL_BEGIN

template< typename T >
class CVector: public std::vector< T, CFusedAllocator< T > >
{
public:
    typedef typename std::vector< T, CFusedAllocator< T > >::size_type size_type;

#if !defined(PNL_BROKEN_TEMPLATE)
    template< typename U >
    explicit inline CVector( CFusedAllocator< U > const & = CFusedAllocator< U >() );
#else
    explicit inline CVector( CFusedAllocator< T > const & = CFusedAllocator< T >() );
#endif

#if !defined(PNL_BROKEN_TEMPLATE)
    template< typename U >
    explicit inline CVector( size_type, T const & = T(), CFusedAllocator< U > const & = CFusedAllocator< U >() );
#else
    explicit inline CVector( size_type, T const & = T(), CFusedAllocator< T > const & = CFusedAllocator< T >() );
#endif

#if !defined(PNL_BROKEN_TEMPLATE)
    template < typename InputIterator, typename U >
    inline CVector( InputIterator first, InputIterator last, CFusedAllocator< U > & = CFusedAllocator< U >() );
#endif

    inline CVector( CVector const & );

#if !defined(PNL_BROKEN_TEMPLATE)
    template< typename U >
    inline CVector( CVector const &, CFusedAllocator< U > & );
#else
    inline CVector( CVector const &, CFusedAllocator< T > & );
#endif

    inline CVector &operator=( CVector const & );
    
    inline ~CVector();
};

template< typename T >
class CVecVector: public CVector< CVector< T > >
{
public:
    typedef typename CVector< CVector< T > >::size_type size_type;

    inline CVecVector();

#if !defined(PNL_BROKEN_TEMPLATE)
    template< typename U >
    inline CVecVector( CFusedAllocator< U > const & );
#else
    inline CVecVector( CFusedAllocator< T > const & );
#endif

#if !defined(PNL_BROKEN_TEMPLATE)
    template< typename U >
    inline CVecVector( size_type, CVector< T > const & = CVector< T >(),
                       CFusedAllocator< U > & = CFusedAllocator< U >() );
#else
    inline CVecVector( size_type, CVector< T > const & = CVector< T >(),
                       CFusedAllocator< T > & = CFusedAllocator< T >() );
#endif

#if !defined(PNL_BROKEN_TEMPLATE)
    template < typename InputIterator, typename U >
    inline CVecVector( InputIterator first, InputIterator last, CFusedAllocator< U > & = CFusedAllocator< U >() );
#endif

    inline CVecVector( CVecVector const & );

    inline void resize( size_type, CVector< T > const & );

    inline void resize( size_type );
    
#if GCC_VERSION >= 30400
    using CVector< CVector<T> >::get_allocator;
#endif
};

#ifndef PNL_DENY_DLLIMPORT_TEMPLATE_DEFINITION

#if !defined(PNL_BROKEN_TEMPLATE)
template< typename T >
template< typename U >
CVector< T >::CVector( CFusedAllocator< U > const &ator ): std::vector< T, CFusedAllocator< T > >( CFusedAllocator< T >( ator ) )
{}
#else
template< typename T >
CVector< T >::CVector( CFusedAllocator< T > const &ator ): std::vector< T, CFusedAllocator< T > >( ator )
{}
#endif

#if !defined(PNL_BROKEN_TEMPLATE)
template< typename T >
template< typename U >
CVector< T >::CVector( size_type n, T const &val, CFusedAllocator< U > const &ator )
    : std::vector< T, CFusedAllocator< T > >( n, val, CFusedAllocator< T >( ator ) )
{}
#else
template< typename T >
CVector< T >::CVector( size_type n, T const &val, CFusedAllocator< T > const &ator )
    : std::vector< T, CFusedAllocator< T > >( n, val, ator )
{}
#endif

#if !defined(PNL_BROKEN_TEMPLATE)
template< typename T >
template < typename InputIterator, typename U >
CVector< T >::CVector( InputIterator first, InputIterator last, CFusedAllocator< U > &ator )
    : std::vector< T, CFusedAllocator< T > >( first, last, CFusedAllocator< T >( ator ) )
{}
#endif

template< typename T >
CVector< T >::CVector( CVector< T > const &vec ): std::vector< T, CFusedAllocator< T > >( vec )
{}

#if !defined(PNL_BROKEN_TEMPLATE)
template< typename T >
template< typename U >
CVector< T >::CVector( CVector< T > const &vec, CFusedAllocator< U > &ator ): std::vector< T, CFusedAllocator< T > >( ator )
#else
template< typename T >
CVector< T >::CVector( CVector< T > const &vec, CFusedAllocator< T > &ator ): std::vector< T, CFusedAllocator< T > >( ator )
#endif
{
    *this = vec;
}

template< typename T >
CVector< T > &CVector< T >::operator=( CVector< T > const &pat )
{
    assign( pat.begin(), pat.end() );
    return *this;
}

template< typename T >
CVector< T >::~CVector()
{}

template< typename T >
CVecVector< T >::CVecVector()
    : CVector< CVector< T > >( CFusedAllocator< CVector< T > >() )
{}

#if !defined(PNL_BROKEN_TEMPLATE)
template< typename T >
template< typename U >
CVecVector< T >::CVecVector( CFusedAllocator< U > const &ator )
#else
template< typename T >
CVecVector< T >::CVecVector( CFusedAllocator< T > const &ator )
#endif
    : CVector< CVector< T > >( PNL_REBIND_FUSED_ALLOCATOR( ator, CVector< T > ) )
{}

#if !defined(PNL_BROKEN_TEMPLATE)
template< typename T >
template< typename U >
CVecVector< T >::CVecVector( size_type n,
                             CVector< T > const &pat,
                             CFusedAllocator< U > &ator )
#else
template< typename T >
CVecVector< T >::CVecVector( size_type n,
                             CVector< T > const &pat,
                             CFusedAllocator< T > &ator )
#endif
    : CVector< CVector< T > >( PNL_REBIND_FUSED_ALLOCATOR( ator, CVector< T > ) )
{
    assign( n, pat );
}

#if !defined(PNL_BROKEN_TEMPLATE)
template< typename T >
template < typename InputIterator, typename U >
CVecVector< T >::CVecVector( InputIterator first, InputIterator last,
                             CFusedAllocator< U > &ator )
    : CVector< CVector< T > >( first, last, ator )
{}
#endif

template< typename T >
CVecVector< T >::CVecVector( CVecVector< T > const &orig )
    : CVector< CVector< T > >( orig )
{}

template< typename T >
void CVecVector< T >::resize( size_type n, CVector< T > const &pat )
{
    CVector< CVector< T > >::resize( n, pat );
}

template< typename T >
void CVecVector< T >::resize( size_type n )
{
    resize( n, CVector< T >( PNL_REBIND_FUSED_ALLOCATOR( get_allocator(), T ) ) );
}

#endif // PNL_DENY_DLLIMPORT_TEMPLATE_DEFINITION

PNL_END

#endif
