#include "BNetCB.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "pnlWGraph.hpp"
#include "pnlWDistributions.hpp"
#include "WDistribFun.hpp"

#include "pnl_dll.hpp"

static int nodeAssociation(Vector<pnl::CNodeType> *paNodeType, bool isDiscrete, int size);

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

pnl::CGraphicalModel *BayesNetCallback::CreateModel(ProbabilisticNet &net)
{
    int nNode = net.nNetNode();
    int i;
    Vector<pnl::CNodeType> aNodeType;
    Vector<int> aNodeAssociation;

    // create BNet
    aNodeAssociation.resize(nNode);
    aNodeType.reserve(nNode > 16 ? 8:4);
    for(i = 0; i < nNode; i++)
    {
	const pnl::CNodeType &nt = net.pnlNodeType(i);

	aNodeAssociation[i] = nodeAssociation(&aNodeType,
	    nt.IsDiscrete(), nt.GetNodeSize());
    }

    pnl::CBNet *pnlNet = pnl::CBNet::Create(nNode, aNodeType.size(),
	&aNodeType.front(), &aNodeAssociation.front(), net.Graph()->Graph(true));

    // attach parameters
    for(i = 0; i < nNode; i++)
    {
	WDistribFun *pWDF = net.Distributions()->Distribution(i);
        PNL_CHECK_IS_NULL_POINTER(pWDF);

        pnlNet->AllocFactor(i);

        if (net.pnlNodeType(i).IsDiscrete())
        {
            pnl::CDenseMatrix<float> *mat = dynamic_cast<WTabularDistribFun*>(pWDF)->Matrix(0);
            PNL_CHECK_IS_NULL_POINTER(mat);
	
            pnlNet->GetFactor(i)->AttachMatrix(mat, pnl::matTable);
	    pnlNet->GetFactor(i)->AttachMatrix(mat->Copy(mat), pnl::matDirichlet);
        }
        else
        {
            if (dynamic_cast<WGaussianDistribFun*>(pWDF)->IsDistributionSpecific() == 1)
            {
                const pnl::pConstNodeTypeVector* ntVec = pnlNet->GetFactor(i)->GetDistribFun()
                    ->GetNodeTypesVector();
                int NumberOfNodes = pnlNet->GetFactor(i)->GetDistribFun()->GetNumberOfNodes();

                pnl::CGaussianDistribFun *gaudf;
                gaudf = pnl::CGaussianDistribFun::CreateUnitFunctionDistribution(NumberOfNodes, 
                    &ntVec->front());
                pnlNet->GetFactor(i)->SetDistribFun(gaudf);
                
                delete gaudf;
            }
            else
            {
                pnl::CDenseMatrix<float> *mean = 
                    dynamic_cast<WGaussianDistribFun*>(pWDF)->Matrix(1);
                PNL_CHECK_IS_NULL_POINTER(mean);

                pnl::CDenseMatrix<float> *cov = 
                    dynamic_cast<WGaussianDistribFun*>(pWDF)->Matrix(2);
                PNL_CHECK_IS_NULL_POINTER(cov);

                pnlNet->GetFactor(i)->AttachMatrix(mean, pnl::matMean);
                pnlNet->GetFactor(i)->AttachMatrix(mean->Copy(mean), pnl::matWishartMean);

                pnlNet->GetFactor(i)->AttachMatrix(cov, pnl::matCovariance);
                pnlNet->GetFactor(i)->AttachMatrix(cov->Copy(cov), pnl::matWishartCov);

		int NDims;
		const int *Ranges;
		cov->GetRanges(&NDims, &Ranges);

                dynamic_cast<pnl::CGaussianDistribFun*>(pnlNet->GetFactor(i)->GetDistribFun())
                    ->SetFreedomDegrees(1 , Ranges[0] + 2); 
                
                int NumOfNds = pnlNet->GetFactor(i)->GetDistribFun()->GetNumberOfNodes();
		for (int parent = 0; parent < (NumOfNds-1); parent++)
		{
                    pnl::CDenseMatrix<float> *weight = 
                        dynamic_cast<WGaussianDistribFun*>(pWDF)->Matrix(3,parent);
                    PNL_CHECK_IS_NULL_POINTER(weight);

                    pnlNet->GetFactor(i)->AttachMatrix(weight, pnl::matWeights,parent);
                }
            }
        }
    }
    return pnlNet;
}

void BayesNetCallback::GenerateSamples(ProbabilisticNet &net,
					pnl::pEvidencesVector *newSamples,
					int nSample, pnl::CEvidence *evid)
{
    static_cast<pnl::CBNet*>(net.Model())->GenerateSamples( newSamples,
	nSample, evid);
}

