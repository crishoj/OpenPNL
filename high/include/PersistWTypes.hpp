#ifndef __PERSISTWTYPES_HPP__
#define __PERSISTWTYPES_HPP__

#include "pnlHighConf.hpp"
#include "pnlPersistence.hpp"

// FORWARDS

PNLW_BEGIN

class PNLHIGH_API PersistTokenArray: public pnl::CPersistence
{
public:
    virtual const char *Signature();
    virtual void Save(pnl::CPNLBase *pObj, pnl::CContextSave *pContext);
    virtual pnl::CPNLBase *Load(pnl::CContextLoad *pContext);
    virtual bool IsHandledType(pnl::CPNLBase *pObj) const;
};

class PNLHIGH_API PersistSSMap: public pnl::CPersistence
{
public:
    virtual const char *Signature();
    virtual void Save(pnl::CPNLBase *pObj, pnl::CContextSave *pContext);
    virtual pnl::CPNLBase *Load(pnl::CContextLoad *pContext);
    virtual bool IsHandledType(pnl::CPNLBase *pObj) const;
};

PNLW_END

#endif // include guard
