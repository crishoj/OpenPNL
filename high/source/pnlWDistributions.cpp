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
#include "WCliques.hpp"

#if defined(_MSC_VER)
#pragma warning(disable : 4239) // nonstandard extension used: 'T' to 'T&'
#endif

PNLW_BEGIN

WDistributions::WDistributions(TokenCover *pToken): m_pToken(pToken), m_bMRF(false)
{
    SpyTo(m_pToken);
    SpyTo(m_pToken->Graph());
}

WDistributions::~WDistributions()
{
    if(m_bMRF)
    {
	delete m_pCliques;
    }
}

void WDistributions::SetMRF(bool mrfFlag /* = true */)
{
    m_bMRF = mrfFlag;
    if(mrfFlag)
    {
	StopSpyTo(m_pToken->Graph());
	m_pCliques = new WCliques(m_pToken->Graph());
	SpyTo(m_pCliques);
    }
    else
    {
	ThrowInternalError("Not yet realized", "SetMRF(false)");
    }
}

void WDistributions::Setup(int iNode)
{
    TokArr ta(Tok(m_pToken->Node(iNode)));
    int nodeClass = m_pToken->NodesClassification(ta);
    if(iNode >= m_aDistribution.size())
    {
	m_aDistribution.resize(iNode + 1, 0);
	m_abDiscrete.resize(iNode + 1, nodeClass == eNodeClassDiscrete );

    }
    delete m_aDistribution[iNode];

    if (nodeClass == eNodeClassDiscrete )
    {
        m_aDistribution[iNode] = new WTabularDistribFun();
    }
    else
    {
        if (nodeClass == eNodeClassContinuous)
	{
            m_aDistribution[iNode] = new WGaussianDistribFun();
	}
        else
	{
	    ThrowUsingError("Uknown type", "Setup");
	}
    }

    Vector<int> aParent;
    
    m_pToken->Graph()->GetParents(&aParent, iNode);
    Vector<TokIdNode*> parTokId = m_pToken->Nodes(aParent);
    m_aDistribution[iNode]->Setup(m_pToken->Node(iNode), parTokId);
    TokIdNode *tok = m_pToken->Node(iNode);
    if (nodeClass == eNodeClassDiscrete )
    {
        if (static_cast<pnl::CNodeType*>(tok->v_prev->data)->GetNodeState() != pnl::nsValue)
        {
            m_aDistribution[iNode]->SetDefaultDistribution();
        }
        else
        {
            static_cast<WTabularDistribFun*>(m_aDistribution[iNode])->SetDefaultUtilityFunction();
        }
    }
    else
    {
        m_aDistribution[iNode]->SetDefaultDistribution();
    }

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

void WDistributions::GetNodeTypeInfo(bool *pbDiscrete, int *pSize, pnl::EIDNodeState *nodeState, int iNode)
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

    TokIdNode *TokId = m_pToken->Node(iNode);
    while(TokId && TokId->tag != eTagNodeType)
    {
	    TokId = TokId->v_prev;
    }
    *nodeState = ((pnl::CNodeType*)(TokId->data))->GetNodeState();
}

void WDistributions::ResetDistribution(int iNode, pnl::CFactor &ft)
{
    const float *pData;
    int nElement;

    DropDistribution(iNode);
    
    TokArr ta(Tok(m_pToken->Node(iNode)));
    int nodeClass = m_pToken->NodesClassification(ta);
    
    if (nodeClass == eNodeClassDiscrete )
    {
        static_cast<pnl::CDenseMatrix<float>*>(ft.GetMatrix(pnl::matTable))->
            GetRawData(&nElement, &pData);
        Distribution(iNode)->Matrix(pnl::matTable)->SetData(pData);
    }
    else
        if (nodeClass == eNodeClassContinuous)
        {
            static_cast<WGaussianDistribFun*>(Distribution(iNode))->CreateDefaultDistribution();

            if (ft.GetDistribFun()->IsDistributionSpecific() == 1)
            {
                static_cast<WGaussianDistribFun*>(Distribution(iNode))
                ->CreateDistribution();
            }
            else
            {
                static_cast<pnl::CDenseMatrix<float>*>(ft.GetMatrix(pnl::matMean))
                    ->GetRawData(&nElement, &pData);
                static_cast<WGaussianDistribFun*>(Distribution(iNode))
                    ->SetData(pnl::matMean, pData);
                
                if (ft.GetDistribFun()->IsDistributionSpecific() != 2)
                {
                    static_cast<pnl::CDenseMatrix<float>*>(ft.GetMatrix(pnl::matCovariance))
                        ->GetRawData(&nElement, &pData);
                    static_cast<WGaussianDistribFun*>(Distribution(iNode))
                        ->SetData(pnl::matCovariance, pData);
                }
               
                int NumOfNodes = ft.GetDomainSize();
                for (int parent = 0; parent < (NumOfNodes-1); parent++)
                {
                    static_cast<pnl::CDenseMatrix<float>*>(ft.GetMatrix(pnl::matWeights, parent))
                        ->GetRawData(&nElement, &pData);
                    static_cast<WGaussianDistribFun*>(Distribution(iNode))->
                        SetData(pnl::matWeights, pData, parent);
                }
            }
        }
}

void WDistributions::FillData(TokArr &value, TokArr &probability,
			      const TokArr &parentValue, pnl::EMatrixType matType)
{
    static const char fname[] = "FillData";

    PNL_CHECK_FOR_ZERO(value.size());

    int index = Token().iNode(value[0]);

    TokArr ta(Tok(m_pToken->Node(index)));
    int nodeClass = m_pToken->NodesClassification(ta);
    if(parentValue.size())
    {
	Token().Resolve(const_cast<TokArr&>(parentValue));
    }

    if (nodeClass == eNodeClassDiscrete)
    {
        Distribution(index)->FillData(pnl::matTable, value, probability, parentValue);
    }
    else
    {
        if (nodeClass == eNodeClassContinuous)
        {
            if (static_cast<WGaussianDistribFun*>(Distribution(index))->
                IsDistributionSpecific() == 1)
            {

                  static_cast<WGaussianDistribFun*>(Distribution(index))->CreateDefaultDistribution();
            }

            Distribution(index)->FillData(matType, value, probability, parentValue);
        }
        else
        {
	    ThrowUsingError("Unsupported type of node", fname);
        }
    }
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

PNLW_END
