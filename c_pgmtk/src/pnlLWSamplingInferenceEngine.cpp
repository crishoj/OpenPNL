/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlLWSamplingInferenceEngine.cpp                            //
//                                                                         //
//  Purpose:   CLWSamplingInfEngine class member functions implementation  //
//                                                                         //
//             LW means Likelyhood weighting                               //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#include "pnlConfig.hpp"
#include "pnlNaiveInferenceEngine.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlGaussianDistribFun.hpp"
#include "pnlGaussianCPD.hpp"
#include "pnlMixtureGaussianCPD.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlCPD.hpp"
#include "pnlBNet.hpp"
#include "pnlMNet.hpp"
#include "pnlLWSamplingInferenceEngine.hpp"
#include "pnlException.hpp"


PNL_USING

CLWSamplingInfEngine * CLWSamplingInfEngine::Create( 
							const CStaticGraphicalModel *pGraphicalModel, int particleCount )
{
	if( !pGraphicalModel )
	{
		PNL_THROW( CNULLPointer, "graphical model" );
		return NULL;
	}
	else
	{
		CLWSamplingInfEngine* newInf = new CLWSamplingInfEngine( pGraphicalModel, particleCount);
		return newInf;
	}
}

CLWSamplingInfEngine::CLWSamplingInfEngine( const CStaticGraphicalModel *pGraphicalModel, int particleCount )
:CInfEngine( itLWSampling, pGraphicalModel )
{
	m_currentEvVec.clear();
	m_pEvidenceMPE = NULL;
	m_pQueryJPD = NULL;

	m_pGraphicalModel = pGraphicalModel;
	SetParemeter( particleCount );
}

CLWSamplingInfEngine::~CLWSamplingInfEngine()
{
  
    delete m_pQueryJPD;
    m_pQueryJPD = NULL;
    
	int i;
    for( i = 0; i < m_currentEvVec.size(); i++ )
    {
        delete m_currentEvVec[i];
    }

	m_currentEvVec.clear();
	m_particleWeight.clear();
}

void CLWSamplingInfEngine::SetParemeter( int particleCount )
{
   	m_particleCount = particleCount;
	
	m_particleWeight.resize(particleCount, 1);

	m_bNormalized = false;
}


pEvidencesVector* CLWSamplingInfEngine::GetCurSamples()
{
	return &m_currentEvVec;
}

floatVector* CLWSamplingInfEngine::GetParticleWeights()
{
	return &m_particleWeight;
}

void CLWSamplingInfEngine::NormalizeWeight()
{
	int i;
	float totalWeight = 0;
	for(i = 0; i < m_particleCount; i++)
	{
		totalWeight += m_particleWeight[i];
	}

	if(totalWeight <= 0)
	{
		for(i = 0; i < m_particleCount; i++)
			m_particleWeight[i] = 1.0f / m_particleCount;
	}
	else
	{
		for(i = 0; i < m_particleCount; i++)
		{
			m_particleWeight[i] /= totalWeight;
		}
	}

	m_bNormalized = true;
}

float CLWSamplingInfEngine::GetNeff()
{
	if( m_bNormalized == false) NormalizeWeight();

	int i;
	float totalWeight2 = 0;
	for(i = 0; i < m_particleCount; i++)
	{
		totalWeight2 += m_particleWeight[i]*m_particleWeight[i];
	}

	if(totalWeight2 <= 0) return -1;

	return 1/totalWeight2;
}

void CLWSamplingInfEngine::
EnterEvidence( const CEvidence *pEvidenceIn , int maximize , int sumOnMixtureNode  )
{
    PNL_CHECK_IS_NULL_POINTER(pEvidenceIn);
  
    LWSampling(pEvidenceIn);

	// Given evidencs, calculate particle weight by CPD 
	float w;
	int iSampleSize = m_currentEvVec.size();
	const int* ObsNodes = pEvidenceIn->GetAllObsNodes();
	int NumberObsNodes  = pEvidenceIn->GetNumberObsNodes();

	int i, j;

	for( i = 0; i < iSampleSize; i++)
	{
		w = 0;
		for( j = 0; j < NumberObsNodes; j++)
		{
			if(pEvidenceIn->IsNodeObserved(ObsNodes[j]))
			{
				CFactor* pFactor = m_pGraphicalModel->GetFactor(ObsNodes[j]);
		 		w = w +  pFactor->GetLogLik( m_currentEvVec[i]); 
			}
		}

		m_particleWeight[i] = (float)exp(w);
	}

	NormalizeWeight();
}

void CLWSamplingInfEngine::
EnterEvidenceProbability( floatVecVector *pEvidenceProbIn )
{
	PNL_CHECK_IS_NULL_POINTER(pEvidenceProbIn);

	int i, j;
	float w;
	int iSamples = pEvidenceProbIn->size();
	int iSampleSize = m_currentEvVec.size();
	
	if(iSamples != iSampleSize) return;

	for( i = 0; i < iSamples; i++)
	{
		w = 1;
			
		for( j = 0; j < pEvidenceProbIn[i].size(); j++)
		{
		 	w = w * (*pEvidenceProbIn)[i][j];
		}

		m_particleWeight[i] = w;
	}
	
	NormalizeWeight();
}


void CLWSamplingInfEngine::
LWSampling(const CEvidence* pEvidenceIn)
{
	for(int i = 0; i < m_currentEvVec.size(); i++ )
    {
        delete m_currentEvVec[i];
    }

	m_currentEvVec.clear();

	m_pEvidence = pEvidenceIn;

    const CBNet *pBNet = static_cast<const CBNet *>( m_pGraphicalModel );
	
	pBNet->GenerateSamples(&m_currentEvVec, m_particleCount, m_pEvidence);

	m_bNormalized = false;
	m_pEvidence   = NULL;
}


void CLWSamplingInfEngine::Estimate(CEvidence * pEstimate)
{
	if( pEstimate == NULL ) return;
	if( m_bNormalized == false ) NormalizeWeight();

	int i, j, k, offset;

	int  iNodes   = pEstimate->GetNumberObsNodes ();
	const int* obsNodes = pEstimate->GetAllObsNodes();
	const CNodeType *const* pNodeTypes = pEstimate->GetNodeTypes();
	
	int totalNodeSize =0;
	intVector nodeSize(iNodes);
	for( j = 0, totalNodeSize = 0; j < iNodes; j++)
	{
		nodeSize[j] = pNodeTypes[j]->GetNodeSize();
		totalNodeSize += nodeSize[j];
	}
	
	floatVector pFloatValue;
	pFloatValue.resize(totalNodeSize, 0);
		
	for( j = 0, offset = 0; j < iNodes; j++)
	{
		if( pNodeTypes[j]->IsDiscrete())  
		{
			for( i = 0; i < m_particleCount; i++)
			{
				Value* pValue = (m_currentEvVec[i])->GetValue(obsNodes[j]);

				// for discrete value type
				pFloatValue[offset + pValue->GetInt()] = pFloatValue[offset + pValue->GetInt()] + m_particleWeight[i];
			}
		}
		else
		{
			for( i = 0; i < m_particleCount; i++)
			{
				Value* pValue = (m_currentEvVec[i])->GetValue(obsNodes[j]);

				// for continuous value type
				for( k =0; k < nodeSize[j]; k++)
				{
					pFloatValue[offset + k] = pFloatValue[offset + k] + m_particleWeight[i] * (pValue+k)->GetFlt();
				}
			}
		}
		offset += nodeSize[j];
	}

	// set pFloatValue data to pEstimate
	for( j = 0, offset = 0; j < iNodes; j++)
	{
		Value* pValue = pEstimate->GetValue(obsNodes[j]);
		
		if( pNodeTypes[j]->IsDiscrete())  
		{
			float maxweight = -99; int itmax = 0;
			for( k =0; k < nodeSize[j]; k++)
			{
				if(maxweight < pFloatValue[offset + k])
				{
					maxweight = pFloatValue[offset + k];
					itmax = k;
				}
				pValue->SetInt(itmax);
			}
		}
		else
		{
			for( k =0; k < nodeSize[j]; k++)
				(pValue+k)->SetFlt( pFloatValue[offset + k] );
		}

		offset += nodeSize[j];
	}
}

void CLWSamplingInfEngine::
MarginalNodes( const int *queryIn, int querySz, int notExpandJPD)
{
	PNL_CHECK_IS_NULL_POINTER(queryIn);

	if( m_bNormalized == false ) NormalizeWeight();

	int i, j, k;
	int offset;
	int nsamples = m_particleCount;
    	
	const CBNet *pBNet = static_cast<const CBNet *>( m_pGraphicalModel );
	int type = 0;
	int totalnodesizes = 0;
	intVector nodesize(querySz);
	intVector mulnodesize(querySz, 1); 
	
	for( i = querySz; --i >=0; )
	{
		const CNodeType* pNodeType = pBNet->GetNodeType(queryIn[i]);
		nodesize[i] = pNodeType->GetNodeSize();			
		
		if(i == querySz-1)	
			mulnodesize[i] = 1;
		else
			mulnodesize[i] *= nodesize[i+1];

		if(pNodeType->IsDiscrete())
		{
			type++;
			
			if(totalnodesizes == 0)
				totalnodesizes = nodesize[i];
			else
				totalnodesizes = totalnodesizes * nodesize[i]; 
		}
		else
		{
			totalnodesizes = totalnodesizes + nodesize[i]; 
		}
	}
	
	if(type == querySz)
	{
		//all query nodes are discrete
		float *tab = new float[totalnodesizes];
		for(i = 0; i < totalnodesizes; i++) tab[i] = 0;

		for( i = 0; i < nsamples; i++)
		{
			CEvidence* pEvidence = m_currentEvVec[i];
			
			int index = 0;
			for(j = 0; j < querySz; j++)
			{
				Value* pValue = pEvidence->GetValue(queryIn[j]);
				index += pValue->GetInt() * mulnodesize[j];
			}					
			
			tab[index] += m_particleWeight[i];
		}

		m_pQueryJPD = CTabularPotential::Create(queryIn, querySz, pBNet->GetModelDomain ()  );
		m_pQueryJPD->AllocMatrix( tab, matTable );

		delete []tab;
	}
	else if(type == 0)
	{
		//all query nodes are gaussian
		float* val  = new float[totalnodesizes];
		float* mean = new float[totalnodesizes];
		float* cov  = new float[totalnodesizes * totalnodesizes];
		for( i = 0; i < totalnodesizes; i++) mean[i] = 0;
		for( i = 0; i < totalnodesizes * totalnodesizes; i++) cov[i] = 0;

		// mean
		for( i = 0; i < nsamples; i++)
		{
			CEvidence* pEvidence = m_currentEvVec[i];
			
			for(j = 0, 	offset = 0; j < querySz; j++)
			{
				Value* pValue = pEvidence->GetValue(queryIn[j]);
				for(k = 0; k < nodesize[j]; k++)
				{
					mean[offset] += m_particleWeight[i] * pValue[k].GetFlt();
					offset++;
				}
			}
		}
		
		// covariance
		for( i = 0; i < nsamples; i++)
		{
			CEvidence* pEvidence = m_currentEvVec[i];
			
			for(j = 0, 	offset = 0; j < querySz; j++)
			{
				Value* pValue = pEvidence->GetValue(queryIn[j]);
				for(k = 0; k < nodesize[j]; k++)
				{
					val[offset] = pValue[k].GetFlt();
					offset++;
				}
			}
			
			for(j = 0; j < totalnodesizes; j++)
			{
				for(k = j; k < totalnodesizes; k++)
				{
					cov[k*totalnodesizes+j] += ( m_particleWeight[i] * ( val[j]- mean[j]) * (val[k] - mean[k]) );
					cov[j*totalnodesizes+k] = cov[k*totalnodesizes+j];
				}
			}
		}

		m_pQueryJPD = CGaussianPotential::Create( queryIn, querySz, pBNet->GetModelDomain () );
		m_pQueryJPD->AllocMatrix( mean, matMean );
		m_pQueryJPD->AllocMatrix( cov, matCovariance);

		delete []val;
		delete []mean;
		delete []cov;
	}

	//Get MPE
	delete m_pEvidenceMPE;
	m_pEvidenceMPE = NULL;

    m_pEvidenceMPE = m_pQueryJPD->GetMPE();
}

const CPotential* CLWSamplingInfEngine::GetQueryJPD() const
{
    if( !m_pQueryJPD )
    {
        PNL_THROW( CInvalidOperation,
            " can't call GetQueryJPD() before calling MarginalNodes() " );
    }
    
    return m_pQueryJPD;
}

const CEvidence* CLWSamplingInfEngine::GetMPE() const
{
	if( !m_pEvidenceMPE )
    {
        PNL_THROW( CInvalidOperation,
            " can't call GetMPE() before calling MarginalNodes() " );
    }
    
    return m_pEvidenceMPE;
}


#ifdef PNL_RTTI
const CPNLType CLWSamplingInfEngine::m_TypeInfo = CPNLType("CLWSamplingInfEngine", &(CInfEngine::m_TypeInfo));

#endif