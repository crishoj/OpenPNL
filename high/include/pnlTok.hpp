#ifndef __PNLTOK_HPP__
#define __PNLTOK_HPP__

#include "pnlHighConf.hpp"

#pragma warning(push, 2)
#include <limits>
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>
#if defined(_MSC_VER)
#pragma warning(disable : 4146) // unary minus operator applied to unsigned type, result still unsigned
#pragma warning(disable : 4284) // return type for 'T::Op' is 'X' (ie; not a UDT or reference to a UDT.  Will produce errors if applied using infix notation)
#endif
#include <deque>
#if defined(_MSC_VER)
#pragma warning(default : 4146)
#pragma warning(default : 4284)
#endif
#include <map>
#pragma warning(pop)

#include "pnlConfig.hpp"
#include "pnlException.hpp"

struct PNLHIGH_API TokId
{
    // internal logic relies on TokId::Match being transitive
    inline bool Match( TokId const & ) const;

    inline bool operator<( TokId const & ) const;
    inline operator String() const;

    inline TokId( String const & );
    inline TokId( char const * );
    inline TokId( int );

    String id;
    bool is_int;
    int int_id;

    inline void Init( String const &s );
};

PNLHIGH_API TokId::TokId( String const &s )
{
    Init( s );
}

PNLHIGH_API TokId::TokId( char const s[] )
{
    Init( s );
}

PNLHIGH_API TokId::TokId( int i ): is_int( true ), int_id( i )
{}

PNLHIGH_API void TokId::Init( String const &s )
{
    int i;

    for ( i = s.size(); i--; )
    {
        if ( !isdigit( s.c_str()[i] ) )
        {
going_str:
            is_int = false;
            id = s;
            return;
        }
    }
    if ( s.size() )
    {
        is_int = true;
        sscanf( s.c_str(), "%d", &int_id );
    }
    else
    {
	goto going_str;
    }
}

PNLHIGH_API bool TokId::Match( TokId const &rhs ) const
{
    switch ( is_int + rhs.is_int )
    {
    case 0:
        return id == rhs.id;
    case 2:
        return int_id == rhs.int_id;
    }
    return false;
}

PNLHIGH_API bool TokId::operator<( TokId const &rhs ) const
{
    if ( is_int )
    {
        if ( rhs.is_int )
        {
            return int_id < rhs.int_id;
        }
        return false;
    }
    if ( rhs.is_int )
    {
        return true;
    }
    return id < rhs.id;
}

PNLHIGH_API TokId::operator String() const
{
    if ( !is_int )
    {
        return id;
    }
    
    return String() << int_id;
}

struct PNLHIGH_API TokIdNode
{
    struct Matcher
    {
        int imm;
        Matcher *chain[2];

        enum Type
        {
            positive,
            bit_eq,
            bit_all,
            bit_any,
            chain_not,
            chain_and,
            chain_or,
            chain_xor
        } type;

        Matcher( int tag ): imm( tag ), type( bit_eq )
        {
            chain[0] = chain[1] = 0;
        }

        Matcher(): type( positive )
        {
            chain[0] = chain[1] = 0;
        }

        Matcher( Type type ): type( type )
        {
            chain[0] = chain[1] = 0;
            if ( type != positive )
            {
                PNL_THROW( CBadArg, "Matcher without additional parameters must be of positive type" );
            }
        }

        Matcher( Type type, int tag ): imm( tag ), type( type )
        {
            chain[0] = chain[1] = 0;
            switch ( type )
            {
            case bit_eq:
            case bit_all:
            case bit_any:
                break;
            default:
                PNL_THROW( CBadArg, "Matcher with one immediate parameter must be of bit_* type" );
            }
        }

        Matcher( Type type, Matcher const &chain ): type( type )
        {
            if ( type != chain_not )
            {
                PNL_THROW( CBadArg, "Matcher with one chain parameter must be of chain_not type" );
            }
            this->chain[0] = new( Matcher )( chain );
            this->chain[1] = 0;
        }

        Matcher( Type type, Matcher const &chain1, Matcher const &chain2 ): type( type )
        {
            switch ( type )
            {
            case chain_and:
            case chain_or:
            case chain_xor:
                break;
            default:
                PNL_THROW( CBadArg, "Matcher with two chain parameters must be of type chain_and, chain_or or chain_xor" );
            }
            chain[0] = new( Matcher )( chain1 );
            chain[1] = new( Matcher )( chain2 );
        }

        Matcher( Matcher const &m ): type( m.type ), imm( m.imm )
        {
            switch ( type )
            {
            case chain_not:
                chain[0] = new( Matcher )( *m.chain[0] );
                chain[1] = 0;
            case chain_and:
            case chain_or:
            case chain_xor:
                chain[1] = new( Matcher )( *m.chain[1] );
                break;
            default:
                chain[0] = chain[1] = 0;
            }
        }

        Matcher const &operator=( Matcher const &m )
        {
            if ( this != &m )
            {
                delete( chain[0] );
                delete( chain[1] );
                type = m.type;
                imm = m.imm;
                if ( m.chain[0] )
                {
                    chain[0] = new( Matcher )( *m.chain[0] );
                }
                if ( m.chain[1] )
                {
                    chain[1] = new( Matcher )( *m.chain[1] );
                }
            }
            return *this;
        }

        bool Match( TokIdNode const *node ) const
        {
            switch ( type )
            {
            case positive:
                return true;
            case bit_eq:
                return node->tag == imm;
            case bit_all:
                return (node->tag & imm) == imm;
            case bit_any:
                return node->tag & imm;
            case chain_not:
                return !chain[0]->Match( node );
            case chain_and:
                return chain[0]->Match( node ) && chain[1]->Match( node );
            case chain_or:
                return chain[0]->Match( node ) || chain[1]->Match( node );
            case chain_xor:
                return chain[0]->Match( node ) ^ chain[1]->Match( node );
            default:
                PNL_THROW( CBadArg, "Matcher appears to have invalid type" );
            }
        }

        ~Matcher()
        {
            delete( chain[0] );
            delete( chain[1] );
        }
    };

    static TokIdNode *root;
    static Vector< TokIdNode * > cemetery;

    inline TokIdNode();
    inline explicit TokIdNode( TokId );

    inline bool Match( TokId const & ) const;
    bool Match( std::deque< TokId >::const_iterator, int len, TokIdNode const *subroot = root ) const;
    bool SubMatch( std::deque< TokId >::const_iterator, int len, TokIdNode const *subroot = root ) const;

    static inline TokIdNode *GetLeaf( TokIdNode const *subroot = root );
    static inline TokIdNode *GetNextLeaf( TokIdNode const *node, TokIdNode const *subroot = root );
    static inline TokIdNode *GetNext( TokIdNode const *node, TokIdNode const *subroot = root );

    static inline std::vector< std::pair< TokIdNode *, int > > AmbigResolve( TokIdNode const *node,
                                                                             std::deque< TokId >::const_iterator arr,
                                                                             int len,
                                                                             TokIdNode const *context = root,
                                                                             TokIdNode const *subroot = root );
    static std::vector< std::pair< TokIdNode *, int > > AmbigResolve( TokIdNode const *node,
                                                                      std::deque< TokId >::const_iterator arr,
                                                                      int len,
                                                                      Matcher const &,
                                                                      TokIdNode const *context = root,
                                                                      TokIdNode const *subroot = root );
    static inline std::pair< TokIdNode *, int > Resolve( TokIdNode const *,
                                                         std::deque< TokId >::const_iterator,
                                                         int len,
                                                         TokIdNode const *hard_context = root,
                                                         TokIdNode const *soft_context = root,
                                                         TokIdNode const *subroot = root );
    static std::pair< TokIdNode *, int > Resolve( TokIdNode const *,
                                                         std::deque< TokId >::const_iterator,
                                                         int len,
                                                         Matcher const &hard_matcher,
                                                         TokIdNode const *hard_context = root,
                                                         TokIdNode const *soft_context = root,
                                                         TokIdNode const *subroot = root );
    static std::pair< TokIdNode *, int > Resolve( TokIdNode const *,
                                                  std::deque< TokId >::const_iterator,
                                                  int len,
                                                  Matcher const &hard_matcher,
                                                  Matcher const &soft_matcher,
                                                  TokIdNode const *hard_context = root,
                                                  TokIdNode const *soft_context = root,
                                                  TokIdNode const *subroot = root );
    static bool Match( TokIdNode const *node1, std::deque< TokId >::const_iterator unres1, int len1,
                       TokIdNode const *node2, std::deque< TokId >::const_iterator unres2, int len2 );
    inline bool IsInContext( TokIdNode const * ) const;
    inline String Name() const;

    TokIdNode *Add( TokId const & );
    void Alias( TokId const & );
    void Unalias( TokId const & );
    void Remove( bool no_cemetery = false );
    inline void Kill();
    static void PurgeCemetery();

    Vector< TokId > id;
    int level; // root has level zero, invalid nodes have negative level
    int tag;
    void *data;

    TokIdNode *v_next;
    TokIdNode *v_prev;
    TokIdNode *h_next;
    TokIdNode *h_prev;

    // contains all descentants of that node for purposes of quick looking up
    typedef std::multimap< TokId, TokIdNode * > Map;
    Map desc;
};

PNLHIGH_API TokIdNode::TokIdNode(): h_next( 0 ), h_prev( 0 ), v_next( 0 ), v_prev( 0 ), level( 0 ), tag( 0 ), data( 0 )
{}

PNLHIGH_API TokIdNode::TokIdNode( TokId id ): h_next( 0 ), h_prev( 0 ), v_next( 0 ), v_prev( 0 ), level( 0 ), tag( 0 ), data( 0 )
{
    Alias( id );
}

PNLHIGH_API bool TokIdNode::Match( TokId const &rhs ) const
{
    int i;

    for ( i = id.size(); i--; )
    {
        if ( id[i].Match( rhs ) )
        {
            return true;
        }
    }
    return false;
}

PNLHIGH_API void TokIdNode::Kill()
{
    Remove( true );
}

PNLHIGH_API TokIdNode *TokIdNode::GetLeaf( TokIdNode const *subroot )
{
    TokIdNode const *nd;
    
    for ( nd = subroot; nd->v_next; nd = nd->v_next ) ;
    return (TokIdNode *)nd;
}

PNLHIGH_API TokIdNode *TokIdNode::GetNext( TokIdNode const *node, TokIdNode const *subroot )
{
    if ( node->v_prev == 0 || node == subroot )
    {
        return 0;
    }
    if ( node->h_next )
    {
        return GetLeaf( node->h_next );
    }
    return (TokIdNode *)node->v_prev;
}

PNLHIGH_API TokIdNode *TokIdNode::GetNextLeaf( TokIdNode const *node, TokIdNode const *subroot )
{
    TokIdNode const *nd;

    for ( nd = node; nd; nd = GetNext( node, subroot ) )
    {
        if ( nd->v_next == 0 )
        {
            return (TokIdNode *)nd;
        }
    }
    return 0;
}

PNLHIGH_API bool TokIdNode::IsInContext( TokIdNode const *context ) const
{
    TokIdNode const *nd;

    for ( nd = this; nd != context && nd != 0; nd = nd->v_prev ) ;
    return nd == context;
}

PNLHIGH_API String TokIdNode::Name() const
{
    return id[0];
}

PNLHIGH_API std::vector< std::pair< TokIdNode *, int > > TokIdNode::AmbigResolve( TokIdNode const *node,
                                                                                  std::deque< TokId >::const_iterator arr,
                                                                                  int len,
                                                                                  TokIdNode const *context,
                                                                                  TokIdNode const *subroot )
{
    return AmbigResolve( node, arr, len, Matcher(), context, subroot );
}

PNLHIGH_API std::pair< TokIdNode *, int > TokIdNode::Resolve( TokIdNode const *node,
                                                              std::deque< TokId >::const_iterator arr,
                                                              int len,
                                                              TokIdNode const *hard_context,
                                                              TokIdNode const *soft_context,
                                                              TokIdNode const *subroot )
{
    return Resolve( node, arr, len, Matcher(), Matcher(), hard_context, soft_context, subroot );
}

template< bool exploit_nan >
struct PNLHIGH_API TokFlt
{
    float fl;
    bool undef;
    
    bool IsUndef() const
    {
        return undef;
    }

    TokFlt(): undef( true )
    {}

    TokFlt( float fl ): undef( false ), fl( fl )
    {}

    operator String()
    {
	char buf[128];

	if ( IsUndef() )
	{
	    return ".";
	}
	sprintf( buf, "%f", fl );
	return buf;
    }
};

template<>
struct PNLHIGH_API TokFlt< true >
{
    float fl;
    
    bool IsUndef() const
    {
        // comparison is that way because (nan == real_number) may be true
        return fl == 0.f && fl == 1.f
               || 0.f != std::numeric_limits< float >::quiet_NaN() && fl == std::numeric_limits< float >::quiet_NaN();
    }
    
    TokFlt(): fl( std::numeric_limits< float >::quiet_NaN() )
    {}

    TokFlt( float fl ): fl( fl )
    {}

    operator String()
    {
	char buf[128];

	if ( IsUndef() )
	{
	    return ".";
	}
	sprintf( buf, "%f", fl );
	return buf;
    }
};

// Tok and TokArr are external interface:
struct TokArr;

struct PNLHIGH_API Tok
{
    typedef TokFlt< std::numeric_limits< float >::has_quiet_NaN > F;
    typedef TokIdNode::Matcher Matcher;

    static Tok root;
    static Tok nil;

    inline Tok();
    inline Tok( String const & );
    inline Tok( char const * );
    inline Tok( int );
    inline Tok( float );
    inline Tok( F );
    inline Tok( std::vector< float > const & );
    inline Tok( std::vector< F > const & );
    explicit inline Tok( TokArr const & );

    operator String() const;
    inline bool operator==( Tok const & ) const;
    inline bool operator!=( Tok const &rhs ) const;

    inline Tok const &operator^=( Tok const & );

    inline bool IsUnres() const;
    void Resolve( int hint_num_nodes,
                  Matcher const &hard_matcher,
                  Matcher const &soft_matcher,
                  TokIdNode const *hard_context = 0,
                  TokIdNode const *soft_context = 0,
                  TokIdNode const *subroot = 0 );
    inline void Resolve( int hint_num_nodes,
                         Matcher const &hard_matcher,
                         TokIdNode const *hard_context = 0,
                         TokIdNode const *soft_context = 0,
                         TokIdNode const *subroot = 0 );
    inline void Resolve( int hint_num_nodes,
                         TokIdNode const *hard_context = 0,
                         TokIdNode const *soft_context = 0,
                         TokIdNode const *subroot = 0 );
    inline void Resolve( Matcher const &hard_matcher,
                         Matcher const &soft_matcher,
                         TokIdNode const *hard_context = 0,
                         TokIdNode const *soft_context = 0,
                         TokIdNode const *subroot = 0 );
    inline void Resolve( Matcher const &hard_matcher,
                         TokIdNode const *hard_context = 0,
                         TokIdNode const *soft_context = 0,
                         TokIdNode const *subroot = 0 );
    inline void Resolve( TokIdNode const *hard_context = 0,
                         TokIdNode const *soft_context = 0,
                         TokIdNode const *subroot = 0 );
    inline void Resolve( int hint_num_nodes,
                         Matcher const &hard_matcher,
                         Matcher const &soft_matcher,
                         Tok hard_context,
                         Tok soft_context = nil,
                         Tok subroot = nil );
    inline void Resolve( int hint_num_nodes,
                         Matcher const &hard_matcher,
                         Tok hard_context,
                         Tok soft_context = nil,
                         Tok subroot = nil );
    inline void Resolve( int hint_num_nodes,
                         Tok hard_context,
                         Tok soft_context = nil,
                         Tok subroot = nil );
    inline void Resolve( Matcher const &hard_matcher,
                         Matcher const &soft_matcher,
                         Tok hard_context,
                         Tok soft_context = nil,
                         Tok subroot = nil );
    inline void Resolve( Matcher const &hard_matcher,
                         Tok hard_context,
                         Tok soft_context = nil,
                         Tok subroot = nil );
    inline void Resolve( Tok hard_context,
                         Tok soft_context = nil,
                         Tok subroot = nil );
    static TokArr PropagateFload( Tok const & );
    inline void Cat( Tok const & );
    inline static Tok Cat( Tok const &, Tok const & );
    bool Match( Tok const & ) const;
    void Unresolve( TokIdNode const *subroot = TokIdNode::root );
    void Unresolve( Matcher const & );
    inline TokIdNode *Node( TokIdNode const * = TokIdNode::root ) const;
    inline TokIdNode *Node( Tok const & ) const;
    inline TokIdNode *Node( int i, TokIdNode const * = TokIdNode::root ) const;
    inline TokIdNode *Node( int i, Tok const & ) const;
    inline String Name() const;
    inline std::deque< TokId > Unresolved( TokIdNode const *context = TokIdNode::root );
    inline std::deque< TokId > Unresolved( Tok const &context );
    inline F FltValue( int = 0 ) const;
    inline int IntValue() const;
    inline bool IsNil() const;
    TokArr GetDescendants() const;
    TokArr GetDescendants( Matcher const & ) const;
    static Tok Context();
    static void Context( Tok const & );

    Vector< TokIdNode * > node;
    Vector< std::deque< TokId > > unres;
    Vector< F > fload;
    mutable TokIdNode *hard_context, *soft_context, *subroot;
    mutable Matcher hard_matcher, soft_matcher;
    mutable int hint_num_nodes;

    inline Tok( TokIdNode * );
    inline Tok( TokIdNode *, TokIdNode * );
    inline Tok( TokIdNode *, F );
    inline Tok( TokIdNode *, std::vector< F > const & );
    inline Tok( TokIdNode *, std::deque< TokId > const & );
    inline Tok( TokIdNode *, std::deque< TokId > const &, std::vector< F > const & );

    char const *Init( char const s[] );
};

PNLHIGH_API Tok::Tok()
    : hard_matcher(),
      soft_matcher(),
      hard_context( TokIdNode::root ),
      soft_context( TokIdNode::root ),
      subroot( TokIdNode::root ),
      hint_num_nodes( -1 )
{
    unres.resize( 1 );
}

PNLHIGH_API Tok::Tok( char const s[] )
    : hard_matcher(),
      soft_matcher(),
      hard_context( TokIdNode::root ),
      soft_context( TokIdNode::root ),
      subroot( TokIdNode::root ),
      hint_num_nodes( -1 )
{
    Init( s );
}

PNLHIGH_API Tok::Tok( String const &s )
    : hard_matcher(),
      soft_matcher(),
      hard_context( TokIdNode::root ),
      soft_context( TokIdNode::root ),
      subroot( TokIdNode::root ),
      hint_num_nodes( -1 )
{
    Init( s.c_str() );
}

PNLHIGH_API Tok::Tok( int i )
    : hard_matcher(),
      soft_matcher(),
      hard_context( TokIdNode::root ),
      soft_context( TokIdNode::root ),
      subroot( TokIdNode::root ),
      hint_num_nodes( -1 )
{
    unres.resize( 1 );
    unres[0].push_back( TokId( i ) );
}

PNLHIGH_API Tok::Tok( float fl )
    : hard_matcher(),
      soft_matcher(),
      hard_context( TokIdNode::root ),
      soft_context( TokIdNode::root ),
      subroot( TokIdNode::root ),
      hint_num_nodes( -1 )
{
    unres.push_back( std::deque< TokId >() );
    fload.push_back( fl );
}

PNLHIGH_API Tok::Tok( F fl )
    : hard_matcher(),
      soft_matcher(),
      hard_context( TokIdNode::root ),
      soft_context( TokIdNode::root ),
      subroot( TokIdNode::root ),
      hint_num_nodes( -1 )
{
    unres.push_back( std::deque< TokId >() );
    fload.push_back( fl );
}

PNLHIGH_API Tok::Tok( std::vector< float > const &fload )
    : hard_matcher(),
      soft_matcher(),
      hard_context( TokIdNode::root ),
      soft_context( TokIdNode::root ),
      subroot( TokIdNode::root ),
      hint_num_nodes( -1 )
{
    int i;

    unres.push_back( std::deque< TokId >() );
    this->fload.resize( fload.size() );
    for ( i = fload.size(); i--; )
    {
        this->fload[i] = fload[i];
    }
}

PNLHIGH_API Tok::Tok( std::vector< F > const &fload )
    : hard_matcher(),
      soft_matcher(),
      hard_context( TokIdNode::root ),
      soft_context( TokIdNode::root ),
      subroot( TokIdNode::root ),
      hint_num_nodes( -1 ),
      fload( fload.begin(), fload.end() )
{
    unres.push_back( std::deque< TokId >() );
}

PNLHIGH_API Tok::Tok( TokIdNode *node )
    : hard_matcher(),
      soft_matcher(),
      hard_context( TokIdNode::root ),
      soft_context( TokIdNode::root ),
      subroot( TokIdNode::root ),
      hint_num_nodes( -1 )
{
    this->node.push_back( node );
    unres.resize( 2 );
}

PNLHIGH_API Tok::Tok( TokIdNode *node, std::vector< F > const &fload )
    : hard_matcher(),
      soft_matcher(),
      hard_context( TokIdNode::root ),
      soft_context( TokIdNode::root ),
      subroot( TokIdNode::root ),
      hint_num_nodes( -1 ),
      fload( fload.begin(), fload.end() )
{
    unres.resize( 1 );
}

PNLHIGH_API Tok::Tok( TokIdNode *node, F fl )
    : hard_matcher(),
      soft_matcher(),
      hard_context( TokIdNode::root ),
      soft_context( TokIdNode::root ),
      subroot( TokIdNode::root ),
      hint_num_nodes( -1 )
{
    unres.resize( 1 );
    fload.push_back( fl );
}

PNLHIGH_API Tok::Tok( TokIdNode *node, std::deque< TokId > const &unres )
    : hard_matcher(),
      soft_matcher(),
      hard_context( TokIdNode::root ),
      soft_context( TokIdNode::root ),
      subroot( TokIdNode::root ),
      hint_num_nodes( -1 )
{
    this->node.push_back( node );
    this->unres.resize( 2 );
    this->unres[1] = unres;
}

PNLHIGH_API Tok::Tok( TokIdNode *node, std::deque< TokId > const &unres, std::vector< F > const &fload )
    : hard_matcher(),
      soft_matcher(),
      hard_context( TokIdNode::root ),
      soft_context( TokIdNode::root ),
      subroot( TokIdNode::root ),
      hint_num_nodes( -1 ),
      fload( fload.begin(), fload.end() )
{
    this->node.push_back( node );
    this->unres.resize( 2 );
    this->unres[1] = unres;
}

PNLHIGH_API bool Tok::operator==( Tok const &rhs ) const
{
    int i;

    if ( (unres[0].size() || node.size()) && (rhs.unres[0].size() || rhs.node.size()) )
    {
        i = Match( rhs );
        if ( fload.empty() || rhs.fload.empty() )
        {
            return i;
        }
        if ( !i )
        {
            return false;
        }
    }
    if ( fload.size() != rhs.fload.size() )
    {
        return false;
    }
    for ( i = fload.size(); i--; )
    {
        if ( !(fload[i].IsUndef() || rhs.fload[i].IsUndef()) )
        {
            if ( fload[i].fl != rhs.fload[i].fl )
            {
                return false;
            }
        }
    }
    return true;
}

PNLHIGH_API bool Tok::operator!=( Tok const &rhs ) const
{
    return !(*this == rhs);
}

PNLHIGH_API bool Tok::IsUnres() const
{
    int i, j;
    for ( i = unres.size(), j = 0; i--; )
    {
        j += unres[i].size();
    }
    return j > 0;
}

PNLHIGH_API bool Tok::IsNil() const
{
    return node.empty() && unres[0].empty() && fload.empty();
}

PNLHIGH_API void Tok::Cat( Tok const &rhs )
{
    unsigned i;

    for ( i = 0; i < rhs.unres[0].size(); ++i )
    {
        unres.back().push_back( rhs.unres[0][i] );
    }
    for ( i = 1; i < rhs.unres.size(); ++i )
    {
        unres.push_back( rhs.unres[i] );
    }
    for ( i = 0; i < rhs.node.size(); ++i )
    {
        node.push_back( rhs.node[i] );
    }
    for ( i = 0; i < rhs.fload.size(); ++i )
    {
        fload.push_back( rhs.fload[i] );
    }
}

PNLHIGH_API Tok Tok::Cat( Tok const &a, Tok const &b )
{
    Tok c = a;

    c.Cat( b );
    return c;
}

PNLHIGH_API void Tok::Resolve( int hint_num_nodes,
                               Matcher const &hard_matcher,
                               TokIdNode const *hard_context,
                               TokIdNode const *soft_context,
                               TokIdNode const *subroot )
{
    Resolve( hint_num_nodes, hard_matcher, Matcher( Matcher::chain_and, soft_matcher, hard_matcher ),
             hard_context, soft_context, subroot );
}

PNLHIGH_API void Tok::Resolve( int hint_num_nodes,
                               TokIdNode const *hard_context,
                               TokIdNode const *soft_context,
                               TokIdNode const *subroot )
{
    Resolve( hint_num_nodes, hard_matcher, soft_matcher,
             hard_context, soft_context, subroot );
}

PNLHIGH_API void Tok::Resolve( Matcher const &hard_matcher,
                               Matcher const &soft_matcher,
                               TokIdNode const *hard_context,
                               TokIdNode const *soft_context,
                               TokIdNode const *subroot )
{
    Resolve( -1, hard_matcher, soft_matcher, hard_context, soft_context, subroot );
}

PNLHIGH_API void Tok::Resolve( Matcher const &hard_matcher,
                               TokIdNode const *hard_context,
                               TokIdNode const *soft_context,
                               TokIdNode const *subroot )
{
    Resolve( -1, hard_matcher, Matcher( Matcher::chain_and, soft_matcher, hard_matcher ),
             hard_context, soft_context, subroot );
}

PNLHIGH_API void Tok::Resolve( TokIdNode const *hard_context,
                               TokIdNode const *soft_context,
                               TokIdNode const *subroot )
{
    Resolve( -1, hard_matcher, soft_matcher,
             hard_context, soft_context, subroot );
}

PNLHIGH_API void Tok::Resolve( int hint_num_nodes,
                               Matcher const &hard_matcher,
                               Matcher const &soft_matcher,
                               Tok hard_context,
                               Tok soft_context,
                               Tok subroot )
{
    TokIdNode *hard_context_node, *soft_context_node, *subroot_node;

    if ( hard_context.IsNil() )
    {
        hard_context_node = 0;
    }
    else
    {
	hard_context.Resolve();
	if ( hard_context.node.size() != 1 || hard_context.IsUnres() )
        {
            PNL_THROW( CBadArg, "context must be a single resolved node" );
        }
        hard_context_node = hard_context.Node();
    }
    if ( soft_context.IsNil() )
    {
        soft_context_node = 0;
    }
    else
    {
	soft_context.Resolve();
        if ( soft_context.node.size() != 1 || soft_context.IsUnres() )
        {
            PNL_THROW( CBadArg, "context must be a single resolved node" );
        }
        soft_context_node = soft_context.Node();
    }
    if ( subroot.IsNil() )
    {
        subroot_node = 0;
    }
    else
    {
	subroot.Resolve();
        if ( subroot.node.size() != 1 || subroot.IsUnres() )
        {
            PNL_THROW( CBadArg, "subroot must be a single resolved node" );
        }
        subroot_node = subroot.Node();
    }

    Resolve( hint_num_nodes, hard_matcher, soft_matcher,
             hard_context_node, soft_context_node, subroot_node );
}

PNLHIGH_API void Tok::Resolve( int hint_num_nodes,
                               Matcher const &hard_matcher,
                               Tok hard_context,
                               Tok soft_context,
                               Tok subroot )
{
    TokIdNode *hard_context_node, *soft_context_node, *subroot_node;

    if ( hard_context.IsNil() )
    {
        hard_context_node = 0;
    }
    else
    {
	hard_context.Resolve();
	if ( hard_context.node.size() != 1 || hard_context.IsUnres() )
        {
            PNL_THROW( CBadArg, "context must be a single resolved node" );
        }
        hard_context_node = hard_context.Node();
    }
    if ( soft_context.IsNil() )
    {
        soft_context_node = 0;
    }
    else
    {
	soft_context.Resolve();
        if ( soft_context.node.size() != 1 || soft_context.IsUnres() )
        {
            PNL_THROW( CBadArg, "context must be a single resolved node" );
        }
        soft_context_node = soft_context.Node();
    }
    if ( subroot.IsNil() )
    {
        subroot_node = 0;
    }
    else
    {
	subroot.Resolve();
        if ( subroot.node.size() != 1 || subroot.IsUnres() )
        {
            PNL_THROW( CBadArg, "subroot must be a single resolved node" );
        }
        subroot_node = subroot.Node();
    }

    Resolve( hint_num_nodes, hard_matcher,
             hard_context_node, soft_context_node, subroot_node );
}

PNLHIGH_API void Tok::Resolve( int hint_num_nodes,
                               Tok hard_context,
                               Tok soft_context,
                               Tok subroot )
{
    TokIdNode *hard_context_node, *soft_context_node, *subroot_node;

    if ( hard_context.IsNil() )
    {
        hard_context_node = 0;
    }
    else
    {
	hard_context.Resolve();
	if ( hard_context.node.size() != 1 || hard_context.IsUnres() )
        {
            PNL_THROW( CBadArg, "context must be a single resolved node" );
        }
        hard_context_node = hard_context.Node();
    }
    if ( soft_context.IsNil() )
    {
        soft_context_node = 0;
    }
    else
    {
	soft_context.Resolve();
        if ( soft_context.node.size() != 1 || soft_context.IsUnres() )
        {
            PNL_THROW( CBadArg, "context must be a single resolved node" );
        }
        soft_context_node = soft_context.Node();
    }
    if ( subroot.IsNil() )
    {
        subroot_node = 0;
    }
    else
    {
	subroot.Resolve();
        if ( subroot.node.size() != 1 || subroot.IsUnres() )
        {
            PNL_THROW( CBadArg, "subroot must be a single resolved node" );
        }
        subroot_node = subroot.Node();
    }

    Resolve( hint_num_nodes, hard_context_node, soft_context_node, subroot_node );
}

PNLHIGH_API void Tok::Resolve( Matcher const &hard_matcher,
                               Matcher const &soft_matcher,
                               Tok hard_context,
                               Tok soft_context,
                               Tok subroot )
{
    TokIdNode *hard_context_node, *soft_context_node, *subroot_node;

    if ( hard_context.IsNil() )
    {
        hard_context_node = 0;
    }
    else
    {
	hard_context.Resolve();
	if ( hard_context.node.size() != 1 || hard_context.IsUnres() )
        {
            PNL_THROW( CBadArg, "context must be a single resolved node" );
        }
        hard_context_node = hard_context.Node();
    }
    if ( soft_context.IsNil() )
    {
        soft_context_node = 0;
    }
    else
    {
	soft_context.Resolve();
        if ( soft_context.node.size() != 1 || soft_context.IsUnres() )
        {
            PNL_THROW( CBadArg, "context must be a single resolved node" );
        }
        soft_context_node = soft_context.Node();
    }
    if ( subroot.IsNil() )
    {
        subroot_node = 0;
    }
    else
    {
	subroot.Resolve();
        if ( subroot.node.size() != 1 || subroot.IsUnres() )
        {
            PNL_THROW( CBadArg, "subroot must be a single resolved node" );
        }
        subroot_node = subroot.Node();
    }

    Resolve( -1, hard_matcher, soft_matcher,
             hard_context_node, soft_context_node, subroot_node );
}

PNLHIGH_API void Tok::Resolve( Matcher const &hard_matcher,
                               Tok hard_context,
                               Tok soft_context,
                               Tok subroot )
{
    TokIdNode *hard_context_node, *soft_context_node, *subroot_node;

    if ( hard_context.IsNil() )
    {
        hard_context_node = 0;
    }
    else
    {
	hard_context.Resolve();
	if ( hard_context.node.size() != 1 || hard_context.IsUnres() )
        {
            PNL_THROW( CBadArg, "context must be a single resolved node" );
        }
        hard_context_node = hard_context.Node();
    }
    if ( soft_context.IsNil() )
    {
        soft_context_node = 0;
    }
    else
    {
	soft_context.Resolve();
        if ( soft_context.node.size() != 1 || soft_context.IsUnres() )
        {
            PNL_THROW( CBadArg, "context must be a single resolved node" );
        }
        soft_context_node = soft_context.Node();
    }
    if ( subroot.IsNil() )
    {
        subroot_node = 0;
    }
    else
    {
	subroot.Resolve();
        if ( subroot.node.size() != 1 || subroot.IsUnres() )
        {
            PNL_THROW( CBadArg, "subroot must be a single resolved node" );
        }
        subroot_node = subroot.Node();
    }

    Resolve( -1, hard_matcher, hard_context_node, soft_context_node, subroot_node );
}

PNLHIGH_API void Tok::Resolve( Tok hard_context,
                               Tok soft_context,
                               Tok subroot )
{
    TokIdNode *hard_context_node, *soft_context_node, *subroot_node;

    if ( hard_context.IsNil() )
    {
        hard_context_node = 0;
    }
    else
    {
	hard_context.Resolve();
	if ( hard_context.node.size() != 1 || hard_context.IsUnres() )
        {
            PNL_THROW( CBadArg, "context must be a single resolved node" );
        }
        hard_context_node = hard_context.Node();
    }
    if ( soft_context.IsNil() )
    {
        soft_context_node = 0;
    }
    else
    {
	soft_context.Resolve();
        if ( soft_context.node.size() != 1 || soft_context.IsUnres() )
        {
            PNL_THROW( CBadArg, "context must be a single resolved node" );
        }
        soft_context_node = soft_context.Node();
    }
    if ( subroot.IsNil() )
    {
        subroot_node = 0;
    }
    else
    {
	subroot.Resolve();
        if ( subroot.node.size() != 1 || subroot.IsUnres() )
        {
            PNL_THROW( CBadArg, "subroot must be a single resolved node" );
        }
        subroot_node = subroot.Node();
    }

    Resolve( -1, hard_context_node, soft_context_node, subroot_node );
}

PNLHIGH_API inline Tok operator^( Tok const &a, Tok const &b )
{
    return Tok::Cat( a, b );
}

PNLHIGH_API Tok const &Tok::operator^=( Tok const &rhs )
{
    *this = *this ^ rhs;
    return *this;
}

PNLHIGH_API TokIdNode *Tok::Node( TokIdNode const *context ) const
{
    return Node( 0, context );
}

PNLHIGH_API TokIdNode *Tok::Node( Tok const &context ) const
{
    return Node( 0, context );
}

PNLHIGH_API TokIdNode *Tok::Node( int i, TokIdNode const *context ) const
{
    Tok t = *this;
    t.Resolve( context );
    if ( t.node.size() <= i )
    {
        PNL_THROW( CBadArg, "inexistant node requested" );
    }
    return t.node[i];
}

PNLHIGH_API TokIdNode *Tok::Node( int i, Tok const &context ) const
{
    if ( context.node.size() != 1 || context.IsUnres() )
    {
        PNL_THROW( CBadArg, "context must be a single resolved node" );
    }

    return Node( i, context.node[0] );
}

PNLHIGH_API String Tok::Name() const
{
    if ( unres.back().size() )
    {
        return unres.back().back();
    }
    if ( node.size() != 1 )
    {
        PNL_THROW( CBadArg, "token should be single node" );
    }
    return node[0]->Name();
}

PNLHIGH_API std::deque< TokId > Tok::Unresolved( TokIdNode const *context )
{
    Resolve( context );
    if ( node.size() > 1 )
    {
        PNL_THROW( CBadArg, "multiple nodes not allowed here" );
    }
    return unres.back();
}

PNLHIGH_API std::deque< TokId > Tok::Unresolved( Tok const &context )
{
    if ( context.node.size() != 1 || context.IsUnres() )
    {
        PNL_THROW( CBadArg, "context must be a single resolved node" );
    }

    return Unresolved( context.node[0] );
}

PNLHIGH_API Tok::F Tok::FltValue( int i ) const
{
    return fload.at(i);
}

PNLHIGH_API int Tok::IntValue() const
{
    int i;

    if ( unres.back().size() )
    {
        if ( unres.back().back().is_int )
        {
            return unres.back().back().int_id;
        }
    }
    else
    {
        for ( i = 0; i < node.back()->id.size(); ++i )
        {
            if ( node.back()->id[i].is_int )
            {
                return node.back()->id[i].int_id;
            }
        }
    }

    PNL_THROW( CBadArg, "attempt to extract integer value from token that looks not like integer" );
}

struct PNLHIGH_API TokArr: public std::deque< Tok >
{
    typedef Tok::Matcher Matcher;

    static TokArr nil;

    inline TokArr();
    inline TokArr( Tok const & );
    inline TokArr( Tok const[], int len );
    inline TokArr( float const[], int len );
    inline TokArr( int const[], int len );
    inline TokArr( String const[], int len );
    inline TokArr( char const * const[], int len );
    inline TokArr( char const * );
    inline TokArr( String const & );

    operator String() const;
    inline Tok const &operator[]( int ) const;
    inline Tok &operator[]( int );
    inline TokArr operator[]( Tok const & ) const;

    inline static TokArr Span( int first, int last );
    static TokArr IntersectSets( TokArr const &, TokArr const & );
    static TokArr UniteSets( TokArr const &, TokArr const & );
    static TokArr DiffSets( TokArr const &, TokArr const & );
    static TokArr SymmDiffSets( TokArr const &, TokArr const & );

    inline void Resolve( int hint_num_nodes,
                         Matcher const &hard_matcher,
                         Matcher const &soft_matcher,
                         TokIdNode const *hard_context = 0,
                         TokIdNode const *soft_context = 0,
                         TokIdNode const *subroot = 0 );
    inline void Resolve( int hint_num_nodes,
                         Matcher const &hard_matcher,
                         TokIdNode const *hard_context = 0,
                         TokIdNode const *soft_context = 0,
                         TokIdNode const *subroot = 0 );
    inline void Resolve( int hint_num_nodes,
                         TokIdNode const *hard_context = 0,
                         TokIdNode const *soft_context = 0,
                         TokIdNode const *subroot = 0 );
    inline void Resolve( Matcher const &hard_matcher,
                         Matcher const &soft_matcher,
                         TokIdNode const *hard_context = 0,
                         TokIdNode const *soft_context = 0,
                         TokIdNode const *subroot = 0 );
    inline void Resolve( Matcher const &hard_matcher,
                         TokIdNode const *hard_context = 0,
                         TokIdNode const *soft_context = 0,
                         TokIdNode const *subroot = 0 );
    inline void Resolve( TokIdNode const *hard_context = 0,
                         TokIdNode const *soft_context = 0,
                         TokIdNode const *subroot = 0 );
    inline void Resolve( int hint_num_nodes,
                         Matcher const &hard_matcher,
                         Matcher const &soft_matcher,
                         Tok const &hard_context,
                         Tok const &soft_context = Tok::nil,
                         Tok const &subroot = Tok::nil );
    inline void Resolve( int hint_num_nodes,
                         Matcher const &hard_matcher,
                         Tok const &hard_context,
                         Tok const &soft_context = Tok::nil,
                         Tok const &subroot = Tok::nil );
    inline void Resolve( int hint_num_nodes,
                         Tok const &hard_context,
                         Tok const &soft_context = Tok::nil,
                         Tok const &subroot = Tok::nil );
    inline void Resolve( Matcher const &hard_matcher,
                         Matcher const &soft_matcher,
                         Tok const &hard_context,
                         Tok const &soft_context = Tok::nil,
                         Tok const &subroot = Tok::nil );
    inline void Resolve( Matcher const &hard_matcher,
                         Tok const &hard_context,
                         Tok const &soft_context = Tok::nil,
                         Tok const &subroot = Tok::nil );
    inline void Resolve( Tok const &hard_context,
                         Tok const &soft_context = Tok::nil,
                         Tok const &subroot = Tok::nil );

    void Init( const char s[] );

    virtual ~TokArr() {}

    typedef std::deque< Tok > Base;
};

#if 1 // TEMPORARY FOR DEBUGGING PURPOSES
inline void vistok( Tok const &t )
{
    int i, j;
    std::cout << "==" << std::endl;
    std::cout << t.node.size() << std::endl;
    std::cout << t.unres.size() << std::endl;
    for ( i = 0; i < t.unres.size(); ++i )
    {
        std::cout << t.unres[i].size() << std::endl;
        for ( j = 0; j < t.unres[i].size(); ++j )
        {
            std::cout << (String)t.unres[i][j] << std::endl;
	}
    }
    std::cout << "hard_context- " << (String)t.hard_context->id[0] << std::endl;
    std::cout << "soft_context- " << (String)t.soft_context->id[0] << std::endl;
    std::cout << "===" << std::endl;
}
#endif

PNLHIGH_API TokArr::TokArr()
{}

PNLHIGH_API TokArr::TokArr( Tok const arr[], int len )
{
    int i;

    for ( i = 0; i < len; ++i )
    {
        push_back( arr[i] );
    }
}

PNLHIGH_API TokArr::TokArr( float const arr[], int len )
{
    int i;

    for ( i = 0; i < len; ++i )
    {
        push_back( arr[i] );
    }
}

PNLHIGH_API TokArr::TokArr( int const arr[], int len )
{
    int i;

    for ( i = 0; i < len; ++i )
    {
        push_back( arr[i] );
    }
}

PNLHIGH_API TokArr::TokArr( String const arr[], int len )
{
    int i;

    for ( i = 0; i < len; ++i )
    {
        push_back( arr[i] );
    }
}

PNLHIGH_API TokArr::TokArr( char const * const arr[], int len )
{
    int i;

    for ( i = 0; i < len; ++i )
    {
        push_back( arr[i] );
    }
}

PNLHIGH_API TokArr::TokArr( char const *s )
{
    Init( s );
}

PNLHIGH_API TokArr::TokArr( String const &s )
{
    Init( s.c_str() );
}

PNLHIGH_API TokArr::TokArr( Tok const &tok )
{
    push_back( tok );
}

inline TokArr &operator<<( TokArr &a, TokArr const &b )
{
    int i;

    for ( i = 0; i < b.size(); ++i )
    {
        a.push_back( b[i] );
    }
    return a;
}

inline TokArr &operator>>( TokArr const &a, TokArr &b )
{
    int i;

    for ( i = a.size(); i--; )
    {
        b.push_front( a[i] );
    }
    return b;
}

PNLHIGH_API TokArr operator^(TokArr const &a, TokArr const &b);

inline TokArr operator&=( TokArr &a, TokArr const &b )
{
    return a << b;
}

inline TokArr operator&( TokArr const &a, TokArr const &b )
{
    TokArr rv = a;
    return rv &= b;
}

PNLHIGH_API Tok const &TokArr::operator[]( int i ) const
{
    return Base::operator[]( i );
}

PNLHIGH_API Tok &TokArr::operator[]( int i )
{
    return Base::operator[]( i );
}

PNLHIGH_API TokArr TokArr::operator[]( Tok const &tok ) const
{
    int i;
    TokArr rv;

    for ( i = 0; i < size(); ++i )
    {
        if ( (*this)[i].Match( tok ) )
        {
            rv.push_back( (*this)[i] );
        }
    }
    return rv;
}

PNLHIGH_API Tok::Tok( TokArr const &arr )
{
    *this = arr[0];
}

PNLHIGH_API TokArr TokArr::Span( int first, int last )
{
    TokArr rv;

    for ( ; first <= last; ++first )
    {
        rv &= Tok( first );
    }
    return rv;
}

PNLHIGH_API void TokArr::Resolve( int hint_num_nodes,
                                  Matcher const &hard_matcher,
                                  Matcher const &soft_matcher,
                                  TokIdNode const *hard_context,
                                  TokIdNode const *soft_context,
                                  TokIdNode const *subroot )
{
    for ( int i = size(); i--; )
    {
        (*this)[i].Resolve( hint_num_nodes, hard_matcher, soft_matcher, hard_context, soft_context, subroot );
    }
}

PNLHIGH_API void TokArr::Resolve( int hint_num_nodes,
                                  Matcher const &hard_matcher,
                                  TokIdNode const *hard_context,
                                  TokIdNode const *soft_context,
                                  TokIdNode const *subroot )
{
    for ( int i = size(); i--; )
    {
        (*this)[i].Resolve( hint_num_nodes, hard_matcher, hard_context, soft_context, subroot );
    }
}

PNLHIGH_API void TokArr::Resolve( int hint_num_nodes,
                                  TokIdNode const *hard_context,
                                  TokIdNode const *soft_context,
                                  TokIdNode const *subroot )
{
    for ( int i = size(); i--; )
    {
        (*this)[i].Resolve( hint_num_nodes, hard_context, soft_context, subroot );
    }
}

PNLHIGH_API void TokArr::Resolve( Matcher const &hard_matcher,
                                  Matcher const &soft_matcher,
                                  TokIdNode const *hard_context,
                                  TokIdNode const *soft_context,
                                  TokIdNode const *subroot )
{
    for ( int i = size(); i--; )
    {
        (*this)[i].Resolve( hard_matcher, soft_matcher, hard_context, soft_context, subroot );
    }
}

PNLHIGH_API void TokArr::Resolve( Matcher const &hard_matcher,
                                  TokIdNode const *hard_context,
                                  TokIdNode const *soft_context,
                                  TokIdNode const *subroot )
{
    for ( int i = size(); i--; )
    {
        (*this)[i].Resolve( hard_matcher, hard_context, soft_context, subroot );
    }
}

PNLHIGH_API void TokArr::Resolve( TokIdNode const *hard_context,
                                  TokIdNode const *soft_context,
                                  TokIdNode const *subroot )
{
    for ( int i = size(); i--; )
    {
        (*this)[i].Resolve( hard_context, soft_context, subroot );
    }
}

PNLHIGH_API void TokArr::Resolve( int hint_num_nodes,
                                  Matcher const &hard_matcher,
                                  Matcher const &soft_matcher,
                                  Tok const &hard_context,
                                  Tok const &soft_context,
                                  Tok const &subroot )
{
    for ( int i = size(); i--; )
    {
        (*this)[i].Resolve( hint_num_nodes, hard_matcher, soft_matcher, hard_context, soft_context, subroot );
    }
}

PNLHIGH_API void TokArr::Resolve( int hint_num_nodes,
                                  Matcher const &hard_matcher,
                                  Tok const &hard_context,
                                  Tok const &soft_context,
                                  Tok const &subroot )
{
    for ( int i = size(); i--; )
    {
        (*this)[i].Resolve( hint_num_nodes, hard_matcher, hard_context, soft_context, subroot );
    }
}

PNLHIGH_API void TokArr::Resolve( int hint_num_nodes,
                                  Tok const &hard_context,
                                  Tok const &soft_context,
                                  Tok const &subroot )
{
    for ( int i = size(); i--; )
    {
        (*this)[i].Resolve( hint_num_nodes, hard_context, soft_context, subroot );
    }
}

PNLHIGH_API void TokArr::Resolve( Matcher const &hard_matcher,
                                  Matcher const &soft_matcher,
                                  Tok const &hard_context,
                                  Tok const &soft_context,
                                  Tok const &subroot )
{
    for ( int i = size(); i--; )
    {
        (*this)[i].Resolve( hard_matcher, soft_matcher, hard_context, soft_context, subroot );
    }
}

PNLHIGH_API void TokArr::Resolve( Matcher const &hard_matcher,
                                  Tok const &hard_context,
                                  Tok const &soft_context,
                                  Tok const &subroot )
{
    for ( int i = size(); i--; )
    {
        (*this)[i].Resolve( hard_matcher, hard_context, soft_context, subroot );
    }
}

PNLHIGH_API void TokArr::Resolve( Tok const &hard_context,
                                  Tok const &soft_context,
                                  Tok const &subroot )
{
    for ( int i = size(); i--; )
    {
        (*this)[i].Resolve( hard_context, soft_context, subroot );
    }
}

#endif

