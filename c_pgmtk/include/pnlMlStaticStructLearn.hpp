/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlMlStaticStructLearn.hpp                                  //
//                                                                         //
//  Purpose:   The base class for all structure learning                   //
//             algorithms on complete data for static graphical models     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLMLSTATICSTRUCTLEARN_HPP__
#define __PNLMLSTATICSTRUCTLEARN_HPP__

#include <math.h>
#include "pnlBNet.hpp"
#include "pnlDAG.hpp"
#include "pnlCPD.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlGaussianCPD.hpp"
#include "pnlTreeCPD.hpp"
#include "pnlSparseMatrix.hpp"
#include "pnlStaticLearningEngine.hpp"
#include "pnlSoftMaxCPD.hpp"
#include "pnlSoftMaxDistribFun.hpp"
#include "pnlCondSoftMaxDistribFun.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

typedef PNL_API enum
{
	BIC,
	AIC,
	WithoutFine,
	VAR	//not implemented	
} EScoreFunTypes;

typedef PNL_API enum
{
	StructLearnHC, //hill climbing
	StructLearnSA,  //Simulated Annealing, not implemented
	StructLearnMCMC	//Markov Chain Monte Carlo, not implemented	
} EOptimizeTypes;

typedef PNL_API enum
{
	MaxLh, //maximized likelihood
	PreAs, //predictive assessment
	MarLh  //marginal likelyhood 
} EScoreMethodTypes;

typedef PNL_API enum
{
	Dirichlet, //using Dirichlet prior
	K2, //using K2 prior
	BDeu  //using Bayesian Dirichlet kikelihood equivalent (uniform) prior 
} EPriorTypes;


class PNL_API CMlStaticStructLearn : public CStaticLearningEngine
{
public:
	
	~CMlStaticStructLearn();    
	virtual void Learn()=0;

	const CBNet* GetResultBNet()const;
	const CDAG*  GetResultDAG()const;
	const int*   GetResultRenaming()const;
	EScoreFunTypes GetScoreFunction();
	EScoreMethodTypes GetScoreMethod();
	EPriorTypes GetPriorType();
	int GetK2PriorParam();
	void  SetMinProgress(float minProgress);

	float ScoreFamily(intVector vFamily);  //vFamily = [parents self]
	float ScoreDAG(CDAG* pDAG, floatVector* familyScore);
	void  CreateResultBNet(CDAG* pDAG);

	void  SetInitGraphicalModel(CGraphicalModel* pGrModel);
	void SetScoreMethod(EScoreMethodTypes Type);
	void SetScoreFunction(EScoreFunTypes ftype);
	void SetPriorType(EPriorTypes ptype);
	// uses only in marginal likelyhood methos with k2 priors
	void SetK2PriorParam(int alfa);
	//used to re-learn from another initial DAG. nodes order and nodes types
	//need to be the same as the original m_pGrModel. 

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CMlStaticStructLearn::m_TypeInfo;
  }
#endif
protected:
	CMlStaticStructLearn(CStaticGraphicalModel *pGrModel, ELearningTypes LearnType, 
						  EOptimizeTypes AlgorithmType, EScoreFunTypes ScoreType, int nMaxFanIn,
						  intVector& vAncestor, intVector& vDescent);

	int		m_nNodes;
	int		m_nMaxFanIn;
	float   m_minProgress;
	int		m_K2alfa; 

	intVector	m_vResultRenaming;
	intVector	m_vAncestor;
	intVector	m_vDescent;
	CBNet*		m_pResultBNet;
	CDAG*		m_pResultDAG;
	EScoreFunTypes	m_ScoreType;
	EOptimizeTypes	m_Algorithm;
	EScoreMethodTypes m_ScoreMethod;
	EPriorTypes m_priorType;
	
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
	float ComputeFamilyScore(intVector vFamily);
	CCPD* ComputeFactor(intVector vFamily, CGraphicalModel* pGrModel, CEvidence** pEvidences);

	CCPD* CreateRandomCPD(int nfamily, const int* family, CGraphicalModel* pGrModel);
	CSparseMatrix<float>**	m_pNodeScoreCache;
};

PNL_END

#endif // __PNLMLSTATICSTRUCTLEARN_HPP__
