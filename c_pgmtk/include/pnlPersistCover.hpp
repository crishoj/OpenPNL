/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPersistCovers.hpp                                        //
//                                                                         //
//  Purpose:   Covers (wrappers) for different types                       //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __PNLPERSISTCOVERS_HPP__
#define __PNLPERSISTCOVERS_HPP__

#include <sstream>
#include "pnlPersistence.hpp"
#include "pnlContextPersistence.hpp"
#include "pnlContextLoad.hpp"
#include "pnlContextSave.hpp"
#include "pnlPersistArray.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

class CCoverGen: public CPNLBase
{
protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif

public:
    CCoverGen() {}
    virtual CPNLBase *GetBaseObj() const = 0;
    virtual int nObject() const { return 1; }
    virtual ~CCoverGen() { }

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }

  static const CPNLType &GetStaticTypeInfo()
  {
    return CCoverGen::m_TypeInfo;
  }
#endif
};

template<typename Type> class CCover: public CCoverGen
{
public:
    CCover(Type *ptr): m_Pointer(ptr) { }
    virtual CPNLBase *GetBaseObj() const
    { return reinterpret_cast<CPNLBase*>(GetPointer()); }
    virtual Type *GetPointer() const { return m_Pointer; }

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CCover<int>::GetStaticTypeInfo();
    }
#endif

protected:
    Type *m_Pointer;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
};

#ifdef PNL_RTTI
template< typename T > 
const CPNLType CCover< T >::m_TypeInfo = CPNLType("CCover", &(CCoverGen::m_TypeInfo));
#endif

template<typename Type> class CCoverDel: public CCover<Type>
{
public:
    CCoverDel(Type *ptr): CCover<Type>(ptr) { }
    virtual ~CCoverDel()
    {
    	delete m_Pointer;
    }

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CCoverDel<int>::GetStaticTypeInfo();
    }
#endif

protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif

#if GCC_VERSION >= 30400
    using CCover<Type>::m_Pointer;
#endif

};

#ifdef PNL_RTTI
template<typename T > 
const CPNLType CCoverDel< T >::m_TypeInfo = CPNLType("CCoverDel", &(CCover< T >::m_TypeInfo));
#endif

class CPersistNodeTypeVector: public CPersistence
{
public:
    virtual const char *Signature() { return "NodeTypeVector"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

class CPersistPNodeTypeVector: public CPersistence
{
public:
    virtual const char *Signature() { return "PNodeTypeVector"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

template<typename Type> class CPersistNumericVector: public CPersistence
{
public:
    CPersistNumericVector(const pnlString &typeName)
    {
	m_Name = typeName;
	m_Name.append("Vector");
    }
    virtual const char *Signature() { return m_Name.c_str(); }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const
    {
	return dynamic_cast<CCover<pnlVector<Type> >*>(pObj) != 0;
    }

private:
    pnlString m_Name;
};

template<typename Type> void
CPersistNumericVector<Type>::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CCover<pnlVector<Type> > *ptr = dynamic_cast<CCover<pnlVector<Type> >*>(pObj);

    PNL_CHECK_IS_NULL_POINTER(ptr);
    
    pnlVector<Type> &vec = *ptr->GetPointer();

    SaveArray(*pContext, &vec.front(), vec.size());
}

template<typename Type> CPNLBase *
CPersistNumericVector<Type>::Load(CContextLoad *pContext)
{
    pnlVector<Type> *pVec = LoadArray<Type>(*pContext);
    CCover<pnlVector<Type> > *pCov =
	new CCoverDel<pnlVector<Type> >(pVec);
    pContext->AutoDelete(pCov);

    return pCov;
}

template<typename Type> class CPersistNumericVecVector: public CPersistence
{
public:
    CPersistNumericVecVector(const pnlString &typeName)
    {
	m_Name = typeName;
	m_Name.append("VecVector");
    }
    virtual const char *Signature() { return m_Name.c_str(); }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const
    {
	return dynamic_cast<CCover<pnlVector<pnlVector<Type> > >*>(pObj) != 0;
    }

private:
    pnlString m_Name;
};

template<typename Type> void
CPersistNumericVecVector<Type>::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CCover<pnlVector<pnlVector<Type> > > *ptr =
	dynamic_cast<CCover<pnlVector<pnlVector<Type> > >*>(pObj);
    pnlString buf, buf2;

    PNL_CHECK_IS_NULL_POINTER(ptr);
    pnlVector<pnlVector<Type> > &vec = *ptr->GetPointer();

    buf2 << int(vec.size());
    pContext->AddAttribute("nVector", buf2.data());    
    
    for(int j = 0; j < vec.size(); ++j)
    {
	SaveArray(buf, &vec[j].front(), vec[j].size());
	buf << '\n';
    }

    pContext->AddText(buf.c_str());
}

template<typename Type> CPNLBase *
CPersistNumericVecVector<Type>::Load(CContextLoad *pContext)
{
    pnlString str;
    pnlString nVector;
    
    pContext->GetText(str);
    std::istringstream buf(str.c_str());
    int j, k, nVect;
    char ch;
    pnlVector<pnlVector<Type> > *pVec = new pnlVector<pnlVector<Type> >;
    pnlVector<Type> arrayFromString;

    pContext->GetAttribute(nVector, pnlString("nVector"));
    nVect = atoi(nVector.c_str());
    pVec->resize(nVect);
    arrayFromString.reserve(64);
    for(k = 0; k < nVect; ++k)
    {
	buf >> ch;
	ASSERT(ch == '[');
	arrayFromString.resize(0);
	for(int i = 0; buf.good() && !buf.eof(); ++i)
	{
	    buf >> ch;
	    if(!isdigit(ch))
	    {
		break;
	    }
	    buf.unget();
	    buf >> j;
	    arrayFromString.push_back(j);
	}
	j = arrayFromString.size();
	(*pVec)[k].resize(j);
	memcpy(&(*pVec)[k][0], &arrayFromString[0], sizeof(Type)*j);
	if(ch != ']')
	{// error?
	    break;
	}
    }

    CCover<pnlVector<pnlVector<Type> > > *pCov =
	new CCoverDel<pnlVector<pnlVector<Type> > >(pVec);
    pContext->AutoDelete(pCov);

    return pCov;
}

PNL_END

#endif // include guard
