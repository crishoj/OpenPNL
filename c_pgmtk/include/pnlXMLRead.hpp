/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlXMLRead.hpp                                              //
//                                                                         //
//  Purpose:   Reading of simplified XML                                   //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLXMLREAD_HPP__
#define __PNLXMLREAD_HPP__

#include "pnlString.hpp"

PNL_BEGIN

class CXMLRead
{
public:
    enum EToken
    {   eTOKEN_TAG
    ,   eTOKEN_ATTRIBUTE
    ,   eTOKEN_TAG_END
    ,   eTOKEN_STRING
    ,   eTOKEN_EOF
    ,   eTOKEN_BAD
    };

    // get token
    int GetToken(pnlString& str);   
        
    const pnlString& GetTokenArg2()
    {   return m_Arg2; }

    CXMLRead(std::istream *pFile)
        : m_bInsideTag(false), m_pFile(pFile), m_Ungetch(-70000), m_BufSize(0), m_BufPos(0)
    {}

protected:
    int Getch();
    void Ungetch(int ch)
    {
        m_Ungetch = ch;
    }
    int GetchAfterSpaces();

    int GetTag(pnlString& str);
    int GetAttribute(pnlString& str);

    int GetField(pnlString& str, const char* aDelimiter = NULL);
    int GetQString(pnlString& str, int quotationMark);

private:
    std::istream *m_pFile;
    bool m_bInsideTag;
    pnlString m_Arg2;
    unsigned char m_Buf[512];
    int m_BufSize;
    int m_BufPos;
    int m_Ungetch;
};

class PNL_API CXMLWriter
{
public:
    CXMLWriter() {}
    virtual ~CXMLWriter() {}

    virtual bool OpenFile(const char * filename) = 0;
    virtual bool CloseFile() = 0;

    virtual void OpenElement(const char * name) = 0;
    virtual void CloseElement(const char * name) = 0;

    virtual void WriteElement(
	const char * name, 
	const char * content = NULL, 
	bool escapeWhitespace = false) = 0;

    virtual void PushAttribute(const char *name, const char *value) = 0;

    virtual void WriteBody(const char *bodyText) = 0;
};

class PNL_API CXMLWriterStd: public CXMLWriter
{
public:
    CXMLWriterStd();
    virtual ~CXMLWriterStd();
    virtual bool OpenFile(const char * filename);
    virtual bool CloseFile();

    virtual void OpenElement(const char * name);
    virtual void CloseElement(const char * name);

    virtual void WriteElement(
	const char * name,
	const char * content,
	bool escapeWhitespace);

    virtual void WriteBody(const char *bodyText);

    virtual void PushAttribute(const char *name, const char *value);

protected:
    std::ofstream &Stream() const { return *m_File; }

private:
    void WriteAttributes();

private:
    std::ofstream *m_File;
    bool m_bDeleteFileOnExit;
    bool m_bEmpty;
    pnlVector<pnlString> m_aAttrName;
    pnlVector<pnlString> m_aAttrValue;
};

PNL_END

#endif // include guard
