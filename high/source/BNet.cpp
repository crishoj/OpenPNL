#include "BNet.hpp"
#include "WInner.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "pnlWDistributions.hpp"
#include "pnlWGraph.hpp"
#include "WDistribFun.hpp"
#include "TokenCover.hpp"
#include "BNetCB.hpp"

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

BayesNet::BayesNet(): m_Inference(0), m_Learning(0), m_nLearnedEvidence(0)
{
    static const char *aInference[] = 
    {
	"Pearl Inference"
    };

    static const char *aLearning[] = 
    {
	"BayesNet Learning"
    };

    m_pNet = new ProbabilisticNet();
    m_pNet->SetCallback(new BayesNetCallback());
    m_pNet->Token()->AddProperty("Inference", aInference,
	sizeof(aInference)/sizeof(aInference[0]));
    m_pNet->Token()->AddProperty("Learning", aLearning,
	sizeof(aLearning)/sizeof(aLearning[0]));
}

BayesNet::~BayesNet()
{
    delete m_Inference;
    delete m_Learning;
    delete m_pNet;
}

void BayesNet::AddNode(TokArr nodes, TokArr subnodes)
{
    Net().AddNode(nodes, subnodes);
}

void BayesNet::DelNode(TokArr nodes)
{
    Net().DelNode(nodes);
}

// returns one of "categoric" or "continuous"
TokArr BayesNet::NodeType(TokArr nodes)
{
    return Net().NodeType(nodes);
}

// manipulating arcs
void BayesNet::AddArc(TokArr from, TokArr to)
{
    Net().AddArc(from, to);
}

void BayesNet::DelArc(TokArr from, TokArr to)
{
    Net().AddArc(from, to);
}

static int cmpTokIdNode(TokIdNode *node1, TokIdNode *node2)
{
    return (char*)node1->data - (char*)node2->data;
}

// It is inner DistribFun
void BayesNet::SetP(TokArr value, TokArr prob, TokArr parentValue)
{
    Net().Distributions()->FillData(value, prob, parentValue);
}

void BayesNet::SetGaussian(TokArr var, TokArr mean, TokArr variance)
{

}

TokArr BayesNet::P(TokArr child, TokArr parents)
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

TokArr BayesNet::JPD( TokArr nodes )
{
    static const char fname[] = "JPD";

    if( !nodes.size())
    {
	ThrowInternalError("undefined query nodes", "JPD");
    }

    pnl::CEvidence *evid = NULL;
    if( Net().EvidenceBoard()->IsEmpty() )
    {
	evid = pnl::CEvidence::Create(Model()->GetModelDomain(), 0, NULL, pnl::valueVector(0));
    }
    else
    {
	evid = Net().CreateEvidence(Net().EvidenceBoard()->GetBoard());
    }
    
    Inference().EnterEvidence( evid );
    
    int nnodes = nodes.size();
    Vector<int> queryNds, queryVls;
    Net().ExtractTokArr(nodes, &queryNds, &queryVls);
    if(!queryVls.size())
    {
	queryVls.assign(nnodes, -1);
    }
    
    Inference().MarginalNodes(&queryNds.front(), queryNds.size());

    const pnl::CPotential *pot = Inference().GetQueryJPD();
    const pnl::CMatrix<float> * mat = pot->GetMatrix(pnl::matTable);
    
    return Net().CutReq( queryNds, queryVls, mat);
}

void BayesNet::Evid(TokArr values, bool bPush)
{
    Net().Evid(values, bPush);
}

void BayesNet::PushEvid(TokArr const values[], int nValue)
{
    Net().PushEvid(values, nValue);
}

void BayesNet::ClearEvid()
{
    Net().ClearEvid();
}

void BayesNet::ClearEvidHistory()
{
    Net().ClearEvidHistory();
    m_nLearnedEvidence = 0;
}

void BayesNet::Learn()
{
    Learn(0, 0);
}

void BayesNet::Learn(TokArr aSample[], int nSample)
{
    bool bFirst = false;

    if(m_nLearnedEvidence > Net().EvidenceBuf()->size())
    {
	ThrowInternalError("inconsistent learning process", "Learn");
    }

    if(m_nLearnedEvidence == Net().EvidenceBuf()->size())
    {
	return;// is it error?
    }

    if(!m_Learning)
    {
	m_Learning = pnl::CBayesLearningEngine::Create(Model());
	bFirst = true;
    }

    if(nSample)
    {
	for(int i = 0; i < nSample; ++i)
	{
	    Net().EvidenceBuf()->push_back(Net().CreateEvidence(aSample[i]));
	}
    }

    m_Learning->AppendData(Net().EvidenceBuf()->size() - m_nLearnedEvidence,
	&(*Net().EvidenceBuf())[m_nLearnedEvidence]);
    m_nLearnedEvidence = Net().EvidenceBuf()->size();
    m_Learning->Learn();
}

#if 0        
BayesNet* BayesNet::LearnStructure(TokArr aSample[], int nSample)
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
}
#else
void BayesNet::LearnStructure(TokArr aSample[], int nSample)
{
    pnl::intVector vAnc, vDesc;//bogus vectors
    pnl::CMlStaticStructLearnHC* pLearning = pnl::CMlStaticStructLearnHC::Create(Model(),
        pnl::itStructLearnML, pnl::StructLearnHC, pnl::BIC, Model()->GetNumberOfNodes(), vAnc, vDesc, 1/*one restart*/ );
	
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
    }
}
#endif

TokArr BayesNet::MPE(TokArr nodes)
{
    if( !nodes.size())
    {
	ThrowInternalError("undefined query nodes", "MPE");
    }
    Net().MustBeNode(nodes);

    pnl::CEvidence *evid = NULL;
    if( Net().EvidenceBoard()->IsEmpty() )
    {
	evid = pnl::CEvidence::Create(Model()->GetModelDomain(), 0, NULL, pnl::valueVector(0));
    }
    else
    {
	evid = Net().CreateEvidence(Net().EvidenceBoard()->GetBoard());
    }
    
    Inference().EnterEvidence(evid, 1);
    
    int nnodes = nodes.size();
    int i;
    Vector<int> queryNds;
    if( nnodes )
    {
	queryNds.resize(nnodes);
	for(i = 0; i < nnodes; ++i)
	{
	    Tok tok = nodes[i];
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

#if 1
        result.push_back(Tok(Net().Graph()->NodeName(queryNds[i]))
            ^ Net().DiscreteValue(queryNds[i], v.GetInt()));
#else
	TokIdNode *node = Net().TokNodeByIndex(queryNds[i]);
	if(Net().pnlNodeType(queryNds[i]).IsDiscrete())
	{

	    node = node->v_next;
	    //may be we should search by id?
	    for(; node; node = node->h_next)
	    {
		if(!node || node->tag != eTagValue)
		{
		    ThrowInternalError("wrong value for node", "MPE");
		}
	    }

	    result.push_back(Tok(node));
	}
#endif
    }

    return result;
}

void BayesNet::SaveNet(const char *filename)
{
    pnl::CContextPersistence saver;

    saver.Put(Model(), "Model");
    saver.Put(this, "BayesNet");
    if(!saver.SaveAsXML(filename))
    {
	ThrowInternalError("Can't save file", "SaveNet");
    }
}

int BayesNet::SaveLearnBuf(const char *filename, NetConst::ESavingType mode)
{
    return Net().SaveLearnBuf(filename, mode);
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

    //delete bnet;
}

int BayesNet::LoadLearnBuf(const char *filename, NetConst::ESavingType mode, TokArr columns)
{
    return Net().LoadLearnBuf(filename, mode, columns);
}

// whatNodes is array of tokens which specify the list of variables and optionally 
// the required portion of observed nodes
// For example, if whatNodes = TokArr("Node1")
//    
void BayesNet::GenerateEvidences( int nSample, bool ignoreCurrEvid, TokArr whatNodes )
{
    Net().GenerateEvidences(nSample, ignoreCurrEvid, whatNodes);
}

void BayesNet::MaskEvidences(TokArr whatNodes)
{   
    Net().MaskEvidences(whatNodes);
}

//= private functions  =================================================


pnl::CMatrix<float> *BayesNet::Matrix(int iNode) const
{
    pnl::CMatrix<float> *mat = Net().Distributions()->Distribution(iNode)->Matrix(pnl::matTable);

    return mat;
}

pnl::CInfEngine &BayesNet::Inference()
{
    if(!m_Inference)
    {
	m_Inference = pnl::CPearlInfEngine::Create(Model());
    }

    return *m_Inference;
}

pnl::CBNet *BayesNet::Model()
{
    return static_cast<pnl::CBNet*>(Net().Model());
}
