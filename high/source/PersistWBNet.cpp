#include "PersistWBNet.hpp"
#include "PersistWNodeInfo.hpp"
#include "pnlPersistCover.hpp"
#include "BNet.hpp"
#include "WInner.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "pnlWDistributions.hpp"
#include "pnlWGraph.hpp"
#include "TokenCover.hpp"

#if defined(_MSC_VER)
#pragma warning(disable : 4239) // nonstandard extension used: 'T' to 'T&'
#endif

const char *PersistWBNet::Signature()
{
    return "BayesNet";
}

void PersistWBNet::Save(pnl::CPNLBase *pObj, pnl::CContextSave *pContext)
{
    BayesNet *bnet = static_cast<BayesNet*>(pObj);
#ifdef OldB
    int mask = bnet->m_Objects & (eGraph | eModel);

    if(mask != (eGraph | eModel))
    {
	ThrowUsingError("BayesNet cann't be saved while model isn't exists", "SaveBayesNet");
    }
#endif

    pContext->AddAttribute("NumberOfNodes", bnet->Net().nNetNode());
}

pnl::CPNLBase *PersistWBNet::Load(pnl::CContextLoad *pContext)
{
    int nNode = -1;
    int i;
    pnl::pnlVector<WNodeInfo*> aNodeInfo;
    BayesNet *bnet = new BayesNet;
    
    pContext->GetAttribute(&nNode, "NumberOfNodes");
    aNodeInfo.resize(nNode);
    for(i = 0; i < nNode; ++i)
    {
	pnl::pnlString name("Node");

	name << i;

	aNodeInfo[i] = static_cast<pnl::CCover<WNodeInfo>*>(pContext->Get(
	    name.c_str()))->GetPointer();
	bnet->AddNode((aNodeInfo[i]->m_NodeType.IsDiscrete() ? categoric:continuous)
	    ^ aNodeInfo[i]->m_Name, aNodeInfo[i]->m_aValue);
    }

#ifdef OldB
    bnet->m_Model = static_cast<pnl::CBNet*>(pContext->Get("Model"));
    bnet->m_Objects |= (eGraph | eModel);
#endif

    return 0;
}

void PersistWBNet::TraverseSubobject(pnl::CPNLBase *pObj, pnl::CContext *pContext)
{
    BayesNet *bnet = static_cast<BayesNet*>(pObj);
    int nNode = bnet->Net().nNetNode();

	pContext->Put(bnet->Net().Model(), "Model");

    for(int i = 0; i < nNode; ++i)
    {
	pnl::pnlString name("Node");
	WNodeInfo *nodeInfo = new WNodeInfo(bnet->Net().NodeName(i), bnet->Net().pnlNodeType(i));

	name << i;
	pnl::CCover<WNodeInfo> *pCov = new pnl::CCoverDel<WNodeInfo>(nodeInfo);
	Tok tok(nodeInfo->m_Name);

	bnet->Net().Token()->Resolve(tok);
	nodeInfo->m_aValue = tok.GetDescendants(eTagValue);
	
	pContext->AutoDelete(pCov);
	pContext->Put(pCov, name.c_str());
    }
}

bool PersistWBNet::IsHandledType(pnl::CPNLBase *pObj) const
{
    return dynamic_cast<BayesNet*>(pObj) != 0;
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
