//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnl2TPFInferenceEngine.hpp                                  //
//                                                                         //
//  Purpose:   C2TPFInfEngine class member functions definition            //
//                                                                         //
//             PF means Particle filtering                                 //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNL2TPFINFERENCEENGINE_HPP__
#define __PNL2TPFINFERENCEENGINE_HPP__


#include "pnl2TBNInferenceEngine.hpp"
#include "pnlDBN.hpp"
#include "pnlBNet.hpp"
#include "pnlPotential.hpp"
#include "pnlLog.hpp"
#include "pnlLogDriver.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

class PNL_API C2TPFInfEngine : public C2TBNInfEngine
{
public:
	virtual ~C2TPFInfEngine();
	static C2TPFInfEngine * Create (const CDynamicGraphicalModel *pGrModelIn, int nParticleCount = 400); // create a 2TPFInfEngine

	void SetParemeter(  int nParticleCount = 400, int nLowThreshold = 30);   // set parameters of 2TPFInfEngine
	void InitSlice0Particles(const CEvidence* pSlice0Evidence = NULL);       // init particles of slice0

#ifndef SWIG	
	pEvidencesVector* GetCurSamples();      // get current particles
	floatVector*      GetParticleWeights(); // get particle weights
	

	virtual void  EnterEvidence(const CEvidence* const* evidencesIn, int nSlices = -1); // enter evidence sequence (slice 0 ~ slice n)
#endif
	virtual void  EnterEvidence(const CEvidence* pEvidenceIn);               // enter current evidence and update partice states and weights
	virtual void  EnterEvidenceProbability( floatVector *pEvidenceProbIn );  // input current particle weights by userself
	virtual void  LWSampling(const CEvidence* pEvidenceIn = NULL);           // enter current evidence and update partice states
	virtual void  Estimate(CEvidence *pEstimate);                            // estimate current real states by particles just ike GetMPE()
	float         GetNeff();                                                 // get the number of current effective particles
	
#ifndef SWIG
	virtual void  MarginalNodes(const int *queryIn, int querySize, int time=0, int notExpandJPD=0 ); // Marginalize the potential of query nodes over all current particles
#endif
	virtual const CPotential* GetQueryJPD();                                 // return query potential
	const CEvidence* GetMPE();                                               // current most probable explaination

	/////////////////////////////////////////////////////////////////////////
	virtual void DefineProcedure(EProcedureTypes procedureType, int lag = 0);// default Filtering
  	void ForwardFirst(const CEvidence *pEvidenceIn, int maximize = 0);		 // intput the first slice 
	//forward step fot time > 0
	void Forward(const CEvidence *pEvidenceIn, int maximize = 0);            // intput the following slice 
	//first backward step after last forward step
	void BackwardT();                       // not use
	void Backward( int maximize = 0 );      // not use
	//Backward step for fixed-lag smoothing procedure
	void BackwardFixLag();                  // not use
	
#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return C2TPFInfEngine::m_TypeInfo;
  }
#endif
protected:	
	C2TPFInfEngine(const CDynamicGraphicalModel *pGrModelIn, int nParticleCount = 400); // create a 2TPFInfEngine
	CEvidence* Sub2Find(int *index = NULL);                                             // select a particle from proposal distribution Q          
	void       NormalizeWeight();                                                       // normalize particle weights
	void       GenerateSample(CEvidence* evidence, const CEvidence *evidIn = NULL);     // generate a sample according to evidence
		
	const CDynamicGraphicalModel *m_pGrModel; //**
   
	CEvidence			   *m_pEvidenceMPE;      // most probable explaination of m_pQueryJPD in current slice
	CPotential			   *m_pQueryJPD;         // query potential

	pEvidencesVector        m_currentEvVec;      // sampled particles
	pEvidencesVector        m_EvVecBackup;       // backup sampled particles for DBNs

	int						m_particleCount;     // particle count
	floatVector             m_particleWeight;    // particle weights
	floatVector             m_CumulativeWeight;  // accumulated particles' weights
	floatVector             m_particleWeightBack;// backup of accumulated particles' weights

	int                     m_Slice0Nodes;       // node  count of slice0
	intVector               m_Slice0NodeSize;    // each  node size of slice0
	int                     m_Slice0ValuesSize;  // total node size of slice0
	int                     m_Slices;            // the slice number

	bool					m_bInitalized;       // initialize particles or not
	bool					m_bNormalized;       // normailzed particles' weights or not
	int                     m_LowThreshold;      // number of the smallest effective particles

#ifdef PNL_RTTI
  static const CPNLType m_TypeInfo;
#endif     
};


PNL_END

#endif //__PNL2TPFINFERENCEENGINE_HPP__
