/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlLWSamplingInferenceEngine.hpp                            //
//                                                                         //
//  Purpose:   CLWSamplingInfEngine class member functions definition      //
//                                                                         //
//             LW means Likelyhood weighting                               //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __PNLLWSAMPLINGINFERENCEENGINE_HPP__
#define __PNLLWSAMPLINGINFERENCEENGINE_HPP__

#include "pnlInferenceEngine.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

class PNL_API CLWSamplingInfEngine : public CInfEngine  
{
public:
	static CLWSamplingInfEngine* Create( const CStaticGraphicalModel *pGraphicalModelIn, int particleCount= 400 ); // create a LWSamplingInfEngine
	void SetParemeter( int particelCount = 400);  // set parameters of 2TPFInfEngine
	
	// main public member-functions, which carry out the engine work
#ifndef SWIG
	pEvidencesVector* GetCurSamples();            // get current particles
	floatVector*      GetParticleWeights();       // get particle weights
#endif
	
	virtual void      EnterEvidence( const CEvidence *pEvidenceIn, int maximize = 0, int sumOnMixtureNode = 1 );// enter evidence and generate partice states and weights
	void              EnterEvidenceProbability(floatVecVector *pEvidenceProbIn ); // input particle weights by userself
	void              LWSampling(const CEvidence* pEvidenceIn = NULL);            // enter evidence and generate partice states without calculating particle weights
	void              Estimate(CEvidence * pEstimate);                            // estimate real states by particles. It is same as GetMPE()
	float             GetNeff( );                 // get the number of effective particles
	
#ifndef SWIG
	virtual void      MarginalNodes( const int *query, int querySize, int notExpandJPD = 0 );//Marginalize the potential of query nodes over all current particles
#endif
	const CPotential* GetQueryJPD() const;        // return query potential
    virtual const CEvidence* GetMPE() const;      // get most probable explaination
    
     // destructor
    virtual ~CLWSamplingInfEngine();
   
#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CLWSamplingInfEngine::m_TypeInfo;
  }
#endif
protected:
    CLWSamplingInfEngine( const CStaticGraphicalModel *pGraphicalModel, int particleCount= 300  );
	void NormalizeWeight();
      
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
	int						m_particleCount;      // particle(sample) count
	floatVector             m_particleWeight;     // particle weights
    bool                    m_bNormalized;        // flag of normalize partice weights
	pEvidencesVector        m_currentEvVec;       // current sampled particles
};

PNL_END

#endif //__PNLLWSAMPLINGINFERENCEENGINE_HPP__
