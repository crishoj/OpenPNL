#ifdef OLD_TOKENS

#pragma warning(push, 2)
#include <queue>
#pragma warning(pop)

#include "pnlTok.hpp"

PNLW_BEGIN

TokIdNode *TokIdNode::root = new(TokIdNode)("");
Vector< TokIdNode * > TokIdNode::cemetery;
Tok Tok::root(TokIdNode::root);
Tok Tok::nil = Tok();

TokId::TokId(String const &s)
{
    Init(s);
}

TokId::TokId(char const s[])
{
    Init(s);
}

void TokId::Init(String const &s)
{
    int i;

    for(i = s.size(); i--;)
    {
        if(!isdigit(s.c_str()[i]))
        {
going_str:
            is_int = false;
            id = s;
            return;
        }
    }
    if(s.size())
    {
        is_int = true;
        sscanf(s.c_str(), "%d", &int_id);
    }
    else
    {
	goto going_str;
    }
}

bool TokId::Match(TokId const &rhs) const
{
    switch(is_int + rhs.is_int)
    {
    case 0:
        return id == rhs.id;
    case 2:
        return int_id == rhs.int_id;
    }
    return false;
}

bool TokId::operator<(TokId const &rhs) const
{
    if(is_int)
    {
        if(rhs.is_int)
        {
            return int_id < rhs.int_id;
        }
        return false;
    }
    if(rhs.is_int)
    {
        return true;
    }
    return id < rhs.id;
}

TokId::operator String() const
{
    if(!is_int)
    {
        return id;
    }
    
    return String() << int_id;
}

TokIdNode::TokIdNode(TokId id): h_next(0), h_prev(0), v_next(0), v_prev(0), level(0), tag(0), data(0)
{
    Alias(id);
}

bool TokIdNode::Match(TokId const &rhs) const
{
    int i;

    for(i = id.size(); i--;)
    {
        if(id[i].Match(rhs))
        {
            return true;
        }
    }
    return false;
}

void TokIdNode::Kill()
{
    Remove(true);
}

TokIdNode *TokIdNode::GetLeaf(TokIdNode const *subroot)
{
    TokIdNode const *nd;
    
    for(nd = subroot; nd->v_next; nd = nd->v_next) ;
    return (TokIdNode *)nd;
}

TokIdNode *TokIdNode::GetNext(TokIdNode const *node, TokIdNode const *subroot)
{
    if(node->v_prev == 0 || node == subroot)
    {
        return 0;
    }
    if(node->h_next)
    {
        return GetLeaf(node->h_next);
    }
    return (TokIdNode *)node->v_prev;
}

TokIdNode *TokIdNode::GetNextLeaf(TokIdNode const *node, TokIdNode const *subroot)
{
    TokIdNode const *nd;

    for(nd = node; nd; nd = GetNext(node, subroot))
    {
        if(nd->v_next == 0)
        {
            return (TokIdNode *)nd;
        }
    }
    return 0;
}

bool TokIdNode::IsInContext(TokIdNode const *context) const
{
    TokIdNode const *nd;

    for(nd = this; nd != context && nd != 0; nd = nd->v_prev) ;
    return nd == context;
}

String TokIdNode::Name() const
{
    return id[0];
}

std::vector< std::pair< TokIdNode *, int > > TokIdNode::AmbigResolve(
    TokIdNode const *node,
    std::deque< TokId >::const_iterator arr,
    int len,
    TokIdNode const *context,
    TokIdNode const *subroot)
{
    return AmbigResolve(node, arr, len, Matcher(), context, subroot);
}

std::pair< TokIdNode *, int > TokIdNode::Resolve(
    TokIdNode const *node,
    std::deque< TokId >::const_iterator arr,
    int len,
    TokIdNode const *hard_context,
    TokIdNode const *soft_context,
    TokIdNode const *subroot)
{
    return Resolve(node, arr, len, Matcher(), Matcher(), hard_context, soft_context, subroot);
}

bool TokIdNode::Match(std::deque< TokId >::const_iterator arr, int len, TokIdNode const *subroot) const
{
    TokIdNode const *p = v_prev;

    if(len == 0)
    {
        return true;
    }
    if(!Match(arr[len - 1]))
    {
        return false;
    }
    if(this == subroot || p == 0)
    {
        return len == 1;
    }
    return p->SubMatch(arr, len - 1, subroot);
}

bool TokIdNode::SubMatch(std::deque< TokId >::const_iterator arr, int len, TokIdNode const *subroot) const
{
    int i;
    TokIdNode const *nd;

    if(len == 0)
    {
        return true;
    }
    for(i = len - 1, nd = this; nd != subroot && nd != 0; nd = nd->v_prev)
    {
        if(nd->Match(arr[i]))
        {
            if(--i < 0)
            {
                return true;
            }
        }
    }
    return false;
}

void TokIdNode::Alias(TokId const &id)
{
    TokIdNode *nd;

    for(nd = h_prev; nd; nd = nd->h_prev)
    {
        if(nd->Match(id))
        {
	    PNL_THROW(pnl::CBadArg, "attempt to make ambiguous alias, brothers cannot have matching aliases");
        }
    }
    for(nd = h_next; nd; nd = nd->h_next)
    {
        if(nd->Match(id))
        {
            PNL_THROW(pnl::CBadArg, "attempt to make ambiguous alias, brothers cannot have matching aliases");
        }
    }
    this->id.push_back(id);
    for(nd = v_prev; nd; nd = nd->v_prev)
    {
        nd->desc.insert(std::make_pair(id, this));
    }
}

void TokIdNode::Unalias(TokId const &id)
{
    TokIdNode *nd;
    Map::iterator it;
    int i;

    for(i = this->id.size(); i--;)
    {
        if(this->id[i].Match(id))
        {
            this->id.erase(this->id.begin() + i);
            for(nd = v_prev; nd; nd = nd->v_prev)
            {
                it = nd->desc.find(id);
                for(; it != nd->desc.end() && it->first.Match(id); ++it)
                {
                    if(it->second == this)
                    {
                        nd->desc.erase(it);
                        goto going_on;
                    }
                }
                PNL_THROW(pnl::CInternalError, "invariant failed: id must be in descendants multimap, but it is not");
going_on:
                ;
            }
            break;
        }
    }
}

void TokIdNode::Remove(bool no_cemetery)
{
    int i, j;
    TokIdNode *nd[2];
    Map::iterator it, itEnd;

    j = cemetery.size();
    for(nd[0] = TokIdNode::GetLeaf(this); nd[0]; nd[0] = TokIdNode::GetNext(nd[0], this))
    {
        for(nd[1] = v_prev; nd[1]; nd[1] = nd[1]->v_prev)
        {
            for(i = nd[0]->id.size(); i--;)
            {
                it = nd[1]->desc.find(nd[0]->id[i]);
                if(it == nd[1]->desc.end())
                {
                oops:
                    PNL_THROW(pnl::CInternalError, "invariant failed: id must be in descendants multimap, but it is not");
                }
		for(itEnd = nd[1]->desc.upper_bound(nd[0]->id[i]);;)
                {
                    if(it->second == nd[0])
                    {
                        nd[1]->desc.erase(it);
                        break;
                    }
                    if((++it == itEnd) || !it->first.Match(nd[0]->id[i]))
                    {
                        goto oops;
                    }
                }
            }
        }
        nd[0]->level = -1;
        cemetery.push_back(nd[0]);
    }
    if(h_prev)
    {
        h_prev->h_next = h_next;
    }
    else if(v_prev)
    {
        if(v_prev->v_next != this)
        {
            PNL_THROW(pnl::CInternalError, "incorrect links in tree nodes");
        }
        v_prev->v_next = h_next;
    }
    if (h_next)
    {
	h_next->h_prev = h_prev;
    }
    for(i = j; i < cemetery.size(); ++i)
    {
        delete(cemetery[i]);
    }
    cemetery.resize(j);
}

void TokIdNode::PurgeCemetery()
{
    int i;

    for(i = cemetery.size(); i--;)
    {
        delete(cemetery[i]);
    }
    cemetery.clear();
}

TokIdNode *TokIdNode::Add(TokId const &id)
{
    TokIdNode *rv = new(TokIdNode);
    rv->v_prev = this;
    rv->v_next = 0;
    rv->h_prev = 0;
    rv->h_next = v_next;
    v_next = rv;
    if(rv->h_next)
    {
        rv->h_next->h_prev = rv;
    }
    rv->Alias(id);

    return rv;
}

std::vector< std::pair< TokIdNode *, int > > TokIdNode::AmbigResolve(
    TokIdNode const *node,
    std::deque< TokId >::const_iterator arr,
    int len,
    Matcher const &matcher,
    TokIdNode const *context,
    TokIdNode const *subroot)
{
    int i;
    Map::const_iterator it, itEnd;
    std::vector< std::pair< TokIdNode *, int > > rv[2];
    TokIdNode const *nd;

    if(len == 0)
    {
        if(node && node->IsInContext(context) && matcher.Match(node))
        {
            rv[0].push_back(std::make_pair((TokIdNode *)node, 0));
        }
        return rv[0];
    }

    if(node && !context->IsInContext(node) && !node->IsInContext(context))
    {
        return rv[0];
    }

    if(!context->IsInContext(subroot))
    {
        PNL_THROW(pnl::CBadArg, "context must lie in subtree rooted at subroot");
    }

    if(node == 0 && subroot->Match(arr[0]))
    {
        rv[1] = AmbigResolve(subroot, arr + 1, len - 1, matcher, context, subroot);
        for(i = rv[1].size(); i--;)
        {
            rv[0].push_back(std::make_pair(rv[1][i].first, rv[1][i].second + 1));
        }
    }

    nd = node ? node : subroot;
    it = nd->desc.find(arr[0]);
    if(it != nd->desc.end())
    {
	for(itEnd = nd->desc.upper_bound(arr[0]); it != itEnd; ++it)
	{
	    rv[1] = AmbigResolve(it->second, arr + 1, len - 1, matcher, context, subroot);
	    for(i = rv[1].size(); i--;)
	    {
		rv[0].push_back(std::make_pair(rv[1][i].first, rv[1][i].second + 1));
	    }
	}
    }

    if(rv[0].empty())
    {
        rv[0].push_back(std::make_pair((TokIdNode *)node, 0));
    }
    return rv[0];
}

std::pair< TokIdNode *, int > TokIdNode::Resolve(
    TokIdNode const *node,
    std::deque< TokId >::const_iterator arr,
    int len,
    Matcher const &hard_matcher,
    Matcher const &soft_matcher,
    TokIdNode const *hard_context,
    TokIdNode const *soft_context,
    TokIdNode const *subroot)
{
    std::vector< std::pair< TokIdNode *, int > > rv;

    if(!hard_context->IsInContext(subroot))
    {
        PNL_THROW(pnl::CBadArg, "hard_context must lie in subtree rooted at subroot");
    }

    if(!soft_context->IsInContext(hard_context))
    {
        soft_context = hard_context;
    }

    rv = AmbigResolve(node, arr, len, hard_matcher, soft_context, subroot);

    if(rv.empty())
    {
        if(soft_context != hard_context)
        {
            return Resolve(node, arr, len, hard_matcher, hard_context, soft_context->v_prev, subroot);
        }
        return Resolve(node, arr, len, soft_matcher, hard_context, soft_context, subroot);
    }

    if(rv.size() > 1)
    {
        return std::make_pair((TokIdNode *)node, 0);
    }
    return rv[0];
}

std::pair< TokIdNode *, int > TokIdNode::Resolve(
    TokIdNode const *node,
    std::deque< TokId >::const_iterator arr,
    int len,
    Matcher const &hard_matcher,
    TokIdNode const *hard_context,
    TokIdNode const *soft_context,
    TokIdNode const *subroot)
{
    std::vector< std::pair< TokIdNode *, int > > rv;

    if(!hard_context->IsInContext(subroot))
    {
        PNL_THROW(pnl::CBadArg, "hard_context must lie in subtree rooted at subroot");
    }

    if(!soft_context->IsInContext(hard_context))
    {
        soft_context = hard_context;
    }

    rv = AmbigResolve(node, arr, len, hard_matcher, soft_context, subroot);

    if(rv.empty())
    {
        if(soft_context != hard_context)
        {
            return Resolve(node, arr, len, hard_matcher, hard_context, soft_context->v_prev, subroot);
        }
        return std::make_pair((TokIdNode *)node, 0);
    }

    if(rv.size() > 1)
    {
        return std::make_pair((TokIdNode *)node, 0);
    }
    return rv[0];
}

bool TokIdNode::Match(TokIdNode const *node1, std::deque< TokId >::const_iterator unres1, int len1,
                      TokIdNode const *node2, std::deque< TokId >::const_iterator unres2, int len2)
{
    int i;

    if(len1 > len2)
    {
        return Match(node2, unres2, len2, node1, unres1, len1);
    }

    for(i = 1; i <= len1; ++i)
    {
        if(!unres1[len1 - i].Match(unres2[len2 - i]))
        {
            return false;
        }
    }

    if(node1 == 0)
    {
        return true;
    }

    if(node2)
    {
        if(len1 == len2)
        {
            return node1->IsInContext(node2) || node2->IsInContext(node1);
        }
        if(!node1->IsInContext(node2))
        {
            return false;
        }
    }

    if(len1 == 0)
    {
        return node1->Match(unres2, len2, node2);
    }
    return node1->SubMatch(unres2, len2 - len1, node2);
}

char const *Tok::Init(char const s[])
{
    char const *t;
    char buf[128];
    int num_digits, num_es, num_dots, num_alphas, num_signs;
    int i;
    float fl;

    node.clear();
    fload.clear();
    unres.clear();
    unres.resize(1);

    while(isspace(*s))
    {
        ++s;
    }
    if(*s == 0)
    {
        unres[0].push_back("");
        return s;
    }

    for(;;)
    {
        if(*s == '^')
        {
            unres[0].push_back("");
            ++s;
            continue;
        }
        num_digits = num_es = num_dots = num_alphas = num_signs = 0;
        for(t = s; *t != 0 && (isalnum(*t) || strchr("+-.", *t)); ++t)
        {
            num_digits += (bool)isdigit(*t);
            num_es += *t == 'E';
            num_es += *t == 'e';
            num_dots += *t == '.';
            num_alphas += (bool)isalpha(*t);
            num_signs += *t == '+';
            num_signs += *t == '-';
        }
        if(t - s >= 128)
        {
            PNL_THROW(pnl::CBadArg, "Too long identifier in token representation");
        }
        memcpy(buf, s, t - s);
        buf[t - s] = 0;
        if(num_digits == t - s)
        {
            // going discrete integer
            sscanf(buf, "%d", &i);
            unres[0].push_back(i);
        }
        else if(num_digits && num_alphas <= num_es && num_dots <= 1)
        {
            // going float
            sscanf(buf, "%f", &fl);
            fload.push_back(fl);
        }
        else if(num_signs + num_dots == t - s)
        {
            // going undef float
            fload.push_back(F());
        }
        else
        {
            // going discrete identifier
            unres[0].push_back(buf);
        }
        s = t + 1;
        if(*t != '^')
        {
            break;
        }
    }
    return t;
}

void Tok::Unresolve(TokIdNode const *subroot)
{
    int i, j;

    for(i = node.size(); i--;)
    {
        for(; node[i] != subroot && node[i] != 0; node[i] = node[i]->v_prev)
        {
            unres[i + 1].push_front(node[i]->id[0]);
        }
        if(node[i] == 0)
        {
            for(j = 0; j < unres[i + 1].size(); ++j)
            {
                unres[i].push_back(unres[i + 1][j]);
            }
            unres.erase(unres.begin() + i + 1);
            node.erase(node.begin() + i);
        }
    }
}

void Tok::Unresolve(Matcher const &matcher)
{
    int i;
    TokIdNode *nd;

    for(i = node.size(); i--;)
    {
        for(nd = node[i]; nd->v_prev != 0 && !matcher.Match(nd); nd = nd->v_prev)
        {
            unres[i + 1].push_front(nd->id[0]);
        }
    }
}

bool Tok::Match(Tok const &tok) const
{
    int i, j;

    if(unres.back().size() > tok.unres.back().size())
    {
        return tok.Match(*this);
    }

    if(!TokIdNode::Match(0, unres.back().begin(), unres.back().size(),
                            0, tok.unres.back().end() - unres.back().size(), unres.back().size()))
    {
        return false;
    }

    if(node.size())
    {
        if(unres.back().size() != tok.unres.back().size())
        {
            for(i = 1; i < tok.unres.back().size() - unres.back().size(); ++i)
            {
                if(!TokIdNode::Match(node.back(), unres.back().begin(), 0,
                                        0, tok.unres.back().end() - unres.back().size() - i, i))
                {
                    break;
                }
            }
            if(tok.node.size())
            {
                Tok a = *this;
                Tok b = tok;

                a.unres.pop_back();
                a.node.pop_back();

                if(i == tok.unres.back().size() - unres.back().size())
                {
                    if(TokIdNode::Match(node.back(), unres.back().begin(), 0,
                                           tok.node.back(), tok.unres.back().end() - unres.back().size() - i, i))
                    {
                        b.unres.pop_back();
                        b.node.pop_back();
                        return a.Match(b);
                    }
                }

                for(j = unres.back().size() + i; j--;)
                {
                    b.unres.back().pop_back();
                }
                return a.Match(b);
            }
            return i == tok.unres.back().size() - unres.back().size()
                   && node.size() <= 1 && unres.front().empty();
        }
        if(tok.node.size())
        {
            if(!node.back()->IsInContext(tok.node.back())
                 && !tok.node.back()->IsInContext(node.back()))
            {
                return false;
            }
            
            Tok a = *this;
            Tok b = tok;

            a.unres.pop_back();
            a.node.pop_back();
            b.unres.pop_back();
            b.node.pop_back();

            return a.Match(b);
        }
    }

    return tok.node.size() <= 1;
}

void Tok::Resolve(int hint_num_nodes,
                  Matcher const &hard_matcher, Matcher const &soft_matcher,
                  TokIdNode const *hard_context, TokIdNode const *soft_context,
                  TokIdNode const *subroot)
{
    int i, j, k;
    TokIdNode *nd;
    std::pair< TokIdNode *, int > rr;

    if(hard_context)
    {
        this->hard_context = (TokIdNode *)hard_context;
    }
    else
    {
        hard_context = TokIdNode::root;
    }
    if(soft_context)
    {
        this->soft_context = (TokIdNode *)soft_context;
    }
    else
    {
        soft_context = TokIdNode::root;
    }
    if(subroot)
    {
        this->subroot = (TokIdNode *)subroot;
    }
    else
    {
        subroot = TokIdNode::root;
    }
    if(hint_num_nodes >= 0)
    {
        this->hint_num_nodes = hint_num_nodes;
    }

    for(k = 0; k < unres.size(); ++k)
    {
        if((i = unres[k].size()) != 0)
        {
            if(k)
            {
                rr = TokIdNode::Resolve(node[k - 1], unres[k].begin(), i,
					hard_matcher, soft_matcher,
                                        hard_context, soft_context, subroot);
            }
            if(k == 0 || rr.second == 0)
            {
		int lastIns = -1;
                for(j = 0; j < unres[k].size(); ++j)
                {
                    rr = TokIdNode::Resolve(0, unres[k].begin() + j, unres[k].size() - j,
                                            hard_matcher, soft_matcher,
					    hard_context, soft_context, subroot);
                    if(rr.second == 0)
                    {
                        continue;
                    }
                    unres[k].erase(unres[k].begin() + j, unres[k].begin() + rr.second + j);
                    node.insert(node.begin() + k, rr.first);
		    lastIns = j;
                    unres[k].insert(unres[k].begin() + j, TokId(""));
                }
		if(lastIns >= 0)
		{
		    unres[k].erase(unres[k].begin() + lastIns);
		}
                continue;
            }
            while(rr.second--)
            {
                unres[k].pop_front();
            }
            if(k)
            {
                node[k - 1] = rr.first;
            }
            else
            {
                node.insert(node.begin(), nd);
                unres.insert(unres.begin(), std::deque< TokId >());
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

    t.Unresolve((TokIdNode *)0);
    for(i = 0; i < t.unres[0].size(); ++i)
    {
        if(i)
        {
            rv += "^";
        }
        rv += t.unres[0][i];
    }
    if(t.unres[0].size() && t.fload.size())
    {
        rv += "^";
    }
    for(i = 0; i < t.fload.size(); ++i)
    {
        if(i)
        {
            rv += "^";
        }
	rv += (String)t.fload[i];
    }

    return rv;
}

TokArr Tok::PropagateFload(Tok const &tok)
{
    TokArr retval;
    int i, j;
    TokIdNode *nd;

    if(tok.fload.size() == 0)
    {
        return retval;
    }
    if(tok.IsUnres())
    {
        return tok;
    }
    
    for(i = 0, j = 0; j < tok.node.size(); ++j)
    {
        nd = TokIdNode::GetLeaf(tok.node[j]);
        for(; i < tok.fload.size(); ++i)
        {
            retval &= Tok(nd, tok.fload[i]);
            nd = TokIdNode::GetNextLeaf(nd, tok.node[j]);
            if(nd == 0)
            {
                break;
            }
        }
    }
    return retval;
}

void TokArr::Init(char const *s)
{
    int num_ampersands;

    resize(0);
    for(;;)
    {
        for(num_ampersands = 0; isspace(*s) || *s == '&'; ++s)
        {
            num_ampersands += *s == '&';
        }
        if(*s == 0)
        {
            while(num_ampersands--)
            {
                push_back("");
            }
            return;
        }
        if(size())
        {
            while(--num_ampersands > 0)
            {
                push_back("");
            }
        }
        else
        {
            while(num_ampersands--)
            {
                push_back("");
            }
        }
        if(!(isalnum(*s) || strchr("-^+.", *s)))
        {
            PNL_THROW(pnl::CBadArg, "alien symbol inside TokArr, one can use alphanumerics or + - . ^ & only");
        }
        push_back(Tok::root);
        s = back().Init(s);
    }
}

TokArr operator^(TokArr const &a, TokArr const &b)
{
    int i, j;
    TokArr rv;

    for(i = 0; i < a.size(); ++i)
    {
        for(j = 0; j < b.size(); ++j)
	{
	    rv.push_back(a[i] ^ b[j]);
	}
    }

    return rv;
}

TokArr::operator String() const
{
    int i;
    String rv;

    for(i = 0; i < size(); ++i)
    {
        if(i)
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

    for(nd[0] = TokIdNode::GetLeaf(nd[1] = Node()); nd[0] != nd[1]; nd[0] = TokIdNode::GetNext(nd[0], nd[1]))
    {
        rv.push_back(nd[0]);
    }

    return rv;
}

TokArr Tok::GetDescendants(Matcher const &matcher) const
{
    TokIdNode *nd[2];
    TokArr rv;

    for(nd[0] = TokIdNode::GetLeaf(nd[1] = Node()); nd[0] != nd[1]; nd[0] = TokIdNode::GetNext(nd[0], nd[1]))
    {
        if(matcher.Match(nd[0]))
        {
            rv.push_back(nd[0]);
        }
    }

    return rv;
}

Tok::Tok(char const s[])
    : hard_matcher(),
      soft_matcher(),
      hard_context(TokIdNode::root),
      soft_context(TokIdNode::root),
      subroot(TokIdNode::root),
      hint_num_nodes(-1)
{
    Init(s);
}

Tok::Tok(String const &s)
    : hard_matcher(),
      soft_matcher(),
      hard_context(TokIdNode::root),
      soft_context(TokIdNode::root),
      subroot(TokIdNode::root),
      hint_num_nodes(-1)
{
    Init(s.c_str());
}

Tok::Tok(std::vector< float > const &fload)
    : hard_matcher(),
      soft_matcher(),
      hard_context(TokIdNode::root),
      soft_context(TokIdNode::root),
      subroot(TokIdNode::root),
      hint_num_nodes(-1)
{
    int i;

    unres.push_back(std::deque< TokId >());
    this->fload.resize(fload.size());
    for(i = fload.size(); i--;)
    {
        this->fload[i] = fload[i];
    }
}

Tok::Tok(std::vector< F > const &fload)
    : hard_matcher(),
      soft_matcher(),
      hard_context(TokIdNode::root),
      soft_context(TokIdNode::root),
      subroot(TokIdNode::root),
      hint_num_nodes(-1),
      fload(fload.begin(), fload.end())
{
    unres.push_back(std::deque< TokId >());
}

Tok::Tok(TokIdNode *node)
    : hard_matcher(),
      soft_matcher(),
      hard_context(TokIdNode::root),
      soft_context(TokIdNode::root),
      subroot(TokIdNode::root),
      hint_num_nodes(-1)
{
    this->node.push_back(node);
    unres.resize(2);
}

Tok::Tok(TokIdNode *node, std::vector< F > const &fload)
    : hard_matcher(),
      soft_matcher(),
      hard_context(TokIdNode::root),
      soft_context(TokIdNode::root),
      subroot(TokIdNode::root),
      hint_num_nodes(-1),
      fload(fload.begin(), fload.end())
{
    unres.resize(1);
}

Tok::Tok(TokIdNode *node, F fl)
    : hard_matcher(),
      soft_matcher(),
      hard_context(TokIdNode::root),
      soft_context(TokIdNode::root),
      subroot(TokIdNode::root),
      hint_num_nodes(-1)
{
    unres.resize(1);
    fload.push_back(fl);
}

Tok::Tok(TokIdNode *node, std::deque< TokId > const &unres)
    : hard_matcher(),
      soft_matcher(),
      hard_context(TokIdNode::root),
      soft_context(TokIdNode::root),
      subroot(TokIdNode::root),
      hint_num_nodes(-1)
{
    this->node.push_back(node);
    this->unres.resize(2);
    this->unres[1] = unres;
}

Tok::Tok(TokIdNode *node, std::deque< TokId > const &unres, std::vector< F > const &fload)
    : hard_matcher(),
      soft_matcher(),
      hard_context(TokIdNode::root),
      soft_context(TokIdNode::root),
      subroot(TokIdNode::root),
      hint_num_nodes(-1),
      fload(fload.begin(), fload.end())
{
    this->node.push_back(node);
    this->unres.resize(2);
    this->unres[1] = unres;
}

bool Tok::operator==(Tok const &rhs) const
{
    int i;

    if((unres[0].size() || node.size()) && (rhs.unres[0].size() || rhs.node.size()))
    {
        i = Match(rhs);
        if(fload.empty() || rhs.fload.empty())
        {
            return i != 0;
        }
        if(!i)
        {
            return false;
        }
    }
    if(fload.size() != rhs.fload.size())
    {
        return false;
    }
    for(i = fload.size(); i--;)
    {
        if(!(fload[i].IsUndef() || rhs.fload[i].IsUndef()))
        {
            if(fload[i].fl != rhs.fload[i].fl)
            {
                return false;
            }
        }
    }
    return true;
}

bool Tok::operator!=(Tok const &rhs) const
{
    return !(*this == rhs);
}

bool Tok::IsUnres() const
{
    int i, j;
    for(i = unres.size(), j = 0; i--;)
    {
        j += unres[i].size();
    }
    return j > 0;
}

bool Tok::IsNil() const
{
    return node.empty() && unres[0].empty() && fload.empty();
}

void Tok::Cat(Tok const &rhs)
{
    unsigned i;

    for(i = 0; i < rhs.unres[0].size(); ++i)
    {
        unres.back().push_back(rhs.unres[0][i]);
    }
    for(i = 1; i < rhs.unres.size(); ++i)
    {
        unres.push_back(rhs.unres[i]);
    }
    for(i = 0; i < rhs.node.size(); ++i)
    {
        node.push_back(rhs.node[i]);
    }
    for(i = 0; i < rhs.fload.size(); ++i)
    {
        fload.push_back(rhs.fload[i]);
    }
}

Tok Tok::Cat(Tok const &a, Tok const &b)
{
    Tok c = a;

    c.Cat(b);
    return c;
}

void Tok::Resolve(int hint_num_nodes, Matcher const &hard_matcher,
                  TokIdNode const *hard_context,
		  TokIdNode const *soft_context,
		  TokIdNode const *subroot)
{
    Resolve(hint_num_nodes, hard_matcher, Matcher(Matcher::chain_and, soft_matcher, hard_matcher),
             hard_context, soft_context, subroot);
}

void Tok::Resolve(int hint_num_nodes,
		  TokIdNode const *hard_context,
		  TokIdNode const *soft_context,
		  TokIdNode const *subroot)
{
    Resolve(hint_num_nodes, hard_matcher, soft_matcher,
             hard_context, soft_context, subroot);
}

void Tok::Resolve(Matcher const &hard_matcher, Matcher const &soft_matcher,
		  TokIdNode const *hard_context,
		  TokIdNode const *soft_context,
		  TokIdNode const *subroot)
{
    Resolve(-1, hard_matcher, soft_matcher, hard_context, soft_context, subroot);
}

void Tok::Resolve(Matcher const &hard_matcher,
		  TokIdNode const *hard_context,
		  TokIdNode const *soft_context,
		  TokIdNode const *subroot)
{
    Resolve(-1, hard_matcher, Matcher(Matcher::chain_and, soft_matcher, hard_matcher),
             hard_context, soft_context, subroot);
}

void Tok::Resolve(TokIdNode const *hard_context,
		  TokIdNode const *soft_context,
		  TokIdNode const *subroot)
{
    Resolve(-1, hard_matcher, soft_matcher,
             hard_context, soft_context, subroot);
}

void Tok::Resolve(int hint_num_nodes, Matcher const &hard_matcher, Matcher const &soft_matcher,
    Tok hard_context, Tok soft_context, Tok subroot)
{
    TokIdNode *hard_context_node, *soft_context_node, *subroot_node;

    hard_context_node = hard_context.ContextCheck();
    soft_context_node = soft_context.ContextCheck();
    subroot_node = subroot.ContextCheck();

    Resolve(hint_num_nodes, hard_matcher, soft_matcher,
             hard_context_node, soft_context_node, subroot_node);
}

void Tok::Resolve(int hint_num_nodes, Matcher const &hard_matcher,
		  Tok hard_context,
		  Tok soft_context,
		  Tok subroot)
{
    TokIdNode *hard_context_node, *soft_context_node, *subroot_node;

    hard_context_node = hard_context.ContextCheck();
    soft_context_node = soft_context.ContextCheck();
    subroot_node = subroot.ContextCheck();

    Resolve(hint_num_nodes, hard_matcher,
             hard_context_node, soft_context_node, subroot_node);
}

void Tok::Resolve(int hint_num_nodes, Tok hard_context, Tok soft_context, Tok subroot)
{
    TokIdNode *hard_context_node, *soft_context_node, *subroot_node;

    hard_context_node = hard_context.ContextCheck();
    soft_context_node = soft_context.ContextCheck();
    subroot_node = subroot.ContextCheck();

    Resolve(hint_num_nodes, hard_context_node, soft_context_node, subroot_node);
}

void Tok::Resolve(Matcher const &hard_matcher, Matcher const &soft_matcher,
		  Tok hard_context,
		  Tok soft_context,
		  Tok subroot)
{
    TokIdNode *hard_context_node, *soft_context_node, *subroot_node;

    hard_context_node = hard_context.ContextCheck();
    soft_context_node = soft_context.ContextCheck();
    subroot_node = subroot.ContextCheck();

    Resolve(-1, hard_matcher, soft_matcher,
             hard_context_node, soft_context_node, subroot_node);
}

void Tok::Resolve(Matcher const &hard_matcher, Tok hard_context, Tok soft_context,
		  Tok subroot)
{
    TokIdNode *hard_context_node, *soft_context_node, *subroot_node;

    hard_context_node = hard_context.ContextCheck();
    soft_context_node = soft_context.ContextCheck();
    subroot_node = subroot.ContextCheck();

    Resolve(-1, hard_matcher, hard_context_node, soft_context_node, subroot_node);
}

void Tok::Resolve(Tok hard_context, Tok soft_context, Tok subroot)
{
    TokIdNode *hard_context_node, *soft_context_node, *subroot_node;

    hard_context_node = hard_context.ContextCheck();
    soft_context_node = soft_context.ContextCheck();
    subroot_node = subroot.ContextCheck();

    Resolve(-1, hard_context_node, soft_context_node, subroot_node);
}

Tok operator^(Tok const &a, Tok const &b)
{
    return Tok::Cat(a, b);
}

Tok const &Tok::operator^=(Tok const &rhs)
{
    *this = *this ^ rhs;
    return *this;
}

TokIdNode *Tok::Node(TokIdNode const *context) const
{
    return Node(0, context);
}

TokIdNode *Tok::Node(Tok const &context) const
{
    return Node(0, context);
}

TokIdNode *Tok::Node(int i, TokIdNode const *context) const
{
    Tok t = *this;
    t.Resolve(context);
    if(t.node.size() <= i)
    {
        PNL_THROW(pnl::CBadArg, "inexistant node requested");
    }
    return t.node[i];
}

Vector<TokIdNode *> Tok::Nodes(TokIdNode const *context) const
{
    Tok t = *this;
    t.Resolve(context);
    return t.node;
}

TokIdNode *Tok::Node(int i, Tok const &context) const
{
    if(context.node.size() != 1 || context.IsUnres())
    {
        PNL_THROW(pnl::CBadArg, "context must be a single resolved node");
    }

    return Node(i, context.node[0]);
}

String Tok::Name() const
{
    if(unres.back().size())
    {
        return unres.back().back();
    }
    if(node.size() != 1)
    {
        PNL_THROW(pnl::CBadArg, "token should be single node");
    }
    return node[0]->Name();
}

std::deque< TokId > Tok::Unresolved(TokIdNode const *context)
{
    Resolve(context);
    if(node.size() > 1)
    {
        PNL_THROW(pnl::CBadArg, "multiple nodes not allowed here");
    }
    return unres.back();
}

std::deque< TokId > Tok::Unresolved(Tok const &context)
{
    if(context.node.size() != 1 || context.IsUnres())
    {
        PNL_THROW(pnl::CBadArg, "context must be a single resolved node");
    }

    return Unresolved(context.node[0]);
}

Tok::F Tok::FltValue(int i) const
{
    return fload.at(i);
}

float Tok::FltValue() const
{
    if(fload.size() != 1 || fload[0].IsUndef())
    {
	ThrowUsingError("Getting number from non-scalar entity", "FltValue");
    }
    return fload[0].fl;
}

int Tok::IntValue() const
{
    int i;

    if(unres.back().size())
    {
        if(unres.back().back().is_int)
        {
            return unres.back().back().int_id;
        }
    }
    else
    {
        for(i = 0; i < node.back()->id.size(); ++i)
        {
            if(node.back()->id[i].is_int)
            {
                return node.back()->id[i].int_id;
            }
        }
    }

    PNL_THROW(pnl::CBadArg, "attempt to extract integer value from token that looks not like integer");
}

TokArr::TokArr()
{}

TokArr::TokArr(Tok const arr[], int len)
{
    int i;

    for(i = 0; i < len; ++i)
    {
        push_back(arr[i]);
    }
}

TokArr::TokArr(float const arr[], int len)
{
    int i;

    for(i = 0; i < len; ++i)
    {
        push_back(arr[i]);
    }
}

TokArr::TokArr(int const arr[], int len)
{
    int i;

    for(i = 0; i < len; ++i)
    {
        push_back(arr[i]);
    }
}

TokArr::TokArr(String const arr[], int len)
{
    int i;

    for(i = 0; i < len; ++i)
    {
        push_back(arr[i]);
    }
}

TokArr::TokArr(char const * const arr[], int len)
{
    int i;

    for(i = 0; i < len; ++i)
    {
        push_back(arr[i]);
    }
}

TokArr::TokArr(char const *s)
{
    Init(s);
}

TokArr::TokArr(String const &s)
{
    Init(s.c_str());
}

TokArr::TokArr(Tok const &tok)
{
    push_back(tok);
}

TokArr &operator<<(TokArr &a, TokArr const &b)
{
    int i;

    for(i = 0; i < b.size(); ++i)
    {
        a.push_back(b[i]);
    }
    return a;
}

TokArr &operator>>(TokArr const &a, TokArr &b)
{
    int i;

    for(i = a.size(); i--;)
    {
        b.push_front(a[i]);
    }
    return b;
}

TokArr operator&=(TokArr &a, TokArr const &b)
{
    return a << b;
}

TokArr operator&(TokArr const &a, TokArr const &b)
{
    TokArr rv = a;
    return rv &= b;
}

Tok const &TokArr::operator[](int i) const
{
    return Base::operator[](i);
}

Tok &TokArr::operator[](int i)
{
    return Base::operator[](i);
}

TokArr TokArr::operator[](Tok const &tok) const
{
    int i;
    TokArr rv;

    for(i = 0; i < size(); ++i)
    {
        if((*this)[i].Match(tok))
        {
            rv.push_back((*this)[i]);
        }
    }
    return rv;
}

Tok::Tok(TokArr const &arr)
{
    *this = arr[0];
}

TokArr TokArr::Span(int first, int last)
{
    TokArr rv;

    for(; first <= last; ++first)
    {
        rv &= Tok(first);
    }
    return rv;
}

void TokArr::Resolve(int hint_num_nodes,
                                  Matcher const &hard_matcher,
                                  Matcher const &soft_matcher,
                                  TokIdNode const *hard_context,
                                  TokIdNode const *soft_context,
                                  TokIdNode const *subroot)
{
    for(int i = size(); i--;)
    {
        (*this)[i].Resolve(hint_num_nodes, hard_matcher, soft_matcher, hard_context, soft_context, subroot);
    }
}

void TokArr::Resolve(int hint_num_nodes,
                                  Matcher const &hard_matcher,
                                  TokIdNode const *hard_context,
                                  TokIdNode const *soft_context,
                                  TokIdNode const *subroot)
{
    for(int i = size(); i--;)
    {
        (*this)[i].Resolve(hint_num_nodes, hard_matcher, hard_context, soft_context, subroot);
    }
}

void TokArr::Resolve(int hint_num_nodes,
                                  TokIdNode const *hard_context,
                                  TokIdNode const *soft_context,
                                  TokIdNode const *subroot)
{
    for(int i = size(); i--;)
    {
        (*this)[i].Resolve(hint_num_nodes, hard_context, soft_context, subroot);
    }
}

void TokArr::Resolve(Matcher const &hard_matcher,
                                  Matcher const &soft_matcher,
                                  TokIdNode const *hard_context,
                                  TokIdNode const *soft_context,
                                  TokIdNode const *subroot)
{
    for(int i = size(); i--;)
    {
        (*this)[i].Resolve(hard_matcher, soft_matcher, hard_context, soft_context, subroot);
    }
}

void TokArr::Resolve(Matcher const &hard_matcher,
                                  TokIdNode const *hard_context,
                                  TokIdNode const *soft_context,
                                  TokIdNode const *subroot)
{
    for(int i = size(); i--;)
    {
        (*this)[i].Resolve(hard_matcher, hard_context, soft_context, subroot);
    }
}

void TokArr::Resolve(TokIdNode const *hard_context,
                                  TokIdNode const *soft_context,
                                  TokIdNode const *subroot)
{
    for(int i = size(); i--;)
    {
        (*this)[i].Resolve(hard_context, soft_context, subroot);
    }
}

void TokArr::Resolve(int hint_num_nodes,
                                  Matcher const &hard_matcher,
                                  Matcher const &soft_matcher,
                                  Tok const &hard_context,
                                  Tok const &soft_context,
                                  Tok const &subroot)
{
    for(int i = size(); i--;)
    {
        (*this)[i].Resolve(hint_num_nodes, hard_matcher, soft_matcher, hard_context, soft_context, subroot);
    }
}

void TokArr::Resolve(int hint_num_nodes,
                                  Matcher const &hard_matcher,
                                  Tok const &hard_context,
                                  Tok const &soft_context,
                                  Tok const &subroot)
{
    for(int i = size(); i--;)
    {
        (*this)[i].Resolve(hint_num_nodes, hard_matcher, hard_context, soft_context, subroot);
    }
}

void TokArr::Resolve(int hint_num_nodes,
                                  Tok const &hard_context,
                                  Tok const &soft_context,
                                  Tok const &subroot)
{
    for(int i = size(); i--;)
    {
        (*this)[i].Resolve(hint_num_nodes, hard_context, soft_context, subroot);
    }
}

void TokArr::Resolve(Matcher const &hard_matcher,
                                  Matcher const &soft_matcher,
                                  Tok const &hard_context,
                                  Tok const &soft_context,
                                  Tok const &subroot)
{
    for(int i = size(); i--;)
    {
        (*this)[i].Resolve(hard_matcher, soft_matcher, hard_context, soft_context, subroot);
    }
}

void TokArr::Resolve(Matcher const &hard_matcher,
                                  Tok const &hard_context,
                                  Tok const &soft_context,
                                  Tok const &subroot)
{
    for(int i = size(); i--;)
    {
        (*this)[i].Resolve(hard_matcher, hard_context, soft_context, subroot);
    }
}

void TokArr::Resolve(Tok const &hard_context,
                                  Tok const &soft_context,
                                  Tok const &subroot)
{
    for(int i = size(); i--;)
    {
        (*this)[i].Resolve(hard_context, soft_context, subroot);
    }
}

#if 0
void vistok(Tok const &t)
{
    int i, j;
    std::cout << "==" << std::endl;
    std::cout << t.node.size() << std::endl;
    std::cout << t.unres.size() << std::endl;
    for(i = 0; i < t.unres.size(); ++i)
    {
        std::cout << t.unres[i].size() << std::endl;
        for(j = 0; j < t.unres[i].size(); ++j)
        {
            std::cout << (String)t.unres[i][j] << std::endl;
	}
    }
    std::cout << "===" << std::endl;
}

int main()
{
    TokIdNode::root->Add("bnet");
    Tok t = "bnet";
    vistok(t);
    t.Node()->Add(0);
    vistok(t);
    std::cout << std::endl << (String)t << std::endl;
    t.Resolve();
    vistok(t);
    std::cout << std::endl << (String)t << std::endl;

    t = "bnet^4.0";
    vistok(t);
    std::cout << std::endl << (String)t << std::endl;
    t.Resolve();
    vistok(t);
    std::cout << std::endl << (String)t << std::endl;

    t = "0";
    vistok(t);
    std::cout << std::endl << (String)t << std::endl;
    t.Resolve();
    vistok(t);
    std::cout << std::endl << (String)t << std::endl;

    t.Node()->Add("node");
    t = "node";
    t.Node()->Add("rock");
    t.Node()->Add("paper");
    t.Node()->Add("scissors");

    t = "bnet";
    t.Node()->Add("1");

    t = "1";
    vistok(t);
    std::cout << std::endl << (String)t << std::endl;
    t.Resolve();
    vistok(t);
    std::cout << std::endl << (String)t << std::endl;

    t = "0";
    vistok(t);
    std::cout << std::endl << (String)t << std::endl;
    t.Resolve();
    vistok(t);
    std::cout << std::endl << (String)t << std::endl;

    std::cout << "huhu2" << std::endl;
    t = "1";
    t.Node()->Add("node");
    std::cout << "huhu3" << std::endl;
    t = "node";
#if 0
    t.Resolve(Tok("1").Node());
#else
    t.Resolve("1");
#endif
    std::cout << "huhu3" << std::endl;
    t.Node()->Add("rock");
    std::cout << "huhu4" << std::endl;
    t.Node()->Add("paper");
    t.Node()->Add("scissors");

    std::cout << "huhu" << std::endl;
    std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAa" << std::endl;

    t = "rock";
    vistok(t);
    std::cout << std::endl << (String)t << std::endl;
    t.Resolve();
    vistok(t);
    std::cout << std::endl << (String)t << std::endl;
    t.Resolve("1");
    vistok(t);
    std::cout << std::endl << (String)t << std::endl;

    std::cout << "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZz" << std::endl;
    t = "1";
    t.Resolve;
    std::cout << std::endl << (String)t << std::endl;

    return 0;
}
#endif

PNLW_END

#endif