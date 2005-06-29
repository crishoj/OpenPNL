#ifndef __DIAGNETWORKPNLH_HPP__
#define __DIAGNETWORKPNLH_HPP__

#undef _DEBUG
#undef DEBUG
#include "windows.h"
#include <vector>
#include <string>
#include "diagnetwork.h"
#include "constants.h"
#include "xmlreader.h"

#include "pnlConfig.hpp"
#include "pnlXMLRead.hpp"
#include "pnlDiagnostics.hpp"
#include "networkPNLH.hpp"

typedef std::string StringGP;

PNLW_BEGIN
class BayesNet;
class ProbabilisticNet;
PNLW_END

PNLW_USING

namespace pnl
{
    class Log;
    class LogDrvStream;
    class pnlString;
};

class DiagNetworkPNL: public IDiagNetwork
{
public:
    DiagNetworkPNL();
    virtual ~DiagNetworkPNL();
    virtual INetwork *GetNetwork();
	
    virtual void Delete();

    virtual void UpdateFaultBeliefs();

    virtual void CollectNetworkInfo();
    virtual void SetDefaultStates();
    virtual void RestartDiagnosis();

    virtual double GetEntropyCostRatio();
    virtual double GetMaxEntropyCostRatio();

    virtual int GetFaultCount();
    virtual int GetFaultNode(int faultIndex);
    virtual int GetFaultState(int faultIndex);
    virtual int FindMostLikelyFault();
    virtual void SetPursuedFault(int fault);
    virtual void SetPursuedFaults(const std::vector<int> &faults);
    virtual int GetPursuedFault();
    virtual void GetPursuedFaults(std::vector<int> &faults);
    virtual int FindFault(int node, int state);

    virtual void InstantiateObservation(int nodeHandle, int outcome);
    virtual void ReleaseObservation(int nodeHandle);
    virtual bool MandatoriesInstantiated();
    virtual void ComputeTestStrengths(int algorithm);
    virtual int GetDefaultAlgorithm();
    virtual double GetEffectiveTestStrength(int testIndex, double costEntropyRatio);
    virtual void GetUnperformedTests(std::vector<int> &tests);

    virtual bool IsDSepEnabled();
    virtual void EnableDSep(bool enable);
    virtual bool AreQuickTestsEnabled();
    virtual void EnableQuickTests(bool enable);

    virtual bool LoadCaseLibrary(const std::string &filename, std::vector<XsclFileItem> &cases);
    virtual bool SaveCaseLibrary(const std::string &filename, const std::vector<XsclFileItem> &cases);
    
    virtual void SetNetwork(NetworkPNL *net);
protected:
    void MarkCallFunction(const char *name, bool bRealized = false, const char *args = 0);
    pnl::CDiagnostics *p_Diag;
    pnl::Log *m_pLog;				// log driver (head) for debugging purposes
    pnl::LogDrvStream *m_pLogStream;		// stream driver for debugging purposes
    NetworkPNL *m_net;
    double maxCostRatio;
    std::vector<std::pair<int, int> > faults;       // target nodes with target states
//    std::vector<std::pair<int, int> > observations;
    std::vector<int> pursuedFaults;
    int alg;
    int algDefault;
    bool quickTests;
    pnl::intVector testList;
    pnl::pnlVector<double> VOIList;
    // XsclFileItem ???
};

#endif // include guard
