#define NEW_TOKENS
#ifdef NEW_TOKENS
#error switch to new tokens!
#else

#ifndef __PNLTOK_HPP__
#define __PNLTOK_HPP__

#include "pnlHighConf.hpp"

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

PNLW_BEGIN

struct PNLHIGH_API TokId
{
    // internal logic relies on TokId::Match being transitive
    bool Match(TokId const &) const;

    bool operator<(TokId const &) const;
    operator String() const;

    TokId(String const &);
    TokId(char const *);
    inline TokId(int);

    String id;
    bool is_int;
    int int_id;

    void Init(String const &s);
};

PNLHIGH_API TokId::TokId(int i): is_int(true), int_id(i)
{}

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

	Matcher(int tag): imm(tag), type(bit_eq)
	{
	    chain[0] = chain[1] = 0;
	}

	Matcher(): type(positive)
	{
	    chain[0] = chain[1] = 0;
	}

	Matcher(Type type): type(type)
	{
	    chain[0] = chain[1] = 0;
	    if(type != positive)
	    {
		ThrowInternalError(
		    "Matcher without additional parameters must be of positive type",
		    "Matcher::Matcher");
	    }
	}

	Matcher(Type type, int tag): imm(tag), type(type)
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

	Matcher(Type type, Matcher const &chain): type(type)
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

	Matcher(Type type, Matcher const &chain1, Matcher const &chain2): type(type)
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

	Matcher(Matcher const &m): type(m.type), imm(m.imm)
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

	Matcher const &operator=(Matcher const &m)
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

	bool Match(TokIdNode const *node) const
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

	~Matcher()
	{
	    delete(chain[0]);
	    delete(chain[1]);
	}
    };

    static TokIdNode *root;
    static Vector< TokIdNode * > cemetery;

    inline TokIdNode();
    explicit TokIdNode(TokId);

    bool Match(TokId const &) const;
    bool Match(std::deque< TokId >::const_iterator, int len, TokIdNode const *subroot = root) const;
    bool SubMatch(std::deque< TokId >::const_iterator, int len, TokIdNode const *subroot = root) const;

    static TokIdNode *GetLeaf(TokIdNode const *subroot = root);
    static TokIdNode *GetNextLeaf(TokIdNode const *node, TokIdNode const *subroot = root);
    static TokIdNode *GetNext(TokIdNode const *node, TokIdNode const *subroot = root);

    static std::vector< std::pair< TokIdNode *, int > >	AmbigResolve(
	TokIdNode const *node,
	std::deque< TokId >::const_iterator arr,
	int len,
	TokIdNode const *context = root,
	TokIdNode const *subroot = root);
    static std::vector< std::pair< TokIdNode *, int > > AmbigResolve(
	TokIdNode const *node,
	std::deque< TokId >::const_iterator arr,
	int len,
	Matcher const &,
	TokIdNode const *context = root,
	TokIdNode const *subroot = root);
    static inline std::pair< TokIdNode *, int > Resolve(
	TokIdNode const *,
	std::deque< TokId >::const_iterator,
	int len,
	TokIdNode const *hard_context = root,
	TokIdNode const *soft_context = root,
	TokIdNode const *subroot = root);
    static std::pair< TokIdNode *, int > Resolve(
	TokIdNode const *,
	std::deque< TokId >::const_iterator,
	int len,
	Matcher const &hard_matcher,
	TokIdNode const *hard_context = root,
	TokIdNode const *soft_context = root,
	TokIdNode const *subroot = root);
    static std::pair< TokIdNode *, int > Resolve(
	TokIdNode const *,
	std::deque< TokId >::const_iterator,
	int len,
	Matcher const &hard_matcher,
	Matcher const &soft_matcher,
	TokIdNode const *hard_context = root,
	TokIdNode const *soft_context = root,
	TokIdNode const *subroot = root);
    static std::pair< TokIdNode *, int > ResolveN(
	TokIdNode const * context,
	std::deque< TokId >::const_iterator unres,
	int len,
	Matcher const &matcher);
    static bool Match(TokIdNode const *node1, std::deque< TokId >::const_iterator unres1, int len1,
		      TokIdNode const *node2, std::deque< TokId >::const_iterator unres2, int len2);
    bool IsInContext(TokIdNode const *) const;
    String Name() const;

    TokIdNode *Add(TokId const &);
    void Alias(TokId const &);
    void Unalias(TokId const &);
    void Remove(bool no_cemetery = false);
    void Kill();
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

TokIdNode::TokIdNode(): h_next(0), h_prev(0), v_next(0), v_prev(0), level(0), tag(0), data(0)
{}

template< bool exploit_nan >
struct PNLHIGH_API TokFlt
{
    float fl;
    bool undef;
    
    bool IsUndef() const
    {
        return undef;
    }

    TokFlt(): undef(true)
    {}

    TokFlt(float fl): undef(false), fl(fl)
    {}

    operator String()
    {
	char buf[128];

	if(IsUndef())
	{
	    return ".";
	}
	sprintf(buf, "%f", fl);
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
    
    TokFlt(): fl(std::numeric_limits< float >::quiet_NaN())
    {}

    TokFlt(float fl): fl(fl)
    {}

    operator String()
    {
	char buf[128];

	if(IsUndef())
	{
	    return ".";
	}
	sprintf(buf, "%f", fl);
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
    Tok(String const &);
    Tok(char const *);
    inline Tok(int);
    inline Tok(float);
    inline Tok(F);
    Tok(std::vector< float > const &);
    Tok(std::vector< F > const &);
    explicit Tok(TokArr const &);

    operator String() const;
    bool operator==(Tok const &) const;
    bool operator!=(Tok const &rhs) const;

    Tok const &operator^=(Tok const &);

    bool IsUnres() const;
    void Resolve(int hint_num_nodes,
                 Matcher const &hard_matcher,
                 Matcher const &soft_matcher,
                 TokIdNode const *hard_context = 0,
                 TokIdNode const *soft_context = 0,
                 TokIdNode const *subroot = 0);
    void Resolve(int hint_num_nodes,
	Matcher const &hard_matcher,
	TokIdNode const *hard_context = 0,
	TokIdNode const *soft_context = 0,
	TokIdNode const *subroot = 0);
    void Resolve(int hint_num_nodes,
	TokIdNode const *hard_context = 0,
	TokIdNode const *soft_context = 0,
	TokIdNode const *subroot = 0);
    void Resolve(Matcher const &hard_matcher,
	Matcher const &soft_matcher,
	TokIdNode const *hard_context = 0,
	TokIdNode const *soft_context = 0,
	TokIdNode const *subroot = 0);
    void Resolve(Matcher const &hard_matcher,
	TokIdNode const *hard_context = 0,
	TokIdNode const *soft_context = 0,
	TokIdNode const *subroot = 0);
    void Resolve(TokIdNode const *hard_context = 0,
	TokIdNode const *soft_context = 0,
	TokIdNode const *subroot = 0);
    void Resolve(int hint_num_nodes,
	Matcher const &hard_matcher,
	Matcher const &soft_matcher,
	Tok hard_context,
	Tok soft_context = nil,
	Tok subroot = nil);
    void Resolve(int hint_num_nodes,
	Matcher const &hard_matcher,
	Tok hard_context,
	Tok soft_context = nil,
	Tok subroot = nil);
    void Resolve(int hint_num_nodes,
	Tok hard_context,
	Tok soft_context = nil,
	Tok subroot = nil);
    void Resolve(Matcher const &hard_matcher,
	Matcher const &soft_matcher,
	Tok hard_context,
	Tok soft_context = nil,
	Tok subroot = nil);
    void Resolve(Matcher const &hard_matcher,
	Tok hard_context,
	Tok soft_context = nil,
	Tok subroot = nil);
    void Resolve(Tok hard_context,
	Tok soft_context = nil,
	Tok subroot = nil);
    static TokArr PropagateFload(Tok const &);
    void Cat(Tok const &);
    static Tok Cat(Tok const &, Tok const &);
    bool Match(Tok const &) const;
    void Unresolve(TokIdNode const *subroot = TokIdNode::root);
    void Unresolve(Matcher const &);
    TokIdNode *Node(TokIdNode const * = TokIdNode::root) const;
    TokIdNode *Node(Tok const &) const;
    TokIdNode *Node(int i, TokIdNode const * = TokIdNode::root) const;
    TokIdNode *Node(int i, Tok const &) const;
    Vector<TokIdNode *> Nodes(TokIdNode const * = TokIdNode::root) const;
    String Name() const;
    std::deque< TokId > Unresolved(TokIdNode const *context = TokIdNode::root);
    std::deque< TokId > Unresolved(Tok const &context);
    F FltValue(int) const;
    float FltValue() const;
    int IntValue() const;
    bool IsNil() const;
    TokArr GetDescendants() const;
    TokArr GetDescendants(Matcher const &) const;
    static Tok Context();
    static void Context(Tok const &);

    Vector< TokIdNode * > node;
    Vector< std::deque< TokId > > unres;
    Vector< F > fload;
    mutable TokIdNode *hard_context, *soft_context, *subroot;
    mutable Matcher hard_matcher, soft_matcher;
    mutable int hint_num_nodes;

    Tok(TokIdNode *);
    Tok(TokIdNode *, TokIdNode *);
    Tok(TokIdNode *, F);
    Tok(TokIdNode *, std::vector< F > const &);
    Tok(TokIdNode *, std::deque< TokId > const &);
    Tok(TokIdNode *, std::deque< TokId > const &, std::vector< F > const &);

    char const *Init(char const s[]);

private:
    TokIdNode *ContextCheck()
    {
	if(IsNil())
	{
	    return 0;
	}
	else
	{
	    Resolve();
	    if(node.size() != 1 || IsUnres())
	    {
		ThrowInternalError("context must be a single resolved node",
		    "Tok::ContextCheck");
	    }
	    return Node();
	}
    }
};

Tok::Tok()
    : hard_matcher(),
      soft_matcher(),
      hard_context(TokIdNode::root),
      soft_context(TokIdNode::root),
      subroot(TokIdNode::root),
      hint_num_nodes(-1)
{
    unres.resize(1);
}

Tok::Tok(int i)
    : hard_matcher(),
      soft_matcher(),
      hard_context(TokIdNode::root),
      soft_context(TokIdNode::root),
      subroot(TokIdNode::root),
      hint_num_nodes(-1)
{
    unres.resize(1);
    unres[0].push_back(TokId(i));
}

Tok::Tok(float fl)
    : hard_matcher(),
      soft_matcher(),
      hard_context(TokIdNode::root),
      soft_context(TokIdNode::root),
      subroot(TokIdNode::root),
      hint_num_nodes(-1)
{
    unres.push_back(std::deque< TokId >());
    fload.push_back(fl);
}

PNLHIGH_API Tok operator^(Tok const &a, Tok const &b);

Tok::Tok(F fl)
    : hard_matcher(),
      soft_matcher(),
      hard_context(TokIdNode::root),
      soft_context(TokIdNode::root),
      subroot(TokIdNode::root),
      hint_num_nodes(-1)
{
    unres.push_back(std::deque< TokId >());
    fload.push_back(fl);
}

struct PNLHIGH_API TokArr: public std::deque< Tok >
{
    typedef Tok::Matcher Matcher;

    static TokArr nil;

    TokArr();
    TokArr(Tok const &);
    TokArr(Tok const[], int len);
    TokArr(float const[], int len);
    TokArr(int const[], int len);
    TokArr(String const[], int len);
    TokArr(char const * const[], int len);
    TokArr(char const *);
    TokArr(String const &);

    operator String() const;
    Tok const &operator[](int) const;
    Tok &operator[](int);
    TokArr operator[](Tok const &) const;

    static TokArr Span(int first, int last);
    static TokArr IntersectSets(TokArr const &, TokArr const &);
    static TokArr UniteSets(TokArr const &, TokArr const &);
    static TokArr DiffSets(TokArr const &, TokArr const &);
    static TokArr SymmDiffSets(TokArr const &, TokArr const &);

    void Resolve(int hint_num_nodes,
	Matcher const &hard_matcher,
	Matcher const &soft_matcher,
	TokIdNode const *hard_context = 0,
	TokIdNode const *soft_context = 0,
	TokIdNode const *subroot = 0);
    void Resolve(int hint_num_nodes,
	Matcher const &hard_matcher,
	TokIdNode const *hard_context = 0,
	TokIdNode const *soft_context = 0,
	TokIdNode const *subroot = 0);
    void Resolve(int hint_num_nodes,
	TokIdNode const *hard_context = 0,
	TokIdNode const *soft_context = 0,
	TokIdNode const *subroot = 0);
    void Resolve(Matcher const &hard_matcher,
	Matcher const &soft_matcher,
	TokIdNode const *hard_context = 0,
	TokIdNode const *soft_context = 0,
	TokIdNode const *subroot = 0);
    void Resolve(Matcher const &hard_matcher,
	TokIdNode const *hard_context = 0,
	TokIdNode const *soft_context = 0,
	TokIdNode const *subroot = 0);
    void Resolve(TokIdNode const *hard_context = 0,
	TokIdNode const *soft_context = 0,
	TokIdNode const *subroot = 0);
    void Resolve(int hint_num_nodes,
	Matcher const &hard_matcher,
	Matcher const &soft_matcher,
	Tok const &hard_context,
	Tok const &soft_context = Tok::nil,
	Tok const &subroot = Tok::nil);
    void Resolve(int hint_num_nodes,
	Matcher const &hard_matcher,
	Tok const &hard_context,
	Tok const &soft_context = Tok::nil,
	Tok const &subroot = Tok::nil);
    void Resolve(int hint_num_nodes,
	Tok const &hard_context,
	Tok const &soft_context = Tok::nil,
	Tok const &subroot = Tok::nil);
    void Resolve(Matcher const &hard_matcher,
	Matcher const &soft_matcher,
	Tok const &hard_context,
	Tok const &soft_context = Tok::nil,
	Tok const &subroot = Tok::nil);
    void Resolve(Matcher const &hard_matcher,
	Tok const &hard_context,
	Tok const &soft_context = Tok::nil,
	Tok const &subroot = Tok::nil);
    void Resolve(Tok const &hard_context,
	Tok const &soft_context = Tok::nil,
	Tok const &subroot = Tok::nil);

    void Init(const char s[]);

    virtual ~TokArr() {}

    typedef std::deque< Tok > Base;
};

PNLHIGH_API TokArr operator^(TokArr const &a, TokArr const &b);
PNLHIGH_API TokArr &operator<<(TokArr &a, TokArr const &b);
PNLHIGH_API TokArr &operator>>(TokArr const &a, TokArr &b);
PNLHIGH_API TokArr operator&=(TokArr &a, TokArr const &b);
PNLHIGH_API TokArr operator&(TokArr const &a, TokArr const &b);

#if 1 // TEMPORARY FOR DEBUGGING PURPOSES
inline void vistok(Tok const &t)
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
    std::cout << "hard_context- " << (String)t.hard_context->id[0] << std::endl;
    std::cout << "soft_context- " << (String)t.soft_context->id[0] << std::endl;
    std::cout << "===" << std::endl;
}
#endif

PNLW_END

#endif // include guard

#endif // temporary redirector
