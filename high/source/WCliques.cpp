#include "Tokens.hpp"
#include "WCliques.hpp"
#include "pnlWGraph.hpp"

PNLW_BEGIN

void DumpMap(WCliques::Map *HashTable)
{
    std::cout << std::endl;
    WCliques::Map::iterator it;
    for(it = HashTable->begin(); it != HashTable->end(); it++)
    {
        std::cout << it->first << "  " << it->second << std::endl;
    }
}

WCliques::WCliques(WGraph *graph): m_pGraph(graph)
{
    SpyTo(graph);
}

WCliques::~WCliques()
{
}

// check if nodes make subclique or if some clique is subset of nodes
// nodes indices must be sorted 
bool WCliques::IsSubClique(const Vector<int> &aiNode)
{
    int i, j;
    for(i = 0; i < m_aCliques.size(); i++)
    {
        int maxSize = aiNode.size() < m_aCliques[i].size() ? aiNode.size() : m_aCliques[i].size();
        for(j = 0; j < maxSize; j++)
        {
            if(aiNode[j] != m_aCliques[i][j])
            {
                break;
            }
        }
        if(j == maxSize)
        {
            return true;
        }
    }
    return false;
}

bool WCliques::FormClique(const Vector<int> &aIndex)
{
    int i;
    for (i = 0; i < aIndex.size(); i++)
    {
	if(!(m_pGraph->IsValidINode(aIndex[i])))
	{
	    return false;
	}
    }
    Vector<int> tmp_vec_clique(aIndex.begin(), aIndex.end());
    std::sort(tmp_vec_clique.begin(), tmp_vec_clique.end());

    if(IsSubClique(tmp_vec_clique))
    {
        return false;
        //ThrowUsingError("Nodes can not be clique. Nodes are subset of some existing clique or some clique is subset of nodes.", "FormClique");
    }

    m_aCliques.push_back(tmp_vec_clique);
    m_HashTable.insert(std::make_pair(hash(tmp_vec_clique), m_aCliques.size() - 1));
    Notify(Message::eInit, m_aCliques.size() - 1);
    return true;
}

bool WCliques::DestroyClique(const Vector<int> &aIndex)
{
    int i;
    int hashIndex = hash(aIndex);
    Map::iterator it, itEnd;
    it = m_HashTable.find(hashIndex);
    if(it == m_HashTable.end())
    {
	return false;
    }
    itEnd = m_HashTable.upper_bound(hashIndex);
    for(; it != itEnd; it++)
    {
	int iClq = it->second;
	if(aIndex.size() != m_aCliques[iClq].size())
	{
	    continue;
	}
	bool isThisClique = true;
	for(i = 0; i < aIndex.size(); i++)
	{
	    if(std::find(m_aCliques[iClq].begin(), m_aCliques[iClq].end(), 
		aIndex[i]) == m_aCliques[iClq].end())
	    {
		isThisClique = false;
		break;
	    }
	}
        if(isThisClique)
        {
            if(iClq < m_aCliques.size() - 1)
            {
                m_aCliques[iClq] = m_aCliques[m_aCliques.size() - 1];
                // number of last clique is changed, we must change it in hash table too
                int indHash = hash(m_aCliques[iClq]);
                Map::iterator it1, itEnd1;
                it1 = m_HashTable.find(indHash);
                itEnd1 = m_HashTable.upper_bound(indHash);
                for(; it1 != itEnd1; it1++)
                {
                    if(it1->second == m_aCliques.size() - 1)
                    {
                        it1->second = iClq;
                        break;
                    }
                }
            }
            m_aCliques.resize(m_aCliques.size() - 1);
            m_HashTable.erase(it); // delete row of this clique from hash table
            return true;
        }
    }
    return false;
}

Vector<int> WCliques::ClqNumbersForNode(int iNode)
{
    if(!m_pGraph->IsValidINode(iNode))
    {
	pnl::pnlString str;
	str << '\'' << iNode << "' is not a node";
	ThrowUsingError(str.c_str(), "ClqNumbersForNode");
    }
    Vector<int> clqs;
    int i;
    for(i = 0; i < m_aCliques.size(); i++)
    {
	if(std::find(m_aCliques[i].begin(), m_aCliques[i].end(), iNode) 
	    != m_aCliques[i].end())
	{
	    clqs.push_back(i);
	}
    }
    return clqs;
}

int WCliques::iClique(const Vector<int> &aIndex) const
{
    int i;
    int hashIndex = hash(aIndex);
    Map::const_iterator it, itEnd;
    it = m_HashTable.find(hashIndex);
    if(it == m_HashTable.end())
    {
	return -1;
    }
    itEnd = m_HashTable.upper_bound(hashIndex);
    for(; it != itEnd; it++)
    {
	int cliqueNum = it->second;
	if(aIndex.size() != m_aCliques[cliqueNum].size())
	{
	    continue;
	}
	bool isThisClique = true;
	for(i = 0; i < aIndex.size(); i++)
	{
	    if(std::find(m_aCliques[cliqueNum].begin(), m_aCliques[cliqueNum].end(), 
		aIndex[i]) == m_aCliques[cliqueNum].end())
	    {
		isThisClique = false;
		break;
	    }
	}
	if(isThisClique)
	{
	    return cliqueNum;
	}
    }
    return -1;
}

int WCliques::iClique(const Vector<TokIdNode *> &aNode) const
{
    int i;
    int nNode = aNode.size();
    Vector<int> clique;
    
    clique.resize(nNode);
    for(i = 0; i < nNode; i++)
    {
        clique[i] = Graph()->INode(aNode[i]->Name());
    }
    return iClique(clique);
}

int WCliques::hash(const Vector<int> &aIndex) const
{
    int i;
    int res = 0;
    for(i = 0; i < aIndex.size(); i++)
    {
	res += aIndex[i] * aIndex[i] + 1000;
    }
    return res;
}

void WCliques::DoNotify(const Message &msg)
{
    switch(msg.MessageId())
    {
    case Message::eMSGDelNode:
        {
	    Vector<int> clqs = ClqNumbersForNode(msg.IntArg());
            int i, iClq;
            for(i = 0; i < clqs.size(); i++)
            {
                iClq = clqs[i];
                DestroyClique(m_aCliques[iClq]);
		Notify(Message::eMSGDelNode, iClq);
            }
            break;
        }
    default:
        ThrowInternalError("Unhandled message arrive" ,"DoNotify");
        return;
    }
}

PNLW_END
