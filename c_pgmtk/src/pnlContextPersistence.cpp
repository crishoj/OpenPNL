/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlContextPersistence.cpp                                   //
//                                                                         //
//  Purpose:   Base class for saving/loading contexts                      //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlContextPersistence.hpp"
#include "pnlPersistence.hpp"
#include "pnlObjHandler.hpp"
#include "pnlXMLRead.hpp"

PNL_USING

extern CPersistenceZoo *GetZoo();

CContextSaveXML::CContextSaveXML(const std::string &filename)
: CContextSave(filename)
{
    m_File << "<?xml version=\"1.0\"?>\n\n<PNLObjects version=\"1.0\">\n";
}

CContextSaveXML::~CContextSaveXML()
{
    m_File << "\n</PNLObjects>\n";
}

bool CContextPersistence::SaveAsXML(const std::string &filename) const
{
    CContextSaveXML xml(filename);

    xml.GetRootObjects(this);

    CObjHandler *pHandler = new CObjInclusionEnumerator(GetZoo());

    xml.SetObjectHandler(pHandler);
    xml.BeginTraverse(true);// enumerate object with subobject
    delete pHandler;
    pHandler = new CObjSaver(GetZoo());
    xml.SetObjectHandler(pHandler);
    xml.BeginTraverse();
    xml.SetObjectHandler(NULL);
    delete pHandler;

    return true;
}

bool CContextPersistence::LoadXML(const std::string &filename)
{
    CContextLoadXML xml(filename);

    xml.SwallowXML();
    xml.RecursiveCopying(2);

    CObjHandler *pHandler = new CObjLoader(GetZoo());

    xml.SetObjectHandler(pHandler);
    xml.BeginTraverse();
    xml.SetObjectHandler(NULL);
    delete pHandler;

    GetRootObjects(&xml);

    return true;
}


void CContextSave::BeginTraverseObject(const std::string &typeName, TreeEntry& rEntry)
{
    m_aTag.push_back(rEntry.m_Name);
    m_aAttrName.resize(1);
    m_aValue.resize(1);
    m_aAttrName[0] = "TypeName";
    m_aValue[0].assign(typeName);
    m_Text.assign("");
    m_bPlanned = false;
}

void CContextSaveXML::BeginTraverseObject(const std::string &typeName, TreeEntry& rEntry)
{
    CContextSave::BeginTraverseObject(typeName, rEntry);

    if(!rEntry.m_pObject)
    {
        stringstream buf;
	buf << '@';
	for(int i = 0; i < rEntry.m_PathToReplacing.size(); ++i)
	{
	    buf << rEntry.m_PathToReplacing[i]().m_Name;
	    if(i < rEntry.m_PathToReplacing.size() - 1)
	    {
		buf << '/';
	    }
	}
	AddAttribute("DuplicatedObject", buf.str().c_str());
    }
}

void CContextSave::EndTraverseObject(const std::string &, TreeEntry&)
{
    DoEndTraverseObject();
    m_aTag.pop_back();
}

void CContextLoad::EndTraverseObject(const std::string &, TreeEntry& rEnt)
{
    if(!Current().m_bDup)
    {
	Current().m_pObject = ((CObjLoader*)m_pObjectHandler)->Object();
    }
}

void CContextSaveXML::BeforeInterior()
{
    if(IsPlanned())
    {
        std::stringstream buf;
	int i;
        
        buf.str().reserve(1024);
	buf << '\n';
	for(i = 0; i < m_Indices.size(); ++i, buf << ' ');

        buf << "<" << m_aTag.back();
        for(i = 0; i < m_aAttrName.size(); ++i)
        {
            buf << " " << m_aAttrName[i] << "=\"" << m_aValue[i] << "\"";
        }
        
        m_File << buf.str() << ">\n";
	for(i = 0; i < m_Indices.size(); ++i, buf << ' ');

        m_File << "    " << m_Text;
    }
    m_Text.resize(0);
    m_aAttrName.resize(0);
    m_aValue.resize(0);
}

void CContextSaveXML::DoEndTraverseObject()
{
    if(IsPlanned())
    {
	m_File << '\n';
	for(int i = 0; i < m_Indices.size(); ++i, m_File << ' ');
        m_File << "</" << m_aTag.back() << '>';
    }
}

void
CContextSave::PlanForWriting()
{
    m_bPlanned = true;
    BeforeInterior();
}

CContextSave::~CContextSave()
{
    if(m_File)
    {
	m_File.close();
    }
}

void
CContextLoad::RecursiveCopying(int iaaEntryTxt)
{
    int iTree = m_Tree.size();
    int j, i;

    m_Tree.resize(iTree + 1);
    m_Tree[iTree].reserve(m_aaEntryTxt[iaaEntryTxt].size());
    for(i = 0; i < m_aaEntryTxt[iaaEntryTxt].size(); ++i)
    {
	j = (m_aaEntryTxt[iaaEntryTxt][i].m_iChildren == -1) ? -1:int(m_Tree.size());
	m_Tree[iTree].push_back(TreeEntry(m_aaEntryTxt[iaaEntryTxt][i].m_Name,
	    j, &m_aaEntryTxt[iaaEntryTxt][i]));
	if(j >= 0)
	{
	    RecursiveCopying(m_aaEntryTxt[iaaEntryTxt][i].m_iChildren);
	}
    }
}

void CContextLoad::BeginTraverseObject(const std::string &typeName, TreeEntry& rEntry)
{
}

void CContextLoadXML::BeginTraverseObject(const std::string &typeName, TreeEntry& rEntry)
{
    CContextLoad::BeginTraverseObject(typeName, rEntry);
    TreeTextEntry &rTextEntry = *(TreeTextEntry*)rEntry.m_pUser;
    if(rTextEntry.m_aAttrName.size() > 1 && rTextEntry.m_aAttrName[1] == "DuplicatedObject")
    {
        istringstream buf(rTextEntry.m_aAttrValue[1]);
	char ch;
	std::string name;
	int iTree = 0;

	buf >> ch;
	rEntry.m_bDup = true;
	ASSERT(ch == '@');
	for(;;)
	{
	    bool bError = true;

	    name.resize(0);
	    for(;buf;)
	    {
		buf.get(ch);
		if(!buf || ch == '/')
		{
		    break;
		}
		name.append(1, ch);
	    }
	    for(int i = 0; i < m_Tree[iTree].size(); ++i)
	    {
		if(m_Tree[iTree][i].m_Name == name)
		{
		    rEntry.m_PathToReplacing.push_back(TreeLeafIndex<TreeEntry>(m_Tree, iTree, i));
		    if(!buf)
		    {
			return;
		    }
		    iTree = m_Tree[iTree][i].m_iSubTree;
		    ASSERT(iTree > 0);
		    bError = false;
		    break;
		}
	    }
	    if(bError)
	    {
		PNL_THROW(CBadArg, "Not found reference in xml-file");
	    }
	}
    }
}

bool
CContextLoadXML::SwallowXML()
{
    int token, nLeaf;
    CXMLRead tokenSource(&m_File);
    std::string arg1;
    pnlVector<XMLLeaf> stackOpened;
    
    m_aaEntryTxt.resize(1);
    m_aaEntryTxt[0].resize(0);
    m_aaEntryTxt[0].push_back(TreeTextEntry(std::string("root")));
    stackOpened.push_back(XMLLeaf(m_aaEntryTxt, 0, 0));
    
    for(;;)
    {
	token = tokenSource.GetToken(arg1);
	nLeaf = m_aaEntryTxt.size();
	switch(token)
	{
	case CXMLRead::eTOKEN_STRING:
	    stackOpened.back()().m_Text += arg1;
	    break;
	case CXMLRead::eTOKEN_EOF:
	default:
	    if(stackOpened.size() < 2 && m_aaEntryTxt.size() > 1)
	    {
		return true;
	    }
	    // FALLTHROUGH
Error:	    return false;
	case CXMLRead::eTOKEN_TAG:
	    if(arg1[0] == '/')
	    {
		// closing tag
		ASSERT(stackOpened.back()().m_Name == arg1);
		stackOpened.pop_back();
		if((token = tokenSource.GetToken(arg1)) != CXMLRead::eTOKEN_TAG_END)
		{
		    goto Error;
		}

		continue;
	    }
	    {
		m_aaEntryTxt.reserve(m_aaEntryTxt.size() + 1);
		TreeTextEntry &rEnt = stackOpened.back()();
		// rEnt must be handled with care
		if(rEnt.m_iChildren < 0)
		{
		    m_aaEntryTxt.resize(nLeaf + 1);
		    rEnt.m_iChildren = nLeaf;
		    m_aaEntryTxt[nLeaf].reserve(4);
		}
		
		stackOpened.push_back(XMLLeaf(m_aaEntryTxt, rEnt.m_iChildren,
		    m_aaEntryTxt[rEnt.m_iChildren].size()));
		m_aaEntryTxt[rEnt.m_iChildren].push_back(TreeTextEntry(arg1));

		TreeTextEntry &rNewEnt = stackOpened.back()();
		// rNewEnt must be handled with care
	    
		while((token = tokenSource.GetToken(arg1)) == CXMLRead::eTOKEN_ATTRIBUTE)
		{
		    rNewEnt.m_aAttrName.push_back(arg1);
		    rNewEnt.m_aAttrValue.push_back(tokenSource.GetTokenArg2());
		}
	    }
	    
	    if(token != CXMLRead::eTOKEN_TAG_END)
	    {
		goto Error;
	    }
	    break;
	}
    }
}
