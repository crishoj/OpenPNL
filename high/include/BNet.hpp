#ifndef __BNET_HPP__
#define __BNET_HPP__

#include <string>
#include "pnlHighConf.hpp"
#include "pnlTok.hpp"
#include "WCover.hpp"

// Tok identifies for categoric and continuous node types
extern PNLHIGH_API TokArr categoric;
extern PNLHIGH_API TokArr continuous;

// FORWARDS
class WDistribFun;
namespace pnl
{
    class CPNLBase;
    class CEvidence;
    class CGraph;
    class CBNet;
    class CInfEngine;
    class CBayesLearningEngine;
    class pnlString;
}

#include "pnl_dll.hpp"

#define INDEX(pTok) ((char*)pTok->data - (char*)0)

// information for concrete node during model building
class TmpNodeInfo
{
public:
    TmpNodeInfo();
    TmpNodeInfo(bool isDiscrete, int size)
	: m_pDistribFun(0), m_NodeType(isDiscrete, size) {}

    Vector<TokIdNode *> m_aParent;
    WDistribFun *m_pDistribFun;
    pnl::CNodeType m_NodeType;
};

class PermanentNodeInfo
{
public:
    PermanentNodeInfo(TokIdNode *node): m_SelfNode(node) {}

    TokIdNode *m_SelfNode;
};

class PNLHIGH_API WModelInfo
{
public:
    Vector<TmpNodeInfo> m_aNode;
};

class PNLHIGH_API WEvid
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

class PNLHIGH_API BayesNet: public pnl::CPNLBase
{
public:
    BayesNet();
    ~BayesNet();
    friend class PersistWBNet;
    friend class Scripting;

    // add node
    // examples: AddNode( "categoric^wheather", "sunny rainy" );
    //           AddNode( categoric
    //               ^ "PreviousCompTurn CurrentHumanTurn PreviousHumanTurn"),
    //               "Rock Paper Scissors");
    void AddNode(TokArr nodes, TokArr subnodes);
    
    // remove node
    //void DelNode(TokArr nodes);
    
    // returns one of "categoric" or "continuous"
    TokArr NodeType(TokArr nodes);
    
    // manipulating arcs
    void AddArc(TokArr from, TokArr to);
    //void DelArc(TokArr from, TokArr to);
    
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
    TokArr JPD(TokArr value );
    // set parameters for gaussian distribution
    void SetGaussian(TokArr var, TokArr mean = TokArr(), TokArr variance = TokArr());
    
    // setting evidence on the board (bPush == false) or to the history
    void Evid(TokArr values = TokArr(), bool bPush = false);
    
    // stores given evidence to the history
    void PushEvid(TokArr const values[], int nValue);
    
    // clears current evidence
    void ClearEvid();
    
    // clears evidence history
    void ClearEvidHistory();
    
    // learns using current evidence and evidence history
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
    
    // get parameters of gaussian distribution
    TokArr GaussianMean(TokArr vars);
    TokArr GaussianCovar(TokArr var, TokArr vars);
    
    typedef enum
    {	eCSV
    ,	eTSV
    //  ,	eXML
    } ESavingType;
    
    void SaveNet(const char *filename);
    int SaveLearnBuf(const char *filename, ESavingType mode = eCSV);
    // or SaveEvidences or SaveLearnData or SaveHistory or SaveEvidHistory
    void LoadNet(const char *filename);
    int LoadLearnBuf(const char *filename, ESavingType mode = eCSV, TokArr colons = TokArr());
    // other variants same as after SaveLearnBuf

    // sets all distributions to uniform;
    // This function temporary here - I think it should be external function
    void MakeUniformDistribution();

    //add several evidences to learning buffer
    //nSamples - number of evidences to generate
    //whatNodes - which nodes should be included. By default - all
    //ignoreCurrEvid - if 'false', then current evidence specifies some fixed values
    //                 if 'true',then no fixed values while generating evidences
    void GenerateEvidences( int nSamples, bool ignoreCurrEvid = false, TokArr whatNodes = "");
    
    //This function hides nodes of current learning buffer with given probability
    //The function applies for all existing values (hidden and observed), 
    //i.e. observed may become hidden and hidden may become observed
    //By default this function unhides all hidden values 
    //If a node did not have sample then it can not be unhidden
    void MaskEvidences( TokArr whatNodes = "");


    
private:
    typedef Vector<int> IIMap;

    void CheckState(int funcId, int characteristic, const char *funcName);
    void AddArc(TokIdNode *from, TokIdNode *to);
    WDistribFun *CreateDistribFun(TokIdNode *node, WDistribFun *pFun = 0);
    void CreateGraph();
    void CreateModel();
    pnl::CEvidence *CreateEvidence(TokArr &aValue);
    int nBayesNode() const;
    void SplitNodesByObservityFlag(Vector<int> *aiObserved, Vector<int> *aiUnobserved);
    Vector<TokIdNode*> ExtractNodes(TokArr &aValue) const;

    void MustBeNode(TokArr &nodes) const;
    bool IsNode(Tok &node) const;
    void Resolve(Tok &from) const;
    WModelInfo *ModelInfo() const;
    int NodesClassification(TokArr &aValue) const;
    CMatrix<float> *Matrix(int iNode) const;
    void Accumulate(TokArr *pResult, Vector<int> &aIndex,
	pnl::CMatrix<float> *mat, String &prtName, int prtValue) const;
    pnl::CInfEngine &Inference();

private:// Bayes node name <-> index
    String NodeName(int iNode) const;
    int NodeIndex(const char *name) const;
    int NodeIndex(TokIdNode *name) const;
    TokIdNode *TokNodeByIndex(int i) const;
    String DiscreteValue(int iNode, int value) const;
    pnl::CNodeType pnlNodeType(int i);
    void ExtractTokArr(TokArr &aNode, Vector<int> *paiNode,
	Vector<int> *paiValue, IIMap *pMap = 0) const;
    static int GetInt(TokIdNode *node);
    void RebindFrom(BayesNet *bnet);

private:
    
    TokArr CutReq( Vector<int>& queryNds, Vector<int>& queryVls, 
			const CMatrix<float> * mat ) const;

private:// DATA members
    
    // Tree for bnet:
    //
    //		   / categoric
    // bnet - nodes
    //		   \ continuous
    TokIdNode *m_pRoot;		// pointer to root node of bnet
    TokIdNode *m_aNode;		// pointer to node of all bnet nodes
    TokIdNode *m_pCategoric;	// pointer to parent node for all categoric nodes
    TokIdNode *m_pContinuous;	// pointer to parent node for all continuous nodes
    short m_Objects;		// bitmap mask with flags for present objects (used in CheckState())
				// See first enum in .cpp
    short m_State;		// state of bnet (for example eModelBuilding)
    pnl::CGraph *m_Graph;	// graph, if it exists
    pnl::CBNet  *m_Model;	// model, if it exists
    pnl::CInfEngine *m_Inference;// inference, if it exists
    pnl::CBayesLearningEngine *m_Learning;   // learning, if it exists
    WEvid m_EvidenceBoard;      // board for evidence (see diagram for evidence buffer)
    Vector<pnl::CEvidence *> m_aEvidenceBuf;// buffer for evidences
    int m_nLearnedEvidence;

    mutable WModelInfo *m_pModelInfo;	// pointer to model information. Using is prohibited after model building
    Vector<PermanentNodeInfo> m_aNodeInfo;// vector of permanent Bayes node information
    std::map<String, int> m_aiNode;// map for 'node name' -> 'node index' translation
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
