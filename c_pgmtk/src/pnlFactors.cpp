/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlFactors.cpp                                              //
//                                                                         //
//  Purpose:   CFactors class member functions implementation              //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlFactors.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlGaussianCPD.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlMixtureGaussianCPD.hpp"
#include "pnlScalarPotential.hpp"
#include "pnlSoftMaxCPD.hpp"

PNL_USING

CFactors* CFactors::Create(int numberOfFactors)
{
    /* bad-args check */
    PNL_CHECK_LEFT_BORDER( numberOfFactors, 1 );
    /* bad-args check end */
    
    CFactors *pFactors = new CFactors(numberOfFactors);
    
    PNL_CHECK_IF_MEMORY_ALLOCATED(pFactors);
    
    return pFactors;
}

CFactors* CFactors::Copy(const CFactors *pFactors)
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pFactors);
    // bad-args check end
    
    CFactors *pFactorsCopy = new CFactors(*pFactors);
    
    PNL_CHECK_IF_MEMORY_ALLOCATED(pFactorsCopy);
    
    return pFactorsCopy;
}

CFactors::CFactors(int numberOfFactors) : m_capacity(numberOfFactors)
{
    m_facts.reserve(numberOfFactors);
}

CFactors::CFactors(const CFactors& rFactors)
                   : m_capacity(rFactors.m_capacity)
{
    m_facts.reserve(m_capacity);
    
    m_facts.resize( rFactors.m_facts.size(), NULL );
    
    pFactorVector::iterator cpFactIt = m_facts.begin();

    pFactorVector::const_iterator factIt   = rFactors.m_facts.begin(),
                                  fact_end = rFactors.m_facts.end();
    
    for( ; factIt != fact_end; ++factIt, ++cpFactIt )
    {
        switch( (*factIt)->GetFactorType() )
        {
            case ftPotential:
            {
                switch( (*factIt)->GetDistributionType() )
                {
                    case dtTabular:
                    {
                        *cpFactIt = CTabularPotential::Copy(
                            static_cast<CTabularPotential *>(*factIt));
                        break;
                    }
                    case dtGaussian:
                    {
                        *cpFactIt = CGaussianPotential::Copy(
                            static_cast<CGaussianPotential *>(*factIt));
                        break;
                    }
                    case dtScalar:
                    {
                        *cpFactIt = CScalarPotential::Copy(
                            static_cast<CScalarPotential *>(*factIt));
                        break;
                    }
                    default:
                    {
                        PNL_THROW( CNotImplemented,
                            " no copy ctor for this type of factor " );
                        break;
                    }
                }
                
                break;
            }
            case ftCPD:
            {
                switch( (*factIt)->GetDistributionType() )
                {
                    case dtTabular:
                    {
                        *cpFactIt = CTabularCPD::Copy(
                            static_cast<CTabularCPD *>(*factIt));
                        break;
                    }
                    case dtGaussian:
                    {
                        *cpFactIt = CGaussianCPD::Copy(
                            static_cast<CGaussianCPD *>(*factIt));
                        break;
                    }
                    case dtMixGaussian:
                    {
                        *cpFactIt = CMixtureGaussianCPD::Copy(
                            static_cast<CMixtureGaussianCPD *>(*factIt));
                        break;
                    }

                    case dtSoftMax:
                    {
                        *cpFactIt = CSoftMaxCPD::Copy(
                            static_cast<CSoftMaxCPD *>(*factIt));
                        break;
                    }

                    default:
                    {
                        PNL_THROW( CNotImplemented,
                            " no copy ctor for this type of factor " );
                        break;
                    }
                }
                
                break;
            }
            default:
            {
                PNL_THROW( CInvalidOperation,
                    " CFactors object can either contain Pots or CPDs " );
                break;
            }
        }
    }
}

CFactors::~CFactors()
{
    pFactorVector::const_iterator factIt   = m_facts.begin(),
        fact_end = m_facts.end();
    
    for( ; factIt != fact_end; ++factIt )
    {
        delete *factIt;
    }
}

void CFactors::ShrinkObsNdsForAllFactors(const CEvidence *pEvidence)
{
/*	// bad-args check
PNL_CHECK_IS_NULL_POINTER(pEvidence);
// bad-args check end

  intVector actuallyObsNodes;
  pnlVector< const unsigned char* > allOffsets;
  intVector obsNodesInDomain;
  // should space for these two vectors be reserved?
  
    pEvidence->GetObsNodesWithValues( &actuallyObsNodes, &allOffsets );
    
      for( pFactorVector::const_iterator factIt = m_facts.begin(),
      fact_end = m_facts.end(); fact_end - factIt; ++factIt )
      {
      // if an actually observed node is one of the nodes in domain, the
      // factor on this domain should be shrinked
      
        int       numOfNdsInDomain;
        const int *factDomain, *domIt, *dom_end;
        
          (*factIt)->GetDomain( &numOfNdsInDomain, &factDomain );
          
            for( domIt = factDomain, dom_end = factDomain + numOfNdsInDomain;
            domIt != dom_end; ++domIt )
            {
            if(std::find( actuallyObsNodes.begin(), actuallyObsNodes.end(),
            *domIt ) != actuallyObsNodes.end() )
            {
            obsNodesInDomain.push_back(*domIt);
            }
            }
            
              if( !obsNodesInDomain.empty() )
              {
              // here is a line, why it all should not work for a cond gaussian
              const CNodeType *pObsNodeNT = 
              m_pJTree->GetNodeType(i)->IsDiscrete()
              ? &m_ObsNodeType : &m_ObsGaussType;
              
                const CPotential *pShrPot = (*factIt)->ShrinkObservedNodes(
                obsNodesInDomain.size(), obsNodesInDomain.begin(),
                allOffsets.begin(), pObsNodeNT );
                
                  // this should be changed to perform a safe operation
                  m_pJTree->GetNodePotential(i)->SetDistribFun(
                  pShrPot->GetDistribFun());
                  
                    delete pShrPot;
                    }
}*/
}

#ifdef PNL_RTTI
const CPNLType CFactors::m_TypeInfo = CPNLType("CFactors", &(CPNLBase::m_TypeInfo));

#endif
