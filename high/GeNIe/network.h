#ifndef GENIE_INTERFACES_NETWORK_H
#define GENIE_INTERFACES_NETWORK_H

// network.h
class ISerializer;
class IErrorOutput;
class IXmlWriterExtension;
class IXmlBinding;
class IDiagNetwork;
class ISearchForOpportunities;
struct LegacyDslFileInfo;

struct GenerateDataFileParams
{
	std::string filename;
	int recordCount;
	int randSeed;
	char stateSeparator;
	std::vector<int> selectedNodes;
	bool addFileHeader;
	bool useStateIndices;
	bool biasSamplesByEvidence;
	bool (__cdecl *progressFunc)(int recordIndex, int rejectedSamples, void *userData);
	void *userData;
};

struct AnnealedMapParams
{
	double speed;
	double Tmin;
	double Tinit;
	double kReheat;

	int cycles;
	int reheatSteps;
	int stopSteps;
};


// INetwork reperesents Bayesian network
// the interface is designed to be
// complete and minimal - there are
// no helper functions here.

class INetwork
{
public:
	typedef std::vector<std::pair<std::string, std::string> >PropertyMap;

	// high-level operations
	virtual void Delete() = 0;

	virtual const char* GetId() = 0;
	virtual bool SetId(const char*id) = 0;

	virtual void SetErrorOutput(IErrorOutput *errorOutput) = 0;
	virtual bool Load(const char *filename, IXmlBinding *externalBinding) = 0;
	virtual bool Save(const char *filename, IXmlWriterExtension *externalExtension, const LegacyDslFileInfo *gnet) = 0;
	virtual bool UpdateBeliefs() = 0;
	
	virtual void SetBnAlgorithm(int algorithm) = 0;
	virtual int GetBnAlgorithm() = 0;
	virtual void SetIdAlgorithm(int algorithm) = 0;
	virtual int GetIdAlgorithm() = 0;

	virtual bool SetAlgorithmParams(int algorithm, const std::vector<double> &params) = 0;
	virtual void GetAlgorithmParams(int algorithm, std::vector<double> &params) = 0;
	virtual bool ValidateAlgorithmParams(int algorithm, const std::vector<double> &params) = 0;
	virtual void GetDefaultAlgorithmParams(int algorithm, std::vector<double> &params) = 0;

	virtual bool GetUpdateImmediately() = 0;
	virtual void SetUpdateImmediately(bool immediate) = 0;

	virtual int GetNumberOfSamples() = 0;
	virtual void SetNumberOfSamples(int samples) = 0;

	virtual void GetNetworkProperties(PropertyMap &map) = 0;
	virtual void SetNetworkProperties(const PropertyMap &map) = 0;

	virtual void InvalidateValues() = 0;
	virtual void ClearAllEvidence() = 0;
	virtual void ClearAllDecisions() = 0;
	virtual void ClearAllTargets() = 0;
	
	virtual IDiagNetwork* CreateDiagNetwork() = 0;
	virtual void SetEntropyCostRatio(double alpha, double alphaMax) = 0;
	virtual void SetDiagPreferences(unsigned int prefs) = 0;
	virtual unsigned int GetDiagPreferences() = 0;

	virtual ISearchForOpportunities* CreateSfo() = 0;

	virtual bool Voi(int targetNode, std::vector<std::pair<int, double> > &observations) = 0;
	
	virtual bool Voi(
		const std::vector<int> & chanceNodes, 
		int decision, 
		int pointOfView, 
		std::vector<int> & parents,
		std::vector<double> & values) = 0;

	virtual bool GenerateDataFile(const GenerateDataFileParams &params) = 0;

	virtual bool AnnealedMap(
		const AnnealedMapParams& params,
		const std::vector<std::pair<int, int> > &evidence,
		const std::vector<int> &mapNodes, 
		std::vector<int> &mapStates,
		double &probM1E, double &probE) = 0;

	virtual void GetActionsAndDecisions(std::vector<int> &actions, std::vector<int> &decisions) = 0;

	virtual void GetNodes(std::vector<int> &nodes) = 0;

	virtual void GetTerminalUtilityMinMax(double &minUtility, double &maxUtility) = 0;

	// node creation/destruction
	virtual int AddNode(int nodeType, const char *nodeId) = 0;
	virtual void DeleteNode(int node) = 0;

	// arc creation/destrucion
	virtual bool AddArc(int nodeFrom, int nodeTo) = 0;
	virtual void DeleteArc(int nodeFrom, int nodeTo) = 0;

	// ACCESSORS - node properties
	virtual void GetParents(int node, std::vector<int> &parents) = 0;
	virtual int GetParentCount(int node) = 0;
	virtual int FindParent(int node, int parentNode) = 0;
	virtual void GetChildren(int node, std::vector<int> &children) = 0;
	virtual int GetChildrenCount(int node) = 0;
	virtual int FindChild(int node, int childNode) = 0;
	virtual int GetOutcomeCount(int node) = 0;
	virtual const char* GetOutcomeId(int node, int outcomeIndex) = 0;
	virtual const char* GetOutcomeLabel(int node, int outcomeIndex) = 0;
	virtual void GetDefinition(int node, std::vector<double> &definition) = 0;
	virtual void GetValue(int node, bool &valueValid, std::vector<int> &parents, std::vector<double> &values) = 0;
	virtual int GetValueParentCount(int node) = 0;
	virtual bool GetValueParents(int node, std::vector<int> &parents) = 0;
	virtual int GetEvidence(int node) = 0;
	virtual int GetControlledValue(int node) = 0;
	virtual int GetNodeType(int node) = 0;
	virtual bool IsIdUnique(const char *nodeId, int nodeToIgnore) = 0;
	virtual int FindNode(const char * id) = 0;
	virtual const char* GetNodeId(int node) = 0;
	virtual bool IsTarget(int node) = 0;
	virtual bool IsEvidence(int node) = 0;
	virtual bool IsRealEvidence(int node) = 0;
	virtual bool IsPropagatedEvidence(int node) = 0;
	virtual bool IsValueValid(int node) = 0;
	virtual bool IsControlled(int node) = 0;
	virtual int GetNodeValueStatus(int node) = 0;
	virtual bool GetMinMaxUtility(int node, double &minUtility, double &maxUtility) = 0;
	

	virtual void GetProperties(int node, PropertyMap &map) = 0;

	// MUTATORS
	virtual bool SetNodeId(int node, const char *id) = 0;
	virtual void SetNodeType(int node, int type) = 0;
	virtual void AddOutcome(int node, int outcomeIndex, const char *outcomeId) = 0;
	virtual void DeleteOutcome(int node, int outcomeIndex) = 0;
	virtual void SetDefinition(int node, const std::vector<double> & definition) = 0;
	virtual bool SetOutcomeId(int node, int outcomeIndex, const char *id) = 0;
	virtual void SetOutcomeIds(int node, const std::vector<std::string> &ids) = 0;
	virtual bool SetOutcomeLabel(int node, int outcomeIndex, const char *label) = 0;
	virtual void SetOutcomeLabels(int node, const std::vector<std::string> &labels) = 0;
	virtual void SetEvidence(int node, int outcomeIndex) = 0;
	virtual void ClearEvidence(int node) = 0;
	virtual void SetControlledValue(int node, int outcomeIndex) = 0;
	virtual void ClearControlledValue(int node) = 0;
	virtual bool IsControllable(int node) = 0;
	virtual void SetTarget(int node, bool asTarget) = 0;
	virtual void SetProperties(int node, const PropertyMap &map) = 0;
	virtual void ObfuscateDefinition(int node, int algorithm, double param) = 0;

	// Noisy-related functions
	virtual void NoisyToCpt(
		int node, 
		int outcomeCount, 
		const std::vector<int> &parentPermutation,
		const std::vector<int> &noisyMaxStrengths,
		const std::vector<double> &noisyAdderWeights,
		const std::vector<int> &noisyAdderDistStates,
		int adderFunction,
		const std::vector<double> &noisy, 
		std::vector<double> &cpt) = 0;
	virtual void NoisyToHenrion(
		int node,
		int outcomeCount,
		const std::vector<int> &parentPermutation,
		const std::vector<double> &noisy, 
		std::vector<double> &henrion) = 0;
	virtual void NoisyFromHenrion(
		int node,
		int outcomeCount,
		const std::vector<int> &parentPermutation,
		const std::vector<double> &henrion, 
		std::vector<double> &noisy) = 0;
	// noisymax
	virtual void NoisyGetParentStrengths(int node, int parentIndex, std::vector<int> &strengths) = 0;
	virtual void NoisySetParentStrengths(int node, int parentIndex, const std::vector<int> &strengths) = 0;
	// noisyadder
	virtual void NoisyGetDistStates(int node, std::vector<int> &distStates) = 0;
	virtual void NoisySetDistStates(int node, const std::vector<int> &distStates) = 0;
	virtual void NoisyGetWeights(int node, std::vector<double> &weights) = 0;
	virtual void NoisySetWeights(int node, const std::vector<double> &weights) = 0;
	virtual int NoisyGetFunction(int node) = 0;
	virtual void NoisySetFunction(int node, int function) = 0;

	// equation-related functions
	virtual bool SetEquation(int node, const char *equation) = 0;
	virtual const char* GetEquation(int node) = 0;
	virtual bool ValidateEquation(int node, const char *alternateId, const char *equation, std::string &errMsg) = 0;
	virtual bool ReplaceVariableEquation(std::string &equation, const char *oldVar, const char *newVar) = 0;

	// copy/paste and drag'n'drop support
	virtual bool CopyNodes(const std::vector<int> &sourceNodes, std::vector<int> &copiedNodes, const std::vector<std::string> &externalIds) = 0;
	virtual bool CopyNodesFrom(const INetwork* sourceNetwork, const std::vector<int> &sourceNodes, std::vector<int> &copiedNodes, const std::vector<std::string> &externalIds) = 0;
	virtual bool Serialize(
		ISerializer *serializer,
		const std::vector<int> &nodes,
		const std::vector<std::pair<int, UUID> > &uuids) = 0;
	virtual bool Deserialize(
		ISerializer *serializer, bool native,
		std::vector<std::pair<int, int> > &createdNodes,
		const std::vector<std::pair<int, UUID> > &uuids,
		const std::vector<std::string> &externalIds) = 0;

	// reordering parents
	virtual void ReorderParents(int node, const std::vector<int> & newOrder) = 0;

	// syncing outcomes with definition grid
	virtual void AddRemoveReorderOutcomes(
		int node, 
		const std::vector<std::string> &outcomeIds,
		const std::vector<int> &permutation) = 0;

	// qualitative stuff
	virtual double QualGetWeight(int node, int parentIndex, bool positive) = 0;
	virtual void QualSetWeight(int node, int parentIndex, bool positive, double weight) = 0;
	virtual double QualGetBaseline(int node) = 0;
	virtual void QualSetBaseline(int node, double baseline) = 0;
	virtual double QualGetValue(int node) = 0;
	virtual int QualGetArcSign(int node, int parentIndex) = 0;

	// diagnosis support
	virtual bool AddCostArc(int nodeFrom, int nodeTo) = 0;
	virtual void DeleteCostArc(int nodeFrom, int nodeTo) = 0;
	virtual void GetCostParents(int node, std::vector<int> &parents) = 0;
	virtual int GetCostParentCount(int node) = 0;
	virtual int FindCostParent(int node, int parentNode) = 0;
	virtual void GetCostChildren(int node, std::vector<int> &children) = 0;
	virtual int GetCostChildrenCount(int node) = 0;
	virtual int FindCostChild(int node, int childNode) = 0;
	virtual void GetCost(int node, std::vector<double> &cost) = 0;
	virtual void SetCost(int node, const std::vector<double> &cost) = 0;
	virtual double GetNotAvailableCostValue() = 0;
	virtual double GetNotRelevantTestValue() = 0;
	virtual void GetGroupCostProperty(std::string &name, std::string &value) = 0;
	virtual int GetDiagType(int node) = 0;
	virtual void SetDiagType(int node, int diagType) = 0;
	virtual void SetRanked(int node, bool ranked) = 0;
	virtual bool IsRanked(int node) = 0;
	virtual void SetMandatory(int node, bool mandatory) = 0;
	virtual bool IsMandatory(int node) = 0;
	virtual int GetDefaultOutcome(int node) = 0;
	virtual void SetDefaultOutcome(int node, int outcome) = 0;
	virtual bool IsFaultOutcome(int node, int outcome) = 0;
	virtual void SetFaultOutcome(int node, int outcome, bool fault) = 0;
};

#endif	// GENIE_INTERFACES_NETWORK_H
