/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlContextLoad.hpp                                          //
//                                                                         //
//  Purpose:   Loading contexts (includes XML loading)                     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __PNLCONTEXTLOAD_HPP__
#define __PNLCONTEXTLOAD_HPP__

#pragma warning (push, 3 )
#include <fstream>
#pragma warning(pop)

#ifndef __PNLCONTEXT_HPP__
#include "pnlContext.hpp"
#endif

PNL_BEGIN

// FORWARDS
class CXMLContainer;

class CContextLoad: public CContext
{
public:
    CContextLoad(const std::string &filename): m_File(filename.c_str(), ios::in|ios::binary) {}
    virtual void BeginTraverseObject(const pnlString &typeName, TreeEntry& rEntry);
    virtual void   EndTraverseObject(const pnlString &typeName, TreeEntry& rEntry);
    struct TreeTextEntry
    {
        TreeTextEntry(const pnlString &name): m_Name(name), m_iChildren(-1) {}
        TreeTextEntry(): m_iChildren(-1) {}
        const TreeTextEntry &operator=(const TreeTextEntry &a)
        {
            if(this != &a)
            {
                m_Text = a.m_Text;
                m_Name = a.m_Name;
                m_aAttrName = a.m_aAttrName;
                m_aAttrValue = a.m_aAttrValue;
                m_iChildren = a.m_iChildren;
            }
            return *this;
        }
	const TreeTextEntry &operator+=(const pnlString &text)
	{
	    m_Text << text;

	    return *this;
	}
#ifdef PNL_VC7
	bool operator<(const TreeTextEntry &nt) const
	{
	    return m_Name < nt.m_Name;
	}
	bool operator==(const TreeTextEntry &nt) const
	{
	    return m_Name == nt.m_Name;
	}
#endif

        pnlString m_Text;
        pnlString m_Name;
        pnlVector<pnlString> m_aAttrName;
        pnlVector<pnlString> m_aAttrValue;
        int m_iChildren;
    };

    ~CContextLoad()
    {
        m_File.close();
    }

    virtual bool HandleObjectBeforeInterior() const { return false; }

    void RecursiveCopying(int iaaEntryTxt);

    void GetText(pnlString &text)
    {
        text = ((TreeTextEntry*)Current().m_pUser)->m_Text;
    }

    void GetAttribute(pnlString &attrValue, const pnlString &attrName)
    {
        TreeTextEntry *pEnt = (TreeTextEntry*)Current().m_pUser;

        FindAttribute(*pEnt, attrValue, attrName);
    }
    void GetAttribute(bool *attrValue, const pnlString &attrName)
    {
	pnlString attrValueStr;

	GetAttribute(attrValueStr, attrName);
	*attrValue = ((attrValueStr.size() > 0) && (attrValueStr[0] == '1'));
    }
    void GetAttribute(int *attrValue, const pnlString &attrName)
    {
	pnlString attrValueStr;

	GetAttribute(attrValueStr, attrName);
	attrValue[0] = atoi(attrValueStr.c_str());
    }
    void GetAttribute(float *attrValue, const pnlString &attrName)
    {
	pnlString attrValueStr;

	GetAttribute(attrValueStr, attrName);
	attrValue[0] = (float)atof(attrValueStr.c_str());
    }

    void GetAttrNames(pnlVector<pnlString> *paAttrName)
    {
	TreeTextEntry &rEnt = *(TreeTextEntry*)Current().m_pUser;

	if(rEnt.m_aAttrName.size())
	{
	    paAttrName->assign(rEnt.m_aAttrName.begin() + 1, rEnt.m_aAttrName.end());
	}
	else
	{
	    paAttrName->resize(0);
	}
    }

    void GetChildrenNames(pnlVector<pnlString> *paChild) const;

protected:
    static void FindAttribute(TreeTextEntry &rEnt, pnlString &attrValue, const pnlString &attrName)
    {
        for(int i = 0; i < rEnt.m_aAttrName.size(); ++i)
        {
            if(rEnt.m_aAttrName[i] == attrName)
            {
                attrValue = rEnt.m_aAttrValue[i];
                return;
            }
        }

        attrValue.resize(0);
    }

    virtual void GetObjWithObjTypeName(CPNLBase **ppObj,
        pnlString *pObjTypeName, const TreeEntry &rEntry) const
    {
        *ppObj = 0;
        FindAttribute(*((TreeTextEntry*)rEntry.m_pUser), *pObjTypeName,
            pnlString("TypeName"));
    }

    CContextLoad() {}

protected:
    std::ifstream m_File;
    pnlVector<pnlVector<TreeTextEntry> > m_aaEntryTxt;

private:
    CContextLoad(const CContextLoad &) // deny copy-ctor
    {}
};

class CContextLoadXML: public CContextLoad
{
public:
    CContextLoadXML(const std::string &filename): CContextLoad(filename) {}

    class XMLLeaf
    {
    public:
        XMLLeaf(pnlVector<pnlVector<TreeTextEntry> >& aaEntry, int major, int minor)
            : m_paaEntry(&aaEntry), m_Major(major), m_Minor(minor)
        {}
        
        TreeTextEntry& operator()()
        {
            return (*m_paaEntry)[m_Major][m_Minor];
        }

#ifdef PNL_VC7
	const XMLLeaf& operator=(const XMLLeaf &nt)
	{
	    m_Major = nt.m_Major;
	    m_Minor = nt.m_Minor;
	    m_paaEntry = nt.m_paaEntry;//???

	    return *this;
	}
	bool operator<(const XMLLeaf &nt) const
	{
	    return (m_Major < nt.m_Major) ? true:
	    ((m_Major > nt.m_Major) ? false:m_Minor < nt.m_Minor);
	}
	bool operator==(const XMLLeaf &nt) const
	{
	    return (m_Major == nt.m_Major) && (m_Minor == nt.m_Minor);
	}
#endif

	XMLLeaf(): m_Major(0), m_Minor(0), m_paaEntry(0) {}

    private:
        pnlVector<pnlVector<TreeTextEntry> > *m_paaEntry;
        int m_Major;
        int m_Minor;
    };

    virtual void BeginTraverseObject(const pnlString &typeName, TreeEntry& rEntry);
    bool SwallowXML();
    bool SwallowXMLToContainer(CXMLContainer *container);

private:
    CContextLoadXML(const CContextLoadXML &) // deny copy-ctor
    {}
};

PNL_END

#endif // include guard
