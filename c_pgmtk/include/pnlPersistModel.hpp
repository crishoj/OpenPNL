/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPersistModels.hpp                                        //
//                                                                         //
//  Purpose:   Implementation of models derived from CGraphicalModel       //
//             saving/loading                                              //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __PNLPERSISTMODELS_HPP__
#define __PNLPERSISTMODELS_HPP__

#include "pnlPersistence.hpp"

PNL_BEGIN

class PNL_API CPersistBNet: public CPersistence
{
public:
    virtual const char *Signature() { return "BNet"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual void TraverseSubobject(CPNLBase *pObj, CContext *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

class PNL_API CPersistIDNet: public CPersistence
{
public:
    virtual const char *Signature() { return "IDNet"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual void TraverseSubobject(CPNLBase *pObj, CContext *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

class PNL_API CPersistDBN: public CPersistence
{
public:
    virtual const char *Signature() { return "DBN"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual void TraverseSubobject(CPNLBase *pObj, CContext *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

class PNL_API CPersistMNet: public CPersistence
{
public:
    virtual const char *Signature() { return "MNet"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual void TraverseSubobject(CPNLBase *pObj, CContext *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

class PNL_API CPersistMRF2: public CPersistence
{
public:
    virtual const char *Signature() { return "MRF2"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual void TraverseSubobject(CPNLBase *pObj, CContext *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

PNL_END

#endif // include guard
