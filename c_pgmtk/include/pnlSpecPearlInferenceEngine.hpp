/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPearlInferenceEngine.hpp                                 //
//                                                                         //
//  Purpose:   CPearlInferenceEngine class definition                      //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// PearlInfEngine.hpp

#ifndef __PNLPEARLINFERENCEENGINE1_HPP__
#define __PNLPEARLINFERENCEENGINE1_HPP__

#include "pnlInferenceEngine.hpp"
#include "pnlDistribFun.hpp"
#include "pnlNodeType.hpp"
#include "pnlGraph.hpp"
#include "pnlFGSumMaxInferenceEngine.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN


//typedef CPotential*              fgMessage;

//typedef pnlVector< fgMessage >     fgMessageVector;

//typedef pnlVector<fgMessageVector> fgMessageVecVector;


class PNL_API CSpecPearlInfEngine : public CInfEngine
{
public:

    static CSpecPearlInfEngine* Create(const CStaticGraphicalModel* pGrModel);

    //static void             Release(CSpecPearlInfEngine** PearlInfEngine);

    static bool             IsInputModelValid(const CStaticGraphicalModel*
                                              pGrModel);

    //set max number of iterations for parallel protocol
    inline void       SetMaxNumberOfIterations(int maxNumOfIters);
    
    // returns the number of iterations Pearl Inference was running
    inline int        GetNumberOfProvideIterations() const;
    
    //set tolerance for check convergency
    inline  void      SetTolerance(float tolerance);
    
    // this function add evidence to the model and provide inference
    void              EnterEvidence( const CEvidence* evidence,
                                     int maximize = 0,
                                     int sumOnMixtureNode = 1 );
    
    // this function creates joint distribution and MPE
    // over family of query(if query is more than 1 node)
    // or joint distribution and MPE for one node (query size = 1)

#ifdef PNL_OBSOLETE
    void              MarginalNodes( const int* query, int querySize,
                                     int notExpandJPD = 0 );
#endif

    ~CSpecPearlInfEngine();


#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CSpecPearlInfEngine::m_TypeInfo;
  }
#endif
protected:

    CSpecPearlInfEngine( const CStaticGraphicalModel *pGraphicalModel );

    inline void    CreateMessagesStorage();
    
    inline void    ClearMessagesStorage();
    
    void           InitEngine(const CEvidence*evidence);
    
    void           InitMessages(const CEvidence* evidence);
     
    void           ComputeMessage( int destination, int source,
                                   int orientFlag, fgMessage& mesInOut) const;
    
    void           ComputeProductPi( int nodeNumber, fgMessage& resMes, 
                                     int except = -1, int multFlag = 0 ) const;
    
    void           ComputeProductLambda( int nodeNumber, fgMessage lambda,
                                  fgMessage& resMes, int except = -1 ) const;

    void           ProductLambdaMsgs( int nodeNumber, fgMessage& resMes,
                                                    int except = -1 ) const;
    
    
    void           ComputeBelief(int nodeNumber);
    
    void           ParallelProtocol();
    
    void           TreeProtocol();

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:

    const int           m_numOfNdsInModel;

    const int           m_bDense;

    //we compute at every iteration fgMessages for next state based on current
    int                 m_curState;
    int                 m_nextState;

    CModelDomain*       m_pModelDomain;

    const CGraph*       m_pModelGraph;
    
    fgMessageVector       m_selfMessages;	
                                    
    pnlVector<fgMessageVecVector>     m_curMessages;

    
    //for every pair of nodes if they are connected there is a potential 
    //(the same as the potential in the model) - for MNet
    fgMessageVecVector    m_nbrDistributions;

    //for every child node there is corresponding distribution from CPD
    pnlVector<CDistribFun*> m_familyDistributions;    
    
    fgMessageVecVector    m_beliefs;//beliefs for every node

    int                 m_maxNumberOfIterations;
    
    int                 m_IterationCounter;
    
    float               m_tolerance;//tolerance to compare beliefs
        
    intVector           m_connNodes;//we need to have it for Gaussian models 

    EDistributionType   m_modelDt;//distribution type for the model

    EModelTypes         m_modelType;//model type - BNet or MRF2

    intVector           m_areReallyObserved;//we need to compute beliefs only for them
};

#ifndef SWIG
/////////////////////////////////////////////////////////////////////////////

inline int CSpecPearlInfEngine::GetNumberOfProvideIterations() const
{
    return m_IterationCounter;
}
/////////////////////////////////////////////////////////////////////////////

inline void CSpecPearlInfEngine::SetMaxNumberOfIterations(int maxNumOfIters)
{
    PNL_CHECK_LEFT_BORDER( maxNumOfIters, 1 );
    
    m_maxNumberOfIterations = maxNumOfIters;
}
/////////////////////////////////////////////////////////////////////////////

inline void CSpecPearlInfEngine::SetTolerance( float tolerance)
{
    PNL_CHECK_RANGES( tolerance, FLT_MIN, FLT_MAX );

    m_tolerance = tolerance;
}
/////////////////////////////////////////////////////////////////////////////

inline void CSpecPearlInfEngine::CreateMessagesStorage()
{
    fgMessageVecVector::iterator mc_begin = m_curMessages[0].begin(),
                               mc_end   = m_curMessages[0].end(),
                               mcIt     = mc_begin,
                               mnIt     = m_curMessages[1].begin(),
                               mnbrDistrIt = m_nbrDistributions.begin();
    
    for( ; mcIt != mc_end; ++mcIt, ++mnIt, ++mnbrDistrIt )
    {
        int numOfNbrs = m_pModelGraph->GetNumberOfNeighbors(mcIt - mc_begin);

        mcIt->resize( numOfNbrs, NULL );

        mnIt->resize( numOfNbrs, NULL );

        mnbrDistrIt->resize( m_numOfNdsInModel, NULL );
    }
    
}
/////////////////////////////////////////////////////////////////////////////

inline void CSpecPearlInfEngine::ClearMessagesStorage()
{
    fgMessageVector::iterator    belIt    = m_beliefs[0].begin(),
                               bel_end  = m_beliefs[0].end(),
                               smIt     = m_selfMessages.begin(),
                               oldBelIt = m_beliefs[1].begin();
    pnlVector<CDistribFun*>::iterator   famDistrIt = m_familyDistributions.begin();
    
    fgMessageVecVector::iterator mcIt     = m_curMessages[0].begin(),
                               mnIt     = m_curMessages[1].begin(),
                               mnbrIt   = m_nbrDistributions.begin();
    
    for( ; belIt != bel_end; ++belIt, ++smIt, ++famDistrIt, ++mcIt, ++mnIt, ++mnbrIt )
    {
        delete *belIt;
        *belIt = NULL;

        delete *smIt;
        *smIt = NULL;

        delete *famDistrIt;
        *famDistrIt = NULL;

        delete *oldBelIt;
        *oldBelIt = NULL;

        fgMessageVector::iterator newMesIt      = mnIt->begin();
        for( fgMessageVector::iterator messIt   = mcIt->begin(),
                                     mess_end = mcIt->end();
            messIt != mess_end;
            ++messIt, ++newMesIt )
        {
            delete *messIt;
            *messIt = NULL;

            delete *newMesIt;
            *newMesIt = NULL;
        }
        
        for( fgMessageVector::iterator nbrFunIt = mnbrIt->begin(),
                                     nbrFnEnd = mnbrIt->end();
                                     nbrFunIt != nbrFnEnd;
                                     ++nbrFunIt )
        {
            delete *nbrFunIt;
            *nbrFunIt = NULL;
        }

    }
    
    //m_connNodes.clear(); 
    //m_areReallyObserved.clear(); 
}

#endif

PNL_END

#endif //__PNLPEARLINFERENCEENGINE1_HPP__
