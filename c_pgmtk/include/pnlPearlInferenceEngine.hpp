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

#ifndef __PNLPEARLINFERENCEENGINE_HPP__
#define __PNLPEARLINFERENCEENGINE_HPP__

#include "pnlParConfig.hpp"
#include "pnlInferenceEngine.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlGaussianDistribFun.hpp"
#include "pnlNodeType.hpp"
#include "pnlGraph.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN


typedef CDistribFun*             message;

typedef pnlVector< message >     messageVector;

typedef pnlVector<messageVector> messageVecVector;

#ifdef SWIG
%rename(IsModelValid) CPearlInfEngine::IsInputModelValid(const CStaticGraphicalModel*);
#endif

#ifdef PAR_RESULTS_RELIABILITY
class CPearlInfEngine;
bool PNL_API EqualResults(CPearlInfEngine& eng1, CPearlInfEngine& eng2,
    float epsilon = 1e-6);
#endif

class PNL_API CPearlInfEngine : public CInfEngine
{
public:

#ifdef PAR_RESULTS_RELIABILITY
    friend bool EqualResults(CPearlInfEngine&, CPearlInfEngine&, float);
#endif

    static CPearlInfEngine* Create(const CStaticGraphicalModel* pGrModel);

    //static void             Release(CPearlInfEngine** PearlInfEngine);

    static bool             IsInputModelValid(const CStaticGraphicalModel*
                                              pGrModel);

    bool AllContinuousNodes( const CStaticGraphicalModel *pGrModel) const;

    //set max number of iterations for parallel protocol
    inline void       SetMaxNumberOfIterations(int maxNumOfIters);

    // returns the maximal number of iterations for parallel protocol
    inline int        GetMaxNumberOfIterations() const;
    
    // returns the number of iterations Pearl Inference was running
    inline int        GetNumberOfProvideIterations() const;
    
    //set tolerance for check convergency
    inline  void      SetTolerance(float tolerance);

    // returns value of tolerance for check convergency
    inline float      GetTolerance() const;
    
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

    ~CPearlInfEngine();

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CPearlInfEngine::m_TypeInfo;
  }
#endif
protected:

    int                 m_IterationCounter;
    
    EDistributionType   m_modelDt;
    
    CPearlInfEngine( const CStaticGraphicalModel *pGraphicalModel );

  
    inline void    CreateMessagesStorage();
    
    inline void    ClearMessagesStorage();
    
    void           InitEngine(const CEvidence*evidence);
    
    void           InitMessages(const CEvidence* evidence);    

    inline message InitMessage( int destination, const CNodeType* type,
                                int isInMoment = 0 ) const;
    
    message        InitMessageWithEvidence( int node, const CNodeType* type,
                                            int val );
    
    message        InitMessageWithEvidence( int node, const CNodeType* type,
                                            float* val );

    message        ComputeMessage( int destination, int source,
                                   int orientFlag ) const;
    
    message        ComputeProductPi( int nodeNumber, int except = -1 ) const;
    
    message        ComputeProductLambda( int nodeNumber, message lambda,
                                         int except = -1 ) const;

    message        ProductLambdaMsgs( int nodeNumber, int except = -1 ) const;
    
    
    void           ComputeBelief(int nodeNumber);
    
    void           ParallelProtocol();
    
    void           TreeProtocol();

    inline const int            GetNumberOfNodesInModel() const;

    inline const CGraph*        GetModelGraph() const;

    inline intVector&           GetConnectedNodes();

    inline intVector&           GetSignsOfReallyObserved();

    inline messageVector&       GetSelfMessages();

    virtual messageVector&      GetCurBeliefs();

    inline messageVecVector&    GetMessagesFromNeighbors();

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:

    const int           m_numOfNdsInModel;

    const CModelDomain* m_pModelDomain;

    const CGraph*       m_pModelGraph;

    
    messageVector       m_selfMessages;	
    
    messageVecVector    m_messagesFromNeighbors;
    
    messageVector       m_beliefs;//beliefs for every node
    
    int                 m_maxNumberOfIterations;
    
    float               m_tolerance;//tolerance to compare beliefs
    
    intVector           m_connNodes;//we need to have it for Gaussian models -  fixme
    
    intVector           m_areReallyObserved;//we need to compute beliefs only for them
};

#ifndef SWIG
/////////////////////////////////////////////////////////////////////////////

inline int CPearlInfEngine::GetNumberOfProvideIterations() const
{
    return m_IterationCounter;
}
/////////////////////////////////////////////////////////////////////////////

inline void CPearlInfEngine::SetMaxNumberOfIterations(int maxNumOfIters)
{
    PNL_CHECK_LEFT_BORDER( maxNumOfIters, 1 );
    
    m_maxNumberOfIterations = maxNumOfIters;
}
/////////////////////////////////////////////////////////////////////////////

inline int CPearlInfEngine::GetMaxNumberOfIterations() const
{
    return m_maxNumberOfIterations;
}
/////////////////////////////////////////////////////////////////////////////

inline void CPearlInfEngine::SetTolerance( float tolerance)
{
    PNL_CHECK_RANGES( tolerance, FLT_MIN, FLT_MAX );

    m_tolerance = tolerance;
}
/////////////////////////////////////////////////////////////////////////////

inline float CPearlInfEngine::GetTolerance() const
{
    return m_tolerance;
}
/////////////////////////////////////////////////////////////////////////////

inline void CPearlInfEngine::CreateMessagesStorage()
{
    messageVecVector::iterator mfn_begin = m_messagesFromNeighbors.begin(),
                               mfn_end   = m_messagesFromNeighbors.end(),
                               mfnIt     = mfn_begin;
    
    for( ; mfnIt != mfn_end; ++mfnIt )
    {
        int numOfNbrs = m_pModelGraph->GetNumberOfNeighbors(mfnIt - mfn_begin);

        mfnIt->resize( numOfNbrs, NULL );
    }
}
/////////////////////////////////////////////////////////////////////////////

inline void CPearlInfEngine::ClearMessagesStorage()
{
    messageVector::iterator    belIt   = m_beliefs.begin(),
                               bel_end = m_beliefs.end(),
                               smIt    = m_selfMessages.begin();
    
    messageVecVector::iterator mfnIt   = m_messagesFromNeighbors.begin();
    
    for( ; belIt != bel_end; ++belIt, ++smIt, ++mfnIt )
    {
        delete *belIt;

        *belIt = NULL;


        delete *smIt;
        
        *smIt = NULL;

        for( messageVector::iterator messIt   = mfnIt->begin(),
                                     mess_end = mfnIt->end();
            messIt != mess_end;
            ++messIt )
        {
            delete *messIt;

            *messIt = NULL;
        }
    }
    
    m_connNodes.clear(); // ???????
    
    m_areReallyObserved.clear(); // ???????
}
/////////////////////////////////////////////////////////////////////////////

inline message CPearlInfEngine::InitMessage( int destination,
                                             const CNodeType* type,
                                             int isInMoment ) const
{
    message retMes = NULL;
    
    switch( m_modelDt )
    {
        case dtTabular:
        {
            int size = type->GetNodeSize();
            
            floatVector dataVec( size, 1.0f );
            
            retMes = CTabularDistribFun::Create( 1, &type, &dataVec.front());
            
            // retMes = CTabularDistribFun::CreateUnitFunctionDistribution() ??????????????????????????
            
            break;
        }
        default:
        {
            //we create all messages in moment form, send and multiply all messages in this form!
            //all messages except observed send uniform distribution in canonical form
            //observed nodes send delta function in moment form
            //beliefs update moment form!
            retMes = CGaussianDistribFun::CreateUnitFunctionDistribution( 1, &type,
                1, 1 - isInMoment );
            break;			
        }
    }
    
    return retMes;
}
/////////////////////////////////////////////////////////////////////////////

inline const int CPearlInfEngine::GetNumberOfNodesInModel() const
{
    return m_numOfNdsInModel;
}
/////////////////////////////////////////////////////////////////////////////

inline const CGraph* CPearlInfEngine::GetModelGraph() const
{
    return m_pModelGraph;
}
/////////////////////////////////////////////////////////////////////////////

inline intVector& CPearlInfEngine::GetConnectedNodes()
{
    return m_connNodes;
}
/////////////////////////////////////////////////////////////////////////////

inline intVector& CPearlInfEngine::GetSignsOfReallyObserved()
{
    return m_areReallyObserved;
}
/////////////////////////////////////////////////////////////////////////////

inline messageVector& CPearlInfEngine::GetSelfMessages()
{
    return m_selfMessages;
}
/////////////////////////////////////////////////////////////////////////////

inline messageVecVector& CPearlInfEngine::GetMessagesFromNeighbors()
{
    return m_messagesFromNeighbors;
}
/////////////////////////////////////////////////////////////////////////////


#endif

PNL_END

#endif //__PNLPEARLINFERENCEENGINE_HPP__
