#undef _DEBUG
#undef DEBUG
#include "xmlreader.h"

#include "networkPNLH.hpp"
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
#include "pnlPersistence.hpp"
#include "pnlXMLRead.hpp"

void NetworkPNL::Delete()
{
    MarkCallFunction("Delete", true);
    delete m_pWNet;
    delete m_pLog;
    delete m_pLogStream;
    delete m_aEvidence;
}

NetworkPNL::NetworkPNL(): m_ErrorOutput(0), m_NetName("PNLBNet")
{
    m_pWNet = new BayesNet;
    m_aNodeProperty.reserve(40);
    m_pLogStream = new pnl::LogDrvStream("GeNIe_PNLModule.log", pnl::eLOG_ALL, pnl::eLOGSRV_ALL);
    m_pLog = new pnl::Log("", pnl::eLOG_DEBUG|pnl::eLOG_NOTICE|pnl::eLOG_INFO, pnl::eLOGSRV_TEST1);
    (*m_pLog) << "Ok, Started!\n";
    m_aEvidence = new WEvidence;
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
    m_pWNet->LoadNet(filename);
    {
	int nNode = m_pWNet->Net().nNetNode();
	m_NetName = "NetWoRk"; // stub
	m_aNodeProperty.assign(nNode, PropertyMap());
	m_aNodeValueStatus.assign(nNode, NetConst::NotUpdated);
	m_aEvidence->Clear();// stub ?
	m_abEvidence.assign(nNode, false);// same as previous
    }

    return reader.Parse(container, externalBinding);
}

// FORWARDS
bool NetworkPNL::Save(const char *filename, IXmlWriterExtension *externalExtension,
		       const LegacyDslFileInfo *gnet)
{
    MarkCallFunction("Save", true);
    static const char fname[] = "Save";
    pnl::CContextPersistence saver;
    pnl::CXMLWriterStd writer;

    if(!writer.OpenFile(filename))
    {
	ThrowUsingError("can't open file", fname);
    }
    if(!m_pWNet->Net().SaveNet(&saver))
    {
	ThrowInternalError("Can't save file", fname);
    }

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

static void LoadAttr(std::string *result, pnl::CContextLoad &context, const char *name)
{
    pnl::pnlString str;

    context.GetAttribute(str, name);
    result->assign(str.c_str(), str.c_str() + str.length());
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
    //    MarkCallFunction("GetUpdateImmediately");
    return false;
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
    mp = m_NetProperty;
}

void NetworkPNL::SetNetworkProperties(const PropertyMap &mp)
{
    MarkCallFunction("SetNetworkProperties", true);
    m_NetProperty = mp;
}

void NetworkPNL::InvalidateValues()
{
    MarkCallFunction("InvalidateValues");
}

void NetworkPNL::ClearAllEvidence()
{
    MarkCallFunction("ClearAllEvidence", true);
    m_aEvidence->Clear();
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
    MarkCallFunction("CreateDiagNetwork");
    return 0;
}

void NetworkPNL::SetEntropyCostRatio(double alpha, double alphaMax)
{
    MarkCallFunction("SetEntropyCostRatio");
    return;
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
    Graph()->Names(&aNode);
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
    int result = Token()->AddNode(String(nodeId));

    if(result >= m_aNodeProperty.size())
    {
	m_aNodeProperty.resize(result + 1);
	m_aNodeValueStatus.resize(result + 1);
	m_abEvidence.resize(result + 1);
    }
    Distributions()->Setup(result);
    m_aNodeValueStatus[result] = NetConst::NotUpdated;
    return result;
}

void NetworkPNL::DeleteNode(int node)
{
    MarkCallFunction("DeleteNode", true);
    Distributions()->DropDistribution(node);
    m_aNodeProperty[node].resize(0);
    Token()->DelNode(node);
}

bool NetworkPNL::AddArc(int nodeFrom, int nodeTo)
{
    MarkCallFunction("AddArc", true);
    Graph()->AddArc(nodeFrom, nodeTo);
    std::vector<int> parents;
    GetParents(nodeTo, parents);
    Distributions()->Setup(nodeTo);
    m_aNodeValueStatus[nodeTo] = NetConst::NotUpdated;
    return true;
}

void NetworkPNL::DeleteArc(int nodeFrom, int nodeTo)
{
    MarkCallFunction("DeleteArc", true);
    Graph()->DelArc(nodeFrom, nodeTo);
    Distributions()->Setup(nodeTo);
    m_aNodeValueStatus[nodeTo] = NetConst::NotUpdated;
}

void NetworkPNL::GetParents(int node, std::vector<int> &parents)
{
    MarkCallFunction("GetParents", true);
    pnl::intVector pnlParents;
    Graph()->GetParents(&pnlParents, node);
    parents.assign(pnlParents.begin(), pnlParents.end());
}

int NetworkPNL::GetParentCount(int node)
{
    MarkCallFunction("GetParentCount", true);
    return Graph()->nParent(node);
}

int NetworkPNL::FindParent(int node, int parentNode)
{
    MarkCallFunction("FindParent", true);
    pnl::intVector pnlParents;
    pnl::CGraph *pnlGraph = Graph()->Graph();
    pnlGraph->GetParents(Graph()->IGraph(node), &pnlParents);
    pnlParents[0] = pnlParents[parentNode];
    pnlParents.resize(1);
    Graph()->IndicesGraphToOuter(&pnlParents, &pnlParents);
    return pnlParents[0];
}

void NetworkPNL::GetChildren(int node, std::vector<int> &children)
{
    MarkCallFunction("GetChildren", true);
    pnl::intVector pnlChildren;

    Graph()->GetChildren(&pnlChildren, node);
    children.assign(pnlChildren.begin(), pnlChildren.end());
}

int NetworkPNL::GetChildrenCount(int node)
{
    MarkCallFunction("GetChildrenCount", true);
    return Graph()->nChild(node);
}

int NetworkPNL::FindChild(int node, int childNode)
{
    MarkCallFunction("FindChild");
    pnl::intVector pnlChildren;
    pnl::CGraph *pnlGraph = Graph()->Graph();
    pnlGraph->GetChildren(Graph()->IGraph(node), &pnlChildren);
    pnlChildren[0] = pnlChildren[childNode];
    pnlChildren.resize(1);
    Graph()->IndicesGraphToOuter(&pnlChildren, &pnlChildren);
    return pnlChildren[0];
}

int NetworkPNL::GetOutcomeCount(int node)
{
    MarkCallFunction("GetOutcomeCount", true, (String() << "node #" << node).c_str());
    
    Vector<String> aValue;
    Token()->GetValues(node, aValue);
    MarkCallFunction("GetOutcomeCount", true, (String() << "size " << int(aValue.size())).c_str());
    return aValue.size();
}

const char* NetworkPNL::GetOutcomeId(int node, int outcomeIndex)
{
    MarkCallFunction("GetOutcomeId", true);
    m_Bad.assign(Token()->Value(node, outcomeIndex).c_str());
    return m_Bad.c_str();// may bring to inconsistent state if pointer will saved!!
}

const char* NetworkPNL::GetOutcomeLabel(int node, int outcomeIndex)
{
    MarkCallFunction("GetOutcomeLabel unfin x", true);
    m_Bad.assign(Token()->Value(node, outcomeIndex).c_str());
    return m_Bad.c_str();// may bring to inconsistent state if pointer will saved!!
}

void NetworkPNL::GetDefinition(int node, std::vector<double> &definition)
{
    MarkCallFunction("GetDefinition unfin x", true);
    WDistribFun *pDF = Distributions()->Distribution(node);
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

    TokArr evid(m_aEvidence->GetBoard());
    parents.resize(0);
    values.resize(0);
    m_pWNet->ClearEvid();
    if(evid.size() && m_pWNet->Net().nNetNode() > 1)
    {
	m_pWNet->EditEvidence(evid);
    }
    else
    {
	valueValid = false;
	return;
    }
    valueValid = true;
    evid = m_pWNet->GetJPD(Graph()->NodeName(node));
    values.resize(len);

    Vector<int> aiNode, aiValue;

    String xx, x;
    int i;
    for(i = 0; i < evid.size(); ++i)
    {
	Vector< std::deque< TokId > > unres = evid[i].unres;

	for(int j = 0; j < unres.size(); ++j)
	{
	    for(int k = 0; k < unres[j].size(); ++k)
	    {
		xx << String(unres[j][k]) << " ";
	    }
	    xx << "\n";
	}
    }

    x = String(evid);

    //    Token()->Resolve(evid);
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

    if(!m_abEvidence[node])
    {
	return -1;
    }

    Vector<int> aiNode, aiValue;
    Net().ExtractTokArr(m_aEvidence->GetBoard(), &aiNode, &aiValue);
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
    Vector<String> aName(Graph()->Names());
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
    return Graph()->INode(id);
}

const char* NetworkPNL::GetNodeId(int node)
{
    MarkCallFunction("GetNodeId", true);
    return Graph()->NodeName(node).c_str();
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
    return m_abEvidence[node];
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
    mp = m_aNodeProperty.at(node);
}

void NetworkPNL::SetProperties(int node, const PropertyMap &mp)
{
    MarkCallFunction("SetProperties", true);
    m_aNodeProperty.at(node) = mp;
}

bool NetworkPNL::SetNodeId(int node, const char *id)
{
    String newId(id);

    newId << " - new name for node #" << node;
    MarkCallFunction("SetNodeId", true, newId.c_str());
    return Graph()->SetNodeName(node, String(id));
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
    WDistribFun *pD = Distributions()->Distribution(node);
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
    Token()->SetValue(node, outcomeIndex, String(id));
    return true;
}

void NetworkPNL::SetOutcomeIds(int node, const std::vector<std::string> &ids)
{
    MarkCallFunction("SetOutcomeIds unchecked unfin x", true);

    SetValues(node, ids);
}

bool NetworkPNL::SetOutcomeLabel(int node, int outcomeIndex, const char *label)
{
    MarkCallFunction("SetOutcomeLabel");
    return false;
}

void NetworkPNL::SetOutcomeLabels(int node, const std::vector<std::string> &labels)
{
    MarkCallFunction("SetOutcomeLabels");
}

void NetworkPNL::SetEvidence(int node, int outcomeIndex)
{
    MarkCallFunction("SetEvidence", true, (String() << "Node #" << node).c_str());

#if 0 // resolving doesn't work for 0^0
    m_aEvidence->Set(Tok(node) ^ outcomeIndex);
#else
    m_aEvidence->Set(Tok(Graph()->NodeName(node)) ^ outcomeIndex);
#endif
    m_abEvidence[node] = true;
    m_aNodeValueStatus[node] = NetConst::Updated;
}

void NetworkPNL::ClearEvidence(int node)
{
    MarkCallFunction("ClearEvidence", true, (String() << "Node #" << node).c_str());
    m_abEvidence[node] = false;
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
    Token()->GetValues(iNode, aIdOld);
    Token()->SetValues(iNode, aIdNew);
    m_aNodeValueStatus[iNode] = NetConst::NotUpdated;
    if(aIdOld.size() != aIdNew.size())
    {
	Distributions()->Setup(iNode);
	std::vector<int> aChild;
	GetChildren(iNode, aChild);

	for(i = aChild.size(); --i >= 0;)
	{
	    Distributions()->Setup(aChild[i]);
	}
    }
}

ProbabilisticNet &NetworkPNL::Net() const
{
    return m_pWNet->Net();
}

WGraph *NetworkPNL::Graph() const
{
    return Net().Graph();
}

TokenCover *NetworkPNL::Token() const
{
    return Net().Token();
}

WDistributions *NetworkPNL::Distributions() const
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
    MarkCallFunction("GetCost");
}

void NetworkPNL::SetCost(int node, const std::vector<double> &cost)
{
    MarkCallFunction("SetCost");
}

double NetworkPNL::GetNotAvailableCostValue()
{
    MarkCallFunction("GetNotAvailableCostValue");
    return 0;
}

double NetworkPNL::GetNotRelevantTestValue()
{
    MarkCallFunction("GetNotRelevantTestValue");
    return 0;
}

void NetworkPNL::GetGroupCostProperty(std::string &name, std::string &value)
{
    MarkCallFunction("GetGroupCostProperty");
}

int NetworkPNL::GetDiagType(int node)
{
    MarkCallFunction("GetDiagType");
    return 0;
}

void NetworkPNL::SetDiagType(int node, int diagType)
{
    MarkCallFunction("SetDiagType");
}

void NetworkPNL::SetRanked(int node, bool ranked)
{
    MarkCallFunction("SetRanked");
}

bool NetworkPNL::IsRanked(int node)
{
    MarkCallFunction("IsRanked NS x", true);
    return false;
}

void NetworkPNL::SetMandatory(int node, bool mandatory)
{
    MarkCallFunction("SetMandatory");
}

bool NetworkPNL::IsMandatory(int node)
{
    MarkCallFunction("IsMandatory NS x", true);
    return false;
}

int NetworkPNL::GetDefaultOutcome(int node)
{
    MarkCallFunction("GetDefaultOutcome");
    return 0;
}

void NetworkPNL::SetDefaultOutcome(int node, int outcome)
{
    MarkCallFunction("SetDefaultOutcome");
}

bool NetworkPNL::IsFaultOutcome(int node, int outcome)
{
    MarkCallFunction("IsFaultOutcome");
    return false;
}

void NetworkPNL::SetFaultOutcome(int node, int outcome, bool fault)
{
    MarkCallFunction("SetFaultOutcome");
}


bool NetworkPNL::LearnStructureAndParams(const char *dataFile, const char *networkFile)
{
    MarkCallFunction("LearnStructureAndParams");
    return false;
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

    //m_pLogStream->Redirect("GeNIe_PNLModule.log");
}
