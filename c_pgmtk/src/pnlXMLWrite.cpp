/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlXMLWrite.cpp                                             //
//                                                                         //
//  Purpose:   Writing of XML-file                                         //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlXMLWrite.hpp"
#include <fstream>

PNL_USING

CXMLWriterStd::CXMLWriterStd(): m_File(0), m_bDeleteFileOnExit(false)
{
}

CXMLWriterStd::~CXMLWriterStd()
{
    if(m_bDeleteFileOnExit)
    {
	delete m_File;
    }
}

bool CXMLWriterStd::OpenFile(const char * filename)
{
    m_File = new std::ofstream();
    Stream().open(filename, ios_base::trunc|ios_base::out);
    Stream() << "<?xml version=\"1.0\"?>\n\n<PNLObjects version=\"1.1\">\n";
    m_bDeleteFileOnExit = true;

    return Stream().good();
}

bool CXMLWriterStd::CloseFile()
{
    Stream() << "\n</PNLObjects>\n";
    m_File->close();

    return true;
}

void CXMLWriterStd::OpenElement(const char *name)
{
    Stream() << "<" << name;
    WriteAttributes();
    Stream() << ">\n";
    m_bEmpty = true;
}

void CXMLWriterStd::WriteAttributes()
{
    for(int i = 0; i < m_aAttrName.size(); ++i)
    {
	Stream() << " " << m_aAttrName[i].c_str()
	    << "=\"" << m_aAttrValue[i].c_str() << '"';
    }

    m_aAttrName.resize(0);
    m_aAttrValue.resize(0);
}

void CXMLWriterStd::CloseElement(const char *name)
{
    Stream() << "</" << name << ">\n";
    m_bEmpty = false;
}

void CXMLWriterStd::WriteElement(
	const char * name,
	const char * content,
	bool escapeWhitespace)
{
    m_bEmpty = false;
    if(content && content[0])
    {
	OpenElement(name);
	Stream() << content;
	CloseElement(name);
    }
    else
    {
	Stream() << "<" << name;
	WriteAttributes();
	Stream() << "/>\n"; 
    }
}

void CXMLWriterStd::PushAttribute(const char *name, const char *value)
{
    m_aAttrName.push_back(name);
    m_aAttrValue.push_back(value);
}

void CXMLWriterStd::WriteBody(const char *bodyText)
{
    Stream() << bodyText;
}
