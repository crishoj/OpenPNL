#ifndef __WCLIQUES_HPP__
#define __WCLIQUES_HPP__

#include "ModelEngine.hpp"

PNLW_BEGIN

// FORWARDS
class WGraph;

class PNLHIGH_API WCliques: public ModelEngine
{
public:
    typedef std::multimap<int, int> Map;

    WCliques(WGraph *graph);
    bool FormClique(const Vector<int> &aIndex);
    bool DestroyClique(const Vector<int> &aIndex);
    Vector<int> ClqNumbersForNode(int iNode);
    int nClique() const { return m_aCliques.size(); }
    Vector<Vector<int> > Cliques() const { return m_aCliques; }
    void GetClique(int iClique, Vector<int> *pClique) const
    { *pClique = m_aCliques[iClique]; }

public:// inlines for access to object fields
    WGraph *Graph() const { return m_pGraph; }

protected:
    virtual void DoNotify(int message, int iNode, ModelEngine *pObj);
    virtual int InterestedIn() const { return eDelNode; }

private:
    int iClique(const Vector<int> &aIndex) const;
    int hash(const Vector<int> &aIndex) const;	// hash function that maps set of 
						//nodes numbers to integer number
private:// DATA members
    WGraph *m_pGraph;			// Stores names of nodes and graph without eages
    Vector<Vector<int> > m_aCliques;	// vector of cliques
    //Vector<pair<int, int> > hashTable;	// hash table for searching clique
    Map m_HashTable;
};

PNLW_END

#endif //__CLIQUES_HPP__
