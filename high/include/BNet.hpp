#ifndef __BNET_HPP__
#define __BNET_HPP__

#include <string>
#include "pnlHighConf.hpp"
#include "pnlTok.hpp"

// FORWARDS
class WGraph;
class TokenCover;
class WDistributions;
class NetCallback;
class ProbabilisticNet;

namespace pnl
{
    class CPNLBase;
    class CEvidence;
    class CGraph;
    class CBNet;
    class CInfEngine;
    class CStaticLearningEngine;
    class pnlString;
    template<typename Type> class CMatrix;
}

class PNLHIGH_API BayesNet
{
public:
    BayesNet();
    ~BayesNet();

    // add node
    // examples: AddNode( "categoric^wheather", "sunny rainy" );
    //           AddNode( categoric
    //               ^ "PreviousCompTurn CurrentHumanTurn PreviousHumanTurn"),
    //               "Rock Paper Scissors");
    void AddNode(TokArr nodes, TokArr subnodes);
    
    // remove node
    void DelNode(TokArr nodes);
    
    // returns one of "categoric" or "continuous"
    TokArr NodeType(TokArr nodes);
    
    // manipulating arcs
    void AddArc(TokArr from, TokArr to);
    void DelArc(TokArr from, TokArr to);
    
#ifndef SEPARATE_FUNS_FOR_NEIG
    // possible values for 'id' are:
    //   "parents", "children", "ancestors", "descendants" and so on
    TokArr Neighbours(const char *id = "parents", TokArr nodes = TokArr());
#else
    TokArr Parents(TokArr nodes);
    TokArr Children(TokArr nodes);
    TokArr Ancestors(TokArr nodes);
    TokArr Descendants(TokArr nodes);
#endif
    
    // set tabular probability
    void SetP(TokArr value, TokArr prob, TokArr parentValue = TokArr());

    // get tabular probability
    TokArr P(TokArr value, TokArr parents = TokArr());
    
    // set parameters for gaussian distribution
    void SetGaussian(TokArr node, TokArr mean = TokArr(), TokArr variance = TokArr(), TokArr weight = TokArr());
    
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
    
    // learns using evidence history
    void Learn();
    void Learn(TokArr aValue[], int nValue);
    
    //learn structure of the network using input evidences or evidence history
#if 0 
    BayesNet* LearnStructure(TokArr aValue[], int nValue); //maybe return some quality measure?
#else
    void LearnStructure(TokArr aValue[], int nValue); //maybe return some quality measure?
#endif

    // returns MPE for nodes using current evidence
    TokArr MPE(TokArr nodes = TokArr());
    
    // returns JPD for nodes using current evidence
    TokArr JPD(TokArr nodes);

    // get parameters of gaussian distribution
    TokArr GaussianMean(TokArr nodes);
    TokArr GaussianCovar(TokArr var, TokArr vars);
    
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
    void MaskEvidences( TokArr whatNodes = "");
    ProbabilisticNet &Net() const { return *m_pNet; }

    void SetProperty(const char *name, const char *value);
    String Property(const char *name) const;

private:
    pnl::CMatrix<float> *Matrix(int iNode) const;
    pnl::CInfEngine &Inference();
    pnl::CStaticLearningEngine &Learning();

    void CreateModel();
    pnl::CBNet *Model();

    const char PropertyAbbrev(const char *name) const;
//    void RebindFrom(BayesNet *bnet);

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

#endif //__BNET_HPP__
