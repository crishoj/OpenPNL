#include "pnlWGraph.hpp"
#include "TokenCover.hpp"

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

TokenCover::TokenCover(const char *rootName, WGraph *graph, bool bAutoNum): m_pGraph(graph)
{
    CreateRoot(rootName, bAutoNum);

    (m_aNode = m_pRoot->Add("nodes"))->tag = eTagService;
    (m_pProperties = m_pRoot->Add("properties"))->tag = eTagService;
    (m_pDiscrete  = m_aNode->Add("discrete")) ->tag = eTagNodeType;
    (m_pContinuous = m_aNode->Add("continuous"))->tag = eTagNodeType;
    m_pDefault = m_pDiscrete;

    m_pChance = m_pDiscrete->Add("chance");	
    m_pDecision = m_pDiscrete->Add("decision");		
    m_pValue = m_pDiscrete->Add("value");	
    
    m_pChance->tag = eTagNodeType;
    m_pDecision->tag = eTagNodeType;
    m_pValue->tag = eTagNodeType;

    m_pChance->data = (void *)(new pnl::CNodeType(1, 2));
    m_pDecision->data = (void *)(new pnl::CNodeType(1, 2, pnl::nsDecision));
    m_pValue->data =  (void *)(new pnl::CNodeType(1, 1, pnl::nsValue));

    m_pDiscrete->data = (void *)(new pnl::CNodeType(1, 2));
    m_pContinuous->data = (void *)( new pnl::CNodeType(0, 1));
    SpyTo(m_pGraph);
}

TokenCover::TokenCover(TokIdNode *root, WGraph *graph):
    m_pRoot(root), m_pGraph(graph)
{
    m_aNode = Tok("nodes").Node(root);
    m_pDiscrete  = Tok("discrete").Node(m_aNode);
    //if(!m_pDiscrete || m_pDiscrete->tag != eTagNodeType) throw();
    m_pContinuous = Tok("continuous").Node(m_aNode);
    //if(!m_pContinuous || m_pContinuous->tag != eTagNodeType) throw();
    m_pProperties = Tok("properties").Node(m_pRoot);
    //if(!m_pProperties || m_pProperties->tag != eTagService) throw();
    SpyTo(m_pGraph);
}

TokenCover::TokenCover(const char *rootName, const TokenCover &tokCovFrom, bool bAutoNum)
{
    CreateRoot(rootName, bAutoNum);
    if(!CopyRecursive(m_pRoot, tokCovFrom.m_pRoot))
    {
	ThrowInternalError("error during Token recursive coping",
	    "TovenCover::TocenCover");
    }
    m_pGraph = new WGraph(*tokCovFrom.m_pGraph);
    SpyTo(m_pGraph);
}

TokenCover::~TokenCover()
{
    m_pRoot->Kill();
}

void TokenCover::CreateRoot(const char *rootName, bool bAutoNum)
{
    TokIdNode *root = flatSearch(TokIdNode::root, rootName);
    if(!root)
    {
	root = TokIdNode::root->Add(rootName);
	root->tag = eTagRootClassification;
    }

    if(bAutoNum)
    {
	// search for first unfilled slot in rootName
	int i;
	pnl::pnlString s;
	for(i = 0; flatSearch(root, (s << i).c_str()); ++i, s.resize(0));
	
	// create substructure
	m_pRoot = root->Add(s.c_str());
    }
    else
    {
	m_pRoot = root;
    }
    m_pRoot->tag = eTagNet;
    m_pRoot->data = this;
}

void TokenCover::Resolve(Tok &tok) const
{
    tok.Resolve(Root());
}

void TokenCover::Resolve(TokArr &aTok) const
{
    for(int i = aTok.size(); --i >= 0; aTok[i].Resolve(Root()));
}

void TokenCover::SetContext(Tok &tok) const
{
    tok.FixContext(Root());
}

void TokenCover::SetContext(TokArr &aTok) const
{
    for(int i = aTok.size(); --i >= 0; aTok[i].FixContext(Root()));
}

bool TokenCover::CopyRecursive(TokIdNode *to, const TokIdNode *from)
{
    TokIdNode *childFrom, *childTo;

    try
    {
	for(childFrom = from->v_next; childFrom; childFrom = childFrom->h_next)
	{
	    childTo = to->Add(childFrom->id[0]);
	    childTo->tag = childFrom->tag;
	    for(int i = 1; i < childFrom->id.size(); ++i)
	    {
		childTo->Alias(childFrom->id[i]);
	    }
	    if(!CopyRecursive(childTo, childFrom))
	    {
		return false;
	    }
	}
    }
    catch(...)
    {
	return false;
    }

    return true;
}

int TokenCover::AddNode(String &nodeName)
{
    Tok t(nodeName);
    TokArr ta("True False");
    return AddNode(t, ta);
}

void TokenCover::AddNode(TokArr &nodes, TokArr &values)
{
    int i;

    for(i = 0; i < nodes.size(); i++)
    {

	AddNode(nodes[i], values);
    }
}

int TokenCover::AddNode(Tok &node, TokArr &aValue)
{
    TokIdNode *parentNode = m_pRoot;
    String nodeName = node.Name();

#ifdef DEBUG_ADDNODE
    const std::deque<TokId> &aUnr = node.Unresolved(parentNode);
    pnl::pnlString str;
    
    str << nodeName << "#" << node.Node(parentNode)->tag << ":";
    for(int i = 0; i < aUnr.size(); ++i)
    {
        str << aUnr[i] << " ";
    }
    PrintTokTree("toktree.txt", TokIdNode::root);
#endif

    if(node.Unresolved(parentNode).size() != 1)
    {
        pnl::pnlString str;
        str << '\'' << nodeName << "' is not a node";
        ThrowUsingError(str.c_str(), "AddNode");
    }

    //If token is unresolved, set default type (discrete)
    if(node.node.empty())
    {
	parentNode = m_pDefault;
    }
    else 
    {
	parentNode = node.Node(parentNode);
    }


    // add Bayes vertex
    TokIdNode *tokNode = parentNode->Add(nodeName);
    tokNode->tag = eTagNetNode;
    TokIdNode *pValue;

    if (tokNode->v_prev == m_pContinuous)
    {
       if (aValue.size() == 0)     
           aValue << "dim1";
    }

    for(int j = 0; j < aValue.size(); ++j)
    { // add vertex's values (or dimension names)
	pValue = tokNode->Add(aValue[j].Name());
	TuneNodeValue(pValue, j);
    }

    int result = 0;

    if(m_pGraph)
    {
	result = m_pGraph->AddNode(nodeName);	
	tokNode->Alias(result);
    }

    return result;
}

bool TokenCover::DelNode(int iNode)
{
    //Notify(Message::eMSGDelNode, iNode);
    if(!m_pGraph)
    {
	ThrowInternalError("Call to TokenCover::DelNode without graph", "DelNode");
	return false;
    }

    TokIdNode *node = Tok(m_pGraph->NodeName(iNode)).Node(m_aNode);
    if(!node)
    {
	return false;
    }
    m_pGraph->DelNode(iNode);
    node->Kill();

    return true;
}

bool TokenCover::DelNode(Tok &nodeName)
{
    String name(nodeName.Name());

    TokIdNode *node = nodeName.Node(m_aNode);

    if(!node)
    {
	return false;
    }

    node->Kill();

    MaskAddOrDelete(Message::eMSGDelNode, true);
    bool result = m_pGraph && !m_pGraph->DelNode(m_pGraph->INode(name.c_str()));
    MaskAddOrDelete(Message::eMSGDelNode, false);

    return result;
}

int TokenCover::nValue(int iNode)
{
    TokIdNode *value = Node(iNode)->v_next;
    int i;

    for(i = 0; value; value = value->h_next, ++i);

    return i;
}

TokIdNode *TokenCover::Node(int iNode) const
{
    if(!m_pGraph)
    {
	ThrowInternalError("This call requires graph", "Node");
	return 0;
    }

    return Tok(m_pGraph->NodeName(iNode)).Node(m_aNode);
}

TokIdNode *TokenCover::Node(const Tok &node) const
{
    return node.Node(m_aNode);
}

void TokenCover::SetValues(int iNode, const Vector<String> &aValue)
{
    SetValues(Node(iNode), aValue, iNode);
}

void TokenCover::SetValues(TokIdNode *node, const Vector<String> &aValue, int iNode)
{
    TokIdNode *pValue;

    KillChildren(node);
    for(int j = 0; j < aValue.size(); ++j)
    { // add vertex's values (or dimension names)
	pValue = node->Add(aValue[j]);
	TuneNodeValue(pValue, j);
    }
    Notify(Message::eChangeNState, iNode);
}

void TokenCover::SetValue(int iNode, int iValue, String &value)
{
}

TokIdNode *TokenCover::NodeValue(int iNode, int iValue) const
{
#ifndef SEARCH_IN_NODE
    TokIdNode *node = Node(iNode);
    
    node = node->v_next;
    for(; node; node = node->h_next)
    {
	if(node->Match(TokId(iValue)))
	{
	    return node;
	}
    }

    return 0;
#else
    return Tok(iValue).Node(Node(iNode));
#endif
}

void TokenCover::TuneNodeValue(TokIdNode *pValue, int iValue)
{
    pValue->tag = eTagValue;
    pValue->Alias(iValue);
}

void TokenCover::KillChildren(TokIdNode *node)
{
    Vector<TokIdNode *> children;
    int i;

    children.reserve(16);
    for(i = 0, node = node->v_next; node; node = node->h_next)
    {
	children.push_back(node);
    }

    for(i = children.size(); --i >= 0; children[i]->Kill());
}

String TokenCover::Value(int iNode, int iValue) const
{
    return NodeValue(iNode, iValue)->Name();
}

void TokenCover::GetValues(int iNode, Vector<String> &aValue)
{
    TokIdNode *node = Node(iNode);
    TokIdNode *value = node->v_next;
    int i;

    for(i = 0; value; value = value->h_next, ++i);
    aValue.resize(i);
    for(value = node->v_next; --i >= 0; value = value->h_next)
    {
	aValue[i] = value->Name();
    }
}

Vector<TokIdNode*> TokenCover::Nodes(Vector<int> aiNode)
{
    Vector<TokIdNode *> result;

    result.resize(aiNode.size());
    for(int i = aiNode.size(); --i >= 0;)
    {
	result[i] = Node(aiNode[i]);
    }
    return result;
}

void TokenCover::SetGraph(WGraph *graph, bool bStableNamesNotIndices)
{
    int i;
    TokIdNode *node;

    if(bStableNamesNotIndices)
    {
	Vector<String> aName(Graph()->Names());
	for(i = 0; i < aName.size(); ++i)
	{
	    node = Node(Tok(aName[i]));
	    node->Unalias(Graph()->INode(aName[i].c_str()));
	}

	StopSpyTo(m_pGraph);
	m_pGraph = graph;
	SpyTo(graph);

	aName = Graph()->Names();
	for(i = 0; i < aName.size(); ++i)
	{
	    node = Node(Tok(aName[i]));
	    node->Alias(Graph()->INode(aName[i].c_str()));
	}
    }
    else
    {
	ThrowInternalError("Not yet realized", "SetGraph(eSTABLE_INDICES)");
    }
}

Vector<TokIdNode*> TokenCover::ExtractNodes(TokArr &aValue) const
{
    Vector<TokIdNode*> result;
    for(int i = 0; i < aValue.size(); ++i)
    {
	TokIdNode *node = aValue[i].Node(m_aNode);
	if(node->tag == eTagValue)
	{
	    node = node->v_prev;
	}
	if(node->tag != eTagNetNode)
	{
	    ThrowUsingError("There is must be node", "ExtractNodes");
	}
	result.push_back(node);
    }

    return result;
}

Vector<TokIdNode*> TokenCover::ExtractNodes(Tok &aValue) const
{
    Vector<TokIdNode*> result;
    //int j = TokIdNode::root->desc.count(aValue.Name());
    Vector<TokIdNode *> nodes = aValue.Nodes(m_aNode);
    for(int i = 0; i < nodes.size(); ++i)
    {
        if(nodes[i]->tag == eTagValue)
        {
            nodes[i] = nodes[i]->v_prev;
        }
        if(nodes[i]->tag != eTagNetNode)
        {
            ThrowUsingError("There is must be node", "ExtractNodes");
        }
        result.push_back(nodes[i]);
    }
    
    return result;
}

int TokenCover::NodesClassification(TokArr &aValue) const
{
    int result = 0;
    int i;
    TokIdNode *node;
    bool IsDiscrete;

    for(i = 0; i < aValue.size(); ++i)
    {
	node = aValue[i].Node(m_aNode);
	while(node && node->tag != eTagNodeType)
	{
	    node = node->v_prev;
	}
	if(!node)
	{
	    result |= eNodeClassUnknown;
	    continue;
	}
        void *tmp = node->data;
        IsDiscrete = ((pnl::CNodeType*)tmp)->IsDiscrete();
        
        if (IsDiscrete == 1)
            result |= eNodeClassDiscrete;
        else
            result |= eNodeClassContinuous;

    }

    return result;
}

int TokenCover::iNode(Tok &tok) const
{
    Resolve(tok);
    TokIdNode *node = tok.Node();
    if(node->tag == eTagValue)
    {
	node = node->v_prev;
    }
    if(node->tag != eTagNetNode)
    {
	ThrowInternalError("Node doesn't contain BayesNode", "iNode");
    }

    return Index(node);
}

Vector<int> TokenCover::aiNode(Tok &tok) const
{
    Resolve(tok);
    Vector<TokIdNode *> nodes = tok.Nodes();
    Vector<int> indices;
    int i;
    for(i = 0; i < nodes.size(); i++)
    {
        if(nodes[i]->tag == eTagValue)
        {
            nodes[i] = nodes[i]->v_prev;
        }
        if(nodes[i]->tag != eTagNetNode)
        {
            ThrowInternalError("nodes[i] doesn't contain Bayesnode", "aiNode");
        }
        indices.push_back(Index(nodes[i]));
    }

    return indices;
}

void TokenCover::AddProperty(const char *name, const char **aValue, int nValue)
{
    TokIdNode *node = m_pProperties->Add(name);

    for(int i = 0; i < nValue; ++i)
    {
	node->Add(aValue[i]);
    }
}

void TokenCover::GetPropertyVariants(const char *name, Vector<String> &aValue) const
{
    TokIdNode *node = Tok(name).Node();

    aValue.resize(0);
    if(!node || !(node = node->v_next))
    {
	return;
    }
    aValue.reserve(8);

    for(; node; node = node->h_next)
    {
	aValue.push_back(node->Name());
    }
}

int TokenCover::Index(Tok &tok)
{
    return Index(tok.Node(Root()));
}

int TokenCover::SIndex(Tok &tok)
{
    return Index(tok.Node());
}

int TokenCover::Index(TokIdNode *node)
{
    for(int i = node->id.size(); --i >= 0;)
    {
	TokId &tid = node->id[i];
	if(tid.is_int)
	{
	    return tid.int_id;
	}
    }

    ThrowInternalError("must have integer id (may be wrong Tok?)", "GetInt");
    return -1;
}

bool TokenCover::IsDiscrete(const TokIdNode *node)
{
    TokIdNode *pType = node->v_prev;
    if(!pType || pType->tag != eTagNodeType || !pType->data)
    {
	ThrowInternalError("wrong node (bad parent)", "TokenCover::IsDiscrete");
    }
    return ((pnl::CNodeType*)pType->data)->IsDiscrete();
}

Tok TokenCover::TokByNodeValue(int iNode, int iValue)
{
    return Tok(Graph()->NodeName(iNode)) ^ Value(iNode, iValue);
}

// non-public functions

void TokenCover::DoNotify(const Message &msg)
{
    switch(msg.MessageId())
    {
    case Message::eChangeName:
	{
	    PNL_CHECK_IS_NULL_POINTER(m_pGraph);
	    Tok tokNode(msg.IntArg());
	    Tok::Matcher matcher(eTagNetNode);

	    tokNode.Resolve(m_aNode, &matcher);
	    
	    TokIdNode *node = tokNode.Node();
	    String name = m_pGraph->NodeName(msg.IntArg());
	    String oldName = node->Name();

	    node->Alias(name);
	    node->Unalias(oldName);
	    if(!(node->Name() == name))
	    {
		node->Unalias(msg.IntArg());
		node->Alias(msg.IntArg());
	    }
	    if(!(node->Name() == name))
	    {
		ThrowInternalError("can't rename node with token", "Notify::eChangeName");
	    }
	}
	break;
    case Message::eMSGDelNode:
	{// if user deletes node via WGraph
	    TokIdNode *node = Node(msg.IntArg());
	    if(!node)
	    {
		break;
	    }
	    if(&msg.Sender() != m_pGraph)
	    {
		ThrowInternalError("Unexpected source of DelNode message",
		    "TokenCover::Notify");
	    }
	    node->Kill();
	    break;
	}
    default:
	ThrowInternalError("Unhandled message arrive" ,"DoNotify");
	break;
    }
    return;
}

PNLW_END
