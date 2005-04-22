#ifndef __PERSISTPROBNET_HPP__
#define __PERSISTPROBNET_HPP__

#ifndef __PNLHIGHCONF_HPP__
#include "pnlHighConf.hpp"
#endif

#include "pnlObject.hpp"
#include "pnlPersistence.hpp"

PNLW_BEGIN

class PNLHIGH_API PersistProbabilisticNet: public pnl::CPersistence
{
ALLOW_TESTING
public:
    virtual const char *Signature();
    virtual void Save(pnl::CPNLBase *pObj, pnl::CContextSave *pContext);
    virtual pnl::CPNLBase *Load(pnl::CContextLoad *pContext);
    virtual void TraverseSubobject(pnl::CPNLBase *pObj, pnl::CContext *pContext);
    virtual bool IsHandledType(pnl::CPNLBase *pObj) const;
};

PNLW_END

#endif // include guard
