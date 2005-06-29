/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlDiagnostics.cpp                                          //
//                                                                         //
//  Purpose:   CDiagnostics class member functions implementation          //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Bader, Chernishova, Gergel, Senin, Sidorov,     //
//             Sysoyev, Tarasov, Vinogradov                                //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlDiagnostics.hpp"
#include "pnlJtreeInferenceEngine.hpp"

PNL_USING

CDiagnostics *CDiagnostics::Create( const CBNet *pBNet)
{
    /* bad-args check */
    PNL_CHECK_IS_NULL_POINTER(pBNet);
    /* bad-args check end */

    CDiagnostics *pDiagnostics = new CDiagnostics(pBNet);

    PNL_CHECK_IF_MEMORY_ALLOCATED(pDiagnostics);

    return pDiagnostics;
}

CDiagnostics::CDiagnostics(const CBNet *pBNet)
{
    aFlag.resize(pBNet->GetNumberOfNodes(),0);
    m_observCost.resize(pBNet->GetNumberOfNodes(),0);
    m_BNet = pBNet;
    m_evid = NULL;
    algorithmNumber = 0;
    costRatio = 0;
}

CDiagnostics::~CDiagnostics()
{
    
}

void CDiagnostics::SetTargetNodes(intVector &targetNodes)
{
    int i;
    for( i=0; i <targetNodes.size(); ++i)
    {
        aFlag[targetNodes[i]] |= 1;
    }
}

void CDiagnostics::SetObservationNodes(intVector &observationNodes)
{
    int i;
    for( i=0; i <observationNodes.size(); ++i)
    {
        aFlag[observationNodes[i]] |= 2;
    }
}

void CDiagnostics::SetTargetState(int node, int state)
{
    int i;
    bool flag=false;
    for (i = 0; i < targetStates.size(); ++i)
    {
        if( (targetStates[i].first == node) && 
            (targetStates[i].second == state) )
            flag = true;
    }
    if( !flag )
    {
        pair<int,int> temp;
        temp.first = node;
        temp.second = state;
        targetStates.push_back(temp);
    }
}

void CDiagnostics::SetTargetStates(int node, intVector &states)
{
    int i;
    int j;
    intVector nodeStates;
    for( j = 0; j < targetStates.size(); ++j )
    {
        if( targetStates[j].first == node)
            nodeStates.push_back(targetStates[j].second );
    }
    for( j = 0; j < states.size(); ++j )
    {
        bool flag=false;
        for( i = 0; i < nodeStates.size(); ++i )
        {
            if( nodeStates[i] == states[j] )
                flag = true;
        }
        if( !flag )
        {
            pair<int,int> temp;
            temp.first = node;
            temp.second = states[j];
            targetStates.push_back(temp);
        }
    }
}

void CDiagnostics::GetTestsList(const intVector &pursuedHypNodes, 
			const intVector &pursuedHypNodesState, intVector &listTestNodes,
		doubleVector &listVOI)
{
    pnlVector<pair<int,float> > rankedTests;
    int i, j;
    if( pursuedHypNodes.size() != pursuedHypNodesState.size() )
    {
        PNL_THROW( CInconsistentSize, "incorrect arguments" );
    }
    for( i = 0; i < pursuedHypNodes.size(); ++i )
    {
        bool f = false;
        for( j = 0; j < targetStates.size(); ++j )
        {
            if( (targetStates[j].first == pursuedHypNodes[i]) &&
                (targetStates[j].second == pursuedHypNodesState[i]) )
                f = true;
        }
        if( !f )
        {
            PNL_THROW( CBadArg, "set necessary target states before using GetTestsList method" );
        }
    }
    pair<int,float> temp;
    intVector hypList;
    for( i = 0; i < aFlag.size(); ++i )
    {
        if( aFlag[i] & 1 )
        {
            hypList.push_back(i);
        }
        else
            if( aFlag[i] & 2 )
            {
                temp.first = i;
                temp.second = 0;
                rankedTests.push_back(temp);
            }
    }
    if( rankedTests.size() == 0 )
        PNL_THROW( COutOfRange, "You must set a test nodes" );
    for( i = 0; i < pursuedHypNodes.size(); ++i )
    {
        bool f = false;
        for( j = 0; j < hypList.size(); ++j )
            if( hypList[j] == pursuedHypNodes[i] )
                f = true;
        if( !f )
            PNL_THROW( CBadArg, "set necessary target states before using"
            "GetTestsList method" );
    }
    if(( pursuedHypNodes.size() == 1) && ( algorithmNumber == 1 ))
		algorithmNumber = 0;
/*        PNL_THROW( CBadArg, "The Marginal Strength 2 algorithm can`t be used"
        " with this pursuedNodes" );*/
    for( i = 0; i < rankedTests.size(); ++i )
    {
        float prob;
        float expectedValue = 0;
        double purFault = 1 - 1/(double)(pursuedHypNodes.size());
        for( j = 0; j < m_BNet->GetModelDomain()->
            GetVariableType(rankedTests[i].first)->GetNodeSize(); ++j )
        {
            prob = 0; // Value Of Information
            const int obsNodesNums[] = { rankedTests[i].first };
            Value val;
            val.SetInt(j);
            valueVector obsNodesVals;
            obsNodesVals.push_back(val);
            CEvidence *evid = CEvidence::Create( m_BNet->GetModelDomain(), 1,
                obsNodesNums, obsNodesVals );
            CJtreeInfEngine *JTInf = CJtreeInfEngine::Create( m_BNet );
            JTInf->EnterEvidence( evid );

            // start computing marginal strength MS1
            int k;
            for( k = 0; k < pursuedHypNodes.size(); ++k )
            {
                intVector query;
                query.push_back(pursuedHypNodes[k]);
                JTInf->MarginalNodes( &query.front(), 1 );
                int const *ind = &pursuedHypNodesState[k];
                double faultMarginal = (JTInf->GetQueryJPD()->GetMatrix(matTable))->
                    GetElementByIndexes(ind);
                if (algorithmNumber == 0)
                    prob += (float)((faultMarginal - 0.5f) * (faultMarginal - 0.5f));
                else 
                    if (algorithmNumber == 1)
                    {
                        if (faultMarginal <= purFault)
                            prob += (float)((faultMarginal - purFault) * (faultMarginal - purFault));
                        else 
                            prob += (float)((faultMarginal - purFault) * (faultMarginal - purFault)
                            *(pursuedHypNodes.size() - 1)*(pursuedHypNodes.size() - 1));
                    }
                    
            }
            if (algorithmNumber == 0)
            {
                prob *= (4/(float)(pursuedHypNodes.size()));
            }
            else
            {
                prob /= (float)((purFault*purFault*pursuedHypNodes.size()));
            }
            prob -= 1;
            // finish computing marginal strength MS1

            valueVector obsNULL;
            CEvidence *evidNULL = CEvidence::Create( m_BNet->GetModelDomain(), 0,
                0, obsNULL );
            CJtreeInfEngine *JTInfForTest = CJtreeInfEngine::Create( m_BNet );
            intVector query;
            query.push_back(rankedTests[i].first);
            JTInfForTest->EnterEvidence(evidNULL);
            JTInfForTest->MarginalNodes( &query.front(), 1 );
            int const ind[] = {j};
            expectedValue += prob*((JTInfForTest->GetQueryJPD()->
                GetMatrix(matTable))->GetElementByIndexes(ind));
        }
        float testVal = 0;
        CJtreeInfEngine *JTInf = CJtreeInfEngine::Create( m_BNet );
        valueVector obsNULL;
        CEvidence *evidNULL = CEvidence::Create( m_BNet->GetModelDomain(), 0,
            0, obsNULL );

        // start computing marginal strength MS1
        int k;
        for( k = 0; k < pursuedHypNodes.size(); ++k )
        {
            intVector query;
            query.push_back(pursuedHypNodes[k]);
            JTInf->EnterEvidence(evidNULL);
            JTInf->MarginalNodes( &query.front(), 1 );
            int const *ind = &pursuedHypNodesState[k];
            double faultMarginal = (JTInf->GetQueryJPD()->GetMatrix(matTable))->
                GetElementByIndexes(ind);
            if (algorithmNumber == 0)
                testVal += (float)((faultMarginal - 0.5f) * (faultMarginal - 0.5f));
            else 
                if (algorithmNumber == 1)
                {
                    if (faultMarginal <= purFault)
                        testVal += (float)((faultMarginal - purFault) * (faultMarginal - purFault));
                    else 
                        testVal += (float)((faultMarginal - purFault) * (faultMarginal - purFault)
                        *(pursuedHypNodes.size() - 1)*(pursuedHypNodes.size() - 1));
                }
        }
        if (algorithmNumber == 0)
        {
            testVal *= (4/(float)(pursuedHypNodes.size()));
        }
        else
        {
            testVal /= (float)((purFault*purFault*pursuedHypNodes.size()));
        }
        testVal -= 1;
        // finish computing marginal strength MS1

        float expectedBenefit = expectedValue - testVal;
        rankedTests[i].second = (-1)*(expectedBenefit/testVal);// - costRatio*m_observCost[rankedTests[i].first];
    }
    intVector res;
    intVector resNotBenfit;
    if( rankedTests[0].second < -1 )
        resNotBenfit.push_back(0);
    else 
        res.push_back(0);
    int counter = 0;
    for( j = 1; j < rankedTests.size(); ++j )
    {   
        if(rankedTests[j].second < -1)
        {
            while((counter < resNotBenfit.size()) && 
                (rankedTests[j].second > rankedTests[resNotBenfit[counter]].second))
            {
                ++counter;
            }
            resNotBenfit.insert(resNotBenfit.begin() + counter, j);
        }
        else
        {
            while((counter < res.size()) && 
                (rankedTests[j].second > rankedTests[res[counter]].second))
            {
                ++counter;
            }
            res.insert(res.begin() + counter, j);
        }
        counter=0;
    }
    res.insert(res.end(),resNotBenfit.begin(),resNotBenfit.begin()+resNotBenfit.size());
    listTestNodes.resize(res.size());
    listVOI.resize(res.size());
    for( i = 0; i < res.size(); ++i)
    {
        listTestNodes[i] = rankedTests[res[i]].first;
		listVOI[i] = rankedTests[res[i]].second;
    }
}

void CDiagnostics::SetCost( int node, float nodeCost )
{
    PNL_CHECK_RANGES( node, 0, m_BNet->GetNumberOfNodes() - 1 );
    if( nodeCost < 0 )
    {
        PNL_THROW( COutOfRange, "observation cost must be positive" ); 
    }
    m_observCost[node] = nodeCost;
}

void CDiagnostics::SetCostRatio(float costR)
{
    if (costR >= 0)
        costRatio = costR;
    else 
        PNL_THROW( CBadArg, "bad argument for SetCostRatio method" );
}

void CDiagnostics::SetAlgorithm(int algNumber)
{
    PNL_CHECK_RANGES( algNumber, 0, 1 );
    algorithmNumber = algNumber;
}

double CDiagnostics::GetEntropyCostRatio()
{
    return costRatio;
}

/*----- for GeNIe support -----*/
/*
void CDiagnostics::EditEvidence(int node, int Value)
{
    PNL_CHECK_RANGES( node, 0, m_BNet->GetNumberOfNodes() - 1 );
    if( !m_evid->IsNodeObserved(node) )
    {
        m_evid->MakeNodeObserved(node);
        *(m_evid->GetValue(node)) = Value;
    }
}

void CDiagnostics::ClearEvidence(int node)
{
    int i;
    if( node > m_BNet->GetNumberOfNodes() )
    {
        PNL_THROW( CBadArg, "bad argument for ClearEvidence method" );
    }
    if( node < 0 ) // delete all information
    {
        for( i = 0; i < m_BNet->GetNumberOfNodes(); ++i )
        {
            if( m_evid->IsNodeObserved(i) )
                m_evid->MakeNodeHidden(i);
        }
    }
    else // clear only one observation
    {
        if( m_evid->IsNodeObserved(node) )
            m_evid->MakeNodeHidden(node);
    }
}

void CDiagnostics::SetEvidence(CEvidence *pEvid)
{
    PNL_CHECK_IS_NULL_POINTER(pEvid);
    m_evid = pEvid;
}*/

#ifdef PNL_RTTI
const CPNLType CDiagnostics::m_TypeInfo = CPNLType("CDiagnostics", &(CPNLBase::m_TypeInfo));

#endif