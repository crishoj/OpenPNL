/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlNodeValues.hpp                                           //
//                                                                         //
//  Purpose:   CNodeValues class definition                                //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// pgmNodeValues.hpp interface for class CNodeValues

#ifndef __PNLNODEVALUES_HPP__
#define __PNLNODEVALUES_HPP__

#include "pnlObject.hpp"
#include "pnlStaticGraphicalModel.hpp"
#include "pnlTypeDefs.hpp"
#include "pnlNodeType.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

#ifdef SWIG
%rename(GetValueBySerialNumberC) CNodeValues::GetValueBySerialNumber(int SerialNumber ) const;
#endif

class PNL_API CNodeValues : public CPNLBase
{
public:
    static CNodeValues* Create( const pConstNodeTypeVector& obsNodesTypesIn,
        const valueVector& obsValsIn );
    void ToggleNodeStateBySerialNumber( const intVector& numsOfNdsIn );
#ifdef PNL_OBSOLETE
    static CNodeValues* Create(int NObsNodes, 
	const CNodeType* const* obsNodesTypes,
	const valueVector& obsValsIn );
    void ToggleNodeStateBySerialNumber( int nNodes, const int *nodeNumbers );
#endif //PNL_OBSOLETE
    
    void SetData( const valueVector& dataIn );
    // you should put in nodeNumbers numbers from the model instead of indices in m_obsNodes
    // const intVector *_pgmGetObsNodes()const;
    int GetNumberObsNodes() const;
#ifndef SWIG
    
    const int *GetObsNodesFlags() const;
    const int *GetOffset() const;
#endif
    void GetObsNodesFlags( intVector* obsNodesFlagsOut ) const;
    
    void GetRawData(valueVector* valuesOut) const;
    
#ifndef SWIG
    const CNodeType *const*GetNodeTypes() const;
#endif
    
    inline Value const* GetValueBySerialNumber( int SerialNumber ) const;
    inline Value* GetValueBySerialNumber( int SerialNumber );
    inline int IsObserved(int SerialNumber) const;
    inline void MakeNodeHiddenBySerialNum( int serialNumber );
    inline void MakeNodeObservedBySerialNum( int serialNumber );
    virtual ~CNodeValues();

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CNodeValues::m_TypeInfo;
  }
#endif
protected:
    CNodeValues ( int nNodes,const CNodeType * const*ObsNodeTypes,
	const valueVector& pValues);	
    /*When Values is creating all nodes from obsNodes are in m_isObsNow*/
    int m_numberObsNodes; /*Number of all potentially observed nodes*/
    pConstNodeTypeVector m_NodeTypes;
    //pointers to Node Types in Model Domain
    valueVector m_rawValues;
    //vector of unions with values, type of value determine by type of node
    intVector m_offset;
    //vector of offsets in m_rawValues to value of i's node*/
    intVector m_isObsNow;
    //some nodes actually observed, some - potentially, here are the flags
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
};

#ifndef SWIG

inline Value const* CNodeValues::GetValueBySerialNumber( int SerialNumber )const
{
    PNL_CHECK_RANGES( SerialNumber, 0, m_numberObsNodes );
    
    return (Value const *)(&m_rawValues[m_offset[SerialNumber]]);
}

inline Value* CNodeValues::GetValueBySerialNumber( int SerialNumber )
{
    PNL_CHECK_RANGES( SerialNumber, 0, m_numberObsNodes - 1 );
    
    return (&m_rawValues[m_offset[SerialNumber]]);
}

inline int CNodeValues::IsObserved( int SerialNumber ) const
{
    PNL_CHECK_RANGES( SerialNumber, 0, m_numberObsNodes - 1 );
    
    return m_isObsNow[SerialNumber];
}

inline void CNodeValues::MakeNodeHiddenBySerialNum( int serialNumber )
{
    PNL_CHECK_RANGES( serialNumber, 0, m_numberObsNodes - 1 );
    
    m_isObsNow[serialNumber] = 0;
}
inline void CNodeValues::MakeNodeObservedBySerialNum( int serialNumber )
{
    PNL_CHECK_RANGES( serialNumber, 0, m_numberObsNodes - 1 );
    
    m_isObsNow[serialNumber] = 1;
}

#endif

PNL_END

#endif//__PNLNODEVALUES_HPP__
