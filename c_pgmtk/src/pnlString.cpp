/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlpnlString.cpp                                            //
//                                                                         //
//  Purpose:   See pnlpnlString.hpp                                        //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlString.hpp"

PNL_BEGIN

#ifdef DEBUG_PERSISTENCE
static int aStatAll;
static int aStat[40];
static int aStatMore;

static void stat(int len)
{
    ++aStatAll;
    if(len < sizeof(aStat)/sizeof(aStat[0]))
    {
	++aStat[len];
    }
    else
    {
	++aStatMore;
    }
}
#endif

pnlString::pnlString(): m_Len(0), m_MaxLen(sizeof(m_Static)), m_Ptr(m_Static)
{
    m_Static[0] = 0;
}

pnlString::pnlString(const char *str)
{
    Alloc(str, strlen(str));
}

pnlString::pnlString(const std::string &str)
{
    Alloc(str.data(), str.length());
}

pnlString::pnlString(const pnlString &str)
{
    Alloc(str.data(), str.length());
}

pnlString::~pnlString()
{
#ifdef DEBUG_PERSISTENCE
    stat(m_Len);
#endif
    if(m_Ptr != m_Static)
    {
	delete[] m_Ptr;
    }
}

void pnlString::Alloc(const char *str, int len)
{
    if((m_Len = len) >= sizeof(m_Static))
    {
	m_MaxLen = RoundSize(len);
	m_Ptr = new char[m_MaxLen];
    }
    else
    {
	m_MaxLen = sizeof(m_Static);
	m_Ptr = m_Static;
    }
    memcpy(m_Ptr, str, len);
    m_Ptr[len] = 0;
}

void pnlString::resize(int minNewSize)
{
    if(minNewSize < m_MaxLen)
    {
	m_Len = minNewSize;
	return;
    }
    m_MaxLen = RoundSize(minNewSize);
    char *p = new char[m_MaxLen];

    memcpy(p, m_Ptr, size());
    if(m_Ptr != m_Static)
    {
	delete[] m_Ptr;
    }
    m_Ptr = p;
    m_Len = minNewSize;
}

void pnlString::reserve(int minNewSize)
{
    int savedSize = size();

    resize(minNewSize);
    m_Len = savedSize;
}

void pnlString::append(const char *str, int len)
{
    if(len < 0)
    {
	len = strlen(str);
    }

    if(len + size() >= m_MaxLen)
    {
	reserve((len + size())*2);
    }

    memcpy(m_Ptr + size(), str, len);
    m_Len += len;
    m_Ptr[size()] = 0;
}

PNL_END
