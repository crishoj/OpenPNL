#include "DBNCB.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "pnlWGraph.hpp"
#include "pnlWDistributions.hpp"
#include "WDistribFun.hpp"

#include "pnl_dll.hpp"

PNLW_USING

pnl::CGraphicalModel *DBNCallback::CreateModel(ProbabilisticNet &net)
{
    Vector<pnl::CNodeType> aNodeType;
    Vector<int> aNodeAssociation;

    // create BNet
    GetNodeInfo(&aNodeType, &aNodeAssociation, net);

    pnl::CBNet *pnlNet = pnl::CBNet::Create(aNodeAssociation.size(), aNodeType.size(),
	&aNodeType.front(), &aNodeAssociation.front(), net.Graph().Graph(true));

    // attach parameters
    CommonAttachFactors(*pnlNet, net);
    pnl::CDBN *pnlDBN = pnl::CDBN::Create(pnlNet);
    // ?? delete pnlNet;?

    return pnlDBN;
}
