/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlFactorGraph.hpp                                          //
//                                                                         //
//  Purpose:   CFactorGraph class definition                               //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLFGSUMMAXINFENG_HPP__
#define __PNLFGSUMMAXINFENG_HPP__

#include "pnlInferenceEngine.hpp"
#include "pnlFactorGraph.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlGaussianPotential.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

typedef CPotential* fgMessage;

typedef pnlVector< fgMessage > fgMessageVector;
typedef pnlVector< fgMessageVector > fgMessageVecVector;

class PNL_API CFGSumMaxInfEngine : public CInfEngine
{
public:
    static CFGSumMaxInfEngine* Create( 
		const CStaticGraphicalModel *pGraphicalModel );
	//this function add evidence to the model and provide inference
	virtual void EnterEvidence( const CEvidence *evidence, int maximize = 0,
        int sumOnMixtureNode = 1);
	//this function creates joint distribution and MPE over family of query(if query is more than 1 node)
	//or joint distribution and MPE for one node (query size = 1)
#ifdef PNL_OBSOLETE
	virtual void MarginalNodes( const int *query, int querySize, int notExpandJPD = 0 );
#endif
	//returns JPD for query from MarginalNodes
	virtual const CPotential* GetQueryJPD() const;
	//returns MPE for query from MarginalNodes
	virtual const CEvidence* GetMPE() const;
	//set max number of iterations for parallel protocol
	void SetMaxNumberOfIterations( int number );
	inline int GetNumberOfProvideIterations() const;
	//set tolerance for check convergency
	void SetTolerance( float tolerance);
	virtual ~CFGSumMaxInfEngine();

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CFGSumMaxInfEngine::m_TypeInfo;
  }
#endif
protected:
	CFGSumMaxInfEngine( const CFactorGraph *pFactorGraph );
	void InitMessages();
	void InitEngine();
	fgMessage CompMessageToFactor( int sourseNode, int numFactor );
    fgMessage CompMessageFromFactor( int posOfDestNode, int numFactor,
        int destNode );
	void ParallelProtocol();
	void TreeProtocol();
	void DestroyAllMessages();
    inline fgMessage InitUnitMessage(int node);

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
    //create new factor graph with shrinked potentials
    CFactorGraph* m_pShrinkedFG;
    
	fgMessageVecVector m_curFGMessagesToVars;
	//vector of vectors to pointers to messages recieved from neighbors - 
	//						the same structure as FactorGraph
    //for every node the list of messages recieved from neighboring factors
    fgMessageVecVector m_curFGMessagesToFactors;
    //vector of vectors of pointers to messages from neighbor variables for every factor
    //the order of messages is the same as the order in domain

    //need in parallel protocol - the same as current but used in different time
	fgMessageVecVector m_newFGMessagesToVars;
    fgMessageVecVector m_newFGMessagesToFactors;

    //create indices for every multiplication
    pnlVector<intVecVector> m_indicesForMultVars;
    //variable i send message to factor nbr[j] -> indicesForMultVars[i][j] is an array
    //of indices in m_curFGMessagesToVars need to be multiplyed 
    pnlVector<intVecVector> m_indicesForMultFactors;
    //factor i send message to variable j -> indicesForMultFactors[i][j] is an array
    //of indices in m_curFGMessagesToFactors need to be multiplyed 

	fgMessageVector m_beliefs;//beliefs for every node
	fgMessageVector m_oldBeliefs;//need to check convergence
	int m_maxNumberOfIterations;
	int m_IterationCounter;
	float m_tolerance;//tolerance to compare beliefs
	EDistributionType m_modelDt;
	intVector m_areReallyObserved;//we need to compute beliefs only for them
    //determine are the matrices dense or sparse
    int m_bDense;
};

#ifndef SWIG
inline int CFGSumMaxInfEngine::GetNumberOfProvideIterations() const
{
	return m_IterationCounter;
}

inline fgMessage CFGSumMaxInfEngine::InitUnitMessage(int node)
{
    if( m_modelDt == dtTabular )
    {
        fgMessage res = CTabularPotential::CreateUnitFunctionDistribution(
                        &node, 1, m_pGraphicalModel->GetModelDomain(), m_bDense );
        return res;
    }
    else if( m_modelDt == dtGaussian )
    {
        fgMessage res = CGaussianPotential::CreateUnitFunctionDistribution(
                        &node, 1, m_pGraphicalModel->GetModelDomain() );
        return res;
    }
    return NULL;
}


#endif


PNL_END

#endif // __PNLFGSUMMAXINFENG_HPP__
