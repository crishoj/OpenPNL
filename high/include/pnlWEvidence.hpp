#ifndef __PNLWEVIDENCE_HPP__
#define __PNLWEVIDENCE_HPP__

#include <string>
#include "pnlHighConf.hpp"
#include "pnlTok.hpp"
#include "WCover.hpp"

// Tok identifies for categoric and continuous node types
extern PNLHIGH_API TokArr categoric;
extern PNLHIGH_API TokArr continuous;

// FORWARDS
class WDistribFun;
class WGraph;
class TokenCover;
class WDistributions;
class NetCallback;

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

#endif //__PNLWEVIDENCE_HPP__
