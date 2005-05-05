#include "pnlWProbabilisticNet.hpp"
#include "NetCallBack.hpp"
#include "pnlWGraph.hpp"
#include "pnlWDistributions.hpp"
#include "TokenCover.hpp"
#include "WDistribFun.hpp"
#include "WCliques.hpp"
#pragma warning(push, 2)
#pragma warning(disable: 4251)
// class X needs to have dll-interface to be used by clients of class Y
#include "pnl_dll.hpp"
#include "pnlGroup.hpp"
#include "pnlString.hpp"
#pragma warning(default: 4251)
#pragma warning(pop)
#include "Wcsv.hpp"
#include "WInner.hpp"
#include "pnlPersistCover.hpp"

#if defined(_MSC_VER)
#pragma warning(disable : 4239) // nonstandard extension used: 'T' to 'T&'
#endif

PNLW_BEGIN

// Tok identifies for discrete and continuous node types
TokArr discrete("nodes^discrete");
TokArr continuous("nodes^continuous");

TokArr chance("nodes^chance");
TokArr decision("nodes^decision");
TokArr value("nodes^value");

ProbabilisticNet::ProbabilisticNet(const char *netType):
m_Model(0), m_pCallback(0), m_bModelValid(false)
{
    m_pGraph = new WGraph();
    m_pTokenCov = new TokenCover(netType, m_pGraph, true);
    m_paDistribution = new WDistributions(m_pTokenCov);
    if(!strcmp(netType, "mrf"))
    {
        Distributions().SetMRF(true);
    }
    SpyTo(m_pGraph);
    SpyTo(m_pTokenCov);
    SpyTo(m_paDistribution);
}

ProbabilisticNet::~ProbabilisticNet()
{
    delete m_paDistribution;
    delete m_pTokenCov;
    delete m_pGraph;
    delete m_pCallback;
}

void ProbabilisticNet::AddNode(TokArr nodes, TokArr subnodes)
{
    for(unsigned int i = 0; i < nodes.size(); ++i)
    {
        Token().AddNode(nodes[i], subnodes);
    }
}

void ProbabilisticNet::DelNode(TokArr nodes)
{
    MustBeNode(nodes);
    for(unsigned int i = 0; i < nodes.size(); ++i)
    {
	Token().DelNode(nodes[i]);
    }
}

// returns one of "discrete" or "continuous"
TokArr ProbabilisticNet::GetNodeType(TokArr nodes)
{
    TokArr res;
    MustBeNode(nodes);// check for existence of such vertices
    for(int i = 0; i < nodes.size(); ++i)
    {
	TokIdNode *pTok = nodes[i].Node(Token().Root());
	res.push_back(pTok->v_prev->Name());
	// Should we search for Discrete or Numeric?
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
	    Graph().AddArc(nameFrom.c_str(), nameTo.c_str());
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
	iTo = Graph().INode(to[i].Name());
	for(j = 0; j < from.size(); ++j)
	{
	    Graph().DelArc(Graph().INode(from[j].Name()), iTo);
	}
    }
}

TokArr ProbabilisticNet::GetNeighbors(TokArr &nodes)
{
    MustBeNode(nodes);

    Vector<int> aiNode;
    ExtractTokArr(nodes, &aiNode, 0, &Graph().MapOuterToGraph());

    pnl::CGraph *graph = Graph().Graph();
    Vector<int> aiResult(nNetNode());
    int i, j;
    pnl::intVector aiNeig;
    pnl::neighborTypeVector aNeigType;

    // accumulate all nodes
    for(i = 0; i < aiNode.size(); ++i)
    {
	graph->GetNeighbors(aiNode[i], &aiNeig, &aNeigType);
	for(j = 0; j < aiNeig.size(); ++j)
	{
	    ++aiResult[aiNeig[j]];
	}
    }

    // list each node once
    TokArr result;
    for(i = 0; i < aiResult.size(); ++i)
    {
	if(!aiResult[i])
	{
	    continue;
	}
	result.push_back(NodeName(Graph().IOuter(i)));
    }

    return result;
}

TokArr ProbabilisticNet::GetParents(TokArr &nodes)
{
    MustBeNode(nodes);

    Vector<int> aiNode;
    pnl::CGraph *graph = Graph().Graph();

    ExtractTokArr(nodes, &aiNode, 0, &Graph().MapOuterToGraph());

    Vector<int> aiResult(nNetNode());
    int i, j;
    pnl::intVector aiNeig;

    // accumulate all nodes
    for(i = 0; i < aiNode.size(); ++i)
    {
	graph->GetParents(aiNode[i], &aiNeig);
	for(j = 0; j < aiNeig.size(); ++j)
	{
	    ++aiResult[aiNeig[j]];
	}
    }

    // list each node once
    TokArr result;
    for(i = 0; i < aiResult.size(); ++i)
    {
	if(!aiResult[i])
	{
	    continue;
	}
	result.push_back(NodeName(Graph().IOuter(i)));
    }

    return result;
}

TokArr ProbabilisticNet::GetChildren(TokArr &nodes)
{
    MustBeNode(nodes);

    Vector<int> aiNode;
    ExtractTokArr(nodes, &aiNode, 0, &Graph().MapOuterToGraph());

    pnl::CGraph *graph = Graph().Graph();
    Vector<int> aiResult(nNetNode());
    int i, j;
    pnl::intVector aiNeig;

    // accumulate all nodes
    for(i = 0; i < aiNode.size(); ++i)
    {
	graph->GetChildren(aiNode[i], &aiNeig);
	for(j = 0; j < aiNeig.size(); ++j)
	{
	    ++aiResult[aiNeig[j]];
	}
    }

    // list each node once
    TokArr result;
    for(i = 0; i < aiResult.size(); ++i)
    {
	if(!aiResult[i])
	{
	    continue;
	}
	result.push_back(NodeName(Graph().IOuter(i)));
    }

    return result;
}

void ProbabilisticNet::EditEvidence(TokArr values)
{
    if(values.size())
    {
	WEvidence evid(m_EvidenceBoard);
	if(!WEvidenceWithCheck(&evid, values))
	{
	    ThrowUsingError("Evidence must be non-empty combination of nodes with values",
		"EditEvidence");
	}
	m_EvidenceBoard = evid;
    }
}

void ProbabilisticNet::ClearEvid()
{
    m_EvidenceBoard.Clear();
}

void ProbabilisticNet::CurEvidToBuf()
{
    EvidenceBuf()->push_back(*EvidenceBoard());
}

void ProbabilisticNet::AddEvidToBuf(TokArr values)
{
    if(values.size())
    {
	WEvidence evid;

	if(WEvidenceWithCheck(&evid, values))
	{
	    EvidenceBuf()->push_back(evid);
	}
    }
}

void ProbabilisticNet::ClearEvidBuf()
{
    EvidenceBuf()->resize(0);
}

bool ProbabilisticNet::WEvidenceWithCheck(WEvidence *pWEvid, TokArr &values)
{
    Token().Resolve(values);
    pWEvid->Set(values);
    try
    {
	pnl::CEvidence *evid = CreateEvidence(pWEvid->Get());
	delete evid;
    }
    catch(...)
    {
	return false;
    }

    return true;
}

int ProbabilisticNet::SaveEvidBuf(const char *filename, NetConst::ESavingType mode)
{
    WLex lex(filename, false/* write */, (mode == NetConst::eCSV) ? ',':'\t');
    int iEvid, iCol, i;
    Vector<int> nUsingCol(nNetNode(), 0);

    // mark nodes for saving
    for(iEvid = 0; iEvid < EvidenceBuf()->size(); ++iEvid)
    {
	TokArr evid = (*EvidenceBuf())[iEvid].Get();
	for(iCol = 0; iCol < evid.size(); ++iCol)
	{
	    i = Graph().IGraph(Token().iNode(evid[iCol]));
	    nUsingCol[i]++;
	}
    }

    Vector<int> aiCSVCol;

    // write header and fill node indices vector
    for(iCol = 0; iCol < nUsingCol.size(); ++iCol)
    {
	if(!nUsingCol[iCol])
	{
	    continue;
	}
	String colName(NodeName(Graph().IOuter(iCol)));
	const pnl::CNodeType &nt = *Model().GetNodeType(iCol);

	aiCSVCol.push_back(iCol);
	if(nt.IsDiscrete())
	{
	    lex.PutValue(colName);
	}
	else
	{
	    String subColName;
	    for(i = 0; i < nt.GetNodeSize(); ++i)
	    {
		subColName = colName;
		subColName << "^" << Token().Value(Graph().IOuter(iCol), i);
		lex.PutValue(subColName);
	    }
	}
    }

    lex.Eol();

    pnl::pnlString str, tmpstr;
    pnl::valueVector v;
    const int *aEvidNode;
    int nEvidNode;

    // write evidences one by one
    for(iEvid = 0; iEvid < EvidenceBuf()->size(); ++iEvid)
    {
	pnl::CEvidence *evid = CreateEvidence((*EvidenceBuf())[iEvid].Get());

	aEvidNode = evid->GetAllObsNodes();
	nEvidNode = evid->GetNumberObsNodes();
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
		evid->GetValues(aEvidNode[i], &v);
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
		    }

		    tmpstr = String(str.c_str());
		    lex.PutValue(tmpstr);
		}
		++i;
	    }
	    else
	    {
                tmpstr = String();
		lex.PutValue(tmpstr);
	    }
	}

	lex.Eol();
    }

    return EvidenceBuf()->size();
}

int ProbabilisticNet::LoadEvidBuf(const char *filename, NetConst::ESavingType mode, TokArr columns)
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
	TokIdNode *node = Tok(colName).Node();

	if(node->tag == eTagNetNode || node->tag == eTagValue)
	{
	    header.push_back(colName);
	    nColInUse++;
	}
	else
	{
	    header.push_back("");
	}
    }

    TokArr tmpTokArr;
    if(columns.size())
    {
	for(iCol = 0; iCol < columns.size(); iCol++)
	{
	    if(columns[iCol] != "")
	    {
                tmpTokArr = TokArr(columns[iCol]);
                MustBeNode(tmpTokArr);
                nColInUse++;
	    }
	}
	header = columns;
    }

    if(nColInUse == 0)
    {
	ThrowUsingError("Nothing to load", funName);
    }

    ClearEvidBuf();

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
	    AddEvidToBuf(evid);
	    nEvid++;
	}
    }

    return nEvid;
}

bool ProbabilisticNet::SaveNet(pnl::CContextPersistence *saver,
			       pnl::CGroupObj *group /* = 0 */)
{
    saver->Put(&Model(), "Model");
    if(group)
    {
	group->Put(new pnl::CCover<ProbabilisticNet>(this), "Nodes", true);
	if(m_aPropertyValue.size())
	{
	    group->Put(new pnl::CCover<SSMap>(&m_aPropertyValue),
		"Properties", true);
	}
    }
    else
    {// this case must be removed later
	saver->Put(new pnl::CCover<ProbabilisticNet>(this), "NodeInfo", true);
	if(m_aPropertyValue.size())
	{
	    saver->Put(new pnl::CCover<SSMap>(&m_aPropertyValue),
		"Properties", true);
	}
    }
    return true;
}

ProbabilisticNet* ProbabilisticNet::LoadNet(pnl::CContextPersistence *loader)
{
    pnl::CGroupObj *group = static_cast<pnl::CGroupObj*>(
	loader->Get("WrapperInfo"));
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

    if(!pCovNet && !group)
    {// Loading of PNL's model
	int iNode, iValue;
	String nodeName;
	String aValue;

	const char *modelName;
        switch(model->GetModelType())
	{
	case pnl::mtBNet:
	    modelName = "bnet";
	    break;
	case pnl::mtIDNet:
	    modelName = "idnet";
	    break;
	case pnl::mtMNet:
	case pnl::mtMRF2:
	    modelName = "mrf";
	    break;
	default:
            ThrowUsingError("Unknown type of model", "LoadNet");
	}
	net = new ProbabilisticNet(modelName);

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
            pnl::EIDNodeState ns = nt.GetNodeState();

            TokArr *classificator;

            if(!nt.IsDiscrete())
            {
                classificator = &continuous;
            }
            else // discrete
            {
                switch(ns)
                {
                case pnl::nsChance:   classificator = &chance;    break;
                case pnl::nsDecision: classificator = &decision;  break;
                case pnl::nsValue:    classificator = &value;     break;
                default: ThrowInternalError("Unknown node state", "LoadNet");break;
                }
            }
            net->AddNode(*classificator ^ nodeName, aValue);
	}
    }
    else
    {
	pnl::CCover<SSMap> *coverProperties;
	if(group)
	{
	    pCovNet = static_cast<pnl::CCover<ProbabilisticNet>*>(group->Get("Nodes"));
	    coverProperties = static_cast<pnl::CCover<SSMap>*>(group->Get("Properties"));
	}
	else
	{// first version of persistence for wrappers
	    coverProperties = static_cast<pnl::CCover<SSMap>*>(loader->Get("Properties"));
	}
	net = pCovNet->GetPointer();

	if(coverProperties)
	{
	    net->m_aPropertyValue = *coverProperties->GetPointer();
	    delete coverProperties;
	}
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
        evid = CreateEvidence(m_EvidenceBoard.Get());
    }

    //define empty buffer of PNL evidences
    pnl::pEvidencesVector newSamples;
    int i;

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
        for(i = nValueIn; --i >= 0;)
        {
	    TokIdNode *node = Token().Node(whatNodes[i]);
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
                aNodePercentage[idx] = whatNodes[i].FltValue();
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

            pnl::CEvidence* newev = pnl::CEvidence::Create( Model().GetModelDomain(), numbers, vv );
            delete ev;
            *it = newev;
        }
    }

    //add generates evidences to wrapper learning buffer
    TokArr token;
    for(i = 0; i < newSamples.size(); ++i)
    {
	GetTokenByEvidence(&token, *newSamples[i]);
	AddEvidToBuf(token);
    }
}

void ProbabilisticNet::MaskEvidBuf(TokArr whatNodes)
{
    static const char fname[] = "MaskEvidBuf";
    int i, j;
    Vector<float> aNodePercentage;

    if(!whatNodes.size())
    {
	return;
    }

    MustBeNode(whatNodes);

    aNodePercentage.resize(whatNodes.size());

    // fill-in probability/frequence for every node
    for(i = 0; i < whatNodes.size(); ++i)
    {
	aNodePercentage[i] = whatNodes[i].fload.size() ? whatNodes[i].FltValue():1.f;
    }

    //loop through generated evidences and clear info about missing variables
    //and delete missing portions of partially observed variables
    for(i = 0; i < m_aEvidence.size(); ++i )
    {
	WEvidence& ev = m_aEvidence[i];

	for(j = 0; j < whatNodes.size(); j++ )
	{
	    if(!ev.IsNodeHere(whatNodes[j]))
	    {
		continue;
	    }

	    const float percentage = aNodePercentage[j];
	    bool bVisible;

	    if( percentage == 0.f )
		bVisible = false;
	    else if( percentage == 1.f )
		bVisible = true;
	    else
	    {
		//random hiding
		bVisible = pnl::pnlRand(0.0f,1.0f) < percentage;
	    }
	    ev.SetVisibility(whatNodes[j], bVisible);
	}
    }
}

void ProbabilisticNet::SetProperty(const char *name, const char *value)
{
    m_aPropertyValue[String(name)] = String(value);
}

String ProbabilisticNet::GetProperty(const char *name) const
{
    SSMap::const_iterator it = m_aPropertyValue.find(String(name));

    return (it == m_aPropertyValue.end()) ? String():it->second;
}

void ProbabilisticNet::Reset(const pnl::CGraphicalModel &model)
{
    if(nNetNode() != model.GetGraph()->GetNumberOfNodes())
    {
	ThrowInternalError("Reset by model with different number of nodes isn't yet implemented", "Reset");
    }
    Graph().Reset(*model.GetGraph());

    int i;

    int nNode = nNetNode();

    if(model.GetModelType() == pnl::mtMNet || model.GetModelType() == pnl::mtMRF2)
    {
	const pnl::CMNet *mrfModel = static_cast<const pnl::CMNet *>(&model);
	nNode = mrfModel->GetNumberOfCliques();
	Vector<int> clique;
	for(i = 0; i < nNode; i++)
	{
	    mrfModel->GetClique(i, &clique);
	    Distributions().Cliques().FormClique(clique);
	}
    }

    for(i = 0; i < nNode; ++i)
    {
	Distributions().ResetDistribution(i, *model.GetFactor(i));
    }
}

//=== inner functions ===

pnl::CEvidence *ProbabilisticNet::CreateEvidence(const TokArr &aValue)
{
    PNL_CHECK_FOR_ZERO(aValue.size());// empty aValue is prohibited
    Vector<int> aiNodeOuter; 
    Vector<int> aiNodeInner, aiValueInner, aNodeFlag;
    Vector<int> aOffset; // offset for every node in vValue
    pnl::valueVector vValue;
    int j, i, nValue;
    static const char fname[] = "CreateEvidence";

    ExtractTokArr(const_cast<TokArr &>(aValue), &aiNodeInner, &aiValueInner, &Graph().MapOuterToGraph());
    Graph().IOuter(&aiNodeInner, &aiNodeOuter);

    Vector<TokIdNode*> apNode = Token().ExtractNodes(const_cast<TokArr &>(aValue));

    aiValueInner.resize(apNode.size(), -1);
    for (i = 0; i< apNode.size(); i++ )
    {
        if (!(static_cast<pnl::CNodeType*>(apNode[i]->v_prev->data)->IsDiscrete()))
        {
	    int size = Token().nValue(aiNodeOuter[i]);
            if (size == 1)
            {
                aiValueInner[i] = GetInt(apNode[i]->v_next);
            }
        }
    }

    aNodeFlag.assign(nNetNode(), 0);

    // mark nodes for evidence (aNodeFlag)
    for(i = aiNodeInner.size(); --i >= 0;)
    {
	++aNodeFlag[aiNodeInner[i]];
    }

    // setup offset for nodes in vValue
    aOffset.assign(aNodeFlag.size(), -1);
    for(j = i = 0; i < aNodeFlag.size(); ++i)
    {
	if(aNodeFlag[i])
	{
	    pnl::CNodeType nt = pnlNodeType(Graph().IOuter(i));
	    aOffset[i] = j;
	    j += (nt.IsDiscrete() ? 1:nt.GetNodeSize());
	}
    }

    nValue = j;

    if(nValue != aValue.size())
    {
	ThrowUsingError("Missed value isn't allowed", fname);
    }

    // check for unambiguity
    vValue.resize(nValue, pnl::Value(0));
    for(i = aiNodeInner.size(); --i >= 0;)
    {
	j = aOffset[aiNodeInner[i]];
	pnl::CNodeType nt = pnlNodeType(Graph().IOuter(aiNodeInner[i]));
	if(!nt.IsDiscrete())
	{
	    j += aiValueInner[i];
	}
	// check for repeating
	if(vValue[j].GetInt())
	{
	    pnl::pnlString str;
	    str << "Ambiguous using of variable " << NodeName(Graph().IOuter(j));
	    ThrowUsingError(str.c_str(), fname);
	}
	vValue[j] = 1;
    }

    // here aiNodeInner is sorted
    for(i = 0, nValue = 0; i < aiNodeInner.size(); ++i)
    {
	pnl::CNodeType nt = pnlNodeType(Graph().IOuter(aiNodeInner[i]));

	if(nt.IsDiscrete())
	{
	    pnl::Value &value = vValue[aOffset[aiNodeInner[i]]];
	    if(aiValueInner[i] == -1)
	    {
		if(aValue[i].FltValue(0).IsUndef())
		{
		    ThrowUsingError("Absent value for node", fname);
		}
		value = pnl::Value(int(aValue[i].FltValue(0).fl));
	    }
	    else
	    {
		value = aiValueInner[i];
	    }
	}
	else // continuous
	{
	    if(aiValueInner[i] == -1)
	    {
		if(nt.GetNodeSize() != 1)
		{
		    ThrowUsingError("Absent value for node", fname);
		}
		aiValueInner[i] = 0;
	    }

	    if(aValue[i].fload.size() != 1)// There is must be one value - check
	    {
		ThrowUsingError("Incorrect evidence - number of values isn't expected", fname);
	    }
	    vValue[aOffset[aiNodeInner[i]] + aiValueInner[i]] = aValue[i].FltValue(0).fl;
	}
    }

    //aiNewNode does not contain one node more than once
    Vector<int> aiNewNode;
    int size = aiNodeInner.size();

    aiNewNode.reserve(size);

    if (size != 0)
    {
	aiNewNode.push_back(aiNodeInner[0]);
    }

    for (int node = 1; node < size; ++node)
    {
	if (aiNodeInner[node] != aiNodeInner[node-1])
	{
	    aiNewNode.push_back(aiNodeInner[node]);
	}
    }

    return pnl::CEvidence::Create(&Model(), aiNewNode, vValue);
}

void ProbabilisticNet::GetTokenByEvidence(TokArr *tEvidence, pnl::CEvidence &evidence)
{
    pnl::intVector aiNode;
    pnl::valueVecVector vValue;
    pnl::pConstNodeTypeVector nodeTypes;
    int i, k;

    evidence.GetObsNodesWithValues(&aiNode, &vValue, &nodeTypes);
    Graph().IndicesGraphToOuter(&aiNode, &aiNode);
    tEvidence->resize(0);
    for(i = 0; i < aiNode.size(); ++i)
    {
	String nodeName = NodeName(aiNode[i]);
	String valName;
	if(nodeTypes[i]->IsDiscrete())
	{
	    valName  = DiscreteValue(aiNode[i], vValue[i][0].GetInt());
	    tEvidence->push_back(Tok(nodeName) ^ valName);
	}
	else
	{
	    if(nodeTypes[i]->GetNodeSize() != vValue[i].size())
	    {
		ThrowInternalError("Inconsistent arrays sizes from PNL",
		    "ProbabilisticNet::GetTokenByEvidence");
	    }
	    for(k = 0; k < vValue[i].size(); ++k)
	    {
		valName = DiscreteValue(aiNode[i], k);
		tEvidence->push_back(Tok(nodeName) ^ valName ^ vValue[i][k].GetFlt());
	    }
//	    ThrowInternalError("Not yet realized", "GetTokenByEvidence");
	}
    }
}

int ProbabilisticNet::nNetNode() const
{
    return Graph().nNode();
}

void ProbabilisticNet::MustBeNode(TokArr &nodes) const
{
    for(int i = nodes.size(); --i >= 0;)
    {
	Token().Resolve(nodes[i]);
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
    return Token().Node(node)->tag == eTagNetNode;
}

String ProbabilisticNet::NodeName(int iNode) const
{
    return Graph().NodeName(iNode);
}

int ProbabilisticNet::NodeIndex(const char *name) const
{
    return Graph().INode(name);
}

int ProbabilisticNet::NodeIndex(TokIdNode *node) const
{
    return Graph().INode(node->Name());
}

String ProbabilisticNet::DiscreteValue(int iNode, int value) const
{
    TokIdNode *node = Token().Node(iNode);
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
    Vector<TokIdNode*> apNode = Token().ExtractNodes(aNode);
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
	bool bValueForCurr = (aNode[i].Node(Token().Root()) != apNode[i]);
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
	    TokIdNode *pValue = aNode[i].Node(Token().Root());
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
	Tok ttmp;

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
		    ttmp = aNode[i - 1];
		    aNode[i - 1] = aNode[i];
		    aNode[i] = ttmp;
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

void ProbabilisticNet::ExtractTok(Tok &aNode, Vector<int> *paiNode,
	Vector<int> *paiValue, float &probValue, IIMap *pMap) const
{
    static const char fname[] = "ExtractTok";
    Vector<TokIdNode*> apNode = Token().ExtractNodes(aNode);
    int i, j, nNode;
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
	bool bValueForCurr = (aNode.Node(Token().Root()) != apNode[i]);
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
    
/*
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
*/
    
    if(bValue)
    {
        for(i = 0; i < nNode; ++i)
        {
            Vector<TokIdNode *> pValue = aNode.Nodes(Token().Root());
            for (j = 0; j < pValue.size(); j++)
            {
                if(pValue[i] != apNode[i])
                {
                    if(pValue[i]->tag != eTagValue)
                    {
                        ThrowInternalError("Inner error or wrong usage of bayes net",
                            fname);
                    }
                    if(paiValue && paiValue->size() != nNode)
                    {
                        paiValue->assign(nNode, -1);
                    }
                    (*paiValue)[i] = GetInt(pValue[i]);
                }
            }
        }
    }

    probValue = aNode.FltValue();
    
/*
    if(pMap)
    {// sort parent by index
	bool bReverse = true;
	int tmp;
	Tok ttmp;

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
		    ttmp = aNode[i - 1];
		    aNode[i - 1] = aNode[i];
		    aNode[i] = ttmp;
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
    */
}

int ProbabilisticNet::GetInt(TokIdNode *node)
{
    return TokenCover::Index(node);
}

pnl::CNodeType ProbabilisticNet::pnlNodeType(int i) const
{
    return m_paDistribution->NodeType(i);
}

void ProbabilisticNet::Accumulate(TokArr *pResult, Vector<int> &aIndex,
			  pnl::CMatrix<float> *mat, String &prtName, int prtValue) const
{
    float val = mat->GetElementByIndexes(&aIndex.front());
    String valName = DiscreteValue(NodeIndex(prtName.c_str()), prtValue);

    (*pResult) << (Tok(prtName) ^ valName ^ val);
}

void ProbabilisticNet::TranslateBufToEvidences(
    pnl::pEvidencesVector *paEvidence, int startEvid)
{
    int i, sz;

    sz = m_aEvidence.size() - startEvid;
    paEvidence->resize(sz);
    for(i = 0; (i + startEvid) < m_aEvidence.size(); ++i)
    {
	(*paEvidence)[i] = CreateEvidence(m_aEvidence[i + startEvid].Get());
    }
}

void DropEvidences(pnl::pEvidencesVector &raEvidence)
{
    for(int i = 0; i < raEvidence.size(); ++i)
    {
	delete raEvidence[i];
    }
    raEvidence.resize(0);
}

Tok ProbabilisticNet::ConvertMatrixToToken(const pnl::CMatrix<float> *mat)
{
    pnl::CDenseMatrix<float> *dMat = mat->ConvertToDense();
    const pnl::floatVector *matVec = dMat->GetVector();
    std::vector<float> matr(matVec->begin(), matVec->end());

    return Tok(matr);
}

void ProbabilisticNet::DoNotify(const Message &msg)
{
    switch(msg.MessageId())
    {
    case Message::eMSGDelNode:
    case Message::eChangeParentNState:
    case Message::eChangeNState:
    case Message::eInit:
    case Message::eSetModelInvalid:
        SetModelInvalid();
        break;
    case Message::eChangeName: break;//Current bug, because node name in all evidences have to  be changed
    default:
        ThrowInternalError("Unhandled message arrive" ,"DoNotify");
        return;
    }
}

void ProbabilisticNet::SetModelInvalid()
{
    if(!IsModelValid())
    {
	return;
    }

    m_bModelValid = false;
    Notify(Message::eSetModelInvalid, -1);
}

void ProbabilisticNet::SplitNodesByObservityFlag(Vector<int> *aiObserved, Vector<int> *aiUnobserved)
{
    PNL_CHECK_IS_NULL_POINTER(aiObserved);

    TokArr board = m_EvidenceBoard.Get();
    Vector<TokIdNode*> aObs = Token().ExtractNodes(board);
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

    Token().SetContext(result);

    return result;
}

pnl::CGraphicalModel &ProbabilisticNet::Model()
{
    if(m_Model && IsModelValid())
    {
	return *m_Model;
    }

    // model is absent or invalid

    // create new model
    pnl::CGraphicalModel *model = m_pCallback->CreateModel(*this);

    if(m_Model)// model exists - then it invalid
    {	// delete old model and set new model
	delete m_Model;
    }
    m_Model = model;

    m_bModelValid = true;

    return *m_Model;
}

PNLW_END
