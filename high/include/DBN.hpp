#ifndef __DBN_HPP__
#define __DBN_HPP__

#include <string>
#include "pnlHighConf.hpp"
#include "pnlTok.hpp"
#include "Wcsv.hpp"
#include "BNet.hpp"


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
    class CDBN;
    class CDynamicInfEngine;
    class CDynamicLearningEngine;
    class CMlDynamicStructLearn;
    class pnlString;
    template<typename Type> class CMatrix;
}

PNLW_BEGIN

class PNLHIGH_API DBN
{
public:
    DBN();
    ~DBN();

    // add node
    // examples: AddNode( "categoric^wheather", "sunny rainy" );
    //           AddNode( categoric
    //               ^ "PreviousCompTurn CurrentHumanTurn PreviousHumanTurn"),
    //               "Rock Paper Scissors");
    void AddNode(TokArr nodes, TokArr subnodes);
    
    // remove node
    void DelNode(TokArr nodes);  
	
    TokArr GetNeighbors(TokArr nodes);
    TokArr GetParents(TokArr nodes);
    TokArr GetChildren(TokArr nodes);

    // returns one of "categoric" or "continuous"
    TokArr GetNodeType(TokArr nodes);
    
    // manipulating arcs
    void AddArc(TokArr from, TokArr to);
    void DelArc(TokArr from, TokArr to);
    
#ifndef SEPARATE_FUNS_FOR_NEIG
    // possible values for 'id' are:
    //   "parents", "children", "ancestors", "descedants" and so on
    TokArr Neighbours(const char *id = "parents", TokArr nodes = TokArr());
#else
    TokArr Parents(TokArr nodes);
    TokArr Children(TokArr nodes);
    TokArr Ancestors(TokArr nodes);
    TokArr Descendants(TokArr nodes);
#endif
    
    // set tabular probability
    void SetPTabular(TokArr value, TokArr prob, TokArr parentValue = TokArr());

    // get tabular probability
    TokArr GetPTabular(TokArr value, TokArr parents = TokArr());
    
    TokArr GetJPD(TokArr value );
    // set parameters for gaussian distribution
    void SetPGaussian(TokArr var, TokArr mean = TokArr(), TokArr variance = TokArr(), TokArr weight = TokArr());
    
    // adds evidence to the buffer
    void AddEvidToBuf(TokArr values);

    // setting evidence on the board 
    void EditEvidence(TokArr values = TokArr());
    
    // stores given evidence to the history
    void PushEvid(TokArr const values[], int nValue);
    
    // clears current evidence
    void ClearEvid();
    
    // clears evidence history
    void ClearEvidBuf();

    // stores current evidence to the buffer
    void CurEvidToBuf();

    void SetNumSlices(int nSlices);
    // sets number of slices

    int GetNumSlices();
    // returnes number of slices
    
    // learn using evidences buffer
    void LearnParameters();

	// unrolls current dbn
    BayesNet* Unroll();

    // checking net model for dbn creation 
    bool IsFullDBN();

    bool IsDBNContainNode(TokArr node);

    // returns MPE for nodes using current evidence
    TokArr GetMPE(TokArr nodes = TokArr());
    
    // get parameters of gaussian distribution
    TokArr GetGaussianMean(TokArr node);
    TokArr GetGaussianCovar(TokArr node, TokArr vars);
    
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
    void GenerateEvidences(TokArr numSlices);
    
    ProbabilisticNet &Net() const { return *m_pNet; }

    void SetProperty(const char *name, const char *value);
    String GetProperty(const char *name) const;

private:
    pnl::CMatrix<float> *Matrix(int iNode) const;
    pnl::CDynamicInfEngine &Inference();
    pnl::CDynamicLearningEngine &Learning();

    int GetSliceNum(String nodeName);
	String GetNodeName(String nodeEvid);
	String GetShortName(String nodeName);
	String GetValue(String nodeEvid);
	TokArr ConvertBNetQueToDBNQue(TokArr bnetQue,int nSlice);
	pnl::intVector GetSlicesNodesCorrespInd();


    void CreateModel();
    pnl::CDBN *Model();

    const char PropertyAbbrev(const char *name) const;

private:// DATA members
    pnl::CDynamicInfEngine *m_Inference;// inference, if it exists
    pnl::CDynamicLearningEngine *m_Learning;   // learning, if it exists
    int m_nLearnedEvidence;
    int m_curSlice;

    pnl::pEvidencesVecVector m_AllEvidences; //all evidences matrix 

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
