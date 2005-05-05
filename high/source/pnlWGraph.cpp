#include "Tokens.hpp"
#include "pnlWGraph.hpp"
#pragma warning(push, 2)
#pragma warning(disable: 4251)
// class X needs to have dll-interface to be used by clients of class Y
#include "pnl_dll.hpp"
#pragma warning(default: 4251)
#pragma warning(pop)
#include "WInner.hpp"

#if defined(_MSC_VER)
#pragma warning(disable : 4239) // nonstandard extension used: 'T' to 'T&'
#endif

PNLW_BEGIN

bool
TopologicalSort::GetOrderReverse(IIMap *pResult, const Vector<Vector<int> > &aParent,
				const Vector<char> &abValid)
{
    std::vector<int> aNode;
    int candidate;
    bool bChange;
    int i, j, all;
    IIMap &result = *pResult;

    aNode.resize(abValid.size(), 0);
    result.assign(aNode.size(), -1);
    for(all = i = 0; i < abValid.size(); ++i)
    {
	if(!abValid[i])
	{
	    aNode[i] = -1;
	}
	else
	{
	    ++all;
	}
    }
    for(i = 0; i < all;)
    {
	candidate = -1;
	bChange = false;
	// walk thru all nodes and add nodes without non-marked parent
	// to topological order vector
	for(j = 0; j < aNode.size(); ++j)
	{
	    if(aNode[j] < 0)
	    {
		continue;
	    }

	    register int k;

	    for(k = aParent[j].size(); --k >= 0 && aNode[aParent[j][k]] == -1;);
	    if(k < 0)
	    {
		result[i++] = j;
		aNode[j] = -1;
		bChange = true;
	    }
	    else
	    {
		candidate = aParent[j][k];
	    }
	}

	if(!bChange)
	{
	    if(candidate >= 0)
	    {
		result[i++] = candidate;
		aNode[candidate] = -1;
	    }
	    else
	    {
		ThrowInternalError("incorrect algo of topological sort",
		    "TopologicalSort::GetOrderDirect");
	    }
	}
    }

    return true;
}

bool
TopologicalSort::GetOrder(IIMap *pDirect, IIMap *pReverse,
			  const Vector<Vector<int> > &aParent,
			  const Vector<char> &abValid)
{
    bool result = GetOrderReverse(pReverse, aParent, abValid);
    int i, j;

    if(!result)
    {
	return false;
    }
    pDirect->assign((i = pReverse->size()), -1);
    for(; --i >= 0;)
    {
	j = (*pReverse)[i];
	if(j < 0)
	{
	    continue;
	}
	(*pDirect)[j] = i;
    }
    
    return true;
}

bool
TopologicalSortDBN::GetOrderReverse(IIMap *pResult, const Vector<Vector<int> > &aParent,
				   const Vector<char> &abValid)
{
    std::vector<int> aNode, aNodeSecSlice;
    int candidate;
    bool bChange;
    int i, j, jh, half;
    IIMap &result = *pResult;
    Vector<int>::const_iterator it, itEnd;

    aNode.resize(abValid.size(), 0);
    aNodeSecSlice.resize(abValid.size(), -1);
    result.assign(aNode.size(), -1);
    half = 0;
    for(i = 0; i < abValid.size(); ++i)
    {
	if(!abValid[i])
	{
	    aNode[m_Map[i]] = -1;
	}
	else
	{
	    half++;
	}
    }
    if(half != m_Map.size())
    {
	ThrowInternalError("inconsistence between mapping and graph",
	    "TopologicalSortDBN::GetOrderDirect");
    }
    half >>= 1;
    // mark second slice nodes
    for(i = 0; i < half; ++i)
    {
	aNode[m_Map[half + i]] = 1;
	aNodeSecSlice[m_Map[i]] = m_Map[i + half];
    }
    for(i = 0; i < half;)
    {
	candidate = -1;
	bChange = false;
	for(j = 0; j < aNode.size(); ++j)
	{
	    if(aNode[j] != 0)
	    {
		continue;
	    }

	    it = aParent[j].begin();
	    itEnd = aParent[j].end();
	    for(; it != itEnd && aNode[*it] == -1; it++);
	    if(it != itEnd)
	    {
		candidate = *it;
		continue;
	    }
	    jh = aNodeSecSlice[j];
	    it = aParent[jh].begin();
	    itEnd = aParent[jh].end();
	    for(; (it != itEnd) && (aNode[*it] != 1); it++);
	    if(it != itEnd)
	    {
		candidate = j;
		continue;
	    }
	    result[i] = j;
	    result[i + half] = jh;
	    aNode[j] = aNode[jh] = -1;
	    i++;
	    bChange = true;
	}

	if(!bChange)
	{
	    if(candidate >= 0)
	    {
		result[i] = candidate;
		result[i + half] = aNodeSecSlice[candidate];
		aNode[candidate] = aNode[aNodeSecSlice[candidate]] = -1;
		i++;
	    }
	    else
	    {
		ThrowInternalError("incorrect algo of topological sort",
		    "TopologicalSortDBN::GetOrderDirect");
	    }
	}
    }

    return true;
}

WGraph::WGraph(): m_pGraph(0), m_bTouched(false), m_pSort(0)
{
}




#if 0
WGraph::WGraph(const WGraph &g):
#ifdef MAP_USAGE
m_iNodeMap(g.m_iNodeMap),
#else 
m_NNames(g.m_NNames), m_NIndexes(g.m_NIndexes),
#endif
m_aNode(g.m_aNode), m_aParent(g.m_aParent),
m_aUnusedIndex(g.m_aUnusedIndex), m_pGraph(0),
m_bTouched(false), m_abValid(g.m_abValid),
m_IndicesGraphToOuter(g.m_IndicesGraphToOuter),
m_IndicesOuterToGraph(g.m_IndicesOuterToGraph)
{

}
#endif

pnl::CGraph *WGraph::Graph(bool bForget)
{
    if(m_pGraph && m_bTouched)
    {
	delete m_pGraph;
	m_pGraph = 0;
    }

    if(!m_pGraph)
    {
	pnl::intVecVector nbrsList;
	pnl::neighborTypeVecVector nbrsTypesList;
	int nodes = nNode();
	int i, j, k, nParent;
	
	nbrsList.resize(nodes);
	nbrsTypesList.resize(nodes);
	
	if(nodes > 8)
	{
	    // to speedup allocating memory
	    for(i = nodes; --i >= 0;)
	    {
		nbrsList[i].reserve(nodes/2);
		nbrsTypesList[i].reserve(nodes/2);
	    }
	}

	if(!m_pSort)
	{
	    m_pSort = new TopologicalSort;
	}
	
	m_pSort->GetOrder(&m_IndicesOuterToGraph, &m_IndicesGraphToOuter, m_aParent, m_abValid);

	for(i = nodes; --i >= 0;)
	{
	    register int ii = m_IndicesGraphToOuter[i];

	    nParent = m_aParent[ii].size();
	    
	    for(j = nParent; --j >= 0;)
	    {
		k = m_IndicesOuterToGraph[m_aParent[ii][j]];
		nbrsList[i].push_back(k);
		nbrsTypesList[i].push_back(pnl::ntParent);
		nbrsList[k].push_back(i);
		nbrsTypesList[k].push_back(pnl::ntChild);
	    }
	}
	try
	{
	    m_pGraph = pnl::CGraph::Create(nbrsList, nbrsTypesList);
	}
	catch(...)
	{
	}
	m_bTouched = false;
    }

    pnl::CGraph *pGraph = m_pGraph;
    //pGraph->Dump();

    if(bForget)
    {
	m_pGraph = 0;
    }

    return pGraph;
}

int WGraph::AddNode(String &nodeName)
{
    int iNode;

    m_bTouched = true;
    if(m_aUnusedIndex.size())
    {
	iNode = m_aUnusedIndex.back();
	m_aUnusedIndex.pop_back();
    }
    else
    {
	iNode = m_aNode.size();
	m_aNode.resize(iNode + 1);
	m_abValid.resize(iNode + 1);
	m_aParent.resize(iNode + 1);
    }
    m_aNode[iNode] = nodeName;
#ifdef MAP_USAGE
	m_iNodeMap[nodeName] = iNode;
#else
	m_NNames.push_back(nodeName);
	m_NIndexes.push_back(iNode);
#endif
	m_abValid[iNode] = 1;
    Notify(Message::eInit, iNode);

    return iNode;
}

bool WGraph::DelNode(int iNode)
{// Assume that this operation is exotic
    if(!IsValidINode(iNode))
    {
	return false;
    }
    Notify(Message::eMSGDelNode, iNode);
    m_aUnusedIndex.push_back(iNode);
    m_abValid[iNode] = 0;
    m_aParent[iNode].resize(0);
	
	int i;
#ifdef MAP_USAGE
	m_iNodeMap.erase(m_aNode[iNode]);
#else
	for(i = 0; i < m_NIndexes.size(); i++)
	{
		if(m_aNode[iNode] == m_NNames[i])
		{
			m_NNames[i] = m_NNames[m_NNames.size() - 1];
			m_NIndexes[i] = m_NIndexes[m_NIndexes.size() - 1];
			m_NNames.resize(m_NNames.size() - 1);
			m_NIndexes.resize(m_NIndexes.size() - 1);
			break;
		}
	}
#endif

    for( i = 0; i < m_aParent.size(); ++i)
    {
	Vector<int> &aParent = m_aParent[i];
	// unefficient block. Assume that this operation is exotic
	for(int j = 0; j < aParent.size(); j++)
	{
	    if(aParent[j] == iNode)
	    {
		aParent.erase(aParent.begin() + j);
		Notify(Message::eChangeParentNState, i);
		break;
	    }
	}
    }

    m_bTouched = true;
    return true;
}

bool WGraph::AddArc(int from, int to)
{
    if(!IsValidINode(from) || !IsValidINode(to))
    {
	return false;
    }
    m_aParent[to].push_back(from);

    m_bTouched = true;
    Notify(Message::eChangeParentNState, to);
    return true;
}

bool WGraph::DelArc(int from, int to)
{
    int delIndex;
    if(!IsValidINode(from) || !IsValidINode(to))
    {
	return false;
    }
    Vector<int> &v = m_aParent[to];
    delIndex = std::find(v.begin(), v.end(), from) - v.begin();	
    if(delIndex == v.size())
    {// not found
	return false;
    }

    v[delIndex] = v.back();
    v.pop_back();

    m_bTouched = true;
    Notify(Message::eChangeParentNState, to);
    return true;
}

bool WGraph::AddArc(const char *from, const char *to)
{
    return AddArc(INode(from), INode(to));
}

bool WGraph::DelArc(const char *from, const char *to)
{
    return DelArc(INode(from), INode(to));
}

int WGraph::INode(const String &name) const
{
#ifdef MAP_USAGE
    MapSI::const_iterator it = m_iNodeMap.find(name);
    return (it != m_iNodeMap.end()) ? it->second:-1;
#else
	int i;
	for (i = 0; i < m_NIndexes.size(); i++)
	{
		if(m_NNames[i] == name)
		{
			return m_NIndexes[i];
		}
	}
	return -1;
#endif
}

String& WGraph::NodeName(int iNode)
{
    return IsValidINode(iNode) ? m_aNode[iNode]:m_Bad;
}

Vector<String> WGraph::NodeNames(Vector<int> &aIndex) const
{
    Vector<String> result;
    int i, j;

    result.reserve(aIndex.size());
    for(i = 0; i < aIndex.size(); i++)
    {
	j = aIndex[i];
	if(IsValidINode(j))
	{
	    result.push_back(m_aNode[j]);
	}
    }

    return result;
}

Vector<String> WGraph::Names(Vector<int> *paIndex) const
{
    Vector<String> result;
    int i;

    result.reserve(nNode());
    if(paIndex)
    {
	paIndex->reserve(nNode());
    }

    for(i = 0; i < m_aNode.size(); ++i)
    {
	if(m_abValid[i] == 0)
	{
	    continue;
	}
	result.push_back(m_aNode[i]);
	if(paIndex)
	{
	    paIndex->push_back(i);
	}
    }

    return result;
}

bool WGraph::SetNodeName(int iNode, String &name)
{
    if(IsValidINode(iNode))
    {
#ifdef MAP_USAGE
	m_iNodeMap.erase(NodeName(iNode));
    m_iNodeMap[name] = iNode;
    m_aNode[iNode] = name;

#else
	int i;
	for(i = 0; i < m_NIndexes.size(); i++)
	{
		if(iNode == m_NIndexes[i])
		{
			m_NNames[i] = name;
			m_aNode[iNode] = name;
			break;
		}
	}
#endif

	Notify(Message::eChangeName, iNode);
	return true;
    }
    return false;
}

void WGraph::GetParents(Vector<int> *parents, int iNode) const
{
    if(IsValidINode(iNode))
    {
	(*parents) = m_aParent[iNode];
    }
}

int WGraph::nParent(int iNode) const
{
    return IsValidINode(iNode) ? m_aParent[iNode].size():0;
}

void WGraph::GetChildren(Vector<int> *children, int iNode)
{
    if(!IsValidINode(iNode))
    {
	return;
    }
    pnl::CGraph *graph = Graph();

    graph->GetChildren(IGraph(iNode), children);
    IndicesGraphToOuter(children, children);
}

int WGraph::nChild(int iNode)
{
    if(!IsValidINode(iNode))
    {
	return 0;
    }
    pnl::CGraph *graph = Graph();
    return graph->GetNumberOfChildren(IGraph(iNode));
}

void WGraph::IndicesGraphToOuter(Vector<int> *outer, Vector<int> *iGraph)
{
    if(m_bTouched)
    {
	Graph();
    }

    register int i = iGraph->size();
    for(outer->resize(i); --i >= 0; (*outer)[i] = m_IndicesGraphToOuter[(*iGraph)[i]]);
}

bool WGraph::IsValidINode(int iNode) const
{
    return iNode >= 0 && iNode < m_aNode.size() && m_abValid[iNode] != 0;
}

void WGraph::Reset(pnl::CGraph &graph)
{
    if(graph.GetNumberOfNodes() != nNode())
    {
	ThrowUsingError("Graph have different number of vertices", "Graph::Reset");
    }

    if(m_bTouched)
    {
	Graph();// build graph
    }

    bool bUseMap = (m_IndicesGraphToOuter.size() == nNode()) && (m_IndicesOuterToGraph.size() == nNode());

    if(!bUseMap && m_aUnusedIndex.size())
    {
	ThrowUsingError("graph with holes and without index mapping", "Graph::Reset");
    }

    m_bTouched = true;
    m_pGraph = 0;

    int iInner, i;
    Vector<int> aParent;
    for(i = 0; i < nNode(); ++i)
    {
	iInner = bUseMap ? m_IndicesGraphToOuter[i] : i;
	m_aParent[iInner].resize(0);
	graph.GetParents(i, &aParent);
	m_aParent[iInner].resize(aParent.size());
	for(int j = 0; j < aParent.size(); ++j)
	{
	    m_aParent[iInner][j] = bUseMap ? m_IndicesGraphToOuter[aParent[j]] : aParent[j];
	}
    }
    for(i = 0; i < nNode(); ++i)
    {
	Notify(Message::eChangeParentNState, i);
    }
}

void WGraph::IGraph(const Vector<int> *iNodes, Vector<int> *iGraph)
{
    if ((iNodes == NULL)||(iGraph == NULL))
    {
	ThrowUsingError("NULL pointers", "WGraph::IGraph");
    }

    if(m_bTouched)
    {
	Graph();// build graph
    }

    int size = iNodes->size();
    int node = 0;
    iGraph->resize(size);
    for (node = 0; node < size; node++)
    {
	(*iGraph)[node] = m_IndicesOuterToGraph.at((*iNodes)[node]);
    }
}

void WGraph::IOuter(const Vector<int> *iGraph, Vector<int> *iNodes)
{
    if ((iNodes == NULL)||(iGraph == NULL))
    {
	ThrowUsingError("NULL pointers", "WGraph::IGraph");
    }

    if(m_bTouched)
    {
	Graph();// build graph
    }

    int size = iGraph->size();
    int node = 0;
    iNodes->resize(size);
    for (node = 0; node < size; node++)
    {
	(*iNodes)[node] = m_IndicesGraphToOuter.at((*iGraph)[node]);
    }
}

PNLW_END
