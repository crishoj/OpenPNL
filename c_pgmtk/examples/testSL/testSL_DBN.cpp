/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AStructureLearning.cpp                                      //
//                                                                         //
//  Purpose:   Test on structure learning of BNet                          //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnl_dll.hpp"
#include "pnlMlStaticStructLearn.hpp"
#include "pnlMlStaticStructLearnHC.hpp"
#include "pnlMlDynamicStructLearn.hpp"

PNL_USING
int main()
{
    int nnodes = 16;
    int nodeslice = 8;
    CBNet* pKjaerulf = pnlExCreateKjaerulfsBNet();
    CDBN*  pKj = CDBN::Create(pKjaerulf);

    int nSeries = 50;
    int	nslices = 101;
    int i;

    intVector nS(nSeries);
    for(i = 0; i < nSeries; i++)
    {
        nS[i] = nslices;
    }

    valueVector vValues;
    vValues.resize(nodeslice);
    intVector obsNodes(nodeslice);
    for( i=0; i<nodeslice; i++)obsNodes[i] = i;
    CEvidence ***pEv;
    pEv = new CEvidence **[nSeries];

    int series, slice, node, val;

    FILE * fp;
    fp = fopen("../Data/kjaerulff.dat", "r");

    if( !fp )
    {
        std::cout<<"can't open cases file"<<std::endl;
        exit(1);
    }

    for( series = 0; series < nSeries; series++ )
    {
        pEv[series] = new CEvidence*[ nslices ];
        for( slice = 0;  slice < nslices; slice++ )
        {
            for( node = 0; node < nodeslice; node++)
            {
                fscanf(fp, "%d,", &val);
		vValues[node].SetFlt(val);
            }
            (pEv[series])[slice] = CEvidence::Create(pKj->GetModelDomain(), obsNodes,  vValues );
        }

    }
    fclose(fp);

    CGraph *pGraph = CGraph::Create(nnodes, NULL, NULL, NULL);
    for(i=0; i<nnodes-1; i++)
    {
	pGraph->AddEdge(i, i+1,1);
    }
    CNodeType *nodeTypes = new CNodeType[1];

    nodeTypes[0].SetType(1, 2);
    int *nodeAssociation = new int[nnodes];

    for ( i = 0; i < nnodes; i++ )
    {
	nodeAssociation[i] = 0;
    }
    CBNet *pBnet = CBNet::Create( nnodes, 1, nodeTypes, nodeAssociation, pGraph );
    pBnet -> AllocFactors();
    floatVector data;
    data.assign(64, 0.0f);

    for ( node = 0; node < nnodes; node++ )
    {
	pBnet->AllocFactor( node );
	(pBnet->GetFactor( node )) ->AllocMatrix( &data.front(), matTable );
    }

    CDBN*  pDBN = CDBN::Create(pBnet);

    CMlDynamicStructLearn *pLearn = CMlDynamicStructLearn::Create(pDBN, itDBNStructLearnML,
	StructLearnHC, BIC, 4, 1, 30);

    pLearn -> SetData(nSeries, &nS.front(), pEv);
    //	pLearn->SetLearnPriorSlice(true);
    //	pLearn->SetMinProgress((float)1e-4);
    pLearn ->Learn();
    const CDAG* pDAG = pLearn->GetResultDag();
    pDAG->Dump();
    ////////////////////////////////////////////////////////////////////////////
    delete pLearn;
    delete pDBN;
    delete pKj;
    for(series = 0; series < nSeries; series++ )
    {
        for( slice = 0;  slice < nslices; slice++ )
        {
            delete (pEv[series])[slice];
        }
	delete[] pEv[series];
    }
    delete[] pEv;
    return 0;
}
