#include "PersistProbNet.hpp"
#include "PersistWNodeInfo.hpp"
#include "pnlPersistCover.hpp"
#include "BNet.hpp"
#include "WInner.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "pnlWDistributions.hpp"
#include "pnlWGraph.hpp"
#include "TokenCover.hpp"

#include "pnlPersistCover.hpp"

#if defined(_MSC_VER)
#pragma warning(disable : 4239) // nonstandard extension used: 'T' to 'T&'
#endif

PNLW_BEGIN

const char *PersistProbabilisticNet::Signature()
{
    return "ProbabilisticNet";
}

void PersistProbabilisticNet::Save(pnl::CPNLBase *pObj, pnl::CContextSave *pContext)
{
    ProbabilisticNet *net = static_cast<pnl::CCover<ProbabilisticNet>*>(pObj)->GetPointer();
    String modelType = net->Token().Root()->v_prev->Name();

    pContext->AddAttribute("ModelType", modelType.c_str());
    pContext->AddAttribute("NumberOfNodes", net->nNetNode());
}

pnl::CPNLBase *PersistProbabilisticNet::Load(pnl::CContextLoad *pContext)
{
    int nNode = -1;
    int i;
    WNodeInfo* pNodeInfo;
    String modelType;
    
    pContext->GetAttribute(modelType, "ModelType");
    if(!modelType.length())
    {
        modelType = "bnet";
    }
    ProbabilisticNet *net = new ProbabilisticNet(modelType.c_str());
    
    pContext->GetAttribute(&nNode, "NumberOfNodes");
    for(i = 0; i < nNode; ++i)
    {
	pnl::pnlString name("Node");

	name << i;

	pNodeInfo = static_cast<WNodeInfo*>(pContext->Get(name.c_str()));
        pnl::EIDNodeState ns = pNodeInfo->m_NodeType.GetNodeState();

        TokArr *classificator;

        if(!pNodeInfo->m_NodeType.IsDiscrete())
        {
            classificator = &continuous;
        }
        else // discrete
        {
            switch(ns)
            {
            case pnl::nsChance:   classificator = &chance;    break;
            case pnl::nsDecision: classificator = &decision;  break;
            case pnl::nsValue:    classificator = &value;     break;
            default: ThrowInternalError("Unknown node state", "LoadNet");break;
            }
        }
        net->AddNode(*classificator ^ pNodeInfo->m_Name, pNodeInfo->m_aValue);
    }

    return new pnl::CCover<ProbabilisticNet>(net);
}

void PersistProbabilisticNet::TraverseSubobject(pnl::CPNLBase *pObj, pnl::CContext *pContext)
{
    ProbabilisticNet *net = static_cast<pnl::CCover<ProbabilisticNet>*>(pObj)->GetPointer();
    int i, j;
    int nNode = net->nNetNode();
    Vector<String> aValue;

    for(i = 0; i < nNode; ++i)
    {
	pnl::pnlString name("Node");
	WNodeInfo *nodeInfo = new WNodeInfo(net->NodeName(i), net->pnlNodeType(i));

	name << i;
	net->Token().GetValues(i, aValue);
	nodeInfo->m_aValue << aValue[0];
	for(j = 1 ; j < aValue.size(); ++j)
	{
	    nodeInfo->m_aValue << ' ' << aValue[j];
	}

	pContext->AutoDelete(nodeInfo);
	pContext->Put(nodeInfo, name.c_str());
    }
}

bool PersistProbabilisticNet::IsHandledType(pnl::CPNLBase *pObj) const
{
    return dynamic_cast<pnl::CCover<ProbabilisticNet>*>(pObj) != 0;
}

// this class holds saver/loader for high-level API objects
// We must register loader/saver for type before we save or load it.
// This class registers all such savers/loaders in constructor
class ExternalPersistenceUsing
{
ALLOW_TESTING
public:
    ExternalPersistenceUsing();
    ~ExternalPersistenceUsing();

private:
    PersistProbabilisticNet      m_BayesNetPersistence;
    PersistWNodeInfo  m_WNodeInfoPersistence;
    PersistTokenArray m_TokenArrayPersistence;
    PersistSSMap      m_MapOfString;
};

ExternalPersistenceUsing::ExternalPersistenceUsing()
{
}

ExternalPersistenceUsing::~ExternalPersistenceUsing()
{
}

static ExternalPersistenceUsing objPersistenceUsing;

PNLW_END
