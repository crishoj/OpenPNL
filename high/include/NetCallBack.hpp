#ifndef __NETCALLBACK_HPP__
#define __NETCALLBACK_HPP__

#include <string>
#include "pnlHighConf.hpp"

// FORWARDS
class ProbabilisticNet;

namespace pnl
{
    class CEvidence;
    class CGraphicalModel;
    class pEvidenceVector;
}

class PNLHIGH_API NetCallback
{
public:
    NetCallback() {}

    virtual pnl::CGraphicalModel *CreateModel(ProbabilisticNet &net) = 0;

    virtual void GenerateSamples(ProbabilisticNet &net,
	pnl::pEvidencesVector *newSamples, int nSample, pnl::CEvidence *evid) = 0;
};

#endif //__NETCALLBACK_HPP__
