// diagnetwork.h

#ifndef GENIE_INTERFACES_DIAGNETWORK_H
#define GENIE_INTERFACES_DIAGNETWORK_H

class INetwork;
//struct LegacySclFileInfo;

struct XsclFileItem
{
	std::string networkName;
	std::string name;
	std::string comment;
	std::vector<std::pair<std::string, std::string> > evidence;
	std::vector<std::pair<std::string, std::string> > pursuedFaults;
	double costRatio;
	double maxCostRatio;
};

class IDiagNetwork
{
public:
	virtual INetwork *GetNetwork() = 0;
	
	virtual void Delete() = 0;

	virtual void UpdateFaultBeliefs() = 0;

	virtual void CollectNetworkInfo() = 0;
	virtual void SetDefaultStates() = 0;
	virtual void RestartDiagnosis() = 0;

	virtual double GetEntropyCostRatio() = 0;
	virtual double GetMaxEntropyCostRatio() = 0;

	virtual int GetFaultCount() = 0;
	virtual int GetFaultNode(int faultIndex) = 0;
	virtual int GetFaultState(int faultIndex) = 0;
	virtual int FindMostLikelyFault() = 0;
	virtual void SetPursuedFault(int fault) = 0;
	virtual void SetPursuedFaults(const std::vector<int> &faults) = 0;
	virtual int GetPursuedFault() = 0;
	virtual void GetPursuedFaults(std::vector<int> &faults) = 0;
	virtual int FindFault(int node, int state) = 0;

	virtual void InstantiateObservation(int nodeHandle, int outcome) = 0;
	virtual void ReleaseObservation(int nodeHandle) = 0;
	virtual bool MandatoriesInstantiated() = 0;
	virtual void ComputeTestStrengths(int algorithm) = 0;
	virtual int GetDefaultAlgorithm() = 0;
	virtual double GetEffectiveTestStrength(int testIndex, double costEntropyRatio) = 0;
	virtual void GetUnperformedTests(std::vector<int> &tests) = 0;

	virtual bool IsDSepEnabled() = 0;
	virtual void EnableDSep(bool enable) = 0;
	virtual bool AreQuickTestsEnabled() = 0;
	virtual void EnableQuickTests(bool enable) = 0;

	virtual bool LoadCaseLibrary(const std::string &filename, std::vector<XsclFileItem> &cases) = 0;
	virtual bool SaveCaseLibrary(const std::string &filename, const std::vector<XsclFileItem> &cases) = 0;
};

class ISearchForOpportunities
{
public:
	struct ActionInfo
	{
		int node;
		int state;
		double voint;
	};
	
	virtual void Delete() = 0;

	virtual void GetFocus(std::vector<int> &focus) = 0;
	virtual void GetMarginals(int node, std::vector<double> &marginals) = 0;
		
	virtual void GetActions(std::vector<ActionInfo> &actions) = 0;

	virtual bool Act(int node, int state) = 0;
	virtual bool Retract(int node, int state) = 0;

	virtual bool MyopicSearch() = 0;
	
	virtual void GetRankedActions(std::vector<ActionInfo> &ranked) = 0;
	virtual void GetPerformedActions(std::vector<ActionInfo> &performed) = 0;
};

#endif // GENIE_INTERFACES_DIAGNETWORK_H
