/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlXMLWrite.hpp                                             //
//                                                                         //
//  Purpose:   Writing of XML-file                                         //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLXMLWRITE_HPP__
#define __PNLXMLWRITE_HPP__

#include "pnlString.hpp"

PNL_BEGIN

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
