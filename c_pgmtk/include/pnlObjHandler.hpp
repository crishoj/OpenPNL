/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlContext.hpp                                              //
//                                                                         //
//  Purpose:   Base class for traversal classes                            //
//             (for example saving/loading classes)                        //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __PNLOBJHANDLER_HPP__
#define __PNLOBJHANDLER_HPP__

PNL_BEGIN

// FORWARDS
class CPNLBase;
class CContext;
class CPersistence;
class CPersistenceZoo;
class pnlString;

class PNL_API CObjHandler
{
public:
    CObjHandler() {};
    virtual ~CObjHandler() {}
    virtual void CallByName(pnlString &name, CPNLBase *pObj, CContext *pContext) = 0;
    virtual void CallByNameForDup(pnlString &name, CPNLBase *pObj, CContext *pContext) {}
    virtual bool GetClassName(pnlString *pName, CPNLBase *pObj) = 0;
};

class PNL_API CObjHandlerForPersistence: public CObjHandler
{
public:
    CObjHandlerForPersistence(CPersistenceZoo *pZoo): m_pZoo(pZoo) {}
    virtual void CallByName(pnlString &name, CPNLBase *pObj, CContext *pContext);
    virtual void Run(CPersistence *pPersist, CPNLBase *pObj, CContext *pContext) = 0;
    virtual bool GetClassName(pnlString *pName, CPNLBase *pObj);

private:
    CPersistenceZoo* m_pZoo;
};

class PNL_API CObjSaver: public CObjHandlerForPersistence
{
public:
    CObjSaver(CPersistenceZoo *pZoo): CObjHandlerForPersistence(pZoo) {}
    virtual void Run(CPersistence *pPersist, CPNLBase *pObj, CContext *pContext);
    virtual void CallByNameForDup(pnlString &name, CPNLBase *pObj, CContext *pContext);
};

class PNL_API CObjInclusionEnumerator: public CObjHandlerForPersistence
{
public:
    CObjInclusionEnumerator(CPersistenceZoo *pZoo): CObjHandlerForPersistence(pZoo) {}
    virtual void Run(CPersistence *pPersist, CPNLBase *pObj, CContext *pContext);
};

class PNL_API CObjLoader: public CObjHandlerForPersistence
{
public:
    CObjLoader(CPersistenceZoo *pZoo): CObjHandlerForPersistence(pZoo), m_pObject(0) {}
    virtual void Run(CPersistence *pPersist, CPNLBase *pObj, CContext *pContext);
    CPNLBase *Object()
    {
	CPNLBase *ret = m_pObject;

	m_pObject = NULL;
	return ret;
    }

private:
    CPNLBase *m_pObject;
};

PNL_END

#endif // include guard
