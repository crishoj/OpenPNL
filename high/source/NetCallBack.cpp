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
    static const char fname[] = "CommonAttachFactors";
 
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

        pnl::CDistribFun *pPNLdf = pPNLF->GetDistribFun();
        PNL_CHECK_IS_NULL_POINTER(pPNLdf);

        if (net.pnlNodeType(iWNode).IsDiscrete())
        {
            bool isSoftMax = false;
            DistribFunDesc  *des = pWDF->desc();
            for (int j=0; j<des->nNode(); j++ )
            {
                TokIdNode *tokId = des->node(j);
                if (!static_cast<pnl::CNodeType*>(tokId->v_prev->data)->IsDiscrete() )
                {
                    isSoftMax = true;
                    break;
                }
            }
            bool isCondSoftMax = false;
            
            if (isSoftMax)
            {
                for (int j=0; j<des->nNode()-1; j++ )
                {
                    TokIdNode * tokId = des->node(j);
                    if (static_cast<pnl::CNodeType*>(tokId->v_prev->data)->IsDiscrete() )
                    {
                        isCondSoftMax = true;
                        break;
                    }
                }
            }
            if (isCondSoftMax)
            {
                pPNLdf->CreateDefaultMatrices();
                pnl::CCondSoftMaxDistribFun *pCondSoftMaxDFHigh = 
                    dynamic_cast<WCondSoftMaxDistribFun*>(pWDF)->GetDistribution();
                PNL_CHECK_IS_NULL_POINTER(pCondSoftMaxDFHigh);
                
                pnl::CCondSoftMaxDistribFun *pCondSoftMaxDFPNL = 
                    dynamic_cast<pnl::CCondSoftMaxDistribFun*>(pPNLdf);
                PNL_CHECK_IS_NULL_POINTER(pCondSoftMaxDFPNL);
                
                pnl::CMatrixIterator<pnl::CSoftMaxDistribFun*>* iterHigh =
                    pCondSoftMaxDFHigh->GetMatrixWithDistribution()->InitIterator();
                
                pnl::CMatrixIterator<pnl::CSoftMaxDistribFun*>* iterPNL =
                    pCondSoftMaxDFPNL->GetMatrixWithDistribution()->InitIterator();

                for (iterHigh, iterPNL; 
                    (pCondSoftMaxDFHigh->GetMatrixWithDistribution()->IsValueHere(iterHigh))&&
                    (pCondSoftMaxDFPNL->GetMatrixWithDistribution()->IsValueHere(iterPNL));
                    pCondSoftMaxDFHigh->GetMatrixWithDistribution()->Next(iterHigh),
                    pCondSoftMaxDFPNL->GetMatrixWithDistribution()->Next(iterPNL) )
                {
                    pnl::CSoftMaxDistribFun* DataHigh =
                        *(pCondSoftMaxDFHigh->GetMatrixWithDistribution()->Value(iterHigh));
                    PNL_CHECK_IS_NULL_POINTER(DataHigh);

                    pnl::CMatrix<float> *matr = DataHigh->GetMatrix(pnl::matWeights);
                    pnl::floatVector *ofVect = DataHigh->GetOffsetVector();
                    
                    pnl::CSoftMaxDistribFun* DataPNL =
                        *(pCondSoftMaxDFPNL->GetMatrixWithDistribution()->Value(iterPNL));
                    PNL_CHECK_IS_NULL_POINTER(DataPNL);

                    DataPNL->AttachOffsetVector(ofVect);
                    DataPNL->AttachMatrix(matr, pnl::matWeights);
                }
            }
            else
                if (isSoftMax)
                {
                    pnl::CDenseMatrix<float> *weight = pWDF->Matrix(pnl::matWeights, 0);
                    PNL_CHECK_IS_NULL_POINTER(weight);
                    pPNLF->AttachMatrix(weight, pnl::matWeights);
                    // offsetVector 
                    pnl::floatVector *offVector = dynamic_cast<WSoftMaxDistribFun*>(pWDF)->
                        OffsetVector();
                    pnl::CDistribFun *df = pPNLF->GetDistribFun();
                    
                    dynamic_cast<pnl::CSoftMaxDistribFun*>(df)->AttachOffsetVector(offVector);
                }
                else
                {
                    pnl::CDenseMatrix<float> *mat = pWDF->Matrix(pnl::matTable);
                    PNL_CHECK_IS_NULL_POINTER(mat);
                    
                    pPNLF->AttachMatrix(mat, pnl::matTable);
                    pPNLF->AttachMatrix(mat->Copy(mat), pnl::matDirichlet);
                }
        }
        else
        {
	    WGaussianDistribFun* pGWDF = dynamic_cast<WGaussianDistribFun* >(pWDF);
	    WCondGaussianDistribFun* pCGWDF = dynamic_cast<WCondGaussianDistribFun* >(pWDF);

            //Gaussian or cond. gaussian case
	    if (pGWDF || pCGWDF)
	    {
                Vector<int> indexes;
                pnl::CMatrix<pnl::CGaussianDistribFun* > * pCondMatrix = 0;

                if (pCGWDF)
                {
                    //Gaussian matrixes for all combinations of discrete parents
                    pCondMatrix = pCGWDF->GetPNLDistribFun()->GetMatrixWithDistribution();
                }
                else
                {
                    const int range = 1;
                    pnl::CGaussianDistribFun* pGDF = dynamic_cast<pnl::CGaussianDistribFun*>(pPNLF->GetDistribFun());
                    //For gaussian case there is only 1 distrib function
                    pCondMatrix = pnl::CDenseMatrix<pnl::CGaussianDistribFun* >::Create(1, &range, &pGDF);
                };

                pnl::CMatrixIterator<pnl::CGaussianDistribFun* > * iterThis = pCondMatrix->InitIterator();
                //Excess all distrib functions to attach them to pPNLF
                for( iterThis; pCondMatrix->IsValueHere( iterThis );
                    pCondMatrix->Next(iterThis))
                {                 
                    //Discrete parent combination for cond. gaussian case
                    pCondMatrix->Index( iterThis, &indexes );

                    bool isDistributionSpecific = (pGWDF)?
                        (pGWDF->IsDistributionSpecific() == 1):(pCGWDF->IsDistributionSpecific() == 1);

                    if (isDistributionSpecific)
		    {
                        if (pGWDF)
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
                            ThrowUsingError("At the moment of writing this function WCondGaussianDistribFun class could not be specific. Please contact developers.", fname);
                        };
		    }
		    else
		    {
                        const int *pDiscrParentValues = 0;

                        if (pCGWDF)
                        {
                            pDiscrParentValues = &(indexes.front());
                        };

		        pnl::CDenseMatrix<float> *mean = pWDF->Matrix(pnl::matMean, -1, pDiscrParentValues);
		        PNL_CHECK_IS_NULL_POINTER(mean);
		        pnl::CDenseMatrix<float> *cov = pWDF->Matrix(pnl::matCovariance, -1, pDiscrParentValues);
		        PNL_CHECK_IS_NULL_POINTER(cov);

		        pPNLF->AttachMatrix(mean, pnl::matMean, -1, pDiscrParentValues);
		        pPNLF->AttachMatrix(mean->Copy(mean), pnl::matWishartMean, -1, pDiscrParentValues);

		        pPNLF->AttachMatrix(cov, pnl::matCovariance, -1, pDiscrParentValues);
		        pPNLF->AttachMatrix(cov->Copy(cov), pnl::matWishartCov, -1, pDiscrParentValues);

		        int NDims;
		        const int *Ranges;
		        cov->GetRanges(&NDims, &Ranges);

                        pnl::CGaussianDistribFun* a = dynamic_cast<pnl::CGaussianDistribFun*>(pPNLF->GetDistribFun());

		        if (pGWDF)
                        {
                            dynamic_cast<pnl::CGaussianDistribFun*>(pPNLF->GetDistribFun())
			        ->SetFreedomDegrees(1 , Ranges[0] + 2); 
                        }
                        else
                        {
                            const_cast<pnl::CGaussianDistribFun *>(dynamic_cast<pnl::CCondGaussianDistribFun*>(pPNLF->GetDistribFun())->GetDistribution(pDiscrParentValues))->
			        SetFreedomDegrees(1 , Ranges[0] + 2); 
                        };
                
                        int ContParent = 0;
		        int NumOfNds = pPNLF->GetDistribFun()->GetNumberOfNodes();
                        const pnl::pConstNodeTypeVector *pNodeTypes = pPNLF->GetDistribFun()->GetNodeTypesVector();
                        for (int parent = 0; parent < (NumOfNds-1); parent++)
		        {
                            if (!((*pNodeTypes)[parent]->IsDiscrete()))
                            {
			        pnl::CDenseMatrix<float> *weight = pWDF->Matrix(pnl::matWeights, ContParent, pDiscrParentValues);
			        PNL_CHECK_IS_NULL_POINTER(weight);

			        pPNLF->AttachMatrix(weight, pnl::matWeights, ContParent, pDiscrParentValues);
                                ContParent++;
                            };
		        }
		    };
                };
	    }
	    else 
	    {
                PNL_CHECK_IS_NULL_POINTER(pGWDF);
	    };
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
