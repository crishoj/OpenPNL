#ifndef __PNLWGRAPH_HPP__
#define __PNLWGRAPH_HPP__

#include "pnlHighConf.hpp"
#include "ModelEngine.hpp"

// FORWARDS
struct TokIdNode;
class WDistribFun;

namespace pnl
{
    class CPNLBase;
    class CGraph;
    class pnlString;
}

class PNLHIGH_API WGraph: public ModelEngine
{
public:
    WGraph();
    WGraph(const WGraph &g);

    int nNode() const		// return number of nodes in graph
    { return m_aNode.size() - m_aUnusedIndex.size(); }
    pnl::CGraph *Graph(bool bForget = false);
    int AddNode(String &nodeName);
    bool DelNode(int iNode);	// Assume that this operation is exotic
    bool AddArc(int from, int to);
    bool AddArc(const char *from, const char *to);
    bool DelArc(int from, int to);
    bool DelArc(const char *from, const char *to);

    int INode(String &name) const;
    int INode(const char *name) const { return INode(String(name)); }
    
    String& NodeName(int iNode);
    bool SetNodeName(int iNode, String &name);
    Vector<String> NodeNames(Vector<int> &aIndex) const;
    Vector<String> Names(Vector<int> *paIndex = 0) const;

    void GetParents(Vector<int> *parents, int iNode) const;
    int nParent(int iNode) const;
    void GetChildren(Vector<int> *children, int iNode);
    int nChild(int iNode);

    int IGraph(int iNode) { return m_IndicesOuterToGraph.at(iNode); }
    void IndicesGraphToOuter(Vector<int> *outer, Vector<int> *iGraph);

    bool IsValidINode(int iNode) const;

    void Reset(pnl::CGraph &graph);

protected:
    typedef std::map<String, int> MapSI;
    virtual void DoNotify(int message, int iNode, ModelEngine *pObj) {}
    virtual int InterestedIn() const { return 0; /* nothing intrested in */ }

private:
    MapSI m_iNodeMap;		// map for 'node name' -> 'node index' translation
    Vector<String> m_aNode;	// names of nodes (this vector may have unused
				// elements after deleting)
    Vector<Vector<int> > m_aParent;// indices for parents
    Vector<int> m_aUnusedIndex;	// unused indices
    pnl::CGraph *m_pGraph;	// graph representing current state (if exists)
    bool m_bTouched;		// if true and graph exists, then graph must be re-created
    String m_Bad;		// used as NULL (error may be checked somehow else)
    Vector<int> m_IndicesGraphToOuter;
    Vector<int> m_IndicesOuterToGraph;
    Vector<char> m_abValid;
};

#endif //__PNLWGRAPH_HPP__
