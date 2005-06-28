#ifndef __MRF_HPP__
#define __MRF_HPP__

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
class Cliques;
PNLW_END

namespace pnl
{
    class CPNLBase;
    class CEvidence;
    class CGraph;
    class CMNet;
    class CInfEngine;
    class CStaticLearningEngine;
//    class pnlString;
    template<typename Type> class CMatrix;
}

PNLW_BEGIN

class PNLHIGH_API MRF: public ModelEngine
{
ALLOW_TESTING
public:
    MRF();
    ~MRF();

    // add node
    // examples: AddNode( "discrete^wheather", "sunny rainy" );
    //           AddNode( discrete
    //               ^ "PreviousCompTurn CurrentHumanTurn PreviousHumanTurn"),
    //               "Rock Paper Scissors");
    void AddNode(TokArr nodes, TokArr subnodes);
    
    // remove node
    void DelNode(TokArr nodes);
    
    // returns one of "discrete" or "continuous"
    TokArr GetNodeType(TokArr nodes);
    
    // unit nodes to clique
    void SetClique(TokArr nodes);
    // destroy clique
    void DestroyClique(TokArr nodes);
    
    // returns number of nodes in network
    int GetNumberOfNodes() const;
    // returns number of cliques in network
    int GetNumberOfCliques() const;
    
    // set tabular probability
    void SetPTabular(TokArr value, TokArr prob);

    // get tabular probability
    TokArr GetPTabular(TokArr value);
    
    // set parameters for gaussian distribution
//    void SetPGaussian(TokArr node, TokArr mean = TokArr(), TokArr variance = TokArr(), TokArr weight = TokArr());
    
    // setting evidence on the board
    void EditEvidence(TokArr values);
    
    // clears current evidence
    void ClearEvid();
    
    // stores current evidence to the buffer
    void CurEvidToBuf();

    // adds evidence to the buffer
    void AddEvidToBuf(TokArr values);
    
    // clears evidence history
    void ClearEvidBuf();
    
    // learns distributions of the network using evidence buffer and new evidences 
    void LearnParameters(TokArr aValue[] = NULL, int nValue = 0);
    
    // returns MPE for nodes using current evidence
    TokArr GetMPE(TokArr nodes);
    
    // returns JPD for nodes using current evidence
    TokArr GetJPD(TokArr nodes);

    // get parameters of gaussian distribution
//    TokArr GetGaussianMean(TokArr nodes);
//    TokArr GetGaussianCovar(TokArr nodes);
    
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
    pnl::CMNet &Model();
    pnl::CEvidence *GetPNLEvidence();

private:
    pnl::CMatrix<float> *Matrix(int iNode) const;

    void SetInferenceProperties(TokArr &nodes);
    void SetParamLearningProperties();
    const char PropertyAbbrev(const char *name) const;
    TokArr GetFullScale(TokArr aValue) const;

    // handles messages - from ModelEngine interface
    virtual void DoNotify(const Message &msg);

private:// DATA members
    pnl::CInfEngine *m_Inference;// inference, if it exists
    pnl::CStaticLearningEngine *m_Learning;   // learning, if it exists
    int m_nLearnedEvidence;

    ProbabilisticNet *m_pNet;
    // controls Graph, Token, Distributions, Evidence Board, Evidence Buffer
};

PNLW_END

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

#endif //__MRF_HPP__
