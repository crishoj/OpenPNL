#include "pnlTok.hpp"
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

WGraph::WGraph(): m_pGraph(0), m_bTouched(false)
{}

WGraph::WGraph(const WGraph &g): m_iNodeMap(g.m_iNodeMap),
    m_aNode(g.m_aNode), m_aParent(g.m_aParent),
    m_aUnusedIndex(g.m_aUnusedIndex), m_pGraph(0),
    m_bTouched(false), m_abValid(g.m_abValid),
    m_IndicesGraphToOuter(g.m_IndicesGraphToOuter),
    m_IndicesOuterToGraph(g.m_IndicesOuterToGraph)
{
}

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
		nbrsTypesList[i].resize(nodes/2);
	    }
	}
	
	m_IndicesGraphToOuter.resize(nodes);
	m_IndicesOuterToGraph.resize(m_aNode.size());
	for(i = j = 0; i < m_aNode.size(); ++i)
	{
	    if(m_abValid[i] == 0)
	    {
		continue;
	    }
	    m_IndicesGraphToOuter[j] = i;
	    m_IndicesOuterToGraph[i] = j++;
	}

	PNL_CHECK_FOR_NON_ZERO(j - nodes);

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
    m_iNodeMap[nodeName] = iNode;
    m_abValid[iNode] = 1;
    Notify(eInit, iNode);

    return iNode;
}

bool WGraph::DelNode(int iNode)
{// Assume that this operation is exotic
    if(!IsValidINode(iNode))
    {
	return false;
    }
    Notify(eDelNode, iNode);
    m_aUnusedIndex.push_back(iNode);
    m_abValid[iNode] = 0;
    m_aParent[iNode].resize(0);
    m_iNodeMap.erase(m_aNode[iNode]);
    for(int i = 0; i < m_aParent.size(); ++i)
    {
	Vector<int> &aParent = m_aParent[i];
	// unefficient block. Assume that this operation is exotic
	for(int j = 0; j < aParent.size(); j++)
	{
	    if(aParent[j] == iNode)
	    {
		aParent.erase(aParent.begin() + j);
		Notify(eChangeParentNState, i);
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
    Notify(eChangeParentNState, to);
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
    Notify(eChangeParentNState, to);
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

bool WGraph::IsValidINode(int iNode) const
{
    return iNode >= 0 && iNode < m_aNode.size() && m_abValid[iNode] != 0;
}

void WGraph::IndicesGraphToOuter(Vector<int> *outer, Vector<int> *iGraph)
{
    register int i = iGraph->size();
    for(outer->resize(i); --i >= 0; (*outer)[i] = m_IndicesGraphToOuter[(*iGraph)[i]]);
}

int WGraph::INode(String &name) const
{
    MapSI::const_iterator it = m_iNodeMap.find(name);
    return (it != m_iNodeMap.end()) ? it->second:-1;
}

String& WGraph::NodeName(int iNode)
{
    return IsValidINode(iNode) ? m_aNode[iNode]:m_Bad;
}

bool WGraph::SetNodeName(int iNode, String &name)
{
    if(IsValidINode(iNode))
    {
	m_iNodeMap.erase(NodeName(iNode));
	m_iNodeMap[name] = iNode;
	m_aNode[iNode] = name;
	Notify(eChangeName, iNode);
	return true;
    }
    return false;
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
    int i, j;

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

void WGraph::Reset(pnl::CGraph &graph)
{
    if(graph.GetNumberOfNodes() != nNode())
    {
	ThrowUsingError("Graph have different number of vertices", "Graph::Reset");
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
    int nParent;
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
	Notify(eChangeParentNState, i);
    }
}