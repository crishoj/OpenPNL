#ifndef __BNET_HPP__
#define __BNET_HPP__

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
    class CEvidence;
    class CGraph;
    class CBNet;
    class CLIMIDInfEngine;
    class CInfEngine;
    class CStaticLearningEngine;
    class pnlString;
    template<typename Type> class CMatrix;
}

PNLW_BEGIN

class PNLHIGH_API BayesNet: public ModelEngine
{
ALLOW_TESTING
public:
    BayesNet();
    ~BayesNet();

    // add node
    // examples: AddNode( "discrete^wheather", "sunny rainy" );
    //           AddNode( discrete
    //               ^ "PreviousCompTurn CurrentHumanTurn PreviousHumanTurn"),
    //               "Rock Paper Scissors");
    void AddNode(TokArr nodes, TokArr subnodes = TokArr());
    
    // remove node
    void DelNode(TokArr nodes);
    
    // returns one of "discrete" or "continuous"
    TokArr GetNodeType(TokArr nodes);
    
    // manipulating arcs
    void AddArc(TokArr from, TokArr to);
    void DelArc(TokArr from, TokArr to);
    
    TokArr GetNeighbors(TokArr nodes);
    TokArr GetParents(TokArr nodes);
    TokArr GetChildren(TokArr nodes);
    
    // set tabular probability
    void SetPTabular(TokArr value, TokArr prob, TokArr parentValue = TokArr());

    // get tabular probability
    TokArr GetPTabular(TokArr value, TokArr parents = TokArr());
    
    // set parameters for gaussian distribution
    void SetPGaussian(TokArr node, TokArr mean, TokArr variance, TokArr weight = TokArr(), TokArr tabParentValue = TokArr());
    
    void SetPSoftMax(TokArr node, TokArr weigth, TokArr offset, TokArr parentValue = TokArr());
    
    TokArr GetSoftMaxOffset(TokArr node, TokArr parent = TokArr());
    TokArr GetSoftMaxWeights(TokArr node, TokArr parent = TokArr());

    // setting evidence on the board
    void EditEvidence(TokArr values);
    
    // clears current evidence
    void ClearEvid();

    // returns logarithm of likelihood for current evidence
    float GetCurEvidenceLogLik();
    
    // returns array of logarithms of likelihood for evidences from buffer
    TokArr GetEvidBufLogLik();
    
    // stores current evidence to the buffer
    void CurEvidToBuf();

    // adds evidence to the buffer
    void AddEvidToBuf(TokArr values);
    
    // clears evidence history
    void ClearEvidBuf();
    
    // learns distributions of the network using evidence buffer and new evidences 
    void LearnParameters(TokArr aValue[] = NULL, int nValue = 0);
    
    //learns structure of the network using input evidences or evidence history
    void LearnStructure(TokArr aValue[] = NULL, int nValue = 0); //maybe return some quality measure?

    // returns criterion value for last learning performance
    float GetEMLearningCriterionValue();

    // returns MPE for nodes using current evidence
    TokArr GetMPE(TokArr nodes);
    
    // returns JPD for nodes using current evidence
    TokArr GetJPD(TokArr nodes);

    // get parameters of gaussian distribution
    TokArr GetGaussianMean(TokArr node, TokArr tabParentValue = TokArr());
    TokArr GetGaussianCovar(TokArr node, TokArr tabParentValue = TokArr());
    TokArr GetGaussianWeights(TokArr node, TokArr parent, TokArr tabParentValue = TokArr());
    
    void SaveNet(const char *filename);
    int SaveEvidBuf(const char *filename, NetConst::ESavingType mode = NetConst::eCSV);
    void LoadNet(const char *filename);
    int LoadEvidBuf(const char *filename, NetConst::ESavingType mode = NetConst::eCSV, TokArr columns = TokArr());

    // sets all distributions to uniform;
    // This function temporary here - I think it should be external function
    //void MakeUniformDistribution();

    //add several evidences to learning buffer
    //nSample - number of evidences to generate
    //whatNodes - which nodes should be included. By default - all
    //ignoreCurrEvid - if 'false', then current evidence specifies some fixed values
    //                 if 'true',then no fixed values while generating evidences
    void GenerateEvidences( int nSample, bool ignoreCurrEvid = false, TokArr whatNodes = "");
    
    //This function hides nodes of current learning buffer with given probability
    //The function applies for all existing values (hidden and observed), 
    //i.e. observed may become hidden and hidden may become observed
    //By default this function unhides all hidden values 
    //If a node did not have sample then it can not be unhidden
    void MaskEvidBuf(TokArr whatNodes = "");

    void SetProperty(const char *name, const char *value);
    String GetProperty(const char *name) const;

    ProbabilisticNet &Net() const { return *m_pNet; }

    pnl::CInfEngine &Inference(bool Recreate = false);
    pnl::CStaticLearningEngine &Learning();
    pnl::CBNet &Model();
    pnl::CEvidence *GetPNLEvidence();

private:
    // handles messages - from ModelEngine interface
    virtual void DoNotify(const Message &msg);

    void SetInferenceProperties(TokArr &nodes);
    void SetParamLearningProperties();
    const char PropertyAbbrev(const char *name) const;

private:// DATA members
    pnl::CInfEngine *m_Inference;// inference, if it exists
    pnl:: CStaticLearningEngine *m_Learning;   // learning, if it exists
    int m_nLearnedEvidence;

    ProbabilisticNet *m_pNet;
    // controls Graph, Token, Distributions, Evidence Board, Evidence Buffer
};

//
// Evidence Board       Sampling
//             |          |
//             |          |
//             V          V
//            Evidence Buffer
//             | ^        |
//             | |        |
//             V |        V
//            File      Learning
//

PNLW_END

#endif //__BNET_HPP__
