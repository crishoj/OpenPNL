#include "pnlTok.hpp"
#pragma warning(push, 2)
#pragma warning(disable: 4251)
// class X needs to have dll-interface to be used by clients of class Y
#include "pnl_dll.hpp"
#pragma warning(default: 4251)
#pragma warning(pop)
#include "WInner.hpp"
#include "WDistribFun.hpp"
#include "TokenCover.hpp"
#include "pnlWGraph.hpp"
#include "pnlWDistributions.hpp"

#if defined(_MSC_VER)
#pragma warning(disable : 4239) // nonstandard extension used: 'T' to 'T&'
#endif

WDistributions::WDistributions(TokenCover *pToken): m_pToken(pToken)
{
    SpyTo(m_pToken);
    SpyTo(m_pToken->Graph());
}

void WDistributions::Setup(int iNode)
{
    if(iNode >= m_aDistribution.size())
    {
	m_aDistribution.resize(iNode + 1, 0);
	m_abDiscrete.resize(iNode + 1, true);
    }
    delete m_aDistribution[iNode];
    m_aDistribution[iNode] = new WTabularDistribFun();

    Vector<int> aParent;
    
    m_pToken->Graph()->GetParents(&aParent, iNode);
    m_aDistribution[iNode]->Setup(m_pToken->Node(iNode), m_pToken->Nodes(aParent));
    m_aDistribution[iNode]->SetDefaultDistribution();
}

void WDistributions::DropDistribution(int iNode)
{
    delete m_aDistribution[iNode];
    m_aDistribution[iNode] = 0;
    Vector<int> aChild;
    
    m_pToken->Graph()->GetChildren(&aChild, iNode);
    for(int i = aChild.size(); --i >= 0;)
    {
	Setup(aChild[i]);
    }
}

void WDistributions::Apply(int iNode)
{
}

bool WDistributions::IsValid(int iNode)
{
    return m_pToken->Graph()->IsValidINode(iNode) && m_aDistribution[iNode]
	&& m_aDistribution[iNode]->Matrix(0);
}

void WDistributions::GetNodeTypeInfo(bool *pbDiscrete, int *pSize, int iNode)
{
    if(iNode >= m_abDiscrete.size())
    {
	if(!m_pToken->Graph()->IsValidINode(iNode))
	{
	    ThrowUsingError("Requested info for non-existant node", "GetNodeTypeInfo");
	}
	Setup(iNode);
    }
    *pbDiscrete = m_abDiscrete[iNode];
    *pSize = m_pToken->nValue(iNode);
}

void WDistributions::ResetDistribution(int iNode, pnl::CFactor &ft)
{
    const float *pData;
    int nElement;

    DropDistribution(iNode);
    static_cast<pnl::CDenseMatrix<float>*>(ft.GetMatrix(pnl::matTable))->GetRawData(&nElement, &pData);
    Distribution(iNode)->Matrix(pnl::matTable)->SetData(pData);
}

void WDistributions::FillData(TokArr &value, TokArr &probability, TokArr &parentValue)
{
    PNL_CHECK_FOR_ZERO(value.size());

    int index = Token().iNode(value[0]);

    if(parentValue.size())
    {
	Token().Resolve(parentValue);
    }

    Distribution(index)->FillData(pnl::matTable, value, probability, parentValue);
}

void WDistributions::DoNotify(int message, int iNode, ModelEngine *pObj)
{
    switch(message)
    {
    case eDelNode:
	if(m_aDistribution[iNode])
	{
	    delete m_aDistribution[iNode];
	    m_aDistribution[iNode] = 0;
	}
	break;
    case eChangeParentNState:
    case eChangeNState:
	Setup(iNode);
	Apply(iNode);
	break;
    case eInit:
	Setup(iNode);
	Apply(iNode);
	break;
    default:
	ThrowInternalError("Unhandled message arrive" ,"DoNotify");
	return;
    }
}

WGraph &WDistributions::Graph()
{
    return *Token().Graph();
}