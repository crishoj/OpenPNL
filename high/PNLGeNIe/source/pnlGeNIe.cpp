#undef _DEBUG
#undef DEBUG
#include "xmlreader.h"

#include "pnlGeNIe.hpp"
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

void INetworkPNL::Delete()
{
    MarkCallFunction("Delete", true);
    delete m_pWNet;
    delete m_pLog;
    delete m_pLogStream;
    delete m_aEvidence;
}

INetworkPNL::INetworkPNL(): m_ErrorOutput(0), m_NetName("PNLBNet")
{
    m_pWNet = new BayesNet;
    m_aNodeProperty.reserve(40);
    m_pLogStream = new pnl::LogDrvStream("GeNIe_PNLModule.log", pnl::eLOG_ALL, pnl::eLOGSRV_ALL);
    m_pLog = new pnl::Log("", pnl::eLOG_DEBUG|pnl::eLOG_NOTICE|pnl::eLOG_INFO, pnl::eLOGSRV_TEST1);
    (*m_pLog) << "Ok, Started!\n";
    m_aEvidence = new WEvidence;
}

bool INetworkPNL::Load(const char *filename, IXmlBinding *externalBinding)
{
    MarkCallFunction("Load", true);

    XmlReaderPNLH reader;
    XMLContainer container;
    XMLContainerFilter filter(&container, "genie");

    return reader.Parse(container, externalBinding, 0);
}

#include "legacydsl.h"
#include "xmlwriter.h"
#include "pnlContextPersistence.hpp"
#include "pnlPersistence.hpp"
#include "pnlXMLRead.hpp"

// Persistence

// FORWARDS
class XmlWriterPNL: public IXmlWriter
{
public:
    XmlWriterPNL(pnl::CXMLWriter *writer): m_pRealWriter(writer) {}
    virtual bool OpenFile(const char * filename);
    virtual bool CloseFile();

    virtual void OpenElement(const char * name);
    virtual void CloseElement(const char * name);

    virtual void WriteElement(
	const char * name, 
	const char * content = NULL, 
	bool escapeWhitespace = false);

    virtual void PushAttribute(const char *name, const char * value);
    virtual void PushAttribute(const char *name, int value);
    virtual void PushAttribute(const char *name, bool value);
    virtual void PushAttribute(const char *name, double value);

    virtual void Delete() { delete this; }

private:
    pnl::CXMLWriter *m_pRealWriter;
};

bool INetworkPNL::Save(const char *filename, IXmlWriterExtension *externalExtension,
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

    XmlWriterPNL genieWriter(&writer);
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

bool INetworkPNL::UpdateBeliefs()
{
    MarkCallFunction("UpdateBeliefs", true);
    for(int i = m_aNodeValueStatus.size(); --i >= 0; m_aNodeValueStatus[i] = NetConst::Updated);
    return true;
}
    
void INetworkPNL::SetBnAlgorithm(int algorithm)
{
    MarkCallFunction("SetBnAlgorithm");
}

int INetworkPNL::GetBnAlgorithm()
{
    MarkCallFunction("GetBnAlgorithm");
    return 0;
}

void INetworkPNL::SetIdAlgorithm(int algorithm)
{
    MarkCallFunction("SetIdAlgorithm");
}

int INetworkPNL::GetIdAlgorithm()
{
    MarkCallFunction("GetIdAlgorithm");
    return 0;
}

bool INetworkPNL::SetAlgorithmParams(int algorithm, const std::vector<double> &params)
{
    MarkCallFunction("SetAlgorithmParams");
    return false;
}

void INetworkPNL::GetAlgorithmParams(int algorithm, std::vector<double> &params)
{
    MarkCallFunction("GetAlgorithmParams");
}

bool INetworkPNL::ValidateAlgorithmParams(int algorithm, const std::vector<double> &params)
{
    MarkCallFunction("ValidateAlgorithmParams");
    return false;
}

void INetworkPNL::GetDefaultAlgorithmParams(int algorithm, std::vector<double> &params)
{
    MarkCallFunction("GetDefaultAlgorithmParams");
}

bool INetworkPNL::GetUpdateImmediately()
{
    //    MarkCallFunction("GetUpdateImmediately");
    return false;
}

void INetworkPNL::SetUpdateImmediately(bool immediate)
{
    MarkCallFunction("SetUpdateImmediately");
}

int INetworkPNL::GetNumberOfSamples()
{
    MarkCallFunction("GetNumberOfSamples");
    return 0;
}

void INetworkPNL::SetNumberOfSamples(int samples)
{
    MarkCallFunction("SetNumberOfSamples");
}

void INetworkPNL::GetNetworkProperties(PropertyMap &mp)
{
    MarkCallFunction("GetNetworkProperties", true);
    mp = m_NetProperty;
}

void INetworkPNL::SetNetworkProperties(const PropertyMap &mp)
{
    MarkCallFunction("SetNetworkProperties", true);
    m_NetProperty = mp;
}

void INetworkPNL::InvalidateValues()
{
    MarkCallFunction("InvalidateValues");
}

void INetworkPNL::ClearAllEvidence()
{
    MarkCallFunction("ClearAllEvidence", true);
    m_aEvidence->Clear();
}

void INetworkPNL::ClearAllDecisions()
{
    MarkCallFunction("ClearAllDecisions");
}

void INetworkPNL::ClearAllTargets()
{
    MarkCallFunction("ClearAllTargets");
}

IDiagNetwork* INetworkPNL::CreateDiagNetwork()
{
    MarkCallFunction("CreateDiagNetwork");
    return 0;
}

void INetworkPNL::SetEntropyCostRatio(double alpha, double alphaMax)
{
    MarkCallFunction("SetEntropyCostRatio");
    return;
}

ISearchForOpportunities* INetworkPNL::CreateSfo()
{
    MarkCallFunction("CreateSfo");
    return 0;
}

bool INetworkPNL::Voi(int targetNode, std::vector<std::pair<int, double> > &observations)
{
    MarkCallFunction("Voi");
    return false;
}

bool INetworkPNL::Voi(
		      const std::vector<int> & chanceNodes, 
		      int decision, 
		      int pointOfView, 
		      std::vector<int> & parents,
		      std::vector<double> & values)
{
    MarkCallFunction("Voi");
    return false;
}

bool INetworkPNL::GenerateDataFile(const GenerateDataFileParams &params)
{
    MarkCallFunction("GenerateDataFile");
    return false;
}

bool INetworkPNL::AnnealedMap(
			      const AnnealedMapParams& params,
			      const std::vector<std::pair<int, int> > &evidence,
			      const std::vector<int> &mapNodes, 
			      std::vector<int> &mapStates,
			      double &probM1E, double &probE)
{
    MarkCallFunction("AnnealedMap");
    return false;
}

void INetworkPNL::GetActionsAndDecisions(std::vector<int> &actions, std::vector<int> &decisions)
{
    MarkCallFunction("GetActionsAndDecisions");
}

void INetworkPNL::GetNodes(std::vector<int> &nodes)
{
    MarkCallFunction("GetNodes", true);
    Vector<int> aNode;
    Graph()->Names(&aNode);
    nodes.assign(aNode.begin(), aNode.end());
}

void INetworkPNL::GetTerminalUtilityMinMax(double &minUtility, double &maxUtility)
{
    MarkCallFunction("GetTerminalUtilityMinMax");
}

int INetworkPNL::AddNode(int nodeType, const char *nodeId)
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

void INetworkPNL::DeleteNode(int node)
{
    MarkCallFunction("DeleteNode", true);
    Distributions()->DropDistribution(node);
    m_aNodeProperty[node].resize(0);
    Token()->DelNode(node);
}

bool INetworkPNL::AddArc(int nodeFrom, int nodeTo)
{
    MarkCallFunction("AddArc", true);
    Graph()->AddArc(nodeFrom, nodeTo);
    std::vector<int> parents;
    GetParents(nodeTo, parents);
    Distributions()->Setup(nodeTo);
    m_aNodeValueStatus[nodeTo] = NetConst::NotUpdated;
    return true;
}

void INetworkPNL::DeleteArc(int nodeFrom, int nodeTo)
{
    MarkCallFunction("DeleteArc", true);
    Graph()->DelArc(nodeFrom, nodeTo);
    Distributions()->Setup(nodeTo);
    m_aNodeValueStatus[nodeTo] = NetConst::NotUpdated;
}

void INetworkPNL::GetParents(int node, std::vector<int> &parents)
{
    MarkCallFunction("GetParents", true);
    pnl::intVector pnlParents;
    Graph()->GetParents(&pnlParents, node);
    parents.assign(pnlParents.begin(), pnlParents.end());
}

int INetworkPNL::GetParentCount(int node)
{
    MarkCallFunction("GetParentCount", true);
    return Graph()->nParent(node);
}

int INetworkPNL::FindParent(int node, int parentNode)
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

void INetworkPNL::GetChildren(int node, std::vector<int> &children)
{
    MarkCallFunction("GetChildren", true);
    pnl::intVector pnlChildren;

    Graph()->GetChildren(&pnlChildren, node);
    children.assign(pnlChildren.begin(), pnlChildren.end());
}

int INetworkPNL::GetChildrenCount(int node)
{
    MarkCallFunction("GetChildrenCount", true);
    return Graph()->nChild(node);
}

int INetworkPNL::FindChild(int node, int childNode)
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

int INetworkPNL::GetOutcomeCount(int node)
{
    MarkCallFunction("GetOutcomeCount", true, (String() << "node #" << node).c_str());
    
    Vector<String> aValue;
    Token()->GetValues(node, aValue);
    MarkCallFunction("GetOutcomeCount", true, (String() << "size " << int(aValue.size())).c_str());
    return aValue.size();
}

const char* INetworkPNL::GetOutcomeId(int node, int outcomeIndex)
{
    MarkCallFunction("GetOutcomeId", true);
    m_Bad.assign(Token()->Value(node, outcomeIndex).c_str());
    return m_Bad.c_str();// may bring to inconsistent state if pointer will saved!!
}

const char* INetworkPNL::GetOutcomeLabel(int node, int outcomeIndex)
{
    MarkCallFunction("GetOutcomeLabel unfin x", true);
    m_Bad.assign(Token()->Value(node, outcomeIndex).c_str());
    return m_Bad.c_str();// may bring to inconsistent state if pointer will saved!!
}

void INetworkPNL::GetDefinition(int node, std::vector<double> &definition)
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

void INetworkPNL::GetValue(int node, bool &valueValid, std::vector<int> &parents, std::vector<double> &values)
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

int INetworkPNL::GetValueParentCount(int node)
{
    MarkCallFunction("GetValueParentCount");
    return 0;
}

bool INetworkPNL::GetValueParents(int node, std::vector<int> &parents)
{
    MarkCallFunction("GetValueParents");
    return false;
}

int INetworkPNL::GetEvidence(int node)
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

int INetworkPNL::GetControlledValue(int node)
{
    MarkCallFunction("GetControlledValue", true, (String() << node).c_str());
    return -1;
}

int INetworkPNL::GetNodeType(int node)
{
    MarkCallFunction("GetNodeType unfin(Chance only) x", true, (String() << node).c_str());
    return NetConst::Chance;// rough!
}

bool INetworkPNL::IsIdUnique(const char *nodeId, int nodeToIgnore)
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

int INetworkPNL::FindNode(const char * id)
{
    MarkCallFunction("FindNode");
    return 0;
}

const char* INetworkPNL::GetNodeId(int node)
{
    MarkCallFunction("GetNodeId", true);
    return Graph()->NodeName(node).c_str();
}

bool INetworkPNL::IsTarget(int node)
{
    MarkCallFunction("IsTarget", true);
    return false;
}

bool INetworkPNL::IsEvidence(int node)
{
    MarkCallFunction("IsEvidence", true);
    return IsRealEvidence(node);
}

bool INetworkPNL::IsRealEvidence(int node)
{
    MarkCallFunction("IsRealEvidence", true, (String() << "Node #" << node).c_str());
    return m_abEvidence[node];
}

bool INetworkPNL::IsPropagatedEvidence(int node)
{
    MarkCallFunction("IsPropagatedEvidence", true);
    return false;
}

bool INetworkPNL::IsValueValid(int node)
{
    MarkCallFunction("IsValueValid", true);
    return m_aNodeValueStatus[node] == NetConst::Updated;
}

bool INetworkPNL::IsControlled(int node)
{
    MarkCallFunction("IsControlled", true);
    return false;
}

int INetworkPNL::GetNodeValueStatus(int node)
{
    MarkCallFunction("GetNodeValueStatus", true, (String() << "node #" << node).c_str());
    return m_aNodeValueStatus[node];
}

bool INetworkPNL::GetMinMaxUtility(int node, double &minUtility, double &maxUtility)
{
    MarkCallFunction("GetMinMaxUtility");
    return false;
}

void INetworkPNL::GetProperties(int node, PropertyMap &mp)
{
    MarkCallFunction("GetProperties", true);
    mp = m_aNodeProperty.at(node);
}

void INetworkPNL::SetProperties(int node, const PropertyMap &mp)
{
    MarkCallFunction("SetProperties", true);
    m_aNodeProperty.at(node) = mp;
}

bool INetworkPNL::SetNodeId(int node, const char *id)
{
    String newId(id);

    newId << " - new name for node #" << node;
    MarkCallFunction("SetNodeId", true, newId.c_str());
    return Graph()->SetNodeName(node, String(id));
}

void INetworkPNL::SetNodeType(int node, int type)
{
    MarkCallFunction("SetNodeType");
}

void INetworkPNL::AddOutcome(int node, int outcomeIndex, const char *outcomeId)
{
    MarkCallFunction("AddOutcome");
}

void INetworkPNL::DeleteOutcome(int node, int outcomeIndex)
{
    MarkCallFunction("DeleteOutcome");
}

void INetworkPNL::SetDefinition(int node, const std::vector<double> & definition)
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

bool INetworkPNL::SetOutcomeId(int node, int outcomeIndex, const char *id)
{
    MarkCallFunction("SetOutcomeId unchecked unfin x", true);
    Token()->SetValue(node, outcomeIndex, String(id));
    return true;
}

void INetworkPNL::SetOutcomeIds(int node, const std::vector<std::string> &ids)
{
    MarkCallFunction("SetOutcomeIds unchecked unfin x", true);

    SetValues(node, ids);
}

bool INetworkPNL::SetOutcomeLabel(int node, int outcomeIndex, const char *label)
{
    MarkCallFunction("SetOutcomeLabel");
    return false;
}

void INetworkPNL::SetOutcomeLabels(int node, const std::vector<std::string> &labels)
{
    MarkCallFunction("SetOutcomeLabels");
}

void INetworkPNL::SetEvidence(int node, int outcomeIndex)
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

void INetworkPNL::ClearEvidence(int node)
{
    MarkCallFunction("ClearEvidence", true, (String() << "Node #" << node).c_str());
    m_abEvidence[node] = false;
    m_aNodeValueStatus[node] = NetConst::NotUpdated;
}

void INetworkPNL::SetControlledValue(int node, int outcomeIndex)
{
    MarkCallFunction("SetControlledValue");
    m_aNodeValueStatus[node] = NetConst::Updated;
}

void INetworkPNL::ClearControlledValue(int node)
{
    MarkCallFunction("ClearControlledValue");
    m_aNodeValueStatus[node] = NetConst::NotUpdated;
}

bool INetworkPNL::IsControllable(int node)
{
    MarkCallFunction("IsControllable", true);
    return false;
}

void INetworkPNL::SetTarget(int node, bool asTarget)
{
    MarkCallFunction("SetTarget");
}

void INetworkPNL::NoisyToCpt(
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

void INetworkPNL::NoisyToHenrion(
				 int node,
				 int outcomeCount,
				 const std::vector<int> &parentPermutation,
				 const std::vector<double> &noisy, 
				 std::vector<double> &henrion)
{
    MarkCallFunction("NoisyToHenrion");
}

void INetworkPNL::NoisyFromHenrion(
				   int node,
				   int outcomeCount,
				   const std::vector<int> &parentPermutation,
				   const std::vector<double> &henrion, 
				   std::vector<double> &noisy)
{
    MarkCallFunction("NoisyFromHenrion");
}

// noisymax
void INetworkPNL::NoisyGetParentStrengths(int node, int parentIndex, std::vector<int> &strengths)
{
    MarkCallFunction("NoisyGetParentStrengths");
}

void INetworkPNL::NoisySetParentStrengths(int node, int parentIndex, const std::vector<int> &strengths)
{
    MarkCallFunction("NoisySetParentStrengths");
}

// noisyadder
void INetworkPNL::NoisyGetDistStates(int node, std::vector<int> &distStates)
{
    MarkCallFunction("NoisyGetDistStates");
}

void INetworkPNL::NoisySetDistStates(int node, const std::vector<int> &distStates)
{
    MarkCallFunction("NoisySetDistStates");
}

void INetworkPNL::NoisyGetWeights(int node, std::vector<double> &weights)
{
    MarkCallFunction("NoisyGetWeights");
}

void INetworkPNL::NoisySetWeights(int node, const std::vector<double> &weights)
{
    MarkCallFunction("NoisySetWeights");
}

int INetworkPNL::NoisyGetFunction(int node)
{
    MarkCallFunction("NoisyGetFunction");
    return 0;
}

void INetworkPNL::NoisySetFunction(int node, int function)
{
    MarkCallFunction("NoisySetFunction");
}


// equation-related functions
bool INetworkPNL::SetEquation(int node, const char *equation)
{
    MarkCallFunction("SetEquation");
    return false;
}

const char* INetworkPNL::GetEquation(int node)
{
    MarkCallFunction("GetEquation");
    return "";
}

bool INetworkPNL::ValidateEquation(int node, const char *alternateId, const char *equation, std::string &errMsg)
{
    MarkCallFunction("ValidateEquation");
    return false;
}

bool INetworkPNL::ReplaceVariableEquation(std::string &equation, const char *oldVar, const char *newVar)
{
    MarkCallFunction("ReplaceVariableEquation");
    return false;
}


// copy/paste and drag'n'drop support
bool INetworkPNL::CopyNodes(const std::vector<int> &sourceNodes, std::vector<int> &copiedNodes, const std::vector<std::string> &externalIds)
{
    MarkCallFunction("CopyNodes");
    return false;
}

bool INetworkPNL::CopyNodesFrom(const INetwork* sourceNetwork, const std::vector<int> &sourceNodes, std::vector<int> &copiedNodes, const std::vector<std::string> &externalIds)
{
    MarkCallFunction("CopyNodesFrom");
    return false;
}

bool INetworkPNL::Serialize(
			    ISerializer *serializer,
			    const std::vector<int> &nodes,
			    const std::vector<std::pair<int, UUID> > &uuids)
{
    MarkCallFunction("Serialize");
    return false;
}

bool INetworkPNL::Deserialize(
			      ISerializer *serializer, bool native,
			      std::vector<std::pair<int, int> > &createdNodes,
			      const std::vector<std::pair<int, UUID> > &uuids,
			      const std::vector<std::string> &externalIds)
{
    MarkCallFunction("Deserialize");
    return false;
}


// reordering parents
void INetworkPNL::ReorderParents(int node, const std::vector<int> & newOrder)
{
    MarkCallFunction("ReorderParents");
}


// syncing outcomes with definition grid;
// permutation contain old indices and -1 instead of new indices
void INetworkPNL::AddRemoveReorderOutcomes(
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


void INetworkPNL::SetValues(int iNode, const std::vector<std::string> &aId)
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

ProbabilisticNet &INetworkPNL::Net() const
{
    return m_pWNet->Net();
}

WGraph *INetworkPNL::Graph() const
{
    return Net().Graph();
}

TokenCover *INetworkPNL::Token() const
{
    return Net().Token();
}

WDistributions *INetworkPNL::Distributions() const
{
    return Net().Distributions();
}

// qualitative stuff
double INetworkPNL::QualGetWeight(int node, int parentIndex, bool positive)
{
    MarkCallFunction("QualGetWeight");
    return 0;
}

void INetworkPNL::QualSetWeight(int node, int parentIndex, bool positive, double weight)
{
    MarkCallFunction("QualSetWeight");
}

double INetworkPNL::QualGetBaseline(int node)
{
    MarkCallFunction("QualGetBaseline");
    return 0;
}

void INetworkPNL::QualSetBaseline(int node, double baseline)
{
    MarkCallFunction("QualSetBaseline");
}

double INetworkPNL::QualGetValue(int node)
{
    MarkCallFunction("QualGetValue");
    return 0;
}

int INetworkPNL::QualGetArcSign(int node, int parentIndex)
{
    MarkCallFunction("QualGetArcSign");
    return 0;
}

// diagnosis support
bool INetworkPNL::AddCostArc(int nodeFrom, int nodeTo)
{
    MarkCallFunction("AddCostArc");
    return false;
}

void INetworkPNL::DeleteCostArc(int nodeFrom, int nodeTo)
{
    MarkCallFunction("DeleteCostArc");
}

void INetworkPNL::GetCostParents(int node, std::vector<int> &parents)
{
    MarkCallFunction("GetCostParents");
}

int INetworkPNL::GetCostParentCount(int node)
{
    MarkCallFunction("GetCostParentCount");
    return 0;
}

int INetworkPNL::FindCostParent(int node, int parentNode)
{
    MarkCallFunction("FindCostParent");
    return 0;
}

void INetworkPNL::GetCostChildren(int node, std::vector<int> &children)
{
    MarkCallFunction("GetCostChildren");
}

int INetworkPNL::GetCostChildrenCount(int node)
{
    MarkCallFunction("GetCostChildrenCount");
    return 0;
}

int INetworkPNL::FindCostChild(int node, int childNode)
{
    MarkCallFunction("FindCostChild");
    return 0;
}

void INetworkPNL::GetCost(int node, std::vector<double> &cost)
{
    MarkCallFunction("GetCost");
}

void INetworkPNL::SetCost(int node, const std::vector<double> &cost)
{
    MarkCallFunction("SetCost");
}

double INetworkPNL::GetNotAvailableCostValue()
{
    MarkCallFunction("GetNotAvailableCostValue");
    return 0;
}

double INetworkPNL::GetNotRelevantTestValue()
{
    MarkCallFunction("GetNotRelevantTestValue");
    return 0;
}

void INetworkPNL::GetGroupCostProperty(std::string &name, std::string &value)
{
    MarkCallFunction("GetGroupCostProperty");
}

int INetworkPNL::GetDiagType(int node)
{
    MarkCallFunction("GetDiagType");
    return 0;
}

void INetworkPNL::SetDiagType(int node, int diagType)
{
    MarkCallFunction("SetDiagType");
}

void INetworkPNL::SetRanked(int node, bool ranked)
{
    MarkCallFunction("SetRanked");
}

bool INetworkPNL::IsRanked(int node)
{
    MarkCallFunction("IsRanked NS x", true);
    return false;
}

void INetworkPNL::SetMandatory(int node, bool mandatory)
{
    MarkCallFunction("SetMandatory");
}

bool INetworkPNL::IsMandatory(int node)
{
    MarkCallFunction("IsMandatory NS x", true);
    return false;
}

int INetworkPNL::GetDefaultOutcome(int node)
{
    MarkCallFunction("GetDefaultOutcome");
    return 0;
}

void INetworkPNL::SetDefaultOutcome(int node, int outcome)
{
    MarkCallFunction("SetDefaultOutcome");
}

bool INetworkPNL::IsFaultOutcome(int node, int outcome)
{
    MarkCallFunction("IsFaultOutcome");
    return false;
}

void INetworkPNL::SetFaultOutcome(int node, int outcome, bool fault)
{
    MarkCallFunction("SetFaultOutcome");
}


bool INetworkPNL::LearnStructureAndParams(const char *dataFile, const char *networkFile)
{
    MarkCallFunction("LearnStructureAndParams");
    return false;
}

void INetworkPNL::MarkCallFunction(const char *name, bool bRealized, const char *args)
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

bool XmlWriterPNL::OpenFile(const char * filename)
{
    return m_pRealWriter->OpenFile(filename);
}

bool XmlWriterPNL::CloseFile()
{
    return m_pRealWriter->CloseFile();
}

void XmlWriterPNL::OpenElement(const char * name)
{
    m_pRealWriter->OpenElement(name);
}

void XmlWriterPNL::CloseElement(const char * name)
{
    m_pRealWriter->CloseElement(name);
}

void XmlWriterPNL::WriteElement(
	const char * name, 
	const char * content, 
	bool escapeWhitespace)
{
    m_pRealWriter->WriteElement(name, content, escapeWhitespace);
}

void XmlWriterPNL::PushAttribute(const char *name, const char * value)
{
    m_pRealWriter->PushAttribute(name, value);
}

void XmlWriterPNL::PushAttribute(const char *name, int value)
{
    pnl::pnlString str;
    
    str << value;
    m_pRealWriter->PushAttribute(name, str.c_str());
}

void XmlWriterPNL::PushAttribute(const char *name, bool value)
{
    pnl::pnlString str;
    
    str << value;
    m_pRealWriter->PushAttribute(name, str.c_str());
}

void XmlWriterPNL::PushAttribute(const char *name, double value)
{
    pnl::pnlString str;
    
    str << value;
    m_pRealWriter->PushAttribute(name, str.c_str());
}

class XmlBindingPNL: public IXmlBinding
{
public:
    XmlBindingPNL(IXmlHandler *handler, const char *name, int validation):
    m_Name(name), m_Handler(handler) {}

    virtual ~XmlBindingPNL() {}

    virtual void AddChild(const IXmlBinding *child, int occurences);
    virtual void AddAttribute(const char *attributeName, bool required);
    virtual void Delete();

    pnl::pnlString Name() const { return m_Name; }
    IXmlHandler *Handler() const { return m_Handler; }

#ifndef NDEBUG
    virtual void Dump(int indent, std::string &output) const;
#endif

    enum
    {
	ChildRequired = 1,
	ChildMultiple = 2,
	ZeroOrMore = ChildMultiple,
	OneOrMore = ChildRequired | ChildMultiple,
	ExactlyOne = ChildRequired,
	ZeroOrOne = 0,
    };

    enum 
    {
	ValidateNone = 0,
	ValidateAttributes = 1,
	ValidateChildren = 2,
	ValidateAll = ValidateAttributes | ValidateChildren,
	ValidateInherit = 4,
    };

private:
    pnl::pnlString m_Name;
    IXmlHandler *m_Handler;
    std::vector<const IXmlBinding*> m_Children;
    std::vector<pnl::pnlString> m_aAttrName;
};

void XmlBindingPNL::AddChild(const IXmlBinding *child, int occurences)
{
    m_Children.push_back(child);
}

void XmlBindingPNL::AddAttribute(const char *attributeName, bool required)
{
    m_aAttrName.push_back(attributeName);
}

void XmlBindingPNL::Delete()
{
    delete this;
}

#ifndef NDEBUG
void XmlBindingPNL::Dump(int indent, std::string &output) const
{

}
#endif

bool XmlReaderPNLH::Parse(const char *filename, const IXmlBinding *root, IXmlErrorHandler *errorHandler)
{
    return true;
}

bool XmlReaderPNLH::Parse(XMLContainer &container, const IXmlBinding *root,
			  IXmlErrorHandler *errorHandler)
{
    const XmlBindingPNL *binding = dynamic_cast<const XmlBindingPNL*>(root);

    if(!binding)
    {
	ThrowInternalError("Unknown Binding object type", "XmlReaderPNLH::Parse");
    }
    pnl::pnlString name = binding->Name();
    m_Container = &container;


    m_Container = 0;
}

void XmlReaderPNLH::StopParse(const char* errorMessage)
{
}

void XmlReaderPNLH::GetCurrentPosition(int &line, int &column)
{
}

const std::string& XmlReaderPNLH::GetContent()
{
    return std::string();
}

void XmlReaderPNLH::GetUnescapedContent(std::string &unescaped)
{
}

void XmlReaderPNLH::GetAttribute(const char *name, std::string &value)
{
}

bool XmlReaderPNLH::GetAttribute(const char *name, bool &value, bool defValue)
{
    return true;// xxx
}

bool XmlReaderPNLH::GetAttribute(const char *name, int &value, int defValue)
{
    return true;// xxx
}

bool XmlReaderPNLH::GetAttribute(const char *name, double &value, double defValue)
{
    return true;// xxx
}

IXmlBinding* XmlReaderPNLH::CreateBinding(IXmlHandler *handler, const char *name, int validation)
{
    return new XmlBindingPNL(handler, name, validation);
}
