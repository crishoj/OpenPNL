#ifndef __LIMID_HPP__
#define __LIMID_HPP__

#include <string>
#include "pnlHighConf.hpp"
#include "pnlTok.hpp"

// FORWARDS
PNLW_BEGIN
class WGraph;
class TokenCover;
class WDistributions;
class NetCallback;
class ProbabilisticNet;
PNLW_END

using namespace pnlw;

namespace pnl
{
    class CPNLBase;
    class CGraph;
    class CIDNet;
    class CLIMIDInfEngine;
    class pnlString;
    template<typename Type> class CMatrix;
}

class PNLHIGH_API LIMID
{
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

private:
    pnl::CMatrix<float> *Matrix(int iNode) const;
    pnl::CLIMIDInfEngine& Inference();
    pnl::CIDNet *Model();
    TokArr GetP(TokArr value, TokArr parents = TokArr());

private:// DATA members

    pnl::CLIMIDInfEngine *m_Inf;// inference, if it exists
    ProbabilisticNet *m_pLIMID;
};

#endif //__LIMID_HPP__
