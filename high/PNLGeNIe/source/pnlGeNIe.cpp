#include "xmlreader.h"

#include "pnlGeNIe.hpp"
#include "BNet.hpp"
#include "pnlLog.hpp"

void INetworkPNL::Delete()
{
    MarkCallFunction("Delete", true);
    delete m_pWNet;
    delete m_pLog;
    delete m_pLogStream;
}

INetworkPNL::INetworkPNL(): m_ErrorOutput(0), m_NetName("PNLBNet")
{
    m_pWNet = new BayesNet;
    m_aNodeProperty.reserve(40);
    m_pLogStream = new pnl::LogDrvStream("GeNIe_PNLModule.log", eLOG_ALL, eLOGSRV_ALL);
    m_pLog = new pnl::Log("", eLOG_DEBUG|eLOG_NOTICE|eLOG_INFO, eLOGSRV_TEST1);
}

bool INetworkPNL::Load(const char *filename, IXmlBinding *externalBinding)
{
    MarkCallFunction("Load");
    return false;
}

bool INetworkPNL::Save(const char *filename, IXmlWriterExtension *externalExtension,
		       const LegacyDslFileInfo *gnet)
{
    MarkCallFunction("Save");
    return false;
}

bool INetworkPNL::UpdateBeliefs()
{
    MarkCallFunction("UpdateBeliefs");
    return false;
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
    return true;
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
    m_pWNet->ClearEvidHistory();
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
    MarkCallFunction("GetNodes");
}

void INetworkPNL::GetTerminalUtilityMinMax(double &minUtility, double &maxUtility)
{
    MarkCallFunction("GetTerminalUtilityMinMax");
}

int INetworkPNL::AddNode(int nodeType, const char *nodeId)
{
    MarkCallFunction("AddNode", true);
    //m_pWNet->AddNode(nodeId, TokArr::Span(0, nodeType - 1));
    m_aNodeProperty.resize(m_aNodeProperty.size() + 1);
    return 0;
}

void INetworkPNL::DeleteNode(int node)
{
    MarkCallFunction("DeleteNode");
}

bool INetworkPNL::AddArc(int nodeFrom, int nodeTo)
{
    MarkCallFunction("AddArc", true);
    m_pWNet->AddArc(Tok(nodeFrom), Tok(nodeTo));
    return true;
}

void INetworkPNL::DeleteArc(int nodeFrom, int nodeTo)
{
    MarkCallFunction("DeleteArc");
}

void INetworkPNL::GetParents(int node, std::vector<int> &parents)
{
    MarkCallFunction("GetParents");
}

int INetworkPNL::GetParentCount(int node)
{
    MarkCallFunction("GetParentCount");
    return 0;
}

int INetworkPNL::FindParent(int node, int parentNode)
{
    MarkCallFunction("FindParent");
    return 0;
}

void INetworkPNL::GetChildren(int node, std::vector<int> &children)
{
    MarkCallFunction("GetChildren");
}

int INetworkPNL::GetChildrenCount(int node)
{
    MarkCallFunction("GetChildrenCount");
    return 0;
}

int INetworkPNL::FindChild(int node, int childNode)
{
    MarkCallFunction("FindChild");
    return 0;
}

int INetworkPNL::GetOutcomeCount(int node)
{
    MarkCallFunction("GetOutcomeCount", true);// fix me!!
    return 0;
}

const char* INetworkPNL::GetOutcomeId(int node, int outcomeIndex)
{
    MarkCallFunction("GetOutcomeId");
    return "";
}

const char* INetworkPNL::GetOutcomeLabel(int node, int outcomeIndex)
{
    MarkCallFunction("GetOutcomeLabel");
    return "";
}

void INetworkPNL::GetDefinition(int node, std::vector<double> &definition)
{
    MarkCallFunction("GetDefinition");
}

void INetworkPNL::GetValue(int node, bool &valueValid, std::vector<int> &parents, std::vector<double> &values)
{
    MarkCallFunction("GetValue");
}

int INetworkPNL::GetValueParentCount(int node)
{
    MarkCallFunction("GetValueParentCount");
    return 0;
}

bool INetworkPNL::GetValueParents(int node, std::vector<int> &parents)
{
    MarkCallFunction("GetValueParents");
    return true;
}

int INetworkPNL::GetEvidence(int node)
{
    MarkCallFunction("GetEvidence");
    return 0;
}

int INetworkPNL::GetControlledValue(int node)
{
    MarkCallFunction("GetControlledValue");
    return 0;
}

int INetworkPNL::GetNodeType(int node)
{
    MarkCallFunction("GetNodeType");
    return 0;
}

bool INetworkPNL::IsIdUnique(const char *nodeId, int nodeToIgnore)
{
    MarkCallFunction("IsIdUnique", true);// fix me!!
    return true;
}

int INetworkPNL::FindNode(const char * id)
{
    MarkCallFunction("FindNode");
    return 0;
}

const char* INetworkPNL::GetNodeId(int node)
{
    MarkCallFunction("GetNodeId");
    return "";
}

bool INetworkPNL::IsTarget(int node)
{
    MarkCallFunction("IsTarget");
    return false;
}

bool INetworkPNL::IsEvidence(int node)
{
    MarkCallFunction("IsEvidence");
    return false;
}

bool INetworkPNL::IsRealEvidence(int node)
{
    MarkCallFunction("IsRealEvidence");
    return false;
}

bool INetworkPNL::IsPropagatedEvidence(int node)
{
    MarkCallFunction("IsPropagatedEvidence");
    return false;
}

bool INetworkPNL::IsValueValid(int node)
{
    MarkCallFunction("IsValueValid");
    return false;
}

bool INetworkPNL::IsControlled(int node)
{
    MarkCallFunction("IsControlled");
    return false;
}

int INetworkPNL::GetNodeValueStatus(int node)
{
    MarkCallFunction("GetNodeValueStatus");
    return 0;
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
    MarkCallFunction("SetNodeId");
    return false;
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
    MarkCallFunction("SetDefinition");
}

bool INetworkPNL::SetOutcomeId(int node, int outcomeIndex, const char *id)
{
    MarkCallFunction("SetOutcomeId");
    return false;
}

void INetworkPNL::SetOutcomeIds(int node, const std::vector<std::string> &ids)
{
    MarkCallFunction("SetOutcomeIds");
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
    MarkCallFunction("SetEvidence");
}

void INetworkPNL::ClearEvidence(int node)
{
    MarkCallFunction("ClearEvidence");
}

void INetworkPNL::SetControlledValue(int node, int outcomeIndex)
{
    MarkCallFunction("SetControlledValue");
}

void INetworkPNL::ClearControlledValue(int node)
{
    MarkCallFunction("ClearControlledValue");
}

bool INetworkPNL::IsControllable(int node)
{
    MarkCallFunction("IsControllable");
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


// syncing outcomes with definition grid
void INetworkPNL::AddRemoveReorderOutcomes(
					   int node, 
					   const std::vector<std::string> &outcomeIds,
					   const std::vector<int> &permutation)
{
    MarkCallFunction("AddRemoveReorderOutcomes");
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
    MarkCallFunction("IsRanked");
    return false;
}

void INetworkPNL::SetMandatory(int node, bool mandatory)
{
    MarkCallFunction("SetMandatory");
}

bool INetworkPNL::IsMandatory(int node)
{
    MarkCallFunction("IsMandatory");
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

void INetworkPNL::MarkCallFunction(const char *name, bool bRealized, pnl::pnlString args)
{
    Log &l = *m_pLog;
    l << "Function '" << name << "'";
    if(args.length())
    {
	l << ", arguments = '" << args.c_str() << "' ";
    }
    l << ((bRealized) ? "realized\n":"not yet realized\n");
    if(!bRealized)
    {
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
