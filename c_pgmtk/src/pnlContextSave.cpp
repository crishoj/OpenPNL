/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlContextSave.cpp                                          //
//                                                                         //
//  Purpose:   Saving contexts (includes saving of XML)                    //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlContextSave.hpp"
#include "pnlPersistence.hpp"
#include "pnlObjHandler.hpp"
#include "pnlXMLWrite.hpp"

PNL_USING

extern CPersistenceZoo *GetZoo();

CContextSaveXML::CContextSaveXML(const std::string &filename)
: m_bDeleteWriter(true)
{
    m_pWriter = new CXMLWriterStd();
    m_pWriter->OpenFile(filename.c_str());
}

CContextSaveXML::CContextSaveXML(CXMLWriter *writer): m_pWriter(writer),
m_bDeleteWriter(false)
{
}

CContextSaveXML::~CContextSaveXML()
{
    if(m_bDeleteWriter)
    {
	m_pWriter->CloseFile();
	delete m_pWriter;
    }
}

void CContextSave::BeginTraverseObject(const pnlString &typeName, TreeEntry& rEntry)
{
    m_aTag.push_back(rEntry.m_Name);
    m_aAttrName.resize(1);
    m_aValue.resize(1);
    m_aAttrName[0] = "TypeName";
    m_aValue[0].assign(typeName);
    m_Text.assign("");
    m_bPlanned = false;
}

void CContextSaveXML::BeginTraverseObject(const pnlString &typeName, TreeEntry& rEntry)
{
    CContextSave::BeginTraverseObject(typeName, rEntry);

    if(!rEntry.m_pObject)
    {
        pnlString buf;
        buf << "@";
        for(int i = 0; i < rEntry.m_PathToReplacing.size(); ++i)
        {
            buf << rEntry.m_PathToReplacing[i]().m_Name;
            if(i < rEntry.m_PathToReplacing.size() - 1)
            {
                buf << "/";
            }
        }
        AddAttribute("DuplicatedObject", buf.c_str());
    }
}

void CContextSave::EndTraverseObject(const pnlString &, TreeEntry&)
{
    DoEndTraverseObject();
    m_aTag.pop_back();
}

void CContextSaveXML::BeforeInterior()
{
    if(IsPlanned())
    {
        int i;
        
        //for(i = 0; i < m_Indices.size(); ++i, buf << ' ');
        for(i = 0; i < m_aAttrName.size(); ++i)
        {
	    m_pWriter->PushAttribute(m_aAttrName[i].c_str(), m_aValue[i].c_str());
        }
	m_pWriter->OpenElement(m_aTag.back().c_str());
	if(m_Text.length())
	{
	    m_pWriter->WriteBody(m_Text.c_str());
	}
    }
    m_Text.resize(0);
    m_aAttrName.resize(0);
    m_aValue.resize(0);
}

void CContextSaveXML::DoEndTraverseObject()
{
    if(IsPlanned())
    {
	m_pWriter->CloseElement(m_aTag.back().c_str());
    }
}

void
CContextSave::PlanForWriting()
{
    m_bPlanned = true;
    BeforeInterior();
}

CContextSave::~CContextSave()
{
}
