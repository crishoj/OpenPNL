#ifndef __NETCALLBACK_HPP__
#define __NETCALLBACK_HPP__

#include <string>
#include "pnlHighConf.hpp"

// FORWARDS
PNLW_BEGIN
class ProbabilisticNet;
PNLW_END

namespace pnl
{
    class CEvidence;
    class CGraphicalModel;
    class pEvidenceVector;
}

PNLW_BEGIN

class PNLHIGH_API NetCallback
{
public:
    NetCallback() {}

    virtual pnl::CGraphicalModel *CreateModel(ProbabilisticNet &net) = 0;

    virtual void GenerateSamples(ProbabilisticNet &net,
	pnl::pEvidencesVector *newSamples, int nSample, pnl::CEvidence *evid) = 0;
};

PNLW_END

#endif //__NETCALLBACK_HPP__
