/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlStaticStructLearnSEM.hpp                                 //
//                                                                         //
//  Purpose:   The class for structure learning algorithms on missing      //
//             data for Bayesian network                                   //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLSTATICSTRUCTLEARNSEM_HPP__
#define __PNLSTATICSTRUCTLEARNSEM_HPP__

#include <math.h>
#include "pnlBNet.hpp"
#include "pnlDAG.hpp"
#include "pnlCPD.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlGaussianCPD.hpp"
#include "pnlMixtureGaussianCPD.hpp"
#include "pnlStaticLearningEngine.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

class PNL_API CStaticStructLearnSEM : public CStaticLearningEngine
{
public:
	
	static CStaticStructLearnSEM* Create(CBNet* pBNet, 
						  ELearningTypes LearnType, 
						  int nMaxFanIn,
						  intVector* vAncestor=NULL,
						  intVector* vDescent=NULL);
	~CStaticStructLearnSEM();    
	virtual void Learn();
	void SetIterOfEM(int iter);
	void SetMinProgress(float minPorgress);
	void SetMaxLoop(int maxLoop);
    inline CBNet * GetCurrBNet() const;

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CStaticStructLearnSEM::m_TypeInfo;
  }
#endif
protected:
	CStaticStructLearnSEM(CBNet *pBNet, 
						  ELearningTypes LearnType, 
						  int nMaxFanIn, 
						  intVector* vAncestor=NULL, 
						  intVector* vDescent=NULL);

	void ConvertToCurrEvidences(CBNet* pBNet);
	bool LearnOneStep();

	int		m_nNodes;
	int		m_nMaxFanIn;
	int		m_IterEM;
	int		m_maxLoop;
	float	m_minProgress;

	intVector   m_vGlobalRenaming;
	intVector	m_vAncestor;
	intVector	m_vDescent;

	CBNet*		m_pCurrBNet;

	pEvidencesVector  m_vCurrEvidences;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
	void CreateNeighborCPDs(CBNet* pBNet, pCPDVector* vNeighborCPDs, EDGEOPVECTOR* vValidMoves, intVector* MoveCorrespCPD);
	CCPD* CreateRandomCPD(int nfamily, const int* family, CBNet* pBNet);

};

inline CBNet* CStaticStructLearnSEM::GetCurrBNet() const
{
    return m_pCurrBNet;
}

PNL_END

#endif // __PNLSTATICSTRUCTLEARNSEM_HPP__