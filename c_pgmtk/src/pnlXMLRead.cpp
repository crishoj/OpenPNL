/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlXMLRead.cpp                                              //
//                                                                         //
//  Purpose:   Reading of simplified XML                                   //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlXMLRead.hpp"
#include <fstream>

PNL_USING

int
CXMLRead::GetToken(pnlString& str)
{
    int ch = (m_bInsideTag) ? GetchAfterSpaces():Getch();
    
    if(ch == -1)
    {
	return eTOKEN_EOF;
    }
    
    if(!m_bInsideTag)
    {
	Ungetch(ch);
	return (ch == '<') ? GetTag(str):GetField(str, "<");
    }
    
    if(ch == '>')
    {
	m_bInsideTag = false;
	str.resize(0);
	return eTOKEN_TAG_END;
    }
    
    if(ch == '/')
    {
	ch = Getch();
	if(ch != '>')
	{
	    return eTOKEN_BAD;
	}

	m_bInsideTag = false;
	str = "/";
	return eTOKEN_TAG_END;
    }
    
    Ungetch(ch);
    return GetAttribute(str);
}

int
CXMLRead::GetTag(pnlString& str)
{
    if(Getch() != '<')
    {
	return eTOKEN_BAD;
    }

    if(GetField(str, " \t\n>") != eTOKEN_STRING)
    {
	return eTOKEN_BAD;
    }

    if(str[0] == '!' || str[0] == '?')
    {
	if(GetField(str, ">") != eTOKEN_STRING || Getch() != '>')
	{
	    return eTOKEN_BAD;
	}

	return GetToken(str);
    }
    
    m_bInsideTag = true;

    return eTOKEN_TAG;
}

int
CXMLRead::GetField(pnlString& str, const char* aDelimiter)
{
    int ch;
    char bitmap[32];
    
    str.resize(0);
    memset((void*)bitmap, 0, sizeof(bitmap));
    if(aDelimiter)
    {
	for(; *aDelimiter; ++aDelimiter)
	{
	    bitmap[*aDelimiter >> 3] |= (1 << (*aDelimiter & 7));
	}
    }
    for(;;)
    {
        if((ch = Getch()) == -1)
	{
            return eTOKEN_STRING;
	}
	if((bitmap[ch >> 3] & (1 << (ch & 7))) != 0)
	{
            Ungetch(ch);
            return eTOKEN_STRING;
	}
        str << char(ch);
    }
}

int
CXMLRead::GetQString(pnlString& str, int quotationMark)
{
    char aDelimiter[2];
    
    aDelimiter[0] = char(quotationMark);
    aDelimiter[1] = 0;

    return (GetField(str, aDelimiter) != eTOKEN_STRING || Getch() != quotationMark)
	? eTOKEN_BAD:eTOKEN_STRING;
}

int
CXMLRead::GetchAfterSpaces()
{
    int ch;
    
    for(;;)
    {
        ch = Getch();
        if(ch == -1 || !isspace(ch))
	{
            return ch;
	}
    }
}

int
CXMLRead::GetAttribute(pnlString& str)
{
    str.resize(0);

    if(GetField(str, "<=>'\"") != eTOKEN_STRING || GetchAfterSpaces() != '=')
    {
	return eTOKEN_BAD;
    }

    int ch = GetchAfterSpaces();

    if(ch != '\'' && ch != '"')
    {
	return eTOKEN_BAD;
    }

    return (GetQString(m_Arg2, ch) == eTOKEN_STRING) ? eTOKEN_ATTRIBUTE:eTOKEN_BAD;
}

int CXMLRead::Getch()
{
    if(m_Ungetch != -70000)
    {
	int ch = m_Ungetch;
	m_Ungetch = -70000;
	return ch;
    }
    
    if(m_BufPos < m_BufSize)
    {
	return m_Buf[m_BufPos++];
    }

    if(!m_pFile->eof())
    {
	m_pFile->read((char*)m_Buf, sizeof(m_Buf));
	m_BufPos = 0;
	m_BufSize = m_pFile->gcount();
	if(m_BufPos < m_BufSize)
	{
	    return m_Buf[m_BufPos++];
	}
    }
    
    Ungetch(-1);

    return -1;
}

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
