#ifndef __PERSISTWBNET_HPP__
#define __PERSISTWBNET_HPP__

#include "pnlHighConf.hpp"
#include "pnl_dll.hpp"
#include "pnlPersistence.hpp"

// FORWARDS

class PNLHIGH_API PersistWBNet: public pnl::CPersistence
{
public:
    virtual const char *Signature();
    virtual void Save(pnl::CPNLBase *pObj, pnl::CContextSave *pContext);
    virtual pnl::CPNLBase *Load(pnl::CContextLoad *pContext);
    virtual void TraverseSubobject(pnl::CPNLBase *pObj, pnl::CContext *pContext);
    virtual bool IsHandledType(pnl::CPNLBase *pObj) const;
};

#endif //__PERSISTWBNET_HPP__
