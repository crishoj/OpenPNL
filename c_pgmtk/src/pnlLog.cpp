/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlLog.cpp                                                  //
//                                                                         //
//  Purpose:   Implementation of log subsystem                             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include <stdarg.h>
#include "pnlLog.hpp"
#include "pnlLogMultiplexor.hpp"

PNL_USING

Log::Log(const char *prefix, int level, int service):
m_Level(level), m_Service(service), m_Prefix(prefix), m_iUpdate(0)
{
    m_iInMultiplexor = LogMultiplexor::StdMultiplexor().AttachLogger(this);
}

struct SavedSignature
{
    std::string m_Signature;
    std::string m_Prefix;
    int m_Level;
    int m_Service;
    
    SavedSignature(const std::string &signature, const std::string &prefix, int level, int service)
        :m_Signature(signature), m_Prefix(prefix), m_Level(level), m_Service(service)
    {}

    SavedSignature(const SavedSignature &s)
        :m_Signature(s.m_Signature), m_Prefix(s.m_Prefix),
        m_Level(s.m_Level), m_Service(s.m_Service)
    {}

    void reset(const std::string &signature, const std::string &prefix, int level, int service);
};

void
SavedSignature::reset(const std::string &signature, const std::string &prefix, int level, int service)
{
    m_Signature.assign(signature);
    m_Prefix.assign(prefix);
    m_Level = level;
    m_Service = service;
}

struct SavedSignature defaultSignature("", "", 0, 0);
static std::list<struct SavedSignature> signList;

void
Log::Register(const char *signature) const
{
    std::string sign(signature);

    if(!signature || !signature[0])
    {
        defaultSignature.reset(sign, m_Prefix, Level(), Service());
        return;
    }

    std::list<struct SavedSignature>::iterator it, end;

    it = signList.begin();
    end = signList.end();
    for(; it != end; ++it)
    {
        if(sign == it->m_Signature)
        {
            (*it).reset(sign, m_Prefix, Level(), Service());
            return;
        }
    }
    signList.push_back(SavedSignature(sign, m_Prefix, Level(), Service()));
}

Log::Log(const char *signature): m_iUpdate(0)
{
    const SavedSignature *pSS = &defaultSignature;

    if(signature && *signature)
    {
        std::string sign(signature);
        std::list<struct SavedSignature>::iterator it, end;
        
        it = signList.begin();
        end = signList.end();
        for(; it != end; ++it)
        {
            if(sign == it->m_Signature)
            {
                pSS = &(*it);
                break;
            }
        }
    }

    m_Level = pSS->m_Level;
    m_Service  = pSS->m_Service;
    m_Prefix   = pSS->m_Prefix;

    m_iInMultiplexor = LogMultiplexor::StdMultiplexor().AttachLogger(this);
}

Log::~Log()
{
    if(m_String.length())
    {
        m_String.append("\n");
        Multiplexor().WriteString(Level(), Service(), m_String.c_str(), m_String.length());
    }
    Multiplexor().DetachLogger(this, m_iInMultiplexor);
}

LogMultiplexor &Log::Multiplexor() const
{
    return LogMultiplexor::StdMultiplexor();
}

void
Log::printf(const char* pFmt, ...)
{
    if(isDeniedToWrite())
        return;
    
    va_list varg;
    
    va_start(varg, pFmt);
    
    char buf[1024];
    char *ptr = buf;
    int outSz, ptrSz = sizeof(buf);
    
    for(;;)
    {
#ifdef _WIN32
        outSz = _vsnprintf(ptr, ptrSz, pFmt, varg);
#else
        outSz = vsnprintf(ptr, ptrSz, pFmt, varg);
#endif
        if(outSz < ptrSz - 2 && outSz != -1)
            break;
        ptrSz = (outSz > 0) ? outSz + 3 : ptrSz*2;
        if(ptr != buf)
            delete[] ptr;
        ptr = new char[ptrSz];
    }
    WriteString(ptr);
    if(ptr != buf)
        delete[] ptr;
    va_end(varg);
}

void
Log::WriteString(const char* str, int strLen)
{
    const char *ptr;

    if(strLen < 0)
    {
        strLen = strlen(str);
    }

    for(;strLen > 0;)
    {
        if(!m_String.length())
        {
            m_String = m_Prefix.c_str();
        }

        ptr = (const char*)memchr(str, '\n', strLen);
        if(!ptr)
        {
            m_String.append(str, strLen);
            return;
        }

        m_String.append(str, ++ptr - str);
        Multiplexor().WriteString(Level(), Service(), m_String.c_str(), m_String.length());
        m_String.resize(0);
        strLen -= ptr - str;
        str = ptr;
    }
}

void
Log::flush()
{
    if(m_String.length())
    {
        Multiplexor().WriteString(Level(), Service(), m_String.c_str(), m_String.length());
        m_String.resize(0);
    }
}
