/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnl2TPFInferenceEngine.cpp                                  //
//                                                                         //
//  Purpose:   C2TPFInfEngine class member functions implementation        //
//                                                                         //
//             PF means Particle filtering                                 //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#include "pnlConfig.hpp"
#include "pnl2TPFInferenceEngine.hpp"
#include "pnlDistribFun.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlGaussianCPD.hpp"
#include "pnlMixtureGaussianCPD.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlScalarPotential.hpp"


PNL_USING

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
C2TPFInfEngine::C2TPFInfEngine(const CDynamicGraphicalModel *pGraphicalModel, int nParticleCount)
:C2TBNInfEngine( pGraphicalModel )
{	
	// init variables
	m_currentEvVec.clear();
	m_pQueryJPD = NULL;
	m_pEvidenceMPE = NULL;
	m_Slices = -1;

	m_pGrModel = pGraphicalModel;

	// set particle count and number of effective particles
	int nLowThreshold = 30;
	SetParemeter( nParticleCount, nLowThreshold);
	
	// get Slice0Nodes info
	m_Slice0Nodes = m_pGrModel->GetNumberOfNodes();
	m_Slice0ValuesSize = 0;
	m_Slice0NodeSize.resize(m_Slice0Nodes);
	
	for(int i = 0; i < m_Slice0Nodes; i++)
	{
		const CNodeType* pNodeType = m_pGrModel->GetNodeType(i);
		m_Slice0NodeSize[i] = pNodeType->GetNodeSize();
		
		if(pNodeType->IsDiscrete())
			m_Slice0ValuesSize++;
		else
			m_Slice0ValuesSize += pNodeType->GetNodeSize();			
	}
}

C2TPFInfEngine::~C2TPFInfEngine()
{
	if(m_pQueryJPD)   delete m_pQueryJPD;
        
    for(int i = 0; i < m_currentEvVec.size(); i++ )
    {
		if(m_currentEvVec[i])  delete m_currentEvVec[i];
		if(m_EvVecBackup[i])   delete m_EvVecBackup[i];
    }

	m_currentEvVec.clear();
	m_EvVecBackup.clear();

	m_particleWeight.clear();
	m_particleWeightBack.clear();
	m_CumulativeWeight.clear();

	m_Slice0NodeSize.clear();
}

C2TPFInfEngine* C2TPFInfEngine::Create(const CDynamicGraphicalModel *pGrModelIn, int nParticleCount )
{
	C2TPFInfEngine *pPFInfEngine = new C2TPFInfEngine( pGrModelIn, nParticleCount);

	return pPFInfEngine;
}

void C2TPFInfEngine::SetParemeter( int nParticleCount, int nLowThreshold)
{
   	m_particleCount = nParticleCount;  // particle count
	m_LowThreshold  = nLowThreshold;   // samllest number of effective particles

	// Re-intit PF Inference Engine
	m_particleWeight.resize(nParticleCount, 1);
	m_particleWeightBack.resize(nParticleCount, 1);
    m_CumulativeWeight.resize(nParticleCount+1, 0);
	
    for(int i = 0; i < m_currentEvVec.size(); i++ )
    {
		if(m_currentEvVec[i])  delete m_currentEvVec[i];
		if(m_EvVecBackup[i])   delete m_EvVecBackup[i];
    }

	m_currentEvVec.clear();
	m_EvVecBackup.clear();


	m_Slices = -1;
	m_bNormalized = false;
	m_bInitalized = false;
}

void C2TPFInfEngine::ForwardFirst(const CEvidence *pEvidenceIn, int maximize)
{
	InitSlice0Particles(pEvidenceIn);
}

//forward step fot time > 0
void C2TPFInfEngine::Forward(const CEvidence *pEvidenceIn, int maximize)
{
	EnterEvidence(pEvidenceIn);
}

//first backward step after last forward step
void C2TPFInfEngine::BackwardT()
{
	// not implement
}

void C2TPFInfEngine::Backward( int maximize)
{
	// not implement
}

//Backward step for fixed-lag smoothing procedure
void C2TPFInfEngine::BackwardFixLag()
{
	// not implement
}


void C2TPFInfEngine::InitSlice0Particles(const CEvidence *pEvidenceIn)
{
	// clear existed particles
	int i, j;
	for(i = 0; i < m_currentEvVec.size(); i++ )
    {
		if(m_currentEvVec[i])  delete m_currentEvVec[i];
		if(m_EvVecBackup[i])   delete m_EvVecBackup[i];
    }

	m_currentEvVec.clear();
	m_EvVecBackup.clear();


	//Create default slice0 evidence
	pEvidencesVector        EvVec;
	const CBNet *pBNet = static_cast<const CBNet *>( m_pGrModel->GetStaticModel ()  );

	pBNet->GenerateSamples(&m_currentEvVec, m_particleCount, pEvidenceIn);

	//hiden slice1 of the BNet
	for( i = 0; i< m_particleCount; i++)
	{
		CEvidence* pEvid = m_currentEvVec[i];

		for( j = 0; j < m_Slice0Nodes; j++)
		{
			if(!pEvid->IsNodeObserved(j))
				pEvid->MakeNodeObserved(j);
			
			if(pEvid->IsNodeObserved(j + m_Slice0Nodes))
				pEvid->MakeNodeHidden(j + m_Slice0Nodes);
		}
	}		

	int nnodes = m_Slice0Nodes*2;
	intVector obsNodes(nnodes);
	valueVector obsVals( m_Slice0ValuesSize*2, (Value)0 );
	for ( i = 0; i < nnodes; i ++ )   obsNodes[i] = i;
    
	m_EvVecBackup.resize(m_particleCount);

	// init particleWeight and CumulativeWeight
	for(i = 0; i < m_particleCount; i++)
	{
 		m_EvVecBackup[i] = CEvidence::Create(pBNet, obsNodes, obsVals);
		
		m_particleWeight[i] = 1;
		m_CumulativeWeight[i+1] = m_CumulativeWeight[i] + m_particleWeight[i]; 
	}

	m_Slices = 0;
	m_bNormalized = false;
	m_bInitalized = true;

}

pEvidencesVector* C2TPFInfEngine::GetCurSamples()
{
	return &m_currentEvVec;
}


floatVector* C2TPFInfEngine::GetParticleWeights()
{
	return &m_particleWeight;
}

void C2TPFInfEngine::NormalizeWeight()
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

void  C2TPFInfEngine::EnterEvidence(const CEvidence* const* evidenceIn, int nSlices)
{
	if( nSlices >= 1)
	{
		for(int i = 0; i < nSlices; i++)
			EnterEvidence(evidenceIn[i]);
	}
	else
		EnterEvidence(evidenceIn[0]);
}

// In DBN, the nodenumber is always in (0,1, .. m_Slice0Nodes)
//Enter evidence and generate particles
void  C2TPFInfEngine::EnterEvidence(const CEvidence* pEvidenceIn)
{
    PNL_CHECK_IS_NULL_POINTER(pEvidenceIn);
    
    LWSampling( pEvidenceIn );

	//Calculate particle weight by CPD given evidencs
	int i, j;
	int iSampleSize = m_currentEvVec.size();
	const int* ObsNodes = pEvidenceIn->GetAllObsNodes();
	int NumberObsNodes  = pEvidenceIn->GetNumberObsNodes();
	
	float w;
	CFactor* pFactor;

	for( i = 0; i < iSampleSize; i++)
	{
		w = 0;
		for( j = 0; j < NumberObsNodes; j++)
		{
			if(pEvidenceIn->IsNodeObserved(ObsNodes[j]))
			{
				//CFactor* pFactor = m_pGrModel->GetFactor(ObsNodes[j]);
				if(m_Slices == 0)
					pFactor = m_pGrModel->GetFactor(ObsNodes[j]);
				else
					pFactor = m_pGrModel->GetFactor(ObsNodes[j]+m_Slice0Nodes);

		 		w = w +  pFactor->GetLogLik( m_currentEvVec[i]); 
			}
		}

		m_particleWeight[i] = m_particleWeight[i] * (float)exp(w);
		m_CumulativeWeight[i+1] = m_CumulativeWeight[i] + m_particleWeight[i]; 
	}

	NormalizeWeight();
}

void C2TPFInfEngine::EnterEvidenceProbability( floatVector *pEvidenceProbIn )
{
	PNL_CHECK_IS_NULL_POINTER(pEvidenceProbIn);
	
	int i;
	int iSamples = pEvidenceProbIn->size();
	int iSampleSize = m_currentEvVec.size();

	if(iSamples != iSampleSize) return;

	for( i = 0; i < iSamples; i++)
	{
		m_particleWeight[i] = m_particleWeight[i] * (*pEvidenceProbIn)[i];
		m_CumulativeWeight[i+1] = m_CumulativeWeight[i] + m_particleWeight[i]; 
	}
	
	NormalizeWeight();
}

void C2TPFInfEngine::LWSampling(const CEvidence* pEvidenceIn )
{
	//init the first slice0
	if( m_bInitalized == false || m_currentEvVec.size() == 0) 
	{
		InitSlice0Particles(pEvidenceIn);
		m_bNormalized = false;
		return;
	}
	else if(m_Slices == 0) //for the slice 1
	{
		m_Slices++;
		for(int i = 0; i < m_particleCount; i++)
			GenerateSample(m_currentEvVec[i], pEvidenceIn);

		m_bNormalized = false;
		return;
	}
	else  // for the following slice 2,3....
	{	
		m_Slices++;
		
		int i, j, k;
		
		const CBNet* pBNet = static_cast<const CBNet *>( m_pGrModel->GetStaticModel ()  );
		float fNeff = GetNeff();   //number of effecitve particles
		
		if(fNeff < m_LowThreshold) //resampling if fNeff is small
		{
			// re-initialize particles
			for(i = 0; i < m_particleCount; i++)
			{
				//InitParticle(m_currentEvVec[i], m_PredicationX);
								
				m_particleWeight[i] = 1;
				m_CumulativeWeight[i+1] = m_CumulativeWeight[i] + m_particleWeight[i]; 
			}
		}
	
		// generate slice t of samples given slice t-1 of selected particles
		CEvidence* pEvidence;
		CEvidence* pSampEvidence;
	
		for(i = 0; i < m_particleCount; i++)
		{
			pEvidence = Sub2Find( &k );
			pSampEvidence = m_EvVecBackup[i];

			//copy slice1 of pEvidence to slice0 of pSampEvidence (m_EvVecBackup[i])
			for( j = 0; j < m_Slice0Nodes; j++)
			{
				if(!pSampEvidence->IsNodeObserved(j))
					pSampEvidence->MakeNodeObserved(j);
				
				if(pSampEvidence->IsNodeObserved(j + m_Slice0Nodes))
					pSampEvidence->MakeNodeHidden(j + m_Slice0Nodes);

				const CNodeType* pNodeType = pBNet->GetNodeType(j);
				if(pNodeType->IsDiscrete())
					memcpy(pSampEvidence->GetValue(j), pEvidence->GetValue(m_Slice0Nodes+j), sizeof(Value));
				else
				{
					int  nodesz =    pNodeType->GetNodeSize();
					memcpy(pSampEvidence->GetValue(j), pEvidence->GetValue(m_Slice0Nodes+j), sizeof(Value)*nodesz);
				}
			}
			
			GenerateSample(pSampEvidence, pEvidenceIn);

			// backup particle weights
			m_particleWeightBack[i] = m_particleWeight[k];
		}

		// restore particle weights and evidences
		for(i = 0; i < m_particleCount; i++)
		{
			m_particleWeight[i] = m_particleWeightBack[i];

			// exchange evidences between m_currentEvVec[i] and m_EvVecBackup[i]
			pSampEvidence = m_currentEvVec[i];
			m_currentEvVec[i] =	m_EvVecBackup[i];
			m_EvVecBackup[i]  = pSampEvidence;
		}
	}

	m_bNormalized = false;
}

// Give observed evidIn, generate a sample "evidence" 
// The function is similar to CBNet::GenerateSamples(), but specially used for DBN C2TPFInfEngine. 
// The difference is that
//     C2TPFInfEngine::GenerateSample() assigan values to the outputsample "evidence", rather than creating a new evidence 
//     For the observed nodes of slice1, it overwrite old values using evidIn
void C2TPFInfEngine::GenerateSample(CEvidence* evidence, const CEvidence *pEvIn) 
{
    const CBNet* selfBNet = static_cast<const CBNet *>( m_pGrModel->GetStaticModel ()  );
    int nnodes = selfBNet->GetNumberOfNodes();

	int i;
	intVector obsNodes;

    const CNodeType * nt;
    intVector mixtureNodes;
    const CFactor *pCPD;
	
	// find mixture gaussians
    for( i = m_Slice0Nodes; i < nnodes; i++)
    {
        pCPD = selfBNet->GetFactor(i);
        if( pCPD->GetDistributionType() != dtMixGaussian )
        	obsNodes.push_back(i);
    }

	int node;
    intVector nodesForSampling;
    nodesForSampling.reserve(obsNodes.size());

    if(pEvIn)
    {
		for ( i = 0; i < obsNodes.size(); i++  )
        {
			node = obsNodes[i] - m_Slice0Nodes;
			//node = obsNodes[i];
			
            if( pEvIn->IsNodeObserved( node ) )
            {
				if(!evidence->IsNodeObserved(obsNodes[i]))
					evidence->MakeNodeObserved(obsNodes[i]);
				
				nt = selfBNet->GetNodeType(node);
				if(nt->IsDiscrete())
					memcpy(evidence->GetValue(obsNodes[i]), pEvIn->GetValue(node), sizeof(Value));
				else
				{
					int  nodesz =    nt->GetNodeSize();
					memcpy(evidence->GetValue(obsNodes[i]), pEvIn->GetValue(node), sizeof(Value)*nodesz);
				}
            }
            else
            {
                nodesForSampling.push_back(obsNodes[i]);
            }
        }
    }
    else
    {
        nodesForSampling = obsNodes;
    }

    for ( i = 0; i < nodesForSampling.size(); i++ )
    {
        selfBNet->GetFactor( nodesForSampling[i] )->GenerateSample( evidence );
    }
}

CEvidence* C2TPFInfEngine::Sub2Find(int *index)
{
	int low = 0; 
	int high = m_particleCount;
	int mid = (low + high) / 2;

	float fLargestAccumWeight = m_CumulativeWeight[m_particleCount];

	float fAccumWeight = fLargestAccumWeight * rand() / RAND_MAX;
	
	while(high > low + 1)
	{
		if(fAccumWeight > m_CumulativeWeight[mid])
			low = mid;
		else
			high =mid;

		mid = (low + high) / 2;
	}
	
	if(index) *index = low;

	return m_currentEvVec[low];
}

float C2TPFInfEngine::GetNeff()
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

void C2TPFInfEngine::Estimate(CEvidence *pEstimate)
{
	PNL_CHECK_IS_NULL_POINTER(pEstimate);

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

void  C2TPFInfEngine::MarginalNodes( const int *queryIn, int querySz, int time, int notExpandJPD)
{
	PNL_CHECK_IS_NULL_POINTER(queryIn);

	if( m_bNormalized == false ) NormalizeWeight();

	int i, j, k;
	int offset;
	int nsamples = m_particleCount;
    	
	const CBNet *pBNet = static_cast<const CBNet *>( m_pGrModel->GetStaticModel () );
	int type = 0;
	int totalnodesizes = 0;
	intVector nodesize(querySz);
	intVector mulnodesize(querySz,1);
	
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

		if(m_pQueryJPD)   delete m_pQueryJPD;
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
			
			for(j = 0, offset = 0; j < querySz; j++)
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

		if(m_pQueryJPD)   delete m_pQueryJPD;
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


const CPotential* C2TPFInfEngine::GetQueryJPD() 
{
	return m_pQueryJPD;
}

const CEvidence*  C2TPFInfEngine::GetMPE()
{
	if( !m_pEvidenceMPE )
    {
        PNL_THROW( CInvalidOperation,
            " can't call GetMPE() before calling MarginalNodes() " );
    }
    
    return m_pEvidenceMPE;
}


//always is ptFiltering
void C2TPFInfEngine::
DefineProcedure( EProcedureTypes procedureType, int lag )
{
	/////////////////////////////////////////////////////////////////////////
	// Selection procedure (smoothing, filtering, ...
	/////////////////////////////////////////////////////////////////////////
	
	// initialization procedure (i.e. lag, ring of jtree inferences)
	
	switch( procedureType )
	{
	    case ptFiltering:
		{
			PNL_CHECK_FOR_NON_ZERO( lag );
			m_Lag = 0;
			m_ProcedureType = ptFiltering;
			break;
		}
	default:
		{
			PNL_THROW( CInconsistentType, "incorrect type of inference procedure" );
		}
	}

	int rg;
    for( rg = 0; rg < m_CRingpEv.size(); rg++ )
	{
		delete m_CRingpEv[rg];
	}
	
	m_CRingpEv.resize(0);
}

#ifdef PNL_RTTI
const CPNLType C2TPFInfEngine::m_TypeInfo = CPNLType("C2TPFInfEngine", &(C2TBNInfEngine::m_TypeInfo));

#endif
//////////////////////////////////////////////////////////////////////////////////////////

























