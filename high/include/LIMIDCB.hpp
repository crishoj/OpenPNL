#ifndef __LIMIDCB_HPP__
#define __LIMIDCB_HPP__

#include <string>
#include "pnlHighConf.hpp"
#include "NetCallBack.hpp"

class PNLHIGH_API LIMIDCallback: public NetCallback
{
public:
    LIMIDCallback() {}

    virtual pnl::CGraphicalModel *CreateModel(ProbabilisticNet &net);

    virtual void GenerateSamples(ProbabilisticNet &net,
        pnl::pEvidencesVector *newSamples, int nSample, pnl::CEvidence *evid) {};

};

#endif //__LIMIDCB_HPP__
