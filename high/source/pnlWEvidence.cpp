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

PNLW_BEGIN

void WEvidence::Clear()
{
    m_EvidenceBoard = TokArr();
    m_VarMap.clear();
    m_abHidden.resize(0);
}

void WEvidence::Set(const TokArr &evidence)
{
    TokIdNode *node, *value;

    for(int i = 0; i < evidence.size(); i++)
    {
	node = const_cast<Tok&>(evidence[i]).Node();
	if(node->tag == eTagNetNode)
	{
	    value = 0;
	}
	else if(node->tag == eTagValue)
	{
	    value = node;
	    node = node->v_prev;
	}
	else
	{
	    ThrowUsingError("It is't net node or value of the net node", "Set");
	}

	pnl::CNodeType &nt = *static_cast<pnl::CNodeType*>(node->v_prev->data);

	if(!value)
	{
	    if(nt.IsDiscrete())
	    {
		ThrowUsingError("You should point value in discrete case", "Set");
	    }
	    else if (nt.GetNodeSize() != 1)
	    {
		ThrowUsingError("You should point name of dimension in multivariate case", "Set");
	    }
	    else
	    {
		value = node->v_next;
	    }
	}
	if(!nt.IsDiscrete() && (evidence[i].fload.size() == 0 || evidence[i].fload[0].IsUndef()))
	{
	    ThrowUsingError("You should point value of continuous variable", "Set");	    
	}
	std::map<TokIdNode*, int>::const_iterator location =
	    m_VarMap.find(nt.IsDiscrete() ? node:value);

	if(location == m_VarMap.end())
	{
	    m_VarMap[node] = m_EvidenceBoard.size();
	    m_EvidenceBoard.push_back(evidence[i]);
	    m_abHidden.push_back(false);
	}
	else
	{
	    m_EvidenceBoard[location->second] = evidence[i];
	}
    }
}

TokArr WEvidence::Get() const
{
    TokArr result;

    for(int i = 0; i < m_EvidenceBoard.size(); ++i)
    {
	if(!m_abHidden[i])
	{
	    result << m_EvidenceBoard[i];
	}
    }

    return result;
}

bool WEvidence::IsHidden(Tok &tok) const
{
    TokIdNode *node = tok.Node();

    if(node->tag == eTagValue)
    {
	node = node->v_prev;
    }

    std::map<TokIdNode*, int>::const_iterator location = m_VarMap.find(node);
    if(location == m_VarMap.end())
    {
	String str;
	str << "Can't find node " << String(tok);
	ThrowUsingError(str.c_str(), "Set");
    }

    return m_abHidden[location->second];
}

void WEvidence::SetVisibility(Tok &tok, bool bVisible)
{
    TokIdNode *node = tok.Node();

    std::map<TokIdNode*, int>::const_iterator location = m_VarMap.find(node);
    if(location == m_VarMap.end())
    {
	String str;
	str << "Can't find node " << String(tok);
	ThrowUsingError(str.c_str(), "Set");
    }

    m_abHidden[location->second] = bVisible;
}

bool WEvidence::IsNodeHere(Tok &tok) const
{
    TokIdNode *node = tok.Node();
    std::map<TokIdNode*, int>::const_iterator location = m_VarMap.find(node);

    return (location != m_VarMap.end());
}

PNLW_END
