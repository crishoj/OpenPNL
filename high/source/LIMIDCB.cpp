#include "LIMIDCB.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "pnlWGraph.hpp"
#include "pnlWDistributions.hpp"
#include "WDistribFun.hpp"

#include "pnl_dll.hpp"

PNLW_BEGIN

pnl::CGraphicalModel *LIMIDCallback::CreateModel(ProbabilisticNet &net)
{
    int i, iWNode;
    Vector<pnl::CNodeType> aNodeType;
    Vector<int> aNodeAssociation;

    // create BNet
    GetNodeInfo(&aNodeType, &aNodeAssociation, net);

    pnl::CIDNet *pIDNet = pnl::CIDNet::Create(aNodeAssociation.size(),
	aNodeType.size(), &aNodeType.front(), &aNodeAssociation.front(),
	net.Graph()->Graph(true));

    // attach parameters
    Vector<String> aNodeName(net.Graph()->Names());

    for(i = 0; i < aNodeName.size(); i++)
    {
	// it is index for wrapper node, pnl node index is 'i'
	iWNode = net.Graph()->IOuter(net.Graph()->INode(aNodeName[i]));

	WDistribFun *pWDF = net.Distributions()->Distribution(iWNode);
        PNL_CHECK_IS_NULL_POINTER(pWDF);

        pIDNet->AllocFactor(i);

        if (net.pnlNodeType(i).IsDiscrete())
        {
            pnl::CDenseMatrix<float> *mat = pWDF->Matrix(pnl::matTable);
            PNL_CHECK_IS_NULL_POINTER(mat);
	
            pIDNet->GetFactor(i)->AttachMatrix(mat, pnl::matTable);
        }
        else
        {
            ThrowUsingError("Wrong type of distribution", "CreateModel");
        }
    }

/*    for (i =0; i<nNode; i++ )
      pIDNet->GetFactor(i)->GetDistribFun()->Dump();
*/
    return pIDNet;
}

PNLW_END
