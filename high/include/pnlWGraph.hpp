#ifndef __PNLWGRAPH_HPP__
#define __PNLWGRAPH_HPP__

#include "pnlHighConf.hpp"
#include "ModelEngine.hpp"
#include "WInner.hpp"

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

    int iNodeMax() const	// return maximum index of iNode
    { return m_aNode.size() - 1; }

    // Return pointer to pnl::CGraph. Construct it if need.
    // If bForget is true - caller is responsible for deleting graph.
    // If bForget is false - pointer to graph is saved inside object and may
    // be deleted during further call of function
    pnl::CGraph *Graph(bool bForget = false);

    // Following group of functions constructs graph
    int AddNode(String &nodeName);
    bool DelNode(int iNode);	// Assume that this operation is exotic
    bool AddArc(int from, int to);
    bool AddArc(const char *from, const char *to);
    bool DelArc(int from, int to);
    bool DelArc(const char *from, const char *to);

    // translation: name of node => index
    int INode(String &name) const;
    int INode(const char *name) const { return INode(String(name)); }
    
    // translation: name of node <= index
    String& NodeName(int iNode);
    Vector<String> NodeNames(Vector<int> &aIndex) const;
    Vector<String> Names(Vector<int> *paIndex = 0) const;

    // set new name for node 'iNode'
    bool SetNodeName(int iNode, String &name);

    // get information about parents or children for some node
    void GetParents(Vector<int> *parents, int iNode) const;
    int nParent(int iNode) const;
    void GetChildren(Vector<int> *children, int iNode);
    int nChild(int iNode);

    // Graph has inner and outer node numbers.
    // Any index - outer, if there isn't explicit note.
    // But indices from pnl::CGraph or from Model differs from outer.
    // It is inner indices. Such indices must be translated before use.
    // Following functions perform translation between inner and outer views.
    int IGraph(int iNode) { return m_IndicesOuterToGraph.at(iNode); }
    int IOuter(int iGraph) { return m_IndicesGraphToOuter.at(iGraph); }
    void IndicesGraphToOuter(Vector<int> *outer, Vector<int> *iGraph);
    IIMap &MapOuterToGraph() { return m_IndicesOuterToGraph; }

    // Return true if node index is valid
    bool IsValidINode(int iNode) const;

    // Reset graph information (neighbours for each node) from the graph
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
    IIMap m_IndicesGraphToOuter;// map from inner to outer indices
    IIMap m_IndicesOuterToGraph;// map from outer to inner indices
    Vector<char> m_abValid;	// validity flag for nodes. This member mustn't be used
				// in functions except {IsValidINode, AddNode, DelNode}
};

#endif //__PNLWGRAPH_HPP__
