#pragma warning(push, 2)
#include <queue>
#pragma warning(pop)

#include "Tokens.hpp"

PNLW_BEGIN

TokIdNode *TokIdNode::root = new(TokIdNode)("");
Vector< TokIdNode * > TokIdNode::cemetery;
Tok Tok::root(TokIdNode::root);
Tok Tok::nil = Tok();

TokId::TokId(int i): is_int(true), int_id(i)
{}

TokId::TokId(): is_int(false)
{}

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

TokIdNode::Matcher::Matcher(int tag): imm(tag), type(bit_eq)
{
    chain[0] = chain[1] = 0;
}

TokIdNode::Matcher::Matcher(): type(positive)
{
    chain[0] = chain[1] = 0;
}

TokIdNode::Matcher::Matcher(Type type): type(type)
{
    chain[0] = chain[1] = 0;
    if(type != positive)
    {
	ThrowInternalError(
	    "Matcher without additional parameters must be of positive type",
	    "Matcher::Matcher");
    }
}

TokIdNode::Matcher::Matcher(Type type, int tag): imm(tag), type(type)
{
    chain[0] = chain[1] = 0;
    switch(type)
    {
    case bit_eq:
    case bit_all:
    case bit_any:
	break;
    default:
	ThrowInternalError(
	    "Matcher with one immediate parameter must be of bit_* type",
	    "Matcher::Matcher");
    }
}

TokIdNode::Matcher::Matcher(Type type, Matcher const &chain): type(type)
{
    if(type != chain_not)
    {
	ThrowInternalError(
	    "Matcher with one chain parameter must be of chain_not type",
	    "Matcher::Matcher");
    }
    this->chain[0] = new(Matcher)(chain);
    this->chain[1] = 0;
}

TokIdNode::Matcher::Matcher(Type type, Matcher const &chain1, Matcher const &chain2): type(type)
{
    switch(type)
    {
    case chain_and:
    case chain_or:
    case chain_xor:
	break;
    default:
	ThrowInternalError(
	    "Matcher with two chain parameters must be of type chain_and, chain_or or chain_xor",
	    "Matcher::Matcher");
    }
    chain[0] = new(Matcher)(chain1);
    chain[1] = new(Matcher)(chain2);
}

TokIdNode::Matcher::Matcher(Matcher const &m): type(m.type), imm(m.imm)
{
    switch(type)
    {
    case chain_not:
	chain[0] = new(Matcher)(*m.chain[0]);
	chain[1] = 0;
    case chain_and:
    case chain_or:
    case chain_xor:
	chain[1] = new(Matcher)(*m.chain[1]);
	break;
    default:
	chain[0] = chain[1] = 0;
    }
}

TokIdNode::Matcher const &TokIdNode::Matcher::operator=(Matcher const &m)
{
    if(this != &m)
    {
	delete(chain[0]);
	delete(chain[1]);
	type = m.type;
	imm = m.imm;
	if(m.chain[0])
	{
	    chain[0] = new(Matcher)(*m.chain[0]);
	}
	if(m.chain[1])
	{
	    chain[1] = new(Matcher)(*m.chain[1]);
	}
    }
    return *this;
}

bool TokIdNode::Matcher::Match(TokIdNode const *node) const
{
    switch(type)
    {
    case positive:
	return true;
    case bit_eq:
	return node->tag == imm;
    case bit_all:
	return (node->tag & imm) == imm;
    case bit_any:
	return (node->tag & imm) != 0;
    case chain_not:
	return !chain[0]->Match(node);
    case chain_and:
	return chain[0]->Match(node) && chain[1]->Match(node);
    case chain_or:
	return chain[0]->Match(node) || chain[1]->Match(node);
    case chain_xor:
	return chain[0]->Match(node) ^ chain[1]->Match(node);
    default:
	ThrowInternalError(
	    "Matcher appears to have invalid type",
	    "Matcher::Matcher");
	return false;
    }
}

TokIdNode::Matcher::~Matcher()
{
    delete(chain[0]);
    delete(chain[1]);
}

ResolveContext::ResolveContext(const ResolveContext &oth, int shift,
			       TokIdNode *context):
m_Context(context), m_Unres(oth.m_Unres + shift),
m_Size(oth.m_Size - shift), m_Matcher(oth.m_Matcher)
{}

ResolveContext::ResolveContext(): m_Context(0), m_Size(0) {}

TokIdNode::TokIdNode(): h_next(0), h_prev(0), v_next(0), v_prev(0), level(0), tag(0), data(0)
{}

TokIdNode::TokIdNode(TokId id): h_next(0), h_prev(0), v_next(0), v_prev(0), level(0), tag(0), data(0)
{
    Alias(id);
}

std::pair<TokIdNode *, int> TokIdNode::ResolveOne(const ResolveContext &resContext)
{
    Map::const_iterator it, itEnd;
    const Map &desc = resContext.m_Context->desc;
    std::pair<TokIdNode *, int> tmpRes, result(const_cast<TokIdNode*>(resContext.m_Context), 0);

    it = desc.find(*resContext.m_Unres);
    if(it == desc.end())
    {
	return result;
    }

    for(itEnd = desc.upper_bound(*resContext.m_Unres); it != itEnd; it++)
    {
	if(resContext.m_Size > 1)
	{
	    tmpRes = ResolveOne(ResolveContext(resContext, 1, it->second));
	    ++tmpRes.second;
	}
	else
	{
	    if(resContext.m_Matcher && !resContext.m_Matcher->Match(it->second))
	    {
		continue;
	    }
	    tmpRes.first = it->second;
	    tmpRes.second = 1;
	}
	if(tmpRes.second > result.second)
	{
	    result = tmpRes;
	}
	else if(tmpRes.second == result.second)
	{
	    result.first = 0;
	    if(result.second == resContext.m_Size)
	    {
		break;
	    }
	}
    }

    return result;
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
    static const char fName[] = "TokIdNode::Alias";
    static const char message[] = "attempt to make ambiguous alias, brothers"
	" cannot have matching aliases";

    for(nd = h_prev; nd; nd = nd->h_prev)
    {
        if(nd->Match(id))
        {
	    ThrowUsingError(message, fName);
        }
    }
    for(nd = h_next; nd; nd = nd->h_next)
    {
        if(nd->Match(id))
        {
	    ThrowUsingError(message, fName);
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
		    ThrowInternalError("invariant failed: id must be in descendants multimap, but it is not", "TokIdNode::Kill");
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

Tok::Tok(): m_bResolved(false), m_Context(0)
{
}

Tok::Tok(int i): m_bResolved(false), m_Context(0)
{
    unres.push_back(i);
}

Tok::Tok(float fl): m_bResolved(false), m_Context(0)
{
    fload.push_back(fl);
}

Tok::Tok(char const s[])
{
    Init(s);
}

Tok::Tok(String const &s)
{
    Init(s.c_str());
}

Tok::Tok(std::vector< float > const &fload): m_bResolved(false), m_Context(0)
{
    int i;

    this->fload.resize(fload.size());
    for(i = fload.size(); i--;)
    {
        this->fload[i] = fload[i];
    }
}

Tok::Tok(TokIdNode *node): m_bResolved(false), m_Context(0)
{
    this->node.push_back(node);
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
    m_bResolved = false;
    m_Context = 0;

    while(isspace(*s))
    {
        ++s;
    }
    if(*s == 0)
    {
        unres.push_back("");
        return s;
    }

    for(;;)
    {
        if(*s == '^')
        {
            unres.push_back("");
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
            unres.push_back(i);
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
            unres.push_back(buf);
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
    int i;

    for(i = node.size(); --i >= 0;)
    {
        for(; node[i] != subroot && node[i] != 0; node[i] = node[i]->v_prev)
        {
            unres.push_front(node[i]->id[0]);
        }
    }
    node.resize(0);
}

bool Tok::Match(Tok const &tok) const
{
    int i, j;

    if(IsResolved() && tok.IsResolved())
    {
	if(node.size() != tok.node.size())
	{
	    return false;
	}
	for(i = node.size(); --i >= 0;)
	{
	    if(node[i] != tok.node[i])
	    {
		return false;
	    }
	}
	return true;
    }

    if((node.size() + tok.node.size()) == 0)
    {
	for(i = unres.size(), j = tok.unres.size(); --i >= 0 && --j >= 0;)
	{
	    if(!unres[i].Match(tok.unres[j]))
	    {
		return false;
	    }
	}
	return true;
    }

    Tok a(*this), b(tok);
    
    a.Unresolve();
    b.Unresolve();

    return a.Match(b);
}

Tok::operator String() const
{
    int i;
    String rv;
    Tok t = *this;

    if(node.size())
    {
	t.Unresolve(m_Context);
    }
    for(i = 0; i < t.unres.size(); ++i)
    {
        if(i)
        {
            rv << "^";
        }
        rv << t.unres[i];
    }
    if(t.unres.size() && t.fload.size())
    {
        rv << "^";
    }
    for(i = 0; i < t.fload.size(); ++i)
    {
        if(i)
        {
            rv << "^";
        }
	rv << (String)t.fload[i];
    }

    return rv;
}

void Tok::Resolve(TokIdNode const *context, Matcher *matcher)
{
    if(unres.size() < 1)
    {
	m_bResolved = true;
	return;
    }
    context = FixContext(context);
    if(IsResolved())
    {
	if(context->IsInContext(m_Context) || m_Context->IsInContext(context))
	{
	    return;
	}
	ThrowInternalError("Unresolve: Not yet realized", "Tok::Resolve");
    }

    ResolveContext resContext(context, unres.begin(), unres.size(), matcher);

    while(true)
    {
	std::pair<TokIdNode*, int> rOne = TokIdNode::ResolveOne(resContext);

	if(rOne.second == 0 || rOne.first == 0)
	{
	    if(node.size() <= 1)
	    {
		return;
	    }
	    ThrowUsingError("Token with multiple nodes must be fully resolved",
		"Tok::Resolve");
	}
	unres.erase(unres.begin(), unres.begin() + rOne.second);
	node.push_back(rOne.first);
	if(unres.size() < 1)
	{
	    m_bResolved = true;
	    break;
	}
	resContext.m_Unres = unres.begin();
	resContext.m_Size = unres.size();
    }
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
    if(!tok.IsResolved())
    {
        return tok;
    }
    
    for(i = 0, j = 0; j < tok.node.size(); ++j)
    {
        nd = TokIdNode::GetLeaf(tok.node[j]);
        for(; i < tok.fload.size(); ++i)
        {
            retval &= (Tok(nd) ^ tok.fload[i].fl);
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
        push_back(Tok());
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

bool Tok::operator==(Tok const &rhs) const
{
    int i;

    if((unres.size() || node.size()) && (rhs.unres.size() || rhs.node.size()))
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

void Tok::Cat(Tok const &rhs)
{
    fload.insert(fload.end(), rhs.fload.begin(), rhs.fload.end());
    if(unres.size() == 0 && rhs.unres.size() == 0)
    {
	node.insert(node.end(), rhs.node.begin(), rhs.node.end());
	return;
    }
    else if(node.size() != 0 || rhs.node.size() != 0)
    {
	Tok rhs_(rhs);

	Unresolve();
	rhs_.Unresolve();
    }
    unres.insert(unres.end(), rhs.unres.begin(), rhs.unres.end());
}

Tok Tok::Cat(Tok const &a, Tok const &b)
{
    Tok c = a;

    c.Cat(b);
    return c;
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

String Tok::Name() const
{
    if(unres.size())
    {
        return unres.back();
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
    return unres;
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

    if(unres.size())
    {
        if(unres.back().is_int)
        {
            return unres.back().int_id;
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
