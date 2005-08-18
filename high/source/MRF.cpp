#include "MRF.hpp"
#include "WInner.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "pnlWDistributions.hpp"
#include "pnlWGraph.hpp"
#include "WDistribFun.hpp"
#include "TokenCover.hpp"
#include "MRFCB.hpp"
#include "WCliques.hpp"

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

PNLW_BEGIN

MRF::MRF(): m_Inference(0), m_Learning(0), m_nLearnedEvidence(0)
{
    static const char *aInference[] =
    {
	"Pearl Inference", "Jtree Inference", "Gibbs Inference", "Naive Inference"
    };

    static const char *aLearning[] =
    {
	"Bayes Learning", "EM Learning"
    };

    m_pNet = new ProbabilisticNet("mrf");
    m_pNet->SetCallback(new MRFCallback());
    //m_pNet->Distributions()->SetMRF(true);
    m_pNet->Token().AddProperty("Inference", aInference,
	sizeof(aInference)/sizeof(aInference[0]));
    m_pNet->Token().AddProperty("Learning", aLearning,
	sizeof(aLearning)/sizeof(aLearning[0]));
}

MRF::~MRF()
{
    delete m_Inference;
    delete m_Learning;
    delete m_pNet;
}

void MRF::AddNode(TokArr nodes, TokArr subnodes)
{
    Net().AddNode(nodes, subnodes);
}

void MRF::DelNode(TokArr nodes)
{
    Net().DelNode(nodes);
}

// returns one of "discrete" or "continuous"
TokArr MRF::GetNodeType(TokArr nodes)
{
    return Net().GetNodeType(nodes);
}

// unite nodes in clique
void MRF::SetClique(TokArr nodes)
{
    Vector<int> aINode(nodes.size());
    for(int i = 0; i < nodes.size(); i++)
    {
	aINode[i] = Net().Graph().INode(nodes[i].Name());
    }
    
    if(!Net().Distributions().Cliques().FormClique(aINode))
    {
        ThrowUsingError("Clique already exists or nodes are subset of some clique or some clique is subset of nodes.", "SetClique");
    }
}

// destroy clique
void MRF::DestroyClique(TokArr nodes)
{
    Vector<int> aINode(nodes.size());
    for(int i = 0; i < nodes.size(); i++)
    {
	aINode[i] = Net().Graph().INode(nodes[i].Name());
    }
    
    Net().Distributions().Cliques().DestroyClique(aINode);
}

int MRF::GetNumberOfNodes() const
{
    return Net().nNetNode();
}

int MRF::GetNumberOfCliques() const
{
    return Net().Distributions().Cliques().nClique();
}

TokArr MRF::GetFullScale(TokArr aValue) const
{
    static const char fname[] = "GetFullScale";
    int aValSize = aValue.size();
    Vector<int> tokNumbers(aValSize, 1);
    int i, j;
    TokIdNode *node;
    int nVal = 1, nval, iNode;
    for(i = aValSize; --i; )
    {
        node = Net().Token().Node(aValue[i]);
        if(node->tag == eTagValue)
        {
            tokNumbers[i - 1] = tokNumbers[i];
            node = node->v_prev;
            iNode = Net().Graph().INode(node->Name());
            if(node->tag != eTagNetNode)
            {
                ThrowUsingError("There is must be node", fname);
            }
        }
        else
        {
            if(node->tag == eTagNetNode)
            {
                iNode = Net().Graph().INode(node->Name());
                nval = Net().Token().nValue(iNode);
                tokNumbers[i - 1] = tokNumbers[i] * nval;
                nVal *= nval;
            }
            else
            {
                ThrowUsingError("There is must be node", fname);
            }
        }
    }
    node = Net().Token().Node(aValue[0]);
    if(node->tag == eTagNetNode)
    {
        iNode = Net().Graph().INode(node->Name());
	nVal *= Net().Token().nValue(iNode);
    }
    if(node->tag == eTagValue)
    {
        node = node->v_prev;
        if(node->tag != eTagNetNode)
        {
            ThrowUsingError("There is must be node", fname);
        }
    }

    int TokInd;
    Vector<Tok> aTok(nVal);
    for(i = 0; i < aValSize; i++)
    {
        node = Net().Token().Node(aValue[i]);
        if(node->tag == eTagValue)
        {
            for(TokInd = 0; TokInd < nVal; TokInd++)
            {
                aTok[TokInd] ^= aValue[i];
            }
        }
        else
        {
            Vector<String> values;
            Net().Token().GetValues(Net().Graph().INode(node->Name()), values);
	    int flag = 1;
            for(TokInd = 0, j = 0; TokInd < nVal; TokInd++, flag++)
            {
                aTok[TokInd] ^= (Tok(node->Name()) ^ values[j]);
                if(flag == tokNumbers[i])
                {
                    j++;
		    flag = 0;
		    if(j == values.size())
		    {
			j = 0;
		    }
                }
            }
        }
    }

    TokArr result(&aTok.front(), nVal);
    Net().Token().SetContext(result);

    return result;

}

void MRF::SetPTabular(TokArr aValue, TokArr prob)
{
    static const char fname[] = "SetPTabular";

    TokArr aScaleValue = GetFullScale(aValue);

    int nVal = aScaleValue.size();
    Vector<int> NodeIndices = Net().Token().aiNode(aScaleValue[0]);

    if(prob.size() != nVal)
    {
        ThrowUsingError("Number of probabilities is incorrect", fname);
    }

    if(Net().Distributions().Cliques().iClique(NodeIndices) == -1)
    {
        // if clique does not exist then create it
        if(!Net().Distributions().Cliques().FormClique(NodeIndices))
        {
            ThrowUsingError("Nodes are subset of some clique or some clique is subset of nodes.", fname);
        }
    }

    int i;
    for(i = 0; i < nVal; i++)
    {
       aScaleValue[i] ^= prob[i];
    }

    Net().Distributions().FillDataNew(pnl::matTable, aScaleValue);
}
/*
void MRF::SetPGaussian(TokArr node, TokArr mean, TokArr variance, TokArr weight)
{
    Net().Distributions().FillData(node, mean, TokArr(), pnl::matMean);
    Net().Distributions().FillData(node, variance, TokArr(), pnl::matCovariance);
    if (weight.size() != 0)
        Net().Distributions().FillData(node, weight, TokArr(), pnl::matWeights);
}

TokArr MRF::GetGaussianMean(TokArr vars)
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

    const pnl::CFactor * cpd = Model().GetFactor(queryNds.front());
    const pnl::CMatrix<float> *mat = cpd->GetMatrix(pnl::matMean);

    return Net().ConvertMatrixToToken(mat);
}

TokArr MRF::GetGaussianCovar(TokArr var)
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

    const pnl::CFactor * cpd = Model().GetFactor(queryNds.front());
    const pnl::CMatrix<float> *mat = cpd->GetMatrix(pnl::matCovariance);

    return Net().ConvertMatrixToToken(mat);
}
*/
TokArr MRF::GetPTabular(TokArr aValue)
{
    static const char fname[] = "GetPTabular";

    TokArr aScaleValue = GetFullScale(aValue);

    Net().Distributions().ExtractData(pnl::matTable, aScaleValue);

    return aScaleValue;
}

void MRF::SetInferenceProperties(TokArr &nodes)
{
    pnl::CInfEngine *infEngine = &Inference();
    
    if(dynamic_cast<pnl::CGibbsSamplingInfEngine *>(infEngine) != NULL)
    {
	pnl::CGibbsSamplingInfEngine *infGibbs = static_cast<pnl::CGibbsSamplingInfEngine *>(infEngine);
	Vector<int> queryVls;
	pnl::intVecVector queries(1);
	queries[0].clear();
	
	Net().ExtractTokArr(nodes, &(queries[0]), &queryVls);
	infGibbs->SetQueries( queries );
	
	if(GetProperty("GibbsNumberOfIterations").length())
	{
	    int MaxTime = atoi(GetProperty("GibbsNumberOfIterations").c_str());
	    if(MaxTime)
	    {
		infGibbs->SetMaxTime(MaxTime);
	    }
	}
	if(GetProperty("GibbsNumberOfStreams").length())
	{
	    int NumStreams = atoi(GetProperty("GibbsNumberOfStreams").c_str());
	    if(NumStreams)
	    {
		infGibbs->SetNumStreams(NumStreams);
	    }
	}
	if(GetProperty("GibbsThresholdIteration").length())
	{
	    int BurnIn = atoi(GetProperty("GibbsThresholdIteration").c_str());
	    if(BurnIn)
	    {
		infGibbs->SetBurnIn(BurnIn);
	    }
	}
    }
    else if (dynamic_cast<pnl::CPearlInfEngine *>(infEngine) != NULL)
    {
	pnl::CPearlInfEngine *infPearl = static_cast<pnl::CPearlInfEngine *>(infEngine);
	if(GetProperty("PearlMaxNumberOfIterations").length())
	{
	    int MaxNumberOfIterations = atoi(GetProperty("PearlMaxNumberOfIterations").c_str());
	    if(MaxNumberOfIterations)
	    {
		infPearl->SetMaxNumberOfIterations(MaxNumberOfIterations);
	    }
	}
	if(GetProperty("PearlTolerance").length())
	{
	    float Tolerance = (float)atof(GetProperty("PearlTolerance").c_str());
	    if(Tolerance)
	    {
		infPearl->SetTolerance(Tolerance);
	    }
	}
    }
}

TokArr MRF::GetJPD( TokArr nodes )
{
    static const char fname[] = "GetJPD";
    
    if( !nodes.size())
    {
	ThrowInternalError("undefined query nodes", "JPD");
    }
    
    pnl::CEvidence *evid = NULL;
    if( Net().EvidenceBoard()->IsEmpty() )
    {
	evid = pnl::CEvidence::Create(Model().GetModelDomain(), 0, NULL, pnl::valueVector(0));
    }
    else
    {
	evid = Net().CreateEvidence(Net().EvidenceBoard()->Get());
    }
    
    pnl::CInfEngine *infEngine = &Inference();
    
    SetInferenceProperties(nodes);
    
    infEngine->EnterEvidence( evid );
    
    int nnodes = nodes.size();
    Vector<int> queryNds, queryVls;
    Net().ExtractTokArr(nodes, &queryNds, &queryVls);
    if(!queryVls.size())
    {
	queryVls.assign(nnodes, -1);
    }
    
    infEngine->MarginalNodes(&queryNds.front(), queryNds.size());
    
    const pnl::CPotential *pot = infEngine->GetQueryJPD();
    
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
    
    delete evid;
    Net().Token().SetContext(res);
    return res;
}

void MRF::EditEvidence(TokArr values)
{
    Net().EditEvidence(values);
}

void MRF::ClearEvid()
{
    Net().ClearEvid();
}

void MRF::CurEvidToBuf()
{
    Net().CurEvidToBuf();
}

void MRF::AddEvidToBuf(TokArr values)
{
    Net().AddEvidToBuf(values);
}

void MRF::ClearEvidBuf()
{
    Net().ClearEvidBuf();
    m_nLearnedEvidence = 0;
}

void MRF::SetParamLearningProperties()
{
    pnl::CStaticLearningEngine *learnEngine = &Learning();
    
    if(dynamic_cast<pnl::CEMLearningEngine *>(learnEngine) != NULL)
    {
	pnl::CEMLearningEngine *learnEM = static_cast<pnl::CEMLearningEngine *>(learnEngine);
	if(GetProperty("EMMaxNumberOfIterations").length())
	{
	    int nIter = atoi(GetProperty("EMMaxNumberOfIterations").c_str());
	    if(nIter)
	    {
		learnEM->SetMaxIterEM(nIter);
	    }
	}
	if(GetProperty("EMTolerance").length())
	{
	    float tolerance = (float)atof(GetProperty("EMTolerance").c_str());
	    if(tolerance)
	    {
		learnEM->SetTerminationToleranceEM(tolerance);
	    }
	}
    }
}

void MRF::LearnParameters(TokArr aSample[], int nSample)
{
    if(m_nLearnedEvidence > Net().EvidenceBuf()->size())
    {
	ThrowInternalError("inconsistent learning process", "Learn");
    }

    if(m_nLearnedEvidence == Net().EvidenceBuf()->size())
    {
	return;// is it error?
    }

    int i;
    if(nSample)
    {
	for(i = 0; i < nSample; ++i)
	{
	    AddEvidToBuf(aSample[i]);
	}
    }

    pnl::pEvidencesVector aEvidence;

    Net().TranslateBufToEvidences(&aEvidence, m_nLearnedEvidence);
    Learning().SetData(aEvidence.size(), &aEvidence.front());
    m_nLearnedEvidence = Net().EvidenceBuf()->size();

    SetParamLearningProperties();
    Learning().Learn();

    for (i = 0; i < GetNumberOfCliques(); i++)
    {
		Net().Distributions().ResetDistribution(i, *Net().Model().GetFactor(i));
	}
    DropEvidences(aEvidence);
}

TokArr MRF::GetMPE(TokArr nodes)
{
    if( !nodes.size())
    {
	ThrowInternalError("undefined query nodes", "MPE");
    }
    Net().MustBeNode(nodes);

    pnl::CEvidence *evid = NULL;
    if( Net().EvidenceBoard()->IsEmpty() )
    {
	evid = pnl::CEvidence::Create(Model().GetModelDomain(), 0, NULL, pnl::valueVector(0));
    }
    else
    {
	evid = Net().CreateEvidence(Net().EvidenceBoard()->Get());
    }


    pnl::CInfEngine *infEngine = &Inference();

    SetInferenceProperties(nodes);

    infEngine->EnterEvidence(evid, 1);

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

    infEngine->MarginalNodes(&queryNds.front(), queryNds.size());

    const pnl::CEvidence *mpe = infEngine->GetMPE();
    TokArr result;

    for(i = 0; i < nnodes; ++i)
    {
	pnl::Value v = *mpe->GetValue(queryNds[i]);

	if((v.IsDiscrete() != 0) != Net().pnlNodeType(queryNds[i]).IsDiscrete())
	{
	    ThrowInternalError("Non-discrete value for discrete variable", "MPE");
	}

        if (Net().pnlNodeType(queryNds[i]).IsDiscrete())
            result.push_back(Net().Token().TokByNodeValue(queryNds[i], v.GetInt()));
        else
    	    result.push_back(v.GetFlt());
    }

    delete evid;
    Net().Token().SetContext(result);

    return result;
}

void MRF::SaveNet(const char *filename)
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
    // BayesNet haven't any specific data for now
}

int MRF::SaveEvidBuf(const char *filename, NetConst::ESavingType mode)
{
    return Net().SaveEvidBuf(filename, mode);
}

void MRF::LoadNet(const char *filename)
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
    m_pNet->SetCallback(new MRFCallback());

    // We must load net-specific data here.
    // BayesNet haven't any specific data for now
}

int MRF::LoadEvidBuf(const char *filename, NetConst::ESavingType mode, TokArr columns)
{
    return Net().LoadEvidBuf(filename, mode, columns);
}

// whatNodes is array of tokens which specify the list of variables and optionally
// the required portion of observed nodes
// For example, if whatNodes = TokArr("Node1")
//
void MRF::GenerateEvidences( int nSample, bool ignoreCurrEvid, TokArr whatNodes )
{
    Net().GenerateEvidences(nSample, ignoreCurrEvid, whatNodes);
}

void MRF::MaskEvidBuf(TokArr whatNodes)
{
    Net().MaskEvidBuf(whatNodes);
}

//= private functions  =================================================

void MRF::DoNotify(const Message &msg)
{
    switch(msg.MessageId())
    {
    case Message::eSetModelInvalid:
	delete m_Learning;
	m_Learning = 0;
	delete m_Inference;
	m_Inference = 0;
	break;
    default:
	ThrowInternalError("Unhandled message arrive" ,"DoNotify");
	return;
    }
}

pnl::CMatrix<float> *MRF::Matrix(int iNode) const
{
    pnl::CMatrix<float> *mat = Net().Distributions().Distribution(iNode)->Matrix(pnl::matTable);

    return mat;
}

pnl::CInfEngine &MRF::Inference(bool Recreate)
{
    if (Recreate)
    {
        delete m_Inference;
        m_Inference = 0;
    };

    switch(PropertyAbbrev("Inference"))
    {
    case 'j': //Junction tree inference
    if(m_Inference)
    {
        pnl::CJtreeInfEngine *infJtree;
        infJtree = dynamic_cast<pnl::CJtreeInfEngine *>(m_Inference);
        if(!infJtree)
        {
            delete m_Inference;
            m_Inference = pnl::CJtreeInfEngine::Create(&Model());
        }
    }
    else
    {
        m_Inference = pnl::CJtreeInfEngine::Create(&Model());
    }
    break;
    case 'g': // Gibbs Sampling
    if(m_Inference)
    {
        pnl::CGibbsSamplingInfEngine *infGibbs;
        infGibbs = dynamic_cast<pnl::CGibbsSamplingInfEngine *>(m_Inference);
        if (!infGibbs)
        {
            delete m_Inference;
            m_Inference = pnl::CGibbsSamplingInfEngine::Create(&Model());
        }
    }
    else
    {
        m_Inference = pnl::CGibbsSamplingInfEngine::Create(&Model());
    }
    break;
    case 'n': // Naive inference
    if(m_Inference)
    {
        pnl::CNaiveInfEngine *infNaive;
        infNaive = dynamic_cast<pnl::CNaiveInfEngine *>(m_Inference);
        if(!infNaive)
        {
            delete m_Inference;
            m_Inference = pnl::CNaiveInfEngine::Create(&Model());
        }
    }
    else
    {
        m_Inference = pnl::CNaiveInfEngine::Create(&Model());
    }
    break;
    case 'p': // Pearl inference
    if(m_Inference)
    {
        pnl::CPearlInfEngine *infPearl;
        infPearl = dynamic_cast<pnl::CPearlInfEngine *>(m_Inference);
        if(!infPearl)
        {
            delete m_Inference;
            m_Inference = pnl::CPearlInfEngine::Create(&Model());
        }
    }
    else
    {
        m_Inference = pnl::CPearlInfEngine::Create(&Model());
    }
    break;
    default: //default inference algorithm
    if(m_Inference)
    {
        pnl::CJtreeInfEngine *infJtree;
        infJtree = dynamic_cast<pnl::CJtreeInfEngine *>(m_Inference);
        if(!infJtree)
        {
            delete m_Inference;
            m_Inference = pnl::CJtreeInfEngine::Create(&Model());
        }
    }
    else
    {
        m_Inference = pnl::CJtreeInfEngine::Create(&Model());
    }
    break;
    }
    
    return *m_Inference;
}

pnl::CStaticLearningEngine &MRF::Learning()
{
    switch(PropertyAbbrev("Learning"))
    {
    case 'b': //Bayessian learning
	if(m_Learning)
	{
	    pnl::CBayesLearningEngine *learnBayes;
	    learnBayes = dynamic_cast<pnl::CBayesLearningEngine *>(m_Learning);
	    if(!learnBayes)
	    {
		delete m_Learning;
		m_Learning = pnl::CBayesLearningEngine::Create(&Model());
	    }
	}
	else
	{
	    m_Learning = pnl::CBayesLearningEngine::Create(&Model());
	}
	break;
    case 'e': //EM learning
	if(m_Learning)
	{
	    pnl::CEMLearningEngine *learnEM;
	    learnEM = dynamic_cast<pnl::CEMLearningEngine *>(m_Learning);
	    if(!learnEM)
	    {
		delete m_Learning;
		m_Learning = pnl::CEMLearningEngine::Create(&Model());
	    }
	}
	else
	{
	    m_Learning = pnl::CEMLearningEngine::Create(&Model());
	}
	break;
    default: // deafult learning algorithm
       	if(m_Learning)
	{
	    pnl::CEMLearningEngine *learnEM;
	    learnEM = dynamic_cast<pnl::CEMLearningEngine *>(m_Learning);
	    if(!learnEM)
	    {
		delete m_Learning;
		m_Learning = pnl::CEMLearningEngine::Create(&Model());
	    }
	}
	else
	{
	    m_Learning = pnl::CEMLearningEngine::Create(&Model());
	}
	break;
    };
    return *m_Learning;
}

pnl::CMNet &MRF::Model()
{
    return static_cast<pnl::CMNet&>(Net().Model());
}

void MRF::SetProperty(const char *name, const char *value)
{
    m_pNet->SetProperty(name, value);
}

String MRF::GetProperty(const char *name) const
{
    return m_pNet->GetProperty(name);
}

const char MRF::PropertyAbbrev(const char *name) const
{
    if(!strcmp(name,"Inference"))
    {
	String infName = GetProperty("Inference");

	if(!infName.length())
	{
	    return 0;// default value
	}

	pnl::pnlVector<char> infNameVec(infName.length());
	for(int i = 0; i < infName.length(); ++i)
	{
	    infNameVec[i] = tolower(infName[i]);
	}
	char *pInfName = &infNameVec.front();

	if(strstr(pInfName, "gibbs"))
	{
	    return 'g';
	}
	if(strstr(pInfName, "pearl"))
	{
	    return 'p';
	}
	if(strstr(pInfName, "jtree"))
	{
	    return 'j';
	}
	if(strstr(pInfName, "naive"))
	{
	    return 'n';
	}
	else
	{
	    return 0;
	}
    }
    if(!strcmp(name,"Learning"))
    {
	String learnName = GetProperty("Learning");

	if(!learnName.length())
	{
	    return 0;// default value
	}

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
	if(strstr(pLearnName, "bayes"))
	{
	    return 'b';
	}
	else
	{
	    return 0;
	}
    }

    return 0;
}

pnl::CEvidence *MRF::GetPNLEvidence()
{
    pnl::CEvidence *evid = NULL;
    if( Net().EvidenceBoard()->IsEmpty() )
    {
	evid = pnl::CEvidence::Create(Model().GetModelDomain(), 0, NULL, pnl::valueVector(0));
    }
    else
    {
	evid = Net().CreateEvidence(Net().EvidenceBoard()->Get());
    }

    return evid;
}

PNLW_END
