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

class PNL_API CPersistence
{
public:
    virtual const char *Signature() = 0;
    virtual void Save(CPNLBase *pObj, CContextSave *pContext) = 0;
    virtual CPNLBase *Load(CContextLoad *pContext) = 0;
    virtual void TraverseSubobject(CPNLBase *pObj, CContext *pContext) {}
};

class PNL_API CPersistenceZoo
{
public:
    typedef std::map<pnlString, CPersistence*> Map;

    void Register(CPersistence *pPersist);

    void Unregister(CPersistence *pPersist);

    CPersistence *Function(pnlString &name)
    {
        Map::iterator it = m_aFuncMap.find(name);
        return (it == m_aFuncMap.end()) ? 0:(*it).second;
    }

private:
    Map m_aFuncMap;
};

PNL_END

#endif // include guard
