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
	// m_pNet->Token()->AddProperty("Learning", aLearning,
	//	sizeof(aLearning)/sizeof(aLearning[0]));
}

DBN::~DBN()
{
    delete m_Inference;
    delete m_Learning;
    delete m_pNet;
}

void DBN::AddNode(TokArr nodes, TokArr subnodes)
{
/*  unsigned int i;
TokArr newBotNodes;
String tmpName;
Tok *newTok;
Net().DelNode(bottomNode);
for(i = 0; i < nodes.size(); ++i)
{
tmpName = nodes[i].Name(); 
//	tmpName[tmpName.length() - 1] = '1';

  std::string mystr(tmpName.c_str());
  mystr[mystr.length() - 1] = '1';
  tmpName = mystr;
  
	newTok = new Tok(tmpName);
	newBotNodes.push_back(*newTok);
	newTok = new Tok(tmpName);
	bottomNode.push_back(*newTok);
    }
    for(i = 0; i < newBotNodes.size(); ++i)
    {
	bottomSubnodes.push_back(newBotNodes[i]);
}*/
    Net().AddNode(nodes, subnodes);
	/* for(i = 0; i < newBotNodes.size(); i++)
    {
	TokArr tmpNode,tmpSubN;
	tmpNode.push_back(newBotNodes[i]);
	tmpSubN.push_back(bottomSubnodes[bottomSubnodes.size()-newBotNodes.size()+i]);
	Net().AddNode(tmpNode, tmpSubN);
    };
    for(i = 0; i < bottomEdgesFr.size(); i++)
    {
	TokArr tmpFr,tmpTo;
	tmpFr.push_back(bottomEdgesFr[i]);
	tmpTo.push_back(bottomEdgesTo[i]);
	Net().AddArc(tmpFr,tmpTo); 
}*/
}

void DBN::DelNode(TokArr nodes)
{
    Net().DelNode(nodes);
}

/*
// returns one of "categoric" or "continuous"
TokArr DBN::NodeType(TokArr nodes)
{
    return Net().NodeType(nodes);
}
*/

// manipulating arcs
void DBN::AddArc(TokArr from, TokArr to)
{
/* TokArr edgeFr,edgeTo;
unsigned int i,j;
String tmpStr;
Tok *pTok;
for(i = 0; i < from.size(); i++)
{
for(j = 0; j < to.size(); j++)
{
tmpStr = to[j].Name();
if(tmpStr[tmpStr.length()-1] == '0')
{
tmpStr = from[i].Name();

  std::string mystr(tmpStr.c_str());
  mystr[mystr.length() - 1] = '1';
  tmpStr = mystr;
		//tmpStr[tmpStr.length()-1] = '1';
		
		  pTok = new Tok(tmpStr);
		  bottomEdgesFr.push_back(*pTok);
		  }
		  else
		  {
		  pTok = new Tok(from[i].Name());
		  bottomEdgesFr.push_back(*pTok);
		  };
		  tmpStr = to[j].Name();
		  
			std::string mystr(tmpStr.c_str());
			mystr[mystr.length() - 1] = '1';
			tmpStr = mystr;
			// tmpStr[tmpStr.length()-1] = '1';
			
			  pTok = new Tok(tmpStr);
			  bottomEdgesTo.push_back(*pTok);
			  edgeFr.push_back(from[i]);
			  edgeTo.push_back(to[j]);
			  }
}*/
    Net().AddArc(from, to);
	/*  for(i = 0; i < edgeFr.size(); i++)
    {
	tmpStr = edgeFr[i].Name();
	if (tmpStr[tmpStr.length() - 1] != '0')
	{
	TokArr tmpFr,tmpTo;
	tmpFr.push_back(edgeFr[i]);
	tmpTo.push_back(edgeTo[i]);
	Net().AddArc(tmpFr,tmpTo); 
	}
};*/
    
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
void DBN::SetP(TokArr value, TokArr prob, TokArr parentValue)
{
    Net().Distributions()->FillData(value, prob, parentValue);
}

void DBN::SetGaussian(TokArr var, TokArr mean, TokArr variance, TokArr weight)
{
    Net().Distributions()->FillData(var, mean, TokArr(), pnl::matMean);
    Net().Distributions()->FillData(var, variance, TokArr(), pnl::matCovariance);
    if (weight.size() != 0)
        Net().Distributions()->FillData(var, weight, TokArr(), pnl::matWeights);
}

TokArr DBN::GaussianMean(TokArr vars)
{
    static const char fname[] = "GaussianMean";
    
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
    const pnl::CMatrix<float> *mat = cpd->GetMatrix(pnl::matMean);
    
    return Net().ConvertMatrixToToken(mat);
}

TokArr DBN::GaussianCovar(TokArr var, TokArr vars)
{
    static const char fname[] = "GaussianCovar";
	
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
    const pnl::CMatrix<float> *mat = cpd->GetMatrix(pnl::matCovariance);
	
    return Net().ConvertMatrixToToken(mat);
}

TokArr DBN::P(TokArr child, TokArr parents)
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

TokArr DBN::JPD( TokArr nodes)
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
	pEvid = new pnl::CEvidence*[m_nSlices];
	int i;
	for(i = 0; i < m_nSlices; i++)
	{
		pEvid[i] = (m_AllEvidences[i])[m_AllEvidences[i].size() - 1]; 
	}
	switch(PropertyAbbrev("Inference"))
	{
	case 's': 
		Inference().DefineProcedure(pnl::ptSmoothing, m_nSlices);
		Inference().EnterEvidence(pEvid,m_nSlices);
		Inference().Smoothing();
		break;
	case 'x':
		Inference().DefineProcedure(pnl::ptFixLagSmoothing,0 );
		int slice;
		// for (slice = 0; slice < nSlice + 1; slice++)
		//	{
		Inference().EnterEvidence( &(pEvid[nSlice]), 1 );
		//	};
		Inference().FixLagSmoothing( nSlice );
		break;
	case 'f':
		Inference().DefineProcedure(pnl::ptFiltering,0 );
		Inference().EnterEvidence( &(pEvid[nSlice]), 1 );
		Inference().Filtering( nSlice );
		break;
	case 'v':break;
	default:break;
	};
	
	for(i = 0; i < nodes.size(); i++)
	{
		tmpStr = nodes[i].Name();
		if(nSlice != 0)
		{
			if(GetSliceNum(tmpStr) == nSlice )
			{
				tmpStr = GetShortName(tmpStr);
				tmpStr.append("-1",2);
				pTok = new Tok(tmpStr);
				NewQue.push_back(*pTok);
			}
			else
			{
				tmpStr = GetShortName(tmpStr);
				tmpStr.append("-0",2);
				pTok = new Tok(tmpStr);
				NewQue.push_back(*pTok);
			};
		}
		else
		{
			NewQue.push_back(nodes[i]);
		}
	};
	//
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
	if (pot->GetDistribFun()->GetDistributionType() == pnl::dtScalar)
	{
		res<<"Scalar";
	}
	else
		if (pot->GetDistribFun()->GetDistributionType() == pnl::dtTabular)
		{
			const pnl::CMatrix<float> *mat = pot->GetMatrix(pnl::matTable);
			res << Net().CutReq( queryNds, queryVls, mat);
		}
		else
		{
			const pnl::CMatrix<float> *mean = pot->GetMatrix(pnl::matMean);
			res << Net().ConvertMatrixToToken(mean);
			const pnl::CMatrix<float> *cov = pot->GetMatrix(pnl::matCovariance);
			res << Net().ConvertMatrixToToken(cov);
		}
		return res;
}

void DBN::EditEvidence(TokArr values)
{
	TokArr NewQue;
	String tmpStr,tmpName,tmp;
	TokIdNode *node;
	Tok *pTok;  
	int i;
	int nSlice = GetSliceNum(GetNodeName(values[0]));
	for(i = 0; i < values.size(); i++)
	{ 	
		tmpName = GetNodeName(values[i]);
		if(GetSliceNum(tmpName) == nSlice)
		{
			tmpStr = GetShortName(tmpName);
			tmpStr.append("-0",2);
			tmpStr.append("^",1);
			tmpStr.append(values[i].Name().c_str(),values[i].Name().length());
			pTok = new Tok(tmpStr);
			NewQue.push_back(*pTok);
		}
		else
		{
			tmpStr = GetShortName(tmpName);
			tmpStr.append("-1",2);
			tmpStr.append("^",1);
			tmpStr.append(values[i].Name().c_str(),values[i].Name().length());
			const char *s = tmpStr.c_str();
			pTok = new Tok(tmpStr);
			NewQue.push_back(*pTok);
		};
	};
	
	Net().EditEvidence(NewQue);
	
	pnl::CEvidence *evid = NULL; 
	evid = Net().CreateEvidence(Net().EvidenceBoard()->GetBoard());
	(m_AllEvidences[nSlice]).push_back(evid);
	
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

void DBN::Learn()
{
    Learn(0, 0);
}

void DBN::Learn(TokArr aSample[], int nSample)
{   
/*if(m_nLearnedEvidence > Net().EvidenceBuf()->size())
{
ThrowInternalError("inconsistent learning process", "Learn");
}

  if(m_nLearnedEvidence == Net().EvidenceBuf()->size())
  {
  return;// is it error?
  }
  
    if(nSample)
    {
	for(int i = 0; i < nSample; ++i)
	{
	Net().EvidenceBuf()->push_back(Net().CreateEvidence(aSample[i]));
	}
	
	  
}*/
	
	if(nSample)
	{
		for(int i = 0; i < nSample; ++i)
		{
			EditEvidence(aSample[i]);
		}
	}
	
	Learning().SetData(static_cast<const pnl::pEvidencesVecVector>(m_AllEvidences));
	Learning().Learn();
}



#if 0        
DBN* DBN::LearnStructure(TokArr aSample[], int nSample)
{
    intVector vAnc, vDesc;//bogus vectors
    CMlStaticStructLearnHC* pLearning = pnl::CMlStaticStructLearnHC::Create(m_Model,
        itStructLearnML, StructLearnHC, BIC, m_Model->GetNumberOfNodes(), vAnc, vDesc, 1/*one restart*/ );
	
    if(nSample)
    {
		for(int i = 0; i < nSample; ++i)
		{
			Net().EvidenceBuf()->push_back(Net().CreateEvidence(aSample[i]));
		}
    }
	
    pLearning->SetData(Net().EvidenceBuf()->size(), &Net().EvidenceBuf()->front() );
    pLearning->Learn();
    const int* pRenaming = pLearning->GetResultRenaming();
    pLearning->CreateResultBNet(const_cast<CDAG*>(pLearning->GetResultDAG()));
    
    CBNet* newNet = CBNet::Copy(pLearning->GetResultBNet());
	
    //now we will create new object of class DBN and construct it from scratch using newNet
    //and required information (node names) on current (this) network
    DBN* newDBN = new DBN;
	
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
        
        newDBN->AddNode( nodeType^nodeName, values );
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
	
    //newDBN->AddArc(
	
    return newDBN;
}
#else
void DBN::LearnStructure(TokArr aSample[], int nSample)
{
	//    pnl::intVector vAnc, vDesc;//bogus vectors
	//    pnl::CMlDynamicStructLearn* pLearning = pnl::CMlDynamicStructLearn::Create(Model(),
	//    pnl::itStructLearnML, pnl::StructLearnHC, pnl::BIC, Model()->GetNumberOfNodes(), vAnc, vDesc, 1/*one restart*/ );	
	/*    if(nSample)
    {
	for(int i = 0; i < nSample; ++i)
	{
	Net().EvidenceBuf()->push_back(Net().CreateEvidence(aSample[i]));
	}
    }
	
	  pLearning->SetData(Net().EvidenceBuf()->size(), &Net().EvidenceBuf()->front() );
	  pLearning->Learn();
	  
		const int* pRenaming = pLearning->GetResultRenaming();
		Vector<int> vRename(pRenaming, pRenaming + Model()->GetNumberOfNodes());
		
		  pRenaming = &vRename.front();
		  pLearning->CreateResultBNet(const_cast<pnl::CDAG*>(pLearning->GetResultDAG()));
		  
			pnl::CBNet* newNet = pnl::CBNet::Copy(pLearning->GetResultBNet());
			
			  int i;
			  
				Net().SetTopologicalOrder(pRenaming, newNet->GetGraph());
				
				  for(i = 0; i < Net().Graph()->nNode(); ++i)
				  {
				  Net().Distributions()->ResetDistribution(i, *newNet->GetFactor(i));
				  }
				  
					//change ordering in current stuff
					// Note! it may happen that old ordering of nodes is consistent (i.e. is topological)
					// with new graph structure so theoretically no reordering required
					//this would be good to have such function in PNL that checks this situation and reorder
					//new network to old ordering. So we would not have to do all below
					
					  //add nodes 
					  int nnodes = newNet->GetNumberOfNodes();
					  //    Net().Token()->RenameGraph(pRenaming);
					  
						//reassign model
						Net().SetModel(0);
						
						  //clear learning engine
						  delete m_Learning;
						  m_Learning = 0;
						  
							//clear inference engine
							delete m_Inference;
							m_Inference = 0;
							
							  //change domain in evidences
							  
								//change evidence on board
								//IT is in Token form, so do not need to change
								
								  //change evidences in evidence buffer
								  for( Vector<pnl::CEvidence*>::iterator it1 = Net().EvidenceBuf()->begin(); it1 != Net().EvidenceBuf()->end(); it1++ )
								  {
								  pnl::CEvidence* oldEv = *it1;
								  pnl::CNodeValues* nv = oldEv;
								  
									
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
	/*   }*/
}
#endif

TokArr DBN::MPE(TokArr nodes)
{
    TokArr NewQue;
    String tmpStr;
    Tok *pTok;
    tmpStr = nodes[nodes.size() - 1].Name();
	int nSlice = GetSliceNum(tmpStr);
    if( !nodes.size())
    {
		ThrowInternalError("undefined query nodes", "MPE");
    }
	//  Net().MustBeNode(nodes);	
    pnl::CEvidence **pEvid;
    pEvid = new pnl::CEvidence*[m_nSlices];
    int i;
    for(i = 0; i < m_nSlices; i++)
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
				tmpStr.append("-1",2);
				pTok = new Tok(tmpStr);
				NewQue.push_back(*pTok);
			}
			else
			{
				tmpStr = GetShortName(tmpStr);
				tmpStr.append("-0",2);
				pTok = new Tok(tmpStr);
				NewQue.push_back(*pTok); 
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
		
		result.push_back(Net().Token()->TokByNodeValue(queryNds[i], v.GetInt()));
		//	result.push_back(Net().Token()->TokByNodeValue(queryNds[i], v.GetFlt()));
    }
	
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
    return Net().SaveEvidBuf(filename, mode);
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
	//!!!!!!!!!!!!!!!1    m_pNet->SetCallback(new DBNCallback());
	
    // We must load net-specific data here.
    // DBN haven't any specific data for now
}

int DBN::LoadEvidBuf(const char *filename, NetConst::ESavingType mode, TokArr columns)
{
    return Net().LoadEvidBuf(filename, mode, columns);
}

// whatNodes is array of tokens which specify the list of variables and optionally 
// the required portion of observed nodes
// For example, if whatNodes = TokArr("Node1")
//    
void DBN::GenerateEvidences(pnl::intVector nSlices)
{
	Model()->GenerateSamples(&m_AllEvidences,nSlices);
}

void DBN::MaskEvidBuf(TokArr whatNodes)
{   
    Net().MaskEvidBuf(whatNodes);
}

//= private functions  =================================================


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
	m_nSlices = nSlices;
	m_AllEvidences.resize(m_nSlices);
}

const char DBN::PropertyAbbrev(const char *name) const
{   
    if(!strcmp(name,"Inference"))
    {
		String infName = GetProperty("Inference");
		
		if(infName.length() == 0)
		{
			return 'f';
		}
		pnl::pnlVector<char> infNameVec(infName.length());
		for(int i = 0; i < infName.length(); ++i)
		{
			infNameVec[i] = tolower(infName[i]);
		}
		char *pInfName = &infNameVec.front();
		
		if(strstr(pInfName, "smoth"))
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
    if(!strcmp(name,"Learning"))
    {
		String learnName = GetProperty("Learning");
		pnl::pnlVector<char> learnNameVec(learnName.length());
		for(int i = 0; i < learnName.length(); ++i)
		{
			learnNameVec[i] = tolower(learnName[i]);
		}
		char *pLearnName = &learnNameVec.front();
		
		if(strstr(pLearnName, "em"))
		{
			return 'e';
		}
		else 
		{
			return 0;
		}   	
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
