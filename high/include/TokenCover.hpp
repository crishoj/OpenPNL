#ifndef __TOKENCOVER_HPP__
#define __TOKENCOVER_HPP__

#include <map>
#ifndef __PNLHIGHCONFIG_HPP__
#include "pnlHighConf.hpp"
#endif
#ifndef __TOKENS_HPP__
#include "Tokens.hpp"
#endif
#include "ModelEngine.hpp"

// FORWARDS
PNLW_BEGIN
class TokIdNode;
class WGraph;
class WDistribFun;
PNLW_END

namespace pnl
{
    class CPNLBase;
    class CGraph;
    class pnlString;
    class CNodeType;
};

PNLW_BEGIN

class PNLHIGH_API TokenCover: public ModelEngine
{
ALLOW_TESTING
public:
    TokenCover(TokIdNode *root, WGraph *graph);
    TokenCover(const char *rootName, WGraph *graph, bool bAutoNum = false);
    TokenCover(const char *rootName, const TokenCover &tokCovFrom, bool bAutoNum = false);
    virtual ~TokenCover();
    TokIdNode *Root() const { return m_pRoot; }
    void Resolve(Tok &tok) const;
    void Resolve(TokArr &aTok) const;
    void SetContext(Tok &tok) const;
    void SetContext(TokArr &tok) const;

    int AddNode(String &nodeName);
    void AddNode(TokArr &nodes, TokArr &values);
    int AddNode(Tok &node, TokArr &values);
    bool DelNode(int iNode);
    bool DelNode(Tok &nodeName);
    int nValue(int iNode);
    void GetValues(int iNode, Vector<String> &aValue);
    void SetValues(int iNode, const Vector<String> &aValue);

    String Value(int iNode, int iValue) const;
    void SetValue(int iNode, int iValue, String &value);

    TokIdNode *Node(int iNode) const;
    TokIdNode *Node(const Tok &node) const;
    Vector<TokIdNode*> Nodes(Vector<int> aiNode);
    WGraph *Graph() const { return m_pGraph; }

    Vector<TokIdNode*> ExtractNodes(TokArr &aValue) const;
    Vector<TokIdNode*> ExtractNodes(Tok &aValue) const;
    int NodesClassification(TokArr &aValue) const;

    int iNode(Tok &tok) const;
    Vector<int> aiNode(Tok &tok) const;
    // Properties support
    void AddProperty(const char *name, const char **aValue, int nValue);
    void GetPropertyVariants(const char *name, Vector<String> &aValue) const;

    void SetGraph(WGraph *graph, bool bStableNamesNotIndices);

    int Index(Tok &tok);
    static int SIndex(Tok &tok);
    static int Index(TokIdNode *node);
    static bool IsDiscrete(const TokIdNode *node);

    Tok TokByNodeValue(int iNode, int iValue);

protected:
    virtual void DoNotify(const Message &msg);
    virtual int InterestedIn() const { return Message::eMSGDelNode | Message::eChangeName; }

private:// FUNCTIONS
    void CreateRoot(const char *rootName, bool bAutoNum = false);
    static bool CopyRecursive(TokIdNode *to, const TokIdNode *from);
    void SetValues(TokIdNode *node, const Vector<String> &aValue, int iNode);
    void KillChildren(TokIdNode *node);
    void TuneNodeValue(TokIdNode *pValue, int iValue);
    TokIdNode *NodeValue(int iNode, int iValue) const;

private:// DATA
    // Tree for any net:
    //
    //		   / discrete
    //  net - nodes
    //		   \ continuous
    TokIdNode *m_pRoot;		// pointer to root node of bnet
    TokIdNode *m_aNode;		// pointer to node of all bnet nodes
    TokIdNode *m_pDiscrete;	// pointer to parent node for all discrete nodes
    TokIdNode *m_pContinuous;	// pointer to parent node for all continuous nodes
    TokIdNode *m_pDefault;	// pointer to default node
    TokIdNode *m_pProperties;	// pointer to properties namespace

    TokIdNode *m_pChance;	
    TokIdNode *m_pDecision;	
    TokIdNode *m_pValue;	

    WGraph *m_pGraph;		// serve graph and naming
};

PNLW_END

#endif //__TOKENCOVER_HPP__
