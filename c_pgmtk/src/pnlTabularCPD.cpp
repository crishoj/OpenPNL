/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlTabularCPD.cpp                                           //
//                                                                         //
//  Purpose:   CTabularCPD class member functions implementation           //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//TabularCPD.cpp
//////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"

#include <float.h>
#include <math.h>
#include <sstream>

#include "pnlTabularCPD.hpp"
#include "pnlDistribFun.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlInferenceEngine.hpp"


PNL_USING

CTabularCPD* CTabularCPD::Create( CModelDomain* pMD, const intVector& domainIn, 
	CMatrix<float>* matIn )
{
    
    
    CTabularCPD *pCPD =  CTabularCPD::Create( &domainIn.front(), domainIn.size(), pMD );
    if( matIn )
    {
	pCPD->AttachMatrix(matIn, matTable);
    }
    return pCPD;
}

CTabularCPD* CTabularCPD::Create( const intVector& domain, CModelDomain* pMD,
                                  const floatVector& data )
{
    const float *p = NULL;
    if( data.size() )
    {
	p = &data.front();
    }
    return CTabularCPD::Create( &domain.front(), domain.size(), pMD, p );
}
CTabularCPD* CTabularCPD::CreateUnitFunctionCPD( const intVector& domain,
                                                CModelDomain* pMD )
{
    return CTabularCPD::CreateUnitFunctionCPD( &domain.front(), domain.size(),
                                                pMD);
}

CTabularCPD* CTabularCPD::Create( const int *domain, int nNodes,
                                 CModelDomain* pMD,
                                 const float *data )
{
    PNL_CHECK_IS_NULL_POINTER( domain );
    PNL_CHECK_IS_NULL_POINTER( pMD );
    PNL_CHECK_LEFT_BORDER( nNodes, 1 );
    
    CTabularCPD *pNewParam = new CTabularCPD( domain, nNodes, pMD );
    PNL_CHECK_IF_MEMORY_ALLOCATED( pNewParam );
    if( data )
    {
        pNewParam->AllocMatrix( data, matTable );
    }
    return pNewParam;
}

CTabularCPD* CTabularCPD::CreateUnitFunctionCPD( const int *domain, int nNodes,
                                                CModelDomain* pMD)
{
    PNL_CHECK_IS_NULL_POINTER( domain );
    PNL_CHECK_IS_NULL_POINTER( pMD );
    PNL_CHECK_LEFT_BORDER( nNodes, 1 );
    
    
    CTabularCPD* resCPD = new CTabularCPD( domain, nNodes, pMD );
    intVector dom = intVector( domain, domain + nNodes );
    pConstNodeTypeVector ntVec;
    pMD->GetVariableTypes( dom, &ntVec );
    CTabularDistribFun* UniData = 
        CTabularDistribFun::CreateUnitFunctionDistribution( nNodes,
        &ntVec.front());
    if(resCPD->m_CorrespDistribFun)
    {
        delete resCPD->m_CorrespDistribFun;
    }
    resCPD->m_CorrespDistribFun = UniData;
    return resCPD;
}

CTabularCPD* CTabularCPD::Copy( const CTabularCPD* pTabCPD )
{
    PNL_CHECK_IS_NULL_POINTER( pTabCPD );
    
    if( pTabCPD->GetDistributionType() != dtTabular )
    {
	PNL_THROW( CInvalidOperation, "input CPD must be tabular here" )
    }
    
    CTabularCPD *retCPD = new CTabularCPD( *pTabCPD );
    PNL_CHECK_IF_MEMORY_ALLOCATED( retCPD );
    return retCPD;
}

CFactor* CTabularCPD::CloneWithSharedMatrices()
{
    CTabularCPD* resCPD = new CTabularCPD(this);
    PNL_CHECK_IF_MEMORY_ALLOCATED(resCPD);

    return resCPD;
}

CFactor* CTabularCPD::Clone() const
{
    const CTabularCPD* self = this;
    CTabularCPD* res = CTabularCPD::Copy(self);
    return res;
}

//typeOfMatrices = 1 - all matrices are random
//only Gaussian covariance matrix is matrix unit
//for ConditionalGaussianDistribution 
//the matrix of Gaussian distribution functions is dense
void CTabularCPD::CreateAllNecessaryMatrices( int typeOfMatrices )
{
    PNL_CHECK_RANGES( typeOfMatrices, 1, 1 );
    //we have only one type of matrices now
    if( m_CorrespDistribFun->IsDistributionSpecific() == 1 )
    {
        PNL_THROW( CInconsistentType,
            "uniform distribution can't have any matrices with data" );
    }
    m_CorrespDistribFun->CreateDefaultMatrices(typeOfMatrices);
    NormalizeCPD();
}


CTabularCPD::CTabularCPD( const CTabularCPD& TabCPD )
:CCPD(dtTabular, ftCPD, TabCPD.GetModelDomain())
{
    //m_CorrespDistribFun = TabCPD.m_CorrespDistribFun->CloneDistribFun();
    delete m_CorrespDistribFun;
    m_CorrespDistribFun = CTabularDistribFun::Copy(
	static_cast<CTabularDistribFun*>(TabCPD.m_CorrespDistribFun));
    m_Domain = intVector( TabCPD.m_Domain );
    
}

CTabularCPD::CTabularCPD( const int *domain, int nNodes, CModelDomain* pMD )
:CCPD( dtTabular, ftCPD, domain, nNodes, pMD  )
{
}

CTabularCPD::CTabularCPD( const CTabularCPD* pTabCPD)
:CCPD(pTabCPD)
{
}

bool CTabularCPD::IsValid(std::string* description) const
{
    if( !m_CorrespDistribFun )
    {
        if( description )
        {
            std::stringstream st;
            st<<"The factor haven't distribution function."<<std::endl;
            description->insert(description->begin(), st.str().begin(),
                st.str().end());
        }
        return 0;
    }
    if( m_Domain.empty() )
    {
        if( description )
        {
            std::stringstream st;
            st<<"The factor haven't domain."<<std::endl;
            std::string s = st.str();
            description->insert( description->begin(), s.begin(), s.end() );
        }
        return 0;
    }
    if( m_CorrespDistribFun->IsValid(description) )
    {
        if( m_CorrespDistribFun->IsDistributionSpecific() == 1 )
        {
            return 1;
        }
        if( static_cast<CTabularDistribFun*>(
            m_CorrespDistribFun)->IsMatrixNormalizedForCPD() )
        {
            return 1;
        }
        else
        {
            if(description)
            {
                std::stringstream st;
                st<<"The factor have matrix that doesn't normalized for CPD form."<<std::endl;
                std::string s = st.str();
                description->insert( description->begin(), s.begin(), s.end() );
            }
            return 0;
        }
    }
    else
    {
        if(description)
        {
            std::stringstream st;
            st<<"The factor have invalid distribution function."<<std::endl;
            std::string s = st.str();
            description->insert( description->begin(), s.begin(), s.end() );
        }
        return 0;
    }
    
    //bogus return (suppress compiler warning) - below line will not be reached 
    return 0;
}

void CTabularCPD :: NormalizeCPD()
{
    CTabularCPD *MyCPD = this;

    EMatrixClass mc = m_CorrespDistribFun->GetMatrix(matTable)->GetMatrixClass();
    if(( mc == mcNumericDense )||( mc == mc2DNumericDense ))
    {
        CNumericDenseMatrix<float> * pxMatrix = static_cast<
            CNumericDenseMatrix<float>*>(MyCPD->GetMatrix(matTable));
        int NumOfDims; const int *pRanges;
        pxMatrix->GetRanges(&NumOfDims,&pRanges);
    
        int data_length; 
        const float *OldData;
        pxMatrix->GetRawData(&data_length, &OldData);
        float *NewData = new float[data_length];
        PNL_CHECK_IF_MEMORY_ALLOCATED( NewData );
    
        float Sum = 0.0f;
        int ChildRange = pRanges[NumOfDims-1];
        int repeat = data_length/ChildRange;
        for (int i1 = 0; i1 < repeat ; i1++)
        {
	    int k = i1*ChildRange;
	    Sum = 0.0f;
	    int i2;
	    for (i2 = 0; i2 < ChildRange ; i2++)
	    {
	        Sum = Sum + OldData[k+i2];
	    }
	    if( fabs( Sum ) <= 1/FLT_MAX )
	    {
	        for (i2 = 0; i2 < ChildRange; i2++)
	        {
		    NewData[k+i2] = (OldData[k+i2]);//to avoid dividing by zero?
	        }
	    }
	    else
	    {
	        for (i2 = 0; i2 < ChildRange; i2++)
	        {
		    NewData[k+i2] = (OldData[k+i2])/Sum;
	        }
	    }
        }
    
        CNumericDenseMatrix<float>* NewMatrix = CNumericDenseMatrix<float>::Create(
            NumOfDims, pRanges, NewData );
        MyCPD->AttachMatrix( NewMatrix, matTable );
        delete []NewData;
    }
    else
	{
		CMatrix<float> *pMat = GetMatrix(matTable);
		int domainSize;
		const int* nodeSizes;
		pMat->GetRanges(&domainSize, &nodeSizes);
		int childeSize = nodeSizes[domainSize -1];
		
		intVector indexes(domainSize, 0);
		indexes[0] = -1;
		int ind;
		float sum;
		float koeff;
		float prob;
		float value;
		int i;
		if( domainSize > 1)
		{
			for( ind = 0; ind < domainSize -1 ; )
			{
			if( indexes[ind] == nodeSizes[ind] - 1 )
			{
				indexes[ind] = 0;
				ind++;
			}
			else
			{
				indexes[ind]++;
				ind = 0;
				for( i = 0, sum = 0.0f; i < childeSize; i++ )
				{
					indexes[domainSize - 1] = i;
					sum += pMat->GetElementByIndexes(&indexes.front());
				}
				koeff = sum > (1/FLT_MAX) ? 1/sum : 0.0f;
				for( i = 0; i < childeSize; i++ )
				{
					indexes[domainSize - 1] = i;
					value = pMat->GetElementByIndexes(&indexes.front());
					prob = koeff > (1/FLT_MAX) ? value*koeff : 1/childeSize;  
					pMat->SetElementByIndexes(prob, &indexes.front());
				}
			}
			
			}
		}
		else
		{
			for( i = 0, sum = 0.0f; i < childeSize; i++ )
			{
				sum += pMat->GetElementByIndexes(&i);
			}
			koeff = sum > (1/FLT_MAX) ? 1/sum : 0.0f;
			for( i = 0; i < childeSize; i++ )
			{  
				value = pMat->GetElementByIndexes(&i);
				prob = koeff > (1/FLT_MAX) ? value*koeff : 1/childeSize;  
				pMat->SetElementByIndexes(prob, &i);
			}
			
		}
    }
    
    
    //return MyCPD;
}

void CTabularCPD::UpdateStatisticsEM(  const CPotential *pMargPot, 
                                    const CEvidence *pEvidence )
{
    
    PNL_CHECK_IS_NULL_POINTER(pMargPot);
    intVector obsPos;
    pMargPot->GetObsPositions(&obsPos);
    
    
    if( obsPos.size() )
        {
            PNL_CHECK_IS_NULL_POINTER(pEvidence);
            CPotential *pExpandPot = pMargPot->ExpandObservedNodes(pEvidence, 0);
            m_CorrespDistribFun->UpdateStatisticsEM(pExpandPot->GetDistribFun(), pEvidence, 1.0f,
                &m_Domain.front());
            delete pExpandPot;
        }
    else
    
    {
        m_CorrespDistribFun->UpdateStatisticsEM( pMargPot->GetDistribFun(), pEvidence, 1.0f,
            &m_Domain.front() );
    }

}

float CTabularCPD::ProcessingStatisticalData(int numberOfEvidences )
{	
    return m_CorrespDistribFun->ProcessingStatisticalData( static_cast<float>(numberOfEvidences) );
}

void CTabularCPD::UpdateStatisticsML(const CEvidence* const* pEvidences, int EvidenceNumber)
{
    
    if( !pEvidences )
    {
	PNL_THROW( CNULLPointer, "evidences" )//no corresp evidences
    }
    if( EvidenceNumber <= 0 )
    {
	PNL_THROW(COutOfRange, "number of evidences must be positively")
    }
    int DomainSize;
    const int *domain;
    GetDomain( &DomainSize, &domain );
    m_CorrespDistribFun->UpdateStatisticsML( pEvidences, EvidenceNumber, domain );
    
}

CPotential *CTabularCPD::ConvertToPotential() const
{
    CDistribFun *pDistr = (m_CorrespDistribFun)->ConvertCPDDistribFunToPot();
    CTabularPotential *resFactor = CTabularPotential::Create(
	&m_Domain.front(), m_Domain.size(), GetModelDomain());
    resFactor->SetDistribFun( pDistr );
    delete pDistr;
    return resFactor;
}

void CTabularCPD::
GenerateSample( CEvidence* evidence, int maximize ) const
{
    //generate samples for node using iformation about its parens
    
    int nnodes;
    const int *domain;
    this->GetDomain( &nnodes, &domain );
    int i;
    for( i = 0; i < nnodes - 1; i++ )
    {
        if( ! evidence->IsNodeObserved( domain[i]) )
        {
            PNL_THROW(CAlgorithmicException, "all parents must be observed");
        }
    }
    if( evidence->IsNodeObserved( domain[nnodes-1] ) )
    {
        PNL_THROW(CAlgorithmicException, "node is already observed");
    }
    CPotential *tmpPot = this->ConvertWithEvidenceToPotential(evidence);
    tmpPot->GenerateSample(evidence, maximize);
    delete tmpPot;
    
    
    /*
    const CMatrix<float> *pDistrMatrix =  m_CorrespDistribFun->GetMatrix( matTable );
        const CMatrix<float> *pReduceMatrix;
        
        const int *pDomain;
        int nnodes;
        this->GetDomain( &nnodes, &pDomain );
        
        intVector nodesValue(nnodes - 1);
        intVector dimsOfInterest(nnodes - 1);
    
        CvRandState rng_state;
        cvRandInit( &rng_state, 0.0f, 1.0f, seed, CV_RAND_UNI );
        
        float rnd;
        CvMat uniValueMat = cvMat( 1, 1, CV_32F, &rnd );
           
        intVector indexes;
       
        int i;
        
        
            PNL_CHECK_IS_NULL_POINTER(evidence);
            if( nnodes > 1)
            {
                for( i = 0; i < nnodes - 1; i++ )
                {
                    if(! evidence->IsNodeObserved( pDomain[i] ) )
                    {
                        PNL_THROW(CAlgorithmicException, "all parents must be observed");
                    }
                    nodesValue[i] = evidence->GetValue( pDomain[i] );
                    dimsOfInterest[i] = i;
                }
                
                pReduceMatrix = pDistrMatrix->
                    ReduceOp( &dimsOfInterest.front(), nnodes - 1, 2, &nodesValue.front() );
            }
            else
            {
                pReduceMatrix = pDistrMatrix;
            }
                 
            cvRand( &rng_state, &uniValueMat );     
                           
            indexes.assign(nnodes, 0);
            indexes[nnodes - 1] = -1;
            float val = 0.0f;
           
            do{
                indexes[nnodes - 1]++;
                val += pReduceMatrix->GetElementByIndexes( &indexes.front() );
                
            }while( rnd > val );
            
            if( ! evidence->IsNodeObserved( pDomain[nnodes-1] ) )
            {
                evidence->MakeNodeObserved( pDomain[nnodes-1] );
            }
            
            ! wrong memcpy( evidence->GetValue( pDomain[nnodes-1] ), &indexes[nnodes-1] , 
                sizeof(Value)/sizeof(unsigned char));
            if( nnodes > 1 )
            {
                delete pReduceMatrix;
            }
            
        
        */
    
}

CPotential* CTabularCPD::ConvertStatisticToPot(int numOfSamples) const
{
    PNL_CHECK_LEFT_BORDER( numOfSamples, 0);
    CMatrix<float>* pLearnMatrix = this->GetDistribFun()->GetStatisticalMatrix(stMatTable);
    CPotential *pPot = this->ConvertToPotential();
    pPot->AttachMatrix( pLearnMatrix->Clone(), matTable );
    pPot->Normalize();
    return pPot;

}

float CTabularCPD::
GetLogLik( const CEvidence* pEv, const CPotential* pShrInfRes ) const
{
    
    CPotential* pPot = ConvertToPotential();
    float logLik = pPot->GetLogLik(pEv, pShrInfRes);
    delete pPot;
    return logLik;
}

#ifdef PAR_PNL
void CTabularCPD::UpdateStatisticsML(CFactor *pPot)
{
    //Проверка на правильность pPot
    if (pPot->GetDistributionType() != dtTabular)
        PNL_THROW(CInconsistentType, 
        "Can not use function CTabularCPD::UpdateStatisticsML with wrong distribution type");

    CDistribFun *pDF = pPot->GetDistribFun();
    m_CorrespDistribFun->UpdateStatisticsML( pDF );
};
#endif // PAR_OMP

#ifdef PNL_RTTI
const CPNLType CTabularCPD::m_TypeInfo = CPNLType("CTabularCPD", &(CCPD::m_TypeInfo));

#endif

float CTabularCPD::GetMatrixValue(const CEvidence *pEv)
{
	int nIndexes;
	int *indexes;
	nIndexes = m_Domain.size();
	indexes = new int[nIndexes];
	intVector pObsNodes;
	pConstValueVector pObsValues;
	pConstNodeTypeVector pNodeTypes;
	pEv->GetObsNodesWithValues(&pObsNodes,&pObsValues,&pNodeTypes);
	int i,j;
	for(i = 0; i < nIndexes; i++)
	{
		for(j = 0; j < pObsNodes.size(); j++)
		{
			if(pObsNodes[j] == m_Domain[i] )
			{
				indexes[i] = (pObsValues[j])->GetInt();
			};
		}
	};
	float value;
	value = (m_CorrespDistribFun->GetMatrix(matTable))->GetElementByIndexes(indexes);
	
	delete []indexes;
	return value;
}
