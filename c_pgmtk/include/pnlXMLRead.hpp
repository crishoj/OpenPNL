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
    int GetToken(std::string& str);   
        
    const std::string& GetTokenArg2()
    {   return m_Arg2; }

    CXMLRead(std::istream *pFile)
        : m_bInsideTag(false), m_pFile(pFile), m_Ungetch(-70000)
    {}

protected:
    int Getch();
    void Ungetch(int ch)
    {
        m_Ungetch = ch;
    }
    int GetchAfterSpaces();

    int GetTag(std::string& str);
    int GetAttribute(std::string& str);

    int GetField(std::string& str, const char* aDelimiter = NULL);
    int GetQString(std::string& str, int quotationMark);

private:
    std::istream *m_pFile;
    bool m_bInsideTag;
    std::string m_Arg2;
    int m_Ungetch;
};

PNL_END

#endif // include guard
