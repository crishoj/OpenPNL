/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlNodeType.hpp                                             //
//                                                                         //
//  Purpose:   CNodeType class definition                                  //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLNODETYPE_HPP__
#define __PNLNODETYPE_HPP__

#include "pnlObject.hpp"
#include "pnlException.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

typedef PNL_API enum
{   nsChance = 0,
    nsDecision,
    nsValue
} EIDNodeState;

/* a class to represent node types structure and operations */
class PNL_API CNodeType : public CPNLBase
{
public:
    inline bool IsDiscrete() const;

    inline int GetNodeSize() const;

    inline EIDNodeState GetNodeState() const;

    //  inline void SetType( bool IsDiscrete, int nodeSize );
    inline void SetType(bool IsDiscrete, int nodeSize,
	EIDNodeState nodeState = nsChance);

    inline bool operator==(const CNodeType &ntIn) const;

    inline bool operator!=(const CNodeType &ntIn) const;

#ifdef PNL_VC7
    // VisualStudio 7 has non-standard STL realization.
    // This STL (vector) requires operator<
    inline bool operator<(const CNodeType &nt) const;
#endif
    explicit CNodeType(bool IsDiscrete = 0, int nodeSize = -1,
	EIDNodeState nodeState = nsChance)
#ifndef SWIG
	: m_IsDiscrete( IsDiscrete ), m_nodeSize(nodeSize)
    {
	m_nodeState = nodeState;
    }
#else
    ;
#endif

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CNodeType::m_TypeInfo;
  }
#endif
protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 

private:
    bool m_IsDiscrete;
    int m_nodeSize;
    EIDNodeState m_nodeState;
};

#ifndef SWIG

inline bool CNodeType::IsDiscrete() const
{
    return m_IsDiscrete;
}

inline int CNodeType::GetNodeSize() const
{
    return m_nodeSize;
}

inline EIDNodeState CNodeType::GetNodeState() const
{
    return m_nodeState;
}

inline void CNodeType::SetType(bool IsDiscrete, int nodeSize,
			       EIDNodeState nodeState)
{
    /* bad-args check */
    PNL_CHECK_LEFT_BORDER( nodeSize, 0 );
    /* bad-args check end */

    m_IsDiscrete = IsDiscrete;
    m_nodeSize = nodeSize;
    m_nodeState = nodeState;
}

inline bool CNodeType::operator==(const CNodeType &nt) const
{
    return (m_IsDiscrete == nt.m_IsDiscrete)
	&& (m_nodeSize == nt.m_nodeSize)
	&& (m_nodeState == nt.m_nodeState);
}

inline bool CNodeType::operator!=(const CNodeType &nt) const
{
    return !operator==(nt);
}

#ifdef PNL_VC7
inline bool CNodeType::operator<(const CNodeType &nt) const
{
    return m_nodeSize < nt.m_nodeSize;
}
#endif

#endif

PNL_END

#endif // __PNLNODETYPE_HPP__
