#include "MRFCB.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "pnlWGraph.hpp"
#include "pnlWDistributions.hpp"
#include "WCliques.hpp"
#include "WDistribFun.hpp"
#include "TokenCover.hpp"

#include "pnl_dll.hpp"

PNLW_BEGIN

pnl::CGraphicalModel *MRFCallback::CreateModel(ProbabilisticNet &net)
{
    int nNode = net.nNetNode();
    int i;
    
    Vector<pnl::CNodeType> aNodeType;
    Vector<int> aNodeAssociation;

    GetNodeInfo(&aNodeType, &aNodeAssociation, net);

    // set cliques
    Vector<Vector<int> > cliques = net.Distributions().Cliques().Cliques();

    bool isMRF2 = true;
    for(i = 0; i < cliques.size(); i++)
    {
	if(cliques[i].size() != 2)
	{
	    isMRF2 = false;
	    break;
	}
    }

    // create MNet
    pnl::CMNet *pnlNet;
    if(isMRF2)
    {
	pnlNet = pnl::CMRF2::Create(nNode, aNodeType, aNodeAssociation, cliques);
    }
    else
    {
	pnlNet = pnl::CMNet::Create(nNode, aNodeType, aNodeAssociation, cliques);
    }

    // attach parameters
    for(i = 0; i < cliques.size(); i++)
    {
	WDistribFun *pWDF = net.Distributions().Distribution(i);
        PNL_CHECK_IS_NULL_POINTER(pWDF);

        pnlNet->AllocFactor(i);

        if (net.Distributions().DistributionType(i).IsDiscrete())
        {
            pnl::CDenseMatrix<float> *mat = dynamic_cast<WTabularDistribFun*>(pWDF)->Matrix(0);
            PNL_CHECK_IS_NULL_POINTER(mat);
	
            pnlNet->GetFactor(i)->AttachMatrix(mat, pnl::matTable);
	    //pnlNet->GetFactor(i)->AttachMatrix(mat->Copy(mat), pnl::matDirichlet);
        }
    }
    return pnlNet;
}

void MRFCallback::GenerateSamples(ProbabilisticNet &net,
					pnl::pEvidencesVector *newSamples,
					int nSample, pnl::CEvidence *evid)
{
    static_cast<pnl::CMNet&>(net.Model()).GenerateSamples( newSamples,
	nSample, evid);
}

PNLW_END
