/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlStackMemProv.hpp                                         //
//                                                                         //
//  Purpose:   Stack-fashioned memory storage,                             //
//             analogue of obstacks and OpenCV's CvMemStorage              //
//                                                                         //
//  Author(s): Denis Lagno                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLSTACKMEMPROV_HPP__
#define __PNLSTACKMEMPROV_HPP__

#include <limits.h>
#include <assert.h>
#include <new>
#include <utility>

#include "pnlImpDefs.hpp"
#include "pnlLog.hpp"

PNL_BEGIN

struct CStackMemStorUnit
{
    double d[1];
};

template< typename Unit = CStackMemStorUnit, int frag = 16370 >
class CStackMemStor
{
    struct Seg
    {
        void *mem;
        int size;
        Seg *next;
        void (*release)( void * );
    };

    void *top;
    int free;
    CStackMemStor *master;
    Seg *bin[2];
    Seg *vac[2];
    int num_children;
 
    CStackMemStor( CStackMemStor * = 0 );

    CStackMemStor( void *first_buf, int size, void (*release)( void * ) = 0 );

    inline ~CStackMemStor();

    void ForceUseBuf( void *buf, int size, void (*release)( void * ) = 0 );

    void *SOS( int );

    Seg *Req();

public:
    static CStackMemStor *Create();

    static CStackMemStor *Create( void *first_buf, int size, void (*release)( void * ) = 0 );

    static void Release( CStackMemStor * );

    void Release();

    void UseBuf( void *buf, int size, void (*release)( void * ) = 0 );
    
    inline void *RequestUnit();

    inline void *RequestMunit( int units );

    inline void *RequestMem( int bytes );

    // !!! use this method only if you sure will not encounter alignment problems !!!
    // for example you are safe if you use Stack Allocator for allocating char strings and nothing else
    inline void *RequestRaw( int bytes );

    CStackMemStor *Spawn();

    void Abjure();

    void Empty();

    inline int GetNumChildren();

    void Dump();
};

#ifndef PNL_VC6
template< typename T = int, typename Unit = CStackMemStorUnit, int frag = 16370 >
class CStackMemProv
{
    typedef CStackMemStor< Unit, frag > Stor;
    Stor *stor;

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
        typedef CStackMemProv< U, Unit, frag > other;
    };

    inline CStackMemProv();

    inline CStackMemProv( Stor * );

    template< typename U >
    inline CStackMemProv( CStackMemProv< U, Unit, frag > const & );

    inline CStackMemProv( void *first_buf, int size, void (*release)( void * ) = 0 );

    inline operator CStackMemStor< Unit, frag > *();

    inline void Release();

    inline Stor *operator->();

    inline pointer address( reference );

    inline const_pointer address( const_reference );

    inline pointer allocate( size_type, void * = 0 );

    inline void deallocate( pointer, size_type );

    size_type max_size();

    inline bool operator==( CStackMemProv const & ) const;

    inline void construct( pointer, T const & );

    inline void destroy( pointer );
};
#endif

template< typename Unit, int frag >
void *CStackMemStor< Unit, frag >::RequestUnit()
{
    return RequestRaw( sizeof( Unit ) );
}

template< typename Unit, int frag >
void *CStackMemStor< Unit, frag >::RequestMunit( int units )
{
    return RequestRaw( sizeof( Unit ) * units );
}

template< typename Unit, int frag >
void *CStackMemStor< Unit, frag >::RequestMem( int bytes )
{
    return RequestMunit( PNL_ALIGN_CAPACITY( char, Unit, bytes ) );
}

template< typename Unit, int frag >
void *CStackMemStor< Unit, frag >::RequestRaw( int bytes )
{
    if ( (free -= bytes) >= 0 )
    {
        return top = (char *)top - bytes;
    }
    return SOS( bytes );
}

template< typename Unit, int frag >
void CStackMemStor< Unit, frag >::ForceUseBuf( void *buf, int size, void (*release)( void * ) )
{
    int reserv;
    Seg *sg;

    size = PNL_ALIGN_CAPACITY_BELOW( char, Unit, size ) * sizeof( Unit );
    reserv = PNL_ALIGN_CAPACITY( Seg, Unit, 1 ) * sizeof( Unit );
    sg = (Seg *)((char *)buf + (size -= reserv));
    sg->mem = buf;
    sg->size = size;
    sg->release = release;
    if ( vac[0] )
    {
        sg->next = vac[0];
        vac[0] = sg;
    }
    else
    {
        sg->next = 0;
        vac[0] = vac[1] = sg;
    }
}

template< typename Unit, int frag >
void CStackMemStor< Unit, frag >::UseBuf( void *buf, int size, void (*release)( void * ) )
{
    if ( size < 32 * sizeof( Unit )
                + PNL_ALIGN_CAPACITY( CStackMemStor, Unit, 1 ) * sizeof( Unit )
                + PNL_ALIGN_CAPACITY( Seg, Unit, 1 ) * sizeof( Seg ) )
    {
        // thank you thank you thank you
        if ( release )
        {
            (*release)( buf );
        }
        return;
    }
    ForceUseBuf( buf, size, release );
}

template< typename Unit, int frag >
CStackMemStor< Unit, frag >::CStackMemStor( CStackMemStor *master )
{
    free = 0;
    this->master = master;
    vac[0] = vac[1] = 0;
    bin[0] = bin[1] = 0;
    num_children = 0;
}

template< typename Unit, int frag >
CStackMemStor< Unit, frag >::CStackMemStor( void *first_buf, int size, void (*release)( void * ) )
{
    new( this )( CStackMemStor );
    UseBuf( first_buf, size, release );
}

template< typename Unit, int frag >
CStackMemStor< Unit, frag >::~CStackMemStor()
{}

template< typename Unit, int frag >
CStackMemStor< Unit, frag > *CStackMemStor< Unit, frag >::Spawn()
{
    ++num_children;
    return new( CStackMemStor )( this );
}

template< typename Unit, int frag >
CStackMemStor< Unit, frag > *CStackMemStor< Unit, frag >::Create()
{
    return new( CStackMemStor );
}

template< typename Unit, int frag >
CStackMemStor< Unit, frag > *CStackMemStor< Unit, frag >::Create( void *first_buf, int size, void (*release)( void * ) )
{
    return new( CStackMemStor )( first_buf, size, release );
}

template< typename Unit, int frag >
void CStackMemStor< Unit, frag >::Release( CStackMemStor *stor )
{
    if ( stor->num_children )
    {
        PNL_THROW( CInvalidOperation, "attempt to release stack memory storage with active children" );
    }
    stor->Empty();
    stor->Abjure();
    if ( stor->master )
    {
        --stor->master->num_children;
    }
    delete( stor );
}

template< typename Unit, int frag >
void CStackMemStor< Unit, frag >::Release()
{
    Release( this );
}

template< typename Unit, int frag >
void CStackMemStor< Unit, frag >::Empty()
{
    if ( bin[1] )
    {
        bin[1]->next = vac[0];
        vac[0] = bin[0];
        if ( vac[1] == 0 )
        {
            vac[1] = bin[1];
        }
        bin[0] = bin[1] = 0;
    }
    free = 0;
}

template< typename Unit, int frag >
void CStackMemStor< Unit, frag >::Abjure()
{
    Seg *sg[2];

    if ( master )
    {
        if ( vac[1] )
        {
            vac[1]->next = master->vac[0];
            master->vac[0] = vac[0];
            vac[0] = vac[1] = 0;
        }
        return;
    }
    for ( sg[0] = vac[0]; sg[0]; sg[0] = sg[1] )
    {
        sg[1] = sg[0]->next;
        if ( sg[0]->release )
        {
            (*sg[0]->release)( sg[0]->mem );
        }
    }
    vac[0] = vac[1] = 0;
}

template< typename Unit, int frag >
int CStackMemStor< Unit, frag >::GetNumChildren()
{
    return num_children;
}

template< typename Unit, int frag >
typename CStackMemStor< Unit, frag >::Seg *CStackMemStor< Unit, frag >::Req()
{
    Seg *rv;
    int sz;

    if ( (rv = vac[0]) != 0 )
    {
        vac[0] = rv->next;
        if ( vac[0] == 0 )
        {
            vac[1] = 0;
        }
        return rv;
    }
    if ( master )
    {
        return master->Req();
    }
    sz = frag * sizeof( Unit ) + PNL_ALIGN_CAPACITY( Seg, Unit, 1 ) * sizeof( Unit );
    ForceUseBuf( operator new( sz ), sz, &operator delete );
    return Req();
}

template< typename Unit, int frag >
void *CStackMemStor< Unit, frag >::SOS( int bytes )
{
    Seg *sg;
    int sz;

    if ( bytes <= sizeof( Unit ) * frag )
    {
    action:
        sg = Req();
        sg->next = bin[0];
        bin[0] = sg;
        if ( bin[1] == 0 )
        {
            bin[1] = sg;
        }
        free = sg->size;
        top = (char *)sg->mem + sg->size;
        return RequestMem( bytes );
    }
    sz = PNL_ALIGN_CAPACITY( char, Unit, bytes ) * sizeof( Unit )
         + PNL_ALIGN_CAPACITY( Seg, Unit, 1 ) * sizeof( Unit );
    sz = PNL_ALIGN_CAPACITY( char, Unit, sz ) * sizeof( Unit );
    ForceUseBuf( operator new( sz ), sz, &operator delete );
    goto action;
}

template< typename Unit, int frag >
void CStackMemStor< Unit, frag >::Dump()
{
    Log dump( "StackMemStor: ", eLOG_RESULT, eLOGSRV_PNL );

    Seg *sg;
    dump << "vac " << vac[0] << " " << vac[1] << '\n';
    for ( sg = vac[0]; sg; sg = sg->next )
    {
        dump << sg << '\n';
    }
    dump << "bin " << bin[0] << " " << bin[1] << '\n';
    for ( sg = bin[0]; sg; sg = sg->next )
    {
        dump << sg << '\n';
    }
    dump.flush();
}

#if !defined( PNL_VC6 ) && !defined( PNL_VC7 )
template< typename T, typename Unit, int frag >
CStackMemProv< T, Unit, frag >::CStackMemProv(): stor( Stor::Create() )
{}

template< typename T, typename Unit, int frag >
CStackMemProv< T, Unit, frag >::CStackMemProv( Stor *stor ): stor( stor )
{}

template< typename T, typename Unit, int frag >
template< typename U >
CStackMemProv< T, Unit, frag >::CStackMemProv( CStackMemProv< U, Unit, frag > const &prov ): stor( prov.stor )
{}

template< typename T, typename Unit, int frag >
CStackMemProv< T, Unit, frag >::CStackMemProv( void *first_buf, int size, void (*release)( void * ) )
{
    stor = Stor::Create( first_buf, size, release );
}

template< typename T, typename Unit, int frag >
void CStackMemProv< T, Unit, frag >::Release()
{
    stor->Release();
}

template< typename T, typename Unit, int frag >
CStackMemProv< T, Unit, frag >::operator CStackMemStor< Unit, frag > *()
{
    return stor;
}

template< typename T, typename Unit, int frag >
CStackMemStor< Unit, frag > *CStackMemProv< T, Unit, frag >::operator->()
{
    return stor;
}

template< typename T, typename Unit, int frag >
T *CStackMemProv< T, Unit, frag >::address( T &ref )
{
    return &ref;
}

template< typename T, typename Unit, int frag >
T const *CStackMemProv< T, Unit, frag >::address( T const &ref )
{
    return &ref;
}

template< typename T, typename Unit, int frag >
T *CStackMemProv< T, Unit, frag >::allocate( size_type n, void * )
{
    return (T *)stor->RequestMem( n * sizeof( T ) );
}

template< typename T, typename Unit, int frag >
void CStackMemProv< T, Unit, frag >::deallocate( T *, size_type )
{}

template< typename T, typename Unit, int frag >
typename CStackMemProv< T, Unit, frag >::size_type CStackMemProv< T, Unit, frag >::max_size()
{
    return (unsigned)INT_MAX / sizeof( T );
}

template< typename T, typename Unit, int frag >
bool CStackMemProv< T, Unit, frag >::operator==( CStackMemProv const &rhs ) const
{
    return stor == rhs.stor;
}

template< typename T, typename Unit, int frag >
void CStackMemProv< T, Unit, frag >::construct( T *p, T const &t )
{
    new( p )( T )( t );
}

template< typename T, typename Unit, int frag >
void CStackMemProv< T, Unit, frag >::destroy( T *p )
{
    p->~T();
}
#endif

PNL_END

#endif

/* end of file */
