#pragma warning(push, 2)
#include <queue>
#pragma warning(pop)

#include "pnlTok.hpp"

TokIdNode *TokIdNode::root = new( TokIdNode )( "" );
Vector< TokIdNode * > TokIdNode::cemetery;
Tok Tok::root( TokIdNode::root );
Tok Tok::nil = Tok();

bool TokIdNode::Match( std::deque< TokId >::const_iterator arr, int len, TokIdNode const *subroot ) const
{
    TokIdNode const *p = v_prev;

    if ( len == 0 )
    {
        return true;
    }
    if ( !Match( arr[len - 1] ) )
    {
        return false;
    }
    if ( this == subroot || p == 0 )
    {
        return len == 1;
    }
    return p->SubMatch( arr, len - 1, subroot );
}

bool TokIdNode::SubMatch( std::deque< TokId >::const_iterator arr, int len, TokIdNode const *subroot ) const
{
    int i;
    TokIdNode const *nd;

    if ( len == 0 )
    {
        return true;
    }
    for ( i = len - 1, nd = this; nd != subroot && nd != 0; nd = nd->v_prev )
    {
        if ( nd->Match( arr[i] ) )
        {
            if ( --i < 0 )
            {
                return true;
            }
        }
    }
    return false;
}

void TokIdNode::Alias( TokId const &id )
{
    TokIdNode *nd;

    for ( nd = h_prev; nd; nd = nd->h_prev )
    {
        if ( nd->Match( id ) )
        {
            PNL_THROW( CBadArg, "attempt to make ambiguous alias, brothers cannot have matching aliases" );
        }
    }
    for ( nd = h_next; nd; nd = nd->h_next )
    {
        if ( nd->Match( id ) )
        {
            PNL_THROW( CBadArg, "attempt to make ambiguous alias, brothers cannot have matching aliases" );
        }
    }
    this->id.push_back( id );
    for ( nd = v_prev; nd; nd = nd->v_prev )
    {
        nd->desc.insert( std::make_pair( id, this ) );
    }
}

void TokIdNode::Unalias( TokId const &id )
{
    TokIdNode *nd;
    Map::iterator it;
    int i;

    for ( i = this->id.size(); i--; )
    {
        if ( this->id[i].Match( id ) )
        {
            this->id.erase( this->id.begin() + i );
            for ( nd = v_prev; nd; nd = nd->v_prev )
            {
                it = nd->desc.find( id );
                for ( ; it != nd->desc.end() && it->first.Match( id ); )
                {
                    if ( it->second == this )
                    {
                        nd->desc.erase( it );
                        goto going_on;
                    }
                }
                PNL_THROW( CInternalError, "invariant failed: id must be in descendants multimap, but it is not" );
going_on:
                ;
            }
            break;
        }
    }
}

void TokIdNode::Remove( bool no_cemetery )
{
    int i, j;
    TokIdNode *nd[2];
    Map::iterator it;

    j = cemetery.size();
    for ( nd[0] = TokIdNode::GetLeaf( this ); nd[0]; nd[0] = TokIdNode::GetNext( nd[0], this ) )
    {
        for ( nd[1] = v_prev; nd[1]; nd[1] = nd[1]->v_prev )
        {
            for ( i = nd[0]->id.size(); i--; )
            {
                it = nd[1]->desc.find( nd[0]->id[i] );
                if ( it == nd[1]->desc.end() )
                {
                oops:
                    PNL_THROW( CInternalError, "invariant failed: id must be in descendants multimap, but it is not" );
                }
                for ( ;; )
                {
                    if ( it->second == nd[0] )
                    {
                        nd[1]->desc.erase( it );
                        break;
                    }
                    if ( ++it == nd[1]->desc.end() || !it->first.Match( nd[0]->id[i] ) )
                    {
                        goto oops;
                    }
                }
            }
        }
        nd[0]->level = -1;
        cemetery.push_back( nd[0] );
    }
    if ( h_prev )
    {
        h_prev->h_next = h_next;
    }
    else if ( v_prev )
    {
        if ( v_prev->v_next != this )
        {
            PNL_THROW( CInternalError, "incorrect links in tree nodes" );
        }
        v_prev->v_next = h_next;
    }
    for ( i = j; i < cemetery.size(); ++i )
    {
        delete( cemetery[i] );
    }
    cemetery.resize( j );
}

void TokIdNode::PurgeCemetery()
{
    int i;

    for ( i = cemetery.size(); i--; )
    {
        delete( cemetery[i] );
    }
    cemetery.clear();
}

TokIdNode *TokIdNode::Add( TokId const &id )
{
    TokIdNode *rv = new( TokIdNode );
    rv->v_prev = this;
    rv->v_next = 0;
    rv->h_prev = 0;
    rv->h_next = v_next;
    v_next = rv;
    if ( rv->h_next )
    {
        rv->h_next->h_prev = rv;
    }
    rv->Alias( id );

    return rv;
}

std::vector< std::pair< TokIdNode *, int > > TokIdNode::AmbigResolve( TokIdNode const *node,
                                                                      std::deque< TokId >::const_iterator arr,
                                                                      int len,
                                                                      Matcher const &matcher,
                                                                      TokIdNode const *context,
                                                                      TokIdNode const *subroot )
{
    int i;
    Map::const_iterator it;
    std::vector< std::pair< TokIdNode *, int > > rv[2];
    TokIdNode const *nd;

    if ( len == 0 )
    {
        if ( node && node->IsInContext( context ) && matcher.Match( node ) )
        {
            rv[0].push_back( std::make_pair( (TokIdNode *)node, 0 ) );
        }
        return rv[0];
    }

    if ( node && !context->IsInContext( node ) && !node->IsInContext( context ) )
    {
        return rv[0];
    }

    if ( !context->IsInContext( subroot ) )
    {
        PNL_THROW( CBadArg, "context must lie in subtree rooted at subroot" );
    }

    if ( node == 0 && subroot->Match( arr[0] ) )
    {
        rv[1] = AmbigResolve( subroot, arr + 1, len - 1, context, subroot );
        for ( i = rv[1].size(); i--; )
        {
            rv[0].push_back( std::make_pair( rv[1][i].first, rv[1][i].second + 1 ) );
        }
    }

    for ( nd = node ? node : subroot, it = nd->desc.find( arr[0] );
          it != nd->desc.end() && it->second->Match( arr[0] );
          ++it )
    {
        rv[1] = AmbigResolve( it->second, arr + 1, len - 1, context, subroot );
        for ( i = rv[1].size(); i--; )
        {
            rv[0].push_back( std::make_pair( rv[1][i].first, rv[1][i].second + 1 ) );
        }
    }

    if ( rv[0].empty() )
    {
        rv[0].push_back( std::make_pair( (TokIdNode *)node, 0 ) );
    }
    return rv[0];
}

std::pair< TokIdNode *, int > TokIdNode::Resolve( TokIdNode const *node,
                                                  std::deque< TokId >::const_iterator arr,
                                                  int len,
                                                  Matcher const &hard_matcher,
                                                  Matcher const &soft_matcher,
                                                  TokIdNode const *hard_context,
                                                  TokIdNode const *soft_context,
                                                  TokIdNode const *subroot )
{
    std::vector< std::pair< TokIdNode *, int > > rv;

    if ( !hard_context->IsInContext( subroot ) )
    {
        PNL_THROW( CBadArg, "hard_context must lie in subtree rooted at subroot" );
    }

    if ( !soft_context->IsInContext( hard_context ) )
    {
        soft_context = hard_context;
    }

    rv = AmbigResolve( node, arr, len, soft_matcher, soft_context, subroot );

    if ( rv.empty() )
    {
        if ( soft_context != hard_context )
        {
            return Resolve( node, arr, len, hard_matcher, hard_context, soft_context->v_prev, subroot );
        }
        return Resolve( node, arr, len, soft_matcher, hard_context, soft_context, subroot );
    }

    if ( rv.size() > 1 )
    {
        return std::make_pair( (TokIdNode *)node, 0 );
    }
    return rv[0];
}

std::pair< TokIdNode *, int > TokIdNode::Resolve( TokIdNode const *node,
                                                  std::deque< TokId >::const_iterator arr,
                                                  int len,
                                                  Matcher const &hard_matcher,
                                                  TokIdNode const *hard_context,
                                                  TokIdNode const *soft_context,
                                                  TokIdNode const *subroot )
{
    std::vector< std::pair< TokIdNode *, int > > rv;

    if ( !hard_context->IsInContext( subroot ) )
    {
        PNL_THROW( CBadArg, "hard_context must lie in subtree rooted at subroot" );
    }

    if ( !soft_context->IsInContext( hard_context ) )
    {
        soft_context = hard_context;
    }

    rv = AmbigResolve( node, arr, len, hard_matcher, soft_context, subroot );

    if ( rv.empty() )
    {
        if ( soft_context != hard_context )
        {
            return Resolve( node, arr, len, hard_matcher, hard_context, soft_context->v_prev, subroot );
        }
        return std::make_pair( (TokIdNode *)node, 0 );
    }

    if ( rv.size() > 1 )
    {
        return std::make_pair( (TokIdNode *)node, 0 );
    }
    return rv[0];
}

bool TokIdNode::Match( TokIdNode const *node1, std::deque< TokId >::const_iterator unres1, int len1,
                       TokIdNode const *node2, std::deque< TokId >::const_iterator unres2, int len2 )
{
    int i;

    if ( len1 > len2 )
    {
        return Match( node2, unres2, len2, node1, unres1, len1 );
    }

    for ( i = 1; i <= len1; ++i )
    {
        if ( !unres1[len1 - i].Match( unres2[len2 - i] ) )
        {
            return false;
        }
    }

    if ( node1 == 0 )
    {
        return true;
    }

    if ( node2 )
    {
        if ( len1 == len2 )
        {
            return node1->IsInContext( node2 ) || node2->IsInContext( node1 );
        }
        if ( !node1->IsInContext( node2 ) )
        {
            return false;
        }
    }

    if ( len1 == 0 )
    {
        return node1->Match( unres2, len2, node2 );
    }
    return node1->SubMatch( unres2, len2 - len1, node2 );
}

char const *Tok::Init( char const s[] )
{
    char const *t;
    char buf[128];
    int num_digits, num_es, num_dots, num_alphas, num_signs;
    int i;
    float fl;

    node.clear();
    fload.clear();
    unres.clear();
    unres.resize( 1 );

    while ( isspace( *s ) )
    {
        ++s;
    }
    if ( *s == 0 )
    {
        unres[0].push_back( "" );
        return s;
    }

    for ( ;; )
    {
        if ( *s == '^' )
        {
            unres[0].push_back( "" );
            ++s;
            continue;
        }
        num_digits = num_es = num_dots = num_alphas = num_signs = 0;
        for ( t = s; *t != 0 && (isalnum( *t ) || strchr( "+-.", *t )); ++t )
        {
            num_digits += (bool)isdigit( *t );
            num_es += *t == 'E';
            num_es += *t == 'e';
            num_dots += *t == '.';
            num_alphas += (bool)isalpha( *t );
            num_signs += *t == '+';
            num_signs += *t == '-';
        }
        if ( t - s >= 128 )
        {
            PNL_THROW( CBadArg, "Too long identifier in token representation" );
        }
        memcpy( buf, s, t - s );
        buf[t - s] = 0;
        if ( num_digits == t - s )
        {
            // going discrete integer
            sscanf( buf, "%d", &i );
            unres[0].push_back( i );
        }
        else if ( num_digits && num_alphas <= num_es && num_dots <= 1 )
        {
            // going float
            sscanf( buf, "%f", &fl );
            fload.push_back( fl );
        }
        else if ( num_signs + num_dots == t - s )
        {
            // going undef float
            fload.push_back( F() );
        }
        else
        {
            // going discrete identifier
            unres[0].push_back( buf );
        }
        s = t + 1;
        if ( *t != '^' )
        {
            break;
        }
    }
    return t;
}

void Tok::Unresolve( TokIdNode const *subroot )
{
    int i, j;

    for ( i = node.size(); i--; )
    {
        for ( ; node[i] != subroot && node[i] != 0; node[i] = node[i]->v_prev )
        {
            unres[i + 1].push_front( node[i]->id[0] );
        }
        if ( node[i] == 0 )
        {
            for ( j = 0; j < unres[i + 1].size(); ++j )
            {
                unres[i].push_back( unres[i + 1][j] );
            }
            unres.erase( unres.begin() + i + 1 );
            node.erase( node.begin() + i );
        }
    }
}

void Tok::Unresolve( Matcher const &matcher )
{
    int i;
    TokIdNode *nd;

    for ( i = node.size(); i--; )
    {
        for ( nd = node[i]; nd->v_prev != 0 && !matcher.Match( nd ); nd = nd->v_prev )
        {
            unres[i + 1].push_front( nd->id[0] );
        }
    }
}

bool Tok::Match( Tok const &tok ) const
{
    int i, j;

    if ( unres.back().size() > tok.unres.back().size() )
    {
        return tok.Match( *this );
    }

    if ( !TokIdNode::Match( 0, unres.back().begin(), unres.back().size(),
                            0, tok.unres.back().end() - unres.back().size(), unres.back().size() ) )
    {
        return false;
    }

    if ( node.size() )
    {
        if ( unres.back().size() != tok.unres.back().size() )
        {
            for ( i = 1; i < tok.unres.back().size() - unres.back().size(); ++i )
            {
                if ( !TokIdNode::Match( node.back(), unres.back().begin(), 0,
                                        0, tok.unres.back().end() - unres.back().size() - i, i ) )
                {
                    break;
                }
            }
            if ( tok.node.size() )
            {
                Tok a = *this;
                Tok b = tok;

                a.unres.pop_back();
                a.node.pop_back();

                if ( i == tok.unres.back().size() - unres.back().size() )
                {
                    if ( TokIdNode::Match( node.back(), unres.back().begin(), 0,
                                           tok.node.back(), tok.unres.back().end() - unres.back().size() - i, i ) )
                    {
                        b.unres.pop_back();
                        b.node.pop_back();
                        return a.Match( b );
                    }
                }

                for ( j = unres.back().size() + i; j--; )
                {
                    b.unres.back().pop_back();
                }
                return a.Match( b );
            }
            return i == tok.unres.back().size() - unres.back().size()
                   && node.size() <= 1 && unres.front().empty();
        }
        if ( tok.node.size() )
        {
            if ( !node.back()->IsInContext( tok.node.back() )
                 && !tok.node.back()->IsInContext( node.back() ) )
            {
                return false;
            }
            
            Tok a = *this;
            Tok b = tok;

            a.unres.pop_back();
            a.node.pop_back();
            b.unres.pop_back();
            b.node.pop_back();

            return a.Match( b );
        }
    }

    return tok.node.size() <= 1;
}

void Tok::Resolve( int hint_num_nodes,
                   Matcher const &hard_matcher, Matcher const &soft_matcher,
                   TokIdNode const *hard_context, TokIdNode const *soft_context,
                   TokIdNode const *subroot )
{
    int i, j, k;
    TokIdNode *nd;
    std::pair< TokIdNode *, int > rr;

    if ( hard_context )
    {
        this->hard_context = (TokIdNode *)hard_context;
    }
    else
    {
        hard_context = TokIdNode::root;
    }
    if ( soft_context )
    {
        this->soft_context = (TokIdNode *)soft_context;
    }
    else
    {
        soft_context = TokIdNode::root;
    }
    if ( subroot )
    {
        this->subroot = (TokIdNode *)subroot;
    }
    else
    {
        subroot = TokIdNode::root;
    }
    if ( hint_num_nodes >= 0 )
    {
        this->hint_num_nodes = hint_num_nodes;
    }

    for ( k = 0; k < unres.size(); ++k )
    {
        if ( (i = unres[k].size()) != 0 )
        {
            if ( k )
            {
                rr = TokIdNode::Resolve( node[k - 1], unres[k].begin(), i,
                                         hard_context, soft_context, subroot );
            }
            if ( k == 0 || rr.second == 0 )
            {
                for ( j = 0; j < unres[k].size(); ++j )
                {
                    rr = TokIdNode::Resolve( 0, unres[k].begin() + j, unres[k].size() - j,
                                             hard_context, soft_context, subroot );
                    if ( rr.second == 0 )
                    {
                        continue;
                    }
                    unres[k].erase( unres[k].begin() + j, unres[k].begin() + j + rr.second );
                    node.insert( node.begin() + k, rr.first );
                    unres.insert( unres.begin() + k, std::deque< TokId >() );
                    ++k;
                }
                continue;
            }
            while ( rr.second-- )
            {
                unres[k].pop_front();
            }
            if ( k )
            {
                node[k - 1] = rr.first;
            }
            else
            {
                node.insert( node.begin(), nd );
                unres.insert( unres.begin(), std::deque< TokId >() );
                ++k;
            }
        }
    }
}

Tok::operator String() const
{
    int i;
    String rv;
    Tok t = *this;

    t.Unresolve( (TokIdNode *)0 );
    for ( i = 0; i < t.unres[0].size(); ++i )
    {
        if ( i )
        {
            rv += "^";
        }
        rv += t.unres[0][i];
    }
    if ( t.unres[0].size() && t.fload.size() )
    {
        rv += "^";
    }
    for ( i = 0; i < t.fload.size(); ++i )
    {
        if ( i )
        {
            rv += "^";
        }
	rv += (String)t.fload[i];
    }

    return rv;
}

TokArr Tok::PropagateFload( Tok const &tok )
{
    TokArr retval;
    int i, j;
    TokIdNode *nd;

    if ( tok.fload.size() == 0 )
    {
        return retval;
    }
    if ( tok.IsUnres() )
    {
        return tok;
    }
    
    for ( i = 0, j = 0; j < tok.node.size(); ++j )
    {
        nd = TokIdNode::GetLeaf( tok.node[j] );
        for ( ; i < tok.fload.size(); ++i )
        {
            retval &= Tok( nd, tok.fload[i] );
            nd = TokIdNode::GetNextLeaf( nd, tok.node[j] );
            if ( nd == 0 )
            {
                break;
            }
        }
    }
    return retval;
}

void TokArr::Init( char const *s )
{
    int num_ampersands;

    resize( 0 );
    for ( ;; )
    {
        for ( num_ampersands = 0; isspace( *s ) || *s == '&'; ++s )
        {
            num_ampersands += *s == '&';
        }
        if ( *s == 0 )
        {
            while ( num_ampersands-- )
            {
                push_back( "" );
            }
            return;
        }
        if ( size() )
        {
            while ( --num_ampersands > 0 )
            {
                push_back( "" );
            }
        }
        else
        {
            while ( num_ampersands-- )
            {
                push_back( "" );
            }
        }
        if ( !isalnum( *s ) && strchr( "^+-.", *s ) )
        {
            PNL_THROW( CBadArg, "alien symbol inside TokArr, one can use alphanumerics or + - . ^ & only" );
        }
        push_back( Tok::root );
        s = back().Init( s );
    }
}

TokArr operator^( TokArr const &a, TokArr const &b )
{
    int i, j;
    TokArr rv;

    for ( i = 0; i < a.size(); ++i )
    {
        for ( j = 0; j < b.size(); ++j )
	{
	    rv.push_back( a[i] ^ b[j] );
	}
    }

    return rv;
}

TokArr::operator String() const
{
    int i;
    String rv;

    for ( i = 0; i < size(); ++i )
    {
        if ( i )
        {
            rv += " ";
        }
        rv += (String)(*this)[i];
    }

    return rv;
}

TokArr Tok::GetDescendants() const
{
    TokIdNode *nd[2];
    TokArr rv;

    for ( nd[0] = TokIdNode::GetLeaf( nd[1] = Node() ); nd[0] != nd[1]; nd[0] = TokIdNode::GetNext( nd[0], nd[1] ) )
    {
        rv.push_back( nd[0] );
    }

    return rv;
}

TokArr Tok::GetDescendants( Matcher const &matcher ) const
{
    TokIdNode *nd[2];
    TokArr rv;

    for ( nd[0] = TokIdNode::GetLeaf( nd[1] = Node() ); nd[0] != nd[1]; nd[0] = TokIdNode::GetNext( nd[0], nd[1] ) )
    {
        if ( matcher.Match( nd[0] ) )
        {
            rv.push_back( nd[0] );
        }
    }

    return rv;
}

#if 0
void vistok( Tok const &t )
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
    std::cout << "===" << std::endl;
}

int main()
{
    TokIdNode::root->Add( "bnet" );
    Tok t = "bnet";
    vistok( t );
    t.Node()->Add( 0 );
    vistok( t );
    std::cout << std::endl << (String)t << std::endl;
    t.Resolve();
    vistok( t );
    std::cout << std::endl << (String)t << std::endl;

    t = "bnet^4.0";
    vistok( t );
    std::cout << std::endl << (String)t << std::endl;
    t.Resolve();
    vistok( t );
    std::cout << std::endl << (String)t << std::endl;

    t = "0";
    vistok( t );
    std::cout << std::endl << (String)t << std::endl;
    t.Resolve();
    vistok( t );
    std::cout << std::endl << (String)t << std::endl;

    t.Node()->Add( "node" );
    t = "node";
    t.Node()->Add( "rock" );
    t.Node()->Add( "paper" );
    t.Node()->Add( "scissors" );

    t = "bnet";
    t.Node()->Add( "1" );

    t = "1";
    vistok( t );
    std::cout << std::endl << (String)t << std::endl;
    t.Resolve();
    vistok( t );
    std::cout << std::endl << (String)t << std::endl;

    t = "0";
    vistok( t );
    std::cout << std::endl << (String)t << std::endl;
    t.Resolve();
    vistok( t );
    std::cout << std::endl << (String)t << std::endl;

    std::cout << "huhu2" << std::endl;
    t = "1";
    t.Node()->Add( "node" );
    std::cout << "huhu3" << std::endl;
    t = "node";
#if 0
    t.Resolve( Tok( "1" ).Node() );
#else
    t.Resolve( "1" );
#endif
    std::cout << "huhu3" << std::endl;
    t.Node()->Add( "rock" );
    std::cout << "huhu4" << std::endl;
    t.Node()->Add( "paper" );
    t.Node()->Add( "scissors" );

    std::cout << "huhu" << std::endl;
    std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAa" << std::endl;

    t = "rock";
    vistok( t );
    std::cout << std::endl << (String)t << std::endl;
    t.Resolve();
    vistok( t );
    std::cout << std::endl << (String)t << std::endl;
    t.Resolve( "1" );
    vistok( t );
    std::cout << std::endl << (String)t << std::endl;

    std::cout << "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZz" << std::endl;
    t = "1";
    t.Resolve;
    std::cout << std::endl << (String)t << std::endl;

    return 0;
}
#endif
