#ifndef __PNLWEVIDENCE_HPP__
#define __PNLWEVIDENCE_HPP__

#include <string>
#include "pnlHighConf.hpp"
#include "pnlTok.hpp"
#include "WCover.hpp"

PNLW_BEGIN

// FORWARDS
class WDistribFun;
class WGraph;
class TokenCover;
class WDistributions;
class NetCallback;

PNLW_END

namespace pnl
{
    class CPNLBase;
    class CEvidence;
    class CGraph;
    class CBNet;
    class CInfEngine;
    class CBayesLearningEngine;
    class pnlString;
    template<typename Type> class CMatrix;
}

PNLW_BEGIN

// Inner storage for evidence (before it will be transformed to PNL's Evidence)
class PNLHIGH_API WEvidence
{
public:
    void Clear();
    void Set(const TokArr &evidence);
    TokArr GetBoard() const { return m_EvidenceBoard; }
    bool IsEmpty() const { return m_EvidenceBoard.size() == 0; }

private:
    TokArr m_EvidenceBoard;
    std::map<TokIdNode *, int> m_VarMap;
};

PNLW_END

#endif //__PNLWEVIDENCE_HPP__
