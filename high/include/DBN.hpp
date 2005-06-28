#ifndef __DBN_HPP__
#define __DBN_HPP__

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
class BayesNet;
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

class PNLHIGH_API DBN: public ModelEngine
{
ALLOW_TESTING
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
    
    // set tabular probability
    void SetPTabular(TokArr value, TokArr prob, TokArr parentValue = TokArr());

    // get tabular probability
    TokArr GetPTabular(TokArr value, TokArr parents = TokArr());
    
    TokArr GetJPD(TokArr value );
    // set parameters for gaussian distribution
    void SetPGaussian(TokArr var, TokArr mean = TokArr(), TokArr variance = TokArr(), TokArr weight = TokArr());
    
	// returns criterion value for last learning performance
    float GetEMLearningCriterionValue();

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

	// sets lag for fixlagsmoothing prosedure
	void SetLag(int lag);

	//returnes lag
	int GetLag();
    
    // learn using evidences buffer
    void LearnParameters();

	// unrolls current dbn
    BayesNet* Unroll();

    // checking net model for dbn creation 
    bool IsFullDBN();

    bool IsDBNContainNode(TokArr node);

    // returns MPE for nodes using current evidence
    TokArr GetMPE(TokArr nodes);
    
    // get parameters of gaussian distribution
    TokArr GetGaussianMean(TokArr node);
    TokArr GetGaussianCovar(TokArr node);
	TokArr GetGaussianWeights(TokArr node, TokArr parent);
    
    void SaveNet(const char *filename);
    int SaveEvidBuf(const char *filename, NetConst::ESavingType mode = NetConst::eCSV);
    void LoadNet(const char *filename);
    int LoadEvidBuf(const char *filename, NetConst::ESavingType mode = NetConst::eCSV, TokArr columns = TokArr());

    // generates array of evidences 
    void GenerateEvidences(TokArr numSlices);
    
    ProbabilisticNet &Net() const { return *m_pNet; }

    void SetProperty(const char *name, const char *value);
    String GetProperty(const char *name) const;

    pnl::CDynamicInfEngine &Inference();
    pnl::CDynamicLearningEngine &Learning();
    pnl::CDBN &Model();
    pnl::pEvidencesVecVector GetPNLEvidences();

private:
    pnl::CMatrix<float> *Matrix(int iNode) const;

    // handles messages - from ModelEngine interface
    virtual void DoNotify(const Message &msg);

    int GetSliceNum(String nodeName);
    String GetNodeName(String nodeEvid);
    String GetShortName(String nodeName);
    String GetValue(String nodeEvid);
    TokArr ConvertBNetQueToDBNQue(TokArr bnetQue,int nSlice);
    pnl::intVector GetSlicesNodesCorrespInd();

    const char PropertyAbbrev(const char *name) const;

private:// DATA members
    pnl::CDynamicInfEngine *m_Inference;// inference, if it exists
    pnl::CDynamicLearningEngine *m_Learning;   // learning, if it exists
    int m_nLearnedEvidence;
    int m_curSlice;
	int m_lag;

    pnl::intVecVector m_AllEvidences; //all evidences matrix 

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

#endif //__DBN_HPP__
