/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlInferenceEngine.cpp                                      //
//                                                                         //
//  Purpose:   CInferenceEngine class member functions implementation      //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// InfEngine.cpp
#include "pnlConfig.hpp"
#include "pnlInferenceEngine.hpp"
#include "pnlEvidence.hpp"
#include "pnlMNet.hpp"

PNL_BEGIN

//determine distribution type of set of nodes if some of them are observed now
EDistributionType
pnlDetermineDistributionType( int nAllNodes, int numberObsNodes,
                              int* pObsNodesIndices,
                              const CNodeType * const* pAllNodesTypes )
{
    // distribution type of factor is:
    // dtTabular, if all non Observed (hidden) nodes are discrete;
    // dt Gaussian, if all hidden nodes are continious (gaussian);
    // dtCondGaussian, if some of them are discrete and some continious
//      return dtTree;
    if( ( nAllNodes < numberObsNodes ) || (nAllNodes < 0 ) || ( numberObsNodes < 0 ) )
    {
        PNL_THROW( COutOfRange, "number of all nodes" );
        return dtInvalid;
    }
    
    int i;
    EDistributionType myDT;
    intVector IsObservedNow = intVector( nAllNodes, 0 );
    
    // If node K is observed, IsObservedNow[K] = 1, 0 owervise
    intVector IsDiscreteNodes;
    // to support numberObsNodes=0 - we fill these arrays
    int numDiscrNodes = 0;
    int numContNodes = 0;

    for( i = 0; i < nAllNodes; ++i )
    {
        if( ( pAllNodesTypes[i] )->IsDiscrete() ) 
        {
            IsDiscreteNodes.push_back(1);
            numDiscrNodes++;
        }
        else 
        {
            IsDiscreteNodes.push_back(0);
            numContNodes++;
        }
    }

    /*for nObsNodes>0 - we fill some data from it*/
    for( i = 0; i < numberObsNodes; ++i )
    {
        IsObservedNow[pObsNodesIndices[i]] = 1;
    }

    /*for both cases: numberObsNodes>=0*/
    intVector isHiddenDiscrete;
    /*we need information only about hidden nodes*/
    
    for( i = 0; i < nAllNodes; i++ )
    {
        if(( !IsObservedNow[i] ))
        {
            isHiddenDiscrete.push_back( IsDiscreteNodes[i] );
        }
    }

    int allDiscrete = 0;/*flag :all hidden nodes are discrete*/
    int allContinious = 0;/*flag: all hidden nodes are gaussian*/
    
    if( isHiddenDiscrete.size() == 0 )
    {   
        //all nodes are observed here 
        myDT = dtScalar;
    }
    else
    {
        for( i=0; i<isHiddenDiscrete.size(); i++ )
        {
            if ( isHiddenDiscrete[i] ) 
            {
                allDiscrete++;
            }
            else
            {
                allContinious++;
            }
        }
        /*determine distribution type according the rule*/
        if( allDiscrete == int(isHiddenDiscrete.size()))
        {
            myDT = dtTabular;
        }
        else
        {
            if( allContinious == int(isHiddenDiscrete.size())) 
            {
                myDT = dtGaussian;
            }
            else
            {
                if (pAllNodesTypes[nAllNodes-1]->IsDiscrete() && 
                    allContinious == (int(isHiddenDiscrete.size())-1))
                {
                    myDT = dtSoftMax;
                }
                else
                {
                    if (pAllNodesTypes[nAllNodes-1]->IsDiscrete() && 
                    allContinious != int(isHiddenDiscrete.size()))
                    {
                        myDT = dtCondSoftMax;
                    }
                    else
                    {
                        myDT = dtCondGaussian;

                    }
                }
            }
        }
    }
    
    return myDT;
}

//determine distribution type of set of nodes if some of them are observed now
EDistributionType
pnlDetermineDistributionType( const CModelDomain* pMD, int nQueryNodes,
                              const int* query, const CEvidence* pEvidence)
{
    PNL_CHECK_IS_NULL_POINTER( pMD );
    PNL_CHECK_IS_NULL_POINTER( query );

//      return dtTree;
    int numNodesInDomain = pMD->GetNumberVariables();
    
    PNL_CHECK_RANGES( nQueryNodes, 0, numNodesInDomain  );
    
    EDistributionType dt = dtInvalid;
    
    intVector observability( nQueryNodes, 0 );
    
    intVector isTabNodes( nQueryNodes, 0 );

    int i;
    int numTabNodes = 0;

    for( i = 0; i < nQueryNodes; i++ )
    {
        if( pMD->GetVariableType(query[i])->IsDiscrete() )
        {
            isTabNodes[i] = 1;
            numTabNodes++;
        }
    }
    int numObsNodes = 0;
    if( pEvidence )
    {
        for( i = 0; i < nQueryNodes; i++ )
        {
            if( pEvidence->IsNodeObserved(query[i]) )
            {
                observability[i] = 1;
                numObsNodes++;
            }
        }
    }
    if( numObsNodes == nQueryNodes )
    {
        dt = dtScalar;
        return dt;
    }
    int numTabHidden = 0;
    int numGauHidden = 0;
    for( i = 0; i < nQueryNodes; i++ )
    {
        if (!observability[i])
        {
            if(isTabNodes[i])
            {
                numTabHidden++;
            }
            else
            {
                numGauHidden++;
            }
        }
    }
    int numAllHidden =  nQueryNodes - numObsNodes;
    if( numAllHidden == numTabHidden )
    {
        dt = dtTabular;
    }
    else
    {
        if( numAllHidden == numGauHidden )
        {
            dt = dtGaussian;
        }
        else
        {
            dt = dtCondGaussian;
        }
    }
    return dt;
}

CNodeType CInfEngine::m_ObsTabNodeType( 1, 1 );

CNodeType CInfEngine::m_ObsGaussNodeType( 0, 0 );

const CNodeType* CInfEngine::GetObsTabNodeType()
{
    return &m_ObsTabNodeType;
}

const CNodeType* CInfEngine::GetObsGauNodeType()
{
    return &m_ObsGaussNodeType;
}

CInfEngine::CInfEngine( EInfTypes infType, const CStaticGraphicalModel* pGM )
                        : m_bMaximize(0), m_pEvidence(NULL),
                        m_pQueryJPD(NULL), m_pEvidenceMPE(NULL)
{
    int i;
    int numFactors = pGM->GetNumberOfFactors();

    for( i = 0; i < numFactors; ++i )
    {
        const CFactor* pFact = pGM->GetFactor(i);
        
        if( !pFact )
        {
            PNL_THROW( CInvalidOperation, "inference on invalid model" );
        }
        else
        {
            if( !pFact->IsValid() )
            {
                PNL_THROW( CInvalidOperation, "inference on invalid model" );
            }
        }
    }
    
    if( pGM->GetModelType() == mtBNet )
    {
        if( pGM->GetNumberOfNodes() != numFactors)
        {
            PNL_THROW( CInvalidOperation,
                "we can't start infererence without valid factors" );
        }
    }
    else
    {
        if( ( pGM->GetModelType() == mtMNet )
            || ( pGM->GetModelType() == mtMRF2 ) )
        {
            if( static_cast<const CMNet*>(pGM)->GetNumberOfCliques()
                != numFactors )
            {
                PNL_THROW( CInvalidOperation,
                    "we can't start infererence without valid factors" );
            }
        }
    }
    
    m_InfType         = infType;
    
    m_pGraphicalModel = pGM;
    
    m_pEvidenceMPE    = NULL;
}

CInfEngine::~CInfEngine()
{
    delete m_pEvidenceMPE;
    
    delete m_pQueryJPD;
}

void CInfEngine::MarginalNodes( const intVector& queryNds, int notExpandJPD )
{
    if( queryNds.empty() )
    {
        PNL_THROW( CInconsistentSize,
            " query nodes vector should not be empty " );
    }
    
    MarginalNodes( &queryNds.front(), queryNds.size(), notExpandJPD );
}

const CEvidence* CInfEngine::GetMPE() const
{
    if( !m_bMaximize )
    {
        PNL_THROW( CInvalidOperation,
            " MPE - only for case of maximization " );
    }
    
    return m_pEvidenceMPE;
}

const CPotential* CInfEngine::GetQueryJPD() const
{
    if( m_bMaximize )
    {
        PNL_THROW( CInvalidOperation, " JPD - only for case of sum " );
    }
    
    return m_pQueryJPD;
}

/*the previous version of this function - it was the method of inference engine*/
/*EDistributionType CInfEngine::pnlDetermineDistributionType()
{
int i;
EDistributionType myDT;
intVector IsObservedNow = intVector(nAllNodes, 0);;
*/
/*If node K is observed, IsObservedNow[K] = 1, 0 owervise*/
/*      intVector IsDiscreteNodes;
int nAllNodes = m_pGraphicalModel->pgmGetNumberOfNodes();
pConstNodeTypeVector NodeTypes;
*/
/*for Inf without evidence*/
/*      int numAllObsNodes = 0;
const int *allObsNodes = NULL; 
intVector IsObservedNowFromEvidence; 
*/
/*for Inf with evidence*/
/*      if(m_pEvidence)
{
numAllObsNodes = m_pEvidence->GetNumberObsNodes();
allObsNodes = m_pEvidence->GetAllObsNodes(); 
IsObservedNowFromEvidence = intVector(m_pEvidence->GetObsNodesFlags(),
m_pEvidence->GetObsNodesFlags()+numAllObsNodes);//fixme - it is bad style!!!
}
*/
/*for all types of infernce: with/without evidence*/
/*      for (i=0; i<nAllNodes;i++)
{
NodeTypes.push_back(m_pGraphicalModel->pgmGetNodeType(i));
if((NodeTypes[i])->IsDiscrete()) IsDiscreteNodes.push_back(1);
else IsDiscreteNodes.push_back(0);
}

  for (i=0; i<numAllObsNodes; i++)
  {
                IsObservedNow[allObsNodes[i]] = IsObservedNowFromEvidence[i];
                }
                intVector isHiddenDiscrete;
                for (i=0; i<nAllNodes; i++)
                {
                if(!IsObservedNow[i]) isHiddenDiscrete.push_back(IsDiscreteNodes[i]);
                }
                int allDiscrete = 0;
                int allContinious = 0;
                for (i=0; i<isHiddenDiscrete.size();i++)
                {
                if (isHiddenDiscrete[i]) 
                allDiscrete++;
                else allContinious++;
                }
                if(allDiscrete == isHiddenDiscrete.size()) myDT = dtTabular;
                else
                {
                if (allContinious == isHiddenDiscrete.size()) myDT = dtGaussian;
                else
                myDT = dtCondGaussian;
                }
                isHiddenDiscrete.clear();
                IsObservedNowFromEvidence.clear();
                IsObservedNow.clear();
                IsDiscreteNodes.clear();
                NodeTypes.clear();
                return myDT;
                }
                */


#ifdef PNL_RTTI
const CPNLType CInfEngine::m_TypeInfo = CPNLType("CInfEngine", &(CPNLBase::m_TypeInfo));

#endif


PNL_END
