/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlContextPersistence.hpp                                   //
//                                                                         //
//  Purpose:   Persistence contexts                                        //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __PNLCONTEXTSAVE_HPP__
#define __PNLCONTEXTSAVE_HPP__

#pragma warning (push, 3 )
#include <fstream>
#pragma warning(pop)

#ifndef __PNLCONTEXT_HPP__
#include "pnlContext.hpp"
#endif

PNL_BEGIN

// FORWARDS
class CXMLWriter;
class CXMLContainer;

class CContextSave: public CContext
{
public:
    CContextSave() {}
    virtual void BeginTraverseObject(const pnlString &typeName, TreeEntry& rEntry);
    virtual void   EndTraverseObject(const pnlString &typeName, TreeEntry& rEntry);
    virtual void DoEndTraverseObject() = 0;
    void AddAttribute(const char *attr, const char *value)
    {
        m_aAttrName.push_back(pnlString(attr));
        m_aValue.push_back(pnlString(value));
    }
    void AddAttribute(const char *attr, const int value)
    {
	char buf[20];

	sprintf(buf, "%i", value);
	AddAttribute(attr, buf);
    }
    void AddAttribute(const char *attr, bool value)
    {
        AddAttribute(attr, value ? "1":"0");
    }
    void AddAttribute(const char *attr, const float value)
    {
	char buf[40];

	sprintf(buf, "%lg", (double)value);
	AddAttribute(attr, buf);
    }
    void AddText(const char *text)
    {
        m_Text.append(text);
    }

    void PlanForWriting();
    virtual void BeforeInterior() {}
    virtual ~CContextSave();

protected:
    bool IsPlanned() const { return m_bPlanned; }

protected:
    pnlVector<pnlString> m_aTag;
    pnlString m_Text;
    pnlVector<pnlString> m_aAttrName;
    pnlVector<pnlString> m_aValue;
    bool m_bPlanned;

private:
    CContextSave(const CContextSave &) // deny copy-ctor
    {}
};

class CContextSaveXML: public CContextSave
{
public:
    CContextSaveXML(const std::string &filename);
    CContextSaveXML(CXMLWriter *writer);
    virtual ~CContextSaveXML();

    virtual void BeginTraverseObject(const pnlString &typeName, TreeEntry& rEntry);
    virtual void DoEndTraverseObject();
    virtual void BeforeInterior();

private:
    CContextSaveXML(const CContextSaveXML &) // deny copy-ctor
    {}

    CXMLWriter *m_pWriter;
    bool m_bDeleteWriter;
};

PNL_END

#endif // include guard
