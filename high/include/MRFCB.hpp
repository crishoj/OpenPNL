#ifndef __MRFCB_HPP__
#define __MRFCB_HPP__

#include <string>
#ifndef __PNLHIGHCONF_HPP__
#include "pnlHighConf.hpp"
#endif
#include "NetCallBack.hpp"

PNLW_BEGIN

class PNLHIGH_API MRFCallback: public NetCallback
{
ALLOW_TESTING
public:
    MRFCallback() {}

    virtual pnl::CGraphicalModel *CreateModel(ProbabilisticNet &net);

    virtual void GenerateSamples(ProbabilisticNet &net,
	pnl::pEvidencesVector *newSamples, int nSample, pnl::CEvidence *evid);
};

PNLW_END

#endif //__MRFCB_HPP__
