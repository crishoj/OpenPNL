/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlCreateStaticModel.cpp                                    //
//                                                                         //
//  Purpose:   Implementation of the algorithm                             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlMRF2.hpp"
#include "pnlBNet.hpp"

PNL_USING

CStaticGraphicalModel* pnlCreateStaticModel( EModelTypes modelType,
                                             int numberOfNodes,
                                             int numberOfNodeTypes,
                                             const CNodeType *nodeTypes,
                                             const int *nodeAssociation,
                                             CGraph *pGraph )
{
	switch ( modelType )
	{
	case mtMNet: 
		CMNet *mn;
		mn = new CMNet( modelType, numberOfNodes, numberOfNodeTypes,
			nodeTypes, nodeAssociation, pGraph );
		return mn;
	case mtMRF2:
		CMRF2 *mr;
		mr = new CMRF2( modelType, numberOfNodes, numberOfNodeTypes,
			nodeTypes, nodeAssociation, pGraph );
		return mr;
	case mtBNet:
		CBNet *bn;
		bn = new CBNet( modelType, numberOfNodes, numberOfNodeTypes,
			nodeTypes, nodeAssociation, pGraph );
		return bn;
	default: return 0;
	}
}

CStaticGraphicalModel* pnlCreateStaticModel( EModelTypes modelType,
								    int numberOfNodes,
								    int numberOfNodeTypes,
									const CNodeType *nodeTypes, 
								    const int *nodeAssociation,
									int numberOfCliques,
								    const int *cliqueSizes,
									int *cliques[] )
{
	switch ( modelType )
	{
	case mtMNet:
		CMNet *mn;
		mn = new CMNet( modelType, numberOfNodes, numberOfNodeTypes,
			nodeTypes, nodeAssociation, numberOfCliques, cliqueSizes,
			cliques );
		return mn;
	case mtMRF2:
		CMRF2 *mr;
		mr = new CMRF2( modelType, numberOfNodes, numberOfNodeTypes,
			nodeTypes, nodeAssociation, numberOfCliques, cliqueSizes,
			cliques );
		return mr;
	default: return 0;
	}
}
