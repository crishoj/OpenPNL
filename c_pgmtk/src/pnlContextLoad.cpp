/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlContextLoad.cpp                                          //
//                                                                         //
//  Purpose:   Loading contexts (includes XML loading)                     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlContextLoad.hpp"
#include "pnlXMLRead.hpp"
#include "pnlXMLContainer.hpp"
#include "pnlObjHandler.hpp"

#include <sstream>

PNL_USING

void CContextLoad::EndTraverseObject(const pnlString &, TreeEntry& rEnt)
{
    if(!Current().m_bDup)
    {
        Current().m_pObject = ((CObjLoader*)m_pObjectHandler)->Object();
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
        j = (m_aaEntryTxt[iaaEntryTxt][i].m_iChildren == -1)
	    ? -1:int(m_Tree.size());
        m_Tree[iTree].push_back(TreeEntry(m_aaEntryTxt[iaaEntryTxt][i].m_Name,
            j, &m_aaEntryTxt[iaaEntryTxt][i]));
        if(j >= 0)
        {
            RecursiveCopying(m_aaEntryTxt[iaaEntryTxt][i].m_iChildren);
        }
    }
}

void
CContextLoad::BeginTraverseObject(const pnlString &typeName, TreeEntry& rEntry)
{
}

void CContextLoad::GetChildrenNames(pnlVector<pnlString> *paChild) const
{
    int i, iTree = Current().m_iSubTree;

    if(iTree < 0)
    {
	paChild->resize(0);
	return;
    }
    i = m_Tree[iTree].size();
    paChild->resize(i);
    for(; --i >= 0; (*paChild)[i] = m_Tree[iTree][i].m_Name);
}

void CContextLoadXML::BeginTraverseObject(const pnlString &typeName, TreeEntry& rEntry)
{
    CContextLoad::BeginTraverseObject(typeName, rEntry);
    TreeTextEntry &rTextEntry = *(TreeTextEntry*)rEntry.m_pUser;
    if(rTextEntry.m_aAttrName.size() > 1 && rTextEntry.m_aAttrName[1] == "DuplicatedObject")
    {
        istringstream buf(rTextEntry.m_aAttrValue[1].c_str());
        char ch;
        pnlString name;
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
                name << ch;
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
    int token, nLeaf, iLevel;
    CXMLRead tokenSource(&m_File);
    pnlString arg1;
    pnlVector<XMLLeaf> stackOpened;
    
    m_aaEntryTxt.resize(1);
    m_aaEntryTxt[0].resize(0);
    m_aaEntryTxt[0].push_back(TreeTextEntry(pnlString("root")));
    stackOpened.push_back(XMLLeaf(m_aaEntryTxt, 0, 0));
    
    for(;;)
    {
        token = tokenSource.GetToken(arg1);
        nLeaf = m_aaEntryTxt.size();
        iLevel = stackOpened.size();
        switch(token)
        {
        case CXMLRead::eTOKEN_STRING:
            stackOpened.back()() += arg1;
            break;
        case CXMLRead::eTOKEN_EOF:
        default:
            if(stackOpened.size() < 2 && m_aaEntryTxt.size() > 1)
            {
                return true;
            }
            // FALLTHROUGH
Error:      return false;
        case CXMLRead::eTOKEN_TAG:
            if(arg1[0] == '/')
            {
                // closing tag
                ASSERT(stackOpened.back()().m_Name == (arg1.data() + 1));
                stackOpened.pop_back();
                if((token = tokenSource.GetToken(arg1)) != CXMLRead::eTOKEN_TAG_END)
                {
                    goto Error;
                }

                continue;
            }
            {
                if(m_aaEntryTxt.capacity() <= nLeaf)
                {
                    m_aaEntryTxt.reserve(nLeaf + 20 + (nLeaf >> 1));
                }
                int sz = stackOpened.size();
                if(stackOpened.capacity() <= sz)
                {
                    stackOpened.reserve(sz + 20 + (sz >> 1));
                }
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
	    if(arg1.length() && arg1[0] == '/')// tag without inserted tags or body
	    {
                stackOpened.pop_back();
	    }
            break;
        }
    }
}

bool
CContextLoadXML::SwallowXMLToContainer(CXMLContainer *container)
{
    int token;
    CXMLRead tokenSource(&m_File);
    pnlString arg1;
    pnlVector<pnlString> stackOpened;

    token = tokenSource.GetToken(arg1);
    if(token == CXMLRead::eTOKEN_STRING)
    {// skip for '<?xml version="1.0"?>'
	token = tokenSource.GetToken(arg1);
    }

    stackOpened.reserve(10);
    for(;;token = tokenSource.GetToken(arg1))
    {
        switch(token)
        {
        case CXMLRead::eTOKEN_STRING:
	    if(!stackOpened.size())
	    {
		goto Error;
	    }
	    container->AddContent(arg1);
            break;
        case CXMLRead::eTOKEN_EOF:
        default:
            if(stackOpened.size() < 1)
            {
                return true;
            }
            // FALLTHROUGH
Error:      return false;
        case CXMLRead::eTOKEN_TAG:
            if(arg1[0] == '/')
            {
                // closing tag
                ASSERT(stackOpened.back() == (arg1.data() + 1));
                stackOpened.pop_back();
		container->Close();
                if((token = tokenSource.GetToken(arg1)) != CXMLRead::eTOKEN_TAG_END)
                {
                    goto Error;
                }

                continue;
            }
	    stackOpened.push_back(arg1);
	    container->CreateNode(arg1);
	    while((token = tokenSource.GetToken(arg1)) == CXMLRead::eTOKEN_ATTRIBUTE)
	    {
		container->AddAttribute(arg1, tokenSource.GetTokenArg2());
	    }
            
            if(token != CXMLRead::eTOKEN_TAG_END)
            {
                goto Error;
            }
	    if(arg1.length() && arg1[0] == '/')// tag without inserted tags or body
	    {
                stackOpened.pop_back();
		container->Close();
	    }
            break;
        }
    }
}
