#ifndef __MRFCB_HPP__
#define __MRFCB_HPP__

#include <string>
#include "pnlHighConf.hpp"
#include "NetCallBack.hpp"

PNLW_BEGIN

class PNLHIGH_API MRFCallback: public NetCallback
{
public:
    MRFCallback() {}

    virtual pnl::CGraphicalModel *CreateModel(ProbabilisticNet &net);

    virtual void GenerateSamples(ProbabilisticNet &net,
	pnl::pEvidencesVector *newSamples, int nSample, pnl::CEvidence *evid);
};

PNLW_END

#endif //__MRFCB_HPP__
