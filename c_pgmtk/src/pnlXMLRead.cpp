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

PNL_USING

int
CXMLRead::GetToken(std::string& str)
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
	return eTOKEN_TAG_END;
    }
    
    Ungetch(ch);
    return GetAttribute(str);
}

int
CXMLRead::GetTag(std::string& str)
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
CXMLRead::GetField(std::string& str, const char* aDelimiter)
{
    int ch;
    char bitmap[32];
    
    str.resize(0);
    memset((void*)bitmap, 0, sizeof(bitmap));
    for(; aDelimiter && *aDelimiter; ++aDelimiter)
    {
	bitmap[*aDelimiter >> 3] |= (1 << (*aDelimiter & 7));
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
        str += char(ch);
    }
}

int
CXMLRead::GetQString(std::string& str, int quotationMark)
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
CXMLRead::GetAttribute(std::string& str)
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
    int ch;
    
    if(m_Ungetch != -70000)
    {
	ch = m_Ungetch;
	m_Ungetch = -70000;
	return ch;
    }
    
    ch = m_pFile->get();

    if(m_pFile->eof())
    {
	ch = -1;
	Ungetch(-1);
    }

    return ch;
}
