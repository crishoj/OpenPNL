#include "PersistWBNet.hpp"
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

const char *PersistWBNet::Signature()
{
    return "ProbabilisticNet";
}

void PersistWBNet::Save(pnl::CPNLBase *pObj, pnl::CContextSave *pContext)
{
    ProbabilisticNet *net = static_cast<pnl::CCover<ProbabilisticNet>*>(pObj)->GetPointer();

    pContext->AddAttribute("NumberOfNodes", net->nNetNode());
}

pnl::CPNLBase *PersistWBNet::Load(pnl::CContextLoad *pContext)
{
    int nNode = -1;
    int i;
    WNodeInfo* pNodeInfo;
    ProbabilisticNet *net = new ProbabilisticNet;
    
    pContext->GetAttribute(&nNode, "NumberOfNodes");
    for(i = 0; i < nNode; ++i)
    {
	pnl::pnlString name("Node");

	name << i;

	pNodeInfo = static_cast<WNodeInfo*>(pContext->Get(name.c_str()));
	net->AddNode((pNodeInfo->m_NodeType.IsDiscrete() ? discrete:continuous)
	    ^ pNodeInfo->m_Name, pNodeInfo->m_aValue);
    }

    return new pnl::CCover<ProbabilisticNet>(net);
}

void PersistWBNet::TraverseSubobject(pnl::CPNLBase *pObj, pnl::CContext *pContext)
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
	net->Token()->GetValues(i, aValue);
	nodeInfo->m_aValue << aValue[0];
	for(j = 1 ; j < aValue.size(); ++j)
	{
	    nodeInfo->m_aValue << ' ' << aValue[j];
	}

	pContext->AutoDelete(nodeInfo);
	pContext->Put(nodeInfo, name.c_str());
    }
}

bool PersistWBNet::IsHandledType(pnl::CPNLBase *pObj) const
{
    return dynamic_cast<pnl::CCover<ProbabilisticNet>*>(pObj) != 0;
}

// this class holds saver/loader for high-level API objects
// We must register loader/saver for type before we save or load it.
// This class registers all such savers/loaders in constructor
class ExternalPersistenceUsing
{
public:
    ExternalPersistenceUsing();
    ~ExternalPersistenceUsing();

private:
    PersistWBNet      m_BayesNetPersistence;
    PersistWNodeInfo  m_WNodeInfoPersistence;
    PersistTokenArray m_TokenArrayPersistence;
};

ExternalPersistenceUsing::ExternalPersistenceUsing()
{
}

ExternalPersistenceUsing::~ExternalPersistenceUsing()
{
}

static ExternalPersistenceUsing objPersistenceUsing;
