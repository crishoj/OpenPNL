#include "pnlTok.hpp"
#include "NetCallBack.hpp"
#include "pnlWGraph.hpp"
#include "pnlWDistributions.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "TokenCover.hpp"
#include "WDistribFun.hpp"
#pragma warning(push, 2)
#pragma warning(disable: 4251)
// class X needs to have dll-interface to be used by clients of class Y
#include "pnl_dll.hpp"
#include "pnlString.hpp"
#pragma warning(default: 4251)
#pragma warning(pop)
#include "Wcsv.hpp"
#include "WInner.hpp"
#include "pnlPersistCover.hpp"

#if defined(_MSC_VER)
#pragma warning(disable : 4239) // nonstandard extension used: 'T' to 'T&'
#endif

static int nodeAssociation(Vector<pnl::CNodeType> *paNodeType, bool isDiscrete, int size);

// Tok identifies for categoric and continuous node types
TokArr categoric("nodes^categoric");
TokArr continuous("nodes^continuous");

ProbabilisticNet::ProbabilisticNet(): m_Model(0), m_pCallback(0)
{
    m_pGraph = new WGraph();
    m_pTokenCov = new TokenCover("bnet", m_pGraph, true);
    m_paDistribution = new WDistributions(m_pTokenCov);
    // create node bnet and all descedants
}

ProbabilisticNet::~ProbabilisticNet()
{
    delete m_paDistribution;
    delete m_pTokenCov;
    delete m_pGraph;
    delete m_pCallback;
}

// for Bayes Net nodes only
void ProbabilisticNet::AddNode(TokArr nodes, TokArr subnodes)
{
    for(unsigned int i = 0; i < nodes.size(); ++i)
    {
	Token()->AddNode(nodes[i], subnodes);
    }
}

void ProbabilisticNet::DelNode(TokArr nodes)
{
    MustBeNode(nodes);
    for(unsigned int i = 0; i < nodes.size(); ++i)
    {
	Token()->DelNode(nodes[i]);
    }
}

// returns one of "categoric" or "continuous"
TokArr ProbabilisticNet::NodeType(TokArr nodes)
{
    TokArr res;
    MustBeNode(nodes);// check for existence of such vertices
    for(int i = 0; i < nodes.size(); ++i)
    {
	TokIdNode *pTok = nodes[i].Node(Token()->Root());
	res.push_back(pTok->v_prev->Name());
	// Should we search for Categoric or Numeric?
    }

    return res;
}

// manipulating arcs
void ProbabilisticNet::AddArc(TokArr from, TokArr to)
{
    int i, j;
    String nameTo, nameFrom;
    
    MustBeNode(from);
    MustBeNode(to);
    for(i = 0; i < to.size(); ++i)
    {
	nameTo = to[i].Name();
	for(j = 0; j < from.size(); ++j)
	{
	    nameFrom = from[j].Name();
	    Graph()->AddArc(nameFrom.c_str(), nameTo.c_str());
	}
    }
}

void ProbabilisticNet::DelArc(TokArr from, TokArr to)
{
    int i, j;
    int iTo;

    MustBeNode(from);
    MustBeNode(to);
    for(i = 0; i < to.size(); ++i)
    {
	iTo = Graph()->INode(to[i].Name());
	for(j = 0; j < from.size(); ++j)
	{
	    Graph()->DelArc(Graph()->INode(from[j].Name()), iTo);
	}
    }
}

void ProbabilisticNet::Evid(TokArr values, bool bPush)
{
    if(values.size())
    {
	m_EvidenceBoard.Set(values);
    }

    if(bPush && !m_EvidenceBoard.IsEmpty())
    {
	// check for empty Board?
	m_aEvidence.push_back(CreateEvidence(m_EvidenceBoard.GetBoard()));
	m_EvidenceBoard.Clear();
    }
}

void ProbabilisticNet::PushEvid(TokArr const values[], int nValue)
{
    for(int i = 0; i < nValue; ++i)
    {
	Evid(values[i], true);
    }
}

void ProbabilisticNet::ClearEvid()
{
    m_EvidenceBoard.Clear();
}

void ProbabilisticNet::ClearEvidHistory()
{
    // ? may be this wrong - we mustn't delete evidences
    for(int i = m_aEvidence.size(); --i >= 0; )
    {
	delete m_aEvidence[i];
    }

    m_aEvidence.resize(0);
}

int ProbabilisticNet::SaveLearnBuf(const char *filename, NetConst::ESavingType mode)
{
    WLex lex(filename, false/* write */, (mode == NetConst::eCSV) ? ',':'\t');
    int iEvid, iCol, i;
    Vector<int> nUsingCol(nNetNode(), 0);
    const int *aEvidNode;
    int nEvidNode;
    
    // mark nodes for saving
    for(iEvid = 0; iEvid < m_aEvidence.size(); ++iEvid)
    {
	aEvidNode = m_aEvidence[iEvid]->GetAllObsNodes();
	nEvidNode = m_aEvidence[iEvid]->GetNumberObsNodes();
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
    for(iEvid = 0; iEvid < m_aEvidence.size(); ++iEvid)
    {
	aEvidNode = m_aEvidence[iEvid]->GetAllObsNodes();
	nEvidNode = m_aEvidence[iEvid]->GetNumberObsNodes();
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
		m_aEvidence[iEvid]->GetValues(aEvidNode[i], &v);
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
//			ThrowInternalError("Not yes realized", "SaveLearnBuf");
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

    return m_aEvidence.size();
}

int ProbabilisticNet::LoadLearnBuf(const char *filename, NetConst::ESavingType mode, TokArr columns)
{
    static const char funName[] = "LoadLearnBuf";

    WLex lex(filename, true/* read */, (mode == NetConst::eCSV) ? ',':'\t');
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

	if(columns.size())
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

    if(columns.size())
    {
	for(iCol = 0; iCol < columns.size(); iCol++)
	{
	    if(columns[iCol] != "")
	    {
		MustBeNode(TokArr(columns[iCol]));
		nColInUse++;
	    }
	}
	header = columns;
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

bool ProbabilisticNet::SaveNet(pnl::CContextPersistence *saver)
{
    saver->Put(Model(), "Model");
    saver->Put(new pnl::CCover<ProbabilisticNet>(this), "NodeInfo", true);

    return true;
}

ProbabilisticNet* ProbabilisticNet::LoadNet(pnl::CContextPersistence *loader)
{
    pnl::CCover<ProbabilisticNet>* pCovNet = static_cast<pnl::CCover<ProbabilisticNet>*>(
	loader->Get("NodeInfo"));
    ProbabilisticNet *net;
    pnl::CGraphicalModel *model = static_cast<pnl::CGraphicalModel*>(
	loader->Get("Model"));

    if(!model)
    {
	ThrowUsingError("File doesn't contain BayesNet - bad file?", "LoadNet");
    }
    else
    {
	loader->AutoDelete(model);
    }
    if(!pCovNet)
    {
	int iNode, iValue;
	String nodeName;
	String aValue;

	net = new ProbabilisticNet;
	for(iNode = 0; iNode < model->GetNumberOfNodes(); ++iNode)
	{
	    nodeName = "Node";
	    nodeName << iNode;
	    aValue.resize(0);

	    const pnl::CNodeType &nt = *model->GetNodeType(iNode);
	    for(iValue = 0; iValue < nt.GetNodeSize(); ++iValue)
	    {
		if(iValue)
		{
		    aValue << ' ';
		}
		aValue << "State" << iValue;
	    }

	    net->AddNode((nt.IsDiscrete() ? categoric:continuous) ^ nodeName, aValue);
	}
    }
    else
    {
	net = pCovNet->GetPointer();
    }

    net->Reset(*model);

    return net;
}

// whatNodes is array of tokens which specify the list of variables and optionally 
// the required portion of observed nodes
// For example, if whatNodes = TokArr("Node1")
//    
void ProbabilisticNet::GenerateEvidences( int nSample, bool ignoreCurrEvid, TokArr whatNodes )
{
    static const char funName[] = "GenerateEvidences";

    if(nSample <= 0 )
    {
	ThrowUsingError("Number of evidences to generate should be > 0", funName);
    }

    //get current evidence if exists and required
    pnl::CEvidence *evid = 0;      
    if( !ignoreCurrEvid && !m_EvidenceBoard.IsEmpty() )
    {
        evid = CreateEvidence(m_EvidenceBoard.GetBoard());
    }
    
    //define empty buffer of PNL evidences
    pnl::pEvidencesVector newSamples;

    //generate some evidences
    m_pCallback->GenerateSamples(*this, &newSamples, nSample, evid);

    //check if whatNodes is not empty
    //and if it contain something - filter out absent nodes from the evidence array
    int nValueIn = whatNodes.size();
    if( nValueIn )
    {
        Vector<int> aNodeFlag;
        aNodeFlag.resize(nNetNode(), 0);

        Vector<float> aNodePercentage;
        aNodePercentage.resize(nNetNode(), 0);
                       
        // check for every variable
        int i;
        for(i = nValueIn; --i >= 0;)
        {
	    TokIdNode *node = Token()->Node(whatNodes[i]);
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
	for( pnl::pEvidencesVector::iterator it = newSamples.begin(); it !=newSamples.end(); it++ )
        {
	    pnl::CEvidence* ev = *it;
            int j = 0;
            for( j = 0; j < aNeedToHide.size(); j++ )
            {
                int idx = aNeedToHide[j];
                float percentage = aNodePercentage[idx];
                if( percentage == 0.f )
		{
                    ev->MakeNodeHidden(idx);
		}
                else
                {
                    //random hiding
                    if(pnl::pnlRand(0.0f,1.0f) > percentage)
                        ev->MakeNodeHidden(idx);
                }
            }
            //recreate evidence
            pnl::intVector numbers;
            pnl::pConstValueVector values;
            ev->GetObsNodesWithValues( &numbers, &values );

            //convert pConstValueVector to valueVector
            pnl::valueVector vv;
            for( pnl::pConstValueVector::iterator vit = values.begin(); vit != values.end(); vit++ )
            {
                vv.push_back((*vit)[0]); //this is true only for single-value nodes                                                                      
            }

            pnl::CEvidence* newev = pnl::CEvidence::Create( Model()->GetModelDomain(), numbers, vv );
            delete ev;
            *it = newev; 
        }
    }                                                                  

    //add generates evidences to wrapper learning buffer
    m_aEvidence.insert(m_aEvidence.end(), newSamples.begin(), newSamples.end() );    
}

void ProbabilisticNet::MaskEvidences(TokArr whatNodes)
{   
    static const char fname[] = "MaskEvidences";

    //and if it contain something - filter out absent nodes from the evidence array
    int nValueIn = whatNodes.size();
    if( nValueIn )
    {
        Vector<int> aNodeFlag;
        aNodeFlag.resize(nNetNode(), 0);

        Vector<float> aNodePercentage;
        aNodePercentage.resize(nNetNode(), 0);
                       
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
	for( Vector<pnl::CEvidence*>::iterator it = m_aEvidence.begin(); it !=m_aEvidence.end(); it++ )
        {
	    pnl::CEvidence* ev = *it;
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
                    if(pnl::pnlRand(0.0f,1.0f) > percentage)
                        ev->MakeNodeHidden(idx);
                    else
                        ev->MakeNodeObserved(idx);
                }
            }
        }
    }
}

void ProbabilisticNet::SetProperty(const char *name, const char *value)
{
    m_aPropertyValue[String(name)] = String(value);
}

String ProbabilisticNet::Property(const char *name) const
{
    SSMap::const_iterator it = m_aPropertyValue.find(String(name));

    return (it == m_aPropertyValue.end()) ? String():it->second;
}

void ProbabilisticNet::Reset(const pnl::CGraphicalModel &model)
{
    if(nNetNode() != model.GetNumberOfNodes())
    {
	ThrowInternalError("Reset by model with different number of nodes isn't yet implemented", "Reset");
    }
    Graph()->Reset(*model.GetGraph());

    int i;

    for(i = 0; i < nNetNode(); ++i)
    {
	Distributions()->ResetDistribution(i, *model.GetFactor(i));
    }
}

//=== inner functions ===

pnl::CEvidence *ProbabilisticNet::CreateEvidence(TokArr &aValue)
{
    PNL_CHECK_FOR_ZERO(aValue.size());// empty aValue is prohibited
    Vector<char> aNodeFlag;
    pnl::intVector aiNode;
    pnl::valueVector vValue;
    int j, i, nValue;
    int nValueIn = aValue.size();
    static const char fname[] = "CreateEvidence";

    aNodeFlag.assign(nNetNode(), '\0');
    aiNode.resize(nValueIn);

    // check for every pair variable-value
    for(i = nValueIn, nValue = 0; --i >= 0;)
    {
	TokIdNode *node = Token()->Node(aValue[i]);
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
	pnl::CNodeType nt = pnlNodeType(aiNode[i]);
	TokIdNode *node = Token()->Node(aValue[i]);
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
	    vValue.push_back(pnl::Value(aValue[i].FltValue(0).fl));
//            ThrowInternalError("Not implemented", fname);
	}
    }

    return pnl::CEvidence::Create(Model(), aiNode, vValue);
}

void ProbabilisticNet::GetTokenByEvidence(TokArr *tEvidence, pnl::CEvidence *evidence)
{
    pnl::intVector aiNode;
    pnl::valueVecVector vValue;

    evidence->GetObsNodesWithValues(&aiNode, &vValue);
    tEvidence->resize(aiNode.size());
    for(int i = 0; i < aiNode.size(); ++i)
    {
	String nodeName = NodeName(aiNode[i]);
	if(!vValue[i][0].IsDiscrete())
	{
	    ThrowInternalError("Not yet realized", "GetTokenByEvidence");
	}
	String valName  = DiscreteValue(aiNode[i], vValue[i][0].GetInt());

	(*tEvidence)[i] = Tok(nodeName) ^ valName;
    }
}

int ProbabilisticNet::nNetNode() const
{
    return Graph()->nNode();
}

void ProbabilisticNet::MustBeNode(TokArr &nodes) const
{
    for(int i = nodes.size(); --i >= 0;)
    {
	Token()->Resolve(nodes[i]);
	if(!IsNode(nodes[i]))
	{
	    pnl::pnlString str;

	    str << '\'' << static_cast<String>(nodes[i]) << "' is not a node";
	    ThrowUsingError(str.c_str(), "MustBeNode");
	}
    }
}

bool ProbabilisticNet::IsNode(Tok &node) const
{
    return Token()->Node(node)->tag == eTagNetNode;
}

String ProbabilisticNet::NodeName(int iNode) const
{
    return Graph()->NodeName(iNode);
}

int ProbabilisticNet::NodeIndex(const char *name) const
{
    return Graph()->INode(name);
}

int ProbabilisticNet::NodeIndex(TokIdNode *node) const
{
    return Graph()->INode(node->Name());
}

TokIdNode *ProbabilisticNet::TokNodeByIndex(int i) const
{
    return Token()->Node(i);
}

String ProbabilisticNet::DiscreteValue(int iNode, int value) const
{
    TokIdNode *node = TokNodeByIndex(iNode);
    TokIdNode::Map::iterator loc = node->desc.find(TokId(value));

    if(loc == node->desc.end())
    {
	ThrowInternalError("Can't find value", "DiscreteValue");
    }

    return loc->second->Name();
}

void ProbabilisticNet::ExtractTokArr(TokArr &aNode, Vector<int> *paiNode, Vector<int> *paiValue,
	IIMap *pMap) const
{
    static const char fname[] = "ExtractTokArr";
    Vector<TokIdNode*> apNode = Token()->ExtractNodes(aNode);
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
	bool bValueForCurr = (aNode[i].Node(Token()->Root()) != apNode[i]);
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
	    TokIdNode *pValue = aNode[i].Node(Token()->Root());
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

int ProbabilisticNet::GetInt(TokIdNode *node)
{
    return TokenCover::Index(node);
}

pnl::CNodeType ProbabilisticNet::pnlNodeType(int i)
{
    int size;
    bool bDiscrete;

    m_paDistribution->GetNodeTypeInfo(&bDiscrete, &size, i);
    return pnl::CNodeType(bDiscrete, size);
}

void ProbabilisticNet::Accumulate(TokArr *pResult, Vector<int> &aIndex,
			  pnl::CMatrix<float> *mat, String &prtName, int prtValue) const
{
    float val = mat->GetElementByIndexes(&aIndex.front());
    String valName = DiscreteValue(NodeIndex(prtName.c_str()), prtValue);

    (*pResult) << (Tok(prtName) ^ valName ^ val);
}

Tok ProbabilisticNet::ConvertMatrixToToken(const pnl::CMatrix<float> *mat)
{
    pnl::CDenseMatrix<float> *dMean = mat->ConvertToDense();
    const pnl::floatVector *meanVec = dMean->GetVector();
    std::vector<float> matr(meanVec->begin(), meanVec->end());

//    Tok meanTok = Tok(matr);
    return Tok(matr);
}


void ProbabilisticNet::SplitNodesByObservityFlag(Vector<int> *aiObserved, Vector<int> *aiUnobserved)
{
    PNL_CHECK_IS_NULL_POINTER(aiObserved);

    Vector<TokIdNode*> aObs = Token()->ExtractNodes(m_EvidenceBoard.GetBoard());
    int i, j;
    int nNode = nNetNode();

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

TokArr ProbabilisticNet::CutReq( Vector<int>& queryNds, Vector<int>& queryVls, 
			const pnl::CMatrix<float> * mat ) const
{
    int nnodes = queryNds.size();
    Vector<int> obsDims, obsVls;
    int i;

    obsDims.reserve(nnodes);
    obsVls.reserve(nnodes);
    
    for( i = 0; i < queryNds.size(); i++ )
    {
	int v = queryVls[i];
	if(  v != -1 )
	{
	    obsVls.push_back(v);
	    obsDims.push_back(i);
	}
    }

    const pnl::CMatrix<float> * resMat;

    if( obsVls.size())
    {
	resMat = mat->ReduceOp(&obsDims.front(), obsDims.size(),2, &obsVls.front());
    }
    else
    {
	resMat = mat;
    }
    
    pnl::CMatrixIterator<float>* iter = resMat->InitIterator();
    pnl::intVector index;
    TokArr result;
    Tok tmp;
    for( iter; resMat->IsValueHere( iter ); resMat->Next(iter) )
    {
	index.clear();
	float val = *(resMat->Value( iter ));
	resMat->Index( iter, &index );
	int j = 0;

	for( i = 0; i < queryNds.size(); i++ )
	{
	    String nodeName = NodeName(queryNds[i]);
	    String valName  = DiscreteValue(NodeIndex(nodeName.c_str()), 
		queryVls[i] != -1 ? queryVls[i] : index[i] );
	    
	    if(i)
	    {
		tmp ^= (Tok(nodeName) ^ valName);
	    }
	    else
	    {
		tmp = (Tok(nodeName) ^ valName);
	    }
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

int ProbabilisticNet::NodesClassification(TokArr &aValue) const
{
    return Token()->NodesClassification(aValue);
}

// assume that this function is called seldom. It isn't optimal.
void ProbabilisticNet::SetTopologicalOrder(const int *renaming, pnl::CGraph *pnlGraph)
{
    WGraph *newGraph = new WGraph;
    WGraph *oldGraph;
    Vector<int> aiNode(renaming, renaming + Graph()->nNode());
    Vector<String> aName(Graph()->NodeNames(aiNode));
    int i;

    for(i = 0; i < aName.size(); ++i)
    {
	newGraph->AddNode(aName[i]);
	Distributions()->DropDistribution(renaming[i]);
    }
    newGraph->Reset(*pnlGraph);
    oldGraph = Graph();
    StopSpyTo(oldGraph);
    m_pGraph = newGraph;
    Token()->SetGraph(newGraph, true /* bStableNamesNotIndices */);
    Distributions()->StopSpyTo(oldGraph);
    Distributions()->SpyTo(newGraph);
    for(i = 0; i < aName.size(); ++i)
    {
	Distributions()->Setup(i);
    }
    delete oldGraph;
}

pnl::CGraphicalModel *ProbabilisticNet::Model()
{
    if(!m_Model)
    {
	m_Model = m_pCallback->CreateModel(*this);
    }

    return m_Model;
}

void ProbabilisticNet::SetModel(pnl::CGraphicalModel* pModel)
{
    m_Model = pModel;
}
