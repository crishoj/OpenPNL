/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlCreateParameter.cpp                                      //
//                                                                         //
//  Purpose:   Implementation of the algorithm                             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//
#include "pnlConfig.hpp"
#include "pnlException.hpp"
#include "pnlTabularFactor.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlGaussianFactor.hpp"
#include "pnlGaussianCPD.hpp"

CFactor *pnlCreateFactor(EDistributionType dt,
								  EFactorType pt, 
								  const CNodeType *const *nt,
								  const int *domain, 
								  int nNodes)
{
/*nt - are node types of only the nodes from domain, 
their number is the same as nNodes, if you have GraphicalModel and domain, 
you can find it so: 
const CNodeType **myNodeTypes = new const CNodeType *[nNodes];
for(int i=0; i<nNodes; i++)
{
myNodeTypes[i] = GraphicalModel->GetNodeType(domain[i]);
}
after that you can call pnlCreateFactor with argument 3 myNodeTypes.
	*/
	if( nNodes<0 )
	{
		PNL_THROW( COutOfRange, "can't create factor with negative number of nodes" )//
			return NULL;
	}
	
	if( !nt )
	{
		PNL_THROW( CNULLPointer, "Input node types" )//no NodeTypes
			return NULL;
	}
	if( !domain )
	{
		PNL_THROW( CNULLPointer, "Input domain" )//no domain
			return NULL;
	}
	switch( pt )
	{
	case ptFactor:
		{
			switch ( dt )
			{
			case dtTabular:
				{
					int allNodesDiscrete = 1;
					for( int i=0; i<nNodes; i++ )
					{
						if( !( nt[i]->IsDiscrete() ) )
						{
							allNodesDiscrete = 0;
							break;
						}
					}
					if( !allNodesDiscrete )
					{
						PNL_THROW( CInconsistentType,
							"dtTabular - not all nodes discrete" )
							/*some nodes are continious - 
							it cannot be TabularFactor*/
							return NULL;
					}
					else
					{
						CTabularFactor *pNewParam = 
							new CTabularFactor(nt,
							domain, nNodes);
						return pNewParam;
					}
				}
			case dtGaussian:
				{
					int allNodesContinious = 1;
					for( int i=0; i<nNodes; i++ )
					{
						if(  nt[i]->IsDiscrete()  )
						{
							allNodesContinious = 0;
							break;
						}
					}
					if( !allNodesContinious )
					{
						PNL_THROW( CInconsistentType,
							"dtGaussian - not all nodes continious" );
							/*some nodes are continious - 
							it cannot be TabularFactor*/
							return NULL;
					}
					else
					{
						CGaussianFactor *pNewParam = new CGaussianFactor(nt,
							domain, nNodes);
						return pNewParam;
					}
				}
			default:
				{
					PNL_THROW( CBadConst, "no such distribution type" )
						//no competent type 
						return NULL;
				}
			}
		}
	case ptCPD:
		{
			switch (dt)
			{
			case dtTabular:
				{
					int allNodesDiscrete = 1;
					for(int i=0; i<nNodes; i++)
					{
						if(!(nt[i]->IsDiscrete()))
						{
							allNodesDiscrete = 0;
							break;
						}
					}
					if(!allNodesDiscrete)
					{
						PNL_THROW( CInconsistentType, 
							"dtTabular - not all nodes discrete" )
							/*some nodes are continious - 
							it cannot be TabularCPD*/
							return NULL;
					}
					else
					{
						CTabularCPD *pNewParam = new CTabularCPD(nt, 
							domain,	nNodes);
						return pNewParam;
					}
				}
			case dtGaussian:
				{
					int allNodesContinious = 1;
					for( int i=0; i<nNodes; i++ )
					{
						if(  nt[i]->IsDiscrete()  )
						{
							allNodesContinious = 0;
							break;
						}
					}
					if( !allNodesContinious )
					{
						PNL_THROW( CInconsistentType,
							"dtGaussian - not all nodes continious" )
							/*some nodes are continious - 
							it cannot be TabularFactor*/
							return NULL;
					}
					else
					{
						CGaussianCPD *pNewParam = new CGaussianCPD(nt,
							domain, nNodes);
						return pNewParam;
					}
				}
			case dtTrte:
				{
					CTreeCPD* pNewParam = new CTreeCPD(nt, domain, nNodes);
						return pNewParam;
				}
			default:
				{
					PNL_THROW( CBadConst, "no such distribution type" )
						return NULL;
				}
			}		
		}
		default:
			{
				PNL_THROW( CBadConst, "no such factor type" )
				return NULL;
				}
	}
}
