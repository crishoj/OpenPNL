#ifndef __WCLIQUES_HPP__
#define __WCLIQUES_HPP__

#ifndef __PNLHIGHCONF_HPP__
#include "pnlHighConf.hpp"
#endif
#include "ModelEngine.hpp"

PNLW_BEGIN

// FORWARDS
class WGraph;
class TokIdNode;

class PNLHIGH_API WCliques: public ModelEngine
{
ALLOW_TESTING
public:
    typedef std::multimap<int, int> Map;

    WCliques(WGraph *graph);
    virtual ~WCliques();
    bool FormClique(const Vector<int> &aIndex);
    bool DestroyClique(const Vector<int> &aIndex);
    Vector<int> ClqNumbersForNode(int iNode);
    int nClique() const { return m_aCliques.size(); }
    Vector<Vector<int> > Cliques() const { return m_aCliques; }
    void GetClique(int iClique, Vector<int> *pClique) const
    { *pClique = m_aCliques[iClique]; }
    int iClique(const Vector<int> &aIndex) const;
    int iClique(const Vector<TokIdNode *> &aIndex) const;

public:// inlines for access to object fields
    WGraph *Graph() const { return m_pGraph; }

protected:
    virtual void DoNotify(const Message &msg);
    virtual int InterestedIn() const { return Message::eMSGDelNode; }

private:
    int hash(const Vector<int> &aIndex) const;	// hash function that maps set of 
						// nodes numbers to integer number
    bool IsSubClique(const Vector<int> &aiNode);    // check if nodes make subclique 
                                                    // or if some clique is subset of nodes
private:// DATA members
    WGraph *m_pGraph;			// Stores names of nodes and graph without eages
    Vector<Vector<int> > m_aCliques;	// vector of cliques
    Map m_HashTable;                    // hash table for searching clique
};

PNLW_END

#endif //__CLIQUES_HPP__
