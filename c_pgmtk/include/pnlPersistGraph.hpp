/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPersistenceGraph.hpp                                     //
//                                                                         //
//  Purpose:   Implementation of CGraph saving/loading                     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __PNLPERSISTENCEGRAPH_HPP__
#define __PNLPERSISTENCEGRAPH_HPP__

#include "pnlPersistence.hpp"

PNL_BEGIN

class PNL_API CGraphPersistence: public CPersistence
{
public:
    virtual const char *Signature() { return "Graph"; }
    virtual void Save(CPNLBase *pObj, CContextSave *pContext);
    virtual CPNLBase *Load(CContextLoad *pContext);
    virtual bool IsHandledType(CPNLBase *pObj) const;
};

PNL_END

#endif // include guard
