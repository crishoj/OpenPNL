/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPersistDistribFun.hpp                                    //
//                                                                         //
//  Purpose:   Implementation of                                           //
//             saving/loading                                              //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __PNLPERSISTDISTRIBFUN_HPP__
#define __PNLPERSISTDISTRIBFUN_HPP__

#include "pnlPersistence.hpp"

PNL_BEGIN

class PNL_API CPersistGaussianDistribFun: public CPersistence
{
public:
    virtual const char *Signature() { return "GaussianDistribFun"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual void TraverseSubobject(CPNLBase *pObj, CContext *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

class PNL_API CPersistSoftMaxDistribFun: public CPersistence
{
public:
    virtual const char *Signature() { return "SoftMaxDistribFun"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual void TraverseSubobject(CPNLBase *pObj, CContext *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

class PNL_API CPersistTabularDistribFun: public CPersistence
{
public:
    virtual const char *Signature() { return "TabularDistribFun"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual void TraverseSubobject(CPNLBase *pObj, CContext *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

class PNL_API CPersistCondGaussianDistribFun: public CPersistence
{
public:
    virtual const char *Signature() { return "ConditionalGaussianDistribFun"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual void TraverseSubobject(CPNLBase *pObj, CContext *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

class PNL_API CPersistCondSoftMaxDistribFun: public CPersistence
{
public:
    virtual const char *Signature() { return "ConditionalSoftMaxDistribFun"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual void TraverseSubobject(CPNLBase *pObj, CContext *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

class PNL_API CPersistScalarDistribFun: public CPersistence
{
public:
    virtual const char *Signature() { return "ScalarDistribFun"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual void TraverseSubobject(CPNLBase *pObj, CContext *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

PNL_END

#endif // include guard
