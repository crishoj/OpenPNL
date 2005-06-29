#ifndef __PERSISTNODEDIAGINFO_HPP__
#define __PERSISTNODEDIAGINFO_HPP__

#ifndef __PNLHIGHCONF_HPP__
#include "pnlHighConf.hpp"
#endif

#include "pnlObject.hpp"
#include "pnlPersistence.hpp"

struct NodeDiagInfo: public pnl::CPNLBase
{
    NodeDiagInfo(const String &diagType);
    NodeDiagInfo() {}

    String m_diagType;
    bool m_isMandatory;
    bool m_isRanked;
    pnl::pnlVector<bool> m_isTarget;
    int m_isDefault;
    double cost;
};

class PersistNodeDiagInfo: public pnl::CPersistence
{
public:
    virtual const char *Signature();
    virtual void Save(pnl::CPNLBase *pObj, pnl::CContextSave *pContext);
    virtual pnl::CPNLBase *Load(pnl::CContextLoad *pContext);
    virtual void TraverseSubobject(pnl::CPNLBase *pObj, pnl::CContext *pContext);
    virtual bool IsHandledType(pnl::CPNLBase *pObj) const;
};

#endif // include guard
