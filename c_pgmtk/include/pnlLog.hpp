/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlLog.hpp                                                  //
//                                                                         //
//  Purpose:   Log system main file.                                       //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __PNLLOG_HPP__
#define __PNLLOG_HPP__

#include "pnlConfig.hpp"
#include <stdio.h>

#pragma warning(disable: 4710) // function not inlined
#pragma warning (push, 3 )
#include <string>
#pragma warning(pop)
#ifndef __PNLLOGMULTIPLEXOR_HPP__
#include "pnlLogMultiplexor.hpp"
#endif

PNL_BEGIN

class PNL_API Log
{
public: // USER INTERFACE
    Log(const char *prefix, int level, int service);
    Log(const char *signature);
    ~Log();

    // Log creating options is registered with 'signature'
    void Register(const char *signature) const;

    // write out output (automatically flashes after 'eol' output by default)
    void flush();

    void SetLevel(int newLevel) { --m_iUpdate; m_Level = newLevel; }
    void SetService(int newService)   { --m_iUpdate; m_Service = newService; }

    void printf(const char* pFmt, ...);
    inline Log& operator<<(const char*);
    inline Log& operator<<(const signed char*);
    inline Log& operator<<(const unsigned char*);
    inline Log& operator<<(char);
    inline Log& operator<<(signed char);
    inline Log& operator<<(unsigned char);
    inline Log& operator<<(unsigned long);
    inline Log& operator<<(long);
    inline Log& operator<<(unsigned short);
    inline Log& operator<<(short);
    inline Log& operator<<(unsigned int);
    inline Log& operator<<(int);
    inline Log& operator<<(bool);
    inline Log& operator<<(float);
    inline Log& operator<<(double);
    inline Log& operator<<(long double);
    inline Log& operator<<(void *);
    inline Log& operator<<(const void *);

public: // FUNCTIONS
    int Level() const { return m_Level; }
    int Service()  const { return m_Service; }

protected:
    LogMultiplexor &Multiplexor() const;
    void WriteString(const char *str, int strLen = -1);
    bool isDeniedToWrite()
    {
        return (m_iUpdate == Multiplexor().iUpdate())
            ? m_bDenyOutput:deriveBDenyOutput();
    }

    bool deriveBDenyOutput()
    {
        return (m_bDenyOutput = Multiplexor().GetBDenyOutput(&m_iUpdate, Level(), Service()));
    }

private: // DATA
    Log& operator=(const Log&) { return *this; } // deny copying

    int m_Level;
    int m_Service;
    int m_iInMultiplexor;// index in multiplexor
    int m_iUpdate;       // index of update (optimization issue)
    bool m_bDenyOutput;  // output is denied till (Multiplexor().iUpdate() == m_iUpdate)
    std::string m_Prefix;
    std::string m_String;
};

PNL_API Log& Log::operator<<(const char* str)
{
    if(!isDeniedToWrite())
    {
        WriteString(str);
    }

    return *this;
}

PNL_API Log& Log::operator<<(const signed char* str)
{
    return Log::operator<<((const char*)str);
}

PNL_API Log& Log::operator<<(const unsigned char* str)
{
    return Log::operator<<((const char*)str);
}



PNL_API Log& Log::operator<<(char ch)
{
    if(!isDeniedToWrite())
    {
        WriteString(&ch, 1);
    }

    return *this;
}

PNL_API Log& Log::operator<<(signed char ch)
{
    return Log::operator<<((char)ch);
}

PNL_API Log& Log::operator<<(unsigned char ch)
{
    return Log::operator<<((char)ch);
}

PNL_API Log& Log::operator<<(long value)
{
    if(!isDeniedToWrite())
    {
        unsigned long v = value  < 0 ? -value:value;
        char buf[24];
        char *ptr;

        ptr = buf + sizeof(buf) - 1;
        *ptr = 0;
        do
        {
#pragma warning(disable:4231)
            *--ptr = (char)('0' + (v % 10));
            v /= 10;
        } while(v > 0);

        if(value < 0)
            *--ptr = '-';

        WriteString(ptr, buf + sizeof(buf) - 1 - ptr);
    }

    return (*this);
}

PNL_API Log& Log::operator<<(unsigned long value)
{
    if(!isDeniedToWrite())
    {
        unsigned long v = value;
        char buf[24];
        char *ptr;

        ptr = buf + sizeof(buf) - 1;
        *ptr = 0;
        do
        {
            *--ptr = (char)('0' + (v % 10));
            v /= 10;
        } while(v > 0);

        WriteString(ptr, buf + sizeof(buf) - 1 - ptr);
    }

    return *this;
}

PNL_API Log& Log::operator<<(unsigned short value)
{
    return (*this)<<(unsigned long)value;
}

PNL_API Log& Log::operator<<(short value)
{
    return (*this)<<(long)value;
}

PNL_API Log& Log::operator<<(unsigned int value)
{
    return (*this)<<(unsigned long)value;
}

PNL_API Log& Log::operator<<(int value)
{
    return (*this)<<(long)value;
}

PNL_API Log& Log::operator<<(bool value)
{
    return (*this)<<(unsigned long)value;
}

PNL_API Log& Log::operator<<(float value)
{
    return (*this)<<(long double)value;
}

PNL_API Log& Log::operator<<(double value)
{
    return (*this)<<(long double)value;
}

PNL_API Log& Log::operator<<(long double value)
{
    if(!isDeniedToWrite())
    {
        char buf[128];
        int strLen = sprintf(buf, "%Lg", value);

        WriteString(buf, strLen);
    }

    return *this;
}

PNL_API Log& Log::operator<<(const void *value)
{
    if(!isDeniedToWrite())
    {
        char buf[68];
        int strLen = sprintf(buf, "%p", value);

        WriteString(buf, strLen);
    }

    return *this;
}

PNL_API Log& Log::operator<<(void *value)
{
    return *this << static_cast<const void *>(value);
}

PNL_END

#endif // include guard
