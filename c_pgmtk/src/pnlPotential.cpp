/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPotential.cpp                                            //
//                                                                         //
//  Purpose:   CPotential class member functions implementation            //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlDistribFun.hpp"
#include "pnlGaussianDistribFun.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlScalarDistribFun.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlScalarPotential.hpp"
#include "pnlException.hpp"
#include "pnlInferenceEngine.hpp"//for method DetermineDistributionType
#include "pnlLogDriver.hpp"
#include "pnlLogUsing.hpp"

PNL_BEGIN
//currently can only deal with dtTabular, dtGaussian & dtScalar
CPotential* pnlMultiply( CPotential* Pot1, CPotential* Pot2, CModelDomain* pMD )
{
    int i;
	EDistributionType dt1 = Pot1->GetDistributionType();
	EDistributionType dt2 = Pot2->GetDistributionType();
	if( (dt1 != dtTabular && dt1 !=dtGaussian && dt1 != dtScalar) &&
		(dt2 != dtTabular && dt2 !=dtGaussian && dt2 != dtScalar) )
	{
		PNL_THROW(CInvalidOperation, "only implemented for tabular/gaussian/scalar potential");
	}
	if(dt1 != dt2)
	{
		if((dt1 != dtScalar) && (dt2 != dtScalar))
			PNL_THROW(CInvalidOperation, "at least one should be dtScalar");
	}
	EDistributionType dt;
	if(dt1 != dtScalar)
		dt = dt1;
	else
		dt = dt2;

	int loc, bigSize;
	intVector Domain1, Domain2, bigDomain;
	intVector Obspos1, Obspos2, Obspos;
	intVector Obsnodes1, Obsnodes2, Obsnodes;
	Pot1->GetDomain(&Domain1);
	Pot2->GetDomain(&Domain2);
	bigDomain = pnlSetUnion(Domain2.size(), &Domain2.front(), Domain1.size(), &Domain1.front());

	Pot1->GetObsPositions(&Obspos1);
	Pot2->GetObsPositions(&Obspos2);
	for(i=0; i<Obspos1.size(); i++)
	{
		Obsnodes1.push_back(Domain1[Obspos1[i]]);
	}
	for(i=0; i<Obspos2.size(); i++)
	{
		Obsnodes2.push_back(Domain2[Obspos2[i]]);
	}
	Obsnodes = pnlSetUnion(Obsnodes1.size(), &Obsnodes1.front(), Obsnodes2.size(), &Obsnodes2.front());
	bigSize = bigDomain.size();
	for(i=0; i<Obsnodes.size(); i++)
	{
		loc = std::find(bigDomain.begin(), bigDomain.end(), Obsnodes[i]) - bigDomain.begin();
		if( loc < bigSize )
			Obspos.push_back(loc);
	}
	CPotential* bigPotential;

//	if(dt == dtTabular || dt == dtScalar)
    if (dt == dtTabular)
    {
        bigPotential = CTabularPotential::CreateUnitFunctionDistribution(
	    &bigDomain.front(), bigDomain.size(), pMD, 1, Obspos);
    } else {
        if (dt == dtScalar)
            bigPotential = CScalarPotential::Create(&bigDomain.front(), bigDomain.size(), 
            pMD, Obspos);
        
        else
            bigPotential = CGaussianPotential::CreateUnitFunctionDistribution(
            &bigDomain.front(), bigDomain.size(), pMD, 1, Obspos);
    }

	*bigPotential *= *Pot1;
	*bigPotential *= *Pot2;
	return bigPotential;
}
PNL_END

PNL_USING

CPotential::CPotential( EDistributionType dt, EFactorType ft, CModelDomain* pMD )
					:CFactor( dt, ft, pMD )
{

}

CPotential :: CPotential(EDistributionType dt,
						 EFactorType pt, const int *domain, int nNodes,
			 CModelDomain* pMD, const intVector& obsIndices) : 
CFactor( dt, pt, domain, nNodes, pMD, obsIndices) 
{
	
}

CPotential::CPotential(const CPotential* potential ):
	    CFactor(potential)
{

}

CPotential* CPotential::Marginalize( const intVector& smallDomain,
			            int maximize ) const
{
    int smallDomSize = smallDomain.size();
    const int* pSmallDom = &smallDomain.front();
    return Marginalize( pSmallDom, smallDomSize, maximize );
}

CPotential * CPotential::Marginalize( const int *pSmallDom, int DomSize, 
				     int maximize) const
{
    if ( DomSize > m_Domain.size()) 
    {
	PNL_THROW( CInconsistentSize, 
	    "small domain size more than this domain size" )
	    /* bad argument check : Small domain 
	    should be subset of the Factor domain*/
    }
    intVector::const_iterator pEquivPos;
    /*temporary for pointer to Equivalent position*/
    int *pEqPositions = new int[DomSize];
    /*contain the numbers of equivalent positions*/	
    PNL_CHECK_IF_MEMORY_ALLOCATED( pEqPositions )
	/*Create new factor of SmallDom size*/
	const CNodeType **pNodeTypesSmD = new const CNodeType*[DomSize];
    PNL_CHECK_IF_MEMORY_ALLOCATED( pNodeTypesSmD );
    const pConstNodeTypeVector *NodeTypesOfDomain = 
	m_CorrespDistribFun->GetNodeTypesVector();
    //before creation need to recognize observed nodes in this potential
    intVector obsPositionsInSmall;
    int numObsPosHere = m_obsPositions.size();
    obsPositionsInSmall.reserve(numObsPosHere);
    int find;
    for (int i=0;i<DomSize;i++)
    {
	pEquivPos = std::find(m_Domain.begin(),m_Domain.end(),
	    pSmallDom[i]);
	if (pEquivPos!=m_Domain.end())
	{
	    pEqPositions[i] = (pEquivPos - m_Domain.begin());
	    pNodeTypesSmD[i] = (*NodeTypesOfDomain)[pEqPositions[i]];
	    if( numObsPosHere > 0 )
	    {
		find = std::find( m_obsPositions.begin(), m_obsPositions.end(),
		    pEqPositions[i]) - m_obsPositions.begin();
		if( find < numObsPosHere ) 
		{
		    obsPositionsInSmall.push_back(i);
		}
	    }
	}
	else 
	{
	    PNL_THROW( CInconsistentSize,
		"small domain isn't subset of domain" )
		return NULL;
	}
	//check that pSmallDom is m_Domain's subset
    };
    //check the distribution type after entering evidence
    //if there are only observed nodes in marginalized potential - it is scalar distribution
    EDistributionType dt = m_DistributionType;
    if( DomSize == int(obsPositionsInSmall.size()))
    {
	dt = dtScalar;
    }
    CPotential *pNewParam = NULL;
    switch (dt)
    {
    case dtTabular:
	{
	    pNewParam = CTabularPotential::Create( pSmallDom,
		DomSize, GetModelDomain(),NULL, obsPositionsInSmall );
	    break;
	}
    case dtGaussian:
	{
	    pNewParam = CGaussianPotential::Create( pSmallDom,
		DomSize, GetModelDomain(), -1, NULL, NULL, 0.0f,
		obsPositionsInSmall );
	    break;
	}
    case  dtScalar:
	{
	    pNewParam = CScalarPotential::Create( pSmallDom, DomSize,
		GetModelDomain(), obsPositionsInSmall );
	    break;
	}
    default:
	{
	    PNL_THROW( CNotImplemented,
		"we have only Tabular & Gaussian now" );
	}
    }
    
    if(	DomSize == 0 )
    {
	PNL_THROW( COutOfRange, "domain size should be positive" );
    }
    pNewParam->GetDistribFun()->MarginalizeData( m_CorrespDistribFun, pEqPositions, 
	DomSize, maximize);
    
    delete [] pEqPositions;		
    delete [] pNodeTypesSmD;
    return pNewParam;
}

void CPotential::MarginalizeInPlace( const CPotential* pOldPot,
                                    const int* corrPositions, int maximize)
{
    PNL_CHECK_IS_NULL_POINTER( pOldPot );
    EDistributionType dtOther = pOldPot->GetDistributionType();
    EDistributionType dtHere = GetDistributionType();
    if( dtOther != dtHere )
    {
        PNL_THROW( CInconsistentType,
            "both potentials must be of the same type" );
    }
    int i;
    intVector bigDom;
    pOldPot->GetDomain(&bigDom);
    const int* domHere;
    int numNodesHere;
    GetDomain( &numNodesHere, &domHere );
    intVector corPos;
    corPos.assign(numNodesHere, 0);
    //check the domains and find/check corresponding positions
    if( corrPositions )
    {
        //check them
        for( i = 0; i < numNodesHere; i++ )
        {
            if( domHere[i] != bigDom[corrPositions[i]] )
            {
                PNL_THROW( CInconsistentState,
                    "corrPositions doesn't correspond the domains" );
            }
        }
    }
    else
    {
        //find corrPositions
        int bigDomSize = bigDom.size();
        int loc;
        for( i = 0; i < numNodesHere; i++ )
        {
            loc = std::find( bigDom.begin(), bigDom.end(),
                domHere[i] ) - bigDom.begin();
            if( loc < bigDomSize )
            {
                corPos[i] = loc;
            }
            else
            {
                PNL_THROW( CInconsistentState,
                    "small domain isn't subset of big" );
            }
        }
        corrPositions = &corPos.front();
    }
    if( dtHere != dtTabular )
    {
        PNL_THROW( CNotImplemented, "we have only for tabular now" );
    }
    static_cast<CTabularDistribFun*>(m_CorrespDistribFun)->Marginalize(
        pOldPot->GetDistribFun(), corrPositions, numNodesHere, maximize );

}

CEvidence* CPotential::GetMPE() const
{
    CNodeValues* values = m_CorrespDistribFun->GetMPE();
    if( values )
    {
	CEvidence* pMPE = CEvidence::Create( values, m_Domain.size(),
	&m_Domain.front(), GetModelDomain() );
	delete values;
	return pMPE;
    }
    else
    {
	return NULL;
    }
}

CPotential * CPotential::ShrinkObservedNodes( const CEvidence* pEvidence ) const 
{
    PNL_CHECK_IS_NULL_POINTER( pEvidence );
    
    //first we need to determine is there any observed nodes in this potential
    intVector obsPositionsFromThisEvidence;
    int domSize = m_Domain.size();
    obsPositionsFromThisEvidence.reserve(domSize);
    intVector obsPositionsInDom;
    obsPositionsInDom.reserve(domSize);
    pConstValueVector obsVals;
    obsVals.reserve(domSize);
    
    int numObsNowHere = m_obsPositions.size();
    const pConstNodeTypeVector* ntVec = GetDistribFun()->GetNodeTypesVector();
    int i;
    if( numObsNowHere )
    {
	for( i = 0; i < domSize; i++  )    
	{
	    int curNode = m_Domain[i];
	    int findInObservedNow = std::find( m_obsPositions.begin(),
		m_obsPositions.end(), i ) - m_obsPositions.begin();
	    if( findInObservedNow >= numObsNowHere )
	    {
		if( pEvidence->IsNodeObserved(curNode) )
		{
		    if( (*ntVec)[i]->IsDiscrete() )
		    {
			obsPositionsFromThisEvidence.push_back(i);
			obsPositionsInDom.push_back(i);
			obsVals.push_back( pEvidence->GetValue(curNode));
		    }
		    else
		    {
			obsPositionsFromThisEvidence.push_back(i);
			obsPositionsInDom.push_back(i);
			obsVals.push_back( pEvidence->GetValue(curNode));
		    }
		}
	    }
	    else
	    {
		obsPositionsInDom.push_back(i);
	    }
	}
    }
    else
    {
       for( i = 0; i < domSize; i++  )    
	{
	    int curNode = m_Domain[i];
	    if( pEvidence->IsNodeObserved(curNode) )
	    {
		if( (*ntVec)[i]->IsDiscrete() )
		{
		    obsPositionsFromThisEvidence.push_back(i);
		    obsPositionsInDom.push_back(i);
		    obsVals.push_back( pEvidence->GetValue(curNode));
		}
		else
		{
		    obsPositionsFromThisEvidence.push_back(i);
		    obsPositionsInDom.push_back(i);
		    obsVals.push_back( pEvidence->GetValue(curNode));
		}
	    }
	}
    }
    int nAllObsNodes = obsPositionsInDom.size();
    int nObsNodesFromTheEvidence = obsPositionsFromThisEvidence.size();

    CPotential *pNewFactor = NULL;
    if( nAllObsNodes == domSize )
    {
	//need to create scalar potential
	pNewFactor = CScalarPotential::Create( m_Domain, GetModelDomain(),
	    obsPositionsInDom );
	return pNewFactor;
    }
    switch( m_DistributionType )
    {
    case dtTabular:
	{
	    pNewFactor = CTabularPotential::Create( &m_Domain.front(),
		m_Domain.size(), GetModelDomain(), NULL, obsPositionsInDom);
	    break;
	}
    case dtGaussian:
	{
	    pNewFactor = CGaussianPotential::Create( &m_Domain.front(),
		m_Domain.size(), GetModelDomain(), -1, NULL, NULL, 0.0f, obsPositionsInDom);
	    break;
	}
    default:
	{
	    PNL_THROW( CNotImplemented, 
		"we have only Tabular & Gaussian now" );
	}
    }
    /*creation a new data for new factor 
    as a result of taking into by m_CorrespDistribFun*/
    CModelDomain* pMD = GetModelDomain();
    if( nObsNodesFromTheEvidence == 0 )
    {
	pNewFactor->GetDistribFun()->ShrinkObservedNodes(
	    m_CorrespDistribFun, NULL, NULL, nObsNodesFromTheEvidence,
	    pMD->GetObsTabVarType(), pMD->GetObsGauVarType());
    }
    else
    {
	pNewFactor->GetDistribFun()->ShrinkObservedNodes( m_CorrespDistribFun,
	    &obsPositionsFromThisEvidence.front(), &obsVals.front(),
	    nObsNodesFromTheEvidence, pMD->GetObsTabVarType(),
	    pMD->GetObsGauVarType());
    }
    return pNewFactor;
}

CPotential *CPotential::ExpandObservedNodes( const CEvidence *evid ,
				int UpdateCanonical) const
{
	//we need to add information at CDistribFun object for expanding data
	//expanding of data is the same for CPD & factor -
	//we just add zeros at needing positions,
	//and positions the same for both them
	//we need to check evidence & domain & node types for compatibility 
	//and change positions to domain positions
	int i,j;
	//check the numbers of nodes & domain
    int domSize = m_Domain.size();
	//now we need to compare node types and create 
	//an array of node types expanded param
    const CNodeType *const *nTypesFromParam = 
	  &m_CorrespDistribFun->GetNodeTypesVector()->front();
    const CNodeType * const *ntFromEvidence = evid->GetNodeTypes();
    //find positions of observed nodes in domain
    int numAllObsNodes = evid->GetNumberObsNodes();
    const int *AllObsNodes = evid->GetAllObsNodes();
    const int *allFlags = evid->GetObsNodesFlags();                                       
    intVector numbersOfExtDims;
    pConstValueVector pValues;
    CNodeType const* * ntInExpandParam = new const CNodeType*[domSize];
	PNL_CHECK_IF_MEMORY_ALLOCATED( ntInExpandParam );
    memcpy( ntInExpandParam, nTypesFromParam, 
		domSize*sizeof(const CNodeType*) );
    for( i = 0; i < domSize; i++ )
    {
	  for( j = 0; j < numAllObsNodes; j++ )
	  {
		if( ( m_Domain[i] == AllObsNodes[j] ) &&( allFlags[j] ) )
		{
		  //we need to set such node type for this node to expand
		  ntInExpandParam[i] = ntFromEvidence[j];
		  pValues.push_back(evid->GetValueBySerialNumber(j));
		  numbersOfExtDims.push_back( i );
		}
	  }
    }
			                        
    int numObsDimsInParam = numbersOfExtDims.size();
			                         
    CPotential *retParam = NULL;
    EDistributionType dtHere  = dtInvalid;

    if( m_DistributionType == dtScalar )
    {
	//need to expand it to type determining by node types
	dtHere = pnlDetermineDistributionType( GetModelDomain(), domSize,
	    &m_Domain.front(), NULL );
    }
    else
    {
	dtHere = m_DistributionType;
    }
	switch (dtHere)
	{
	case dtTabular:
		{
			retParam = CTabularPotential::Create(&m_Domain.front(),
		domSize, GetModelDomain() );
			break;
		}
	case dtGaussian:
		{
			retParam = CGaussianPotential::Create(&m_Domain.front(),
		domSize, GetModelDomain() );
			break;
		}
	default:
		{
			PNL_THROW( CNotImplemented,
				"we have only Tabular & Gaussian now" );
		}
	}
    if( m_DistributionType != dtScalar )
    {
	retParam->SetDistribFun( m_CorrespDistribFun );
	if(  numObsDimsInParam != 0 )
	{
	    /*EDistributionType dtParamAfterExpand =  
    			pnlDetermineDistributionType(domSize, 0, NULL, ntInExpandParam);
		    if( dtParamAfterExpand != m_DistributionType )
		    {
    		    PNL_THROW( CInconsistentType, "distribution type after expand")
		    }*/
		    retParam->GetDistribFun()->ExpandData( &numbersOfExtDims.front(), 
		 numObsDimsInParam, &pValues.front(), ntInExpandParam , 
			    UpdateCanonical);
	}
    }
    else
    {
	//need to create corresponding data for scalar data - matrices
	CDistribFun* resDistr = static_cast<CScalarDistribFun*>(
	    m_CorrespDistribFun)->ExpandScalarData(dtHere,
	    &numbersOfExtDims.front(), numbersOfExtDims.size(),
	    &pValues.front(), ntInExpandParam, UpdateCanonical );
	retParam->SetDistribFun(resDistr);
        delete resDistr;
    }
    delete []ntInExpandParam;
    return retParam;
}
			                     
CPotential *CPotential::Multiply(const CPotential *pOtherFactor ) const
{
    if( GetModelDomain() != pOtherFactor->GetModelDomain() )
    {
	PNL_THROW( CInconsistentType,
	    "potentials based on the same ModelDomain can be multiplied" )
    }
    EDistributionType my_dt = m_DistributionType;
    EDistributionType other_dt = pOtherFactor->GetDistributionType();
    int i;
    if((( my_dt == other_dt )&&(( my_dt == dtTabular )||(my_dt == dtGaussian))))
    {
	
	int s1 = GetDomainSize();
	int s2 = pOtherFactor->GetDomainSize();
	const CPotential *bigFactor = NULL;
	const CPotential *smallFactor = NULL;
	if (s1 > s2)  
	{
	    bigFactor = this;
	    smallFactor = pOtherFactor;
	}
	else
	{
	    bigFactor = pOtherFactor;
	    smallFactor = this;
	}
	//check inclusion of small domain in big
	int bigDomSize; 
	const int *bigDomain;
	bigFactor->GetDomain( &bigDomSize, &bigDomain );
	intVector bigDomCheck( bigDomain, bigDomain+bigDomSize );
	int smDomSize; 
	const int *smDomain;
	smallFactor->GetDomain( &smDomSize, &smDomain );
        if(( bigDomSize == 2 )&&( smDomSize == 1 ))
        {
            if(!(( smDomain[0] == bigDomain[0] )||(smDomain[0] == bigDomain[1])))
            {
                PNL_THROW( COutOfRange, 
		    "small domain isn't subset of big" );
            }
        }
        else
        {
            int loc;
	    for( i = 0; i < smDomSize; i++ )
	    {
    	        loc = std::find( bigDomCheck.begin(), bigDomCheck.end(), 
		    smDomain[i] )- bigDomCheck.begin();
	        if( loc >= bigDomCheck.size() )
	        {
		    PNL_THROW( COutOfRange, 
		        "small domain isn't subset of big" );
	        }
	        bigDomCheck.erase( bigDomCheck.begin()+loc );
            }
        }
	CPotential *resFactor = NULL;
	//we create new factor of the same node types as in big domain
	switch( my_dt )
	{
	case dtTabular:
	    {
		resFactor = CTabularPotential::Create( bigDomain, bigDomSize,
		    GetModelDomain(), NULL, m_obsPositions);
		break;
	    }
	case dtGaussian:
	    {
		resFactor = CGaussianPotential::Create( bigDomain, bigDomSize,
		    GetModelDomain(),-1, NULL, NULL, 0.0f, m_obsPositions);
		break;
	    }
	default:
	    {
		PNL_THROW( CNotImplemented,
		    "we have only Tabular & Gaussian now" );
	    }
	}
        CDistribFun* bigDistr = bigFactor->GetDistribFun();
        CDistribFun* smallDistr = smallFactor->GetDistribFun();
        int smDense = smallDistr->IsDense();
        int bigDense = bigDistr->IsDense();
        if( bigDense == smDense )
        {
	    resFactor->SetDistribFun( bigDistr );
	    resFactor->GetDistribFun()->MultiplyInSelfData( bigDomain,
		smDomain, smallDistr);
        }
        else
        {
	    if( bigDense && !smDense )
	    {
	        CDistribFun* selfSp = bigDistr->ConvertToSparse();
	        resFactor->SetDistribFun(selfSp);
	        delete selfSp;
	        m_CorrespDistribFun->MultiplyInSelfData( &m_Domain.front(),
    	            &smallFactor->m_Domain.front(), smallDistr );
	    }
	    else
	    {
	        if( !bigDense && smDense )
	        {
		    CDistribFun* psmSpDistr = smallDistr->ConvertToSparse();
		    resFactor->GetDistribFun()->MultiplyInSelfData( 
		        &m_Domain.front(), &smallFactor->m_Domain.front(),
		        psmSpDistr );
		    delete psmSpDistr;
	        }
	    }
        }
        return resFactor;
    }
    else
    {
	if(( my_dt == dtScalar )||( other_dt == dtScalar ))
	{
            //can multiply by scalar
	    int s1 = GetDomainSize();
	    int s2 = pOtherFactor->GetDomainSize();
	    const CPotential *bigFactor = NULL;
	    const CPotential *smallFactor = NULL;
	    if (s1 > s2)  
	    {
		bigFactor = this;
		smallFactor = pOtherFactor;
	    }
	    else
	    {
		bigFactor = pOtherFactor;
		smallFactor = this;
	    }
	    //check inclusion of small domain in big
	    int bigDomSize; 
	    const int *bigDomain;
	    bigFactor->GetDomain( &bigDomSize, &bigDomain );
	    intVector bigDomCheck( bigDomain, bigDomain+bigDomSize );
	    int smDomSize; 
	    const int *smDomain;
	    smallFactor->GetDomain( &smDomSize, &smDomain );
	    int loc;
	    for( i = 0; i < smDomSize; i++ )
	    {
		loc = std::find( bigDomCheck.begin(), bigDomCheck.end(), 
			smDomain[i] )- bigDomCheck.begin();
		if( loc >= bigDomCheck.size() )
		{
			PNL_THROW( COutOfRange, 
				"small domain isn't subset of big" );
		}
		bigDomCheck.erase( bigDomCheck.begin()+loc );
	    }
	    CPotential *resFactor = NULL;
	    switch( bigFactor->GetDistributionType() )
	    {
	    case dtTabular:
		{
		    resFactor = CTabularPotential::Copy(
			static_cast<const CTabularPotential*>(bigFactor));
		    resFactor->GetDistribFun()->MultiplyInSelfData( 
			bigDomain, smDomain, smallFactor->GetDistribFun() );
		    break;
		}
	    case dtGaussian:
		{
		    resFactor = CGaussianPotential::Copy(
			static_cast<const CGaussianPotential*>(bigFactor));
                    resFactor->GetDistribFun()->MultiplyInSelfData( 
			bigDomain, smDomain, smallFactor->GetDistribFun() );
		    break;
		}
	    case dtScalar:
		{
		    EDistributionType dtSmall = smallFactor->GetDistributionType();
		    if(( s1 == s2 )&&( dtSmall != dtScalar ))
		    {
			if( dtSmall == dtGaussian )
			{
			    resFactor = CGaussianPotential::Copy(
			        static_cast<const CGaussianPotential*>(smallFactor));
			}
			else if(dtSmall == dtTabular)
			{
			    resFactor = CTabularPotential::Copy(
			        static_cast<const CTabularPotential*>(smallFactor));
			}
			else
			{
			    PNL_THROW( CNotImplemented,
			        "have only Tabular, Gaussian & Scalar potentials now" );
			}
		    }
		    else
		    {
			if( smallFactor->GetDistributionType() == dtScalar )
			{
			    resFactor = CScalarPotential::Copy(
			        static_cast<const CScalarPotential*>(bigFactor) );
			}
			else
			{
			    PNL_THROW( CInconsistentType, 
	       "scalar distribution can't be multiplied by smaller non-scalar" );
			}
		    }
		    break;
		}
	    default:
		{
		    PNL_THROW( CNotImplemented,
			"have only Tabular, Gaussian and Scalar types now" )
		}
	    }
	    return resFactor;
	}
	else
	{
	    PNL_THROW( CNotImplemented, 
		"multiply of different types of factors" );
	}
    }
    return NULL;
}

CPotential& CPotential::operator *=( const CPotential &pSmallPotential )
{
    if( GetModelDomain() != pSmallPotential.GetModelDomain() )
    {
	PNL_THROW( CInconsistentType,
	    "potentials based on the same ModelDomain can be multiplied" )
    }
    EDistributionType my_dt = m_DistributionType;
    EDistributionType other_dt = pSmallPotential.GetDistributionType();
    int s1 = m_Domain.size();
    int s2 = pSmallPotential.m_Domain.size();
    int i;
    if( s1 < s2 )
    {
	PNL_THROW( CInvalidOperation, 
	    "we can multiply only by smaller factor" )
    }
    int smDomSize;
    const int *smDomain;
    pSmallPotential.GetDomain( &smDomSize, &smDomain );
    //check the domain for node size = 2
    if( (s1 == 2) && (s2 == 1))
    {
        if(!(( smDomain[0] == m_Domain[0] )||( smDomain[0] == m_Domain[1])))
        {
            PNL_THROW( COutOfRange, 
	            "small domain isn't subset of big" );
        }
    }
    else
    {
        intVector bigDomCheck( m_Domain );
        int loc;
        for( i = 0; i < smDomSize; i++ )
        {
            loc = std::find( bigDomCheck.begin(), bigDomCheck.end(), 
	        smDomain[i] )- bigDomCheck.begin();
            if( loc >= bigDomCheck.size() )
            {
	        PNL_THROW( COutOfRange, 
	            "small domain isn't subset of big" );
            }
            bigDomCheck.erase( bigDomCheck.begin()+loc );
        }
    }
    if(( my_dt == other_dt )&&(( my_dt == dtTabular )||(my_dt == dtGaussian)))
    {
	CDistribFun* psmallDistr = pSmallPotential.GetDistribFun();
	int bigDense = m_CorrespDistribFun->IsDense();
	int smDense = psmallDistr->IsDense();
	if( bigDense == smDense )
	{
	    m_CorrespDistribFun->MultiplyInSelfData( &m_Domain.front(),
		&pSmallPotential.m_Domain.front(), psmallDistr );
	}
	else
	{
	    if( bigDense && !smDense )
	    {
		CDistribFun* selfSp = m_CorrespDistribFun->ConvertToSparse();
		delete m_CorrespDistribFun;
		m_CorrespDistribFun = selfSp;
		m_CorrespDistribFun->MultiplyInSelfData( &m_Domain.front(),
    	            &pSmallPotential.m_Domain.front(), psmallDistr );
	    }
	    else
	    {
		if( !bigDense && smDense ) 
		{
		    CDistribFun* psmSpDistr = psmallDistr->ConvertToSparse();
		    m_CorrespDistribFun->MultiplyInSelfData( &m_Domain.front(),
			&pSmallPotential.m_Domain.front(), psmSpDistr );
		    delete psmSpDistr;
		}
	    }
	}
    }
    else
    {   
	if( other_dt == dtScalar )
	{
	    return *this;
	}
	if(( my_dt == dtScalar )&&( s1 == s2 ))
	{
	    //need to set other distribution type and check node types
	    CDistribFun* psmallDistr = pSmallPotential.GetDistribFun();
	    intVector smallDom;
	    pSmallPotential.GetDomain(&smallDom);
	    //need to change order of nodes in distribution function if its need!
	    intVector positions;
	    positions.resize(s1);
            int loc;
	    for( i = 0; i < s2; i++ )
	    {
		loc = std::find( smallDom.begin(), smallDom.end(),
		    m_Domain[i] ) - smallDom.begin();
		positions[i] = loc;
	    }
	    m_CorrespDistribFun->MarginalizeData( psmallDistr,
		&positions.front(), s1, 0 );
	}
	else  
	{
	    PNL_THROW(CNotImplemented, "multiply of different types of factors");
	}
    }
    return *this;
}

CPotential& CPotential::operator /=( const CPotential &pSmallPotential )
{
    if( GetModelDomain() != pSmallPotential.GetModelDomain() )
    {
	PNL_THROW( CInconsistentType,
	    "potentials based on the same ModelDomain can be divided" )
    }
    EDistributionType my_dt = m_DistributionType;
    EDistributionType other_dt = pSmallPotential.GetDistributionType();
    if(( my_dt == other_dt )&&(( my_dt == dtTabular )||(my_dt == dtGaussian)))
    {
	int i;
	int s1 = m_Domain.size();
	int s2 = pSmallPotential.m_Domain.size();
	if( s1 < s2 )
	{
	    PNL_THROW( CInvalidOperation, 
		"we can divide only by smaller factor" )
	}
	intVector bigDomCheck( m_Domain );
	int smDomSize;
	const int *smDomain;
	pSmallPotential.GetDomain( &smDomSize, &smDomain );
	int loc;
	for( i = 0; i < smDomSize; i++ )
	{
	    loc = std::find( bigDomCheck.begin(), bigDomCheck.end(), 
		smDomain[i] )- bigDomCheck.begin();
	    if( loc >= bigDomCheck.size() )
	    {
		PNL_THROW( COutOfRange, 
		    "small domain isn't subset of big" );
	    }
	    bigDomCheck.erase( bigDomCheck.begin()+loc );
	}
	CDistribFun* psmallDistr = pSmallPotential.GetDistribFun();
	int bigDense = m_CorrespDistribFun->IsDense();
	int smDense = psmallDistr->IsDense();
	if( bigDense == smDense )
	{
	    m_CorrespDistribFun->DivideInSelfData( &m_Domain.front(),
		&pSmallPotential.m_Domain.front(), psmallDistr );
	}
	else
	{
	    if( bigDense && !smDense )
	    {
		CDistribFun* selfSp = m_CorrespDistribFun->ConvertToSparse();
		delete m_CorrespDistribFun;
		m_CorrespDistribFun = selfSp;
		m_CorrespDistribFun->DivideInSelfData( &m_Domain.front(),
    	            &pSmallPotential.m_Domain.front(), psmallDistr );
	    }
	    else
	    {
		if( !bigDense && smDense ) 
		{
		    CDistribFun* psmSpDistr = psmallDistr->ConvertToSparse();
		    m_CorrespDistribFun->DivideInSelfData( &m_Domain.front(),
			&pSmallPotential.m_Domain.front(), psmSpDistr );
		    delete psmSpDistr;
		}
	    }
	}
    }
    else
    {
	if( other_dt != dtScalar )
	{
	    PNL_THROW( CInvalidOperation,
		"can't divide distributions of different types(except scalar)" );
	}
    }

    return *this;
}

CPotential* CPotential::GetNormalized() const
{
    CDistribFun *normData = m_CorrespDistribFun->GetNormalized();
    EDistributionType myDt = m_CorrespDistribFun->GetDistributionType();
    
    CPotential *pResultFactor = NULL;
    const CPotential* potHere = this;
    switch( myDt )
    {
    case dtTabular:
	{
	    pResultFactor = CTabularPotential::Copy( 
		static_cast<const CTabularPotential*>(potHere) );
	    break;
	}
    case dtGaussian:
	{
	    pResultFactor = CGaussianPotential::Copy( 
		static_cast<const CGaussianPotential*>(potHere));
	    break;
	}
	
    case dtScalar:
	{
	    pResultFactor = CScalarPotential::Copy( 
		static_cast<const CScalarPotential*>(potHere) );
	    break;
	}
    default:
	{
	    PNL_THROW( CNotImplemented, "we have only Tabular & Gaussian now" );
	}
    }
    delete pResultFactor->m_CorrespDistribFun;
    pResultFactor->m_CorrespDistribFun = normData;
    return pResultFactor;
}

void CPotential::Normalize()
{
    m_CorrespDistribFun->Normalize();
}
			                     
CPotential* CPotential::Divide(const CPotential *pOtherFactor) const
{
    if( GetModelDomain() != pOtherFactor->GetModelDomain() )
    {
	PNL_THROW( CInconsistentType,
	    "potentials based on the same ModelDomain can be divided" )
    }
    EDistributionType my_dt=GetDistributionType();
    EDistributionType other_dt=pOtherFactor->GetDistributionType();
    if(( my_dt == other_dt )&&((  my_dt == dtTabular )||
	(my_dt == dtGaussian)))
    {
	int i;
	int s1 = GetDomainSize();
	int s2 = pOtherFactor->GetDomainSize();
	const CPotential *bigFactor = NULL;
	const CPotential *smallFactor = NULL;
	if( s1 >= s2 )  
	{
	    bigFactor = this;
	    smallFactor = pOtherFactor;
	}
	else
	{
	    bigFactor = pOtherFactor;
	    smallFactor = this;
	}
	//check inclusion of small domain in big
	int bigDomSize;
	const int *bigDomain;
	bigFactor->GetDomain( &bigDomSize, &bigDomain );
	intVector bigDomCheck( bigDomain, bigDomain+bigDomSize );
	int smDomSize; 
	const int *smDomain;
	smallFactor->GetDomain( &smDomSize, &smDomain );
	int loc;
	for( i = 0; i < smDomSize; i++ )
	{
	    loc = std::find( bigDomCheck.begin(), bigDomCheck.end(), 
		smDomain[i] )- bigDomCheck.begin();
	    if( loc >= bigDomCheck.size() )
	    {
		PNL_THROW( COutOfRange, 
		    "small domain isn't subset of big" );
	    }
	    bigDomCheck.erase( bigDomCheck.begin() +  loc );
	}
	CPotential *resFactor = NULL;
	//we create new factor of the same node types as in big domain
	switch( my_dt )
	{
	case dtTabular:
	    {
		resFactor = CTabularPotential::Create( bigDomain, bigDomSize, 
		    GetModelDomain(), NULL, m_obsPositions);
		break;
	    }
	case dtGaussian:
	    {
		resFactor = CGaussianPotential::Create( bigDomain, bigDomSize,
		    GetModelDomain(), -1, NULL, NULL, 0.0f, m_obsPositions);
		break;
	    }
	default:
	    {
		PNL_THROW( CNotImplemented,
		    "we have only Tabular & Gaussian now" );
	    }
	}
	CDistribFun* bigDistr = bigFactor->GetDistribFun();
	CDistribFun* smallDistr = smallFactor->GetDistribFun();
	int smDense = smallDistr->IsDense();
	int bigDense = bigDistr->IsDense();
	if( bigDense == smDense )
	{
	    resFactor->SetDistribFun( bigDistr );
	    resFactor->GetDistribFun()->DivideInSelfData( bigDomain,
		smDomain, smallDistr);
	}
	else
	{
	    if( bigDense && !smDense )
	    {
		CDistribFun* selfSp = bigDistr->ConvertToSparse();
		resFactor->SetDistribFun(selfSp);
		delete selfSp;
		m_CorrespDistribFun->DivideInSelfData( &m_Domain.front(),
		    &smallFactor->m_Domain.front(), smallDistr );
	    }
	    else
	    {
		if( !bigDense && smDense )
		{
		    resFactor->SetDistribFun( bigDistr );
		    CDistribFun* psmSpDistr = smallDistr->ConvertToSparse();
		    resFactor->GetDistribFun()->DivideInSelfData( 
			&m_Domain.front(), &smallFactor->m_Domain.front(),
			psmSpDistr );
		    delete psmSpDistr;
		}
	    }
	}
	return resFactor;
    }
    else
    {
	if( other_dt == dtScalar )
	{
	    //can divide by scalar
	    const CPotential *bigFactor = this;
	    const CPotential *smallFactor = pOtherFactor;
	    //check inclusion of small domain in big
	    int bigDomSize; 
	    const int *bigDomain;
	    bigFactor->GetDomain( &bigDomSize, &bigDomain );
	    intVector bigDomCheck( bigDomain, bigDomain+bigDomSize );
	    int smDomSize; 
	    const int *smDomain;
	    smallFactor->GetDomain( &smDomSize, &smDomain );
	    int loc;
	    for( int i = 0; i < smDomSize; i++ )
	    {
		loc = std::find( bigDomCheck.begin(), bigDomCheck.end(), 
		    smDomain[i] )- bigDomCheck.begin();
		if( loc >= bigDomCheck.size() )
		{
		    PNL_THROW( COutOfRange, 
			"small domain isn't subset of big" );
		}
		bigDomCheck.erase( bigDomCheck.begin()+loc );
	    }
	    CPotential *resFactor = NULL;
	    switch( m_DistributionType )
	    {
	    case dtTabular:
		{
		    resFactor = CTabularPotential::Copy(
			static_cast<const CTabularPotential*>(bigFactor));
		    resFactor->GetDistribFun()->DivideInSelfData( 
			bigDomain, smDomain, smallFactor->GetDistribFun() );
		    break;
		}
	    case dtGaussian:
		{
		    resFactor = CGaussianPotential::Copy(
			static_cast<const CGaussianPotential*>(bigFactor));
		    resFactor->GetDistribFun()->DivideInSelfData( 
			bigDomain, smDomain, smallFactor->GetDistribFun() );
		    break;
		}
	    case dtScalar:
		{
		    EDistributionType dtSmall = smallFactor->GetDistributionType();
		    if( dtSmall == dtScalar )
		    {
			resFactor = CScalarPotential::Copy(
			    static_cast<const CScalarPotential*>(bigFactor) );
		    }
		    else
		    {
			PNL_THROW( CInconsistentType, 
			    "scalar distribution can't be divided by smaller non-scalar" );
		    }
		    break;
		}
	    default:
		{
		    PNL_THROW( CNotImplemented,
			"have only Tabular, Gaussian and Scalar types now" )
		}
	    }
	    return resFactor;
	}
	else
	{
	    if( my_dt == dtScalar )
	    {
		//can divide only if the other distribution is of the same size
	    }
	    else
	    {
		PNL_THROW( CNotImplemented,
		    "multiply of different types of factors" );
		
	    }
	}
    }
    return NULL;
}

int
CPotential::GetMultiplyedDelta(const int **positions, const float **values,
			       const int **offsets )const
{
    int i;
    i = m_CorrespDistribFun->GetMultipliedDelta( positions, values, offsets );
    return i;
}

void CPotential::GetMultipliedDelta( intVector* positions,
			            floatVector* values, intVector* offsets ) const
{
    const int* pos;
    const float* vals;
    const int* offs;
    int size = GetMultiplyedDelta( &pos, &vals, &offs );
    positions->assign( pos, pos + size );
    values->assign( vals, vals + size );
    offsets->assign( offs, offs + size );
}

void CPotential::SetDump(const char *fileName)
{
    pnl::SetDump(fileName);
}

Log& CPotential::dump()
{
    return *LogPotential();
}

#ifdef PNL_RTTI
const CPNLType CPotential::m_TypeInfo = CPNLType("CPotential", &(CFactor::m_TypeInfo));

#endif

/*
void CPotential::GenerateSample( CEvidence* evidence, unsigned int seed, int maximize  ) const
{
    PNL_THROW(CNotImplemented , "not implemented for undirected models");
}
*/
