#include "MRFCB.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "pnlWGraph.hpp"
#include "pnlWDistributions.hpp"
#include "WCliques.hpp"
#include "WDistribFun.hpp"

#include "pnl_dll.hpp"

PNLW_BEGIN

#if !defined(REMOVE_DOUBLE_CODE)
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
#endif

pnl::CGraphicalModel *MRFCallback::CreateModel(ProbabilisticNet &net)
{

    int nNode = net.nNetNode();
    int i;
    // set nodes types 
    pnl::nodeTypeVector aNodeType;
    //Vector<pnl::CNodeType> aNodeType;
    Vector<int> aNodeAssociation;
    aNodeAssociation.resize(nNode);
    aNodeType.reserve(nNode > 16 ? 8:4);
    for(i = 0; i < nNode; i++)
    {
	const pnl::CNodeType &nt = net.pnlNodeType(i);

	aNodeAssociation[i] = nodeAssociation(&aNodeType,
	    nt.IsDiscrete(), nt.GetNodeSize());
    }

    // set cliques
    Vector<Vector<int> > cliques = net.Distributions()->Cliques()->Cliques();

    // create MNet
    pnl::CMNet *pnlNet = pnl::CMNet::Create(nNode, aNodeType, aNodeAssociation,
	cliques);

    // attach parameters
    for(i = 0; i < cliques.size(); i++)
    {
	WDistribFun *pWDF = net.Distributions()->Distribution(i);
        PNL_CHECK_IS_NULL_POINTER(pWDF);

        pnlNet->AllocFactor(i);

        if (net.pnlNodeType(i).IsDiscrete())
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
    static_cast<pnl::CMNet*>(net.Model())->GenerateSamples( newSamples,
	nSample, evid);
}

PNLW_END
