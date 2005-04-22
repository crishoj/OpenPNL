#ifndef __WCOVER_HPP__
#define __WCOVER_HPP__

#ifndef __PNLHIGHCONF_HPP__
#include "pnlHighConf.hpp"
#endif

PNLW_BEGIN

class PNLHIGH_API WCover
{
ALLOW_TESTING
public:
    virtual ~WCover() {}

protected:
    WCover() {}
};

PNLW_END

#endif //__WCOVER_HPP__
