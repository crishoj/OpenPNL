/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlGibbsSamplingInferenceEngine.cpp                         //
//                                                                         //
//  Purpose:   CGibbsSamplingInfEngine class member functions              //
//             implementation                                              //
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
#include "pnlGibbsSamplingInferenceEngine.hpp"
#include "pnlException.hpp"
#include "pnlSoftMaxCPD.hpp"
#include "pnlSoftMaxDistribFun.hpp"
#include "pnlCondSoftMaxDistribFun.hpp"
#include "pnlScalarPotential.hpp"
#include <time.h>

PNL_USING

CGibbsSamplingInfEngine * CGibbsSamplingInfEngine::Create( 
                                                          const CStaticGraphicalModel *pGraphicalModel )
{
  if( !pGraphicalModel )
  {
    PNL_THROW( CNULLPointer, "graphical model" );
    return NULL;
  }
  else
  {
    CGibbsSamplingInfEngine* newInf = new CGibbsSamplingInfEngine( pGraphicalModel );
    return newInf;
  }
}


CGibbsSamplingInfEngine::CGibbsSamplingInfEngine( const CStaticGraphicalModel
                                                 *pGraphicalModel )
                                                 :CSamplingInfEngine( pGraphicalModel ),
                                                 m_queryNodes( 0 ),
                                                 m_queryes(0),
                                                 m_queryFactors(0),
                                                 m_pPotMPE(NULL),
                                                 m_bUsingDSep(false)
                                                 
                                                 
{
  m_bMaximize = 0;
  Initialization();
  m_SoftMaxGaussianFactors.resize(pGraphicalModel->GetNumberOfNodes(), NULL);
  FindEnvironment(&m_environment);
  m_NSamplesForSoftMax = 10;
  m_MaxNSamplesForSoftMax = 1000000;
}

CGibbsSamplingInfEngine::~CGibbsSamplingInfEngine()
{
  DestroyQueryFactors();
  delete m_pQueryJPD;
  m_pQueryJPD = NULL;
  delete m_pPotMPE;
  m_pPotMPE = NULL;
  int NNodes = m_pGraphicalModel->GetNumberOfNodes();
  for (int node = 0; node < NNodes; node++) {
    if (m_SoftMaxGaussianFactors[node]!=NULL) {
      delete m_SoftMaxGaussianFactors[node];
      m_SoftMaxGaussianFactors[node] = NULL;
    }
  }
}


void CGibbsSamplingInfEngine::UseDSeparation(bool isUsing)
{
  if( GetModel()->GetModelType() != mtBNet )
  {
    PNL_THROW(CNotImplemented, "d-separation have been realized for DAG" );
  }
  m_bUsingDSep = true;
}

void CGibbsSamplingInfEngine::Initialization()
{
  
  int nFactors = GetModel()->GetNumberOfFactors();
  pFactorVector *currentFactors = GetCurrentFactors();
  currentFactors->resize(nFactors);
  int i;
  for( i = 0; i < nFactors; i++ )
  {
    (*currentFactors)[i] = GetModel()->GetFactor(i);
  }
  
}

void CGibbsSamplingInfEngine::SetQueries(intVecVector &queryes)
{
  int nQueryes = queryes.size();
  PNL_CHECK_LEFT_BORDER( nQueryes, 1 );
  
  int i;
  for( i = 0; i < m_queryFactors.size(); i++ )
  {
    delete m_queryFactors[i];
  }
  
  intVector tmp;
  for( i = 0; i < nQueryes; i++ )
  {
    PNL_CHECK_RANGES( queryes[i].size(), 1, m_pGraphicalModel->GetNumberOfNodes() );
    tmp = queryes[i];
    std::sort( tmp.begin(), tmp.end() );
    intVector::iterator it = std::unique( tmp.begin(), tmp.end() );
    tmp.erase( it, tmp.end() );
    if( tmp.size() != queryes[i].size() )
    {
      PNL_THROW(CAlgorithmicException, "equal nodes in qurey");
    }
    tmp.clear();
  }
  m_queryes = queryes;    
  
  
}

void CGibbsSamplingInfEngine::DestroyQueryFactors()
{
  pFactorVector *queryFactors = GetQueryFactors();
  int i;
  for( i = 0; i < queryFactors->size(); i++ )
  {
    delete (*queryFactors)[i];
  }
  queryFactors->clear();
}

void CGibbsSamplingInfEngine::
EnterEvidence( const CEvidence *pEvidenceIn, int maximize, int sumOnMixtureNode )
{
  if( !m_queryes.size() )
  {
    PNL_THROW( CAlgorithmicException, "Possible queryes must be defined");
  }
  
  PNL_CHECK_IS_NULL_POINTER(pEvidenceIn);
  m_pEvidence = pEvidenceIn;
  m_bMaximize = maximize;
  
  DestroyCurrentEvidences();
  DestroyQueryFactors();
  
  
  if(GetModel()->GetModelType() == mtBNet)
  {
    static_cast< const CBNet* >(GetModel())->
      GenerateSamples( GetCurrentEvidences(), GetNumStreams(), pEvidenceIn );
  }
  else
  {
    static_cast< const CMNet* >(GetModel())->
      GenerateSamples( GetCurrentEvidences(), GetNumStreams(), pEvidenceIn );
    
  }
  
  CreateQueryFactors();
  
  boolVector sampleIsNeed;
  if( m_bUsingDSep )
  {
    ConsDSep( m_queryes, &sampleIsNeed, m_pEvidence );
  }
  else
  {
    FindCurrentNdsForSampling( &sampleIsNeed );
  }
  SetSamplingNdsFlags(sampleIsNeed);
  
  Sampling( 0, GetMaxTime() );
}

void CGibbsSamplingInfEngine::
Sampling( int statTime, int endTime )
{
  
  intVector ndsForSampling;
  GetNdsForSampling( &ndsForSampling );
  boolVector sampleIsNeed;
  GetSamplingNdsFlags( &sampleIsNeed );
  
  int numNdsForSampling = ndsForSampling.size();
  
  pEvidencesVector currentEvidences;
  GetCurrentEvidences( &currentEvidences );
  CEvidence * pCurrentEvidence;
  
  int t; 
  int i;
  for( t = statTime; t < endTime; t++ )
  {
    int series;
    for( series = 0; series < GetNumStreams(); series++ )
    {
      pCurrentEvidence = currentEvidences[series];
      
      
      for( i = 0; i < numNdsForSampling; i++ )
      {
        if( sampleIsNeed[i] )
        {
          pCurrentEvidence->ToggleNodeStateBySerialNumber(1, &i);
          bool canBeSample = ConvertingFamilyToPot( ndsForSampling[i], pCurrentEvidence );
          if(canBeSample)
          {
            
            GetPotToSampling(ndsForSampling[i])->GenerateSample( pCurrentEvidence, m_bMaximize );
          }
          else
          {
            pCurrentEvidence->ToggleNodeStateBySerialNumber(1, &i);
          }
        }
        
      }	
      
    }
    if( t > GetBurnIn())
    {
      pFactorVector queryFactors;
      GetQueryFactors( &queryFactors );
      int i;
      for( i = 0; i < queryFactors.size(); i++ )
      {
        queryFactors[i]->UpdateStatisticsML( &(GetCurrentEvidences()->front()), GetNumStreams() );
      }  
      
    }
  }
  
}


void CGibbsSamplingInfEngine::
MarginalNodes( const intVector& queryNdsIn, int notExpandJPD  )
{
  MarginalNodes( &queryNdsIn.front(), queryNdsIn.size(), notExpandJPD );
}

void CGibbsSamplingInfEngine::
MarginalNodes( const int *queryIn, int querySz, int notExpandJPD )
{
  delete m_pQueryJPD;
  m_pQueryJPD = NULL;
  
  delete m_pPotMPE;
  m_pPotMPE = NULL;
  
  delete m_pEvidenceMPE;
  m_pEvidenceMPE = NULL;
  
  const CFactor *pFactor;
  CPotential *pPot =  NULL;
  int *begin1;
  int *end1;
  int *begin2;
  int *end2;
  
  intVector domainVec;
  intVector queryVec;
  intVector obsQueryVec;
  queryVec.reserve(querySz);
  obsQueryVec.reserve(querySz);
  int i;
  for( i = 0; i < querySz; i++ )
  {
    m_pEvidence->IsNodeObserved(queryIn[i]) ? 
      obsQueryVec.push_back(queryIn[i]):
    queryVec.push_back(queryIn[i]);
  }
  
  
  CPotential *tmpPot = NULL;
  
  if( queryVec.size() )
  {
    for( i = 0; i < m_queryFactors.size(); i++)     
    {
      
      domainVec.clear();
      pFactor = m_queryFactors[i];
      pFactor->GetDomain(&domainVec);
      begin1 = &domainVec.front();
      end1 = &domainVec.back() + 1;
      std::sort(begin1, end1);
      
      begin2 = &queryVec.front();
      end2 = &queryVec.back() + 1;
      std::sort(begin2, end2);
      
      if( std::includes(begin1, end1, begin2, end2) )
      {
        pPot = pFactor->ConvertStatisticToPot( (GetMaxTime()-GetBurnIn()-1)*GetNumStreams() );
        tmpPot = pPot->Marginalize( queryVec );
        delete pPot;
        break;
      }
      		   
    }
    if( !tmpPot )
    {
      PNL_THROW(CInvalidOperation, "Invalid query");
    }
  }
  delete m_pQueryJPD; 
  
  if( obsQueryVec.size() )
  {
    
    EDistributionType paramDistrType = 
      pnlDetermineDistributionType( GetModel()->GetModelDomain(), querySz, queryIn, m_pEvidence);
    
    
    CPotential *pQueryPot;
    switch( paramDistrType )
    {
    case dtTabular:
      {
        pQueryPot = CTabularPotential::CreateUnitFunctionDistribution(
          queryIn, querySz, m_pGraphicalModel->GetModelDomain() );
        break;
      }
      
    case dtGaussian:
      {
        pQueryPot = CGaussianPotential::CreateUnitFunctionDistribution(
          queryIn, querySz, m_pGraphicalModel->GetModelDomain()  );
        break;
      }
    case dtScalar:
      {
        pQueryPot = CScalarPotential::Create(
          queryIn, querySz, m_pGraphicalModel->GetModelDomain()  );
        break;
      }
    case dtCondGaussian:
      {
        PNL_THROW( CNotImplemented, "conditional gaussian factors" )
          break;
      }
    default:
      {
        PNL_THROW( CInconsistentType, "distribution type" )
      }
    }
    
    if( tmpPot)
    {
      (*pQueryPot) *= (*tmpPot);
      delete tmpPot;
    }
    
    if( m_bMaximize )
    {
      m_pPotMPE   = static_cast<CPotential*>
        ( pQueryPot->ExpandObservedNodes( m_pEvidence, 0) );
      
      m_pEvidenceMPE = m_pPotMPE->GetMPE();
    }
    else
    {
      m_pQueryJPD = static_cast<CPotential*>( pQueryPot->ExpandObservedNodes( m_pEvidence, 0) );
    }
    
    delete pQueryPot;
  }
  else
  {
    if( m_bMaximize )
    {
      m_pPotMPE = tmpPot;
      m_pEvidenceMPE = m_pPotMPE->GetMPE();
    }
    else
    {
      m_pQueryJPD = tmpPot;
      
    }
  }
  
  
}

const CPotential* CGibbsSamplingInfEngine::
GetQueryJPD() const
{
  if( !m_pQueryJPD )
  {
    PNL_THROW( CInvalidOperation,
      " can't call GetQueryJPD() before calling MarginalNodes() " );
  }
  
  return m_pQueryJPD;
}

const CEvidence* CGibbsSamplingInfEngine::GetMPE() const
{
  if( !m_bMaximize )
  {
    PNL_THROW( CInvalidOperation,
      " you have not been computing the MPE ");
  }
  
  if( !m_pEvidenceMPE )
  {
    PNL_THROW( CInvalidOperation,
      " can't call GetMPE() before calling MarginalNodes() " );
  }
  
  return m_pEvidenceMPE;
}



void CGibbsSamplingInfEngine::CreateQueryFactors()
{
  pConstNodeTypeVector ntVec;
  const CNodeType *nt;
  intVector query;
  for( int number = 0; number < m_queryes.size(); number++ )
  {
    for( int node = 0; node < m_queryes[number].size(); node++)
    {
      if( !m_pEvidence->IsNodeObserved(m_queryes[number][node]) )
      {
        query.push_back(m_queryes[number][node]);
        nt = m_pGraphicalModel->GetNodeType(m_queryes[number][node]);
        if ( ntVec.size() )
        {
          if(  !( ( nt->IsDiscrete() && ntVec.back()->IsDiscrete() ) ||
            ( !nt->IsDiscrete() && !ntVec.back()->IsDiscrete() ) ))
            
          {
            PNL_THROW(CAlgorithmicException, "invalid query");
          }
        }
        ntVec.push_back( nt );
      }
      
    }
    if( query.size() )
    {
      if( ntVec[0]->IsDiscrete() )
      {
        m_queryFactors.push_back( CTabularPotential::Create( query, 
          m_pGraphicalModel->GetModelDomain(), NULL ) );
      }
      else
      {
        m_queryFactors.push_back( CGaussianPotential::Create( query, 
          m_pGraphicalModel->GetModelDomain(), 0 ) );
      }
    }
    ntVec.clear();
    query.clear();
  }
  
  int NNodes = m_pGraphicalModel->GetNumberOfNodes();
  CGraph *pGraph = m_pGraphicalModel->GetGraph();
  intVector Children;
  
  for (int node = 0; node < NNodes; node++) 
  {
    nt = m_pGraphicalModel->GetNodeType(node);
    
    if ((!nt->IsDiscrete())&&(!(m_pEvidence->IsNodeObserved(node))))
    {
      pGraph->GetChildren(node, &Children);
      
      for (int child = 0; child < Children.size(); child++) 
      {
        nt = m_pGraphicalModel->GetNodeType(Children[child]);
        
        if (nt->IsDiscrete()) {
          m_SoftMaxGaussianFactors[node] = CGaussianPotential::Create(&node, 1, m_pGraphicalModel->GetModelDomain());
          break;
        };
      }
    }
  }
}

void CGibbsSamplingInfEngine::
ConsDSep(intVecVector &allNds, boolVector *sampleIsNeed, const CEvidence *pEv ) const
{
  
  intVector ndsForSample;
  GetNdsForSampling(&ndsForSample);
  
  int nnodes = pEv->GetNumberObsNodes();
  const int* obsNds = pEv->GetAllObsNodes();
  const int* flags = pEv->GetObsNodesFlags();
  intVector separator;
  separator.reserve(nnodes);
  
  int i;
  for( i = 0; i < nnodes; i++ )
  {
    if( flags[i] )
    {
      separator.push_back( obsNds[i] );
    }
  }
  
  
  const CGraph *pGraph = GetModel()->GetGraph();   
  boolVector flagsDSep( pGraph->GetNumberOfNodes() );
  
  intVector dsep;
  
  for( i = 0; i < allNds.size(); i++ )
  {
    int j;
    for( j = 0; j < allNds[i].size(); j++ )
    {
      dsep.clear();
      pGraph->GetDConnectionList( (allNds[i])[j], separator, &dsep );
      
      int k;
      for( k = 0; k < dsep.size(); k++ )
      {
        flagsDSep[dsep[k]] = true;
      }
    }
  }
  
  FindCurrentNdsForSampling( sampleIsNeed );
  for( i = 0; i < ndsForSample.size(); i++ )
  {
    (*sampleIsNeed)[i] = (*sampleIsNeed)[i] && flags[ndsForSample[i]];
  }
  
}


bool CGibbsSamplingInfEngine::
ConvertingFamilyToPot( int node, const CEvidence* pEv )
{
  bool ret = false;
  CPotential* potToSample = GetPotToSampling(node);
  Normalization(potToSample);
  int i;
  bool isTreeNode = false;

  //Has node got discrete child with more than 2 values
  bool bHasNodeGotDChldWMrThn2Vl = false;

  if( GetModel()->GetModelType() == mtBNet )
  {
      for (int ii = 0; ((ii < (m_environment[node].size()-1))&&(!bHasNodeGotDChldWMrThn2Vl)); ii++) {
	int num = m_environment[node][ii];
	if ((GetModel()->GetNodeType(num)->IsDiscrete())&&(GetModel()->GetNodeType(num)->GetNodeSize() > 2))
	  bHasNodeGotDChldWMrThn2Vl = true;
      };
  };
  
  int *obsNds = NULL;
  valueVector obsVals;
  CEvidence *pSoftMaxEvidence = NULL; 
  int NNodes = m_pGraphicalModel->GetNumberOfNodes();
  

  if( GetModel()->GetModelType() == mtBNet )
  {
      if ((bHasNodeGotDChldWMrThn2Vl)&&(m_SoftMaxGaussianFactors[node] != NULL)) 
      {
	obsVals.resize(NNodes);
	obsNds = new int[NNodes];
    
	const_cast<CEvidence*> (pEv)->ToggleNodeState(1, &node);
    
	for (i = 0; i < NNodes; i++) 
	{
	  const CNodeType *nt = m_pGraphicalModel->GetNodeType(i);
	  if (nt->IsDiscrete()) 
	    obsVals[i].SetInt(pEv->GetValue(i)->GetInt()); 
	  else 
	    obsVals[i].SetFlt(pEv->GetValue(i)->GetFlt()); 
      
	  obsNds[i] = i;
	}
    
	const_cast<CEvidence*> (pEv)->ToggleNodeState(1, &node);
      }
  };
  
  if( GetModel()->GetModelType() == mtBNet )
  {
      for( i = 0; i < m_environment[node].size(); i++ )
      {            
	int num = m_environment[node][i];
	if ( (*GetCurrentFactors())[num]->GetDistribFun()->GetDistributionType() == dtTree)
	{
	  isTreeNode = true;
	};
      };
  };

  if( (!IsAllNdsTab()) || (isTreeNode == true))
  {
    if( GetModel()->GetModelType() == mtBNet )
    {
      if (m_SoftMaxGaussianFactors[node] == NULL) {	  
        for( i = 0; i < m_environment[node].size(); i++ )
        {            
          int num = m_environment[node][i];
          CPotential *pot1 = (!(((*GetCurrentFactors())[num]->GetDistribFun()->GetDistributionType() == dtSoftMax)||
            ((*GetCurrentFactors())[num]->GetDistribFun()->GetDistributionType() == dtCondSoftMax)))?
            
            (static_cast< CCPD* >( (*GetCurrentFactors())[num] )
    	  	    ->ConvertWithEvidenceToPotential(pEv)):
          
          (static_cast< CSoftMaxCPD* >( (*GetCurrentFactors())[num] )
  	  	      ->ConvertWithEvidenceToTabularPotential(pEv));
          
          CPotential *pot2 = pot1->Marginalize(&node, 1);
          delete pot1;
          *potToSample *= *pot2;
          delete pot2;
        } //for( i = 0; i < m_envir...
      }
      else {
        if (!bHasNodeGotDChldWMrThn2Vl) {
          for( i = 0; i < m_environment[node].size(); i++ )
          {       
            int num = m_environment[node][i];
            
            if (!(((*GetCurrentFactors())[num]->GetDistribFun()->GetDistributionType() == dtSoftMax)||
              ((*GetCurrentFactors())[num]->GetDistribFun()->GetDistributionType() == dtCondSoftMax))) {           
              CPotential *pot1 = (static_cast< CCPD* >( (*GetCurrentFactors())[num] )
                ->ConvertWithEvidenceToPotential(pEv));
              
              CPotential *pot2 = pot1->Marginalize(&node, 1);
              delete pot1;
              *potToSample *= *pot2;  
              //static_cast<CGaussianDistribFun *>(potToSample->GetDistribFun())->UpdateMomentForm();
              //potToSample->Dump();
              delete pot2;
            }
            else {
              //Converting distribution to gaussiang
              //Variational Approzimation (see Kevin P. Murphy
              //"A variational Approximation for Bayesian Networks with Disrete and Continuous Latent Variables")
              floatVector MeanContParents;
              C2DNumericDenseMatrix<float>* CovContParents;
              
              static_cast<const CSoftMaxCPD*>((*GetCurrentFactors())[num])->
                CreateMeanAndCovMatrixForNode(num, pEv, static_cast<const CBNet*> (GetModel()), 
                MeanContParents, &CovContParents);
              
              intVector discParents(0);
              static_cast <const CBNet *>(GetModel())->GetDiscreteParents(num, &discParents);
              
              int *parentComb = new int [discParents.size()];
              
              intVector pObsNodes;
              pConstValueVector pObsValues;
              pConstNodeTypeVector pNodeTypes;
              pEv->GetObsNodesWithValues(&pObsNodes, &pObsValues,
                &pNodeTypes);
              
              int location;
              for (int k = 0; k < discParents.size(); k++)
              {
                location = 
                  std::find(pObsNodes.begin(), pObsNodes.end(), 
                  discParents[k]) - pObsNodes.begin();
                parentComb[k] = pObsValues[location]->GetInt();
              }
              
              int index = 0; 
              int multidimindexes[2];
              
              const CSoftMaxDistribFun* dtSM = NULL;
              
              if ((*GetCurrentFactors())[num]->GetDistribFun()->GetDistributionType() == dtCondSoftMax)
                dtSM = 
                static_cast<CCondSoftMaxDistribFun*>((*GetCurrentFactors())[num]->GetDistribFun())->
                GetDistribution(parentComb);
              else 
                dtSM = 
                static_cast<CSoftMaxDistribFun*>((*GetCurrentFactors())[num]->GetDistribFun());
              
              intVector Domain;
              (*GetCurrentFactors())[num]->GetDomain(&Domain);
              
              const CNodeType *nt;
              for(int ii = 0; ii < Domain.size(); ii++)
              {
                nt = GetModel()->GetNodeType( Domain[ii] );
                if(!(nt->IsDiscrete()))
                {
                  if (Domain[ii] != node) 
                    index++;
                  else
                    break;
                }
              }
              
              multidimindexes[0] = index;
              CMatrix<float>* pMatWeights = dtSM->GetMatrix(matWeights);
              
              multidimindexes[1] = 0;
              float weight0 = pMatWeights->GetElementByIndexes(multidimindexes);
              multidimindexes[1] = 1;
              float weight1 = pMatWeights->GetElementByIndexes(multidimindexes);;
              
              if (weight0 != weight1) {
                GetModel()->GetModelDomain()->ChangeNodeType(num, 1);
                
 	              CPotential *pot1 = (static_cast< CSoftMaxCPD* >( (*GetCurrentFactors())[num] )
                  ->ConvertWithEvidenceToGaussianPotential(pEv, MeanContParents, CovContParents, parentComb));

                CPotential *pot2 = pot1->Marginalize(&node, 1);
                
                delete pot1;
               	*potToSample *= *pot2;
           	    delete pot2;
                
                GetModel()->GetModelDomain()->ChangeNodeType(num, 0);
              };
              
              delete[] parentComb;
              delete CovContParents;
            }
          }
        }  //if (!bHasNodeGotDChldWMrThn2Vl) {
        else {
          int numberOfCorrectSamples = 0;
          int numberOfAllSamples = 0;
          
          m_SoftMaxGaussianFactors[node]->GetDistribFun()->ClearStatisticalData();
          
          for (;(numberOfCorrectSamples < m_NSamplesForSoftMax)&&(numberOfAllSamples < m_MaxNSamplesForSoftMax);numberOfAllSamples++) {
            
            //Generating of the continuous parent
            pSoftMaxEvidence = CEvidence::Create( m_pGraphicalModel, NNodes, obsNds, obsVals );  
            
            pSoftMaxEvidence->ToggleNodeState(1, &node);
            
            CPotential *pDeltaPotToSample = dynamic_cast<CPotential*>(potToSample->Clone());
            
            const CNodeType *nt;
            for( i = 0; i < m_environment[node].size(); i++ )
            {            
              int num = m_environment[node][i];
              nt = m_pGraphicalModel->GetNodeType(num);
              if (!nt->IsDiscrete()) {
                CPotential *pot1 = static_cast< CCPD* >( (*GetCurrentFactors())[num] )
                  ->ConvertWithEvidenceToPotential(pSoftMaxEvidence);
                CPotential *pot2 = pot1->Marginalize(&node, 1);
                delete pot1;
                *pDeltaPotToSample *= *pot2;
                delete pot2;
              }
            }//for( i = 0; i < m_envir...
            
            pDeltaPotToSample->GenerateSample( pSoftMaxEvidence, m_bMaximize );
            
            delete pDeltaPotToSample;
            
            //Generating of the children's values
            for( i = 0; i < m_environment[node].size(); i++ )
            {            
              int child = m_environment[node][i];
              nt = m_pGraphicalModel->GetNodeType(child);
              
              if (nt->IsDiscrete()) {
                pDeltaPotToSample = dynamic_cast<CPotential*>(GetPotToSampling(child)->Clone());
                pSoftMaxEvidence->ToggleNodeState(1, &child);
                
                for(int j = 0; j < m_environment[child].size(); j++ )
                {        
                  int grandchild = m_environment[child][j];     
                  
                  CPotential *pot1 = (!(((*GetCurrentFactors())[grandchild]->GetDistribFun()->GetDistributionType() == dtSoftMax)||
                    ((*GetCurrentFactors())[grandchild]->GetDistribFun()->GetDistributionType() == dtCondSoftMax)))?
                    
                    (static_cast< CCPD* >( (*GetCurrentFactors())[grandchild] )
                    ->ConvertWithEvidenceToPotential(pSoftMaxEvidence)):
                  
                  (static_cast< CSoftMaxCPD* >( (*GetCurrentFactors())[grandchild] )
                    ->ConvertWithEvidenceToTabularPotential(pSoftMaxEvidence));
                  CPotential *pot2 = pot1->Marginalize(&child, 1);
                  delete pot1;
                  *pDeltaPotToSample *= *pot2;
                  delete pot2;
                }
                
                pDeltaPotToSample->GenerateSample( pSoftMaxEvidence, m_bMaximize );
                
                delete pDeltaPotToSample;        
              }
            }//for( i = 0; i < m_envir...
            
            //Verification of children values
            bool NeedToUpgrade = true;
            for( i = 0; i < m_environment[node].size(); i++ )
            {            
              int child = m_environment[node][i];
              nt = m_pGraphicalModel->GetNodeType(child);
              
              if (nt->IsDiscrete()) {
                if (pSoftMaxEvidence->GetValue(child)->GetInt() != pEv->GetValue(child)->GetInt())   
                  NeedToUpgrade = false;
              }
            }//for( i = 0; i < m_envir...
            
            if (NeedToUpgrade) {
              m_SoftMaxGaussianFactors[node]->UpdateStatisticsML(&pSoftMaxEvidence, 1);
              numberOfCorrectSamples++;
            }
            
            delete pSoftMaxEvidence;
          }//for ((numberOfCorrectSamples < m_NSamplesForS...
          
          if (numberOfCorrectSamples) {
            CPotential *tempPot = m_SoftMaxGaussianFactors[node]
              ->ConvertStatisticToPot(numberOfCorrectSamples);
            *potToSample *= *tempPot;
            delete tempPot;
          }
        }
      }
    }//if( GetModel()->GetModelType() == mtBNe...
    else
    {
      for( i = 0; i < m_environment[node].size(); i++ )
      {
        int num = m_environment[node][i];
        CPotential *pot1 = static_cast< CPotential* >( (*GetCurrentFactors())[num] )
          ->ShrinkObservedNodes(pEv);
        CPotential *pot2 = pot1->Marginalize(&node, 1);
        delete pot1;
        *potToSample *= *pot2;
        delete pot2;
      }
    }
  }
  else //  if( !IsAllNdsTab() )...
  {
    
    CMatrix< float > *pMatToSample;
    pMatToSample = static_cast<CTabularDistribFun*>(potToSample->GetDistribFun())
      ->GetMatrix(matTable);
    
    intVector dims;
    intVector vls;
    intVector domain;
    
    for( i = 0; i < m_environment[node].size(); i++ )
    {            
      int num = m_environment[node][i];
      (*GetCurrentFactors())[num]->GetDomain(&domain);
      GetObsDimsWithVls( domain, node, pEv, &dims, &vls); 
      CMatrix< float > *pMat;
      pMat = static_cast<CTabularDistribFun*>((*GetCurrentFactors())[num]->
        GetDistribFun())->GetMatrix(matTable);
      pMat->ReduceOp( &dims.front(), dims.size(), 2, &vls.front(),
        pMatToSample, PNL_ACCUM_TYPE_MUL );
      dims.clear();
      vls.clear();
      domain.clear();
      
    }
  }  
  //check for non zero elements
  CMatrix<float> *pMat;
  if( potToSample->GetDistributionType()==dtTabular )
  {	
    pMat = potToSample->GetDistribFun()->GetMatrix(matTable);
  }
  else
  {
    CGaussianDistribFun* pDistr = static_cast<CGaussianDistribFun*>(potToSample->GetDistribFun());
    if(pDistr->GetMomentFormFlag())
    {
      pMat = pDistr->GetMatrix(matCovariance);
    }
    else
    {
      pMat = pDistr->GetMatrix(matK);
    }
  }
  
  CMatrixIterator<float>* iter = pMat->InitIterator();
  for( iter; pMat->IsValueHere( iter ); pMat->Next(iter) )
  {
    
    if(*(pMat->Value( iter )) > FLT_EPSILON)
    {
      ret = true;
      break;
    }
  }
  
  if (obsNds != NULL)
    delete obsNds;
  delete iter;
  return ret;
}

#ifdef PNL_RTTI
const CPNLType CGibbsSamplingInfEngine::m_TypeInfo = CPNLType("CGibbsSamplingInfEngine", &(CSamplingInfEngine::m_TypeInfo));

#endif
