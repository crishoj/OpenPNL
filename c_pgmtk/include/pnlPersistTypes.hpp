/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPersistTypes.hpp                                         //
//                                                                         //
//  Purpose:   Implementation of different types's saving/loading          //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __PNLPERSISTTYPES_HPP__
#define __PNLPERSISTTYPES_HPP__

#include "pnlPersistence.hpp"

PNL_BEGIN

// FORWARDS
class CDistribFun;

class PNL_API CPersistNodeType: public CPersistence
{
public:
    virtual const char *Signature() { return "NodeType"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

class PNL_API CPersistNodeValues: public CPersistence
{
public:
    virtual const char *Signature() { return "NodeValues"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual void TraverseSubobject(CPNLBase *pObj, CContext *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

class PNL_API CPersistEvidence: public CPersistNodeValues
{
public:
    virtual const char *Signature() { return "Evidence"; }
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual void TraverseSubobject(CPNLBase *pObj, CContext *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

class PNL_API CPersistMatrixFlt: public CPersistence
{
public:
    virtual const char *Signature() { return "MatrixOfFloat"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const
    { return dynamic_cast<CMatrix<float>*>(pObj) != 0; }
};

class PNL_API CPersistMatrixDistribFun: public CPersistence
{
public:
    virtual const char *Signature() { return "MatrixDistribFun"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual void TraverseSubobject(CPNLBase *pObj, CContext *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const
    { return dynamic_cast<CMatrix<CDistribFun*>*>(pObj) != 0; }
};

class PNL_API CPersistValueVector: public CPersistence
{
public:
    virtual const char *Signature() { return "ValueVector"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

class PNL_API CPersistGroup: public CPersistence
{
public:
    virtual const char *Signature() { return "Group"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual void TraverseSubobject(CPNLBase *pObj, CContext *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

PNL_END

#endif // include guard
