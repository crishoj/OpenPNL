#undef _DEBUG
#undef DEBUG
#include "xmlreader.h"

#include "networkPNLH.hpp"
#include "diagnetworkPNLH.hpp"
#include "XMLReaderPNLH.hpp"
#include "XMLWriterPNLH.hpp"
#include "XMLContainer.hpp"
#include "BNet.hpp"
#include "WDistribFun.hpp"
#include "pnlLog.hpp"
#undef _DEBUG
#undef DEBUG
#include "constants.h"
#include "TokenCover.hpp"
#include "pnlWGraph.hpp"
#include "pnlWEvidence.hpp"
#include "pnlWDistributions.hpp"
#include "pnlWProbabilisticNet.hpp"

#include "pnlGraph.hpp"

#include "legacydsl.h"
#include "xmlwriter.h"
#include "pnlContextPersistence.hpp"
#include "pnlContextLoad.hpp"
#include "pnlContextSave.hpp"
#include "pnlPersistence.hpp"
#include "pnlXMLRead.hpp"
#include "pnlXMLWrite.hpp"
#include "pnlPersistCover.hpp"
#include "PersistNodeDiagInfo.hpp"
#include "pnlGroup.hpp"

static void TranslatePropertyToReal(NetworkPNL::PropertyRealMap *pTo, const NetworkPNL::PropertyMap &from)
{
    pTo->clear();
    for(int i = 0; i < from.size(); ++i)
    {
	(*pTo)[from[i].first] = from[i].second;
    }
}

static void TranslatePropertyFromReal(NetworkPNL::PropertyMap *to, const NetworkPNL::PropertyRealMap &from)
{
    NetworkPNL::PropertyRealMap::const_iterator it, itEnd;
    int i;

    it = from.begin();
    itEnd = from.end();
    to->resize(0);
    to->reserve(from.size());
    for(i = 0; it != itEnd; ++it, ++i)
    {
	to->push_back(std::pair<std::string, std::string>(it->first.c_str(), it->second.c_str()));
    }
    if(i != from.size())
    {
	ThrowInternalError("Bad coping", "TranslatePropertyFromReal");
    }
}

void NetworkPNL::Delete()
{
    MarkCallFunction("Delete", true);
    delete this;
}

NetworkPNL::NetworkPNL(): m_ErrorOutput(0), m_NetName("PNLBayesNet")
{
    m_pWNet = new BayesNet;

    m_cost.resize(0);
    m_diagType.resize(0);
    m_ranked.resize(0);
    m_mandatory.resize(0);
    m_defaultOutcome.resize(0);
    m_faultOutcome.resize(0);
    m_stateLabels.resize(0);
    m_entropy = 0.0;
    m_entropyMax = 10.0;

    m_aNodeProperty.reserve(40);
    m_NetProperty["NetName"] = m_NetName;
    m_pLogStream = new pnl::LogDrvStream("GeNIe_PNLModule.log", pnl::eLOG_ALL, pnl::eLOGSRV_ALL);
    m_pLog = new pnl::Log("", pnl::eLOG_DEBUG|pnl::eLOG_NOTICE|pnl::eLOG_INFO, pnl::eLOGSRV_TEST1);
    (*m_pLog) << "Ok, Started!\n";
//    m_aEvidence = new WEvidence;
}

NetworkPNL::~NetworkPNL()
{
    delete m_pWNet;
    delete m_pLog;
    delete m_pLogStream;
//    delete m_aEvidence;
}

const char* NetworkPNL::GetId()
{
    return m_NetName.c_str();
}

bool NetworkPNL::SetId(const char*id)
{
    m_NetName.assign(id);
    m_NetProperty["NetName"] = id;
    return true;
}
    
// Persistence

bool NetworkPNL::Load(const char *filename, IXmlBinding *externalBinding)
{
    MarkCallFunction("Load", true);

    XmlReaderPNLH reader;
    XMLContainer container;
    XMLContainerFilter filter(&container, "genie");
    pnl::CContextPersistence xml;
    
    xml.LoadXMLToContainer(&filter, filename);
    xml.LoadXML(filename);
    m_pWNet->LoadNet(filename);
    {
        int nNode = m_pWNet->Net().nNetNode();

        m_aNodeProperty.assign(nNode, PropertyRealMap());
        m_aNodeValueStatus.assign(nNode, NetConst::NotUpdated);
        m_pWNet->Net().EvidenceBoard()->Clear();// stub ?

        pnl::CGroupObj *propertyGroup = static_cast<pnl::CGroupObj *>(xml.Get("Bridge_PNL_to_GeNIe"));
        pnl::CGroupObj *propertyDiag = static_cast<pnl::CGroupObj *>(xml.Get("DiagnosisInfo"));
        pnl::CGroupObj *propertyNodes;
        NodeDiagInfo dInfo;

        m_cost.resize(nNode);
        m_diagType.resize(nNode);
        m_ranked.resize(nNode);
        m_mandatory.resize(nNode);
        m_defaultOutcome.resize(nNode);
        int iter;
        m_faultOutcome.resize(nNode);
        m_stateLabels.resize(nNode);
        for(iter = 0; iter < nNode; ++iter)
        {
            m_faultOutcome[iter].resize(GetOutcomeCount(iter));
            m_stateLabels[iter].resize(GetOutcomeCount(iter));
        }
        m_NetProperty = *static_cast<pnl::CCover<PropertyRealMap>*>(
            propertyGroup->Get("NetProperty", true))->GetPointer();
        m_NetName.assign(m_NetProperty["NetName"].c_str());
        propertyNodes = static_cast<pnl::CGroupObj *>(propertyGroup->Get("Nodes", true));

        if(propertyNodes)
        {
            pnl::CCover<PropertyRealMap>* pProperty;
            pnl::pnlVector<pnl::pnlString> aName;
            String name;
            int i, j;

            propertyNodes->GetChildrenNames(&aName);

            for(i = 0; i < aName.size(); ++i)
            {
                if(aName[i] == "NetProperty")
                {
                    continue;
                }
                pProperty = static_cast<pnl::CCover<PropertyRealMap>*>(
                    propertyNodes->Get(aName[i].c_str(), true));
                name.resize(0);
                name.append(aName[i].c_str(), aName[i].length() - 9);// remove ".property"
                j = m_pWNet->Net().Graph().INode(name);
                if(j < 0 || j >= nNode)
                {
                    continue;
                }
                m_aNodeProperty[j] = *pProperty->GetPointer();
            }
        }
        if(propertyDiag)
        {
            pnl::CCover<NodeDiagInfo>* pProperty;
            pnl::pnlVector<pnl::pnlString> aName;
            propertyDiag->GetChildrenNames(&aName);
            int i, j;
            NodeDiagInfo *pndInfo;

            for(i = 0; i < aName.size(); ++i)
            {
                pProperty = static_cast<pnl::CCover<NodeDiagInfo>*>(
                    propertyDiag->Get(aName[i].c_str(), true));
                j = m_pWNet->Net().Graph().INode(aName[i]);
                if(j < 0 || j >= nNode)
                {
                    continue;
                }
                pndInfo = pProperty->GetPointer();
                int diagType;
                if (pndInfo->m_diagType == "Target")
                    diagType = 1;
                else
                    if (pndInfo->m_diagType == "Observation")
                        diagType = 2;
                    else
                        if (pndInfo->m_diagType == "Auxiliary")
                            diagType =3;
                SetDiagType(j, diagType);
                SetRanked(j, pndInfo->m_isRanked);
                SetMandatory(j, pndInfo->m_isMandatory);
                SetDefaultOutcome(j, pndInfo->m_isDefault);
                int k;
                for( k=0; k < pndInfo->m_isTarget.size(); ++k)
                {
                    SetFaultOutcome(j,k,pndInfo->m_isTarget[k]);
                }
            }
        }
    }

    return reader.Parse(container, externalBinding);
}

bool NetworkPNL::Save(const char *filename, IXmlWriterExtension *externalExtension,
                       const LegacyDslFileInfo *gnet)
{
    MarkCallFunction("Save", true);
    static const char fname[] = "Save";
    pnl::CContextPersistence saver;
    pnl::CXMLWriterStd writer;
    pnl::CGroupObj pnlhGroup;
    bool bWriteNodeProperty = false;

    if(!writer.OpenFile(filename))
    {
        ThrowUsingError("can't open file", fname);
    }
    if(!m_pWNet->Net().SaveNet(&saver, &pnlhGroup))
    {
        ThrowInternalError("Can't save file", fname);
    }
    saver.Put(&pnlhGroup, "WrapperInfo", false);

    pnl::CGroupObj propertyGroup, propertyNodes, propDiagInfo;

    propertyGroup.Put(new pnl::CCover<PropertyRealMap >(&m_NetProperty),
        "NetProperty", true);

    Vector<String> aNodeName = Graph().Names();
    String name;
    int i, j, k;
    std::vector<NodeDiagInfo> diagInfo;
    diagInfo.resize(aNodeName.size());

    for(i = 0; i < aNodeName.size(); ++i)
    {
        switch (GetDiagType(i))
        {
        case 1:
            diagInfo[i].m_diagType = "Target";
            break;
        case 2:
            diagInfo[i].m_diagType = "Observation";
            break;
        case 3:
            diagInfo[i].m_diagType = "Auxiliary";
            break;
        }
        diagInfo[i].m_isMandatory = IsMandatory(i);
        diagInfo[i].m_isRanked = IsRanked(i);
        diagInfo[i].m_isDefault = GetDefaultOutcome(i);
        std::vector<bool> targetStates;
        diagInfo[i].m_isTarget.resize(GetOutcomeCount(i));
        for (k = 0; k < GetOutcomeCount(i); ++k)
        {
            diagInfo[i].m_isTarget[k] = IsFaultOutcome(i,k);
        }
        std::vector<double> costs;
        costs.resize(0);
        GetCost(i, costs);
        diagInfo[i].cost = costs[0];
        name = aNodeName[i];
        propDiagInfo.Put(new pnl::CCover<NodeDiagInfo >(&diagInfo[i]),
            name.c_str(), true);
        j = Graph().INode(aNodeName[i]);
        if(!m_aNodeProperty[j].size())
        {
            continue;
        }
        bWriteNodeProperty = true;
        name = aNodeName[i];
        name << ".property";
        propertyNodes.Put(new pnl::CCover<PropertyRealMap >(&m_aNodeProperty[j]),
            name.c_str(), true);
    }
    if(bWriteNodeProperty)
    {
        propertyGroup.Put(&propertyNodes, "Nodes", false);
    }

    saver.Put(&propertyGroup, "Bridge_PNL_to_GeNIe", false);
    saver.Put(&propDiagInfo, "DiagnosisInfo", false);

    if(!saver.SaveViaWriter(&writer))
    {
	ThrowInternalError("Can't save file", fname);
    }

    XmlWriterPNLH genieWriter(&writer);
    externalExtension->WriteExtension(&genieWriter);
    if(!writer.CloseFile())
    {
	ThrowInternalError("Can't close file", fname);
    }

    return true;
}

bool NetworkPNL::UpdateBeliefs()
{
    MarkCallFunction("UpdateBeliefs", true);
    for(int i = m_aNodeValueStatus.size(); --i >= 0; m_aNodeValueStatus[i] = NetConst::Updated);
    return true;
}
    
void NetworkPNL::SetBnAlgorithm(int algorithm)
{
    MarkCallFunction("SetBnAlgorithm");
}

int NetworkPNL::GetBnAlgorithm()
{
    MarkCallFunction("GetBnAlgorithm");
    return 0;
}

void NetworkPNL::SetIdAlgorithm(int algorithm)
{
    MarkCallFunction("SetIdAlgorithm");
}

int NetworkPNL::GetIdAlgorithm()
{
    MarkCallFunction("GetIdAlgorithm");
    return 0;
}

bool NetworkPNL::SetAlgorithmParams(int algorithm, const std::vector<double> &params)
{
    MarkCallFunction("SetAlgorithmParams");
    return false;
}

void NetworkPNL::GetAlgorithmParams(int algorithm, std::vector<double> &params)
{
    MarkCallFunction("GetAlgorithmParams");
}

bool NetworkPNL::ValidateAlgorithmParams(int algorithm, const std::vector<double> &params)
{
    MarkCallFunction("ValidateAlgorithmParams");
    return false;
}

void NetworkPNL::GetDefaultAlgorithmParams(int algorithm, std::vector<double> &params)
{
    MarkCallFunction("GetDefaultAlgorithmParams");
}

bool NetworkPNL::GetUpdateImmediately()
{
    MarkCallFunction("GetUpdateImmediately");
    return true;
}

void NetworkPNL::SetUpdateImmediately(bool immediate)
{
    MarkCallFunction("SetUpdateImmediately");
}

int NetworkPNL::GetNumberOfSamples()
{
    MarkCallFunction("GetNumberOfSamples");
    return 0;
}

void NetworkPNL::SetNumberOfSamples(int samples)
{
    MarkCallFunction("SetNumberOfSamples");
}

void NetworkPNL::GetNetworkProperties(PropertyMap &mp)
{
    MarkCallFunction("GetNetworkProperties", true);
    TranslatePropertyFromReal(&mp, m_NetProperty);
}

void NetworkPNL::SetNetworkProperties(const PropertyMap &mp)
{
    MarkCallFunction("SetNetworkProperties", true);
    TranslatePropertyToReal(&m_NetProperty, mp);
}

void NetworkPNL::InvalidateValues()
{
    MarkCallFunction("InvalidateValues");
}

void NetworkPNL::ClearAllEvidence()
{
    MarkCallFunction("ClearAllEvidence", true);
    m_pWNet->ClearEvid();
//    m_aEvidence->Clear();
}

void NetworkPNL::ClearAllDecisions()
{
    MarkCallFunction("ClearAllDecisions");
}

void NetworkPNL::ClearAllTargets()
{
    MarkCallFunction("ClearAllTargets");
}

IDiagNetwork* NetworkPNL::CreateDiagNetwork()
{
    MarkCallFunction("CreateDiagNetwork",true);
    DiagNetworkPNL* Diagnosis = new DiagNetworkPNL();
    Diagnosis->SetNetwork(this);
    return Diagnosis;
}

void NetworkPNL::SetEntropyCostRatio(double alpha, double alphaMax)
{
    MarkCallFunction("SetEntropyCostRatio", true);
    m_entropy = alpha;
    m_entropyMax = alphaMax;
    return;
}

void NetworkPNL::SetDiagPreferences(unsigned int prefs)
{
    MarkCallFunction("SetDiagPreferences");
    m_diagpref = prefs;
}

unsigned int NetworkPNL::GetDiagPreferences()
{
    MarkCallFunction("GetDiagPreferences");
    return m_diagpref;
}

ISearchForOpportunities* NetworkPNL::CreateSfo()
{
    MarkCallFunction("CreateSfo");
    return 0;
}

bool NetworkPNL::Voi(int targetNode, std::vector<std::pair<int, double> > &observations)
{
    MarkCallFunction("Voi");
    return false;
}

bool NetworkPNL::Voi(
		      const std::vector<int> & chanceNodes, 
		      int decision, 
		      int pointOfView, 
		      std::vector<int> & parents,
		      std::vector<double> & values)
{
    MarkCallFunction("Voi");
    return false;
}

bool NetworkPNL::GenerateDataFile(const GenerateDataFileParams &params)
{
    MarkCallFunction("GenerateDataFile");
    return false;
}

bool NetworkPNL::AnnealedMap(
			      const AnnealedMapParams& params,
			      const std::vector<std::pair<int, int> > &evidence,
			      const std::vector<int> &mapNodes, 
			      std::vector<int> &mapStates,
			      double &probM1E, double &probE)
{
    MarkCallFunction("AnnealedMap");
    return false;
}

void NetworkPNL::GetActionsAndDecisions(std::vector<int> &actions, std::vector<int> &decisions)
{
    MarkCallFunction("GetActionsAndDecisions");
}

void NetworkPNL::GetNodes(std::vector<int> &nodes)
{
    MarkCallFunction("GetNodes", true);
    Vector<int> aNode;
    Graph().Names(&aNode);
    nodes.assign(aNode.begin(), aNode.end());
}

void NetworkPNL::GetTerminalUtilityMinMax(double &minUtility, double &maxUtility)
{
    MarkCallFunction("GetTerminalUtilityMinMax");
}

int NetworkPNL::AddNode(int nodeType, const char *nodeId)
{
    MarkCallFunction("AddNode", true);
    if(nodeType != NetConst::Chance)
    {
        MarkCallFunction("AddNode with unsupported node type", true);
	return -1;
    }

    int result = Token().AddNode(String(nodeId));

    m_cost.resize(m_cost.size() + 1);
    m_diagType.resize(m_diagType.size() + 1);
    m_ranked.resize(m_ranked.size() + 1);
    m_mandatory.resize(m_mandatory.size() + 1);
    m_defaultOutcome.resize(m_defaultOutcome.size() + 1, -1);
    m_faultOutcome.resize(m_faultOutcome.size() + 1);
    m_faultOutcome[m_faultOutcome.size()-1].resize(2);
    m_stateLabels.resize(m_stateLabels.size()+1);
    m_stateLabels[m_stateLabels.size()-1].resize(2);

    if(result >= m_aNodeProperty.size())
    {
	m_aNodeProperty.resize(result + 1);
	m_aNodeValueStatus.resize(result + 1);
//	m_abEvidence.resize(result + 1, false);
    }
    Distributions().Setup(result);
    m_aNodeValueStatus[result] = NetConst::NotUpdated;
    return result;
}

void NetworkPNL::DeleteNode(int node)
{
    MarkCallFunction("DeleteNode", true);
    Distributions().DropDistribution(node);
    m_aNodeProperty[node].clear();
    Token().DelNode(node);
}

bool NetworkPNL::AddArc(int nodeFrom, int nodeTo)
{
    MarkCallFunction("AddArc", true);
    Graph().AddArc(nodeFrom, nodeTo);
    std::vector<int> parents;
    GetParents(nodeTo, parents);
    Distributions().Setup(nodeTo);
    m_aNodeValueStatus[nodeTo] = NetConst::NotUpdated;
    return true;
}

void NetworkPNL::DeleteArc(int nodeFrom, int nodeTo)
{
    MarkCallFunction("DeleteArc", true);
    Graph().DelArc(nodeFrom, nodeTo);
    Distributions().Setup(nodeTo);
    m_aNodeValueStatus[nodeTo] = NetConst::NotUpdated;
}

void NetworkPNL::GetParents(int node, std::vector<int> &parents)
{
    MarkCallFunction("GetParents", true);
    pnl::intVector pnlParents;
    Graph().GetParents(&pnlParents, node);
    parents.assign(pnlParents.begin(), pnlParents.end());
}

int NetworkPNL::GetParentCount(int node)
{
    MarkCallFunction("GetParentCount", true);
    return Graph().nParent(node);
}

int NetworkPNL::FindParent(int node, int parentNode)
{
    MarkCallFunction("FindParent", true);
    pnl::intVector pnlParents;
    pnl::CGraph *pnlGraph = Graph().Graph();
    pnlGraph->GetParents(Graph().IGraph(node), &pnlParents);
    pnlParents[0] = pnlParents[parentNode];
    pnlParents.resize(1);
    Graph().IndicesGraphToOuter(&pnlParents, &pnlParents);
    return pnlParents[0];
}

void NetworkPNL::GetChildren(int node, std::vector<int> &children)
{
    MarkCallFunction("GetChildren", true);
    pnl::intVector pnlChildren;

    Graph().GetChildren(&pnlChildren, node);
    children.assign(pnlChildren.begin(), pnlChildren.end());
}

int NetworkPNL::GetChildrenCount(int node)
{
    MarkCallFunction("GetChildrenCount", true);
    return Graph().nChild(node);
}

int NetworkPNL::FindChild(int node, int childNode)
{
    MarkCallFunction("FindChild");
    pnl::intVector pnlChildren;
    pnl::CGraph *pnlGraph = Graph().Graph();
    pnlGraph->GetChildren(Graph().IGraph(node), &pnlChildren);
    pnlChildren[0] = pnlChildren[childNode];
    pnlChildren.resize(1);
    Graph().IndicesGraphToOuter(&pnlChildren, &pnlChildren);
    return pnlChildren[0];
}

int NetworkPNL::GetOutcomeCount(int node)
{
    MarkCallFunction("GetOutcomeCount", true, (String() << "node #" << node).c_str());
    
    Vector<String> aValue;
    Token().GetValues(node, aValue);
    MarkCallFunction("GetOutcomeCount", true, (String() << "size " << int(aValue.size())).c_str());
    return aValue.size();
}

const char* NetworkPNL::GetOutcomeId(int node, int outcomeIndex)
{
    MarkCallFunction("GetOutcomeId", true);
    m_Bad.assign(Token().Value(node, outcomeIndex).c_str());
    return m_Bad.c_str();// may bring to inconsistent state if pointer will saved!!
}

const char* NetworkPNL::GetOutcomeLabel(int node, int outcomeIndex)
{
    MarkCallFunction("GetOutcomeLabel unfin x", true);
    
    return m_stateLabels[node][outcomeIndex].c_str();// may bring to inconsistent state if pointer will saved!!
}

void NetworkPNL::GetDefinition(int node, std::vector<double> &definition)
{
    MarkCallFunction("GetDefinition unfin x", true);
    WDistribFun *pDF = Distributions().Distribution(node);
    pnl::CDenseMatrix<float> *mat = pDF->Matrix(pnl::matTable);
    const float *pVal;
    int len;
    
    mat->GetRawData(&len, &pVal);
    definition.resize(len);
    for(int i = len; --i >= 0; definition[i] = pVal[i]);
    MarkCallFunction("GetDefinition unfin x", true, (String() << node << "#" << len).c_str());
}

void NetworkPNL::GetValue(int node, bool &valueValid, std::vector<int> &parents, std::vector<double> &values)
{
    MarkCallFunction("GetValue unfin x", true);
    int len = GetOutcomeCount(node);
    MarkCallFunction("GetValue unfin x", true, (String() << node << "#" << len).c_str());

    TokArr evid(m_pWNet->Net().EvidenceBoard()->Get());
    parents.resize(0);
    values.resize(0);
    m_pWNet->ClearEvid();
    if(evid.size())
    {
        if ( m_pWNet->Net().nNetNode() > 1)
        {
            m_pWNet->EditEvidence(evid);
            evid = m_pWNet->GetJPD(Graph().NodeName(node));
        }
        else
        {
    	    valueValid = false;
	        return;
        }
    }
    else
    {
        evid = m_pWNet->GetJPD(Graph().NodeName(node));
    }
    valueValid = true;
    values.resize(len);

    Vector<int> aiNode, aiValue;

    String x;
    int i;

    x = String(evid);

    //    Token().Resolve(evid);
    Net().ExtractTokArr(evid, &aiNode, &aiValue);
    for(i = evid.size(); --i >= 0;)
    {
	if(aiValue[0] >= len || aiValue[0] < 0)
	{
	    ThrowInternalError("outcome index exceed limit", "GetValue");
	    valueValid = false;
	    return;
	}
	values[aiValue[i]] = evid[i].FltValue();
    }
}

int NetworkPNL::GetValueParentCount(int node)
{
    MarkCallFunction("GetValueParentCount");
    return 0;
}

bool NetworkPNL::GetValueParents(int node, std::vector<int> &parents)
{
    MarkCallFunction("GetValueParents");
    return false;
}

int NetworkPNL::GetEvidence(int node)
{
    MarkCallFunction("GetEvidence", true);
    Vector<int> aiNode, aiValue;
    if (m_pWNet->Net().EvidenceBoard()->IsEmpty())
        return -1;
    if (!(m_pWNet->Net().EvidenceBoard()->IsNodeHere(Tok(m_pWNet->Net().Token().Node(node)))) || 
        m_pWNet->Net().EvidenceBoard()->IsHidden(Tok(m_pWNet->Net().Token().Node(node))))
        return -1;
    Net().ExtractTokArr(m_pWNet->Net().EvidenceBoard()->Get(), &aiNode, &aiValue);
    for(int i = aiNode.size(); --i >= 0;)
    {
	if(aiNode[i] == node)
	{
	    return aiValue[i];
	}
    }

    ThrowInternalError("Not found evidence", "GetEvidence");// must be here, but ...
    return -1;
}

int NetworkPNL::GetControlledValue(int node)
{
    MarkCallFunction("GetControlledValue", true, (String() << node).c_str());
    return -1;
}

int NetworkPNL::GetNodeType(int node)
{
    MarkCallFunction("GetNodeType unfin(Chance only) x", true, (String() << node).c_str());
    return NetConst::Chance;// rough!
}

bool NetworkPNL::IsIdUnique(const char *nodeId, int nodeToIgnore)
{
    MarkCallFunction("IsIdUnique", true, (String() << nodeId).c_str());
    Vector<String> aName(Graph().Names());
    String name(nodeId);

    for(int i = aName.size(); --i >= 0;)
    {
	if((i != nodeToIgnore) && (aName[i] == name))
	{
	    return false;
	}
    }
    return true;
}

int NetworkPNL::FindNode(const char * id)
{
    MarkCallFunction("FindNode", true, id);
    return Graph().INode(id);
}

const char* NetworkPNL::GetNodeId(int node)
{
    MarkCallFunction("GetNodeId", true);
    return Graph().NodeName(node).c_str();
}

bool NetworkPNL::IsTarget(int node)
{
    MarkCallFunction("IsTarget", true);
    return false;
}

bool NetworkPNL::IsEvidence(int node)
{
    MarkCallFunction("IsEvidence", true);
    return IsRealEvidence(node);
}

bool NetworkPNL::IsRealEvidence(int node)
{
    MarkCallFunction("IsRealEvidence", true, (String() << "Node #" << node).c_str());
    return (m_pWNet->Net().EvidenceBoard()->IsNodeHere(Tok(m_pWNet->Net().Token().Node(node))) && !m_pWNet->Net().EvidenceBoard()->IsHidden(Tok(m_pWNet->Net().Token().Node(node))));
}


bool NetworkPNL::IsPropagatedEvidence(int node)
{
    MarkCallFunction("IsPropagatedEvidence", true);
    return false;
}

bool NetworkPNL::IsValueValid(int node)
{
    MarkCallFunction("IsValueValid", true);
    return m_aNodeValueStatus[node] == NetConst::Updated;
}

bool NetworkPNL::IsControlled(int node)
{
    MarkCallFunction("IsControlled", true);
    return false;
}

int NetworkPNL::GetNodeValueStatus(int node)
{
    MarkCallFunction("GetNodeValueStatus", true, (String() << "node #" << node).c_str());
    return m_aNodeValueStatus[node];
}

bool NetworkPNL::GetMinMaxUtility(int node, double &minUtility, double &maxUtility)
{
    MarkCallFunction("GetMinMaxUtility");
    return false;
}

void NetworkPNL::GetProperties(int node, PropertyMap &mp)
{
    MarkCallFunction("GetProperties", true);
    TranslatePropertyFromReal(&mp, m_aNodeProperty.at(node));
}

void NetworkPNL::SetProperties(int node, const PropertyMap &mp)
{
    MarkCallFunction("SetProperties", true);
    TranslatePropertyToReal(&m_aNodeProperty.at(node), mp);
}

void NetworkPNL::ObfuscateDefinition(int node, int algorithm, double param)
{
    MarkCallFunction("ObfuscateDefinition", false,
	(String()<< "Node #" << node << "; algo #" << algorithm << " ;param=" << param).c_str());
}

bool NetworkPNL::SetNodeId(int node, const char *id)
{
    String newId(id);

    newId << " - new name for node #" << node;
    MarkCallFunction("SetNodeId", true, newId.c_str());
    return Graph().SetNodeName(node, String(id));
}

void NetworkPNL::SetNodeType(int node, int type)
{
    MarkCallFunction("SetNodeType");
}

void NetworkPNL::AddOutcome(int node, int outcomeIndex, const char *outcomeId)
{
    MarkCallFunction("AddOutcome");
}

void NetworkPNL::DeleteOutcome(int node, int outcomeIndex)
{
    MarkCallFunction("DeleteOutcome");
}

void NetworkPNL::SetDefinition(int node, const std::vector<double> & definition)
{
    MarkCallFunction("SetDefinition unfin x", true);
    WDistribFun *pD = Distributions().Distribution(node);
    pnl::CDenseMatrix<float> *m = static_cast<pnl::CDenseMatrix<float> *>(pD->Matrix(pnl::matTable));
    Vector<float> def;
    int i = definition.size();

    if(m->GetRawDataLength() != i)
    {
	MarkCallFunction("SetDefinition bad call", true);
	return;
    }
    def.resize(i);
    for(; --i >= 0; def[i] = definition[i]);

    m->SetData(&def.front());
    m_aNodeValueStatus[node] = NetConst::NotUpdated;
}

bool NetworkPNL::SetOutcomeId(int node, int outcomeIndex, const char *id)
{
    MarkCallFunction("SetOutcomeId unchecked unfin x", true);
    Token().SetValue(node, outcomeIndex, String(id));
    return true;
}

void NetworkPNL::SetOutcomeIds(int node, const std::vector<std::string> &ids)
{
    MarkCallFunction("SetOutcomeIds unchecked unfin x", true);

    SetValues(node, ids);
}

bool NetworkPNL::SetOutcomeLabel(int node, int outcomeIndex, const char *label)
{
    MarkCallFunction("SetOutcomeLabel",true);
    m_stateLabels[node][outcomeIndex] = (std::string)label;
    return true;
}

void NetworkPNL::SetOutcomeLabels(int node, const std::vector<std::string> &labels)
{
    MarkCallFunction("SetOutcomeLabels");
    m_stateLabels[node].clear();
    m_stateLabels[node].insert(m_stateLabels[node].begin(), 
        labels.begin(), labels.begin()+labels.size());
}

void NetworkPNL::SetEvidence(int node, int outcomeIndex)
{
    MarkCallFunction("SetEvidence", true, (String() << "Node #" << node).c_str());
    m_pWNet->EditEvidence(Tok(Graph().NodeName(node))^outcomeIndex);
    m_aNodeValueStatus[node] = NetConst::Updated;
}

void NetworkPNL::ClearEvidence(int node)
{
    MarkCallFunction("ClearEvidence", true, (String() << "Node #" << node).c_str());
    m_pWNet->Net().EvidenceBoard()->SetVisibility(Tok(m_pWNet->Net().Token().Node(node)),true);
    m_aNodeValueStatus[node] = NetConst::NotUpdated;
}

void NetworkPNL::SetControlledValue(int node, int outcomeIndex)
{
    MarkCallFunction("SetControlledValue");
    m_aNodeValueStatus[node] = NetConst::Updated;
}

void NetworkPNL::ClearControlledValue(int node)
{
    MarkCallFunction("ClearControlledValue");
    m_aNodeValueStatus[node] = NetConst::NotUpdated;
}

bool NetworkPNL::IsControllable(int node)
{
    MarkCallFunction("IsControllable", true);
    return false;
}

void NetworkPNL::SetTarget(int node, bool asTarget)
{
    MarkCallFunction("SetTarget");
}

void NetworkPNL::NoisyToCpt(
			     int node, 
			     int outcomeCount, 
			     const std::vector<int> &parentPermutation,
			     const std::vector<int> &noisyMaxStrengths,
			     const std::vector<double> &noisyAdderWeights,
			     const std::vector<int> &noisyAdderDistStates,
			     int adderFunction,
			     const std::vector<double> &noisy, 
			     std::vector<double> &cpt)
{
    MarkCallFunction("NoisyToCpt");
}

void NetworkPNL::NoisyToHenrion(
				 int node,
				 int outcomeCount,
				 const std::vector<int> &parentPermutation,
				 const std::vector<double> &noisy, 
				 std::vector<double> &henrion)
{
    MarkCallFunction("NoisyToHenrion");
}

void NetworkPNL::NoisyFromHenrion(
				   int node,
				   int outcomeCount,
				   const std::vector<int> &parentPermutation,
				   const std::vector<double> &henrion, 
				   std::vector<double> &noisy)
{
    MarkCallFunction("NoisyFromHenrion");
}

// noisymax
void NetworkPNL::NoisyGetParentStrengths(int node, int parentIndex, std::vector<int> &strengths)
{
    MarkCallFunction("NoisyGetParentStrengths");
}

void NetworkPNL::NoisySetParentStrengths(int node, int parentIndex, const std::vector<int> &strengths)
{
    MarkCallFunction("NoisySetParentStrengths");
}

// noisyadder
void NetworkPNL::NoisyGetDistStates(int node, std::vector<int> &distStates)
{
    MarkCallFunction("NoisyGetDistStates");
}

void NetworkPNL::NoisySetDistStates(int node, const std::vector<int> &distStates)
{
    MarkCallFunction("NoisySetDistStates");
}

void NetworkPNL::NoisyGetWeights(int node, std::vector<double> &weights)
{
    MarkCallFunction("NoisyGetWeights");
}

void NetworkPNL::NoisySetWeights(int node, const std::vector<double> &weights)
{
    MarkCallFunction("NoisySetWeights");
}

int NetworkPNL::NoisyGetFunction(int node)
{
    MarkCallFunction("NoisyGetFunction");
    return 0;
}

void NetworkPNL::NoisySetFunction(int node, int function)
{
    MarkCallFunction("NoisySetFunction");
}


// equation-related functions
bool NetworkPNL::SetEquation(int node, const char *equation)
{
    MarkCallFunction("SetEquation");
    return false;
}

const char* NetworkPNL::GetEquation(int node)
{
    MarkCallFunction("GetEquation");
    return "";
}

bool NetworkPNL::ValidateEquation(int node, const char *alternateId, const char *equation, std::string &errMsg)
{
    MarkCallFunction("ValidateEquation");
    return false;
}

bool NetworkPNL::ReplaceVariableEquation(std::string &equation, const char *oldVar, const char *newVar)
{
    MarkCallFunction("ReplaceVariableEquation");
    return false;
}


// copy/paste and drag'n'drop support
bool NetworkPNL::CopyNodes(const std::vector<int> &sourceNodes, std::vector<int> &copiedNodes, const std::vector<std::string> &externalIds)
{
    MarkCallFunction("CopyNodes");
    return false;
}

bool NetworkPNL::CopyNodesFrom(const INetwork* sourceNetwork, const std::vector<int> &sourceNodes, std::vector<int> &copiedNodes, const std::vector<std::string> &externalIds)
{
    MarkCallFunction("CopyNodesFrom");
    return false;
}

bool NetworkPNL::Serialize(
			    ISerializer *serializer,
			    const std::vector<int> &nodes,
			    const std::vector<std::pair<int, UUID> > &uuids)
{
    MarkCallFunction("Serialize");
    return false;
}

bool NetworkPNL::Deserialize(
			      ISerializer *serializer, bool native,
			      std::vector<std::pair<int, int> > &createdNodes,
			      const std::vector<std::pair<int, UUID> > &uuids,
			      const std::vector<std::string> &externalIds)
{
    MarkCallFunction("Deserialize");
    return false;
}


// reordering parents
void NetworkPNL::ReorderParents(int node, const std::vector<int> & newOrder)
{
    MarkCallFunction("ReorderParents");
}


// syncing outcomes with definition grid;
// permutation contain old indices and -1 instead of new indices
void NetworkPNL::AddRemoveReorderOutcomes(
					   int node, 
					   const std::vector<std::string> &outcomeIds,
					   const std::vector<int> &permutation)
{
    MarkCallFunction("AddRemoveReorderOutcomes permutation not used x", true, (String() << node).c_str());

    int nValue, i;

    nValue = outcomeIds.size();
    for(i = 0; i < nValue; ++i)
    {
#if 0
	MarkCallFunction("AddRemoveReorderOutcomes: index", true,
	    (String() << permutation[i] << " from " << int(permutation.size())).c_str());
#endif
    }
    SetValues(node, outcomeIds);
    m_aNodeValueStatus[node] = NetConst::Updated;
}

void NetworkPNL::SetValues(int iNode, const std::vector<std::string> &aId)
{
    Vector<String> aIdNew, aIdOld;
    int nValue, i;

    nValue = aId.size();
    aIdNew.resize(nValue);
    for(i = 0; i < nValue; ++i)
    {
        aIdNew[i] = aId[i];
    }
    Token().GetValues(iNode, aIdOld);
    Token().SetValues(iNode, aIdNew);
    m_aNodeValueStatus[iNode] = NetConst::NotUpdated;

    int nSize = m_faultOutcome[iNode].size();
    if (nValue > nSize)
        m_faultOutcome[iNode].push_back(0);
    else 
        m_faultOutcome[iNode].pop_back();

    if(aIdOld.size() != aIdNew.size())
    {
	Distributions().Setup(iNode);
	std::vector<int> aChild;
	GetChildren(iNode, aChild);

	for(i = aChild.size(); --i >= 0;)
	{
	    Distributions().Setup(aChild[i]);
	}
    }
}

ProbabilisticNet &NetworkPNL::Net() const
{
    return m_pWNet->Net();
}

WGraph &NetworkPNL::Graph() const
{
    return Net().Graph();
}

TokenCover &NetworkPNL::Token() const
{
    return Net().Token();
}

WDistributions &NetworkPNL::Distributions() const
{
    return Net().Distributions();
}

// qualitative stuff
double NetworkPNL::QualGetWeight(int node, int parentIndex, bool positive)
{
    MarkCallFunction("QualGetWeight");
    return 0;
}

void NetworkPNL::QualSetWeight(int node, int parentIndex, bool positive, double weight)
{
    MarkCallFunction("QualSetWeight");
}

double NetworkPNL::QualGetBaseline(int node)
{
    MarkCallFunction("QualGetBaseline");
    return 0;
}

void NetworkPNL::QualSetBaseline(int node, double baseline)
{
    MarkCallFunction("QualSetBaseline");
}

double NetworkPNL::QualGetValue(int node)
{
    MarkCallFunction("QualGetValue");
    return 0;
}

int NetworkPNL::QualGetArcSign(int node, int parentIndex)
{
    MarkCallFunction("QualGetArcSign");
    return 0;
}

// diagnosis support
bool NetworkPNL::AddCostArc(int nodeFrom, int nodeTo)
{
    MarkCallFunction("AddCostArc");
    return false;
}

void NetworkPNL::DeleteCostArc(int nodeFrom, int nodeTo)
{
    MarkCallFunction("DeleteCostArc");
}

void NetworkPNL::GetCostParents(int node, std::vector<int> &parents)
{
    MarkCallFunction("GetCostParents");
}

int NetworkPNL::GetCostParentCount(int node)
{
    MarkCallFunction("GetCostParentCount");
    return 0;
}

int NetworkPNL::FindCostParent(int node, int parentNode)
{
    MarkCallFunction("FindCostParent");
    return 0;
}

void NetworkPNL::GetCostChildren(int node, std::vector<int> &children)
{
    MarkCallFunction("GetCostChildren");
}

int NetworkPNL::GetCostChildrenCount(int node)
{
    MarkCallFunction("GetCostChildrenCount");
    return 0;
}

int NetworkPNL::FindCostChild(int node, int childNode)
{
    MarkCallFunction("FindCostChild");
    return 0;
}

void NetworkPNL::GetCost(int node, std::vector<double> &cost)
{

    MarkCallFunction("GetCost",true);
    cost.resize(1);
    cost[0] = m_cost[node];
}

void NetworkPNL::SetCost(int node, const std::vector<double> &cost)
{
    MarkCallFunction("SetCost",true);
    m_cost[node] = cost[0];  
}

double NetworkPNL::GetNotAvailableCostValue()
{
    MarkCallFunction("GetNotAvailableCostValue",true);
    return 8.0e-308;
}

double NetworkPNL::GetNotRelevantTestValue()
{
    MarkCallFunction("GetNotRelevantTestValue");
    return 8.0e-308;
}

void NetworkPNL::GetGroupCostProperty(std::string &name, std::string &value)
{
    MarkCallFunction("GetGroupCostProperty");
}

int NetworkPNL::GetDiagType(int node)
{
    MarkCallFunction("GetDiagType",true);
    
    return m_diagType[node];
}

void NetworkPNL::SetDiagType(int node, int diagType)
{
    MarkCallFunction("SetDiagType",true);
    m_diagType[node] = diagType;
}

void NetworkPNL::SetRanked(int node, bool ranked)
{
    MarkCallFunction("SetRanked",true);
    m_ranked[node] = (int)ranked;
}

bool NetworkPNL::IsRanked(int node)
{
    MarkCallFunction("IsRanked NS x", true);

    return (bool)m_ranked[node];
}

void NetworkPNL::SetMandatory(int node, bool mandatory)
{
    MarkCallFunction("SetMandatory",true);
    m_mandatory[node] = (int)mandatory;
}

bool NetworkPNL::IsMandatory(int node)
{
    MarkCallFunction("IsMandatory NS x", true);
    return (bool)m_mandatory[node];
}

int NetworkPNL::GetDefaultOutcome(int node)
{
    MarkCallFunction("GetDefaultOutcome",true);
    return m_defaultOutcome[node];
}

void NetworkPNL::SetDefaultOutcome(int node, int outcome)
{
    MarkCallFunction("SetDefaultOutcome",true);
    m_defaultOutcome[node] = outcome;
}

bool NetworkPNL::IsFaultOutcome(int node, int outcome)
{
    MarkCallFunction("IsFaultOutcome",true);
    if (node >= m_faultOutcome.size())
        return false;
    if (outcome >= m_faultOutcome[node].size())
        return false;
    return (bool)m_faultOutcome[node][outcome];
}

void NetworkPNL::SetFaultOutcome(int node, int outcome, bool fault)
{
    MarkCallFunction("SetFaultOutcome",true);
    m_faultOutcome[node][outcome] = (int)fault;	
}


bool NetworkPNL::LearnStructureAndParams(const char *dataFile, const char *networkFile)
{
    MarkCallFunction("LearnStructureAndParams");
    return false;
}


BayesNet *NetworkPNL::GetBayesNet()
{
    return m_pWNet;
}

void NetworkPNL::MarkCallFunction(const char *name, bool bRealized, const char *args)
{
    if(m_pLog == 0)
    {
	return;
    }
    pnl::Log &l = *m_pLog;
    l << "Function '" << name << "'";
    if(args)
    {
	l << ", arguments = '" << args << "'";
    }
    l << ((bRealized) ? " realized\n":" not yet realized\n");
    if(!bRealized && false)
    {
	m_pLogStream->Redirect("GeNIe_PNLModule2.log");
	delete m_pLogStream;
	m_pLogStream = 0;
	delete m_pLog;
	m_pLog = 0;
	pnl::pnlString s;
	s << "Function '" << name << "' not yet implemented";
	PNL_THROW(pnl::CNotImplemented, s.c_str());
    }
}
