/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlNaiveInferenceEngine.hpp                                 //
//                                                                         //
//  Purpose:   CNaiveInferenceEngine class definition                      //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// NaiveInfEng.hpp: interface for the CNaiveInfEng class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __PNLNAIVEINFENG_HPP__
#define __PNLNAIVEINFENG_HPP__

#include "pnlInferenceEngine.hpp"
#include "pnlNodeType.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

class PNL_API CNaiveInfEngine : public CInfEngine
{
public:
	static CNaiveInfEngine* Create(
		const CStaticGraphicalModel *pGraphicalModel);
    void SetExclude( const intVector& excludeNodes );
#ifdef PNL_OBSOLETE
	void SetExclude( int nNodesExclude, const int *excludeNodes );
   	virtual void MarginalNodes( const int *query, int querySize, int notExpandJPD = 0 );
#endif
	virtual void EnterEvidence( const CEvidence *evidence, int maximize = 0,
        int sumOnMixtureNode = 1 );
	virtual const CPotential* GetQueryJPD() const;
	virtual const CEvidence* GetMPE() const;
	~CNaiveInfEngine();

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CNaiveInfEngine::m_TypeInfo;
  }
#endif
protected:
	CNaiveInfEngine( const CStaticGraphicalModel *pGraphicalModel );

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
	intVector			m_excludeNodes;
	//const CEvidence *	m_pEvidence;
	/*nodes which are not included in JPD, their factors&CPDs are not use here*/
	CPotential *		m_pJPD;
};

PNL_END

#endif //__PNLNAIVEINFENG_HPP__
