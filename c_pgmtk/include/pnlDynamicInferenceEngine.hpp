/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlDynamicInferenceEngine.hpp                               //
//                                                                         //
//  Purpose:   Base class definitiona for all inference                    //
//             algorithms for dynamic graphical models                     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLDYNAMICINFERENCEENGINE_HPP__
#define __PNLDYNAMICINFERENCEENGINE_HPP__

#include "pnlObject.hpp"
#include "pnlDynamicGraphicalModel.hpp"
#include "pnlInferenceEngine.hpp"
#include "pnlEvidence.hpp"
#include "pnlRing.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

typedef PNL_API enum
{
	ptFiltering,
	ptSmoothing,
	ptFixLagSmoothing,
	ptViterbi,
	ptPrediction
			
} EProcedureTypes;

class PNL_API CDynamicInfEngine : public CPNLBase
{
public:
    virtual ~CDynamicInfEngine();
    virtual void DefineProcedure(EProcedureTypes prType ,int lag ) = 0;
#ifdef PNL_OBSOLETE
    virtual void EnterEvidence( const CEvidence* const* evidencesIn, int nSlices) = 0;
    virtual void MarginalNodes( const int *queryIn, int querySize, int time = 0, int notExpandJPD = 0  ) = 0;
#endif
    virtual void EnterEvidence( const pConstEvidenceVector& evidencesIn );
	virtual void MarginalNodes( const intVector& queryIn, int time = 0, int notExpandJPD = 0 );
    const CDynamicGraphicalModel * GetDynamicModel() const;
    EProcedureTypes GetProcedureType() const;
   

	virtual const CPotential* GetQueryJPD() = 0;
    virtual const CEvidence* GetMPE() = 0;
    virtual void Filtering(int time)=0;
    virtual void Smoothing()=0;
    virtual void FixLagSmoothing(int time)=0;
    virtual void FindMPE()=0;

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CDynamicInfEngine::m_TypeInfo;
    }
#endif
protected:
    inline int GetLag() const;
    inline int GetTime() const;
    inline void SetEvidenceMPE(CEvidence *pEv);
    inline void SetQueryPot(CPotential* pPot);
    inline  CEvidence* GetEvidenceMPE() const;
    inline  CPotential* GetQueryPot() const;

    CDynamicInfEngine( 	const CDynamicGraphicalModel *pGrModel );

    const CDynamicGraphicalModel *GrModel() const { return m_pGrModel; }
    void DropEvidences(); // destroy all evidences and clear container
    CEvidence *Evidence(int i) { return m_CRingpEv[i]; }
    int nEvidence() { return m_CRingpEv.size(); }
    typedef CRing<CEvidence*>::iterator EvidenceIterator;
    EvidenceIterator EvidenceBegin() { return m_CRingpEv.begin(); }
    //EvidenceIterator EvidenceEnd() const { return .end(); }
    void SetEvidence(int iEvidence, CEvidence *pEvidence)
    {
	if(iEvidence < nEvidence()) 
	{
	    delete Evidence(iEvidence);
	    m_CRingpEv[iEvidence] = pEvidence;
	}
	else if(iEvidence == nEvidence())
	{
	    m_CRingpEv.push_back(pEvidence);
	}
	else
	{
	    throw(CBadArg(__FILE__, __LINE__, "Ring: Out of range"));
	}
    }

    EProcedureTypes m_ProcedureType;
    int m_Lag;
    int m_CurrentTime;
    CRing<CEvidence *> m_CRingpEv;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
    const CDynamicGraphicalModel *m_pGrModel;

    CEvidence *m_pEvidenceMPE;
    CPotential *m_pQueryPot;
};

#ifndef SWIG

inline EProcedureTypes CDynamicInfEngine::GetProcedureType() const
{
    return m_ProcedureType;
}
inline void CDynamicInfEngine::SetEvidenceMPE(CEvidence *pEv)
{
    m_pEvidenceMPE = pEv;
}
inline void CDynamicInfEngine::SetQueryPot(CPotential* pPot)
{
    m_pQueryPot = pPot;
}

inline  CEvidence* CDynamicInfEngine::GetEvidenceMPE() const
{
    return m_pEvidenceMPE;
}

inline  CPotential* CDynamicInfEngine::GetQueryPot() const
{
    return m_pQueryPot;
}

inline int CDynamicInfEngine::GetLag() const
{
    return m_Lag;
}

inline int CDynamicInfEngine::GetTime() const
{
    return m_CurrentTime;
}

inline const CDynamicGraphicalModel *CDynamicInfEngine::
GetDynamicModel() const
{
    return m_pGrModel;
}

#endif

PNL_END

#endif // __PNLDYNAMICINFERENCEENGINE_HPP__
