/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlModelDomain.hpp                                          //
//                                                                         //
//  Purpose:   CModelDomain class definition                               //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLMODELDOMAIN_HPP__
#define __PNLMODELDOMAIN_HPP__

#include "pnlParConfig.hpp"
//#include "pnlFactor.hpp"
#include "pnlReferenceCounter.hpp"
//#include "pnlNodeType.hpp"
#include "pnlTypeDefs.hpp"
#include "pnlNodeType.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

class CFactor;
class CGraphicalModel;

#ifdef SWIG
%rename (CreateIfAllTheSame) CModelDomain::Create(int, const CNodeType&, CGraphicalModel*);
%rename (GetTypesOfVars) CModelDomain::GetVariableTypes(intVector&, pConstNodeTypeVector*) const;
#endif

#ifdef PAR_OMP

#include <omp.h>

const int c_MaxThreadNumber = 256;

class PNL_API CModelDomain : public CReferenceCounter
{
public:
    //the class is derived from CReferenceCounter to store the pointers to Graphical Model
    static CModelDomain* Create( const nodeTypeVector& variableTypesIn,
            const intVector& variableAssociationIn, CGraphicalModel* pCreaterOfMD = NULL );
    static CModelDomain* Create(int numVariables, 
        const CNodeType& commonVariableType = CNodeType(1,2), 
        CGraphicalModel* pCreaterOfMD = NULL);

    //methods for work with factors
    int AttachFactor(const CFactor *pFactor);
    void ReleaseFactor(const CFactor *pFactor);
    bool IsAFactorOwner(const CFactor *pFactor);

    //methods for work with variable types
    const CNodeType* GetVariableType(int nodeNumber) const;
    
    void GetVariableTypes( intVector& varsIn,
                    pConstNodeTypeVector* varTypesOut ) const;
    inline const CNodeType* GetObsGauVarType() const;
    inline const CNodeType* GetObsTabVarType() const;
    //methods to direct access to node types 
    // returns number of node types of the model 
    inline int GetNumberOfVariableTypes() const;
#ifndef SWIG
    void GetVariableTypes( pConstNodeTypeVector* varTypesOut )const;
#endif
    void GetVariableTypes( nodeTypeVector* varTypesOut )const;
    inline size_t GetNumberVariables() const;
    inline int GetNumVlsForNode( int nodeNumber) const;
    void GetVariableAssociations(intVector* nodeAssociationOut) const;
#ifndef SWIG
    inline const int* GetVariableAssociations()const;
#endif
    inline int GetVariableAssociation(int variable)const;

    void ChangeNodeType(int NodeNumber, bool ToCont, intVector *pVector = NULL);
    void ChangeNodeTypeOnThread(int NodeNumber, bool ToCont);
	void ClearNodeTypeCopies();

    virtual ~CModelDomain();

    void AddRef(void* pObjectIn);

    void Release(void* pObjectIn);

    int  GetNumOfReferences();

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CModelDomain::m_TypeInfo;
  }
#endif
protected:
    //model domain is based on CNodeTypes
    CModelDomain( const nodeTypeVector& variableTypes,
            const intVector& variableAssociation,
            CGraphicalModel* pCreaterOfMD = NULL );
    //model domain with one node type for all nodes
    CModelDomain( int numVariables, const CNodeType& commonVariableType,
            CGraphicalModel* pCreaterOfMD = NULL);

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
    // heap of pointers to all factors created for this model domain
    pConstFactorVector m_factorsHeap[c_MaxThreadNumber];
    
    // numbers of heap's cells, where a pointer to a factor can be stored
    intQueue m_freeNumsInHeap[c_MaxThreadNumber];
    
    // evidence of some sort
    
    //variable types and association to variable types
    nodeTypeVector m_variableTypes;
    intVector m_variableAssociation;
    
    CNodeType m_obsGauVarType;
    CNodeType m_obsTabVarType;

    bool      m_bSelfCreated;

    omp_lock_t m_heap_lock[c_MaxThreadNumber];

    //This is a temporary object
    //We use it for correct working of the Release function
    int *m_pStubObject;

	intVector * m_pVariableAssociationOnThread [c_MaxThreadNumber];
};

#ifndef SWIG

inline const CNodeType* CModelDomain::GetObsGauVarType() const
{
    return &m_obsGauVarType;
}

inline const CNodeType* CModelDomain::GetObsTabVarType() const
{
    return &m_obsTabVarType;
}

inline int CModelDomain::GetNumberOfVariableTypes() const
{
    return m_variableTypes.size();
}

inline size_t CModelDomain::GetNumberVariables() const
{
    return m_variableAssociation.size();
}
inline const int* CModelDomain::GetVariableAssociations()const
{
	int ThreadId = omp_get_thread_num();

	if (m_pVariableAssociationOnThread[ThreadId])
	{
		return &(m_pVariableAssociationOnThread[ThreadId]->front());
	};
    return &m_variableAssociation.front();
}

inline int CModelDomain::GetVariableAssociation( int variable) const
{
    PNL_CHECK_RANGES( variable, 0, m_variableAssociation.size() );

	int ThreadId = omp_get_thread_num();

	if (m_pVariableAssociationOnThread[ThreadId])
	{
		return (*(m_pVariableAssociationOnThread[ThreadId]))[variable];
	};

    return m_variableAssociation[variable];
}

inline int CModelDomain::GetNumVlsForNode( int nodeNumber ) const
{
    const CNodeType* nt = GetVariableType(nodeNumber);
    if( nt->IsDiscrete() )
    {
        return 1;
    }
    return nt->GetNodeSize();
    
}

#endif // SWIG

#else  // !defined(PAR_OMP)

class PNL_API CModelDomain : public CReferenceCounter
{
public:
    //the class is derived from CReferenceCounter to store the pointers to Graphical Model
    static CModelDomain* Create( const nodeTypeVector& variableTypesIn,
            const intVector& variableAssociationIn, CGraphicalModel* pCreaterOfMD = NULL );
    static CModelDomain* Create(int numVariables, 
        const CNodeType& commonVariableType = CNodeType(1,2), 
        CGraphicalModel* pCreaterOfMD = NULL);

    //methods for work with factors
    int AttachFactor(const CFactor *pFactor);
    void ReleaseFactor(const CFactor *pFactor);
    bool IsAFactorOwner(const CFactor *pFactor);

    //methods for work with variable types
    const CNodeType* GetVariableType(int nodeNumber) const;
    
    void GetVariableTypes( intVector& varsIn,
                    pConstNodeTypeVector* varTypesOut ) const;
    inline const CNodeType* GetObsGauVarType() const;
    inline const CNodeType* GetObsTabVarType() const;
    //methods to direct access to node types 
    // returns number of node types of the model 
    inline int GetNumberOfVariableTypes() const;
#ifndef SWIG
    void GetVariableTypes( pConstNodeTypeVector* varTypesOut )const;
#endif
    void GetVariableTypes( nodeTypeVector* varTypesOut )const;
    inline size_t GetNumberVariables() const;
    inline int GetNumVlsForNode( int nodeNumber) const;
    void GetVariableAssociations(intVector* nodeAssociationOut) const;
#ifndef SWIG
    inline const int* GetVariableAssociations()const;
#endif
    inline int GetVariableAssociation(int variable)const;

    void ChangeNodeType(int NodeNumber, bool ToCont);

    virtual ~CModelDomain();

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return m_TypeInfo;
  }
#endif
protected:
    //model domain is based on CNodeTypes
    CModelDomain( const nodeTypeVector& variableTypes,
            const intVector& variableAssociation,
            CGraphicalModel* pCreaterOfMD = NULL );
    //model domain with one node type for all nodes
    CModelDomain( int numVariables, const CNodeType& commonVariableType,
            CGraphicalModel* pCreaterOfMD = NULL);

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
    // heap of pointers to all factors created for this model domain
    pConstFactorVector m_factorsHeap;
    
    // numbers of heap's cells, where a pointer to a factor can be stored
    intQueue m_freeNumsInHeap;
    
    // evidence of some sort
    
    //variable types and association to variable types
    nodeTypeVector m_variableTypes;
    intVector m_variableAssociation;
    
    CNodeType m_obsGauVarType;
    CNodeType m_obsTabVarType;

    bool      m_bSelfCreated;

};

#ifndef SWIG

inline const CNodeType* CModelDomain::GetObsGauVarType() const
{
    return &m_obsGauVarType;
}

inline const CNodeType* CModelDomain::GetObsTabVarType() const
{
    return &m_obsTabVarType;
}

inline int CModelDomain::GetNumberOfVariableTypes() const
{
    return m_variableTypes.size();
}

inline size_t CModelDomain::GetNumberVariables() const
{
    return m_variableAssociation.size();
}
inline const int* CModelDomain::GetVariableAssociations()const
{
    return &m_variableAssociation.front();
}

inline int CModelDomain::GetVariableAssociation( int variable) const
{
    PNL_CHECK_RANGES( variable, 0, m_variableAssociation.size() );

    return m_variableAssociation[variable];
}

inline int CModelDomain::GetNumVlsForNode( int nodeNumber ) const
{
    const CNodeType* nt = GetVariableType(nodeNumber);
    if( nt->IsDiscrete() )
    {
        return 1;
    }
    return nt->GetNodeSize();
    
}

#endif // SWIG

#endif // PAR_OMP

PNL_END

#endif // __PNLMODELDOMAIN_HPP__
