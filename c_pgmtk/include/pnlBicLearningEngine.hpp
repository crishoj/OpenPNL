/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlBicLearningEngine.hpp                                    //
//                                                                         //
//  Purpose:   Class definition for the structure learning algorithm for   //
//             BNet that uses Bayesian information criterion               //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLBICLEARNINGENGINE_HPP__
#define __PNLBICLEARNINGENGINE_HPP__

#include "pnlStaticLearningEngine.hpp" 

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN
#ifdef SWIG
%rename(IsModelValid) CBICLearningEngineIsInputModelValid(const CStaticGraphicalModel *);
#endif
class PNL_API CBICLearningEngine : public CStaticLearningEngine
{
public:
	static CBICLearningEngine* 
		 Create( CStaticGraphicalModel *pGrModelIn );	
	static bool IsInputModelValid(const CStaticGraphicalModel *pGrModel);
	virtual ~CBICLearningEngine();
	void Learn();
	const CStaticGraphicalModel *GetGraphicalModel() const;
	inline void GetOrder(intVector* reordering) const;

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CBICLearningEngine::m_TypeInfo;
  }
#endif
protected:
	CBICLearningEngine(CStaticGraphicalModel *pGrModel, ELearningTypes LearnType);
	int DimOfModel(const CStaticGraphicalModel *pModel);
	void FindNodesByNumber(int *startNode, int *endNode, 
		int nNodes, int linearNumber);
	inline int Alpha_i(int nNodes, int line);
	
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
	intVector m_resultRenaming;
	CStaticGraphicalModel *m_pResultGrModel;
	
	
	
};

#ifndef SWIG
inline void CBICLearningEngine::GetOrder(intVector* reordering) const
{
    reordering->assign(m_resultRenaming.begin(), m_resultRenaming.end());
}
#endif


PNL_END

#endif //__PNLBICLEARNINGENGINE_HPP__
