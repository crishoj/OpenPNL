#ifndef __NETCALLBACK_HPP__
#define __NETCALLBACK_HPP__

#include <string>
#ifndef __PNLHIGHCONF_HPP__
#include "pnlHighConf.hpp"
#endif

// FORWARDS
PNLW_BEGIN
class ProbabilisticNet;
PNLW_END

namespace pnl
{
    class CNodeType;
    class CEvidence;
    class CGraphicalModel;
    class pEvidenceVector;
    class CGraphicalModel;
}

PNLW_BEGIN

class PNLHIGH_API NetCallback
{
ALLOW_TESTING
public:
    NetCallback() {}

    virtual pnl::CGraphicalModel *CreateModel(ProbabilisticNet &net) = 0;

    virtual void GenerateSamples(ProbabilisticNet &net,
	pnl::pEvidencesVector *newSamples, int nSample, pnl::CEvidence *evid) = 0;

    static bool GetNodeInfo(Vector<pnl::CNodeType> *paNodeType,
	Vector<int> *paNodeAssociation,	const ProbabilisticNet &net);
    static bool CommonAttachFactors(pnl::CGraphicalModel &model,
	const ProbabilisticNet &net);
    static int NodeAssociation(Vector<pnl::CNodeType> *paNodeType,
	bool isDiscrete, int size, int nodeState = 0);
};

PNLW_END

#endif //__NETCALLBACK_HPP__
