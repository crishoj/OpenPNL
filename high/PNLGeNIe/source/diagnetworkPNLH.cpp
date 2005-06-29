#undef _DEBUG
#undef DEBUG

#include "BNet.hpp"
#include "pnlWGraph.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "diagnetworkPNLH.hpp"
#include "pnlLog.hpp"

INetwork * DiagNetworkPNL::GetNetwork()
{
    MarkCallFunction("GetNetwork",true);
    return m_net;
}

void DiagNetworkPNL::Delete()
{
    MarkCallFunction("Delete",true);
    delete this;
}

DiagNetworkPNL::DiagNetworkPNL()
{
    p_Diag = 0;
    m_pLogStream = new pnl::LogDrvStream("GeNIe_PNLModule.log", pnl::eLOG_ALL, pnl::eLOGSRV_ALL);
    m_pLog = new pnl::Log("", pnl::eLOG_DEBUG|pnl::eLOG_NOTICE|pnl::eLOG_INFO, pnl::eLOGSRV_TEST1);
    (*m_pLog) << "Ok, Started!\n";
    m_net = 0;
    maxCostRatio = 10;
    algDefault = 1;
    alg = algDefault;
    quickTests = false;
    testList.resize(0);
    VOIList.resize(0);
}

DiagNetworkPNL::~DiagNetworkPNL()
{
    delete m_pLog;
    delete m_pLogStream;
}

void DiagNetworkPNL::UpdateFaultBeliefs()
{
    MarkCallFunction("UpdateFaultBeliefs");
    int i;
    std::vector<int> par;
    std::vector<double> vals;
    bool valid;
    TokArr evid;
    for( i = 0; i < faults.size(); ++i)
    {
        m_net->GetValue(faults[i].first, valid, par, vals);
    }
}

void DiagNetworkPNL::CollectNetworkInfo()
{
    MarkCallFunction("CollectNetworkInfo");
}

void DiagNetworkPNL::SetDefaultStates()
{
    MarkCallFunction("SetDefaultStates",true);
	int numOfNodes = m_net->Net().nNetNode();
    int i;
    for ( i = 0; i < numOfNodes; ++i)
    {
        if ( m_net->GetDefaultOutcome(i) > -1)
        {
            m_net->SetEvidence( i, m_net->GetDefaultOutcome(i));
        }
    }
}

void DiagNetworkPNL::RestartDiagnosis()
{
    MarkCallFunction("RestartDiagnosis");
    m_net->ClearAllEvidence();
}

double DiagNetworkPNL::GetEntropyCostRatio()
{
    MarkCallFunction("GetEntropyCostRatio",true);
    if (m_net)
    {
        return m_net->m_entropy;
    }
    else
    {
        return 1.0;
    }
}

double DiagNetworkPNL::GetMaxEntropyCostRatio()
{
    MarkCallFunction("GetMaxEntropyCostRatio",true);
    return m_net->m_entropyMax;
}

int DiagNetworkPNL::GetFaultCount()
{
    MarkCallFunction("GetFaultCount",true);
    return faults.size();
}

int DiagNetworkPNL::GetFaultNode(int faultIndex)
{
    MarkCallFunction("GetFaultNode",true);
    return faults[faultIndex].first;
}

int DiagNetworkPNL::GetFaultState(int faultIndex)
{
    MarkCallFunction("GetFaultState",true);
    return faults[faultIndex].second;
}

int DiagNetworkPNL::FindMostLikelyFault()
{
    MarkCallFunction("FindMostLikelyFault");
    int i;
    std::vector<int> par;
    std::vector<double> vals;
    bool valid;
    int mostLikelyInd = -1;
    double mostLikelyVal = 0.0;
    for( i = 0; i < faults.size(); ++i)
    {
        m_net->GetValue(faults[i].first, valid, par, vals);
        if (vals[faults[i].second] > mostLikelyVal)
        {
            mostLikelyInd = i;
            mostLikelyVal = vals[faults[i].second];
        }
    }
    return mostLikelyInd;
}

void DiagNetworkPNL::SetPursuedFault(int fault)
{
    MarkCallFunction("SetPursuedFault",true);
    pursuedFaults.clear();
    pursuedFaults.push_back(fault);
}

void DiagNetworkPNL::SetPursuedFaults(const std::vector<int> &faults)
{
    MarkCallFunction("SetPursuedFaults",true);
    pursuedFaults.clear();
    pursuedFaults.insert(pursuedFaults.begin() + pursuedFaults.size(), 
        faults.begin(), faults.begin() + faults.size());
}

int DiagNetworkPNL::GetPursuedFault()
{
    MarkCallFunction("GetPursuedFault",true);
    if (pursuedFaults.size() == 1)
        return pursuedFaults[0];
    return -1;
}

void DiagNetworkPNL::GetPursuedFaults(std::vector<int> &faults)
{
    MarkCallFunction("GetPursuedFaults",true);
    faults.clear();
    faults.insert(faults.begin(),pursuedFaults.begin(),
        pursuedFaults.begin() + pursuedFaults.size());
}

int DiagNetworkPNL::FindFault(int node, int state)
{
    MarkCallFunction("FindFault",true);
    int i=0;
    while ((faults[i].first != node) || (faults[i].second != state))
        ++i;
    if (i == faults.size())
        i=-1;
    return i;
}

void DiagNetworkPNL::InstantiateObservation(int nodeHandle, int outcome)
{
    MarkCallFunction("InstantiateObservation",true);
    m_net->SetEvidence(nodeHandle, outcome);
}

void DiagNetworkPNL::ReleaseObservation(int nodeHandle)
{
    MarkCallFunction("ReleaseObservation",true);
    m_net->ClearEvidence(nodeHandle);
}

bool DiagNetworkPNL::MandatoriesInstantiated()
{
    MarkCallFunction("MandatoriesInstantiated",true);
    return true;
}

void DiagNetworkPNL::ComputeTestStrengths(int algorithm)
{
    MarkCallFunction("ComputeTestStrengths",true);
    pnl::intVector targetNodes;
    pnl::intVector obsNodes;
    int numOfNodes = m_net->Net().nNetNode();
    int i;
    for ( i = 0; i < numOfNodes; ++i)
    {
        switch (m_net->GetDiagType(i))
        {
        case 1:
            targetNodes.push_back(i);
            break;
        case 2:
            obsNodes.push_back(i);
            break;
        }
    }
    p_Diag->SetTargetNodes(targetNodes);
    p_Diag->SetObservationNodes(obsNodes);
    for ( i = 0; i < faults.size(); ++i)
    {
        p_Diag->SetTargetState(faults[i].first,faults[i].second);
    }
    pnl::intVector pursuedNodes;
    pnl::intVector pursuedStates;
    for ( i = 0; i < pursuedFaults.size(); ++i)
    {
        pursuedNodes.push_back(faults[pursuedFaults[i]].first);
        pursuedStates.push_back(faults[pursuedFaults[i]].second);
    }
    p_Diag->SetCostRatio(GetEntropyCostRatio());
	switch (algorithm)
	{
        case 1:
            p_Diag->SetAlgorithm(0);
            break;
        case 2:
            p_Diag->SetAlgorithm(1);
            break;
	}
    p_Diag->GetTestsList(pursuedNodes, pursuedStates, testList, VOIList);
}

int DiagNetworkPNL::GetDefaultAlgorithm()
{
    MarkCallFunction("GetDefaultAlgorithm",true);
    return algDefault;
}

double DiagNetworkPNL::GetEffectiveTestStrength(int testIndex, double costEntropyRatio)
{
    MarkCallFunction("GetEffectiveTestStrength");
    std::vector<double> costs;
    costs.resize(0);
    std::vector<int> test;
    test.resize(0);
    GetUnperformedTests(test);
    m_net->GetCost(test[testIndex], costs);
    return VOIList[testIndex]-costEntropyRatio*costs[0];
}

void DiagNetworkPNL::GetUnperformedTests(std::vector<int> &tests)
{
    MarkCallFunction("GetUnperformedTests",true);
    tests.clear();
    int numOfNodes = m_net->Net().nNetNode();
    int i;
    for ( i = 0; i < numOfNodes; ++i)
    {
        if (m_net->GetDiagType(i) == 2)
        {
            if (m_net->GetEvidence(i) == -1)
                tests.push_back(i);
        }
    }
}

bool DiagNetworkPNL::IsDSepEnabled()
{
    MarkCallFunction("IsDSepEnabled",true);
    return false;
}

void DiagNetworkPNL::EnableDSep(bool enable)
{
    MarkCallFunction("EnableDSep");
}

bool DiagNetworkPNL::AreQuickTestsEnabled()
{
    MarkCallFunction("AreQuickTestsEnabled",true);
    return quickTests;
}

void DiagNetworkPNL::EnableQuickTests(bool enable)
{
    MarkCallFunction("EnableQuickTests",true);
    quickTests = enable;
}

bool DiagNetworkPNL::LoadCaseLibrary(const std::string &filename, std::vector<XsclFileItem> &cases)
{
    MarkCallFunction("LoadCaseLibrary");
    return 0;
}

bool DiagNetworkPNL::SaveCaseLibrary(const std::string &filename, const std::vector<XsclFileItem> &cases)
{
    MarkCallFunction("SaveCaseLibrary");
    return 0;
}

void DiagNetworkPNL::SetNetwork(NetworkPNL *net)
{
    MarkCallFunction("SetNetwork",true);
    if (net != 0)
    {
        m_net = net;
        p_Diag = pnl::CDiagnostics::Create(&((m_net->GetBayesNet())->Model()));
        int i, j;
    	int numOfNodes = m_net->Net().nNetNode();
        for( i = 0; i < numOfNodes; ++i)
        {
            for ( j = 0; j < m_net->GetOutcomeCount(i); ++j )
            {
                if (m_net->IsFaultOutcome( i, j ))
                {
                    std::pair<int, int> temp;
                    temp.first =  i;
                    temp.second = j;
                    faults.push_back(temp);
                }
            }
        }
    }
}

void DiagNetworkPNL::MarkCallFunction(const char *name, bool bRealized, const char *args)
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
	m_pLogStream->Redirect("GeNIe_PNLDiagModule2.log");
	delete m_pLogStream;
	m_pLogStream = 0;
	delete m_pLog;
	m_pLog = 0;
	pnl::pnlString s;
	s << "Function '" << name << "' not yet implemented";
	PNL_THROW(pnl::CNotImplemented, s.c_str());
    }
}
