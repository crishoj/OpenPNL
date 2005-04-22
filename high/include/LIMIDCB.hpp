#ifndef __LIMIDCB_HPP__
#define __LIMIDCB_HPP__

#include <string>
#ifndef __PNLHIGHCONF_HPP__
#include "pnlHighConf.hpp"
#endif
#include "NetCallBack.hpp"

PNLW_BEGIN

class PNLHIGH_API LIMIDCallback: public NetCallback
{
ALLOW_TESTING
public:
    LIMIDCallback() {}

    virtual pnl::CGraphicalModel *CreateModel(ProbabilisticNet &net);

    virtual void GenerateSamples(ProbabilisticNet &net,
        pnl::pEvidencesVector *newSamples, int nSample, pnl::CEvidence *evid) {};

};

PNLW_END

#endif //__LIMIDCB_HPP__
