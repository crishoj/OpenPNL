#ifndef __DBN_HPP__
#define __DBN_HPP__

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
    class CDBN;
    class CDynamicInfEngine;
    class CDynamicLearningEngine;
    class CMlDynamicStructLearn;
    class pnlString;
    template<typename Type> class CMatrix;
}

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
    void SetP(TokArr value, TokArr prob, TokArr parentValue = TokArr());

    // get tabular probability
    TokArr P(TokArr value, TokArr parents = TokArr());
    
    TokArr GetJPD(TokArr value );
    // set parameters for gaussian distribution
    void SetGaussian(TokArr var, TokArr mean = TokArr(), TokArr variance = TokArr(), TokArr weight = TokArr());
    
    // setting evidence on the board 
    void EditEvidence(TokArr values = TokArr());
    
    // stores given evidence to the history
    void PushEvid(TokArr const values[], int nValue);
    
    // clears current evidence
    void ClearEvid();
    
    // clears evidence history
    void ClearEvidBuf();

	void SetNumSlices(int nSlices);
	// sets number of slices
    
    // learns using current evidence and evidence history
    void LearnParameters(TokArr aValue[] = NULL, int nValue = 0);

    // returns MPE for nodes using current evidence
    TokArr GetMPE(TokArr nodes = TokArr());
    
    // get parameters of gaussian distribution
    TokArr GetGaussianMean(TokArr vars);
    TokArr GetGaussianCovar(TokArr var, TokArr vars);
    
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
    void GenerateEvidences(pnl::intVector nSlices);
    
    //This function hides nodes of current learning buffer with given probability
    //The function applies for all existing values (hidden and observed), 
    //i.e. observed may become hidden and hidden may become observed
    //By default this function unhides all hidden values 
    //If a node did not have sample then it can not be unhidden
    void MaskEvidBuf( TokArr whatNodes = "");
    ProbabilisticNet &Net() const { return *m_pNet; }

    void SetProperty(const char *name, const char *value);
    String GetProperty(const char *name) const;

private:
    typedef Vector<int> IIMap;

    pnl::CMatrix<float> *Matrix(int iNode) const;
    pnl::CDynamicInfEngine &Inference();
    pnl::CDynamicLearningEngine &Learning();

    int GetSliceNum(String nodeName);
	String GetNodeName(String nodeEvid);
	String GetShortName(String nodeName);
	TokArr ConvertBNetQueToDBNQue(TokArr bnetQue,int nSlice);

    void CreateModel();
    pnl::CDBN *Model();

    const char PropertyAbbrev(const char *name) const;
//    void RebindFrom(DBN *bnet);

private:// DATA members
    pnl::CDynamicInfEngine *m_Inference;// inference, if it exists
    pnl::CDynamicLearningEngine *m_Learning;   // learning, if it exists
    int m_nLearnedEvidence;
    int m_nSlices;
//	int m_Lag;
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

#endif //__BNET_HPP__
