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
    int iW, iPNL, i;
    Vector<String> aNodeName(net.Graph().Names());

    paNodeAssociation->resize(aNodeName.size());
    paNodeType->reserve(aNodeName.size() > 16 ? 8:4);

    for(i = 0; i < aNodeName.size(); i++)
    {
	iW = net.Graph().INode(aNodeName[i]);
	iPNL = net.Graph().IGraph(iW);
	const pnl::CNodeType &nt = net.pnlNodeType(iW);

	(*paNodeAssociation)[iPNL] = NodeAssociation(paNodeType,
	    nt.IsDiscrete(), nt.GetNodeSize(), nt.GetNodeState());
    }

    return true;
}

bool
NetCallback::CommonAttachFactors(pnl::CGraphicalModel &pnlModel,
	const ProbabilisticNet &net)
{
    int i, iPNL, iWNode;
    Vector<String> aNodeName(net.Graph().Names());

    // attach parameters for every nodes
    for(i = 0; i < aNodeName.size(); i++)
    {
	// it is index for wrapper node, pnl node index is 'iPNL'
	iWNode = net.Graph().INode(aNodeName[i]);
	iPNL = net.Graph().IGraph(iWNode);

	WDistribFun *pWDF = net.Distributions().Distribution(iWNode);
        PNL_CHECK_IS_NULL_POINTER(pWDF);

        pnlModel.AllocFactor(iPNL);

	pnl::CFactor *pPNLF = pnlModel.GetFactor(iPNL);
        PNL_CHECK_IS_NULL_POINTER(pPNLF);

        if (net.pnlNodeType(iWNode).IsDiscrete())
        {
            bool isSoftMax = false;
            DistribFunDesc  *des = pWDF->desc();
            for (int j=0; j<des->nNode(); j++ )
            {
                TokIdNode * tokId = des->node(j);
                if (!static_cast<pnl::CNodeType*>(tokId->v_prev->data)->IsDiscrete() )
                {
                    isSoftMax = true;
                    break;
                }
            }
            if (!isSoftMax)
            {
                pnl::CDenseMatrix<float> *mat = pWDF->Matrix(pnl::matTable);
                PNL_CHECK_IS_NULL_POINTER(mat);
                
                pPNLF->AttachMatrix(mat, pnl::matTable);
                pPNLF->AttachMatrix(mat->Copy(mat), pnl::matDirichlet);
            }
            else
            {
                pnl::CDenseMatrix<float> *weight = pWDF->Matrix(pnl::matWeights, 0);
                PNL_CHECK_IS_NULL_POINTER(weight);
                pPNLF->AttachMatrix(weight, pnl::matWeights);
                // offsetVector 
                pnl::floatVector *offVector = dynamic_cast<WSoftMaxDistribFun*>(pWDF)->
                    OffsetVector();
                pnl::CDistribFun *df = pPNLF->GetDistribFun();
                
                float *offVec = new float[offVector->size()];
                memcpy(&offVec[0], &offVector->front(), (offVector->size())*sizeof(float) );

                dynamic_cast<pnl::CSoftMaxDistribFun*>(df)->
                    AllocOffsetVector(offVec);

            }
        }
        else
        {
	    WGaussianDistribFun* pGWDF = dynamic_cast<WGaussianDistribFun*>(pWDF);
	    PNL_CHECK_IS_NULL_POINTER(pGWDF);
            if (pGWDF->IsDistributionSpecific() == 1)
            {
                const pnl::pConstNodeTypeVector* ntVec = pPNLF->GetDistribFun()
                    ->GetNodeTypesVector();
                int NumberOfNodes = pPNLF->GetDistribFun()->GetNumberOfNodes();

                pnl::CGaussianDistribFun *gaudf = pnl::CGaussianDistribFun
		    ::CreateUnitFunctionDistribution(NumberOfNodes, &ntVec->front());
                pPNLF->SetDistribFun(gaudf);
                
                delete gaudf;
            }
            else
            {
                pnl::CDenseMatrix<float> *mean = pWDF->Matrix(pnl::matMean);
                PNL_CHECK_IS_NULL_POINTER(mean);
                pnl::CDenseMatrix<float> *cov = pWDF->Matrix(pnl::matCovariance);
                PNL_CHECK_IS_NULL_POINTER(cov);

                pPNLF->AttachMatrix(mean, pnl::matMean);
                pPNLF->AttachMatrix(mean->Copy(mean), pnl::matWishartMean);

                pPNLF->AttachMatrix(cov, pnl::matCovariance);
                pPNLF->AttachMatrix(cov->Copy(cov), pnl::matWishartCov);

		int NDims;
		const int *Ranges;
		cov->GetRanges(&NDims, &Ranges);

                dynamic_cast<pnl::CGaussianDistribFun*>(pPNLF->GetDistribFun())
                    ->SetFreedomDegrees(1 , Ranges[0] + 2); 
                
                int NumOfNds = pPNLF->GetDistribFun()->GetNumberOfNodes();
		for (int parent = 0; parent < (NumOfNds-1); parent++)
		{
                    pnl::CDenseMatrix<float> *weight = pWDF->Matrix(pnl::matWeights, parent);
                    PNL_CHECK_IS_NULL_POINTER(weight);

                    pPNLF->AttachMatrix(weight, pnl::matWeights, parent);
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
