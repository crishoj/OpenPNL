/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlEvidence.hpp                                             //
//                                                                         //
//  Purpose:   CEvidence class definition                                  //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// pgmEvidence.hpp interface for class CEvidence

#ifndef __PNLEVIDENCE_HPP__
#define __PNLEVIDENCE_HPP__

#include "pnlNodeValues.hpp"
#include "pnlStaticGraphicalModel.hpp"
#include "pnlTypeDefs.hpp"
#include "pnlNodeType.hpp"
#include "pnlModelDomain.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

#ifdef SWIG
%rename (CreateByNodeValues) CEvidence::Create(const CNodeValues *, const intVector&, const CModelDomain*, int);
%rename (CreateByModelDomain) CEvidence::Create( const CModelDomain*, const intVector&, const valueVector&);
%rename (GetValueConst) CEvidence::GetValue(int) const;
%rename (SaveForStaticModel) CEvidence::Save(const char *fname,  pConstEvidenceVector& evVec);
%rename (LoadForStaticModel) CEvidence::Load(const char *fname,  pEvidencesVector* evVecOut, const CModelDomain *pMD);
%rename (SaveForDBN) CEvidence::Save(const char *fname,  const pEvidencesVecVector& evVec);
%rename (LoadForDBN) CEvidence::Load(const char *fname,  pEvidencesVecVector* evVec, const CModelDomain *pMD);
#endif


class PNL_API CEvidence : public CNodeValues
{
public:
    static CEvidence* Create( const CModelDomain* pMD,
        const intVector& obsNodesIn, const valueVector& obsValuesIn );
    static CEvidence* Create( const CNodeValues* values,
        const intVector& obsNodesIn, const CModelDomain* pMD,
        int takeIntoObservationFlags = 0 );
    static CEvidence* Create( const CGraphicalModel* pGrModel, 
        const intVector& obsNodesIn, const valueVector& obsValuesIn);
    void ToggleNodeState( const intVector& numsOfNdsIn );
    
    static bool Save(const char *fname,  pConstEvidenceVector& evVec);
    static bool Load(const char *fname,  pEvidencesVector* evVecOut, const CModelDomain *pMD);
    static bool Save(const char *fname,  const pEvidencesVecVector& evVec);
    static bool Load(const char *fname,  pEvidencesVecVector* evVecOut, const CModelDomain *pMD);
#ifdef PNL_OBSOLETE
    static CEvidence* Create( const CModelDomain *pMD,
	int NObsNodes,
	const int *obs_nodes,
                              const valueVector& obsValuesIn );
    static CEvidence* Create( const CGraphicalModel *pGrModel,
                              int NObsNodes,
                              const int *obs_nodes,
                              const valueVector& obsValuesIn );
    static CEvidence* Create( const CNodeValues *values, 
                        int nObsNodes, const int *obsNodes,
                        const CModelDomain* pMD, 
                        int takeIntoObservationFlags = 0 );
    void ToggleNodeState( int nNodes, const int *nodeNumbers );
#endif //PNL_OBSOLETE
    
#ifndef SWIG
    const Value* GetValue( int nodeNumber ) const;
    Value* GetValue( int nodeNumber );
    const int *GetAllObsNodes() const; 
    void GetObsNodesWithValues( intVector* pObsNodesOut,
        pConstValueVector* pObsValuesOut,
        pConstNodeTypeVector* pNodeTypesOut = NULL ) const;
#endif
    /*this constructor set flags of really observation exactly from values 
                if takeIntoToggleNodes ==1  */
    
    void GetValues(int nodeNumber, valueVector* vlsOut) const;
    void GetAllObsNodes( intVector* obsNdsOut ) const;
    void GetObsNodesWithValues( intVector* pObsNodesOut,
        valueVecVector* pObsValuesOut,
        pConstNodeTypeVector* pNodeTypesOut = NULL ) const;
    
    int IsNodeObserved( int nodeNumber ) const;
    void MakeNodeObserved( int nodeNumber );
    void MakeNodeHidden( int nodeNumber );
   
    bool IsAllDiscreteNodesObs( const CStaticGraphicalModel *pGrModel) const;
    bool IsAllCountinuesNodesObs( const CStaticGraphicalModel *pGrModel) const;

    inline const CModelDomain* GetModelDomain() const;
    void Dump() const;
    ~CEvidence();

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CEvidence::m_TypeInfo;
  }
#endif
protected:

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
    CEvidence ( int nNodes, const int *obsNodes,
                const valueVector& pEvidence,
                const CNodeType *const *ObsNodeTypes, const CModelDomain* pMD );
    /*When evidence is creating all nodes from obsNodes are in m_isObsNow*/
    intVector m_obsNodes; /*Numbers of all potentially observed nodes 
                        (from GraphicalModel)*/
    const CModelDomain* m_pMD;
};

inline const CModelDomain* CEvidence::GetModelDomain() const
{
    return m_pMD;
}

PNL_END

#endif //__PNLEVIDENCE_HPP__
