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
#include "pnlGroup.hpp"
#pragma warning(default: 4251)
#pragma warning(pop)


#if defined(_MSC_VER)
#pragma warning(disable : 4239) // nonstandard extension used: 'T' to 'T&'
#endif

PNLW_BEGIN

BayesNet::BayesNet(): m_Inference(0), m_Learning(0), m_nLearnedEvidence(0)
{
    static const char *aInference[] =
    {
	"Pearl Inference", "Jtree Inference", "Gibbs Inference", "Naive Inference"
    };

    static const char *aLearning[] =
    {
	"Bayes Learning", "EM Learning"
    };

	static const char *aLearningStructMethod[] =
    {
	"Maximized likelihood", "Predictive assessment", "Marginal likelyhood"
    };

	static const char *aLearningStructScoreFun[] =
    {
	"BIC", "AIC", "Without penalty"
    };

	static const char *aLearningStructPriorType[] =
    {
	"Dirichlet", "K2", "BDeu"
    };

	static const char *aLearningStructK2PriorVal[] =
    {
	"0"
    };

    m_pNet = new ProbabilisticNet();
    SpyTo(m_pNet);
    m_pNet->SetCallback(new BayesNetCallback());
    m_pNet->Token().AddProperty("Inference", aInference,
	sizeof(aInference)/sizeof(aInference[0]));
	m_pNet->Token().AddProperty("Learning", aLearning,
	sizeof(aLearning)/sizeof(aLearning[0]));
    m_pNet->Token().AddProperty("LearningStructureMethod", aLearning,
	sizeof(aLearningStructMethod)/sizeof(aLearningStructMethod[0]));
	m_pNet->Token().AddProperty("LearningStructureScoreFun", aLearning,
	sizeof(aLearningStructScoreFun)/sizeof(aLearningStructScoreFun[0]));
	m_pNet->Token().AddProperty("LearningStructurePrior", aLearning,
	sizeof(aLearningStructPriorType)/sizeof(aLearningStructPriorType[0]));

	m_pNet->Token().AddProperty("LearningStructureK2PriorVal", aLearning,
	sizeof(aLearningStructK2PriorVal)/sizeof(aLearningStructK2PriorVal[0]));

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

// returns one of "discrete" or "continuous"
TokArr BayesNet::GetNodeType(TokArr nodes)
{
    return Net().GetNodeType(nodes);
}

// manipulating arcs
void BayesNet::AddArc(TokArr from, TokArr to)
{
    Net().AddArc(from, to);
}

void BayesNet::DelArc(TokArr from, TokArr to)
{
    Net().DelArc(from, to);
}

TokArr BayesNet::GetNeighbors(TokArr nodes)
{
    return Net().GetNeighbors(nodes);
}

TokArr BayesNet::GetParents(TokArr nodes)
{
    return Net().GetParents(nodes);
}

TokArr BayesNet::GetChildren(TokArr nodes)
{
    return Net().GetChildren(nodes);
}

// It is inner DistribFun
void BayesNet::SetPTabular(TokArr value, TokArr prob, TokArr parentValue)
{
    Net().Distributions().FillData(value, prob, parentValue);
}

void BayesNet::SetPGaussian(TokArr node, TokArr mean, TokArr variance, TokArr weight, TokArr tabParentValue)
{
    Net().Distributions().FillData(node, mean, tabParentValue, pnl::matMean);
    Net().Distributions().FillData(node, variance, tabParentValue, pnl::matCovariance);
    if (weight.size() != 0)
    {
	Net().Distributions().FillData(node, weight, tabParentValue, pnl::matWeights);
    }
}

void BayesNet::SetPSoftMax(TokArr node, TokArr weigth, TokArr offset, TokArr parentValue)
{
    const int par[] = {0}; 
    
    if (parentValue.size() == 0)
    {
        int size = weigth.size();
        Net().Distributions().FillData(node, weigth, TokArr(), pnl::matWeights);
        Net().Distributions().FillData(node, offset, TokArr(), vectorOffset);

    }
    else
    {
        Net().Distributions().FillData(node, weigth, parentValue, pnl::matWeights);
        Net().Distributions().FillData(node, offset, parentValue, vectorOffset);
    }
}


TokArr BayesNet::GetSoftMaxOffset(TokArr node, TokArr parents)
{
    static const char fname[] = "GetSoftMaxOffset";
    TokArr res;
    int nnodes = node.size();
    if(nnodes != 1)
    {
	ThrowUsingError("Offset may be got only for one node", fname);
    }
    Vector<int> queryNdsOuter, queryVls;
    Vector<int> queryNdsInner;
    Net().ExtractTokArr(node, &queryNdsInner, &queryVls, &Net().Graph().MapOuterToGraph());
    Net().Graph().IOuter(&queryNdsInner, &queryNdsOuter);
    if(!queryVls.size())
    {
	queryVls.assign(nnodes, -1);
    }

    const pnl::CFactor * cpd = Model().GetFactor(queryNdsInner.front());
    pnl::CDistribFun *df = cpd->GetDistribFun();

    pnl::floatVector *offVector;

    if (parents.size())
    {
        Vector<int> discrParentValuesIndices = Net().Distributions().Distribution(
            queryNdsOuter[0])->GetDiscreteParentValuesIndexes(parents);
        int *discrPar = new int[discrParentValuesIndices.size()];
        memcpy(discrPar, &(discrParentValuesIndices.front()), 
            sizeof(int)*discrParentValuesIndices.size());
        offVector = 
            dynamic_cast<pnl::CCondSoftMaxDistribFun *>(df)->GetOffsetVector(discrPar);
    }
    else
    {
        offVector = dynamic_cast<pnl::CSoftMaxDistribFun *>(df)->GetOffsetVector();
    }
    std::vector<float> vec(offVector->begin(), offVector->end());
    
    res = Tok(vec);

    return res;
}


TokArr BayesNet::GetSoftMaxWeights(TokArr node, TokArr parents )
{
    static const char fname[] = "GetSoftMaxWeights";
    TokArr res;
    int nnodes = node.size();
    if(nnodes != 1)
    {
	ThrowUsingError("Weights matrix may be got only for one node", fname);
    }
    Vector<int> queryNdsOuter, queryVls;
    Vector<int> queryNdsInner;
    Net().ExtractTokArr(node, &queryNdsInner, &queryVls, &Net().Graph().MapOuterToGraph());
    Net().Graph().IOuter(&queryNdsInner, &queryNdsOuter);
    if(!queryVls.size())
    {
	queryVls.assign(nnodes, -1);
    }

    const pnl::CFactor * cpd = Model().GetFactor(queryNdsInner.front());
    const pnl::CMatrix<float> *mat;

    if (parents.size())
    {
        Vector<int> discrParentValuesIndices = Net().Distributions().Distribution(
            queryNdsOuter[0])->GetDiscreteParentValuesIndexes(parents);
        int *discrPar = new int[discrParentValuesIndices.size()];
        memcpy(discrPar, &(discrParentValuesIndices.front()), 
            sizeof(int)*discrParentValuesIndices.size());
        mat = cpd->GetMatrix(pnl::matWeights, 0, discrPar);
    }
    else
    {
            mat = cpd->GetMatrix(pnl::matWeights);
    }

    return Net().ConvertMatrixToToken(mat);
}

TokArr BayesNet::GetGaussianMean(TokArr node, TokArr tabParentValue)
{
  static const char fname[] = "GetGaussianMean";
  
  int nnodes = node.size();
  if(nnodes != 1)
  {
    ThrowUsingError("Mean may be got only for one node", fname);
  }
  
  Vector<int> queryNdsOuter, queryVls;
  Vector<int> queryNdsInner;
  Net().ExtractTokArr(node, &queryNdsInner, &queryVls, &Net().Graph().MapOuterToGraph());
  Net().Graph().IOuter(&queryNdsInner, &queryNdsOuter);
  if(!queryVls.size())
  {
    queryVls.assign(nnodes, -1);
  }
  
  const pnl::CFactor * cpd = Model().GetFactor(queryNdsInner.front());
  
  if (cpd->IsDistributionSpecific() == 1)
  {
    TokArr res;
    res << "uniform";
    return res;
  }
  else
  {
    if (cpd->GetDistribFun()->GetDistributionType() == pnl::dtScalar)
    {
      TokArr res;
      res << "scalar";
      return res;  
    }
    else
    {
      const pnl::CMatrix<float> *mat;
      if (tabParentValue.size())
      {
        Vector<int> discrParentValuesIndexes = Net().Distributions().Distribution(
          queryNdsOuter[0])->GetDiscreteParentValuesIndexes(tabParentValue);
        mat = cpd->GetMatrix(pnl::matMean, -1, 
          &(discrParentValuesIndexes.front()));
      }
      else
      {
        mat = cpd->GetMatrix(pnl::matMean);
      };
      
      return Net().ConvertMatrixToToken(mat);
    }
  }
}

TokArr BayesNet::GetGaussianCovar(TokArr node, TokArr tabParentValue)
{
    static const char fname[] = "GetGaussianCovar";

    int nnodes = node.size();
    if(nnodes != 1)
    {
	ThrowUsingError("Variance may be got only for one node", fname);
    }

    Vector<int> queryNdsOuter;
    Vector<int> queryNdsInner, queryVls;
    Net().ExtractTokArr(node, &queryNdsInner, &queryVls, &Net().Graph().MapOuterToGraph());
    Net().Graph().IOuter(&queryNdsInner, &queryNdsOuter);
    if(!queryVls.size())
    {
	queryVls.assign(nnodes, -1);
    }

    const pnl::CFactor * cpd = Model().GetFactor(queryNdsInner.front());

    if (cpd->GetDistribFun()->IsDistributionSpecific() == 2) // delta
    {
        TokArr res;
        res << "0";
        return res;
    }
    else
    {
        if (cpd->GetDistribFun()->IsDistributionSpecific() == 1)
        {
            TokArr res;
            res << "uniform";
            return res;
        }
        else
	{
            if (cpd->GetDistribFun()->GetDistributionType() == pnl::dtScalar)
            {
                TokArr res;
                res << "scalar";
                return res;
            }
            else
            {
                const pnl::CMatrix<float> *mat = 0;
                
                if (tabParentValue.size())
                {
                    Vector<int> discrParentValuesIndexes = Net().Distributions().Distribution(
                      queryNdsOuter[0])->GetDiscreteParentValuesIndexes(tabParentValue);
                    mat = cpd->GetMatrix(pnl::matCovariance, -1, &(discrParentValuesIndexes.front()));
                }
                else
                {
                    mat = cpd->GetMatrix(pnl::matCovariance);
                };
                
                return Net().ConvertMatrixToToken(mat);
            }
	}
    }
}

TokArr BayesNet::GetGaussianWeights(TokArr node, TokArr parent, TokArr tabParentValue)
{
    static const char fname[] = "GetGaussianWeight";

    if( (!node.size())||(!parent.size()) )
    {
	ThrowUsingError("Node and parent variables must be specified", fname);
    }
    if(node.size() > 1)
    {
	ThrowUsingError("Weights may be got only for one node", fname);
    }

    Vector<int> queryNdsOuter;   
    Vector<int> queryNdsInner, queryVls;
    Net().ExtractTokArr(node, &queryNdsInner, &queryVls, &Net().Graph().MapOuterToGraph());
    Net().Graph().IOuter(&queryNdsInner, &queryNdsOuter);

    Vector<int> parentsInner;
    Net().ExtractTokArr(parent, &parentsInner, &queryVls, &Net().Graph().MapOuterToGraph());
    
    int iNode = queryNdsInner[0];
    int iParent = parentsInner[0];

    const pnl::CFactor * cpd = Model().GetFactor(iNode);

    if (cpd->GetDistribFun()->IsDistributionSpecific() == 2) // delta
    {
        TokArr res;
        res << "0";
        return res;
    }
    if (cpd->GetDistribFun()->IsDistributionSpecific() == 1)
    {
	TokArr res;
	res << "uniform";
	return res;
    }
    else
    {
	pnl::intVector Domain;
	cpd->GetDomain(&Domain);
	int WeightsIndex = -1;

        const pnl::pConstNodeTypeVector *Types = cpd->GetDistribFun()->
            GetNodeTypesVector();
        int WIndex = 0;
	for (int i = 0; (i < Domain.size())&&(WeightsIndex == -1); i++)
	{
	    if (Domain[i] == iParent)
	    {
		WeightsIndex = WIndex;
	    };
            if (!((*Types)[i]->IsDiscrete()))
            {
                WIndex++;
            };
	}

	if (WeightsIndex == -1)
	{
	    ThrowUsingError("Wrong parameters in function", fname);
	}
	const pnl::CMatrix<float> *mat = 0;
        
        if (tabParentValue.size())
        {
            Vector<int> discrParentValuesIndexes = Net().Distributions().Distribution(
                queryNdsOuter[0])->GetDiscreteParentValuesIndexes(tabParentValue);
            mat = cpd->GetMatrix(pnl::matWeights, WeightsIndex, &(discrParentValuesIndexes.front()));
        }
        else
        {
            mat = cpd->GetMatrix(pnl::matWeights, WeightsIndex);
        };
        
	return Net().ConvertMatrixToToken(mat);
    }
}

TokArr BayesNet::GetPTabular(TokArr child, TokArr parents)
{
    static const char fname[] = "GetPTabular";
	
    int nchldComb = child.size();
    if( !nchldComb )
    {
	ThrowUsingError("Must be at least one combination for a child node", fname);
    }

    Vector<int> childNd, childVl;
    Vector<int> childNdInner;
    int i;
    Net().ExtractTokArr(child, &childNd, &childVl);

    Net().Graph().IGraph( &childNd, &childNdInner);

    // node for all child must be same (its may differ by combination only)
    for(i = childNd.size(); --i > 0; ++i)
    {
	if(childNd[i] != childNd[0])
	{
	    ThrowUsingError("Can't return probabilities for different nodes", fname);
	}
    }
	
    if( !childVl.size())
    {
	childVl.assign(nchldComb, -1);
    }

    Vector<int> parentNds, parentVls;
    Vector<int> parentNdsInner;
    int nparents = parents.size();
    if( nparents )
    {
	Net().ExtractTokArr(parents, &parentNds, &parentVls);
	Net().Graph().IGraph( &parentNds, &parentNdsInner);
	if( parentVls.size() == 0 ||
	    std::find(parentVls.begin(), parentVls.end(), -1 ) != parentVls.end() )
	{
	    ThrowInternalError("undefindes values for given parent nodes", "GetPTabular");
	}
    }
    else
    {
	Net().Graph().Graph()->GetParents( childNdInner.front(), &parentNdsInner );
	Net().Graph().IOuter( &parentNdsInner, &parentNds);
	nparents = parentNds.size();
	parentVls.assign(nparents, -1);
    }

    parentNds.resize(nparents + 1);
    parentNdsInner.resize(nparents + 1);
    parentVls.resize(nparents + 1);

    const pnl::CFactor * cpd = Model().GetFactor(childNdInner.front());
    const pnl::CMatrix<float> *mat = cpd->GetMatrix(pnl::matTable);
	
    TokArr result = "";
    for( i = 0; i < nchldComb; i++ )
    {
	parentNds[nparents] = childNd.front();
	parentNdsInner[nparents] = childNdInner.front();
	parentVls[nparents] = childVl[i];
	result << Net().CutReq( parentNds, parentVls, mat);
    }

    Net().Token().SetContext(result);
    return result;
}

void BayesNet::SetInferenceProperties(TokArr &nodes)
{
    pnl::CInfEngine *infEngine = &Inference();
    
    if(dynamic_cast<pnl::CGibbsSamplingInfEngine *>(infEngine) != NULL)
    {
	infEngine = &Inference(true);

	pnl::CGibbsSamplingInfEngine *infGibbs = static_cast<pnl::CGibbsSamplingInfEngine *>(infEngine);
	Vector<int> queryVls;
	pnl::intVecVector queries(1);
	queries[0].clear();
	
	Net().ExtractTokArr(nodes, &(queries[0]), &queryVls, &Net().Graph().MapOuterToGraph());
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
	/*
	const char *aGibbsProp[] = { "GibbsMaxTime", "GibbsNumStreams", "GibbsBurnIn" };
	const int nGibbsProp = sizeof(aGibbsProp)/sizeof(aGibbsProp[0]);
	
	int i;
	for(i = 0; i < nGibbsProp; i++)
	{
	    if(GetProperty(aGibbsProp[i]).length())
	    {
		int number = atoi(GetProperty(aGibbsProp[i]).c_str());
		if(number)
		{
		    switch(i)
		    {
		    case 0:
			infGibbs->SetMaxTime(number);
			break;
		    case 1:
			infGibbs->SetNumStreams(number);
			break;
		    case 2:
			infGibbs->SetBurnIn(number);
			break;
		    }
		}
	    }
	}
	*/	
	/*if(GetProperty("GibbsUseDSeparation").length())
	{
	    int BurnIn = atoi(GetProperty("GibbsUseDSeparation").c_str());
	    if(BurnIn)
	    {
		infGibbs->SetBurnIn(BurnIn);
	    }
	}*/
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

TokArr BayesNet::GetJPD( TokArr nodes )
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

    SetInferenceProperties(nodes);

    pnl::CInfEngine *infEngine = &Inference();
    
    infEngine->EnterEvidence( evid );
    
    int nnodes = nodes.size();
    Vector<int> queryNds, queryVls;
    Vector<int> queryNdsInner;
    Net().ExtractTokArr(nodes, &queryNds, &queryVls);
    Net().Graph().IGraph(&queryNds, &queryNdsInner);
    if(!queryVls.size())
    {
	queryVls.assign(nnodes, -1);
    }
    
    infEngine->MarginalNodes(&queryNdsInner.front(), queryNdsInner.size());
    
    const pnl::CPotential *pot = infEngine->GetQueryJPD();
    
    TokArr res = "";
    if (pot->GetDistribFun()->GetDistributionType() == pnl::dtScalar)
    {
        // ?
        res << "scalar";
    }
    else
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
//            res << "distribution";
        }
    }
    
    delete evid;
    Net().Token().SetContext(res);
    return res;
}

void BayesNet::EditEvidence(TokArr values)
{
    Net().EditEvidence(values);
}

void BayesNet::ClearEvid()
{
    Net().ClearEvid();
}

void BayesNet::CurEvidToBuf()
{
    Net().CurEvidToBuf();
}

void BayesNet::AddEvidToBuf(TokArr values)
{
    Net().AddEvidToBuf(values);
}

void BayesNet::ClearEvidBuf()
{
    Net().ClearEvidBuf();
    m_nLearnedEvidence = 0;
}

void BayesNet::SetParamLearningProperties()
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

void BayesNet::LearnParameters(TokArr aSample[], int nSample)
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
    m_nLearnedEvidence = Net().EvidenceBuf()->size();
    Learning().SetData(aEvidence.size(), &aEvidence.front());

    SetParamLearningProperties();
    Learning().Learn();
    for (i = 0; i < Net().Graph().nNode(); i++)
    {
	if(Net().Graph().IsValidINode(i))
	{
	    Net().Distributions().ResetDistribution(i, *Net().Model().GetFactor(Net().Graph().IGraph(i)));
	}
    }
    DropEvidences(aEvidence);
}

void BayesNet::LearnStructure(TokArr aSample[], int nSample)
{
    pnl::intVector vAnc, vDesc;//bogus vectors
	String priorVal;
	Vector<String> names;
    names = Net().Graph().Names();
    pnl::CBNet &bnet = Model();
	pnl::CBNet* newNet;
    pnl::CMlStaticStructLearnHC* pLearning = pnl::CMlStaticStructLearnHC::Create(
	&bnet, pnl::itStructLearnML, pnl::StructLearnHC, pnl::BIC,
	bnet.GetNumberOfNodes(), vAnc, vDesc, 1/*one restart*/ );

	switch(PropertyAbbrev("LearningStructureMethod"))
    {
    case 'm': //maximized likelihood score method
		pLearning->SetScoreMethod(pnl::MaxLh);
		break;
	case 'p': //predictive assessment score method
		pLearning->SetScoreMethod(pnl::PreAs);
		break;
	case 'b': //marginal likelyhood score method
		pLearning->SetScoreMethod(pnl::MarLh);
		break;	
	default:// default value
		pLearning->SetScoreMethod(pnl::MaxLh);
		break;
	};
	
	switch(PropertyAbbrev("LearningStructureScoreFun"))
    {
    case 'b': //using Bayessian Information Criterion 
		pLearning->SetScoreFunction(pnl::BIC);
		break;
	case 'a': //using Akaike`s Information Criterion 
		pLearning->SetScoreFunction(pnl::AIC);
		break;
	case 'w': //using function that does not penalty score
		pLearning->SetScoreFunction(pnl::WithoutFine);
		break;	
	default:// default value
		pLearning->SetScoreFunction(pnl::BIC);
		break;
	};

	switch(PropertyAbbrev("LearningStructurePrior"))
    {
    case 'd': //using Dirichlet priors only for marginal likelyhood score method
		pLearning->SetPriorType(pnl::Dirichlet);
		break;
	case 'k': //using K2 prior only for marginal likelyhood score method
		pLearning->SetPriorType(pnl::K2);
		priorVal = GetProperty("LearningStructureK2PriorVal");
		pLearning->SetK2PriorParam(atoi(priorVal.c_str()));	
		break;
	case 'b': //using BDeu priors only for marginal likelyhood score method
		pLearning->SetPriorType(pnl::BDeu);
		break;	
	default:// default value
		pLearning->SetPriorType(pnl::Dirichlet);
		break;
	};

    if(nSample)
    {
	for(int i = 0; i < nSample; ++i)
	{
	    AddEvidToBuf(aSample[i]);
	}
    }

    pnl::pEvidencesVector aEvidence;

    Net().TranslateBufToEvidences(&aEvidence, m_nLearnedEvidence);
    pLearning->SetData(aEvidence.size(), &aEvidence.front());
    pLearning->Learn();

#if 1
	if(pLearning->GetResultDAG()->IsTopologicallySorted())
	{
    const int* pRenaming = pLearning->GetResultRenaming();
    Vector<int> vRename(pRenaming, pRenaming + Model().GetNumberOfNodes());
    pRenaming = &vRename.front();
    pLearning->CreateResultBNet(const_cast<pnl::CDAG*>(pLearning->GetResultDAG()));
    newNet = pnl::CBNet::Copy(pLearning->GetResultBNet());
	Net().Reset(*newNet);
	}
	else
	{
	
	const int* pRenaming = pLearning->GetResultRenaming();
    pLearning->CreateResultBNet(const_cast<pnl::CDAG*>(pLearning->GetResultDAG()));
    newNet = pnl::CBNet::Copy(pLearning->GetResultBNet());
	Net().Reset(*newNet);
	int nnodes= names.size();
	int i;
	String tmp = "SS";
	for(i = 0; i < nnodes; i++)
	{
	Net().Graph().SetNodeName(i,tmp<<i);
	}
	for(i = 0; i < nnodes; i++)
	{
	Net().Graph().SetNodeName(i,names[pRenaming[i]]);
	}
	}
    int i;
	
   // Net().SetTopologicalOrder(pRenaming, newNet->GetGraph());

    for(i = 0; i < Net().Graph().nNode(); ++i)
    {
	Net().Distributions().ResetDistribution(i, *newNet->GetFactor(i));
    }

    //change ordering in current stuff
    // Note! it may happen that old ordering of nodes is consistent (i.e. is topological)
    // with new graph structure so theoretically no reordering required
    //this would be good to have such function in PNL that checks this situation and reorder
    //new network to old ordering. So we would not have to do all below

    int nnodes = newNet->GetNumberOfNodes();
//    Net().Token().RenameGraph(pRenaming);

    //clear learning engine
    delete m_Learning;
    m_Learning = 0;

    //clear inference engine
    delete m_Inference;
    m_Inference = 0;

    //change domain in evidences

    //change evidence on board
    //IT is in Token form, so do not need to change

   /* //change evidences in evidence buffer
    for( Vector<pnl::CEvidence*>::iterator it1 = Net().EvidenceBuf()->begin(); it1 != Net().EvidenceBuf()->end(); it1++ )
    {
        pnl::CEvidence* oldEv = *it1;
        pnl::CNodeValues* nv = oldEv;
    }*/
	
	
#endif
    DropEvidences(aEvidence);
}

TokArr BayesNet::GetMPE(TokArr nodes)
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

    SetInferenceProperties(nodes);

    pnl::CInfEngine *infEngine = &Inference();

    infEngine->EnterEvidence(evid, 1);

    int nnodes = nodes.size();
    int i;
    Vector<int> queryNds;
    Vector<int> queryNdsInner;
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

    Net().Graph().IGraph(&queryNds, &queryNdsInner);

    infEngine->MarginalNodes(&queryNdsInner.front(), queryNdsInner.size());

    const pnl::CEvidence *mpe = infEngine->GetMPE();
    TokArr result;

    for(i = 0; i < nnodes; ++i)
    {
	const pnl::Value *v = mpe->GetValue(queryNdsInner[i]);

	if((v->IsDiscrete() != 0) != Net().pnlNodeType(queryNds[i]).IsDiscrete())
	{
	    ThrowInternalError("Non-discrete value for discrete variable", "MPE");
	}

        if (Net().pnlNodeType(queryNds[i]).IsDiscrete())
            result.push_back(Net().Token().TokByNodeValue(queryNds[i], v->GetInt()));
        else
	{
	    int size = Net().pnlNodeType(queryNds[i]).GetNodeSize();
	    std::vector< float > NodeValues(size);

	    for (int i = 0; i < size; i++)
	    {
		NodeValues[i] = v[i].GetFlt();
	    }

    	    result.push_back(Tok(NodeValues));
	}
    }

    delete evid;
    Net().Token().SetContext(result);

    return result;
}

void BayesNet::SaveNet(const char *filename)
{
    pnl::CContextPersistence saver;
    pnl::CGroupObj group;

    if(!Net().SaveNet(&saver, &group))
    {
	ThrowInternalError("Can't save file", "SaveNet");
    }
    saver.Put(&group, "WrapperInfo", false);

    if(!saver.SaveAsXML(filename))
    {
	ThrowInternalError("Can't save file", "SaveNet");
    }
    // we must save net-specific data here.
    // BayesNet haven't any specific data for now
}

int BayesNet::SaveEvidBuf(const char *filename, NetConst::ESavingType mode)
{
    return Net().SaveEvidBuf(filename, mode);
}

void BayesNet::LoadNet(const char *filename)
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

    delete m_pNet;
    m_pNet = pNewNet;
	SpyTo(m_pNet);
    m_pNet->SetCallback(new BayesNetCallback());

    delete &Inference();
    m_Inference = NULL;

    delete &Learning();
    m_Learning = NULL;

    // We can load net-specific data here.
    // BayesNet haven't any specific data for now
}

int BayesNet::LoadEvidBuf(const char *filename, NetConst::ESavingType mode, TokArr columns)
{
    return Net().LoadEvidBuf(filename, mode, columns);
}

// whatNodes is array of tokens which specify the list of variables and optionally
// the required portion of observed nodes
// For example, if whatNodes = TokArr("Node1")
//
void BayesNet::GenerateEvidences( int nSample, bool ignoreCurrEvid, TokArr whatNodes )
{
    Net().GenerateEvidences(nSample, ignoreCurrEvid, whatNodes);
}

void BayesNet::MaskEvidBuf(TokArr whatNodes)
{
    Net().MaskEvidBuf(whatNodes);
}

//= private functions  =================================================

void BayesNet::DoNotify(const Message &msg)
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

pnl::CInfEngine &BayesNet::Inference(bool Recreate)
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

	    if(!infGibbs)
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
    }
    
    return *m_Inference;
}

pnl::CStaticLearningEngine &BayesNet::Learning()
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
    }
    return *m_Learning;
}

pnl::CBNet &BayesNet::Model()
{
    return static_cast<pnl::CBNet&>(Net().Model());
}

void BayesNet::SetProperty(const char *name, const char *value)
{
    m_pNet->SetProperty(name, value);
}

String BayesNet::GetProperty(const char *name) const
{
    return m_pNet->GetProperty(name);
}

const char BayesNet::PropertyAbbrev(const char *name) const
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

	if(!strcmp(name,"LearningStructureMethod"))
    {
	String learnNameMethod = GetProperty("LearningStructureMethod");

	if(!learnNameMethod.length())
	{
	    return 0;// default value
	}

	pnl::pnlVector<char> learnNameMethodVec(learnNameMethod.length());
	for(int i = 0; i < learnNameMethod.length(); ++i)
	{
	    learnNameMethodVec[i] = tolower(learnNameMethod[i]);
	}
	char *pLearnNameMethod = &learnNameMethodVec.front();

	if(strstr(pLearnNameMethod, "maxlh"))
	{
	    return 'm';
	}
	if(strstr(pLearnNameMethod, "preas"))
	{
	    return 'p';
	}
	if(strstr(pLearnNameMethod, "marlh"))
	{
	    return 'b';
	}
	else
	{
	    return 0;
	}
    }

	if(!strcmp(name,"LearningStructureScoreFun"))
    {
	String learnNameScoreFun = GetProperty("LearningStructureScoreFun");

	if(!learnNameScoreFun.length())
	{
	    return 0;// default value
	}

	pnl::pnlVector<char> learnNameScoreFunVec(learnNameScoreFun.length());
	for(int i = 0; i < learnNameScoreFun.length(); ++i)
	{
	    learnNameScoreFunVec[i] = tolower(learnNameScoreFun[i]);
	}
	char *pLearnNameScoreFun = &learnNameScoreFunVec.front();

	if(strstr(pLearnNameScoreFun, "bic"))
	{
	    return 'b';
	}
	if(strstr(pLearnNameScoreFun, "aic"))
	{
	    return 'a';
	}
	if(strstr(pLearnNameScoreFun, "withoutpenalty"))
	{
	    return 'w';
	}
	else
	{
	    return 0;
	}
	}

	if(!strcmp(name,"LearningStructurePrior"))
    {
	String learnNamePrior = GetProperty("LearningStructurePrior");

	if(!learnNamePrior.length())
	{
	    return 0;// default value
	}

	pnl::pnlVector<char> learnNamePriorVec(learnNamePrior.length());
	for(int i = 0; i < learnNamePrior.length(); ++i)
	{
	    learnNamePriorVec[i] = tolower(learnNamePrior[i]);
	}
	char *pLearnNamePrior = &learnNamePriorVec.front();

	if(strstr(pLearnNamePrior, "dirichlet"))
	{
	    return 'd';
	}
	if(strstr(pLearnNamePrior, "k2"))
	{
	    return 'k';
	}
	if(strstr(pLearnNamePrior, "bdeu"))
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

// returns logarithm of likelihood for current evidence
float BayesNet::GetCurEvidenceLogLik()
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
    float logLik = Model().ComputeLogLik(evid);

    delete evid;

    //TokArr result = TokArr(logLik);
    //Net().Token().SetContext(result);

    //return result;
    return logLik;
}

// returns array of logarithms of likelihood for evidences from buffer
TokArr BayesNet::GetEvidBufLogLik()
{
    TokArr result;
    pnl::pEvidencesVector evBuf;

    Net().TranslateBufToEvidences(&evBuf, 0);
    for(int i = 0; i < evBuf.size(); i++)
    {
	result.push_back(Model().ComputeLogLik(evBuf[i]));
	delete evBuf[i];
    }

    Net().Token().SetContext(result);

    return result;
}

// returns criterion value for last learning performance
float BayesNet::GetEMLearningCriterionValue()
{
    pnl::CStaticLearningEngine *learnEng = &Learning();
    if(dynamic_cast<pnl::CEMLearningEngine *>(learnEng) == NULL)
    {
	ThrowInternalError("Criterion value can be got only after EM learning", "GetEMLearningCriterionValue");
    }

    int nsteps;
    const float * score;
    learnEng->GetCriterionValue(&nsteps, &score);

    if(nsteps < 1)
    {
	ThrowInternalError("Learning was not done yet", "GetEMLearningCriterionValue");
    }

    return score[0];
}

pnl::CEvidence *BayesNet::GetPNLEvidence()
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
