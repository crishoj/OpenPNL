/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlParGibbsSamplingInferenceEngine.cpp                      //
//                                                                         //
//  Purpose:   CParGibbsSamplingInfEngine class member functions           //
//             implementation                                              //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlParGibbsSamplingInferenceEngine.hpp"
 
#ifdef PAR_PNL

#include "pnlTabularDistribFun.hpp"
#include "pnlGaussianDistribFun.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlCPD.hpp"
#include "pnlMixtureGaussianCPD.hpp"
#include "pnlMNet.hpp"
#include "pnlSoftMaxCPD.hpp"
#include "pnlSoftMaxDistribFun.hpp"
#include "pnlCondSoftMaxDistribFun.hpp"

#ifdef PAR_OMP
#include <omp.h>
#endif

#ifdef PAR_MPI
#include <mpi.h>
#endif

// ----------------------------------------------------------------------------
// definitions for conditional compilation of OpenMP-parallel parts
#define PAR_OMP_FAKE_NUM_THREADS 0

#ifdef PAR_OMP
#define PAR_OMP_NUM_THREADS             omp_get_num_procs()
#define PAR_OMP_SET_NUM_THREADS(__val)  omp_set_num_threads(__val)
#define PAR_OMP_NUM_CURR_THREAD         omp_get_thread_num()
#else
#define PAR_OMP_NUM_THREADS             1
#define PAR_OMP_SET_NUM_THREADS(__val)  
#define PAR_OMP_NUM_CURR_THREAD         0
#endif

#define PAR_OMP_CORRECT_NUM_THREADS(__var)      \
    if (PAR_OMP_FAKE_NUM_THREADS == __var) {    \
    __var = PAR_OMP_NUM_THREADS;                \
    PAR_OMP_SET_NUM_THREADS(__var); }

// ----------------------------------------------------------------------------

PNL_USING

// ----------------------------------------------------------------------------

CParGibbsSamplingInfEngine * CParGibbsSamplingInfEngine::Create(
    const CStaticGraphicalModel *pGraphicalModel)
{
    if(!pGraphicalModel)
    {
        PNL_THROW(CNULLPointer, "graphical model is absent");
        return NULL;
    }
    else
    {
        CParGibbsSamplingInfEngine* newInf = 
            new CParGibbsSamplingInfEngine(pGraphicalModel);
        return newInf;
    }
}
// ----------------------------------------------------------------------------

CParGibbsSamplingInfEngine::CParGibbsSamplingInfEngine(
    const CStaticGraphicalModel *pGraphicalModel):
    CGibbsSamplingInfEngine(pGraphicalModel)
{
    m_NumberOfThreads = PAR_OMP_FAKE_NUM_THREADS;

    InitIPCConsts();

    potsPVector& potsToSampling = *GetPotsToSampling();

    for(int i = 0; i < potsToSampling.size(); i++ )
    {
        delete potsToSampling[i];
    }
    potsToSampling.clear();

    CreateSamplingPotentials( &potsToSampling );
    
    FindEnvironment(&m_environment);

    m_SoftMaxGaussianFactorsForOMP.resize(pGraphicalModel->GetNumberOfNodes()*m_NumberOfThreads, NULL);

    Initialization();
}
// ----------------------------------------------------------------------------

CParGibbsSamplingInfEngine::~CParGibbsSamplingInfEngine()
{
    int NFactors = m_pGraphicalModel->GetNumberOfNodes()*m_NumberOfThreads;
    for (int factor = 0; factor < NFactors; factor++) {
        if (m_SoftMaxGaussianFactorsForOMP[factor]!=NULL) {
            delete m_SoftMaxGaussianFactorsForOMP[factor];
            m_SoftMaxGaussianFactorsForOMP[factor] = NULL;
        }
    }

    //Destroy query factors, that was created in CParGibbsSamplingInfEngine::Initialization()
    int nFactors = GetModel()->GetNumberOfFactors();
    int nFactorsOMP = GetModel()->GetNumberOfFactors()*m_NumberOfThreads;
    pFactorVector *currentFactors = GetCurrentFactors();
    int i;
    for( i = nFactors; i < nFactorsOMP; i++ )
    {
        if ((*currentFactors)[i])
        {
            delete (*currentFactors)[i];
            (*currentFactors)[i] = NULL;
        };
    }
}
// ----------------------------------------------------------------------------

void CParGibbsSamplingInfEngine::InitIPCConsts() 
{
#ifdef PAR_MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &m_MyRank);
    MPI_Comm_size(MPI_COMM_WORLD, &m_NumberOfProcesses);
#else
    m_MyRank = 0;
    m_NumberOfProcesses = 1;
#endif
}
// ----------------------------------------------------------------------------

void CParGibbsSamplingInfEngine::MarginalNodes(const int *queryIn, int querySz,
    int notExpandJPD)
{
    PNL_CHECK_IS_NULL_POINTER(queryIn);

    EDistributionType paramDistrType = 
        pnlDetermineDistributionType( GetModel()->GetModelDomain(), querySz, queryIn, m_pEvidence);
    
    bool IsUnobservedNodes = false;
    
    for (int variable = 0; variable < querySz; variable++)
    {
        if (!(m_pEvidence->IsNodeObserved(queryIn[variable])))
            IsUnobservedNodes = true;
    };

    intVector Query(querySz);
    int NumberOfFactor;
    pFactorVector queryFactors;
    GetQueryFactors( &queryFactors);
    
    switch (paramDistrType)
    {
    case dtTabular:
        CGibbsSamplingInfEngine::MarginalNodes(queryIn, querySz, notExpandJPD);
        
        if (IsUnobservedNodes)
            if (m_MyRank != 0)
            {
                SendMyPotentialViaMpi();
            }
            else
            {
                CNumericDenseMatrix<float> *mat;
                int dataLength;
                const float *pData;
                
                //need to know size of vector pData
                CPotential *ShrinkPot;
                
                ShrinkPot = m_pQueryJPD->ShrinkObservedNodes(m_pEvidence);
                
                mat = static_cast<CNumericDenseMatrix<float>*>
                    ((ShrinkPot->GetDistribFun())->GetMatrix(matTable));
                mat->GetRawData(&dataLength, &pData);
                
                intVector NumbersOfSamples(m_NumberOfProcesses, -1);
                NumbersOfSamples[0] =
                    (m_currentTime-GetBurnIn()-1) * GetNumStreams();
                
                floatVecVector ProbDistrib(m_NumberOfProcesses,
                    floatVector(dataLength, -1));
                ProbDistrib[0].assign(pData, &(pData[dataLength - 1]));
                ProbDistrib[0].push_back( pData[dataLength - 1] );
                
                RecvPotentialsViaMpi(&NumbersOfSamples, &ProbDistrib);
                
                //Claculating of evidences
                int SumNumberOfSamples = 0;
                floatVector SumNumberOfProperSamples(dataLength, 0);
                floatVector FinalDistrib(dataLength, 0);
                
                for (int i = 0; i < m_NumberOfProcesses; i++)
                {
                    SumNumberOfSamples += NumbersOfSamples[i];
                    
                    for (int variant = 0; variant < dataLength; variant++)
                        SumNumberOfProperSamples[variant] +=
                        ProbDistrib[i][variant]*NumbersOfSamples[i];
                }
                
                for (int variant = 0; variant < dataLength; variant++)
                {
                    FinalDistrib[variant] =
                        SumNumberOfProperSamples[variant] / SumNumberOfSamples;
                }
                
                // set new potential
                CModelDomain* pMD=ShrinkPot->GetModelDomain();
                
                // this is a vector of domain
                intVector Domain(querySz);
                
                Domain.assign(queryIn, queryIn + (querySz - 1));
                Domain.push_back( queryIn[querySz - 1] );
                
                intVector obsIndicesIn(0);
                
                for (int variable = 0; variable < querySz; variable++)
                {
                    if (m_pEvidence->IsNodeObserved(queryIn[variable]))
                        obsIndicesIn.push_back(queryIn[variable]);
                }
                
                //need to recv Final Distribution
                CPotential *deltaPot = CTabularPotential::Create(Domain, pMD,
                    &FinalDistrib.front(), obsIndicesIn);
                
                delete ShrinkPot;
                
                ShrinkPot = deltaPot->ExpandObservedNodes(m_pEvidence);
                
                m_pQueryJPD->SetDistribFun(ShrinkPot->GetDistribFun());
                
                delete deltaPot;
            }
            break;
    case dtGaussian:
        int i;
        for (i=0; i< querySz; i++) Query[i] = queryIn[i];
        NumberOfFactor = GetFactorNumber(Query);
        if (IsUnobservedNodes)
            if (m_MyRank != 0)
            {
                SendMyGaussianStatisticsViaMpi(NumberOfFactor);
            }
            else
            {
                intVector NumberOfSamples;
                floatVecVector LearnMean;
                floatVecVector LearnCov;
                
                RecvGaussianStatisticsViaMpi(&NumberOfSamples, &LearnMean, 
                    &LearnCov, NumberOfFactor);
                
                CFactor *factor = CGaussianPotential::Create(Query, 
                    GetModel()->GetModelDomain());
                
                CNumericDenseMatrix<float> *pMatLearnMean = 
                    dynamic_cast<CNumericDenseMatrix<float> *> (queryFactors[NumberOfFactor]->
                    GetDistribFun()->GetStatisticalMatrix(stMatMu));
                CNumericDenseMatrix<float> *pMatLearnCov = 
                    dynamic_cast<CNumericDenseMatrix<float> *> (queryFactors[NumberOfFactor]->
                    GetDistribFun()->GetStatisticalMatrix(stMatSigma));
                
                int length = 0;
                const pConstNodeTypeVector *pNodeTypes;
                pNodeTypes = queryFactors[NumberOfFactor]->GetDistribFun()->
                    GetNodeTypesVector();
                int NumberOfNodes = pNodeTypes->size();
                
                for (int node = 0; node < NumberOfNodes; node++)
                {
                    length += ( (*pNodeTypes)[node]->GetNodeSize() );
                }
                
                int DimMean = 2;
                int DimCov = 2;
                const int RangeMean[2] = {length, 1} ;
                const int RangeCov[2] = {length, length};
                
                float *pData = new float[length * length];
                
                for (i = 1; i < m_NumberOfProcesses; i++)
                {
                    C2DNumericDenseMatrix<float> * pLearnMean = 
                        C2DNumericDenseMatrix<float>::Create(RangeMean, &(LearnMean[i].front()));
                    C2DNumericDenseMatrix<float> * pLearnCov = 
                        C2DNumericDenseMatrix<float>::Create(RangeCov, &(LearnCov[i].front()));
                    //pLearnMean->GetRaw
                    
                    
                    factor->SetStatistics(pLearnMean, stMatMu);
                    factor->SetStatistics(pLearnCov, stMatSigma);
                    
                    queryFactors[NumberOfFactor]->UpdateStatisticsML(factor);
                };
                
                int AdditionalSamples = 0;
                for (i = 1; i < m_NumberOfProcesses; i++)
                {
                    AdditionalSamples += NumberOfSamples[i];
                };
                
                int OldMaxTime = GetMaxTime();
                int OldBarnIn = GetBurnIn();
                int OldNumberOfStreams = GetNumStreams();
                
                AdditionalSamples += (OldMaxTime-OldBarnIn-1)*OldNumberOfStreams;
                
                SetMaxTime(AdditionalSamples+1);
                SetBurnIn(0);
                SetNumStreams(1);
                
                CGibbsSamplingInfEngine::MarginalNodes(queryIn, querySz, notExpandJPD);
                
                SetMaxTime(OldMaxTime);
                SetBurnIn(OldBarnIn);
                SetNumStreams(OldNumberOfStreams);
            };
            
            break;
    default:
        {
            PNL_THROW( CInconsistentType, "distribution type" )
        }
        
    }
}
// ----------------------------------------------------------------------------

void CParGibbsSamplingInfEngine::SendMyPotentialViaMpi()
{
#ifdef PAR_MPI
    if (m_MyRank == 0)
        return;

    PNL_CHECK_IS_NULL_POINTER(m_pQueryJPD);

    //send number of samples 
    int NumberOfSamples = (m_currentTime-GetBurnIn()-1) * GetNumStreams();
    MPI_Send(&NumberOfSamples, 1, MPI_INT, 0, m_MyRank, MPI_COMM_WORLD);

    //send vector of probability
    CNumericDenseMatrix<float> *matForSending;
    CPotential *newPot;
    int dataLength;
    const float *pDataForSending;

    newPot = m_pQueryJPD->ShrinkObservedNodes(m_pEvidence);

    matForSending = static_cast<CNumericDenseMatrix<float>*>
        ((newPot->GetDistribFun())->GetMatrix(matTable));

    matForSending->GetRawData(&dataLength, &pDataForSending);

    MPI_Send((float*)pDataForSending, dataLength, MPI_FLOAT, 0, m_MyRank, 
        MPI_COMM_WORLD);

    delete newPot;
#endif // PAR_MPI
}
// ----------------------------------------------------------------------------

void CParGibbsSamplingInfEngine::RecvPotentialsViaMpi(
    intVector *pNumberOfSamples, floatVecVector *pProbDistrib)
{
#ifdef PAR_MPI
    if (m_MyRank != 0)
        return;

    PNL_CHECK_IS_NULL_POINTER(m_pQueryJPD);
    PNL_CHECK_IS_NULL_POINTER(pNumberOfSamples);
    PNL_CHECK_IS_NULL_POINTER(pProbDistrib);

    // HaveMsg[0] is never used
    intVector HaveMsg(m_NumberOfProcesses, -1);
    // -1 - no messages
    //  1 - have int
    //  2 - have int and float array

    // number of recieved msges
    int NumberOfMsg = 0;

    // number of distrib func states 
    int NumberOfStates = pProbDistrib->operator[](0).size();

    while (NumberOfMsg != (m_NumberOfProcesses - 1))
    {
        for (int i = 1; i < m_NumberOfProcesses; i++)
            if (HaveMsg[i] != 2)
            {
                //have some msg from i ?
                int flag;
                MPI_Status status;

                MPI_Iprobe(i , MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

                if (flag)
                    if (HaveMsg[i] == -1)
                    {
                        MPI_Recv(&(pNumberOfSamples->operator[](i)), 1,
                            MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                        HaveMsg[i] = 1;
                    }
                    else
                    {//HaveMsg[i] == 1
                        MPI_Recv(&(pProbDistrib->operator[](i).front()),
                            NumberOfStates, MPI_FLOAT, i, MPI_ANY_TAG,
                            MPI_COMM_WORLD, &status);
                        HaveMsg[i] = 2;
                        NumberOfMsg++;
                    }
            } //if (HaveMsg[i] != 2...
    } //while...
#endif // PAR_MPI
}
// ----------------------------------------------------------------------------

void CParGibbsSamplingInfEngine::
Sampling( int statTime, int endTime )
{
    PAR_OMP_CORRECT_NUM_THREADS(m_NumberOfThreads);

    intVector ndsForSampling;
    GetNdsForSampling( &ndsForSampling );
    boolVector sampleIsNeed;
    GetSamplingNdsFlags( &sampleIsNeed );

    int numNdsForSampling = ndsForSampling.size();

    intVector ndsSampleIsNeed(numNdsForSampling);
    m_NumberOfNodesForSample = numNdsForSampling;
    int ndsSampleIsNeedSize = 0;
    for (int j = 0; j < numNdsForSampling; j++) {
        if (sampleIsNeed[j])
            ndsSampleIsNeed[ndsSampleIsNeedSize++] = j;
    };
    ndsSampleIsNeed.resize(ndsSampleIsNeedSize);

    pEvidencesVector currentEvidences;
    GetCurrentEvidences( &currentEvidences );

    int t; 
    int i;
    int series;

    int NStreams = GetNumStreams();
    int BurnIn = GetBurnIn();

    pFactorVector queryFactors;
    GetQueryFactors( &queryFactors );
    bool *was_updated = new bool[queryFactors.size()];
    for (int gh = 0; gh < queryFactors.size(); gh++) {
        was_updated[gh] = false;
    };

#ifdef PAR_OMP
#pragma omp parallel for private(series, t, i) default(shared)
#endif
    for( series = 0; series < NStreams; series++ )
    {
        int myid = PAR_OMP_NUM_CURR_THREAD;

        CEvidence * pCurrentEvidence = currentEvidences[series];

        for( t = statTime; t < endTime; t++ )
        {
            for( i = 0; i < ndsSampleIsNeedSize; i++ )
            {
                pCurrentEvidence->ToggleNodeStateBySerialNumber(1, 
                    &(ndsSampleIsNeed[i]));

                int digit = ndsForSampling[(ndsSampleIsNeed[i])] + 
                    myid * numNdsForSampling;
           
                bool canBeSample = ConvertingFamilyToPot( digit, pCurrentEvidence );

                if(canBeSample)
                {
                    CPotential *pPot =
                        GetPotToSampling(ndsForSampling[(ndsSampleIsNeed[i])] + 
                        myid * numNdsForSampling);
                    pPot->GenerateSample( pCurrentEvidence, m_bMaximize );
                }
                else
                {
                    pCurrentEvidence->ToggleNodeStateBySerialNumber(1, 
                        &(ndsSampleIsNeed[i]));
                }
            }
            if (t > BurnIn)
            {
                pnlVector<CFactor*>* queryFactors = GetQueryFactors();
                //pFactorVector queryFactors;
                //GetQueryFactors( &queryFactors );
                int ii;
                int queryFactorsSize =
                    ((float)(queryFactors->size())) / (m_NumberOfThreads);
                for( ii = 0; ii < queryFactorsSize; ii++ )
                {
                    was_updated[ii + myid * queryFactorsSize] = true;
                    (*queryFactors)[ii + myid * queryFactorsSize]->
                        UpdateStatisticsML(&pCurrentEvidence, 1);
                }
            }
        }
    }

    m_currentTime = endTime;

    pFactorVector& rfQueryFactors = *GetQueryFactors();
    int zztop = rfQueryFactors.size();
    int queryFactorsSize = ((float)(rfQueryFactors.size())) / (m_NumberOfThreads);

#ifdef PAR_OMP
    for (i = 0; i < queryFactorsSize; i++ )
        for (int j = 1; j < m_NumberOfThreads; j++) 
        {
            if (was_updated[i + j * queryFactorsSize])
                rfQueryFactors[i]->UpdateStatisticsML(
                rfQueryFactors[i + j * queryFactorsSize]);
        };
#endif // PAR_OMP

    delete[] was_updated;

    for( i = queryFactorsSize; i < rfQueryFactors.size(); i++ )
    {
        delete rfQueryFactors[i];
    };

    rfQueryFactors.resize(queryFactorsSize);

#ifdef PAR_OMP
    GetModel()->GetModelDomain()->ClearNodeTypeCopies();
#endif
}
// ----------------------------------------------------------------------------

void CParGibbsSamplingInfEngine::
CreateSamplingPotentials( potsPVector* potsToSampling )
{
    PAR_OMP_CORRECT_NUM_THREADS(m_NumberOfThreads);

    CModelDomain *pMD = GetModel()->GetModelDomain();

    intVector ndsForSampling;
    GetNdsForSampling( &ndsForSampling );
    int ndsForSamplingSize = ndsForSampling.size();
    potsToSampling->resize( ndsForSamplingSize * m_NumberOfThreads );

    CPotential *tmpPot;

    int i;
    for( i = 0; i < potsToSampling->size(); i++ )
    {
        const CNodeType* nt =
            pMD->GetVariableType(ndsForSampling[i % ndsForSamplingSize]);
        if( nt->IsDiscrete() )
        {
            tmpPot = CTabularPotential::
                Create( &ndsForSampling[i % ndsForSamplingSize], 1, pMD, 
                &floatVector(nt->GetNodeSize(), 1.0f).front() );
        }
        else
        {
            tmpPot = CGaussianPotential::CreateUnitFunctionDistribution(
                &ndsForSampling[i % ndsForSamplingSize], 1, pMD );
        }

        (*potsToSampling)[i] = tmpPot;
    }
}
// ----------------------------------------------------------------------------

bool CParGibbsSamplingInfEngine::
ConvertingFamilyToPot( int node, const CEvidence* pEv )
{
    int NNodes = m_pGraphicalModel->GetNumberOfNodes();
    int nodeModndsSize = node%NNodes;

    int ThreadId = PAR_OMP_NUM_CURR_THREAD;
    
    bool ret = false;
    CPotential* potToSample = GetPotToSampling(node);
    Normalization(potToSample);
    int i;
    bool isTreeNode = false;
    
    //Has node got discrete child with more than 2 values
    bool bHasNodeGotDChldWMrThn2Vl = false;
    
    for (int ii = 0; ((ii < (m_environment[nodeModndsSize].size()-1))&&(!bHasNodeGotDChldWMrThn2Vl)); ii++) {
        int num = m_environment[nodeModndsSize][ii];
        if ((GetModel()->GetNodeType(num)->IsDiscrete())&&(GetModel()->GetNodeType(num)->GetNodeSize() > 2))
            bHasNodeGotDChldWMrThn2Vl = true;
    };
    
    int *obsNds = NULL;
    valueVector obsVals;
    CEvidence *pSoftMaxEvidence = NULL;

    if ((bHasNodeGotDChldWMrThn2Vl)&&(m_SoftMaxGaussianFactorsForOMP[node] != NULL)) 
    {
        obsVals.resize(NNodes);
        obsNds = new int[NNodes];
        
        const_cast<CEvidence*> (pEv)->ToggleNodeState(1, &nodeModndsSize);
        
        for (i = 0; i < NNodes; i++) 
        {
            const CNodeType *nt = m_pGraphicalModel->GetNodeType(i);
            if (nt->IsDiscrete()) 
                obsVals[i].SetInt(pEv->GetValue(i)->GetInt()); 
            else 
                obsVals[i].SetFlt(pEv->GetValue(i)->GetFlt()); 
            
            obsNds[i] = i;
        }
        
        const_cast<CEvidence*> (pEv)->ToggleNodeState(1, &nodeModndsSize);
    }

    for( i = 0; i < m_environment[nodeModndsSize].size(); i++ )
    {            
        int num = m_environment[nodeModndsSize][i]+ThreadId*NNodes;
        if ( (*GetCurrentFactors())[num]->GetDistribFun()->GetDistributionType() == dtTree)
        {
            isTreeNode = true;
        };
    };

    if( (!IsAllNdsTab()) || (isTreeNode == true))
    {
        if( GetModel()->GetModelType() == mtBNet )
        {
            if (m_SoftMaxGaussianFactorsForOMP[node] == NULL) 
            {
                for( i = 0; i < m_environment[nodeModndsSize].size(); i++ )
                {            
                    int num = m_environment[nodeModndsSize][i]+ThreadId*NNodes;
                    CPotential *pot1 = (!(((*GetCurrentFactors())[num]->GetDistribFun()->GetDistributionType() == dtSoftMax)||
                        ((*GetCurrentFactors())[num]->GetDistribFun()->GetDistributionType() == dtCondSoftMax)))?
                        
                        (static_cast< CCPD* >( (*GetCurrentFactors())[num] )
                        ->ConvertWithEvidenceToPotential(pEv)):
                    
                    (static_cast< CSoftMaxCPD* >( (*GetCurrentFactors())[num] )
                        ->ConvertWithEvidenceToTabularPotential(pEv));
                    
                    CPotential *pot2 = pot1->Marginalize(&nodeModndsSize, 1);
                    delete pot1;
                    *potToSample *= *pot2;
                    delete pot2;
                } //for( i = 0; i < m_envir...
            }
            else {
                if (!bHasNodeGotDChldWMrThn2Vl) {
                    for( i = 0; i < m_environment[nodeModndsSize].size(); i++ )
                    {       
                        int num = m_environment[nodeModndsSize][i]+ThreadId*NNodes;
                        
                        if (!(((*GetCurrentFactors())[num]->GetDistribFun()->GetDistributionType() == dtSoftMax)||
                            ((*GetCurrentFactors())[num]->GetDistribFun()->GetDistributionType() == dtCondSoftMax))) {           
                            CPotential *pot1 = (static_cast< CCPD* >( (*GetCurrentFactors())[num] )
                                ->ConvertWithEvidenceToPotential(pEv));
                            
                            CPotential *pot2 = pot1->Marginalize(&nodeModndsSize, 1);
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
                                CreateMeanAndCovMatrixForNode(num%NNodes, pEv, static_cast<const CBNet*> (GetModel()), 
                                MeanContParents, &CovContParents);
                            
                            intVector discParents(0);
                            static_cast <const CBNet *>(GetModel())->GetDiscreteParents(num%NNodes, &discParents);
                            
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
                                    if (Domain[ii] != nodeModndsSize) 
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
#ifdef PAR_OMP
                                GetModel()->GetModelDomain()->ChangeNodeTypeOnThread(num%NNodes, 1);
#else
                                GetModel()->GetModelDomain()->ChangeNodeType(num%NNodes, 1);
#endif
                                
                                CPotential *pot1 = (static_cast< CSoftMaxCPD* >( (*GetCurrentFactors())[num] )
                                    ->ConvertWithEvidenceToGaussianPotential(pEv, MeanContParents, CovContParents, parentComb));
                                
                                CPotential *pot2 = pot1->Marginalize(&nodeModndsSize, 1);
                                
                                delete pot1;
                                *potToSample *= *pot2;
                                delete pot2;
                                
#ifdef PAR_OMP
                                GetModel()->GetModelDomain()->ChangeNodeTypeOnThread(num%NNodes, 0);
#else
                                GetModel()->GetModelDomain()->ChangeNodeType(num%NNodes, 0);
#endif
                            };
                            
                            delete[] parentComb;
                            delete CovContParents;
                        }
          }
        }  //if (!bHasNodeGotDChldWMrThn2Vl) {
        else {
            int numberOfCorrectSamples = 0;
            int numberOfAllSamples = 0;
            
            m_SoftMaxGaussianFactorsForOMP[node]->GetDistribFun()->ClearStatisticalData();
            
            for (;(numberOfCorrectSamples < GetNSamplesForSoftMax())&&(numberOfAllSamples < GetMaxNSamplesForSoftMax());numberOfAllSamples++) {
                
                //Generating of the continuous parent
                pSoftMaxEvidence = CEvidence::Create( m_pGraphicalModel, NNodes, obsNds, obsVals );  
                
                pSoftMaxEvidence->ToggleNodeState(1, &nodeModndsSize);
                
                CPotential *pDeltaPotToSample = dynamic_cast<CPotential*>(potToSample->Clone());
                
                const CNodeType *nt;
                for( i = 0; i < m_environment[nodeModndsSize].size(); i++ )
                {            
                    int num = m_environment[nodeModndsSize][i]+ThreadId*NNodes;
                    nt = m_pGraphicalModel->GetNodeType(num%NNodes);
                    if (!nt->IsDiscrete()) {
                        CPotential *pot1 = static_cast< CCPD* >( (*GetCurrentFactors())[num] )
                            ->ConvertWithEvidenceToPotential(pSoftMaxEvidence);
                        CPotential *pot2 = pot1->Marginalize(&nodeModndsSize, 1);
                        delete pot1;
                        *pDeltaPotToSample *= *pot2;
                        delete pot2;
                    }
                }//for( i = 0; i < m_envir...
                
                pDeltaPotToSample->GenerateSample( pSoftMaxEvidence, m_bMaximize );
                
                delete pDeltaPotToSample;
                
                //Generating of the children's values
                for( i = 0; i < m_environment[nodeModndsSize].size(); i++ )
                {            
                    int child = m_environment[nodeModndsSize][i];
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
                for( i = 0; i < m_environment[nodeModndsSize].size(); i++ )
                {            
                    int child = m_environment[nodeModndsSize][i];
                    nt = m_pGraphicalModel->GetNodeType(child);
                    
                    if (nt->IsDiscrete()) {
                        if (pSoftMaxEvidence->GetValue(child)->GetInt() != pEv->GetValue(child)->GetInt())   
                            NeedToUpgrade = false;
                    }
                }//for( i = 0; i < m_envir...
                
                if (NeedToUpgrade) {
                    m_SoftMaxGaussianFactorsForOMP[node]->UpdateStatisticsML(&pSoftMaxEvidence, 1);
                    numberOfCorrectSamples++;
                }
                
                delete pSoftMaxEvidence;
            }//for ((numberOfCorrectSamples < m_NSamplesForS...
            
            if (numberOfCorrectSamples) {
                CPotential *tempPot = m_SoftMaxGaussianFactorsForOMP[node]
                    ->ConvertStatisticToPot(numberOfCorrectSamples);
                *potToSample *= *tempPot;
                delete tempPot;
            }
        }
      }
    }//if( GetModel()->GetModelType() == mtBNe...
    else
    {
        for( i = 0; i < m_environment[nodeModndsSize].size(); i++ )
        {
            int num = m_environment[nodeModndsSize][i]+ThreadId*NNodes;
            CPotential *pot1 = static_cast< CPotential* >( (*GetCurrentFactors())[num] )
                ->ShrinkObservedNodes(pEv);
            CPotential *pot2 = pot1->Marginalize(&nodeModndsSize, 1);
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
      
      for( i = 0; i < m_environment[nodeModndsSize].size(); i++ )
      {            
          int num = m_environment[nodeModndsSize][i]+ThreadId*NNodes;
          (*GetCurrentFactors())[num]->GetDomain(&domain);
          GetObsDimsWithVls( domain, nodeModndsSize, pEv, &dims, &vls); 
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

// ----------------------------------------------------------------------------

void CParGibbsSamplingInfEngine::EnterEvidence( const CEvidence *pEvidenceIn,
    int maximize, int sumOnMixtureNode )
{
    if( !GetQueryes().size() )
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
            GenerateSamples( GetCurrentEvidences(), GetNumStreams(),
            pEvidenceIn );
    }
    else
    {
        static_cast< const CMNet* >(GetModel())->
            GenerateSamples( GetCurrentEvidences(), GetNumStreams(),
            pEvidenceIn );
    }

    CreateQueryFactors();

    boolVector sampleIsNeed;
    if( GetSignOfUsingDSeparation() )
    {
        ConsDSep( GetQueryes(), &sampleIsNeed, m_pEvidence );
    }
    else
    {
        FindCurrentNdsForSampling( &sampleIsNeed );
    }
    SetSamplingNdsFlags(sampleIsNeed);

    Sampling( 0, GetMaxTime() );
}
// ----------------------------------------------------------------------------

void CParGibbsSamplingInfEngine::CreateQueryFactors()
{
    int NumThreads = PAR_OMP_NUM_THREADS;

    pConstNodeTypeVector ntVec;
    const CNodeType *nt;
    intVector query;

    intVecVector& rfQueryes = GetQueryes();
    pFactorVector& rfQueryFactors = *GetQueryFactors();

    //Create additional factors for every processes
    for (int proc = 0; proc < NumThreads; proc++)
    {
        for( int number = 0; number < rfQueryes.size(); number++ )
        {
            for( int node = 0; node < rfQueryes[number].size(); node++)
            {
                if( !m_pEvidence->IsNodeObserved(rfQueryes[number][node]) )
                {
                    query.push_back(rfQueryes[number][node]);
                    nt = m_pGraphicalModel->GetNodeType(rfQueryes[number][node]);
                    if ( ntVec.size() )
                    {
                        if( !( ( nt->IsDiscrete() && ntVec.back()->IsDiscrete() ) ||
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
                    rfQueryFactors.push_back( CTabularPotential::Create( query, 
                        m_pGraphicalModel->GetModelDomain(), NULL ) );
                }
                else
                {
                    rfQueryFactors.push_back( CGaussianPotential::Create( query, 
                        m_pGraphicalModel->GetModelDomain(), NULL ) );
                }
            }
            ntVec.clear();
            query.clear();
        }
    };

    //Create m_SoftMaxGaussianFactorsForOMP potentials
    int NNodes = m_pGraphicalModel->GetNumberOfNodes();
    int NFactors = m_pGraphicalModel->GetNumberOfNodes()*m_NumberOfThreads;
    CGraph *pGraph = m_pGraphicalModel->GetGraph();
    intVector Children;
    
    for (int factor = 0; factor < NFactors; factor++) 
    {
        nt = m_pGraphicalModel->GetNodeType(factor%NNodes);
        
        if ((!nt->IsDiscrete())&&(!(m_pEvidence->IsNodeObserved(factor%NNodes))))
        {
            pGraph->GetChildren(factor%NNodes, &Children);
            
            for (int child = 0; child < Children.size(); child++) 
            {
                nt = m_pGraphicalModel->GetNodeType(Children[child]);
                
                if (nt->IsDiscrete()) {
                    int node = factor%NNodes;
                    m_SoftMaxGaussianFactorsForOMP[factor] = CGaussianPotential::Create(&node, 1, m_pGraphicalModel->GetModelDomain());
                    break;
                };
            }
        }
    };
}
// ----------------------------------------------------------------------------

void CParGibbsSamplingInfEngine::Initialization()
{
    int nFactors = GetModel()->GetNumberOfFactors();
    int nFactorsOMP = GetModel()->GetNumberOfFactors()*m_NumberOfThreads;
    pFactorVector *currentFactors = GetCurrentFactors();
    currentFactors->resize(nFactorsOMP);
    int i;
    for( i = 0; i < nFactorsOMP; i++ )
    {
        if (i<nFactors)
        {
            (*currentFactors)[i] = GetModel()->GetFactor(i);
        }
        else
        {
            (*currentFactors)[i] = GetModel()->GetFactor(i%nFactors)->Clone();
        };
    }
}
// ----------------------------------------------------------------------------

int CParGibbsSamplingInfEngine::GetFactorNumber(intVector queryIn)
{
    int FactorNumber = -1;
    const CFactor *pFactor;
    int *begin1;
    int *end1;
    int *begin2;
    int *end2;
    intVector domainVec;
    intVector queryVec;
    pFactorVector queryFactors;

    queryVec.reserve(queryIn.size());
    int i;
    int querySz = queryIn.size();
    for( i = 0; i < querySz; i++ )
    {
        if (! m_pEvidence->IsNodeObserved(queryIn[i]))
            queryVec.push_back(queryIn[i]);
    }
    
    GetQueryFactors( &queryFactors);

    if( queryVec.size() )
    {
        for( i = 0; i < queryFactors.size(); i++)     
        {
            domainVec.clear();
            pFactor = queryFactors[i];
            pFactor->GetDomain(&domainVec);
            begin1 = &domainVec.front();
            end1 = &domainVec.back() + 1;
            std::sort(begin1, end1);
            
            begin2 = &queryVec.front();
            end2 = &queryVec.back() + 1;
            std::sort(begin2, end2);
            
            if( std::includes(begin1, end1, begin2, end2) )
            {
                FactorNumber = i;
                break;
            }
            
        }
    }
    
    return FactorNumber;
}

// ----------------------------------------------------------------------------

// send gaussian learn matrixes, where index in m_queryFactors is NumberOfFactor
void CParGibbsSamplingInfEngine::SendMyGaussianStatisticsViaMpi(int NumberOfFactor)
{
#ifdef PAR_MPI
    if (m_MyRank == 0)
        return;

    pFactorVector queryFactors;
    GetQueryFactors( &queryFactors);
    PNL_CHECK_RANGES(NumberOfFactor, 0, queryFactors.size()-1);

    //send number of samples 
    int NumberOfSamples = (m_currentTime-GetBurnIn()-1) * GetNumStreams();
    MPI_Send(&NumberOfSamples, 1, MPI_INT, 0, m_MyRank, MPI_COMM_WORLD);

    //send learn matrix of mean
    CNumericDenseMatrix<float> *pMatLearnMean = dynamic_cast<CNumericDenseMatrix<float> *> (queryFactors[NumberOfFactor]->
        GetDistribFun()->GetStatisticalMatrix(stMatMu));
    
    int dataLength;
    const float *pDataForSending;

    pMatLearnMean->GetRawData(&dataLength, &pDataForSending);

    MPI_Send((float*)pDataForSending, dataLength, MPI_FLOAT, 0, m_MyRank, 
        MPI_COMM_WORLD);

    //send learn matrix of cov
    CNumericDenseMatrix<float> *pMatLearnCov = dynamic_cast<CNumericDenseMatrix<float> *> (queryFactors[NumberOfFactor]->
        GetDistribFun()->GetStatisticalMatrix(stMatSigma));

    pMatLearnCov->GetRawData(&dataLength, &pDataForSending);

    MPI_Send((float*)pDataForSending, dataLength, MPI_FLOAT, 0, m_MyRank, 
        MPI_COMM_WORLD);
#endif // PAR_MPI
}
// ----------------------------------------------------------------------------

// recv gaussian learn matrixes from all processes but 0
void CParGibbsSamplingInfEngine::
RecvGaussianStatisticsViaMpi(intVector *pNumberOfSamples, floatVecVector *pLearnMean, floatVecVector *pLearnCov, int NumberOfFactor)
{
#ifdef PAR_MPI
    if (m_MyRank != 0)
        return;

    PNL_CHECK_IS_NULL_POINTER(pNumberOfSamples);
    PNL_CHECK_IS_NULL_POINTER(pLearnMean);
    PNL_CHECK_IS_NULL_POINTER(pLearnCov);

    pFactorVector queryFactors;
    GetQueryFactors( &queryFactors);
    PNL_CHECK_RANGES(NumberOfFactor, 0, queryFactors.size()-1);

    // HaveMsg[0] is never used
    intVector HaveMsg(m_NumberOfProcesses, -1);
    // -1 - no messages
    //  1 - have int
    //  2 - have int and one float array
    //  3 - have int and 2 float arrays

    //Resizing
    pNumberOfSamples->resize(m_NumberOfProcesses,-1);
    pLearnMean->resize(m_NumberOfProcesses);
    pLearnCov->resize(m_NumberOfProcesses);

    int numOfDimsOut;
    const int *rangesOut;
    queryFactors[NumberOfFactor]->GetDistribFun()->GetStatisticalMatrix(stMatMu)
        ->GetRanges(&numOfDimsOut, &rangesOut);

    int Rank = 1;
    int i;
    for (i = 0; i < numOfDimsOut; i++)
    {
        Rank *= rangesOut[i];
    };

    if (!numOfDimsOut)
    {
        Rank = 0;
    };

    // number of recieved msges
    int NumberOfMsg = 0;

    while (NumberOfMsg != (m_NumberOfProcesses - 1))
    {
        for (int i = 1; i < m_NumberOfProcesses; i++)
            if (HaveMsg[i] != 3)
            {
                //have some msg from i ?
                int flag;
                MPI_Status status;

                MPI_Iprobe(i , MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

                if (flag)
                    if (HaveMsg[i] == -1)
                    {
                        MPI_Recv(&(pNumberOfSamples->operator[](i)), 1,
                            MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                        HaveMsg[i] = 1;
                    }
                    else
                    if (HaveMsg[i] == 1)
                    {//HaveMsg[i] == 1
                        MPI_Recv(&(pLearnMean->operator[](i).front()),
                            Rank, MPI_FLOAT, i, MPI_ANY_TAG,
                            MPI_COMM_WORLD, &status);
                        HaveMsg[i] = 2;
                    }
                    else
                    {//HaveMsg[i] == 2
                        MPI_Recv(&(pLearnCov->operator[](i).front()),
                            Rank*Rank, MPI_FLOAT, i, MPI_ANY_TAG,
                            MPI_COMM_WORLD, &status);
                        HaveMsg[i] = 3;
                        NumberOfMsg++;
                    };
            } //if (HaveMsg[i] != 3...
    } //while...
#endif // PAR_MPI
}

// ----------------------------------------------------------------------------
#ifdef PNL_RTTI
const CPNLType CParGibbsSamplingInfEngine::m_TypeInfo = CPNLType("CParGibbsSamplingInfEngine", &(CGibbsSamplingInfEngine::m_TypeInfo));

#endif

#endif // PAR_PNL

// end of file ----------------------------------------------------------------