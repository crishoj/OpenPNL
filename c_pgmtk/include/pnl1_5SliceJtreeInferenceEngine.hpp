/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnl1_5SliceJtreeInferenceEngine.hpp                         //
//                                                                         //
//  Purpose:   Class definition for the 1.5 Slice Junction tree            //
//             inference lgorithm                                          //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNL1_5SLICEJTREEINFERENCEENGINE_HPP__
#define __PNL1_5SLICEJTREEINFERENCEENGINE_HPP__


#include "pnl1_5SliceInferenceEngine.hpp"
#include "pnlJunctionTree.hpp"
#include "pnlJtreeInferenceEngine.hpp"
#ifdef PAR_OMP
    #include "pnlParJtreeInferenceEngine.hpp"
#endif

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

class PNL_API C1_5SliceJtreeInfEngine : public C1_5SliceInfEngine
{
public:
	virtual ~C1_5SliceJtreeInfEngine();
	static C1_5SliceJtreeInfEngine* 
        Create( const CDynamicGraphicalModel *pGrModelIn);
	//forward step at time = 0
	void ForwardFirst(const CEvidence *pEvidenceIn, int maximize = 0);
	//forward step fot time > 0
	void Forward(const CEvidence *pEvidenceIn, int maximize = 0);
	//first backward step after last forward step
	void BackwardT();
	void Backward( int maximize = 0 );
	//Backward step for fixed-lag smoothing procedure
	void BackwardFixLag();
	virtual void DefineProcedure(EProcedureTypes procedureType, 
        int lag = 0);
#ifndef SWIG
	virtual void MarginalNodes( const int *queryIn, int querySize, 
	    int time = 0,  int notExpandJPD = 0 );

	virtual void EnterEvidence(const CEvidence* const* evidenceIn, 
	    int nSlices);
#endif
	virtual const CPotential* GetQueryJPD();
	virtual const CEvidence* GetMPE();
	virtual const CPotential* GetQueryMPE();

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return C1_5SliceJtreeInfEngine::m_TypeInfo;
  }
#endif
protected:
    C1_5SliceJtreeInfEngine(const CDynamicGraphicalModel *pGraphicalModel );
   /*
   
       CPotential* ShrinkObserved(const CPotential* potToShrink, const CEvidence *evidence, 
           int isForvard, int maximize );*/
    
    void FindNonObsSubset( const intVector &subset, const CEvidence *evidence, intVector *nonObsSubset, intVector *indices);
    
#ifdef PNL_RTTI
  static const CPNLType m_TypeInfo;
#endif      
private:
    
#ifdef PAR_OMP
    CParJtreeInfEngine *m_pPriorSliceJtreeInf;
    CParJtreeInfEngine *m_p1_5SliceJtreeInf;
    
    CRing<CParJtreeInfEngine *> m_CRingJtreeInf;
    CRing<CParJtreeInfEngine *>::iterator m_JTreeInfIter;
#else
    CJtreeInfEngine *m_pPriorSliceJtreeInf;
    CJtreeInfEngine *m_p1_5SliceJtreeInf;
    
    CRing<CJtreeInfEngine *> m_CRingJtreeInf;
    CRing<CJtreeInfEngine *>::iterator m_JTreeInfIter;
#endif  
    
    
    CRing<CPotential*> m_CRingDistrOnSep;
    CRing<CPotential*>::iterator m_DistrOnSepIter;
    
    
    
    CJunctionTree *m_pJTreeTmp;
    
    
    int m_QuerrySlice;
    int m_interfaceClique;
    
    
};


PNL_END

#endif //__PNL1_5SLICEJTREEINFERENCEENGINE_HPP__
