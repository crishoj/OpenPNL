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
using namespace pnl;
WDistributions::WDistributions(TokenCover *pToken): m_pToken(pToken)
{
    SpyTo(m_pToken);
    SpyTo(m_pToken->Graph());
}

void WDistributions::Setup(int iNode)
{
    int nodeClass = m_pToken->NodesClassification(TokArr(Tok(m_pToken->Node(iNode))));

    if(iNode >= m_aDistribution.size())
    {
	m_aDistribution.resize(iNode + 1, 0);
	m_abDiscrete.resize(iNode + 1, nodeClass == eNodeClassDiscrete );
    }
    delete m_aDistribution[iNode];
    if (nodeClass == eNodeClassDiscrete )
        m_aDistribution[iNode] = new WTabularDistribFun();
    else
        if (nodeClass == eNodeClassContinuous)
            m_aDistribution[iNode] = new WGaussianDistribFun();
        else
	    ThrowUsingError("Uknown type", "Setup");


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
    
    int nodeClass = m_pToken->NodesClassification(TokArr(Tok(m_pToken->Node(iNode))));
    if (nodeClass == eNodeClassDiscrete )
    {
        static_cast<pnl::CDenseMatrix<float>*>(ft.GetMatrix(pnl::matTable))->GetRawData(&nElement, &pData);
        Distribution(iNode)->Matrix(pnl::matTable)->SetData(pData);
    }
    else
        if (nodeClass == eNodeClassContinuous)
        {
            static_cast<WGaussianDistribFun*>(Distribution(iNode))->CreateDefaultDistribution();
            static_cast<pnl::CDenseMatrix<float>*>(ft.GetMatrix(pnl::matMean))
                ->GetRawData(&nElement, &pData);
            static_cast<WGaussianDistribFun*>(Distribution(iNode))
                ->SetData(pnl::matMean, pData);

            static_cast<pnl::CDenseMatrix<float>*>(ft.GetMatrix(pnl::matCovariance))
                ->GetRawData(&nElement, &pData);
            static_cast<WGaussianDistribFun*>(Distribution(iNode))
                ->SetData(pnl::matCovariance, pData);

            int NumOfNodes = ft.GetDomainSize();
            if (NumOfNodes > 1)
            {
                static_cast<pnl::CDenseMatrix<float>*>(ft.GetMatrix(pnl::matWeights, 0))
                    ->GetRawData(&nElement, &pData);
                static_cast<WGaussianDistribFun*>(Distribution(iNode))->
                    SetData(pnl::matWeights, pData);
            }
        }
}

void WDistributions::FillData(TokArr &value, TokArr &probability,
			      TokArr &parentValue, pnl::EMatrixType matType)
{
    static const char fname[] = "FillData";

    PNL_CHECK_FOR_ZERO(value.size());

    int index = Token().iNode(value[0]);

    int nodeClass = m_pToken->NodesClassification(TokArr(Tok(m_pToken->Node(index))));

    if(parentValue.size())
    {
	Token().Resolve(parentValue);
    }

    if (nodeClass == eNodeClassDiscrete)
    {
        Distribution(index)->FillData(pnl::matTable, value, probability, parentValue);
    }
    else 
        if (nodeClass == eNodeClassContinuous)
        {
            if (static_cast<WGaussianDistribFun*>(Distribution(index))->
                IsDistributionSpecific() == 1)
            {
/*                if (m_aDistribution[index] != 0)
                    delete m_aDistribution[index];
*/
                  static_cast<WGaussianDistribFun*>(Distribution(index))->CreateDefaultDistribution();
            }

            Distribution(index)->FillData(matType, value, probability, parentValue);
        }
        else
        {
	    ThrowUsingError("Unsupported type of node", fname);
        };
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