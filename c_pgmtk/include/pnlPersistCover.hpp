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

PNL_BEGIN

class CCoverGen: public CPNLBase
{
public:
    CCoverGen() {}
    virtual CPNLBase *GetBaseObj() const = 0;
    virtual int nObject() const { return 1; }
    virtual ~CCoverGen() { }
};

template<typename Type> class CCover: public CCoverGen
{
public:
    CCover(Type *ptr): m_Pointer(ptr) { }
    virtual CPNLBase *GetBaseObj() const
    { return reinterpret_cast<CPNLBase*>(GetPointer()); }
    virtual Type *GetPointer() const { return m_Pointer; }

protected:
    Type *m_Pointer;
};

template<typename Type> class CCoverDel: public CCover<Type>
{
public:
    CCoverDel(Type *ptr): CCover<Type>(ptr) { }
    virtual ~CCoverDel()
    {
	delete m_Pointer;
    }
};

class CPersistNodeTypeVector: public CPersistence
{
public:
    virtual const char *Signature() { return "NodeTypeVector"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
};

class CPersistPNodeTypeVector: public CPersistence
{
public:
    virtual const char *Signature() { return "PNodeTypeVector"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
};

template<typename Type> class CPersistNumericVector: public CPersistence
{
public:
    CPersistNumericVector(const std::string &typeName)
    {
	m_Name = typeName;
	m_Name.append("Vector");
    }
    virtual const char *Signature() { return m_Name.c_str(); }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);

private:
    std::string m_Name;
};

template<typename Type> void
CPersistNumericVector<Type>::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CCover<pnlVector<Type> > *ptr = dynamic_cast<CCover<pnlVector<Type> >*>(pObj);
    std::stringstream buf;

    PNL_CHECK_IS_NULL_POINTER(ptr);
    pnlVector<Type> &vec = *ptr->GetPointer();

    buf << '[';
    for(int i = 0; i < vec.size(); ++i)
    {
        buf << vec[i] << ((i != int(vec.size()) - 1) ? ' ':']');
    }

    pContext->AddText(buf.str().c_str());
}

template<typename Type> CPNLBase *
CPersistNumericVector<Type>::Load(CContextLoad *pContext)
{
    std::string str;
    
    pContext->GetText(str);
    std::istringstream buf(str);
    Type j;
    char ch;
    pnlVector<Type> *pVec = new pnlVector<Type>;

    buf >> ch;
    pVec->reserve((str.length() - 2) >> 2);
    ASSERT(ch == '[');
    for(int i = 0; buf.good() && !buf.eof(); ++i)
    {
	buf >> ch;
	if(!isdigit(ch))
	{
	    break;
	}
	buf.unget();
        buf >> j;
	pVec->push_back(j);
    }
    ASSERT(ch == ']');

    CCover<pnlVector<Type> > *pCov =
	new CCoverDel<pnlVector<Type> >(pVec);
    pContext->AutoDelete(pCov);

    return pCov;
}

template<typename Type> class CPersistNumericVecVector: public CPersistence
{
public:
    CPersistNumericVecVector(const std::string &typeName)
    {
	m_Name = typeName;
	m_Name.append("VecVector");
    }
    virtual const char *Signature() { return m_Name.c_str(); }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);

private:
    std::string m_Name;
};

template<typename Type> void
CPersistNumericVecVector<Type>::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CCover<pnlVector<pnlVector<Type> > > *ptr =
	dynamic_cast<CCover<pnlVector<pnlVector<Type> > >*>(pObj);
    std::stringstream buf;

    PNL_CHECK_IS_NULL_POINTER(ptr);
    pnlVector<pnlVector<Type> > &vec = *ptr->GetPointer();

    char buf2[20];
    
    sprintf(buf2, "%i", vec.size());
    
    pContext->AddAttribute("nVector", buf2);
    
    for(int j = 0; j < vec.size(); ++j)
    {
	buf << '[';
	for(int i = 0; i < vec[j].size(); ++i)
	{
	    buf << vec[j][i] << ((i != int(vec[j].size()) - 1) ? ' ':']');
	}
	buf << '\n';
    }

    pContext->AddText(buf.str().c_str());
}

template<typename Type> CPNLBase *
CPersistNumericVecVector<Type>::Load(CContextLoad *pContext)
{
    std::string str;
    std::string nVector;
    
    pContext->GetText(str);
    std::istringstream buf(str);
    int j, k, nVect;
    char ch;
    pnlVector<pnlVector<Type> > *pVec = new pnlVector<pnlVector<Type> >;

    pContext->GetAttribute(nVector, std::string("nVector"));
    nVect = atoi(nVector.c_str());
    buf >> ch;
    pVec->resize(nVect);
    ASSERT(ch == '[');
    for(k = 0; k < nVect; ++k)
    {
	(*pVec)[k].reserve(16);
	for(int i = 0; buf.good() && !buf.eof(); ++i)
	{
	    buf >> ch;
	    if(!isdigit(ch))
	    {
		break;
	    }
	    buf.unget();
	    buf >> j;
	    (*pVec)[k].push_back(j);
	}
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
