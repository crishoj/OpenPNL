#include "pnlTok.hpp"
#include "BNet.hpp"
#include "WDistribFun.hpp"
#pragma warning(push, 2)
#pragma warning(disable: 4251)
// class X needs to have dll-interface to be used by clients of class Y
#include "pnl_dll.hpp"
#include "pnlMlStaticStructLearnHC.hpp"
#include "pnlString.hpp"
#pragma warning(default: 4251)
#pragma warning(pop)
#include "Wcsv.hpp"
#include "WInner.hpp"

#if defined(_MSC_VER)
#pragma warning(disable : 4239) // nonstandard extension used: 'T' to 'T&'
#endif

// Tok identifies for categoric and continuous node types
TokArr categoric("nodes^categoric");
TokArr continuous("nodes^continuous");

void BayesNet::Resolve(Tok &from) const
{
    from.Resolve(m_pRoot);
}

int BayesNet::NodesClassification(TokArr &aValue) const
{
    int result = 0;
    int i;
    TokIdNode *node;

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
	if(node == m_pCategoric)
	{
	    result |= eNodeClassCategoric;
	}
	else if(node == m_pContinuous)
	{
	    result |= eNodeClassContinuous;
	}
	else
	{
	    ThrowInternalError("Non-consistent node classification", "NodesClassification");
	}
    }

    return result;
}

BayesNet::BayesNet(): m_pRoot(0), m_aNode(0), m_pCategoric(0), m_pContinuous(0),
m_Objects(0), m_State(0), m_Graph(0), m_Model(0),
m_Inference(0), m_Learning(0), m_nLearnedEvidence(0), m_pModelInfo(0)
{
    // create node bnet and all descedants
    TokIdNode *bnet = flatSearch(TokIdNode::root, "bnet");
    if(!bnet)
    {
	bnet = TokIdNode::root->Add("bnet");
	bnet->tag = eTagRootClassification;
    }

    // search for first unfilled slot in "bnet"
    int i;
    pnl::pnlString s;
    for(i = 0; flatSearch(bnet, (s << i).c_str()); ++i, s.resize(0));

    // create substructure
    m_pRoot = bnet->Add(s.c_str());
    m_pRoot->tag = eTagNet;
    m_pRoot->data = this;

    (m_aNode = m_pRoot->Add("nodes"))->tag = eTagNodeType;
    (m_pCategoric  = m_aNode->Add("categoric")) ->tag = eTagNodeType;
    (m_pContinuous = m_aNode->Add("continuous"))->tag = eTagNodeType;
}

BayesNet::~BayesNet()
{
    m_pRoot->Kill();
    //delete m_Graph;
    
    delete m_Inference;
    delete m_Learning;
    delete m_Model;
    delete m_pModelInfo;
}

void BayesNet::MustBeNode(TokArr &nodes) const
{
    for(int i = nodes.size(); --i >= 0;)
    {
	Resolve(nodes[i]);
	if(!IsNode(nodes[i]))
	{
	    pnl::pnlString str;

	    str << '\'' << static_cast<String>(nodes[i]) << "' is not a node";
	    ThrowUsingError(str.c_str(), "MustBeNode");
	}
    }
}

bool BayesNet::IsNode(Tok &node) const
{
    return node.Node(m_pRoot)->tag == eTagNetNode;
}

// for Bayes Net nodes only
void BayesNet::AddNode(TokArr nodes, TokArr subnodes)
{
    CheckState(eAddNode, PROHIBIT(eAllObjects), "AddNode");

    WModelInfo *pBuildModel = ModelInfo();
    
    for(unsigned int i = 0; i < nodes.size(); ++i)
    {
	//Resolve(nodes[i]);
	Tok &tok = nodes[i];
	String s = tok;
	if(tok.Unresolved(m_pRoot).size() != 1)
	{
	    if(tok.Node(m_pRoot)->v_prev->tag != eTagNetNode) // ? what is it?
	    { // error if such node exists
		pnl::pnlString str;

		str << '\'' << static_cast<String>(tok) << "' is not a node";
		ThrowUsingError(str.c_str(), "AddNode");
	    }
	}

	bool isDiscrete = (tok.Node(m_pRoot) == m_pCategoric);
        // add Bayes vertex
	TokIdNode *node = tok.Node(m_pRoot)->Add(tok.Unresolved(m_pRoot)[0]);
	node->tag = eTagNetNode;

	TokIdNode *pValue;
	for(int j = 0; j < subnodes.size(); ++j)
	{ // add vertex's values (or dimension names)
	    pValue = node->Add(subnodes[j].Name());
	    pValue->tag = eTagValue;
	    pValue->data = (void*)((char*)0 + j);// set INDEX for node
	    pValue->Alias(j);
	}

	int iNode = nBayesNode();

	node->Alias(iNode);
	node->data = (void*)((char*)0 + iNode);// set INDEX for value
	pBuildModel->m_aNode.push_back(TmpNodeInfo(isDiscrete, subnodes.size()));
	m_aNodeInfo.push_back(PermanentNodeInfo(node));
	m_aiNode[node->Name()] = iNode;
    }
}

String BayesNet::NodeName(int iNode) const
{
    return m_aNodeInfo.at(iNode).m_SelfNode->Name();
}

int BayesNet::NodeIndex(const char *name) const
{
    std::map<String, int>::const_iterator loc = m_aiNode.find(String(name));

    if(loc == m_aiNode.end())
    {
	pnl::pnlString str;

	str << "No such node: '" << name << "'";

	ThrowUsingError(str.c_str(), "NodeIndex");
    }

    return loc->second;
}

int BayesNet::NodeIndex(TokIdNode *node) const
{
    return INDEX(node);
}

TokIdNode *BayesNet::TokNodeByIndex(int i) const
{
    return m_aNodeInfo.at(i).m_SelfNode;
}

pnl::CNodeType BayesNet::pnlNodeType(int i)
{
    return ModelInfo()->m_aNode.at(i).m_NodeType;
}

int BayesNet::nBayesNode() const
{
    return m_aNodeInfo.size();
}

// returns one of "categoric" or "continuous"
TokArr BayesNet::NodeType(TokArr nodes)
{
    CheckState(eNodeType, 0, "NodeType");
    TokArr res;
    MustBeNode(nodes);// check for existence of such vertices
    for(int i = 0; i < nodes.size(); ++i)
    {
	TokIdNode *pTok = nodes[i].Node(m_pRoot);
	res.push_back(pTok->v_prev->Name());
	// Should we search for Categoric or Numeric?
    }

    return res;
}

WModelInfo *BayesNet::ModelInfo() const
{
    // we must exclude this variant ea
    // if(m_State != ...) ThrowInternalError
    if(m_pModelInfo == 0 && m_Objects == 0)
    {
	m_pModelInfo = new WModelInfo;
    }

    return static_cast<WModelInfo*>(m_pModelInfo);
}

void BayesNet::AddArc(TokIdNode *from, TokIdNode *to)
{
    ModelInfo()->m_aNode[NodeIndex(to)].m_aParent.push_back(from);
}

static int cmpTokIdNode(TokIdNode *node1, TokIdNode *node2)
{
    return (char*)node1->data - (char*)node2->data;
}

// It is inner DistribFun
WDistribFun *BayesNet::CreateDistribFun(TokIdNode *node, WDistribFun *pFun)
{
    Vector<TokIdNode*> &aParent = ModelInfo()->m_aNode[NodeIndex(node)].m_aParent;

    std::sort(aParent.begin(), aParent.end(), cmpTokIdNode);

    if(!pFun)
    {
	pFun = new WTabularDistribFun();
    }

    pFun->Setup(node, aParent);

    return (ModelInfo()->m_aNode[NodeIndex(node)].m_pDistribFun = pFun);
}

void BayesNet::MakeUniformDistribution()
{// for each distributions call MakeUniform
    CheckState(eMakeUniformDistribution, 0, "MakeUniformDistribution");

    int nNode = nBayesNode();

    for(int i = 0; i < nNode; ++i)
    {
	WTabularDistribFun *pTabF = dynamic_cast<WTabularDistribFun*>(
	    ModelInfo()->m_aNode[i].m_pDistribFun);
	if( pTabF == 0)
	{
	    pTabF = dynamic_cast<WTabularDistribFun*>(
		CreateDistribFun(TokNodeByIndex(i)));
	}
	pTabF->MakeUniform();
    }
}

void BayesNet::CreateGraph()
{
    pnl::intVecVector nbrsList;
    pnl::neighborTypeVecVector nbrsTypesList;
    WModelInfo &info = *ModelInfo();
    int nNode = nBayesNode();
    int i, j, k, nParent;

    nbrsList.resize(nNode);
    nbrsTypesList.resize(nNode);

    if(nNode > 8)
    {
	// to speedup allocating memory
	for(i = nNode; --i >= 0;)
	{
	    nbrsList[i].reserve(nNode/2);
	    nbrsTypesList[i].resize(nNode/2);
	}
    }

    for(i = nNode; --i >= 0;)
    {
	nParent = info.m_aNode[i].m_aParent.size();

	for(j = nParent; --j >= 0;)
	{
	    k = NodeIndex(info.m_aNode[i].m_aParent[j]);
	    nbrsList[i].push_back(k);
	    nbrsTypesList[i].push_back(pnl::ntParent);
	    nbrsList[k].push_back(i);
	    nbrsTypesList[k].push_back(pnl::ntChild);
	}
    }

    m_Graph = pnl::CGraph::Create(nbrsList, nbrsTypesList);

    m_Objects |= eGraph;// mark graph as created
}

static int nodeAssociation(Vector<pnl::CNodeType> *paNodeType, bool isDiscrete, int size)
{
    pnl::CNodeType nt(isDiscrete ? 1:0, size);

    for(int i = paNodeType->size(); --i >= 0;)
    {
	if((*paNodeType)[i] == nt)
	{
	    return i;
	}
    }

    paNodeType->push_back(nt);

    return paNodeType->size() - 1;
}

void BayesNet::CreateModel()
{
    WModelInfo &info = *ModelInfo();
    int nNode = nBayesNode();
    int i;
    Vector<pnl::CNodeType> aNodeType;
    Vector<int> aNodeAssociation;

    if(!m_Graph)
    {
	CreateGraph();
    }

    // create BNet
    PNL_CHECK_IS_NULL_POINTER(m_Graph);
    aNodeAssociation.resize(nNode);
    aNodeType.reserve(nNode > 16 ? 8:4);
    for(i = 0; i < nNode; i++)
    {
	pnl::CNodeType &nt = info.m_aNode[i].m_NodeType;

	aNodeAssociation[i] = nodeAssociation(&aNodeType,
	    nt.IsDiscrete(), nt.GetNodeSize());
    }

    m_Model = pnl::CBNet::Create(nNode, aNodeType.size(),
	&aNodeType.front(), &aNodeAssociation.front(), m_Graph);

    // attach parameters
    for(i = 0; i < nNode; i++)
    {// tabular only
	PNL_CHECK_IS_NULL_POINTER(info.m_aNode[i].m_pDistribFun);
	WDistribFun *pWDF = info.m_aNode[i].m_pDistribFun;
#if 0
	pnl::CDistribFun *pDF = info.m_aNode[i].m_pDistribFun->DistribFun();
	PNL_CHECK_IS_NULL_POINTER(pDF);
	m_Model->GetFactor(i)->SetDistribFun(pDF);
#else
	PNL_CHECK_IS_NULL_POINTER(dynamic_cast<WTabularDistribFun*>(pWDF));
	pnl::CDenseMatrix<float> *mat = dynamic_cast<WTabularDistribFun*>(pWDF)->Matrix();
	PNL_CHECK_IS_NULL_POINTER(mat);
	m_Model->AllocFactor(i);
	m_Model->GetFactor(i)->AttachMatrix(mat, pnl::matTable);
	m_Model->GetFactor(i)->AttachMatrix(mat->Copy(mat), pnl::matDirichlet);
#endif
    }

    m_Objects |= eModel;// mark model as created
}

// manipulating arcs
void BayesNet::AddArc(TokArr from, TokArr to)
{
    CheckState(eAddArc, PROHIBIT(eAllObjects), "AddNode");

    int i;
    
    MustBeNode(from);
    MustBeNode(to);
    if(from.size() == 1)
    {
	for(i = 0; i < to.size(); ++i)
	{
	    AddArc(from[0].Node(m_aNode), to[i].Node(m_aNode));
	}
    }
    else if(to.size() == 1)
    {
	for(i = 0; i < from.size(); ++i)
	{
	    AddArc(from[i].Node(m_aNode), to[0].Node(m_aNode));
	}
    }
    else if(to.size() == from.size())
    {
	for(i = 0; i < to.size(); ++i)
	{
	    AddArc(from[i].Node(m_aNode), to[i].Node(m_aNode));
	}
    }
    else
    {
	ThrowUsingError("inconsistent number of nodes between 'to' and 'from'", "AddArc");
    }
}

void BayesNet::SetP(TokArr value, TokArr prob, TokArr parentValue)
{
    CheckState(eSetP, 0, "SetP");

    PNL_CHECK_FOR_NON_ZERO(value.size() - 1);

    int index = NodeIndex(ExtractNodes(value)[0]);
    TmpNodeInfo &nodeInfo = ModelInfo()->m_aNode[index];

    if(!nodeInfo.m_pDistribFun)
    {
	CreateDistribFun(m_aNodeInfo[index].m_SelfNode, new WTabularDistribFun);
    }
    if(parentValue.size())
    {
	ExtractNodes(parentValue);
    }

    nodeInfo.m_pDistribFun->FillData(pnl::matTable, value, prob, parentValue);
}

// This function checks for state of BayesNet object.
//
// All function from user interface (most of public functions) must call
// CheckState as first call.
//
// There are possible cases:
// 1 function can be called, all necessary objects are there
// 2 function can be called, but some work should be done before
//   (build graph or model by existing data)
// 3 function can not be called - mandatory objects is absented
void BayesNet::CheckState(int funcId, int characteristic, const char *funcName)
{
    const char *objName = "";

    if((GET_PROHIBITION(characteristic) & m_Objects) != 0)
    {// third case
	ThrowUsingError("function can not be called at current state", "CheckState");
    }

    if(m_Objects == GET_NEEDS(characteristic))
    {
	return;// first case
    }

    int objectToBeBuild = GET_NEEDS(characteristic) & (~m_Objects);
    // second case

    try
    {
	// build graph if need
	if((objectToBeBuild & eGraph) != 0)
	{
	    objName = "Graph";
	    CreateGraph();
	}
	
	// build model if need
	if((objectToBeBuild & eModel) != 0)
	{
	    objName = "Graphical Model";
	    CreateModel();
	}

	// create inference engine object if need
	if((objectToBeBuild & eInference) != 0)
	{
	    objName = "Inference";
	    m_Inference = pnl::CPearlInfEngine::Create(m_Model);
	    m_Objects |= eInference;
	}
    }

    catch(pnl::CException &ex)
    {
	pnl::pnlString s;

	s << "Objects that required for function execution (" << objName << ") can not be build\n";
	s << "PNL Diagnostic: " << ex.GetMessage();
	ThrowUsingError(s.c_str(), "CheckState");
    }

    catch(...)
    {
	pnl::pnlString s;

	s << "Objects that required for function execution (" << objName << ") can not be build";
	ThrowUsingError("Objects that required for function execution can not be build", "CheckState");
    }
}

#if 0
void BayesNet::CheckStateAlt(int funcId, int characteristic, const char *funcName)
{
    switch(m_State)
    {
    case 0:
	if((objects & (eGraph | eModel)) == objects)
	{
	    CreateGraph();
	    CreateModel();
	    m_Objects = eModel;
	    // change state
	}
	else
	{
	    ThrowUsingError("CheckState(): model is empty", funcName);
	    // Not reached
	}
	
	// FALLTHRU
	
    case 1:
    case eInference:
    case eLearning:
    default:
	ThrowInternalError("CheckState(): unknown or unhandled state", funcName);
    }
}
#endif

pnl::CEvidence *BayesNet::CreateEvidence(TokArr &aValue)
{
    PNL_CHECK_FOR_ZERO(aValue.size());// empty aValue is prohibited
    Vector<char> aNodeFlag;
    pnl::intVector aiNode;
    pnl::valueVector vValue;
    int j, i, nValue;
    int nValueIn = aValue.size();
    static const char fname[] = "CreateEvidence";

    aNodeFlag.assign(m_Model->GetNumberOfNodes(), '\0');
    aiNode.resize(nValueIn);

    // check for every pair variable-value
    for(i = nValueIn, nValue = 0; --i >= 0;)
    {
	TokIdNode *node = aValue[i].Node(m_aNode);
        PNL_CHECK_IS_NULL_POINTER(node);
	TokIdNode *bayesNode = (node->tag == eTagNetNode) ? node:node->v_prev;
	if(!bayesNode || bayesNode->tag != eTagNetNode)
	{
	    ThrowUsingError("Wrong name for node or value of Bayes Net", fname);
	}
	int idx = NodeIndex(bayesNode);

	aiNode[i] = idx;

	if(aNodeFlag[idx])
	{
	    pnl::pnlString str;
	    str << "Variable " << bayesNode->Name() << " is used twice or more";
	    ThrowUsingError(str.c_str(), fname);
	}

	aNodeFlag[idx] = 1;
	pnl::CNodeType nt = pnlNodeType(idx);

	if(nt.IsDiscrete() && node == bayesNode)
	{
	    String s = aValue[i];
	    if(aValue[i].fload.size() != 1)
	    {
		ThrowUsingError("Number of values for discrete value must be 1", fname);
	    }
	    j = aValue[i].IntValue();
	    if(j < 0 || j >= nt.GetNodeSize())
	    {
		pnl::pnlString str;
		str << "Bad value " << j
		    << ". Value for discrete variable " << bayesNode->Name()
		    << " must be more than 0 and less than " << nt.GetNodeSize();
		ThrowUsingError(str.c_str(), fname);
	    }
	}
	nValue += (nt.IsDiscrete() ? 1:nt.GetNodeSize());
    }

    vValue.reserve(nValue);
    for(i = 0, nValue = 0; i < aValue.size(); ++i)
    {
	pnl::CNodeType nt = *m_Model->GetNodeType(aiNode[i]);
	TokIdNode *node = aValue[i].Node(m_aNode);
	TokIdNode *bayesNode = (node->tag == eTagNetNode) ? node:node->v_prev;
	// we assume that .fl for aValue[i] if defined
	if(node == bayesNode)
	{
	    vValue.push_back(nt.IsDiscrete()
		? pnl::Value(int(aValue[i].FltValue(0).fl))
		:pnl::Value(aValue[i].FltValue(0).fl));
	}
	else if(nt.IsDiscrete())
	{
	    //vValue.push_back(pnl::Value(aValue[i].IntValue()));
	    vValue.push_back(pnl::Value(GetInt(aValue[i].Node())));
	}
	else
	{
	    ThrowInternalError("Not implemented", fname);
	}
    }

    return pnl::CEvidence::Create(m_Model, aiNode, vValue);
}

void BayesNet::Evid(TokArr values, bool bPush)
{
    CheckState(eEvidence, NEED(eModel), "Evid");

    if(values.size())
    {
	m_EvidenceBoard.Set(values);
    }

    if(bPush && !m_EvidenceBoard.IsEmpty())
    {
	// check for empty Board?
	m_aEvidenceBuf.push_back(CreateEvidence(m_EvidenceBoard.GetBoard()));
	m_EvidenceBoard.Clear();
    }
}

void BayesNet::ClearEvid()
{
    CheckState(eClearEvid, NEED(eModel), "ClearEvid");

    m_EvidenceBoard.Clear();
}

void BayesNet::ClearEvidHistory()
{
    CheckState(eClearEvidHistory, NEED(eModel), "ClearEvidHistory");

    // ? may be this wrong - we mustn't delete evidences
    for(int i = m_aEvidenceBuf.size(); --i >= 0; )
    {
	delete m_aEvidenceBuf[i];
    }

    m_aEvidenceBuf.resize(0);
    m_nLearnedEvidence = 0;
}

void BayesNet::Learn()
{
    Learn(0, 0);
}

void BayesNet::Learn(TokArr aSample[], int nSample)
{
    CheckState(eLearn, NEED(eModel), "Learn");
    bool bFirst = false;

    if(m_nLearnedEvidence > m_aEvidenceBuf.size())
    {
	ThrowInternalError("inconsistent learning process", "Learn");
    }

    if(m_nLearnedEvidence == m_aEvidenceBuf.size())
    {
	return;// is it error?
    }

    if(!m_Learning)
    {
	m_Learning = pnl::CBayesLearningEngine::Create(m_Model);
	bFirst = true;
    }

    if(nSample)
    {
	for(int i = 0; i < nSample; ++i)
	{
	    m_aEvidenceBuf.push_back(CreateEvidence(aSample[i]));
	}
    }

    m_Learning->AppendData(m_aEvidenceBuf.size() - m_nLearnedEvidence,
	&m_aEvidenceBuf[m_nLearnedEvidence]);
    m_nLearnedEvidence = m_aEvidenceBuf.size();
    m_Learning->Learn();

    /*for(int i = 0; i < nBayesNode(); ++i)
    {
	CNumericDenseMatrix<float> *mat = static_cast<CNumericDenseMatrix<float>*>(
	    m_Model->GetFactor(i)->GetMatrix(pnl::matTable));

	int len;
	const float *v;

	mat->GetRawData(&len, &v);
	for(int j = 0; j < len; ++j)
	{
	    printf("%.3f ", v[j]);
	}
	printf("\n");
    }*/
}

#if 0        
BayesNet* BayesNet::LearnStructure(TokArr aSample[], int nSample)
{
    CheckState(eLearn, NEED(eModel), "LearnStructure");
    //bool bFirst = false; //we will relearn model with the same data anyway because we will allow 
    // random seed and this may lead to different result
    
    intVector vAnc, vDesc;//bogus vectors
    CMlStaticStructLearnHC* pLearning = pnl::CMlStaticStructLearnHC::Create(m_Model,
        itStructLearnML, StructLearnHC, BIC, m_Model->GetNumberOfNodes(), vAnc, vDesc, 1/*one restart*/ );
	
    if(nSample)
    {
	    for(int i = 0; i < nSample; ++i)
	    {
	        m_aEvidenceBuf.push_back(CreateEvidence(aSample[i]));
	    }
    }

   // m_Learning->AppendData(m_aEvidenceBuf.size() - m_nLearnedEvidence,
	//&m_aEvidenceBuf[m_nLearnedEvidence]);
   // m_nLearnedEvidence = m_aEvidenceBuf.size();
    pLearning->SetData(m_aEvidenceBuf.size(), &m_aEvidenceBuf.front() );
    pLearning->Learn();
    const int* pRenaming = pLearning->GetResultRenaming();
    pLearning->CreateResultBNet(const_cast<CDAG*>(pLearning->GetResultDAG()));
    
    CBNet* newNet = CBNet::Copy(pLearning->GetResultBNet());

    //now we will create new object of class BayesNet and construct it from scratch using newNet
    //and required information (node names) on current (this) network
    BayesNet* newBayesNet = new BayesNet;

    //add nodes 
    int nnodes = newNet->GetNumberOfNodes();
    int* revren = new int[nnodes];
    for( int i = 0 ; i < nnodes; i++ )
    {
        revren[pRenaming[i]] = i;
    }   
    TokArr NodeOrdering;
    for( i = 0 ; i < nnodes; i++ )
    { 
        int idx = revren[i];
        
        //get info about node being added
        
        //get node name
        Tok nodeName = NodeName(idx); 
        //cout << nodeName << endl;
        
        //get node type 
        Tok nodeType = NodeType( nodeName );
        //cout << nodeType << endl;
        
        //get node values 
        Resolve(nodeName);
        TokArr values = nodeName.GetDescendants(eTagValue);
        //convert to single word representation
        for( int i = 0; i < values.size(); i++ )
        {
            values[i] = values[i].Name();
        }
        //cout << values << endl;
        
        newBayesNet->AddNode( nodeType^nodeName, values );
        //concatenate
        NodeOrdering << nodeName;
    }

    cout << NodeOrdering;

    CGraph* graph = newNet->GetGraph();
    intVector children;
    for( i = 0 ; i < nnodes; i++ )
    { 
        graph->GetChildren( i, &children );
        
    }    
    

    //get edges of new graph and add them to new network
    CGraph* newGraph = newNet->GetGraph();
    //for( i = 0 ; i < newGraph->

    //newBayesNet->AddArc(
              
    return newBayesNet;
    

    /*for(int i = 0; i < nBayesNode(); ++i)
    {
	CNumericDenseMatrix<float> *mat = static_cast<CNumericDenseMatrix<float>*>(
	    m_Model->GetFactor(i)->GetMatrix(pnl::matTable));

	int len;
	const float *v;

	mat->GetRawData(&len, &v);
	for(int j = 0; j < len; ++j)
	{
	    printf("%.3f ", v[j]);
	}
	printf("\n");
    }*/
}
#else
void BayesNet::LearnStructure(TokArr aSample[], int nSample)
{
    CheckState(eLearn, NEED(eModel), "LearnStructure");
    //bool bFirst = false; //we will relearn model with the same data anyway because we will allow 
    // random seed and this may lead to different result
    
    intVector vAnc, vDesc;//bogus vectors
    CMlStaticStructLearnHC* pLearning = pnl::CMlStaticStructLearnHC::Create(m_Model,
        itStructLearnML, StructLearnHC, BIC, m_Model->GetNumberOfNodes(), vAnc, vDesc, 1/*one restart*/ );
	
    if(nSample)
    {
	    for(int i = 0; i < nSample; ++i)
	    {
	        m_aEvidenceBuf.push_back(CreateEvidence(aSample[i]));
	    }
    }

   // m_Learning->AppendData(m_aEvidenceBuf.size() - m_nLearnedEvidence,
	//&m_aEvidenceBuf[m_nLearnedEvidence]);
   // m_nLearnedEvidence = m_aEvidenceBuf.size();
    pLearning->SetData(m_aEvidenceBuf.size(), &m_aEvidenceBuf.front() );
    pLearning->Learn();
    const int* pRenaming = pLearning->GetResultRenaming();
    pLearning->CreateResultBNet(const_cast<CDAG*>(pLearning->GetResultDAG()));
    
    CBNet* newNet = CBNet::Copy(pLearning->GetResultBNet());

    //change ordering in current stuff
    // Note! it may happen that old ordering of nodes is consistent (i.e. is topological)
    // with new graph structure so theoretically no reordering required
    //this would be good to have such function in PNL that checks this situation and reorder
    //new network to old ordering. So we would not have to do all below

    //reorder list of names

    //add nodes 
    int nnodes = newNet->GetNumberOfNodes();
    int* revren = new int[nnodes];
    for( int i = 0 ; i < nnodes; i++ )
    {
        revren[pRenaming[i]] = i;
    } 

    //reassign model
    CBNet* oldModel = m_Model;
    m_Model = newNet;

    //clear learning engine
    delete m_Learning;
    m_Learning = 0;

    //clear inference engine
    delete m_Inference;
    m_Inference = 0;

    //should we release old graph before this? potential memory leak
    //why do we have m_Graph at all? Is it used only during model creation?
    m_Graph = m_Model->GetGraph();
      
    //change node->index map
    for( std::map<String,int>::iterator it = m_aiNode.begin(); it!= m_aiNode.end(); it++ )
    {   
        (*it).second = revren[(*it).second];
        //cout << (*it).first << " " <<(*it).second << endl;
    }
    
    //change domain in evidences
    
    //change evidence on board
    //IT is in Token form, so do not need to change

    //change evidences in evidence buffer
    for( Vector<pnl::CEvidence*>::iterator it1 = m_aEvidenceBuf.begin(); it1 != m_aEvidenceBuf.end(); it1++ )
    {
        CEvidence* oldEv = *it1;
        CNodeValues* nv = oldEv;


       /* { //below block of code borrowed from CMLStaticStructLearn class of PNL
            intVector obsnodes(nnodes);
            for(i=0; i<nnodes; i++) obsnodes[i] = i;
            valueVector new_data;
            const Value* val;
            for(i = 0 ; i < nEv; i++)
            {
	        for(j=0; j<nnodes; j++)
	        {
	            val = m_Vector_pEvidences[i]->GetValue(m_vResultRenaming[j]);
		        nt = m_pResultBNet->GetNodeType(j);
	            if(nt->IsDiscrete())
	            {
		            new_data.push_back(*val);
	            }
	            else
	            {
		            ns = nt->GetNodeSize();
		            for(k=0; k<ns; k++)
		                new_data.push_back(*(val+k));
	            }
	        }
        } //end block of borrowed code

	    pEv[i] = CEvidence::Create(m_pResultBNet, nnodes, &obsnodes.front(), new_data);
	     





        CEvidence* newEv = CEvidence::Create(

        old->


           */



    }






    /*for(int i = 0; i < nBayesNode(); ++i)
    {
	CNumericDenseMatrix<float> *mat = static_cast<CNumericDenseMatrix<float>*>(
	    m_Model->GetFactor(i)->GetMatrix(pnl::matTable));

	int len;
	const float *v;

	mat->GetRawData(&len, &v);
	for(int j = 0; j < len; ++j)
	{
	    printf("%.3f ", v[j]);
	}
	printf("\n");
    }*/
}


#endif
Vector<TokIdNode*> BayesNet::ExtractNodes(TokArr &aValue) const
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

void BayesNet::SplitNodesByObservityFlag(Vector<int> *aiObserved, Vector<int> *aiUnobserved)
{
    PNL_CHECK_IS_NULL_POINTER(aiObserved);

    Vector<TokIdNode*> aObs = ExtractNodes(m_EvidenceBoard.GetBoard());
    int i, j;
    int nNode = nBayesNode();

    for(i = 0; i < aObs.size(); ++i)
    {
	aiObserved->push_back(NodeIndex(aObs[i]));
    }
    std::sort(aiObserved->begin(), aiObserved->end());
    for(j = i = 0; i < nNode; ++i)
    {
	if((*aiObserved)[j] == i)
	{
	    ++j;
	}
	else
	{
	    aiUnobserved->push_back(i);
	}
    }
}


TokArr BayesNet::P(TokArr childe, TokArr parents)
{
    static const char fname[] = "P";
    CheckState(eP, NEED(eGraph|eModel), fname);
    

    int nchldComb = childe.size();
    if( !nchldComb )
    {
	ThrowUsingError("Must be at least one combination for a childe node", fname);
    }

    Vector<int> childeNd, childeVl;
    ExtractTokArr(childe, &childeNd, &childeVl);

    if( !childeVl.size())
    {
	childeVl.assign(nchldComb, -1);
    }
    
    
    Vector<int> parentNds, parentVls;
    int nparents = parents.size();
    if( nparents )
    {
	ExtractTokArr(parents, &parentNds, &parentVls);
	if( parentVls.size() == 0 || 
	    std::find(parentVls.begin(), parentVls.end(), -1 ) != parentVls.end() )
	{
	    ThrowInternalError("undefindes values for given parent nodes", "P");
	}
    }
    else
    {
	m_Graph->GetParents( childeNd.front(), &parentNds );
	nparents = parentNds.size();
	parentVls.assign(nparents, -1);
    }

    parentNds.resize(nparents + 1);
    parentVls.resize(nparents + 1);
    
    
    const CFactor * cpd = m_Model->GetFactor(childeNd.front());
    const CMatrix<float> *mat = cpd->GetMatrix(matTable);
    
    TokArr result = "";
    int i;
    for( i = 0; i < nchldComb; i++ )
    {
	parentNds[nparents] = childeNd.front();
	parentVls[nparents] = childeVl[i];
	result << CutReq( parentNds, parentVls, mat);
    }    
    
    return result;
}



TokArr BayesNet::JPD( TokArr nodes )
{
    
    
    static const char fname[] = "JPD";
    CheckState(eP, NEED(eModel|eInference), fname);

    if( !nodes.size())
    {
	ThrowInternalError("undefined query nodes", "JPD");
    }


    pnl::CEvidence *evid = NULL;
    if( m_EvidenceBoard.IsEmpty() )
    {
	evid = CEvidence::Create(m_Model->GetModelDomain(), 0, NULL, valueVector(0));
    }
    else
    {
	evid = CreateEvidence(m_EvidenceBoard.GetBoard());
    }
    
    m_Inference->EnterEvidence( evid );
    
    int nnodes = nodes.size();
    Vector<int> queryNds, queryVls;
    ExtractTokArr(nodes, &queryNds, &queryVls);
    if(!queryVls.size())
    {
	queryVls.assign(nnodes, -1);
    }
    
    m_Inference->MarginalNodes(&queryNds.front(), queryNds.size());

    const CPotential *pot = m_Inference->GetQueryJPD();
    const CMatrix<float> * mat = pot->GetMatrix(matTable);
    
    return CutReq( queryNds, queryVls, mat);
}


TokArr BayesNet::CutReq( Vector<int>& queryNds, Vector<int>& queryVls, 
			const CMatrix<float> * mat ) const
{
    
    int nnodes = queryNds.size();
    
    Vector<int> obsDims, obsVls;
    obsDims.reserve(nnodes);
    obsVls.reserve(nnodes);
    
    int i;
    for( i = 0; i < queryNds.size(); i++ )
    {
	int v = queryVls[i];
	if(  v != -1 )
	{
	    obsVls.push_back(v);
	    obsDims.push_back(i);
	}
	
    }
    const CMatrix<float> * resMat;
    if( obsVls.size())
    {
	resMat = mat->ReduceOp(&obsDims.front(), obsDims.size(),2, &obsVls.front());
    }
    else
    {
	resMat = mat;
    }
    
        
    CMatrixIterator<float>* iter = resMat->InitIterator();
    intVector index;
    TokArr result;
    Tok tmp;
    for( iter; resMat->IsValueHere( iter ); resMat->Next(iter) )
    {
	index.clear();
	float val = *(resMat->Value( iter ));
	resMat->Index( iter, &index );
	int j = 0;

	tmp = "";
	for( i = 0; i < queryNds.size(); i++ )
	{
	    String nodeName = NodeName(queryNds[i]);
	    String valName  = DiscreteValue(NodeIndex(nodeName.c_str()), 
		queryVls[i] != -1 ? queryVls[i] : index[i] );
	    
	    tmp ^= (Tok(nodeName) ^ valName);
	}
	result << (tmp^val);
	
    }

    delete iter;
    if(obsVls.size())
    {
	delete resMat;
    }
    return result;
    
    
}

TokArr BayesNet::MPE(TokArr nodes)
{
    CheckState(eMPE, NEED(eModel|eInference), "MPE");
    
    if( !nodes.size())
    {
	ThrowInternalError("undefined query nodes", "MPE");
    }
    MustBeNode(nodes);

    pnl::CEvidence *evid = NULL;
    if( m_EvidenceBoard.IsEmpty() )
    {
	evid = CEvidence::Create(m_Model->GetModelDomain(), 0, NULL, valueVector(0));
    }
    else
    {
	evid = CreateEvidence(m_EvidenceBoard.GetBoard());
    }
    
    m_Inference->EnterEvidence(evid, 1);
    
    int nnodes = nodes.size();
    int i;
    Vector<int> queryNds;
    if( nnodes )
    {
	queryNds.resize(nnodes);
	for(i = 0; i < nnodes; ++i)
	{
	    Tok tok = nodes[i];
	    queryNds[i] = NodeIndex( tok.Node() );
	}
    }
    else
    {
	/*
	Vector<int> aiObs;
	SplitNodesByObservityFlag(&aiObs, &queryNds);
	nnodes = queryNds.size();
	*/
    }

    m_Inference->MarginalNodes(&queryNds.front(), queryNds.size());

    const pnl::CEvidence *mpe = m_Inference->GetMPE();
    
    
    TokArr result;

    for(i = 0; i < nnodes; ++i)
    {
	pnl::Value v = *mpe->GetValue(queryNds[i]);

	if((v.IsDiscrete() != 0) != pnlNodeType(queryNds[i]).IsDiscrete())
	{
	    ThrowInternalError("Non-discrete value for discrete variable", "MPE");
	}

	TokIdNode *node = TokNodeByIndex(queryNds[i]);
	if(pnlNodeType(queryNds[i]).IsDiscrete())
	{
	    node = node->v_next;
	    //may be we should search by id?
	    for(; node; node = node->h_next)
	    {
		if(!node || node->tag != eTagValue)
		{
		    ThrowInternalError("wrong value for node", "MPE");
		}
		if(INDEX(node) == v.GetInt())
		{
		    break;
		}
	    }

	    PNL_CHECK_FOR_NON_ZERO(INDEX(node) - v.GetInt());
	    result.push_back(Tok(node));
	}
    }

    return result;
}

/*
TokArr BayesNet::MPE(TokArr nodes)
{
    CheckState(eMPE, NEED(eModel|eInference), "MPE");

    pnl::CEvidence *evid = CreateEvidence(m_EvidenceBoard.GetBoard());
    Vector<int> aiObs, aiUnobs;
    int i;

    SplitNodesByObservityFlag(&aiObs, &aiUnobs);
#if 0
    // We create evidence on every call - so we doesn't need for this code
    for(i = 0; i < aiObs.size(); ++i)
    {
	if(!evid->isNodeObserved(aiObs[i]))
	{
	    evid->ToggleNodeState(1, &aiObs[i]);
	}
    }
#endif

    m_Inference->EnterEvidence(evid, 1);
    delete evid;
    if(nodes.size())
    {
	ThrowInternalError("Not yet realized", "MPE");
    }
    m_Inference->MarginalNodes(&aiUnobs.front(), aiUnobs.size());

    const pnl::CEvidence *mpe = m_Inference->GetMPE();
    TokArr result;

    for(i = 0; i < aiUnobs.size(); ++i)
    {
	pnl::Value v = *mpe->GetValue(aiUnobs[i]);

	if((v.IsDiscrete() != 0) != pnlNodeType(aiUnobs[i]).IsDiscrete())
	{
	    ThrowInternalError("Non-discrete value for discrete variable", "MPE");
	}

	TokIdNode *node = TokNodeByIndex(aiUnobs[i]);
	if(pnlNodeType(aiUnobs[i]).IsDiscrete())
	{
	    node = node->v_next;
	    //may be we should search by id?
	    for(; node; node = node->h_next)
	    {
		if(!node || node->tag != eTagValue)
		{
		    ThrowInternalError("wrong value for node", "MPE");
		}
		if(INDEX(node) == v.GetInt())
		{
		    break;
		}
	    }

	    PNL_CHECK_FOR_NON_ZERO(INDEX(node) - v.GetInt());
	    result.push_back(Tok(node));
	}
    }

    return result;
}
*/

void WEvid::Clear()
{
    m_EvidenceBoard = TokArr();
    m_VarMap.clear();
}

void WEvid::Set(const TokArr &evidence)
{
    TokIdNode *node;

    for(int i = 0; i < evidence.size(); i++)
    {
	node = const_cast<Tok&>(evidence[i]).Node();
	if(node->tag == eTagValue)
	{
	    node = node->v_prev;
	}
	if(node->tag != eTagNetNode)
	{
	    ThrowUsingError("Wrong name for node of Bayes Net", "SetEvidence");
	}
	std::map<TokIdNode*, int>::iterator location = m_VarMap.find(node);
	if(location == m_VarMap.end())
	{
	    m_VarMap[node] = m_EvidenceBoard.size();
	    m_EvidenceBoard.push_back(evidence[i]);
	}
	else
	{
	    m_EvidenceBoard[location->second] = evidence[i];
	}
    }
}

int BayesNet::SaveLearnBuf(const char *filename, ESavingType mode)
{
    WLex lex(filename, false/* write */, (mode == eCSV) ? ',':'\t');
    int iEvid, iCol, i;
    Vector<int> nUsingCol(nBayesNode(), 0);
    const int *aEvidNode;
    int nEvidNode;
    
    // mark nodes for saving
    for(iEvid = 0; iEvid < m_aEvidenceBuf.size(); ++iEvid)
    {
	aEvidNode = m_aEvidenceBuf[iEvid]->GetAllObsNodes();
	nEvidNode = m_aEvidenceBuf[iEvid]->GetNumberObsNodes();
	for(iCol = 0; iCol < nEvidNode; ++iCol)
	{
	    nUsingCol[aEvidNode[iCol]]++;
	}
    }

    Vector<int> aiCSVCol;

    // write header and fill node indices vector
    for(iCol = 0; iCol < nUsingCol.size(); ++iCol)
    {
	if(nUsingCol[iCol])
	{
	    aiCSVCol.push_back(iCol);
	    lex.PutValue(NodeName(iCol));
	}
    }

    lex.Eol();

    pnl::pnlString str;
    pnl::valueVector v;
    
    // write evidences one by one
    for(iEvid = 0; iEvid < m_aEvidenceBuf.size(); ++iEvid)
    {
	aEvidNode = m_aEvidenceBuf[iEvid]->GetAllObsNodes();
	nEvidNode = m_aEvidenceBuf[iEvid]->GetNumberObsNodes();
	// sort indices of nodes in evidence
	for(iCol = 1; iCol < nEvidNode; ++iCol)
	{
	    if(aEvidNode[iCol - 1] > aEvidNode[iCol])
	    {
		nUsingCol.assign(aEvidNode, aEvidNode + nEvidNode);
		std::sort(nUsingCol.begin(), nUsingCol.end());
		aEvidNode = &nUsingCol.front();
		break;
	    }
	}

	// iCol here - index in aiCSVCol
	for(iCol = 0, i = 0; iCol < aiCSVCol.size(); ++iCol)
	{
	    if(aiCSVCol[iCol] == aEvidNode[i])
	    {
		m_aEvidenceBuf[iEvid]->GetValues(aEvidNode[i], &v);
		for(int j = 0; j < v.size(); ++j)
		{
		    str.resize(0);
		    if(v[j].IsDiscrete())
		    {
			if(v.size() != 1)
			{
			    ThrowInternalError("We don't support discrete node "
				"with multidimensions", "SaveLearnBuf");
			}
			str << DiscreteValue(aEvidNode[i], v[j].GetInt());
		    }
		    else
		    {
			str << v[j].GetFlt();
			ThrowInternalError("Not yes realized", "SaveLearnBuf");
		    }
		    
		    lex.PutValue(String(str.c_str()));
		}
		++i;
	    }
	    else
	    {
		lex.PutValue(String());
	    }
	}

	lex.Eol();
    }

    return m_aEvidenceBuf.size();
}

String BayesNet::DiscreteValue(int iNode, int value) const
{
    TokIdNode *node = TokNodeByIndex(iNode);
    TokIdNode::Map::iterator loc = node->desc.find(TokId(value));

    if(loc == node->desc.end())
    {
	ThrowInternalError("Can't find value", "DiscreteValue");
    }

    return loc->second->Name();
}

int BayesNet::LoadLearnBuf(const char *filename, ESavingType mode, TokArr colons)
{
    static const char funName[] = "LoadLearnBuf";

    WLex lex(filename, true/* read */, (mode == eCSV) ? ',':'\t');
    TokArr header;
    int iCol, nCol, nColInUse;
    int nEvid;
    String colName;
    
    for(nColInUse = nCol = 0; nCol == 0 || !lex.IsEol(); ++nCol)
    {
	if(!lex.GetValue(&colName))
	{
	    ThrowUsingError("Loaded file has wrong structure", funName);
	}

	if(colons.size())
	{
	    continue;
	}
	Tok tok(colName);

	// now it must be node - we handling Discrete value only
	if(IsNode(tok))
	{
	    header.push_back(tok);
	    nColInUse++;
	}
	else
	{
	    header.push_back("");
	}
    }

    if(colons.size())
    {
	for(iCol = 0; iCol < colons.size(); iCol++)
	{
	    if(colons[iCol] != "")
	    {
		MustBeNode(TokArr(colons[iCol]));
		nColInUse++;
	    }
	}
	header = colons;
    }

    if(nColInUse == 0)
    {
	ThrowUsingError("Nothing to load", funName);
    }

    TokArr evid;
    for(nEvid = 0; lex.IsEof() == false;)
    {
	evid.resize(0);
	for(iCol = 0; (iCol == 0 || lex.IsEol() != true) && !lex.IsEof(); ++iCol)
	{
	    // colName - used as buffer for value
	    if(lex.GetValue(&colName) && header[iCol] != "")
	    {
		evid.push_back(header[iCol] ^ colName);
		//String a = evid;
	    }
	}

	if(lex.IsEof())
	{
	    break;
	}

	if( iCol > nCol )
	{
	    ThrowUsingError("Loaded file has wrong structure", funName);
	}

	if(evid.size() > 0)
	{
	    Evid(evid, true);
	    nEvid++;
	}
    }

    return nEvid;
}

// whatNodes is array of tokens which specify the list of variables and optionally 
// the required portion of observed nodes
// For example, if whatNodes = TokArr("Node1")
//    
void BayesNet::GenerateEvidences( int nSamples, bool ignoreCurrEvid, TokArr whatNodes )
{
    static const char funName[] = "GenerateEvidences";

    if(nSamples <= 0 )
	{
	    ThrowUsingError("Number of evidences to generate should be > 0", funName);
	}

    //check that Bnet is ready to generate samples 
    //(i.e. parameters and structure are fully specified)
    //in case of check fails the exception will be thrown
    CheckState(eGenerateEvidences, NEED(eModel), "GenerateEvidences");

    //get current evidence if exists and required
    pnl::CEvidence *evid = NULL;      
    if( !ignoreCurrEvid && !m_EvidenceBoard.IsEmpty() )
    {
        evid = CreateEvidence(m_EvidenceBoard.GetBoard());
    }
    
    //define empty buffer of PNL evidences
    pEvidencesVector newSamples;

    //generate some evidences
    m_Model->GenerateSamples( &newSamples, nSamples, evid); 

    //check if whatNodes is not empty
    //and if it contain something - filter out absent nodes from the evidence array
    int nValueIn = whatNodes.size();
    if( nValueIn )
    {
        Vector<int> aNodeFlag;
        aNodeFlag.resize(m_Model->GetNumberOfNodes(), 0);

        Vector<float> aNodePercentage;
        aNodePercentage.resize(m_Model->GetNumberOfNodes(), 0);
                       
        // check for every variable
        int i;
        for(i = nValueIn; --i >= 0;)
        {
	    TokIdNode *node = whatNodes[i].Node(m_aNode);
            PNL_CHECK_IS_NULL_POINTER(node);
    	    if(!node || node->tag != eTagNetNode)
	        {
	            ThrowUsingError("Wrong name for node of Bayes Net", funName);
	        }
	        int idx = NodeIndex(node);

            //check if node has probability/frequence 
            if( !whatNodes[i].fload.size() )
            {
                aNodePercentage[idx] = 1.f;
                aNodeFlag[idx] = 1;
            }
            else
            {
                aNodePercentage[idx] = whatNodes[i].FltValue().fl;
                aNodeFlag[idx] = 0;
            }            
        } 
        Vector<int> aNeedToHide;
        for( i = 0; i < aNodeFlag.size(); i++ )
        {
            if( !aNodeFlag[i] )
                aNeedToHide.push_back(i);
        }                                        

        //loop through generated evidences and clear info about missing variables
        //and delete missing portions of partially observed variables
        for( pEvidencesVector::iterator it = newSamples.begin(); it !=newSamples.end(); it++ )
        {
            CEvidence* ev = *it;
            int j = 0;
            for( j = 0; j < aNeedToHide.size(); j++ )
            {
                int idx = aNeedToHide[j];
                float percentage = aNodePercentage[idx];
                if( percentage == 0.f )
                    ev->MakeNodeHidden(idx);
                else
                {
                    //random hiding
                    if(pnlRand(0.0f,1.0f) > percentage)
                        ev->MakeNodeHidden(idx);
                }
            }
            //recreate evidence
            intVector numbers;
            pConstValueVector values;
            ev->GetObsNodesWithValues( &numbers, &values );

            //convert pConstValueVector to valueVector
            valueVector vv;
            for( pConstValueVector::iterator vit = values.begin(); vit != values.end(); vit++ )
            {
                vv.push_back((*vit)[0]); //this is true only for single-value nodes                                                                      
            }

            CEvidence* newev = pnl::CEvidence::Create( m_Model->GetModelDomain(), numbers, vv );
            delete ev;
            *it = newev; 
        }
    }                                                                  

    //add generates evidences to wrapper learning buffer
    m_aEvidenceBuf.insert(m_aEvidenceBuf.end(), newSamples.begin(), newSamples.end() );    
}

void BayesNet::MaskEvidences(TokArr whatNodes)
{   
    static const char fname[] = "MaskEvidences";

//    CheckState(eGenerateEvidences, NEED(eModel), "GenerateEvidences");

    //and if it contain something - filter out absent nodes from the evidence array
    int nValueIn = whatNodes.size();
    if( nValueIn )
    {
        Vector<int> aNodeFlag;
        aNodeFlag.resize(m_Model->GetNumberOfNodes(), 0);

        Vector<float> aNodePercentage;
        aNodePercentage.resize(m_Model->GetNumberOfNodes(), 0);
                       
        // check for every variable
        int i;
        for(i = nValueIn; --i >= 0;)
        {
	        TokIdNode *node = whatNodes[i].Node();
            PNL_CHECK_IS_NULL_POINTER(node);
    	    if(!node || node->tag != eTagNetNode)
	        {
	            ThrowUsingError("Wrong name for node of Bayes Net", fname);
	        }
	        int idx = NodeIndex(node);

            //check if node has probability/frequence 
            if( !whatNodes[i].fload.size() )
            {
                aNodePercentage[idx] = 1.f;
            }
            else
            {
                aNodePercentage[idx] = whatNodes[i].FltValue().fl;
            }                         
            //mark node for processing
            aNodeFlag[idx] = 1;

        } 
        Vector<int> aNeedToProcess;
        for( i = 0; i < aNodeFlag.size(); i++ )
        {
            if( aNodeFlag[i] )
                aNeedToProcess.push_back(i);
        }                                        

        //loop through generated evidences and clear info about missing variables
        //and delete missing portions of partially observed variables
        for( Vector<CEvidence*>::iterator it = m_aEvidenceBuf.begin(); it !=m_aEvidenceBuf.end(); it++ )
        {
            CEvidence* ev = *it;
            int j = 0;
            for( j = 0; j < aNeedToProcess.size(); j++ )
            {
                int idx = aNeedToProcess[j];
                float percentage = aNodePercentage[idx];
                if( percentage == 0.f )
                    ev->MakeNodeHidden(idx);
                else if( percentage == 1.f )
                    ev->MakeNodeObserved(idx);
                else
                {
                    //random hiding
                    if(pnlRand(0.0f,1.0f) > percentage)
                        ev->MakeNodeHidden(idx);
                    else
                        ev->MakeNodeObserved(idx);
                }
            }            
        }
    }                                                                  
}


                                                                   
void BayesNet::ExtractTokArr(TokArr &aNode, Vector<int> *paiNode, Vector<int> *paiValue,
	IIMap *pMap) const
{
    static const char fname[] = "ExtractTokArr";
    Vector<TokIdNode*> apNode = ExtractNodes(aNode);
    int i, nNode;
    bool bValue = false;
    bool bMustBeValue = false && (paiValue == 0);
    // replace 'false' with meaningful condition

    nNode = apNode.size();
    paiNode->resize(nNode);
    if(paiValue)
    {
	paiValue->resize(0);
    }

    for(i = 0; i < nNode; ++i)
    {
	(*paiNode)[i] = NodeIndex(apNode[i]);
	bool bValueForCurr = (aNode[i].Node(m_pRoot) != apNode[i]);
	if(bMustBeValue)
	{
	    if(!bValueForCurr)
	    {
		ThrowUsingError("Must be value for every node", fname);
	    }
	}
	else if(bValueForCurr)
	{
	    bValue = paiValue != 0;
	}
    }

    if(pMap)
    {
	for(i = paiNode->size(); --i >= 0;)
	{
	    (*paiNode)[i] = (*pMap)[(*paiNode)[i]];
	    if((*paiNode)[i] == -1)
	    {
		pnl::pnlString str;

		str << "Unknown node " << apNode[i]->Name() << " in family";
		ThrowUsingError(str.c_str(), fname);
	    }
	    // check for unexistence?
	}
    }

    if(bValue)
    {
	for(i = 0; i < nNode; ++i)
	{
	    TokIdNode *pValue = aNode[i].Node(m_pRoot);
	    if(pValue != apNode[i])
	    {
		if(pValue->tag != eTagValue)
		{
		    ThrowInternalError("Inner error or wrong usage of bayes net",
			fname);
		}
		if(paiValue && paiValue->size() != nNode)
		{
		    paiValue->assign(nNode, -1);
		}
		(*paiValue)[i] = GetInt(pValue);
	    }
	}
    }

    if(pMap)
    {// sort parent by index
	bool bReverse = true;
	int tmp;

	for(; bReverse;)
	{// bubble sort - assume that we work with small numbers of nodes
	    bReverse = false;
	    for(i = 1; i < nNode; ++i)
	    {
		if((*paiNode)[i - 1] > (*paiNode)[i])
		{// exchange
		    tmp = (*paiNode)[i - 1];
		    (*paiNode)[i - 1] = (*paiNode)[i];
		    (*paiNode)[i] = tmp;
		    if(paiValue)
		    {
			tmp = (*paiValue)[i - 1];
			(*paiValue)[i - 1] = (*paiValue)[i];
			(*paiValue)[i] = tmp;
		    }
		    bReverse = true;
		}
	    }
	}
    }
}

int BayesNet::GetInt(TokIdNode *node)
{
    {
	TokId &tid = node->id.back();
	
	if(tid.is_int)
	{
	    return tid.int_id;
	}
    }
    
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

CMatrix<float> *BayesNet::Matrix(int iNode) const
{
    CMatrix<float> *mat;

    if(m_Objects & eModel)
    {
	mat = m_Model->GetFactor(iNode)->GetMatrix(pnl::matTable);
    }
    else
    {
	mat = static_cast<WTabularDistribFun*>(
	    ModelInfo()->m_aNode[iNode].m_pDistribFun)
	    ->Matrix();
    }

    return mat;
}

pnl::CInfEngine &BayesNet::Inference()
{
    return *m_Inference;
}


#if 0

TokArr BayesNet::P(TokArr value, TokArr parents)
{
    static const char fname[] = "P";
    CheckState(eP, NEED(eGraph|eModel), fname);
    

    if(value.size() < 1)
    {
	ThrowUsingError("Must be at least one node", fname);
    }
    
    int val = eNodeClassCategoric;
    int mask = val | eNodeClassContinuous | eNodeClassUnknown;
    if(((NodesClassification(value) & mask) != val)
	|| (parents.size() && (NodesClassification(parents) & mask) != val))
    {
	ThrowUsingError("either value and parents must be discrete nodes", fname);
    }
    
    Vector<int> aiNode, aiValue, aiParent, aiParentValue, map;
    int i;
    CMatrix<float> *mat;
    if( ! m_EvidenceBoard.IsEmpty() )
    {
	/*
	ExtractTokArr(value, &aiNode, &aiValue);
		pnl::CEvidence * ev = CreateEvidence(m_EvidenceBoard.GetBoard());
		SplitNodesByObservityFlag(&aiObs, &aiUnobs);
		
		m_Inference->EnterEvidence(evid);
		if(nodes.size())
		{
		    ThrowInternalError("Not yet realized", "MPE");
		}
		m_Inference->MarginalNodes(&aiUnobs.front(), aiUnobs.size());
		m_Inference->GetQueryJPD()->GetMatrix(matTable);
		*/
	
	
    }
    else
    {
		
	ExtractTokArr(value, &aiNode, &aiValue);
	if(aiValue.size() != aiNode.size())
	{
	    aiValue.assign(aiNode.size(), -1);
	}
	
	// get matrix
	mat = Matrix(aiNode[0]);
    }
    // check parents
    int nParent = mat->GetNumberDims() - 1;

    int nFeature = 0;
    int iOmit = -1;// Cycle variable index in Bayes Net
    bool bDifferentNode = false;// true if 'value' contains references to different Bayes node

    for(i = 0; i < aiValue.size(); ++i)
    {
	if(aiValue[i] < 0)
	{
	    iOmit = nParent;
	    if((++nFeature) > 1)
	    {
		ThrowUsingError("More then 1 values with omitted value", fname);
	    }
	}
    }
    if(aiNode.size() > 1)
    {
	iOmit = nParent;
	if((++nFeature) > 1)
	{
	    ThrowUsingError("Incorrect combination of omitted value and multinode",
		fname);
	}
	for(i = 1; i < aiNode.size(); ++i)
	{
	    if(aiNode[i] != aiNode[i - 1])
	    {
		bDifferentNode = true;
		break;
	    }
	}
	// different nodes may has different set of parents
	if(bDifferentNode)
	{
	    // get etalon aParent
	    pnl::intVector aParentEtalon, aParent;
	    
	    m_Graph->GetParents(aiNode[0], &aParentEtalon);
	    std::sort(aParentEtalon.begin(), aParentEtalon.end());
	    for(i = 1; i < aiNode.size(); ++i)
	    {
		if(aiNode[i] == aiNode[0] || aiNode[i] == aiNode[i - 1])
		{
		    continue;
		}
		m_Graph->GetParents(aiNode[i], &aParent);
		if(aParent.size() != aParentEtalon.size())
		{
		    ThrowUsingError("More then 1 values with different numbers of parents",
			fname);
		    break;
		}
	    }

	    ThrowUsingError("multiValues not yet realized", fname);// temporary
	}
    }
    
    Vector<int> *pMap = &map;
    
    if(pMap && parents.size())
    {// fill map
	map.assign(nBayesNode(), -1);
	pnl::intVector aParent;

	m_Graph->GetParents(aiNode[0], &aParent);
	for(i = 0; i < aParent.size(); ++i)
	{
	    map[aParent[i]] = i;
	}
    }

    // check parents
    ExtractTokArr(parents, &aiParent, &aiParentValue, pMap);
    for(i = aiParentValue.size(); --i >= 0;)
    {
	if(aiParentValue[i] < 0)
	{
	    iOmit = i;
	    aiParentValue.erase(aiParentValue.begin() + i);
	    aiParent.erase(aiParent.begin() + i);
	}
    }
    
    if(nParent - aiParent.size() + nFeature > 1)
    {
	ThrowUsingError("Too many omitted variables", fname);
    }

    // find omitted value between parents
    if(nParent != aiParent.size() && iOmit < 0)
    {
	for(iOmit = 0; iOmit < aiParent.size() && iOmit == aiParent[iOmit]; iOmit++);
    }

    Vector<float> aOmitValue;// Values for cycle variable will be stored here
    // fill aOmitValue
    if(iOmit == nParent)
    {// special fill from value
	if(aiValue.size() > 1)
	{
	    aOmitValue.resize(aiValue.size());
	    for(i = aiValue.size(); --i >= 0;)
	    {
		aOmitValue[i] = aiValue[i];
	    }
	}
    }
    else if(aiParentValue.size() > nParent && nParent == 1)
    {// special fill from parents 
	aOmitValue.resize(aiParentValue.size());
	for(i = aiParentValue.size(); --i >= 0;)
	{
	    aOmitValue[i] = aiParentValue[i];
	}
    }
    if(iOmit < 0)
    {
	if(nFeature)
	{
	    ThrowInternalError("iOmit must be not null", fname);
	}
	iOmit = nParent;
	aOmitValue.assign(1, aiValue[0]);
    }
    if(aOmitValue.size() == 0)
    {
	int nDim;
	const int *ranges;

	mat->GetRanges(&nDim, &ranges);
	for(i = 0; i < ranges[iOmit]; ++i)
	{
	    aOmitValue.push_back(i);
	}
    }
    if(iOmit < nParent)
    {
	aiParent.insert(aiParent.begin() + iOmit, -1);
	aiParentValue.insert(aiParentValue.begin() + iOmit, aOmitValue[0]);
    }

    aiParentValue.push_back(aiValue[0]);

    String omitName;

    if(iOmit == nParent)
    {
	omitName = NodeName(aiNode[0]);
    }
    else
    {
	pnl::intVector aParent;

	m_Graph->GetParents(aiNode[0], &aParent);
	omitName = NodeName(aParent[iOmit]);
    }

    TokArr result;

    for(i = 0; i < aOmitValue.size(); ++i)
    {
	aiParentValue[iOmit] = aOmitValue[i];
	Accumulate(&result, aiParentValue, mat, omitName, aOmitValue[i]);
    }

    return result;
}

#endif

void BayesNet::Accumulate(TokArr *pResult, Vector<int> &aIndex,
			  pnl::CMatrix<float> *mat, String &prtName, int prtValue) const
{
    float val = mat->GetElementByIndexes(&aIndex.front());
    String valName = DiscreteValue(NodeIndex(prtName.c_str()), prtValue);

    (*pResult) << (Tok(prtName) ^ valName ^ val);
}

void BayesNet::SaveNet(const char *filename)
{
    pnl::CContextPersistence saver;

    saver.Put(m_Model, "Model");
    saver.Put(m_Graph, "Graph");
    saver.Put(this, "BayesNet");
    if(!saver.SaveAsXML(filename))
    {
	ThrowInternalError("Can't save file", "SaveNet");
    }
}

void BayesNet::LoadNet(const char *filename)
{
    pnl::CContextPersistence loader;

    if(!loader.LoadXML(filename))
    {
	ThrowUsingError("Can't load file - bad file?", "LoadNet");
    }

    BayesNet *bnet = static_cast<BayesNet*>(loader.Get("BayesNet"));

    if(!bnet)
    {
	ThrowUsingError("File doesn't contain BayesNet - bad file?", "LoadNet");
    }

    RebindFrom(bnet);
    //delete bnet;
}

void BayesNet::RebindFrom(BayesNet *bnet)
{
    // very dirty - when we carry out functionality to inner objects,
    // this function will be obsolete
    m_pRoot = bnet->m_pRoot;
    m_aNode = bnet->m_aNode;
    m_pCategoric = bnet->m_pCategoric;
    m_pContinuous = bnet->m_pContinuous;
    m_Objects = bnet->m_Objects;
    m_State = bnet->m_State;
    m_Graph = bnet->m_Graph;
    m_Model = bnet->m_Model;
    m_Inference = 0;
    m_Learning = 0;
    m_aEvidenceBuf = bnet->m_aEvidenceBuf;
    m_nLearnedEvidence = bnet->m_nLearnedEvidence;
    m_pModelInfo = bnet->m_pModelInfo;
    m_aNodeInfo = bnet->m_aNodeInfo;
    m_aiNode = bnet->m_aiNode;
}
