#include "DBN.hpp"
#include "WInner.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "pnlWDistributions.hpp"
#include "pnlWGraph.hpp"
#include "WDistribFun.hpp"
#include "TokenCover.hpp"
#include "DBNCB.hpp"

#pragma warning(push, 2)
#pragma warning(disable: 4251)
// class X needs to have dll-interface to be used by clients of class Y
#include "pnl_dll.hpp"
#include "pnlMlStaticStructLearnHC.hpp"
#include "pnlString.hpp"
#pragma warning(default: 4251)
#pragma warning(pop)

#if defined(_MSC_VER)
#pragma warning(disable : 4239) // nonstandard extension used: 'T' to 'T&'
#endif

DBN::DBN(): m_Inference(0), m_Learning(0), m_nLearnedEvidence(0)
{
    static const char *aInference[] = 
    {
	"Smothing", "FixLagSmoothing", "Filtering", "Viterbi"
    };
    
    static const char *aLearning[] = 
    {
	"EM Learning for DBN" 
    };
    
    m_pNet = new ProbabilisticNet();
    m_pNet->SetCallback(new DBNCallback());
    m_pNet->Token()->AddProperty("Inference", aInference,
	sizeof(aInference)/sizeof(aInference[0]));
}

DBN::~DBN()
{
    delete m_Inference;
    delete m_Learning;
    delete m_pNet;
}

void DBN::AddNode(TokArr nodes, TokArr subnodes)
{
    
    Net().AddNode(nodes, subnodes);
    
}

void DBN::DelNode(TokArr nodes)
{
    TokArr priorNodes,fsliceNodes;
    String tmpStr,prStr,slStr;
    int i;
    for(i = 0; i < nodes.size();i++ )
    {
	tmpStr = nodes[i].Name();
	tmpStr = GetShortName(tmpStr);
	prStr = tmpStr;
	slStr = tmpStr;
	const char *s = prStr.c_str();
	prStr<<"-0";
	slStr<<"-1";
        priorNodes.push_back(prStr);
	fsliceNodes.push_back(slStr);
    }
    const char *s = prStr.c_str();
    Net().DelNode(fsliceNodes);
    Net().DelNode(priorNodes);
    
}


// returns one of "categoric" or "continuous"
TokArr DBN::GetNodeType(TokArr nodes)
{
    return Net().GetNodeType(nodes);
}


// manipulating arcs
void DBN::AddArc(TokArr from, TokArr to)
{
    Net().AddArc(from, to);   
}

void DBN::DelArc(TokArr from, TokArr to)
{
    Net().AddArc(from, to);
}

static int cmpTokIdNode(TokIdNode *node1, TokIdNode *node2)
{
    return (char*)node1->data - (char*)node2->data;
}

// It is inner DistribFun
void DBN::SetPTabular(TokArr value, TokArr prob, TokArr parentValue)
{
    Net().Distributions()->FillData(value, prob, parentValue);
}

void DBN::SetPGaussian(TokArr node, TokArr mean, TokArr variance, TokArr weight)
{
    Net().Distributions()->FillData(node, mean, TokArr(), pnl::matMean);
    Net().Distributions()->FillData(node, variance, TokArr(), pnl::matCovariance);
    if (weight.size() != 0)
        Net().Distributions()->FillData(node, weight, TokArr(), pnl::matWeights);
}

TokArr DBN::GetGaussianMean(TokArr vars)
{
    static const char fname[] = "GetGaussianMean";

    if( !vars.size() )
    {
	ThrowUsingError("Must be at least one combination for a child node", fname);
    }

    int nnodes = vars.size();
    Vector<int> queryNds, queryVls;
    Net().ExtractTokArr(vars, &queryNds, &queryVls);
    if(!queryVls.size())
    {
	queryVls.assign(nnodes, -1);
    }

    const pnl::CFactor * cpd = Model()->GetFactor(queryNds.front());

    if (cpd->IsDistributionSpecific() == 1)         
    {
        TokArr res;
        res << "uniform";
        return res;
    }
    else
        if (cpd->GetDistribFun()->GetDistributionType() == pnl::dtScalar)
        {
            TokArr res;
            res << "scalar";
            return res;  
        }
        else
        {
            const pnl::CMatrix<float> *mat = cpd->GetMatrix(pnl::matMean);
            return Net().ConvertMatrixToToken(mat);
        }
}

TokArr DBN::GetGaussianCovar(TokArr var, TokArr vars)
{
 static const char fname[] = "GetGaussianCovar";

    if( !var.size() )
    {
	ThrowUsingError("Must be at least one combination for a child node", fname);
    }

    int nnodes = var.size();
    Vector<int> queryNds, queryVls;
    Net().ExtractTokArr(var, &queryNds, &queryVls);
    if(!queryVls.size())
    {
	queryVls.assign(nnodes, -1);
    }

    const pnl::CFactor * cpd = Model()->GetFactor(queryNds.front());

    if (cpd->GetDistribFun()->IsDistributionSpecific() == 2) // delta
    {
        TokArr res;
        res << "0";
        return res;
    }
    else
        if (cpd->GetDistribFun()->IsDistributionSpecific() == 1)
        {
            TokArr res;
            res << "uniform";
            return res;
        }
        else
            if (cpd->GetDistribFun()->GetDistributionType() == pnl::dtScalar)
            {
                TokArr res;
                res << "scalar";
                return res;
            }
            else
            {
                const pnl::CMatrix<float> *mat = cpd->GetMatrix(pnl::matCovariance);
                return Net().ConvertMatrixToToken(mat);
            }
}

TokArr DBN::GetPTabular(TokArr child, TokArr parents)
{
    static const char fname[] = "P";
    
    int nchldComb = child.size();
    if( !nchldComb )
    {
	ThrowUsingError("Must be at least one combination for a child node", fname);
    }
    
    Vector<int> childNd, childVl;
    Net().ExtractTokArr(child, &childNd, &childVl);
    
    if( !childVl.size())
    {
	childVl.assign(nchldComb, -1);
    }
    
    Vector<int> parentNds, parentVls;
    int nparents = parents.size();
    if( nparents )
    {
	Net().ExtractTokArr(parents, &parentNds, &parentVls);
	if( parentVls.size() == 0 || 
	    std::find(parentVls.begin(), parentVls.end(), -1 ) != parentVls.end() )
	{
	    ThrowInternalError("undefindes values for given parent nodes", "P");
	}
    }
    else
    {
	Net().Graph()->Graph()->GetParents( childNd.front(), &parentNds );
	nparents = parentNds.size();
	parentVls.assign(nparents, -1);
    }
    
    parentNds.resize(nparents + 1);
    parentVls.resize(nparents + 1);
    
    const pnl::CFactor * cpd = Model()->GetFactor(childNd.front());
    const pnl::CMatrix<float> *mat = cpd->GetMatrix(pnl::matTable);
    
    TokArr result = "";
    int i;
    for( i = 0; i < nchldComb; i++ )
    {
	parentNds[nparents] = childNd.front();
	parentVls[nparents] = childVl[i];
	result << Net().CutReq( parentNds, parentVls, mat);
    }    
    
    return result;
}

TokArr DBN::GetJPD( TokArr nodes)
{
    static const char fname[] = "JPD";
    
    TokArr NewQue;
    String tmpStr;
    Tok *pTok;
    tmpStr = nodes[nodes.size() - 1].Name();
    int nSlice = GetSliceNum(tmpStr);
    
    if( !nodes.size())
    {
	ThrowInternalError("undefined query nodes", "JPD");
    }
    
    /* pnl::CEvidence *evid = NULL;
    if( Net().EvidenceBoard()->IsEmpty() )
    {
    evid = pnl::CEvidence::Create(Model()->GetModelDomain(), 0, NULL, pnl::valueVector(0));
    }
    else
    {
    evid = Net().CreateEvidence(Net().EvidenceBoard()->GetBoard());
    }
    */
    pnl::CEvidence **pEvid;
    pEvid = new pnl::CEvidence*[GetNumSlices()];
    int i;
    for(i = 0; i < GetNumSlices(); i++)
    {
	pEvid[i] = (m_AllEvidences[i])[m_AllEvidences[i].size() - 1]; 
    }
    switch(PropertyAbbrev("Inference"))
    {
    case 's': 
	Inference().DefineProcedure(pnl::ptSmoothing, GetNumSlices());
	Inference().EnterEvidence(pEvid,GetNumSlices());
	Inference().Smoothing();
	break;
    case 'x':
	Inference().DefineProcedure(pnl::ptFixLagSmoothing,0 );
	int slice;
	Inference().EnterEvidence( &(pEvid[nSlice]), 1 );
	Inference().FixLagSmoothing( nSlice );
	break;
    case 'f':
	Inference().DefineProcedure(pnl::ptFiltering,0 );
	Inference().EnterEvidence( &(pEvid[nSlice]), 1 );
	Inference().Filtering( nSlice );
	break;		
    default:
	ThrowUsingError("Setted wrong property", fname);
	break;
    };
    
    for(i = 0; i < nodes.size(); i++)
    {
	tmpStr = nodes[i].Name();
	if(nSlice != 0)
	{
	    if(GetSliceNum(tmpStr) == nSlice )
	    {
		tmpStr = GetShortName(tmpStr);
		tmpStr<<"-1";
		NewQue.push_back(tmpStr);
	    }
	    else
	    {
		tmpStr = GetShortName(tmpStr);
		tmpStr<<"-0";
		NewQue.push_back(tmpStr);
	    };
	}
	else
	{
	    NewQue.push_back(nodes[i]);
	}
    };
    int nnodes = nodes.size();
    Vector<int> queryNds, queryVls;
    
    Net().ExtractTokArr(NewQue, &queryNds, &queryVls);
    if(!queryVls.size())
    {
	queryVls.assign(nnodes, -1);
    }
    
    
    Inference().MarginalNodes(&queryNds.front(), queryNds.size(),nSlice);
    
    
    const pnl::CPotential *pot = Inference().GetQueryJPD();
    
    TokArr res = "";
    if (pot->GetDistribFun()->GetDistributionType() == pnl::dtTabular)
    {
        const pnl::CMatrix<float> *mat = pot->GetMatrix(pnl::matTable);
        res << Net().CutReq( queryNds, queryVls, mat);
    }
    else
    {
        if (pot->GetDistribFun()->IsDistributionSpecific() != 1)
        {
            const pnl::CMatrix<float> *mean = pot->GetMatrix(pnl::matMean);
            res << Net().ConvertMatrixToToken(mean);
	    
            if (pot->GetDistribFun()->IsDistributionSpecific() != 2)
            {
                const pnl::CMatrix<float> *cov = pot->GetMatrix(pnl::matCovariance);
                res << Net().ConvertMatrixToToken(cov);
            }
            else
            {
                res << "0";
            }
        }
        else
        {
	    // ?
            res << "uniform";
            res << "distribution";
        }
    }
    if (nSlice <= 1)  
    {
	Net().Token()->Resolve(res);
    }
    res = ConvertBNetQueToDBNQue(res,nSlice); 
    return res;	
}

void DBN::EditEvidence(TokArr values)
{
    TokArr NewQue;
    String tmpStr,tmpName;
    int i;
    int nSlice = GetSliceNum(GetNodeName(values[0]));
    for(i = 0; i < values.size(); i++)
    { 	
	tmpName = GetNodeName(values[i]);
	if(GetSliceNum(tmpName) == nSlice)
	{
	    tmpStr = GetShortName(tmpName);
	    tmpStr << "-0^";
            tmpStr << GetValue(values[i]);
	    NewQue.push_back(tmpStr);
	}
	else
	{
	    tmpStr = GetShortName(tmpName);
	    tmpStr << "-1^";
            tmpStr << GetValue(values[i]);
	    NewQue.push_back(tmpStr);
	};
    };    
    Net().EditEvidence(NewQue); 
    m_curSlice = nSlice;
}

void DBN::CurEvidToBuf()
{
    pnl::CEvidence *evid = NULL; 
    evid = Net().CreateEvidence(Net().EvidenceBoard()->GetBoard());
    (m_AllEvidences[m_curSlice]).push_back(evid);
}

// adds evidence to the buffer
void DBN::AddEvidToBuf(TokArr values)
{
    EditEvidence(values);
    CurEvidToBuf();
}

void DBN::PushEvid(TokArr const values[], int nValue)
{
    // Net().PushEvid(values, nValue);
    int i;
    for(i = 0; i < nValue; i++)
    {
	EditEvidence(values[i]);
    }
}

void DBN::ClearEvid()
{
    Net().ClearEvid();
    m_AllEvidences.clear();
    
}

void DBN::ClearEvidBuf()
{
    Net().ClearEvidBuf();
    m_nLearnedEvidence = 0;
}


void DBN::LearnParameters()
{  
    int i;
    /*if(m_nLearnedEvidence > Net().EvidenceBuf()->size())
    {
    ThrowInternalError("inconsistent learning process", "Learn");
    }
    
      if(m_nLearnedEvidence == Net().EvidenceBuf()->size())
      {
      return;// is it error?
      }     
}*/
    
    Learning().SetData(static_cast<const pnl::pEvidencesVecVector>(m_AllEvidences));
    Learning().Learn();
    for (i = 0; i < Net().Graph()->iNodeMax(); i++)
    {
	if(Net().Graph()->IsValidINode(i))
	{
	    Net().Distributions()->ResetDistribution(i, *Net().Model()->GetFactor(Net().Graph()->IGraph(i)));
	}
    }
}

TokArr DBN::GetMPE(TokArr nodes)
{
    TokArr NewQue,OutQue;
    String tmpStr, tmpRez,tmpS,tmpVal;
    Tok *pTok;
    tmpStr = nodes[nodes.size() - 1].Name();
    int nSlice = GetSliceNum(tmpStr);
    if( !nodes.size())
    {
	ThrowInternalError("undefined query nodes", "MPE");
    }
    //  Net().MustBeNode(nodes);	
    pnl::CEvidence **pEvid;
    pEvid = new pnl::CEvidence*[GetNumSlices()];
    int i,j;
    for(i = 0; i < GetNumSlices(); i++)
    {
	pEvid[i] = (m_AllEvidences[i])[m_AllEvidences[i].size() - 1]; 
    }
    switch(PropertyAbbrev("Inference"))
    {
    case 'v':
	
	Inference().DefineProcedure(pnl::ptViterbi,nSlice);
	Inference().EnterEvidence( pEvid, nSlice);
	Inference().FindMPE();
	
	break;
    default:
	ThrowInternalError("Procedure must be viterbi for mpe", "MPE");
	break;
    };
    
    
    for(i = 0; i < nodes.size(); i++)
    {
	tmpStr = nodes[i].Name();
	if(nSlice != 0)
	{
	    if(GetSliceNum(tmpStr) == nSlice + 1 )
	    {
		tmpStr = GetShortName(tmpStr);
		tmpStr<<"-1";
		NewQue.push_back(tmpStr);
	    }
	    else
	    {
		tmpStr = GetShortName(tmpStr);
		tmpStr<<"-0";
		NewQue.push_back(tmpStr); 
	    };
	}
	else
	{
	    NewQue.push_back(nodes[i]);
	}
    };
    
    int nnodes = nodes.size();
    Vector<int> queryNds;
    if( nnodes )
    {
	queryNds.resize(nnodes);
	for(i = 0; i < nnodes; ++i)
	{
	    Tok tok = NewQue[i];
	    queryNds[i] = Net().NodeIndex( tok.Node() );
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
    
    
    Inference().MarginalNodes(&queryNds.front(), queryNds.size());
    
    const pnl::CEvidence *mpe = Inference().GetMPE();
    TokArr result;
    
    for(i = 0; i < nnodes; ++i)
    {
	pnl::Value v = *mpe->GetValue(queryNds[i]);
	
	if((v.IsDiscrete() != 0) != Net().pnlNodeType(queryNds[i]).IsDiscrete())
	{
	    ThrowInternalError("Non-discrete value for discrete variable", "MPE");
	}
	
        if (Net().pnlNodeType(queryNds[i]).IsDiscrete())
            result.push_back(Net().Token()->TokByNodeValue(queryNds[i], v.GetInt()));
        else
	    result.push_back(v.GetFlt());
    }
    if (nSlice <= 1)  
    {
	Net().Token()->Resolve(result);
    }
    result = ConvertBNetQueToDBNQue(result,nSlice);
    return result;
}

void DBN::SaveNet(const char *filename)
{
    pnl::CContextPersistence saver;
    
    if(!Net().SaveNet(&saver))
    {
	ThrowInternalError("Can't save file", "SaveNet");
    }
    
    if(!saver.SaveAsXML(filename))
    {
	ThrowInternalError("Can't save file", "SaveNet");
    }
    // we must save net-specific data here.
    // DBN haven't any specific data for now
}

int DBN::SaveEvidBuf(const char *filename, NetConst::ESavingType mode)
{
    WLex lex(filename, false/* write */, (mode == NetConst::eCSV) ? ',':'\t');
    int iEvid, iCol, i,j;
    Vector<int> nUsingCol(Net().nNetNode(), 0);
//    Vector<int> nSlices;
    int evNum = 0;
    const int *aEvidNode;
    int nEvidNode;
    
    // mark nodes for saving
    for(i = 0; i < GetNumSlices(); i++)
    for(iEvid = 0; iEvid < m_AllEvidences[i].size(); ++iEvid)
    {
	evNum++;
	aEvidNode = m_AllEvidences[i][iEvid]->GetAllObsNodes();
	nEvidNode = m_AllEvidences[i][iEvid]->GetNumberObsNodes();
	for(iCol = 0; iCol < nEvidNode; ++iCol)
	{
	    nUsingCol[aEvidNode[iCol]]++;
	}
    }

    Vector<int> aiCSVCol;
    String sliceHeader = "Slice";
    lex.PutValue(sliceHeader); 
    // write header and fill node indices vector
    for(iCol = 0; iCol < nUsingCol.size(); ++iCol)
    {
	if(!nUsingCol[iCol])
	{
	    continue;
	}
	String colName(Net().NodeName(Net().Graph()->IOuter(iCol)));
	const pnl::CNodeType &nt = *Model()->GetNodeType(iCol);

	aiCSVCol.push_back(iCol);
	if(nt.IsDiscrete())
	{
	    lex.PutValue(GetShortName(colName));
	}
	else
	{
	    String subColName;
	    for(i = 0; i < nt.GetNodeSize(); ++i)
	    {
		subColName = colName;
		subColName << "^" <<Net().Token()->Value(Net().Graph()->IOuter(iCol), i);
		lex.PutValue(GetShortName(subColName));
	    }
	}
    }

    lex.Eol();

    pnl::pnlString str, tmpstr;
    pnl::valueVector v;
    
    // write evidences one by one
    for(j = 0; j < GetNumSlices(); j++)
    for(iEvid = 0; iEvid < m_AllEvidences[i].size(); ++iEvid)
    {
	aEvidNode = m_AllEvidences[j][iEvid]->GetAllObsNodes();
	nEvidNode = m_AllEvidences[j][iEvid]->GetNumberObsNodes();
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
	char c[2];
	itoa(j,c,10);
	tmpstr = String(c);
	lex.PutValue(tmpstr);
	// iCol here - index in aiCSVCol
	for(iCol = 0, i = 0; iCol < aiCSVCol.size(); ++iCol)
	{
	    if(aiCSVCol[iCol] == aEvidNode[i])
	    {
		m_AllEvidences[j][iEvid]->GetValues(aEvidNode[i], &v);
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
			str << Net().DiscreteValue(aEvidNode[i], v[j].GetInt());
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

    return evNum;
}

void DBN::LoadNet(const char *filename)
{
    pnl::CContextPersistence loader;
    
    if(!loader.LoadXML(filename))
    {
	ThrowUsingError("Can't load file - bad file?", "LoadNet");
    }
    
    ProbabilisticNet *pNewNet = ProbabilisticNet::LoadNet(&loader);
    if(!pNewNet)
    {
	ThrowUsingError("Can't load file - bad file?", "LoadNet");
    }
    
    delete &Net();
    m_pNet = pNewNet;
    m_pNet->SetCallback(new DBNCallback());
    
    // We must load net-specific data here.
    // DBN haven't any specific data for now
}

int DBN::LoadEvidBuf(const char *filename, NetConst::ESavingType mode, TokArr columns)
{
    static const char funName[] = "LoadLearnBuf";

    WLex lex(filename, true/* read */, (mode == NetConst::eCSV) ? ',':'\t');
    TokArr header;
    int iCol, nCol, nColInUse;
    int nEvid;
    int nslice;
    String colName, sliceHeader,numSlice;
    
    lex.GetValue(&sliceHeader);

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
	colName<<"-0";
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
                Net().MustBeNode(tmpTokArr);
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
	lex.GetValue(&numSlice);
	const char * ns = numSlice.c_str();
	nslice = atoi(ns);
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
	    //AddEvidToBuf(evid);
	    Net().ClearEvid();
	    EditEvidence(evid);
	    pnl::CEvidence *evid = NULL; 
            evid = Net().CreateEvidence(Net().EvidenceBoard()->GetBoard());
           (m_AllEvidences[nslice]).push_back(evid);
	    nEvid++;
	}
    }

    return nEvid;
}

// whatNodes is array of tokens which specify the list of variables and optionally 
// the required portion of observed nodes
// For example, if whatNodes = TokArr("Node1")
//    
void DBN::GenerateEvidences(pnl::intVector nSlices)
{
    Model()->GenerateSamples(&m_AllEvidences,nSlices);
}

pnl::CMatrix<float> *DBN::Matrix(int iNode) const
{
    pnl::CMatrix<float> *mat = Net().Distributions()->Distribution(iNode)->Matrix(pnl::matTable);
    
    return mat;
}

pnl::CDynamicInfEngine &DBN::Inference()
{
    if (!m_Inference)
    {
	m_Inference = pnl::C1_5SliceJtreeInfEngine::Create(Model());  
    }
    return *m_Inference;
}

pnl::CDynamicLearningEngine &DBN::Learning()
{
    if(!m_Learning)
    {
	m_Learning = pnl::CEMLearningEngineDBN::Create(Model());
    };
    return *m_Learning;
}

pnl::CDBN *DBN::Model()
{
    return static_cast<pnl::CDBN*>(Net().Model());
}

void DBN::SetProperty(const char *name, const char *value)
{
    m_pNet->SetProperty(name, value);
}

String DBN::GetProperty(const char *name) const
{
    return m_pNet->GetProperty(name);
}

void DBN::SetNumSlices(int nSlices)
{
    char c[2];
    itoa(nSlices,c,10);
    m_pNet->SetProperty("NumSlices",c);
    m_AllEvidences.resize(nSlices);
}

int DBN::GetNumSlices()
{
    String nslicesStr = GetProperty("NumSlices");
    if(nslicesStr.length() == 0)
    {
	return 2;
    };
    return atoi(nslicesStr.c_str());
}
    

const char DBN::PropertyAbbrev(const char *name) const
{   
    if(!strcmp(name,"Inference"))
    {
	String infName = GetProperty("Inference");
	
	if(infName.length() == 0)
	{
	    return 's';
	}
	pnl::pnlVector<char> infNameVec(infName.length());
	for(int i = 0; i < infName.length(); ++i)
	{
	    infNameVec[i] = tolower(infName[i]);
	}
	char *pInfName = &infNameVec.front();
	
	if(strstr(pInfName, "smooth"))
	{
	    return 's';
	}
	if(strstr(pInfName, "fix"))
	{
	    return 'x';
	}
	if(strstr(pInfName, "filt"))
	{
	    return 'f';
	}
	if(strstr(pInfName, "viter"))
	{
	    return 'v';
	}
	else 
	{
	    return 's';
	}
    }
   else
   {
       return 0;
   }
}

int DBN::GetSliceNum(String nodeName)
{
    String tmpStr;
    bool flag = false; 
    int i;
    char s;
    for(i = 0; i < nodeName.length(); i++)
    {
	if(flag)
	{
	    s = nodeName[i];
	    tmpStr.append(&s,1);
	}
	if(nodeName[i] == '-') 
	{
	    flag = true;
	}
    }
    return atoi(tmpStr.c_str());
}

String DBN::GetNodeName(String nodeEvid)
{
    String tmpName;
    int i;
    for( i = 0; i < nodeEvid.length();i++)
    {
	if(nodeEvid[i] == '^') break;
	char s = nodeEvid[i];
	tmpName.append(&s,1);
    }
    return tmpName;
}

String DBN::GetShortName(String nodeName)
{
    String tmpName;
    int i;
    for( i = 0; i < nodeName.length();i++)
    {
	if(nodeName[i] == '-') break;
	char s = nodeName[i];
	tmpName.append(&s,1);
    }
    return tmpName;
    
    
}

String DBN::GetValue(String nodeEvid)
{
    String tmpValue;
    bool flag = false;
    int i;
    for( i = 0; i < nodeEvid.length(); i++)
    {
	if(flag)
	{
	    char s = nodeEvid[i];
	    tmpValue.append(&s,1); 
	}
	if( nodeEvid[i] == '^')
	{
	    flag = true;
	}
    }
    return tmpValue;
}

TokArr DBN::ConvertBNetQueToDBNQue(TokArr bnetQue,int nSlice)
{
    int i,j;
    String tmpRez,tmpS,tmpVal;
    TokArr  OutQue;
    
    if(nSlice != 0)
    {
	for(i = 0; i < bnetQue.size(); i++)
	{
	    tmpRez = String(bnetQue[i]);
	    tmpVal.resize(0);
	    for(j = 0; j < tmpRez.length(); j++)
	    {
		if(tmpRez[j] != '^')
		{
		    const char e = tmpRez[j];
		    tmpVal.append(&e,1);
		}
		else
		{
		    if(!(GetShortName(tmpVal) == tmpVal))
		    {
			if(GetSliceNum(tmpVal) == 0)
			{
			    tmpVal = GetShortName(tmpVal);
			    tmpVal<<"-";
			    char c[2];  
			    itoa(nSlice - 1,c,2);
			    tmpVal.append(c,strlen(c));
			}
			else
			{
			    tmpVal = GetShortName(tmpVal);
			    tmpVal<<"-";
			    char c[2];  
			    itoa(nSlice,c,10);
			    tmpVal.append(c,strlen(c));
			}
			
		    }
		    
		    tmpS.append(tmpVal.c_str(),tmpVal.length());
		    tmpS<<"^";
		    tmpVal.resize(0);
		}
	    }
	    tmpS.append(tmpVal.c_str(),tmpVal.length());
	    OutQue.push_back(tmpS);	
	    tmpS.resize(0);
	}
	return OutQue;
    }
    else
    {
	return bnetQue;
    }
    
}

TokArr DBN::GetNeighbors(TokArr nodes)
{
    TokArr nodeNeighbors, nodeChildren;
    int i;
    nodeNeighbors = GetParents(nodes);
    nodeChildren = GetChildren(nodes);
    for(i = 0; i < nodeChildren.size(); i++)
    {
	nodeNeighbors.push_back(nodeChildren[i]);
    };
    return nodeNeighbors;
}

TokArr DBN::GetParents(TokArr nodes)
{
    TokArr NewQue, tmpParents, nodesParents;
    String tmpStr;
    int i;
    
    tmpStr = nodes[nodes.size() - 1].Name();
    int nSlice = GetSliceNum(tmpStr);
    
    for(i = 0; i < nodes.size(); i++)
    {
	tmpStr = nodes[i].Name();
	if(nSlice != 0)
	{	    
	    tmpStr = GetShortName(tmpStr);
	    tmpStr<<"-1";
	    NewQue.push_back(tmpStr);
	}
	else
	{
	    NewQue.push_back(nodes[i]);
	}
    };
    
    tmpParents = Net().GetParents(NewQue);
    
    for(i = 0; i < tmpParents.size(); i++)
    {
	tmpStr = tmpParents[i].Name();
	if(nSlice != 0)
	{
	    if(GetSliceNum(tmpStr) == 1)
	    {
		tmpStr = GetShortName(tmpStr);
		tmpStr<<"-";
		char c[2];  
		itoa(nSlice,c,10);
		tmpStr.append(c,strlen(c));
		nodesParents.push_back(tmpStr);
	    }
	    else
	    {
		tmpStr = GetShortName(tmpStr);
                tmpStr<<"-";
		char c[2];  
		itoa(nSlice - 1,c,10);
		tmpStr.append(c,strlen(c));
		nodesParents.push_back(tmpStr);
	    };
	}
	else
	{
	    nodesParents.push_back(nodes[i]);
	}
    };
    
    return nodesParents;
}

TokArr DBN::GetChildren(TokArr nodes)
{
    TokArr NewQue1, NewQue2, tmpChildren1,tmpChildren2, nodesChildren;
    String tmpStr;
    Tok *pTok;
    int i;
    
    tmpStr = nodes[nodes.size() - 1].Name();
    int nSlice = GetSliceNum(tmpStr);
    
    for(i = 0; i < nodes.size(); i++)
    {
	tmpStr = nodes[i].Name();
	if(nSlice != 0)
	{	    
	    tmpStr = GetShortName(tmpStr);
	    tmpStr<<"-0";
	    const char *s = tmpStr.c_str();
	    
	    NewQue1.push_back(tmpStr);
	    
	    tmpStr.resize(tmpStr.length() - 2);
	    tmpStr<<"-0";
	    NewQue2.push_back(tmpStr);
	    
	}
	else
	{
	    NewQue2.push_back(nodes[i]);
	}
    };
    tmpChildren1 = Net().GetChildren(NewQue1);
    tmpChildren2 = Net().GetChildren(NewQue2);
    
    
    for(i = 0; i < tmpChildren1.size(); i++)
    {
	tmpStr = tmpChildren1[i].Name();
	if(GetSliceNum(tmpStr) == 1)
	{
	    tmpStr = GetShortName(tmpStr);
	    tmpStr<<"-";
	    char c[2];  
	    itoa(nSlice,c,10);
	    tmpStr.append(c,strlen(c));
	    nodesChildren.push_back(tmpStr);
	}	    
    }
    for(i = 0; i < tmpChildren2.size(); i++)
    {
	tmpStr = tmpChildren2[i].Name();
	if(nSlice != 0)
	{
	    if(GetSliceNum(tmpStr) == 1)
	    {
		tmpStr = GetShortName(tmpStr);
		tmpStr<<"-";
		char c[2];  
		itoa(nSlice + 1,c,10);
		tmpStr.append(c,strlen(c));
		nodesChildren.push_back(tmpStr);
	    }
	}
	else
	{
	    nodesChildren.push_back(tmpChildren2[i]);
	}
    }
    return nodesChildren;
}