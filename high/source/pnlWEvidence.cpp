#include "pnlTok.hpp"
#include "pnlWEvidence.hpp"
#include "WDistribFun.hpp"
#pragma warning(push, 2)
#pragma warning(disable: 4251)
// class X needs to have dll-interface to be used by clients of class Y
#include "pnl_dll.hpp"
#include "pnlString.hpp"
#pragma warning(default: 4251)
#pragma warning(pop)
#include "WInner.hpp"

#if defined(_MSC_VER)
#pragma warning(disable : 4239) // nonstandard extension used: 'T' to 'T&'
#endif

void WEvidence::Clear()
{
    m_EvidenceBoard = TokArr();
    m_VarMap.clear();
}

void WEvidence::Set(const TokArr &evidence)
{
    TokIdNode *node;

    for(int i = 0; i < evidence.size(); i++)
    {
	node = const_cast<Tok&>(evidence[i]).Node();
	if(node->tag == eTagValue)
	{
	    node = node->v_prev;
	}
	if(node->tag != eTagNetNode)
	{
	    ThrowUsingError("Wrong name for node of Bayes Net", "SetEvidence");
	}
	std::map<TokIdNode*, int>::iterator location = m_VarMap.find(node);
	if(location == m_VarMap.end())
	{
	    m_VarMap[node] = m_EvidenceBoard.size();
	    m_EvidenceBoard.push_back(evidence[i]);
	}
	else
	{
	    m_EvidenceBoard[location->second] = evidence[i];
	}
    }
}
