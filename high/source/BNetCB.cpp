#include "BNetCB.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "pnlWGraph.hpp"
#include "pnlWDistributions.hpp"
#include "WDistribFun.hpp"

#include "pnl_dll.hpp"

PNLW_BEGIN

pnl::CGraphicalModel *BayesNetCallback::CreateModel(ProbabilisticNet &net)
{
    Vector<pnl::CNodeType> aNodeType;
    Vector<int> aNodeAssociation;

    // create BNet
    GetNodeInfo(&aNodeType, &aNodeAssociation, net);

    pnl::CBNet *pnlNet = pnl::CBNet::Create(aNodeAssociation.size(), aNodeType.size(),
	&aNodeType.front(), &aNodeAssociation.front(), net.Graph().Graph(true));

    // attach parameters
    CommonAttachFactors(*pnlNet, net);

    return pnlNet;
}

void BayesNetCallback::GenerateSamples(ProbabilisticNet &net,
					pnl::pEvidencesVector *newSamples,
					int nSample, pnl::CEvidence *evid)
{
    static_cast<pnl::CBNet&>(net.Model()).GenerateSamples( newSamples,
	nSample, evid);
}

PNLW_END
