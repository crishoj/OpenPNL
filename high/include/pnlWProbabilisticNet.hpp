#ifndef __PNLWPROBABILISTICNET_HPP__
#define __PNLWPROBABILISTICNET_HPP__

#include <string>
#include "pnlHighConf.hpp"
#include "ModelEngine.hpp"
#include "pnlWEvidence.hpp"
#include "WInner.hpp"

// FORWARDS
PNLW_BEGIN
class WDistribFun;
class WGraph;
class TokenCover;
class WDistributions;
class WEvidence;
class NetCallback;
PNLW_END

namespace pnl
{
    class CPNLBase;
    class CEvidence;
    class CGraphicalModel;
    class CInfEngine;
    class CBayesLearningEngine;
    class pnlString;
    template<typename Type> class CMatrix;
    class CContextPersistence;
}

PNLW_BEGIN

class PNLHIGH_API ProbabilisticNet: protected ModelEngine
{
public:
    ProbabilisticNet(const char *netType = "bnet");
    virtual ~ProbabilisticNet();

    void SetCallback(NetCallback *pCallback) { m_pCallback = pCallback; }
    void AddNode(TokArr nodes, TokArr subnodes);
    void DelNode(TokArr nodes);
    TokArr GetNodeType(TokArr nodes);
    void AddArc(TokArr from, TokArr to);
    void DelArc(TokArr from, TokArr to);

    TokArr GetNeighbors(TokArr &nodes);
    TokArr GetParents(TokArr &nodes);
    TokArr GetChildren(TokArr &nodes);
    
    void EditEvidence(TokArr values);
    void ClearEvid();
    void CurEvidToBuf();
    void AddEvidToBuf(TokArr values);
    void ClearEvidBuf();

    int SaveEvidBuf(const char *filename, NetConst::ESavingType mode);
    int LoadEvidBuf(const char *filename, NetConst::ESavingType mode, TokArr columns);
    // other variants same as after SaveLearnBuf

    // save net. return true if successful saving
    bool SaveNet(pnl::CContextPersistence *saver);
    // load net. return true if successful loading
    static ProbabilisticNet* LoadNet(pnl::CContextPersistence *loader);

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
    void MaskEvidBuf( TokArr whatNodes = "");

    void SetProperty(const char *name, const char *value);
    String GetProperty(const char *name) const;

    // now assume that model has same number of node as 'this'
    void Reset(const pnl::CGraphicalModel &model);

public:
    pnl::CEvidence *CreateEvidence(const TokArr &aValue);
    void GetTokenByEvidence(TokArr *tEvidence, pnl::CEvidence *evidence);
    int nNetNode() const;
    void MustBeNode(TokArr &nodes) const;
    bool IsNode(Tok &node) const;
    WEvidence *EvidenceBoard() { return &m_EvidenceBoard; }
    Vector<pnl::CEvidence *> *EvidenceBuf() { return &m_aEvidence; }
    virtual void DoNotify(int message, int iNode, ModelEngine *pObj) {/* realize! */}

public:// Bayes node name (or TokIdNode) <-> index
    String NodeName(int iNode) const;
    int NodeIndex(const char *name) const;
    int NodeIndex(TokIdNode *name) const;
    TokIdNode *TokNodeByIndex(int i) const;
    String DiscreteValue(int iNode, int value) const;
    void ExtractTokArr(TokArr &aNode, Vector<int> *paiNode,
	Vector<int> *paiValue, IIMap *pMap = 0) const;
    static int GetInt(TokIdNode *node);

public:
    pnl::CNodeType pnlNodeType(int i);
    void Accumulate(TokArr *pResult, Vector<int> &aIndex,
	pnl::CMatrix<float> *mat, String &prtName, int prtValue) const;
    void SplitNodesByObservityFlag(Vector<int> *aiObserved, Vector<int> *aiUnobserved);
    TokArr CutReq( Vector<int>& queryNds, Vector<int>& queryVls, 
			const pnl::CMatrix<float> * mat ) const;

    int NodesClassification(TokArr &aValue) const;
    void SetTopologicalOrder(const int *renaming, pnl::CGraph *pnlGraph);

public:// inlines for access to object fields
    WGraph *Graph() const { return m_pGraph; }
    TokenCover *Token() const { return m_pTokenCov; }
    WDistributions *Distributions() const { return m_paDistribution; }
    pnl::CGraphicalModel *Model();
    void SetModel(pnl::CGraphicalModel* pModel);
    Tok ConvertMatrixToToken(const pnl::CMatrix<float> *mat);

private:
    int iNodeMax() const;

private:// DATA members
    // Tree for bnet:
    //
    //		   / discrete
    // bnet - nodes
    //		   \ continuous
    pnl::CGraphicalModel *m_Model;	    // model, if it exists
    WEvidence m_EvidenceBoard;		    // board for evidence (see diagram for evidence buffer)
    Vector<pnl::CEvidence *> m_aEvidence;   // buffer for evidences
    WDistributions *m_paDistribution;    // It holds all distributions
					    // It is moreover alters distribution as need
    TokenCover *m_pTokenCov;		    // token stuff
    WGraph *m_pGraph;			    // Stores graph and names of nodes

    typedef std::map<String, String> SSMap; // type for mapping string to string
    SSMap m_aPropertyValue;		    // Properties: value for every property

    NetCallback *m_pCallback;
};

PNLW_END

#endif //__PNLWPROBABILISTICNET_HPP__
