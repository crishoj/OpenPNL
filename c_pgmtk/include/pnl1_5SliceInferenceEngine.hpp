/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnl1_5SliceInferenceEngine.hpp                              //
//                                                                         //
//  Purpose:   Class definition for the 1.5 Slice structure                //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNL1_5SLICEINFERENCEENGINE_HPP__
#define __PNL1_5SLICEINFERENCEENGINE_HPP__


#include "pnl2TBNInferenceEngine.hpp"
#include "pnlBNet.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

class PNL_API C1_5SliceInfEngine : public C2TBNInfEngine
{
public:
	virtual ~C1_5SliceInfEngine();
	
	//forward step at time = 0
	void ForwardFirst(const CEvidence *pEvidenceIn, int maximize = 0) = 0;
	//forward step fot time > 0
	void Forward(const CEvidence *pEvidenceIn, int maximize = 0) = 0;
	//first backward step after last forward step
	void BackwardT() = 0;
	void Backward( int maximize = 0 ) = 0;
	//Backward step for fixed-lag smoothing procedure
	void BackwardFixLag() = 0;
	virtual void DefineProcedure(EProcedureTypes procedureType, 
        int lag = 0) = 0;
#ifndef SWIG
	virtual void MarginalNodes( const int *queryIn, int querySize, 
        int time = 0,  int notExpandJPD = 0 ) = 0;

	virtual void EnterEvidence(const CEvidence* const* evidenceIn, 
        int nSlices) = 0;
#endif
	virtual const CPotential* GetQueryJPD() = 0;
    virtual const CEvidence* GetMPE() = 0;
    
#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return C1_5SliceInfEngine::m_TypeInfo;
    }
#endif
protected:
    
    
    C1_5SliceInfEngine( const CDynamicGraphicalModel *pGraphicalModel);
    CGraph *Create1_5SliceGraph();
    CBNet *Create1_5SliceBNet();
    
    inline const CBNet *GetPriorSliceBNet() const;
    inline const CBNet *Get1_5SliceBNet() const;
    inline void FindFinalQuery( intVector &queryIn, int slice, intVector *finalQuery) const;
    inline const intVector *  GetIntNdsPriorSlice() const;
    inline const intVector *  GetIntNdsISlice() const ;
    inline const intVector *  GetRootNdsISlice() const;
    inline int GetNumIntNds() const;
    inline void GetQueryNodes( intVector * queryNds ) const;
    inline void SetQueryNodes(int nnodes, const int * nds);

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif     
private:
    intVector m_VectorIntNodesPriorSlice;
    intVector m_VectorIntNodesISlice;
    intVector m_VectorRootNodesISlice;
    int m_nIntNodes;

    intVector m_queryNodes;
    
    void Initialization(  const CDynamicGraphicalModel *pDBN );
    CBNet *m_pPriorSliceBNet, *m_p1_5SliceBNet;
};

inline void  C1_5SliceInfEngine::GetQueryNodes( intVector * queryNds ) const
{
    queryNds->assign( m_queryNodes.begin(), m_queryNodes.end() );
}

inline void C1_5SliceInfEngine::SetQueryNodes(int nnodes, const int * nds)
{
    m_queryNodes.assign( nds, nds + nnodes );
}

inline const intVector *  C1_5SliceInfEngine::GetIntNdsPriorSlice() const
{
    return &m_VectorIntNodesPriorSlice;
}
inline const intVector *  C1_5SliceInfEngine::GetIntNdsISlice() const 
{
    return &m_VectorIntNodesISlice;
}
inline const intVector *  C1_5SliceInfEngine::GetRootNdsISlice() const
{
    return &m_VectorRootNodesISlice;
}
inline int C1_5SliceInfEngine::GetNumIntNds() const
{
    return m_nIntNodes;
}

inline const CBNet* C1_5SliceInfEngine::GetPriorSliceBNet() const
{
    return m_pPriorSliceBNet;
}

inline const CBNet* C1_5SliceInfEngine::Get1_5SliceBNet() const
{
    return m_p1_5SliceBNet;
}

inline void C1_5SliceInfEngine::
FindFinalQuery( intVector &queryIn, int slice, intVector *finalQuery) const
{
  
    int nnodesPerSlice = GrModel()->GetNumberOfNodes();
    int sz = queryIn.size();
    finalQuery->resize(sz);
    int i;
    if( !slice )
    {
	for( i = 0; i < sz; i++ )
	{
	    PNL_CHECK_RIGHT_BORDER( queryIn[i], nnodesPerSlice - 1 );
	    (*finalQuery)[i] = queryIn[i];
	}
    }
    else
    {
	intVector::const_iterator it;
	for( i = 0; i < sz; i++ )
	{
	    if( queryIn[i] < nnodesPerSlice )
	    {
		it = std::find(GetIntNdsPriorSlice()->begin(), GetIntNdsPriorSlice()->end(), queryIn[i]);
		if( it == GetIntNdsPriorSlice()->end() )
		{
		    PNL_THROW(COutOfRange," node number");
		}
		else
		{
		    (*finalQuery)[i] = it - GetIntNdsPriorSlice()->begin();
		}
	    }
	    else
	    {
		PNL_CHECK_RIGHT_BORDER( queryIn[i], 2*nnodesPerSlice - 1 );
		(*finalQuery)[i] = queryIn[i] - nnodesPerSlice + GetNumIntNds();

	    }
	}

    }
}


PNL_END

#endif //__PNL1_5SLICEINFERENCEENGINE_HPP__
