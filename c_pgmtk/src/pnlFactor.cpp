/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlFactor.cpp                                               //
//                                                                         //
//  Purpose:   CFactor class member functions implementation               //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlGaussianDistribFun.hpp"
#include "pnlSoftMaxDistribFun.hpp"
#include "pnlCondGaussianDistribFun.hpp"
#include "pnlCondSoftMaxDistribFun.hpp"
#include "pnlScalarDistribFun.hpp"
#include "pnlTreeDistribFun.hpp"

#include "pnlFactor.hpp"
#include "pnlPotential.hpp"
#include "pnlEvidence.hpp"
#include "pnlException.hpp"
//#include "pnlLearningEngine.hpp"

#include "pnlTabularPotential.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlScalarPotential.hpp"
#include "pnlMixtureGaussianCPD.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlGaussianCPD.hpp"
#include "pnlSoftMaxCPD.hpp"
#include "pnlTreeCPD.hpp"
#include <sstream>

PNL_BEGIN
bool pnlIsSubset(int smaSize, int* smaDomain, int bigSize, int* bigDomain)
{
	if(smaSize > bigSize) return false;
	int curDomain;
	for(int i=0; i<smaSize; i++)
	{
		curDomain = smaDomain[i];
		if( std::find(bigDomain, bigDomain+bigSize, curDomain) == bigDomain+bigSize)
			return false;
	}
	return true;
}

intVector pnlIntersect(int size1, int* Domain1, int size2, int* Domain2)
{
	intVector ret;
	int i, cur;
	for(i=0; i<size1; i++)
	{
		cur = Domain1[i];
		if( std::find(Domain2, Domain2+size2, cur) != Domain2+size2)
			ret.push_back(cur);
	}
	std::sort(ret.begin(), ret.end());
	return ret;
}

intVector pnlSetUnion(int size1, int* Domain1, int size2, int* Domain2)
{
	intVector ret;
	ret.assign(Domain2, Domain2+size2);
	int i,cur;
	for(i=0; i<size1; i++)
	{
		cur = Domain1[i];
		if( (std::find(Domain2, Domain2+size2, cur) == Domain2+size2))
			ret.push_back(cur);
	}
	std::sort(ret.begin(), ret.end());
	return ret;
}

bool pnlIsIdentical(int size1, int* Domain1, int size2, int* Domain2)
{
	if( size1 != size2 ) return false;
	for(int i=0; i<size1; i++)
	{
		if(Domain1[i] != Domain2[i]) return false;
	}
	return true;
}

PNL_END

PNL_USING

CFactor::CFactor( EDistributionType dt, EFactorType ft, CModelDomain* pMD )
{
    m_DistributionType = dt;
    m_FactorType = ft;
    m_CorrespDistribFun = NULL;
    m_pMD = pMD;
    m_factNumInHeap = m_pMD->AttachFactor(this);
}

CFactor::CFactor( EDistributionType dt,
                  EFactorType pt,
                  const int *domain, int nNodes, CModelDomain* pMD,
                  const intVector& obsIndices )
                  : m_Domain( domain, domain + nNodes )
{	
    /*fill enum fields:*/
    m_DistributionType = dt;
    m_FactorType = pt;
    m_pMD = pMD;
    m_factNumInHeap = m_pMD->AttachFactor(this);
    int i;
    pConstNodeTypeVector nt;
    intVector dom = intVector( domain, domain+nNodes );
    pMD->GetVariableTypes( dom, &nt );
    m_obsPositions.assign( obsIndices.begin(), obsIndices.end() );
    int numObsNodesHere = obsIndices.size();
    switch (dt)
    {
    case dtScalar:
        {
            if( pt == ftCPD )
            {
                PNL_THROW( CInvalidOperation, "scalar is only potential - to multiply" );
            }
            //if there are observed nodes - get corresponding node types
            if( numObsNodesHere )
            {
                if ( numObsNodesHere != nNodes )
                {
                    PNL_THROW( CInconsistentType,
                        "all nodes in scalar distribution must be observed" )
                }
                //need to find observed nodes in domain and check including their changed types
                for( i = 0; i < numObsNodesHere; i++ )
                {
                    nt[obsIndices[i]] = nt[obsIndices[i]]->IsDiscrete() ? pMD->GetObsTabVarType():
                        pMD->GetObsGauVarType();
                }
            }
            m_CorrespDistribFun = CScalarDistribFun::Create(nNodes, &nt.front());
            break;
        }
	case dtTree:
        {
            if( pt != ftCPD )
            {
                PNL_THROW( CInvalidOperation, "Tree is only CPD" );
            }
            m_CorrespDistribFun = CTreeDistribFun::Create(nNodes, &nt.front());
            break;
        }
    case dtTabular:
        {
            
            if(( pt == ftPotential )&&( numObsNodesHere ))
            {
                //need to find observed nodes in domain and check including their changed types
                for( i = 0; i < numObsNodesHere; i++ )
                {
                    //change node type for this node
                    nt[obsIndices[i]] = nt[obsIndices[i]]->IsDiscrete() ? pMD->GetObsTabVarType():
                    pMD->GetObsGauVarType();
                }
            }
            //check all node types corresponds Tabular distribution
            for( i = 0; i < nNodes; i++ )
            {
                if((!(( nt[i]->IsDiscrete() )||
                    ( !nt[i]->IsDiscrete()&&(nt[i]->GetNodeSize() == 0)))))
                {
                    PNL_THROW( CInconsistentType, 
                        "node types must corresponds Tabular type" );
                }
            }
            m_CorrespDistribFun = CTabularDistribFun::Create( nNodes,
                &nt.front(), NULL );
            break;
        }
    case dtGaussian:
        {
            switch (pt)
            {
            case ftPotential:
                {
                    //need to find observed nodes in domain and check including their changed types
                    for( i = 0; i < numObsNodesHere; i++ )
                    {
                        //change node type for this node
                        nt[obsIndices[i]] = nt[obsIndices[i]]->IsDiscrete() ?
                                pMD->GetObsTabVarType():pMD->GetObsGauVarType();
                    }
                    for( i = 0; i < nNodes; i++ )
                    {
                        if( nt[i]->IsDiscrete() && (nt[i]->GetNodeSize() != 1))
                        {
                            PNL_THROW( CInvalidOperation,
                                "Gaussian potential must be of Gaussian nodes only" )
                        }
                    }
                    m_CorrespDistribFun = 
                        CGaussianDistribFun::CreateInMomentForm( 1, nNodes,
                        &nt.front(), NULL, NULL, NULL  );
                    break;
                }
            case ftCPD:
                {
                    //can check if there are both Continuous & Discrete nodes
                    int noDiscrete = 1;
                    for( int i = 0; i < nNodes; i++ )
                    {
                        if( nt[i]->IsDiscrete() )
                        {
                            noDiscrete = 0;
                            break;
                        }
                    }
                    if( noDiscrete )
                    {
                        m_CorrespDistribFun = 
                            CGaussianDistribFun::CreateInMomentForm( 0, nNodes,
                            &nt.front(), NULL, NULL, NULL );
                        break;
                    }
                    else
                    {
                        m_CorrespDistribFun = 
                            CCondGaussianDistribFun::Create( 0, nNodes, &nt.front() );
                        break;
                    }
                }
            default:
                {
                    PNL_THROW( CBadConst, 
                        "no competent type as EFactorType" );
                    break;
                }
            }
            break;
        }
    case dtMixGaussian:
        {
            switch(pt)
            {
            case ftCPD:
                {
                    //check if where is discrete node - mixture node
                    int noDiscrete = 1;
                    for( int i = 0; i < nNodes; i++ )
                    {
                        if( nt[i]->IsDiscrete() )
                        {
                            noDiscrete = 0;
                            break;
                        }
                    }
                    if( !noDiscrete  )
                    {
                        m_CorrespDistribFun = CCondGaussianDistribFun::Create( 0,
                            nNodes, &nt.front() );
                    }
                    else
                    {
                        PNL_THROW( CInconsistentType, 
                            "mixture Gaussian CPD must have mixture node - discrete" );
                    }
                    break;
                }
            default:
                {
                    PNL_THROW( CNotImplemented, "mixture gaussian potential" );  
                }
            }
            break;
        }
    case dtSoftMax:
        {
            switch (pt)
            {
            case ftPotential:
                {
                  PNL_THROW( CNotImplemented, "only CPD yet" );
                    break;
                }
            case ftCPD:
                {
                    //can check if there are both Continuous & Discrete nodes
                    int noDiscrete = 1;
                    for( int i = 0; i < nNodes-1; i++ )
                    {
                        if( nt[i]->IsDiscrete() )
                        {
                            noDiscrete = 0;
                            break;
                        }
                    }
                    if( noDiscrete )
                    {
                        m_CorrespDistribFun = 
                            CSoftMaxDistribFun::Create( nNodes,
                            &nt.front(), NULL, NULL );
                        break;
                    }
                    else
                    {
                        m_CorrespDistribFun = 
                            CCondSoftMaxDistribFun::Create( nNodes, &nt.front() );
                        break;
                    }
/*
                  //can check if there are both Continuous & Discrete nodes
                        m_CorrespDistribFun = 
                            CSoftMaxDistribFun::CreateUnitFunctionDistribution( nNodes, &nt.front() );
                        break;
*/
                }
            default:
                {
                    PNL_THROW( CBadConst, 
                        "no competent type as EFactorType" );
                    break;
                }
            }
            break;
        }
    default:
        {
            PNL_THROW ( CBadConst,
                "we have no such factor type at EDistributionType");
        }
    }
}

CFactor::CFactor( const CFactor* factor)
{
    m_DistributionType = factor->m_DistributionType;
    m_FactorType = factor->m_FactorType;
    m_pMD = factor->m_pMD;
    m_factNumInHeap = m_pMD->AttachFactor(this);
    m_CorrespDistribFun = factor->m_CorrespDistribFun->CloneWithSharedMatrices();
    m_obsPositions.assign( factor->m_obsPositions.begin(), factor->m_obsPositions.end() );
    m_Domain.assign(factor->m_Domain.begin(), factor->m_Domain.end());
}

void CFactor::CreateAllNecessaryMatrices( int typeOfMatrices )
{
    PNL_CHECK_RANGES( typeOfMatrices, 1, 1 );
    //we have only one type of matrices now
    if( m_CorrespDistribFun->IsDistributionSpecific() == 1 )
    {
        PNL_THROW( CInconsistentType,
            "uniform distribution can't have any matrices with data" );
    }
    m_CorrespDistribFun->CreateDefaultMatrices(typeOfMatrices);
}

int CFactor::GetNumInHeap() const
{
    return m_factNumInHeap;
}
    
void CFactor::ChangeOwnerToGraphicalModel() const
{
    m_pMD->ReleaseFactor(this);
}
    
bool CFactor::IsOwnedByModelDomain() const
{
    return m_pMD->IsAFactorOwner(this);
}

CFactor::~CFactor()
{
    /* destroy all data */
    if( IsOwnedByModelDomain() )
    {
        m_pMD->ReleaseFactor(this);
    }
    if( m_CorrespDistribFun )
    {
        delete(m_CorrespDistribFun);
    }
}

CFactor & CFactor::operator= ( const CFactor& pInputFactor )
{
    if( this == &pInputFactor  )
    {
        return *this;
    }
    int i;
    int paramsTheSame = 1;
    if( m_pMD != pInputFactor.GetModelDomain() )
    {
        paramsTheSame = 0;
    }
    if( m_DistributionType != pInputFactor.GetDistributionType() )
    {
        paramsTheSame = 0;
    }
    if( m_FactorType != pInputFactor.GetFactorType() )
    {
        paramsTheSame = 0;
    }
    if( m_Domain.size() != pInputFactor.GetDomainSize() )
    {
        paramsTheSame = 0;
    }
    else
    {
        
        int domSize;
        const int *domain;
        pInputFactor.GetDomain( &domSize, &domain );
        for( i = 0; i < domSize; i++ )
        {
            if( m_Domain[i] != domain[i] )
            {
                paramsTheSame = 0;
                break;
            }
        }
    }
    if( m_obsPositions.size() != pInputFactor.m_obsPositions.size() )
    {
        paramsTheSame = 0;
    }
    else
    {
        int numObsPos = m_obsPositions.size();
        int loc;
        for( i = 0; i < numObsPos; i++ )
        {
            loc = std::find( m_obsPositions.begin(), m_obsPositions.end(),
                pInputFactor.m_obsPositions[i] ) - m_obsPositions.begin();
            if( loc > numObsPos )
            {
                paramsTheSame = 0;
                break;
            }
        }
    }
    if( paramsTheSame )
    {
        (*m_CorrespDistribFun) = ( *pInputFactor.m_CorrespDistribFun );
    }
    else
    {
        PNL_THROW( CInvalidOperation, 
            "parameters must have the same size, type and domain and have the same Model Domain" )
    }
    return *this;
}

void CFactor::GetDomain( intVector* domain ) const
{
    int domSize;
    const int* dom;
    GetDomain( &domSize, &dom );
    domain->assign( dom, dom + domSize  );
}

void CFactor::GetDomain(int *DomainSize, const int **domain) const
{
    if( ( !DomainSize ) || ( !domain ) )
    {
        PNL_THROW( CNULLPointer, "domain or domain size" )
    }	
    
    (*DomainSize) = m_Domain.size();
    (*domain) = &m_Domain.front();
}

int CFactor::IsFactorsDistribFunEqual(const CFactor *param,
                                      float epsilon, int withCoeff,
                                      float* maxDifference) const
{
    if( !param )
    {
        PNL_THROW( CNULLPointer, "parameter" )
    }
    
    return (m_CorrespDistribFun->IsEqual(param->GetDistribFun(), epsilon,
        withCoeff, maxDifference));
}

bool CFactor::IsValid(std::string* description) const
{
    if( !m_CorrespDistribFun )
    {
        if( description )
        {
            std::stringstream st;
            st<<"The factor haven't distribution function."<<std::endl;
            std::string s = st.str();
            description->insert(description->begin(), s.begin(), s.end());
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
            description->insert(description->begin(), s.begin(), s.end());
        }
        return 0;
    }
    if( m_CorrespDistribFun->IsValid(description) )
    {
        return 1;
    }
    else
    {
        if( description )
        {
            std::stringstream st;
            st<< "The factor have invalid distribution function."<<std::endl;
            std::string s = st.str();
            description->insert(description->begin(), s.begin(), s.end());
        }
        return 0;
    }
}

const pConstNodeTypeVector *CFactor::GetArgType() const
{
    return m_CorrespDistribFun->GetNodeTypesVector();
}

CMatrix<float>* CFactor::GetMatrix(EMatrixType mType, int numberOfMatrix,
                                   const int* discrParentValuesIndices )const
{
    /*Return pointer to CMatrix of mType*/
    CMatrix<float>* ReturnMatr = m_CorrespDistribFun->GetMatrix(
        mType, numberOfMatrix, discrParentValuesIndices);
    return ReturnMatr;
}

void CFactor::AllocMatrix( const float* data, EMatrixType mType,
                          int numberOfMatrix,
                          const int *discrParentValuesIndices)
{
    /*Allocate the matrix as m_Type for this tape of DistribFun*/
    
    PNL_CHECK_IS_NULL_POINTER( data );
    
    m_CorrespDistribFun->AllocMatrix( data, mType, numberOfMatrix,
        discrParentValuesIndices );
    
    
}

void CFactor::AttachMatrix(CMatrix<float> *matrix, 
                           EMatrixType mType, int numberOfMatrix, 
                           const int* discrParentValuesIndices)
{
    /*Attach the matrix as m_Type for this type of DistribFun*/
    PNL_CHECK_IS_NULL_POINTER( matrix );
    
    m_CorrespDistribFun->AttachMatrix( matrix, mType, numberOfMatrix,
        discrParentValuesIndices );
}

void CFactor::ConvertToSparse()
{
    CDistribFun* sparseDistr = m_CorrespDistribFun->ConvertToSparse();
    SetDistribFun( sparseDistr );
    delete sparseDistr;
}

void CFactor::ConvertToDense()
{
    CDistribFun* denceDistr = m_CorrespDistribFun->ConvertToDense();
    SetDistribFun( denceDistr );
    delete denceDistr;
}

int CFactor::IsDense() const
{
    return m_CorrespDistribFun->IsDense();
}

int CFactor::IsSparse() const
{
    return m_CorrespDistribFun->IsSparse();
}

//void CFactor::Clamp(int clampingType)
//{
//	m_CorrespDistribFun->SetClamped(clampingType);
/*clamping type are:
const int ClampTable = 1;
const int ClampMean = 2;
const int ClampCov = 4;
const int ClampWeights = 8; their combinations clamp unit of them*/
//}


void CFactor::SetDistribFun(const CDistribFun *data )
{
    if ( !data)
    {
        PNL_THROW( CNULLPointer, "data" )
    }
    if ( !m_CorrespDistribFun )
    {
        PNL_THROW( CInvalidOperation, "no corresponding data - we can't set new data" )
    }
    //we need to compare factors of data - they must be the same
    EDistributionType dtCorr = m_CorrespDistribFun->GetDistributionType();
    EDistributionType dtIn = data->GetDistributionType();
    const pConstNodeTypeVector *ntCorr = m_CorrespDistribFun->GetNodeTypesVector();
    const pConstNodeTypeVector *ntIn   = data->GetNodeTypesVector();
    int sizeCorr = ntCorr->size();
    int sizeIn = ntIn->size();
    //fixme - is there any other possible combinations
    if(!(( dtCorr == dtIn )||(dtCorr == dtScalar)||(dtIn == dtScalar)))
    {
        PNL_THROW( CInconsistentType, "distribution type" )
    }
    if( sizeCorr != sizeIn )
    {
        PNL_THROW( CInconsistentSize, "size of data" )
    }
    /*	int nodeSizeCorr; int IsDiscreteCorr; int nodeSizeIn; int IsDiscreteIn;
    for ( int i = 0; i < sizeCorr; i++ )
    {
    nodeSizeIn = (*ntIn)[i]->GetNodeSize();
    IsDiscreteIn = (*ntIn)[i]->IsDiscrete();
    nodeSizeCorr = (*ntCorr)[i]->GetNodeSize();
    IsDiscreteCorr = (*ntCorr)[i]->IsDiscrete();
    if( nodeSizeIn != nodeSizeCorr )
    {
    PNL_THROW( CInconsistentType, "node size" ) 
    }
    if( IsDiscreteCorr != IsDiscreteIn )
    {
    PNL_THROW( CInconsistentType, "discreteness of node" ) 
    }
    }
    */
    //now we check all and can replace correaponding data by input data
    delete m_CorrespDistribFun;
    CDistribFun *dat = data->Clone();
    m_CorrespDistribFun = dat;
}

int CFactor::IsDistributionSpecific()const
{
    return m_CorrespDistribFun->IsDistributionSpecific();
}

void CFactor::MakeUnitFunction()
{
    m_CorrespDistribFun->SetUnitValue(1);
}


void CFactor::TieDistribFun( CFactor *factor )
{
    PNL_CHECK_IS_NULL_POINTER( factor );
    if( m_Domain.size() != factor->m_Domain.size() )
    {
        PNL_THROW( CInvalidOperation, "domain has different sizes" );
    }
    if( m_DistributionType != factor->m_DistributionType )
    {
        PNL_THROW( CInvalidOperation, "distribution type should be the same" )
    }
    if( m_FactorType != factor->m_FactorType )
    {
        PNL_THROW( CInvalidOperation, "parameter type should be the same" )
    }
    SetDistribFun( factor->GetDistribFun() );
}

void CFactor::UpdateStatisticsML( const pConstEvidenceVector& evidences )
{
    UpdateStatisticsML( &evidences.front(), evidences.size() );
}



CFactor* CFactor::CopyWithNewDomain(const CFactor *factor, intVector &domain, 
                                                 CModelDomain *pMDNew,
                                                 const intVector& /* obsIndices */)
{
    int domSize = domain.size();
    intVector domOld;
    factor->GetDomain( &domOld );
    if( int(domOld.size()) != domSize )
    {
        PNL_THROW( CBadArg, "number of nodes" );
    }
    CModelDomain *pMDOld = factor->GetModelDomain();
    
    //check is the types are the same
    const pConstNodeTypeVector* ntFactor = factor->GetArgType();
    
    /*
    const CNodeType *nt;
    
    for( int i = 0; i < domSize; i++ )
    {
        nt = (*ntFactor)[i];
        if( nt->IsDiscrete() )
        {
            if( nt->GetNodeSize() == 1 )
            {
                if( *pMDOld->GetVariableType(domOld[i]) != 
                    *pMDNew->GetVariableType(domain[i]))
                {
                    PNL_THROW(CInconsistentType, "types of variables should correspond");
                }
                
            }
            else
            {
                if( *nt != *pMDNew->GetVariableType(domain[i]) )
                {
                    PNL_THROW(CInconsistentType, "types of variables should correspond");
                }
            }
        }
        else
        {
            if( nt->GetNodeSize() == 0 )
            {
                if( *pMDOld->GetVariableType(domOld[i]) != 
                    *pMDNew->GetVariableType(domain[i]))
                {
                    PNL_THROW(CInconsistentType, "types of variables should correspond");
                }
                
            }
            else
            {
                if( *nt != *pMDNew->GetVariableType(domain[i]) )
                {
                    PNL_THROW(CInconsistentType, "types of variables should correspond");
                }
            }
        }

        
    }
    */
    const CNodeType *nt;
    int i;
    intVector obsPositions;
    factor->GetObsPositions(&obsPositions);
    if( obsPositions.size() )
    {
	intVector::iterator iterEnd = obsPositions.end();
        for( i = 0; i < domSize; i++)
        {
            if( std::find( obsPositions.begin(),iterEnd, i) != iterEnd )
            {
                if( *pMDOld->GetVariableType(domOld[i]) != 
                    *pMDNew->GetVariableType(domain[i]))
                {
                    PNL_THROW(CInconsistentType, "types of variables should correspond");
                }
            }
        }
    }
    else
    {
        for( i = 0; i < domSize; i++ )
        {
            nt = (*ntFactor)[i];
            if( *nt != *pMDNew->GetVariableType(domain[i]) )
            {
                PNL_THROW(CInconsistentType, "types of variables should correspond");
            }
            
        }
    }
    
    

    CFactor *pNewFactor;
    switch ( factor->GetFactorType() )
    {
    case ftPotential:
            {
                switch ( factor->GetDistributionType() )
                {
            case dtTabular:
                {
                    pNewFactor = CTabularPotential::
                        Copy(static_cast<const CTabularPotential*>(factor) );
                    break;
                }
            case dtGaussian:
                {
                    pNewFactor = CGaussianPotential::
                        Copy(static_cast<const CGaussianPotential*>(factor));
                    break;
                }
            case dtScalar:
                {
                    pNewFactor = CScalarPotential::
                        Copy( static_cast<const CScalarPotential*>(factor) );
                    break;
                }
            default:
                {
                    PNL_THROW(CNotImplemented, "distribution type" );
                }
            }
            break;
        }
    case ftCPD:
        {
            switch ( factor->GetDistributionType() )
            {
            case dtTabular:
                {
                    pNewFactor = CTabularCPD::
                        Copy(static_cast<const CTabularCPD*>(factor));
                    break;
                }
            case dtGaussian:
                {
                    pNewFactor = CGaussianCPD::
                        Copy(static_cast<const CGaussianCPD*>(factor));
                    break;
                }
            case dtCondGaussian:
                {
                    pNewFactor = CGaussianCPD::
                        Copy(static_cast<const CGaussianCPD*>(factor));
                    break;
                }
            case dtSoftMax:
                {
                    pNewFactor = CSoftMaxCPD::
                        Copy(static_cast<const CSoftMaxCPD*>(factor));
                    break;
                }
            case dtCondSoftMax:
                {
                    pNewFactor = CSoftMaxCPD::
                        Copy(static_cast<const CSoftMaxCPD*>(factor));
                    break;
                }
            case dtMixGaussian:
                {
                    pNewFactor = CMixtureGaussianCPD::Copy( 
                        static_cast<const CMixtureGaussianCPD*>(factor));
                    break;
                }
            default:
                {
                    PNL_THROW(CNotImplemented, "distribution type" );
                }
            }
            break;
        }
    default:
        {
            PNL_THROW(CNotImplemented, "factor type" );
        }
    }
    PNL_CHECK_IF_MEMORY_ALLOCATED(pNewFactor);
    /*
    if( pMDNew == factor->GetModelDomain())
    {
        return pNewFactor;
    }
    else*/
    {
        pNewFactor->m_Domain = intVector(domain);
        pNewFactor->SetModelDomain(pMDNew, 0);
        return pNewFactor;
    }
    
}

void CFactor::SetModelDomain( CModelDomain* pMD, bool checkNodeTypesinMD )
{
    if( pMD == m_pMD )
    {
        return;
    }
    if( checkNodeTypesinMD )
    {
        //check if node types are the same - check only node types in models!
        int i;
        int domSize = m_Domain.size();
    
        for( i = 0; i < domSize; i++ )
        {
            if( *(m_pMD->GetVariableType(m_Domain[i])) != *(pMD->GetVariableType(m_Domain[i]) ) )
            {
                PNL_THROW( CInconsistentType, "types of variables should correspond" );
            }
        }
    }
    //need to set new ModelDomain
    CDistribFun *pNewDistribFun;
    pNewDistribFun = GetDistribFun();
    pConstNodeTypeVector ntFromNewModelDomain;
    pMD->GetVariableTypes( m_Domain, &ntFromNewModelDomain );

    const pConstNodeTypeVector *nodeTypes = GetDistribFun()->GetNodeTypesVector();
    const CNodeType *nt;
    int i;
    int numObsPos = m_obsPositions.size();
    if( numObsPos )
    {
        for( i = 0; i < numObsPos; i++ )
        {
            nt = (*nodeTypes)[m_obsPositions[i]];
            if( nt->IsDiscrete() )
            {
                
                ntFromNewModelDomain[m_obsPositions[i]] = pMD->GetObsTabVarType();
            }
            else
            {
                ntFromNewModelDomain[m_obsPositions[i]] = pMD->GetObsGauVarType();
                
            }
            
        }
    }
    pNewDistribFun->ResetNodeTypes( ntFromNewModelDomain );
    CFactor* obj = this;
    if( m_pMD->IsAFactorOwner(obj) )
    {
        m_pMD->ReleaseFactor(obj);
        m_pMD = pMD;
        int num = m_pMD->AttachFactor( obj );
        m_factNumInHeap = num;
    }
    else
    {
        m_pMD = pMD;
    }
}

void CFactor::SetStatistics( const CMatrix<float> *pMat, 
			    EStatisticalMatrix matrix, const int* parentsComb)
{
    CDistribFun *pDistr = GetDistribFun();
    PNL_CHECK_IS_NULL_POINTER(pDistr);
    pDistr->SetStatistics( pMat, matrix, parentsComb );
}

int CFactor::GetNumberOfFreeParameters() const
{
	return m_CorrespDistribFun->GetNumberOfFreeParameters();
}

#ifdef PNL_RTTI
const CPNLType CFactor::m_TypeInfo = CPNLType("CFactor", &(CPNLBase::m_TypeInfo));

#endif

