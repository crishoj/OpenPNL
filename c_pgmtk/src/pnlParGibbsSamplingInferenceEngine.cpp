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
#include "pnlMixtureGaussiancpd.hpp"
#include "pnlMNet.hpp"

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

    CGibbsSamplingInfEngine::MarginalNodes(queryIn, querySz, notExpandJPD);

    bool IsUnobservedNodes = false;

    for (int variable = 0; variable < querySz; variable++)
    {
        if (!(m_pEvidence->IsNodeObserved(queryIn[variable])))
            IsUnobservedNodes = true;
    }

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
                (m_currentTime-GetBurnIn()) * GetNumStreams();

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
                FinalDistrib.begin(), obsIndicesIn);

            delete ShrinkPot;

            ShrinkPot = deltaPot->ExpandObservedNodes(m_pEvidence);

            m_pQueryJPD->SetDistribFun(ShrinkPot->GetDistribFun());

            delete deltaPot;
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
    int NumberOfSamples = (m_currentTime-GetBurnIn()) * GetNumStreams();
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
                bool canBeSample =
                    ConvertingFamilyToPot( digit, pCurrentEvidence );

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
                
                if (t > BurnIn)
                {
                    pFactorVector queryFactors;
                    GetQueryFactors( &queryFactors );
                    int ii;
                    int queryFactorsSize =
                        ((float)(queryFactors.size())) / (m_NumberOfThreads);
                    for( ii = 0; ii < queryFactorsSize; ii++ )
                    {
                        was_updated[ii + myid * queryFactorsSize] = true;
                        queryFactors[ii + myid * queryFactorsSize]->
                            UpdateStatisticsML(&pCurrentEvidence, 1);
                    }
                };
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
    //intVector ndsForSampling;
    //GetNdsForSampling( &ndsForSampling );
    int ndsSize = m_NumberOfNodesForSample;//ndsForSampling.size();
    int nodeModndsSize = node%ndsSize;
    pFactorVector& currentFactors = *GetCurrentFactors();

    bool ret = false;
    CPotential* potToSample = GetPotToSampling(node);
    Normalization(potToSample);
    int i;
    if( !IsAllNdsTab() )
    {
        if( GetModel()->GetModelType() == mtBNet )
        {
            for( i = 0; i < m_environment[nodeModndsSize].size(); i++ )
            {
                int num = m_environment[nodeModndsSize][i];
                CPotential *pot1 =
                    static_cast< CCPD* >( currentFactors[ num ] )->
                    ConvertWithEvidenceToPotential(pEv);
                int delta = nodeModndsSize;
                CPotential *pot2 = pot1->Marginalize(&delta, 1);
                delete pot1;
                *potToSample *= *pot2;
                delete pot2;
            }
        }
        else
        {
            for( i = 0; i < m_environment[nodeModndsSize].size(); i++ )
            {
                int num = m_environment[nodeModndsSize][i];
                CPotential *pot1 =
                    static_cast< CPotential* >( currentFactors[ num ] )->
                    ShrinkObservedNodes(pEv);
                int delta = nodeModndsSize;
                CPotential *pot2 = pot1->Marginalize(&delta, 1);
                delete pot1;
                *potToSample *= *pot2;
                delete pot2;
            }
        }
    }
    else
    {
        CMatrix< float > *pMatToSample;
        pMatToSample = 
            static_cast<CTabularDistribFun*>(potToSample->GetDistribFun())->
            GetMatrix(matTable);
        intVector dims;
        intVector vls;
        intVector domain;

        for( i = 0; i < m_environment[nodeModndsSize].size(); i++ )
        {
            int num = m_environment[nodeModndsSize][i];
            currentFactors[ num ]->GetDomain(&domain);
            GetObsDimsWithVls( domain, nodeModndsSize, pEv, &dims, &vls); 
            CMatrix< float > *pMat;
            pMat = static_cast<CTabularDistribFun*>(currentFactors[ num ]->
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
        CGaussianDistribFun* pDistr =
            static_cast<CGaussianDistribFun*>(potToSample->GetDistribFun());
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

    //Цикл создания дополнительных факторов для каждого из процессов
    for (int proc = 0; proc < NumThreads; proc++)
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
// ----------------------------------------------------------------------------

#endif // PAR_PNL

// end of file ----------------------------------------------------------------