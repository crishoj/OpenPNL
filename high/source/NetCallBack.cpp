#include "NetCallBack.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "pnlWGraph.hpp"
#include "pnlWDistributions.hpp"
#include "WDistribFun.hpp"
#include "TokenCover.hpp"

#include "pnl_dll.hpp"

PNLW_USING

bool
NetCallback::GetNodeInfo(Vector<pnl::CNodeType> *paNodeType,
			 Vector<int> *paNodeAssociation,
			 const ProbabilisticNet &net)
{
    int i;
    Vector<String> aNodeName(net.Graph().Names());

    // create BNet
    paNodeAssociation->resize(aNodeName.size());
    paNodeType->reserve(aNodeName.size() > 16 ? 8:4);

    for(i = 0; i < aNodeName.size(); i++)
    {
	const pnl::CNodeType &nt = net.pnlNodeType(net.Graph().INode(aNodeName[i]));

	(*paNodeAssociation)[i] = NodeAssociation(paNodeType,
	    nt.IsDiscrete(), nt.GetNodeSize(), nt.GetNodeState());
    }

    return true;
}

bool
NetCallback::CommonAttachFactors(pnl::CGraphicalModel &pnlModel,
	const ProbabilisticNet &net)
{
    int i, iWNode;
    Vector<String> aNodeName(net.Graph().Names());

    // attach parameters for every nodes
    for(i = 0; i < aNodeName.size(); i++)
    {
	// it is index for wrapper node, pnl node index is 'i'
	iWNode = net.Graph().IOuter(net.Graph().INode(aNodeName[i]));

	WDistribFun *pWDF = net.Distributions().Distribution(iWNode);
        PNL_CHECK_IS_NULL_POINTER(pWDF);

        pnlModel.AllocFactor(i);

        if (net.pnlNodeType(iWNode).IsDiscrete())
        {
            pnl::CDenseMatrix<float> *mat = pWDF->Matrix(pnl::matTable);
            PNL_CHECK_IS_NULL_POINTER(mat);
	
            pnlModel.GetFactor(i)->AttachMatrix(mat, pnl::matTable);
	    pnlModel.GetFactor(i)->AttachMatrix(mat->Copy(mat), pnl::matDirichlet);
        }
        else
        {
	    WGaussianDistribFun* pGWDF = dynamic_cast<WGaussianDistribFun*>(pWDF);
	    PNL_CHECK_IS_NULL_POINTER(pGWDF);
            if (pGWDF->IsDistributionSpecific() == 1)
            {
                const pnl::pConstNodeTypeVector* ntVec = pnlModel.GetFactor(i)->GetDistribFun()
                    ->GetNodeTypesVector();
                int NumberOfNodes = pnlModel.GetFactor(i)->GetDistribFun()->GetNumberOfNodes();

                pnl::CGaussianDistribFun *gaudf = pnl::CGaussianDistribFun
		    ::CreateUnitFunctionDistribution(NumberOfNodes, &ntVec->front());
                pnlModel.GetFactor(i)->SetDistribFun(gaudf);
                
                delete gaudf;
            }
            else
            {
                pnl::CDenseMatrix<float> *mean = pWDF->Matrix(pnl::matMean);
                PNL_CHECK_IS_NULL_POINTER(mean);
                pnl::CDenseMatrix<float> *cov = pWDF->Matrix(pnl::matCovariance);
                PNL_CHECK_IS_NULL_POINTER(cov);

                pnlModel.GetFactor(i)->AttachMatrix(mean, pnl::matMean);
                pnlModel.GetFactor(i)->AttachMatrix(mean->Copy(mean), pnl::matWishartMean);

                pnlModel.GetFactor(i)->AttachMatrix(cov, pnl::matCovariance);
                pnlModel.GetFactor(i)->AttachMatrix(cov->Copy(cov), pnl::matWishartCov);

		int NDims;
		const int *Ranges;
		cov->GetRanges(&NDims, &Ranges);

                dynamic_cast<pnl::CGaussianDistribFun*>(pnlModel.GetFactor(i)->GetDistribFun())
                    ->SetFreedomDegrees(1 , Ranges[0] + 2); 
                
                int NumOfNds = pnlModel.GetFactor(i)->GetDistribFun()->GetNumberOfNodes();
		for (int parent = 0; parent < (NumOfNds-1); parent++)
		{
                    pnl::CDenseMatrix<float> *weight = pWDF->Matrix(pnl::matWeights, parent);
                    PNL_CHECK_IS_NULL_POINTER(weight);

                    pnlModel.GetFactor(i)->AttachMatrix(weight, pnl::matWeights, parent);
                }
            }
        }
    }

    return true;
}

int
NetCallback::NodeAssociation(Vector<pnl::CNodeType> *paNodeType,
			     bool isDiscrete, int size, int nodeState)
{
    pnl::CNodeType nt(isDiscrete ? 1:0, size, pnl::EIDNodeState(nodeState));

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
