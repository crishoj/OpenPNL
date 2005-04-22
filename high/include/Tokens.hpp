#ifndef __TOKENS_HPP__
#define __TOKENS_HPP__

#ifndef __PNLHIGHCONFIG_HPP__
#include "pnlHighConf.hpp"
#endif

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

// FORWARDS
class TokIdNode;
class TokId;
class ResolveContext;

class PNLHIGH_API TokId
{
ALLOW_TESTING
public:
    // internal logic relies on TokId::Match being transitive
    bool Match(TokId const &) const;

    bool operator<(TokId const &) const;
    operator String() const;

    TokId(String const &);
    TokId(char const *);
    TokId(int);
    TokId();

    String id;
    bool is_int;
    int int_id;

    void Init(String const &s);
};

class PNLHIGH_API TokIdNode
{
ALLOW_TESTING
public:
    class Matcher
    {
public:
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

	Matcher(int tag);
	Matcher();
	Matcher(Type type);
	Matcher(Type type, int tag);
	Matcher(Type type, Matcher const &chain);
	Matcher(Type type, Matcher const &chain1, Matcher const &chain2);
	Matcher(Matcher const &m);
	Matcher const &operator=(Matcher const &m);
	bool Match(TokIdNode const *node) const;
	~Matcher();
    };

    static TokIdNode *root;
    static Vector< TokIdNode * > cemetery;

    TokIdNode();
    explicit TokIdNode(TokId);

    bool Match(TokId const &) const;
    bool Match(std::deque< TokId >::const_iterator, int len, TokIdNode const *subroot = root) const;
    bool SubMatch(std::deque< TokId >::const_iterator, int len, TokIdNode const *subroot = root) const;

    static TokIdNode *GetLeaf(TokIdNode const *subroot = root);
    static TokIdNode *GetNextLeaf(TokIdNode const *node, TokIdNode const *subroot = root);
    static TokIdNode *GetNext(TokIdNode const *node, TokIdNode const *subroot = root);

    static std::pair< TokIdNode *, int > ResolveOne(const ResolveContext &resContext);

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

class ResolveContext
{
ALLOW_TESTING
public:
    ResolveContext();
    ResolveContext(const ResolveContext &, int, TokIdNode *);
    ResolveContext(TokIdNode const * context,
	std::deque<TokId>::const_iterator unres,
	int length, TokIdNode::Matcher const *matcher):
    m_Context(context), m_Unres(unres), m_Size(length), m_Matcher(matcher)
    {}

    TokIdNode const * m_Context;
    std::deque<TokId>::const_iterator m_Unres;
    int m_Size;
    TokIdNode::Matcher const *m_Matcher;
};

template< bool exploit_nan >
class PNLHIGH_API TokFlt
{
ALLOW_TESTING
public:
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
class PNLHIGH_API TokFlt< true >
{
ALLOW_TESTING
public:
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
class TokArr;

class PNLHIGH_API Tok
{
ALLOW_TESTING
public:
    typedef TokFlt< std::numeric_limits< float >::has_quiet_NaN > F;
    typedef TokIdNode::Matcher Matcher;

    static Tok root;
    static Tok nil;

    Tok();
    Tok(String const &);
    Tok(char const *);
    Tok(int);
    Tok(float);
    Tok(std::vector< float > const &);
    Tok(TokIdNode *);
    explicit Tok(TokArr const &);

    operator String() const;
    bool operator==(Tok const &) const;
    bool operator!=(Tok const &rhs) const;

    Tok const &operator^=(Tok const &);

    char const *Init(char const s[]);
    static TokArr PropagateFload(Tok const &);
    static Tok Cat(Tok const &, Tok const &);
    void Cat(Tok const &);
    bool Match(Tok const &) const;
    void Resolve(TokIdNode const *context = 0, Matcher *matcher = 0);
    void Unresolve(TokIdNode const *subroot = TokIdNode::root);
    TokIdNode *Node(TokIdNode const * = TokIdNode::root) const;
    TokIdNode *Node(int i, TokIdNode const * = TokIdNode::root) const;
    Vector<TokIdNode *> Nodes(TokIdNode const * = TokIdNode::root) const;
    String Name() const;
    std::deque< TokId > Unresolved(TokIdNode const *context = TokIdNode::root);
    F FltValue(int) const;
    float FltValue() const;
    int IntValue() const;
    bool IsResolved() const { return m_bResolved; }
    const TokIdNode *FixContext(const TokIdNode *context)
    {
	if(!context)
	{
	    return (!m_Context)
		? (m_Context = TokIdNode::root)	: m_Context;
	}
	else if(!m_Context)
	{
	    return (m_Context = context);
	}
	else if(m_Context->IsInContext(context))
	{
	    return m_Context;
	}

	return (m_Context = context);
    }

    Vector< TokIdNode * > node;
    std::deque<TokId> unres;
    Vector< F > fload;
    const TokIdNode *m_Context;
    bool m_bResolved;
};

PNLHIGH_API Tok operator^(Tok const &a, Tok const &b);

class PNLHIGH_API TokArr: public std::deque< Tok >
{
ALLOW_TESTING
public:
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
    int i;

    std::cout << "==" << std::endl;
    std::cout << t.node.size() << std::endl;
    std::cout << t.unres.size() << std::endl;
    for(i = 0; i < t.unres.size(); ++i)
    {
	std::cout << (String)t.unres[i] << std::endl;
    }
    std::cout << "===" << std::endl;
}
#endif

PNLW_END

#endif // include guard
