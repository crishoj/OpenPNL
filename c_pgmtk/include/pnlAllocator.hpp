/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlAllocator.hpp                                            //
//                                                                         //
//  Purpose:   Fused allocator                                             //
//             polymorphic object that can allocate memory different ways  //
//                                                                         //
//  Author(s): Denis Lagno                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLALLOCATOR_HPP__
#define __PNLALLOCATOR_HPP__

#include <assert.h>
#include <new>
#include <utility>

#include "pnlStackMemProv.hpp"
#include "pnlImpDefs.hpp"

PNL_BEGIN

#define PNL_DEFINE_AUTOALLOCATOR_SCOPE( ATOR, SIZE )                                \
    char ATOR ## _buf[SIZE + 1];                                                    \
    CStackMemStor<> *ATOR ## _stor = CStackMemStor<>::Create( ATOR ## _buf, SIZE ); \
    CFusedAllocator<> ATOR( ATOR ## _stor );                                        \
    {

#define PNL_RELEASE_AUTOALLOCATOR( ATOR )   \
    ATOR ## _stor->Release()

#define PNL_RELEASE_AUTOALLOCATOR_SCOPE( ATOR ) \
    }                                           \
    PNL_RELEASE_AUTOALLOCATOR( ATOR )

#define PNL_INSTANTIATE_AUTOALLOCATOR( ITOR, ATOR, DST_TYPE ) \
    CFusedAllocator< DST_TYPE > ITOR( ATOR ## _stor )

/* we have following macros not only because I love preprocessor *
 * but also because of poor implementation of templates in VC6   */
#if !defined(PNL_MSC)
#define PNL_REBIND_FUSED_ALLOCATOR( ATOR, DST_TYPE ) CFusedAllocator< DST_TYPE >( ATOR )
#else
#define PNL_REBIND_FUSED_ALLOCATOR( ATOR, DST_TYPE ) CFusedAllocator< DST_TYPE >( ATOR.kludge )
#endif

#define PNL_INSTANTIATE_FUSED_ALLOCATOR( ATOR, ORIG, DST_TYPE ) \
    CFusedAllocator< DST_TYPE > ATOR = PNL_REBIND_FUSED_ALLOCATOR( ORIG, DST_TYPE )

class CAllocKludge;
class CStdAllocKludge;

#if !defined(PNL_BROKEN_TEMPLATE)
template< typename Unit = CStackMemStorUnit, int frag = 16370 >
#endif
class CStackAllocKludge;

template< typename T = int >
class CFusedAllocator
{
public:
    typedef T *pointer;
    typedef T const *const_pointer;
    typedef T &reference;
    typedef T const &const_reference;
    typedef T value_type;
    typedef unsigned size_type;
    typedef int difference_type;

    template< typename U >
    struct rebind
    {
        typedef CFusedAllocator< U > other;
    };

#if !defined(PNL_BROKEN_TEMPLATE)
    template< typename U >
    inline CFusedAllocator( CFusedAllocator< U > const & );
#else
    inline CFusedAllocator( CFusedAllocator const & );
#endif

    inline CFusedAllocator();

#if !defined(PNL_BROKEN_TEMPLATE)
    template< typename Unit, int frag >
    CFusedAllocator( CStackMemStor< Unit, frag > * );
#else
    CFusedAllocator( CStackMemStor<> * );
#endif

    inline ~CFusedAllocator();

    inline pointer address( reference ) const;

    inline const_pointer address( const_reference ) const;

    inline pointer allocate( size_type, void * = 0 );

    inline void deallocate( pointer, size_type );

    size_type max_size() const;

    inline bool operator==( CFusedAllocator const & ) const;

    inline void construct( pointer, T const & ) const;

    inline void destroy( pointer ) const;

    inline CFusedAllocator &operator=( CFusedAllocator & );

    CAllocKludge *kludge;

    CFusedAllocator( CAllocKludge * );

#if !defined(PNL_BROKEN_TEMPLATE)
    template< typename ORG, typename DST >
    friend CFusedAllocator< DST > rebindFusedAllocator( CFusedAllocator< ORG > );
#endif
};

#if !defined(PNL_VC6)
template< typename ORG, typename DST >
CFusedAllocator< DST > rebindFusedAllocator( CFusedAllocator< ORG > orig )
{
    return CFusedAllocator< DST >( orig->kludge );
}
#endif

class PNL_API CAllocKludge
{
public:
    void *(*request_mem)( CAllocKludge *, int );
    void (*release_mem)( CAllocKludge *, void *, int );
    void (*release_kludge)( CAllocKludge * );
    CAllocKludge *(*clone)( CAllocKludge * );
};

class PNL_API CStdAllocKludge: public CAllocKludge
{
    static void *RequestMem( CAllocKludge *, int );
    static void ReleaseMem( CAllocKludge *, void *, int );
    static void ReleaseKludge( CAllocKludge * );
    static CAllocKludge *Clone( CAllocKludge * );

public:
    static CStdAllocKludge *Create();
    inline CStdAllocKludge();
};

#if !defined(PNL_BROKEN_TEMPLATE)
template< typename Unit, int frag >
class CStackAllocKludge: public CAllocKludge
#else
class PNL_API CStackAllocKludge: public CAllocKludge
#endif
{
#if !defined(PNL_BROKEN_TEMPLATE)
    typedef CStackMemStor< Unit, frag > Stor;
#else
    typedef CStackMemStor<> Stor;
#endif

    Stor *stor;

    static void *RequestMem( CAllocKludge *, int );
    static void ReleaseMem( CAllocKludge *, void *, int );
    static void ReleaseKludge( CAllocKludge * );
    static CAllocKludge *Clone( CAllocKludge * );
    inline CStackAllocKludge( Stor * );

public:
    static CStackAllocKludge *Create( Stor * );
};

extern CStdAllocKludge global_std_alloc_kludge;

#if !defined(PNL_BROKEN_TEMPLATE)
template< typename T >
template< typename U >
CFusedAllocator< T >::CFusedAllocator( CFusedAllocator< U > const &orig )
{
    kludge = (*orig.kludge->clone)( orig.kludge );
}
#else
template< typename T >
CFusedAllocator< T >::CFusedAllocator( CFusedAllocator< T > const &orig )
{
    kludge = (*orig.kludge->clone)( orig.kludge );
}
#endif

template< typename T >
CFusedAllocator< T >::CFusedAllocator()
{
    kludge = CStdAllocKludge::Create();
}

template< typename T >
CFusedAllocator< T >::CFusedAllocator( CAllocKludge *kl ): kludge( kl )
{}

#if !defined(PNL_BROKEN_TEMPLATE)
template< typename T >
template< typename Unit, int frag >
CFusedAllocator< T >::CFusedAllocator( CStackMemStor< Unit, frag > *stor )
{
    kludge = CStackAllocKludge< Unit, frag >::Create( stor );
}
#else
template< typename T >
CFusedAllocator< T >::CFusedAllocator( CStackMemStor<> *stor )
{
    kludge = CStackAllocKludge::Create( stor );
}
#endif

template< typename T >
CFusedAllocator< T >::~CFusedAllocator()
{
    (*kludge->release_kludge)( kludge );
}

template< typename T >
T *CFusedAllocator< T >::address( T &ref ) const
{
    return &ref;
}

template< typename T >
T const *CFusedAllocator< T >::address( T const &ref ) const
{
    return &ref;
}

template< typename T >
void CFusedAllocator< T >::construct( T *p, T const &t ) const
{
    new( p )( T )( t );
}

template< typename T >
void CFusedAllocator< T >::destroy( T *p ) const
{
    p->~T();
}

template< typename T >
T *CFusedAllocator< T >::allocate( size_type n, void * )
{
    return (T *)(*kludge->request_mem)( kludge, n * sizeof( T ) );
}

template< typename T >
void CFusedAllocator< T >::deallocate( T *p, size_type n )
{
    (*kludge->release_mem)( kludge, p, n * sizeof( T ) );
}

template< typename T >
bool CFusedAllocator< T >::operator==( CFusedAllocator< T > const &rhs ) const
{
    return kludge == rhs.kludge;
}

template< typename T >
typename CFusedAllocator< T >::size_type CFusedAllocator< T >::max_size() const
{
    return (unsigned)INT_MAX / sizeof( T );
}

template< typename T >
CFusedAllocator< T > &CFusedAllocator< T >::operator=( CFusedAllocator< T > &orig )
{
    std::cout << "OPEQ!" << std::endl;
    (*kludge->release_kludge)( kludge );
    kludge = (*orig.kludge->clone)( orig->kludge );
}

#if !defined(PNL_BROKEN_TEMPLATE)
template< typename Unit, int frag >
CStackAllocKludge< Unit, frag >::CStackAllocKludge( CStackMemStor< Unit, frag > *stor ): stor( stor )
{
    request_mem = &CStackAllocKludge::RequestMem;
    release_mem = &CStackAllocKludge::ReleaseMem;
    release_kludge = &CStackAllocKludge::ReleaseKludge;
    clone = &CStackAllocKludge::Clone;
}

template< typename Unit, int frag >
CStackAllocKludge< Unit, frag > *CStackAllocKludge< Unit, frag >::Create( CStackMemStor< Unit, frag > *stor )
{
#if 1
    return new( CStackAllocKludge< Unit, frag > )( stor );
#else // following is flawed
    CStackAllocKludge< Unit, frag > *kl;
    kl = (CStackAllocKludge< Unit, frag > *)stor->RequestMem( sizeof( CStackAllocKludge< Unit, frag > ) );
    return new( kl )( CStackAllocKludge< Unit, frag > )( stor );
#endif
}

template< typename Unit, int frag >
void CStackAllocKludge< Unit, frag >::ReleaseKludge( CAllocKludge *kl )
{
    delete( kl );
}

template< typename Unit, int frag >
CAllocKludge *CStackAllocKludge< Unit, frag >::Clone( CAllocKludge *kl )
{
    return Create( ((CStackAllocKludge< Unit, frag > *)kl)->stor );
}

template< typename Unit, int frag >
void *CStackAllocKludge< Unit, frag >::RequestMem( CAllocKludge *kl, int n )
{
    return ((CStackAllocKludge< Unit, frag > *)kl)->stor->RequestMem( n );
}

template< typename Unit, int frag >
void CStackAllocKludge< Unit, frag >::ReleaseMem( CAllocKludge *, void *, int )
{}
#endif

PNL_END

#endif

/* end of file */
