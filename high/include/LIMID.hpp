#ifndef __LIMID_HPP__
#define __LIMID_HPP__

#include <string>
#ifndef __PNLHIGHCONF_HPP__
#include "pnlHighConf.hpp"
#endif
#ifndef __TOKENS_HPP__
#include "Tokens.hpp"
#endif
#include "ModelEngine.hpp"

// FORWARDS
PNLW_BEGIN
class WGraph;
class TokenCover;
class WDistributions;
class NetCallback;
class ProbabilisticNet;
PNLW_END

namespace pnl
{
    class CPNLBase;
    class CGraph;
    class CIDNet;
    class CLIMIDInfEngine;
    class pnlString;
    template<typename Type> class CMatrix;
}

PNLW_BEGIN

class PNLHIGH_API LIMID: public ModelEngine
{
ALLOW_TESTING
public:
    LIMID();
    ~LIMID();

    // add node
    void AddNode(TokArr nodes, TokArr subnodes);
    
    // remove node
    void DelNode(TokArr nodes);
    
    // manipulating arcs
    void AddArc(TokArr from, TokArr to);
    void DelArc(TokArr from, TokArr to);
    
    void SetPChance(TokArr value, TokArr prob, TokArr parentValue = TokArr());
    void SetPDecision(TokArr value, TokArr prob, TokArr parentValue = TokArr());
    void SetValueCost(TokArr value, TokArr prob, TokArr parentValue = TokArr());

    TokArr GetPChance(TokArr value, TokArr parents = TokArr());
    TokArr GetPDecision(TokArr value, TokArr parents = TokArr());
    TokArr GetValueCost(TokArr value, TokArr parents = TokArr());

    void SaveNet(const char *filename);
    void LoadNet(const char *filename);

    void SetProperty(const char *name, const char *value);
    String GetProperty(const char *name) const;

    // Set max amount of iterations
    void SetIterMax(int IterMax);

    // Get final expectation
    TokArr GetExpectation();

    // Get all politics
    TokArr GetPolitics();

    ProbabilisticNet &Net() const { return *m_pLIMID; }

    pnl::CLIMIDInfEngine& Inference();
    pnl::CIDNet &Model();

private:
    TokArr GetP(TokArr value, TokArr parents = TokArr());

    // handles messages - from ModelEngine interface
    virtual void DoNotify(const Message &msg);

private:// DATA members

    pnl::CLIMIDInfEngine *m_Inf;// inference, if it exists
    ProbabilisticNet *m_pLIMID;
};

PNLW_END

#endif //__LIMID_HPP__
