/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlFactors.hpp                                              //
//                                                                         //
//  Purpose:   CFactor class definition                                    //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLFACTORS_HPP__
#define __PNLFACTORS_HPP__

#include "pnlObject.hpp"
#include "pnlFactor.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

/* a class to represent factors structure and operations */
class PNL_API CFactors : public CPNLBase
{
public:

	static CFactors* Create(int numberOfFactors);

	static CFactors* Copy(const CFactors *pFactorsIn);

	/* returns number of factors currently stored in CFactors object */
	inline int GetNumberOfFactors() const;

	/* returns a pointer to a factor indexed factorNumber */
	inline CFactor* GetFactor(int factorNumber) const;

	/* adds a pFactor to the factors vector */
	inline int AddFactor(CFactor *pFactorIn);

	/* shrinks the stored Factors taking the input Evidence into account */
	void ShrinkObsNdsForAllFactors(const CEvidence *pEvidenceIn);

	~CFactors();
	
#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CFactors::m_TypeInfo;
  }
#endif
protected:

	/* constructors */
	CFactors(int numberOfFactors);
	CFactors(const CFactors& rFactors);

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:

	int m_capacity;
    pFactorVector m_facts;
};

#ifndef SWIG

inline int CFactors::GetNumberOfFactors() const
{
	return m_facts.size();
}

inline CFactor* CFactors::GetFactor(int factorNumber) const
{
	PNL_CHECK_RANGES(factorNumber, 0, m_facts.size() - 1);
	return m_facts[factorNumber];
}

inline int CFactors::AddFactor(CFactor *pFactor)
{
	/* bad-args check */
	PNL_CHECK_IS_NULL_POINTER(pFactor);
	/* bad-args check end */
	
	/* check if adding factor to the factors vector is possible */
	if( int(m_facts.size()) == m_capacity )
	{
		PNL_THROW( CInvalidOperation, " cannot add more factors " );
	}

	m_facts.push_back(pFactor);
    if( pFactor->IsOwnedByModelDomain() )
    {
        pFactor->ChangeOwnerToGraphicalModel();
    }

    return m_facts.size() - 1;
}

#endif

PNL_END

#endif //__PNLFACTORS_HPP__
