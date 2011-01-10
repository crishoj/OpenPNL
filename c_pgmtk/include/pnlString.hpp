/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlpnlString.hpp                                            //
//                                                                         //
//  Purpose:   String classes. Contain special function for conversions.   //
//             Has constant or linear mean time                            //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __PNLSTRING_HPP__
#define __PNLSTRING_HPP__

#include <stdio.h>
#include <cstring>
#ifndef __PNLCONFIG_H__
#include "pnlConfig.h"
#endif

PNL_BEGIN

// FORWARDS

class PNL_API pnlString
{
public: // USER INTERFACE
    pnlString();
    pnlString(const std::string &str);
    pnlString(const char *str);
    pnlString(const pnlString &str);
    virtual ~pnlString();
    void append(const char *str, int len = -1);
    unsigned size() const { return m_Len; }
    unsigned length() const { return size(); }
    const char* data() const { return m_Ptr; }
    const char* c_str() const { return m_Ptr; }
    inline pnlString &operator<<(const char *str);
    inline pnlString &operator<<(const std::string &str);
    inline pnlString &operator<<(const pnlString &str);

    // Have to overload for each primitive type to avoid an ambiguity 
    // error caused by someone passing in a type with no exact match.
    inline pnlString &operator<<(char ch);
    inline pnlString &operator<<(signed char ch);
    inline pnlString &operator<<(unsigned char ch);
    inline pnlString &operator<<(bool);
    inline pnlString &operator<<(short);
    inline pnlString &operator<<(unsigned short);
    inline pnlString &operator<<(int);
    inline pnlString &operator<<(unsigned int);
    inline pnlString &operator<<(long);
    inline pnlString &operator<<(unsigned long);
    inline pnlString &operator<<(float);
    inline pnlString &operator<<(double);
    inline pnlString &operator<<(long double);

    bool operator!=(const char *str) const { return !operator==(str); }
    bool operator!=(const std::string &str) const { return !operator==(str); }
    bool operator!=(const pnlString &str) const { return !operator==(str); }
    bool operator==(const char *str) const
    {
	return (size() == 0 && str[0] == 0) || !memcmp(str, data(), size() + 1);
    }
    bool operator==(const std::string &str) const
    {
	register unsigned sz = size();
	return str.size() == sz && (sz == 0 || !memcmp(str.data(), data(), sz));
    }
    bool operator==(const pnlString &str) const
    {
	register unsigned sz = size();
	return str.size() == sz && (sz == 0 || !memcmp(str.data(), data(), sz));
    }
    bool operator<(const pnlString &str) const
    {
        unsigned minSize = size() < str.size() ? size():str.size();
	return memcmp(data(), str.data(), minSize + 1) < 0;
    }
    const pnlString& operator=(const pnlString &str)
    {
	if(&str != this)
	{
	    m_Len = 0;
	    append(str.data(), str.size());
	}
	return *this;
    }
    void assign(const pnlString &str)
    {
	if(&str != this)
	{
	    m_Len = 0;
	    append(str.data(), str.size());
	}
    }
    char operator[](int i)
    {
	return m_Ptr[i];
    }
    void resize(int minNewSize);
    void reserve(int minNewSize);
    const pnlString &operator+=(const char *str)
    {
	return operator<<(str);
    }
    const pnlString &operator+=(const pnlString &str)
    {
	return operator<<(str);
    }
    void append(int count, char ch)
    {
	for(; --count >= 0;)
	{
	    operator<<(ch);
	}
    }

protected:
    // it returns always more then minNewSize
    inline int RoundSize(int minNewSize) const;
    void Alloc(const char *str, int len);

private:
    int m_Len;
    int m_MaxLen;
    char *m_Ptr;
    char m_Static[36];
};

// it returns always more then minNewSize
inline int pnlString::RoundSize(int minNewSize) const
{
    return (minNewSize & (~3)) + 4;
}

inline pnlString &pnlString::operator<<(const char *str)
{
    append(str);

    return *this;
}

inline pnlString &pnlString::operator<<(const std::string &str)
{
    append(str.data(), str.length());

    return *this;
}

inline pnlString &pnlString::operator<<(const pnlString &str)
{
    append(str.data(), str.size());

    return *this;
}

inline pnlString &pnlString::operator<<(char ch)
{
    append(&ch, 1);

    return *this;
}

inline pnlString &pnlString::operator<<(signed char sch)
{
    (*this) << (char) sch;

    return *this;
}

inline pnlString &pnlString::operator<<(unsigned char uch)
{
    (*this) << (char) uch;

    return *this;
}

inline pnlString &pnlString::operator<<(bool val)
{
    (*this) << (unsigned long) val;

    return *this;
}

inline pnlString &pnlString::operator<<(short val)
{
    (*this) << (long) val;

    return *this;
}

inline pnlString &pnlString::operator<<(unsigned short val)
{
    (*this) << (unsigned long) val;

    return *this;
}

inline pnlString &pnlString::operator<<(int val)
{
    (*this) << (long) val;

    return *this;
}

inline pnlString &pnlString::operator<<(unsigned int val)
{
    (*this) << (unsigned long) val;

    return *this;
}

inline pnlString &pnlString::operator<<(long val)
{
    if(val < 0)
    {
	append("-", 1);
	(*this) << (((unsigned long)(-(val + 1))) + 1);
    }
    else
    {
	(*this) << (unsigned long)val;
    }

    return *this;
}

inline pnlString &pnlString::operator<<(unsigned long val)
{
    char buf[22];
    char *ptr = buf + sizeof(buf);

    do
    {
#pragma warning (push, 2)
	*--ptr = '0' + char(val % 10);
#pragma warning (pop)
	val /= 10;// integer division
    } while(val != 0);

    append(ptr, buf + sizeof(buf) - ptr);

    return *this;
}

inline pnlString &pnlString::operator<<(float val)
{
    (*this) << (long double)val;

    return *this;
}

inline pnlString &pnlString::operator<<(double val)
{
    (*this) << (long double)val;

    return *this;
}

inline pnlString &pnlString::operator<<(long double val)
{
    char buf[128];
    int l = sprintf(buf, "%Lf", val);

    if(l < 1)
    {
	l = -1;
    }
    append(buf, l);

    return *this;
}

inline std::ostream &operator<<(std::ostream &stream, const pnlString &str)
{
    return stream << str.c_str();
}

PNL_END

#endif // include guard
