/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPersistence.hpp                                          //
//                                                                         //
//  Purpose:   Base class for object's saving/loading                      //
//             (for example saving/loading CGraph)                         //
//             + CPersistenceZoo class                                     //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __PNLPERSISTENCE_HPP__
#define __PNLPERSISTENCE_HPP__

#ifndef __PNLSTRING_HPP__
#include "pnlString.hpp"
#endif

PNL_BEGIN

// FORWARDS
class CPNLBase;
class CContext;
class CContextLoad;
class CContextSave;
class CPersistenceZoo;

class PNL_API CPersistence
{
public:
    virtual ~CPersistence();
    virtual const char *Signature() = 0;
    virtual void Save(CPNLBase *pObj, CContextSave *pContext) = 0;
    virtual CPNLBase *Load(CContextLoad *pContext) = 0;
    virtual void TraverseSubobject(CPNLBase *pObj, CContext *pContext) {}
    virtual bool IsHandledType(CPNLBase *pObj) const = 0;
    friend class CPersistenceZoo;

    // functions IsHandledType, Signature, and omitted ParentType is related
    // to Type functions. When class 'Type' be ready, it must be removed from here

protected:
    CPersistence();

private:
    CPersistence *m_pNext;
    CPersistence *m_pPrev;
};

class PNL_API CPersistenceZoo
{
public:
    typedef std::map<pnlString, CPersistence*> Map;

    void Register(CPersistence *pPersist);

    void Unregister(CPersistence *pPersist, bool bDTOR = false);

    CPersistence *ObjectBySignature(pnlString &name);

    bool GetClassName(pnlString *pName, CPNLBase *pObj);

    ~CPersistenceZoo();
    CPersistenceZoo();

private:
    void RescanIfNeed();// Rescan if need (uses m_iUpdate)

    int m_iUpdate;// If this index != saved index (see .cpp) then perform rescan
    Map m_aFuncMap;// storage for CPersistence objects. We search CPersistence objects by typename
    bool m_bUnregister;// false if inside destructor and mustn't handle Unregister
};

PNL_END

#endif // include guard
