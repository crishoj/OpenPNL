#include "LIMIDCB.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "pnlWGraph.hpp"
#include "pnlWDistributions.hpp"
#include "WDistribFun.hpp"

#include "pnl_dll.hpp"

PNLW_BEGIN

static int nodeAssociation(Vector<pnl::CNodeType> *paNodeType, bool isDiscrete, int size, 
                           pnl::EIDNodeState nodeState);

static int nodeAssociation(Vector<pnl::CNodeType> *paNodeType, bool isDiscrete, int size, 
                           pnl::EIDNodeState nodeState)
{
    pnl::CNodeType nt(isDiscrete ? 1:0, size, nodeState);

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

pnl::CGraphicalModel *LIMIDCallback::CreateModel(ProbabilisticNet &net)
{
    int nNode = net.nNetNode();
    int i;
    Vector<pnl::CNodeType> aNodeType;
    Vector<int> aNodeAssociation;

    // create IDNet
    aNodeAssociation.resize(nNode);
    aNodeType.reserve(nNode > 16 ? 8:4);
    for(i = 0; i < nNode; i++)
    {
	const pnl::CNodeType &nt = net.pnlNodeType(i);

	aNodeAssociation[i] = nodeAssociation(&aNodeType,
	    nt.IsDiscrete(), nt.GetNodeSize(), nt.GetNodeState());
    }

    pnl::CIDNet *pIDNet = pnl::CIDNet::Create(nNode, aNodeType.size(), &aNodeType.front(),
    &aNodeAssociation.front(), net.Graph()->Graph(true));

    // attach parameters
    for(i = 0; i < nNode; i++)
    {
	WDistribFun *pWDF = net.Distributions()->Distribution(i);
        PNL_CHECK_IS_NULL_POINTER(pWDF);

        pIDNet->AllocFactor(i);

        if (net.pnlNodeType(i).IsDiscrete())
        {
            pnl::CDenseMatrix<float> *mat = dynamic_cast<WTabularDistribFun*>(pWDF)->Matrix(0);
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
