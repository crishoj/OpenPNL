/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlXMLContainer.hpp                                         //
//                                                                         //
//  Purpose:   This class keeps parsed XML document                        //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLXMLCONTAINER_HPP__
#define __PNLXMLCONTAINER_HPP__

#include "pnlString.hpp"

PNL_BEGIN

// pure virtual class
class PNL_API CXMLContainer
{
public:
    virtual void CreateNode(const pnlString &name) = 0;
    virtual void AddContent(const pnlString &addition) = 0;
    virtual void AddAttribute(const pnlString &attrName, const pnlString &attrValue) = 0;
    virtual void Close() = 0;

protected:
    virtual ~CXMLContainer() {}
};

PNL_END

#endif // include guard
