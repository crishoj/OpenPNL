/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlBicLearningEngine.cpp                                    //
//                                                                         //
//  Purpose:   Implementation of the structure learning algorithm for      //
//             BNet that uses Bayesian information criterion               //
//                                                                         //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlCPD.hpp"
#include "pnlBNet.hpp"
#include "pnlBicLearningEngine.hpp"
#include "pnlStaticLearningEngine.hpp"

#include "pnlEmLearningEngine.hpp"
#include <math.h>

#include "pnlExampleModels.hpp"

PNL_USING

CBICLearningEngine::CBICLearningEngine( CStaticGraphicalModel *pGrModel, ELearningTypes LearnType):
CStaticLearningEngine(pGrModel, LearnType), m_pResultGrModel(NULL), m_resultRenaming(0)
{
    
    
}

CBICLearningEngine::~CBICLearningEngine()
{
    delete m_pResultGrModel;
}


CBICLearningEngine * CBICLearningEngine::
Create( CStaticGraphicalModel *pGrModel)
{

    if(!IsInputModelValid(pGrModel))
    {
	 PNL_THROW( CInconsistentType, " input model is invalid " );
    }
    CBICLearningEngine *pNewEngine =
	new CBICLearningEngine( pGrModel, itParamLearnEM );
    return pNewEngine;
}


const CStaticGraphicalModel* CBICLearningEngine::GetGraphicalModel() const
{
    return m_pResultGrModel;
}



void CBICLearningEngine::Learn()
{
    CEMLearningEngine *pLearn = NULL;

    float resultBIC = -FLT_MAX;
    CBNet *pResultBNet = NULL;
    intVector resultOrder;
    
    
    pEvidencesVector pEv(m_Vector_pEvidences.size(), NULL );
    
    CModelDomain *pMD = m_pGrModel->GetModelDomain();
    
    int nnodes = m_pGrModel->GetNumberOfNodes();
    
    nodeTypeVector varTypes;
    pMD->GetVariableTypes(&varTypes);

    intVector varAss( pMD->GetVariableAssociations(), pMD->GetVariableAssociations() + nnodes );
       
    intVector currentAssociation(nnodes);
    intVector currentObsNodes(nnodes);
    int i;
    for( i = 0; i < nnodes; i++ )
    {
	currentObsNodes[i] = i;
    }

    CGraph *pGraph = CGraph::Create(nnodes, NULL, NULL, NULL);
    CBNet *pBNet;
    int lineSz = int( nnodes * ( nnodes - 1 ) / 2 );
    intVecVector connect;
    intVector indexes(lineSz, 0);
    int startNode, endNode;
    int ind;
    for( ind = 0; ind < lineSz ; )
    {
	if( indexes[ind] == 1 )
	{
	    FindNodesByNumber(&startNode, &endNode, nnodes, ind);
	    pGraph->RemoveEdge(startNode, endNode );
	    indexes[ind] = 0;
	    ind++;
	}
	else
	{
	    FindNodesByNumber(&startNode, &endNode, nnodes, ind);
	    pGraph->AddEdge(startNode, endNode, 1 );
	    indexes[ind] = 1;
	    ind = 0;
	    connect.clear();
	    pGraph->GetConnectivityComponents(&connect);
	    if( connect.size() == 1 )
	    {
		
		do
		{
		    CGraph *pCopyGraph = CGraph::Copy(pGraph);
		    int j;
		    for( j = 0; j < nnodes; j++ )
		    {
			currentAssociation[j] = varAss[currentObsNodes[j]];
		    }
		    
		    pBNet = CBNet::Create(nnodes, varTypes, currentAssociation, pCopyGraph);
		    pBNet->AllocFactors();
		    for( j = 0; j < nnodes; j++ )
		    {
			pBNet->AllocFactor( j );
			pBNet->GetFactor(j)->CreateAllNecessaryMatrices();
		    }

		    int dimOfModel = DimOfModel(pBNet);
		    int k;
		    for( k = 0; k < pEv.size(); k++ )
		    {
			valueVector vls; 
			m_Vector_pEvidences[k]->GetRawData(&vls);
			pEv[k] = CEvidence::Create( pBNet->GetModelDomain(),currentObsNodes, vls );
		    }
		    
		    
		    pLearn = CEMLearningEngine::Create(pBNet);
		    pLearn->SetData(pEv.size(), &pEv.front());
		    pLearn->Learn();
		    int nsteps;
		    const float *score;
		    pLearn->GetCriterionValue(&nsteps, &score);
		    float log_lik = score[nsteps-1];
		    float BIC = log_lik - 0.5f*float( dimOfModel*log(float(pEv.size())) );
		    
		    if( BIC >= resultBIC )
		    {
			delete pResultBNet;
			resultBIC = BIC;
			m_critValue.push_back(BIC);
			pResultBNet = pBNet;
			resultOrder.assign( currentObsNodes.begin(), currentObsNodes.end() );
		    }
		    else
		    {
			delete pBNet;
		    }
		    for( k = 0; k < pEv.size(); k++ )
		    {
			delete pEv[k];
		    }

		    delete pLearn;
		}while(std::next_permutation(currentObsNodes.begin(), currentObsNodes.end()));
		
	    }
	    
	}
    }
    
    delete pGraph;
    m_pResultGrModel = pResultBNet;
    m_resultRenaming.assign(resultOrder.begin(), resultOrder.end());
    
}


int CBICLearningEngine::DimOfModel(const CStaticGraphicalModel *pModel)
{
/*
compute dimension of the model in (d)
it using in BIC criterion:
BIC = LogLic - 0.5*d*log(N)
    */
    int nParam = pModel->GetNumberOfFactors();
    CFactor *param = NULL;
    int dimOfModel = 0;
    int dim = 1;
    CMatrix<float> *matrix;;
    for (int  i = 0; i < nParam; i++)
    {
	dim = 1;
	param = pModel->GetFactor(i);
	switch (param->GetFactorType())
	{
	case ftCPD:
	    {
		switch (param->GetDistributionType())
		{
		case dtTabular:
		    {

			matrix = param->GetMatrix(matTable);
			int size;
			const int *ranges;
			static_cast<CNumericDenseMatrix<float>*>(matrix)->
			    GetRanges(&size, &ranges);
			for(int j=0; j < size - 1; j++)
			{
			    dim *= ranges[j];

			}
			dim *= ranges[size-1]-1;
			break;

		    }//case dtTabular
		case dtGaussian:
		    {
			PNL_THROW(CNotImplemented,"Gaussian")
			    break;
		    }//case dtGaussian
		case dtCondGaussian:
		    {
			PNL_THROW(CNotImplemented,"CondGaussian")
			    break;
		    }//case dtCondGaussian
		default:
		    {
			PNL_THROW(CBadConst,"distribution type")
			    break;
		    }
		}//swith(param->GetFactorType)
		break;
	    }//end case ftCPD
	case ftPotential:
	    {
		PNL_THROW(CNotImplemented,"Factor")
		    break;
	    }
	default:
	    {
		PNL_THROW(CBadConst,"FactorType")
		    break;
	    }
	}//end switch(param->GetFactor)

	dimOfModel += dim;
    }//end for(i)
    return dimOfModel;
}



inline int CBICLearningEngine::Alpha_i(int nNodes, int line)
{
    return line*(nNodes - 1) - line * (line - 1) / 2;
}

void CBICLearningEngine::FindNodesByNumber(int *startNode, int *endNode,
					    int nNodes, int linearNumber )
{
    int alpha_i = 0;
    int beta_i = 0;
    (*startNode) = -1;
    do
    {
	(*startNode)++;
	alpha_i = Alpha_i(nNodes, (*startNode) );
	beta_i = Alpha_i(nNodes, (*startNode) + 1) - 1;

    }while ((linearNumber < alpha_i) || (linearNumber > beta_i));
    (*endNode) = nNodes - beta_i + linearNumber - 1;
}

bool CBICLearningEngine::IsInputModelValid(const CStaticGraphicalModel *pGrModel)
{
    if( pGrModel->GetModelType() != mtBNet )
    {
        return false;
    }
    
    const CModelDomain * pMD = pGrModel->GetModelDomain();
    nodeTypeVector varTypes;
    pMD->GetVariableTypes(&varTypes);
    int i;
    for( i = 0; i < varTypes.size(); i++ )
    {
        if( !varTypes[i].IsDiscrete() )
        {
            return false;
        }
    }
    return true;
}

#ifdef PNL_RTTI
const CPNLType CBICLearningEngine::m_TypeInfo = CPNLType("CBICLearningEngine", &(CStaticLearningEngine::m_TypeInfo));

#endif