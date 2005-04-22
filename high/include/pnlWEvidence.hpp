#ifndef __PNLWEVIDENCE_HPP__
#define __PNLWEVIDENCE_HPP__

#include <string>
#ifndef __PNLHIGHCONFIG_HPP__
#include "pnlHighConf.hpp"
#endif
#ifndef __TOKENS_HPP__
#include "Tokens.hpp"
#endif
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
ALLOW_TESTING
public:
    void Clear();
    void Set(const TokArr &evidence);
    TokArr Get() const;
    bool IsEmpty() const { return m_EvidenceBoard.size() == 0; }

    bool IsHidden(Tok &tok) const;
    void SetVisibility(Tok &tok, bool bVisible);
    bool IsNodeHere(Tok &tok) const;

private:
    TokArr m_EvidenceBoard;
    std::map<TokIdNode *, int> m_VarMap;
    Vector<char> m_abHidden;
};

PNLW_END

#endif //__PNLWEVIDENCE_HPP__
