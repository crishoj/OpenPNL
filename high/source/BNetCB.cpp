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
	pnl::CNodeType &nt = net.pnlNodeType(i);

	aNodeAssociation[i] = nodeAssociation(&aNodeType,
	    nt.IsDiscrete(), nt.GetNodeSize());
    }

    pnl::CBNet *pnlNet = pnl::CBNet::Create(nNode, aNodeType.size(),
	&aNodeType.front(), &aNodeAssociation.front(), net.Graph()->Graph(true));

    // attach parameters
    for(i = 0; i < nNode; i++)
    {// tabular only
	WDistribFun *pWDF = net.Distributions()->Distribution(i);
	PNL_CHECK_IS_NULL_POINTER(dynamic_cast<WTabularDistribFun*>(pWDF));
	pnl::CDenseMatrix<float> *mat = dynamic_cast<WTabularDistribFun*>(pWDF)->Matrix(0);
	PNL_CHECK_IS_NULL_POINTER(mat);
	pnlNet->AllocFactor(i);
	pnlNet->GetFactor(i)->AttachMatrix(mat, pnl::matTable);
	pnlNet->GetFactor(i)->AttachMatrix(mat->Copy(mat), pnl::matDirichlet);
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

