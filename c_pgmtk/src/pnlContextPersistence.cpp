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
    m_File << "<?xml version=\"1.0\"?>\n\n<PNLObjects version=\"1.1\">\n";
}

CContextSaveXML::~CContextSaveXML()
{
    m_File << "\n</PNLObjects>\n";
}

bool CContextPersistence::SaveAsXML(const std::string &filename) const
{
    CContextSaveXML xml(filename);
#ifdef DEBUG_PERSISTENCE
    CTimer tm;

    tm.Start();
    fprintf(stderr, "Saving started\n");
#endif

    xml.GetRootObjects(this);

    CObjHandler *pHandler = new CObjInclusionEnumerator(GetZoo());

    xml.SetObjectHandler(pHandler);
    xml.BeginTraverse(true);// enumerate object with subobject
    delete pHandler;
#ifdef DEBUG_PERSISTENCE
    tm.Stop();
    fprintf(stderr, "Saving: enumeration of objects %lf sec\n", tm.DurationLast());
    tm.Start();
#endif
    pHandler = new CObjSaver(GetZoo());
    xml.SetObjectHandler(pHandler);
    xml.BeginTraverse();
    xml.SetObjectHandler(NULL);
    delete pHandler;
#ifdef DEBUG_PERSISTENCE
    tm.Stop();
    fprintf(stderr, "Saving: writing to disk %lf sec\n", tm.DurationLast());
    fprintf(stderr, "Saving: full time %lf sec\n", tm.Duration());
#endif

    return true;
}

bool CContextPersistence::LoadXML(const std::string &filename)
{
    CContextLoadXML xml(filename);
#ifdef DEBUG_PERSISTENCE
    CTimer tm;

    tm.Start();
    fprintf(stderr, "Loading started\n");
#endif

    if(!xml.SwallowXML())
    {
        return false;
    }
#ifdef DEBUG_PERSISTENCE
    tm.Stop();
    fprintf(stderr, "Loading: swallow file in %lf sec\n", tm.DurationLast());
    tm.Start();
#endif
    xml.RecursiveCopying(2);

#ifdef DEBUG_PERSISTENCE
    tm.Stop();
    fprintf(stderr, "Loading: recursive copying in %lf sec\n", tm.DurationLast());
    tm.Start();
#endif
    CObjHandler *pHandler = new CObjLoader(GetZoo());

    xml.SetObjectHandler(pHandler);
    xml.BeginTraverse();
    xml.SetObjectHandler(NULL);
    delete pHandler;
#ifdef DEBUG_PERSISTENCE
    tm.Stop();
    fprintf(stderr, "Loading: parse and create objects in %lf sec\n",
        tm.DurationLast());
    tm.Start();
#endif

    GetRootObjects(&xml);
#ifdef DEBUG_PERSISTENCE
    tm.Stop();
    fprintf(stderr, "Loading: getting root objects in %lf sec\n", tm.DurationLast());
    fprintf(stderr, "Loading: full time %lf sec\n", tm.Duration());
    SetAutoDeleteRootObjects();
#endif

    return true;
}


void CContextSave::BeginTraverseObject(const pnlString &typeName, TreeEntry& rEntry)
{
    m_aTag.push_back(rEntry.m_Name);
    m_aAttrName.resize(1);
    m_aValue.resize(1);
    m_aAttrName[0] = "TypeName";
    m_aValue[0].assign(typeName);
    m_Text.assign("");
    m_bPlanned = false;
}

void CContextSaveXML::BeginTraverseObject(const pnlString &typeName, TreeEntry& rEntry)
{
    CContextSave::BeginTraverseObject(typeName, rEntry);

    if(!rEntry.m_pObject)
    {
        pnlString buf;
        buf << "@";
        for(int i = 0; i < rEntry.m_PathToReplacing.size(); ++i)
        {
            buf << rEntry.m_PathToReplacing[i]().m_Name;
            if(i < rEntry.m_PathToReplacing.size() - 1)
            {
                buf << "/";
            }
        }
        AddAttribute("DuplicatedObject", buf.c_str());
    }
}

void CContextSave::EndTraverseObject(const pnlString &, TreeEntry&)
{
    DoEndTraverseObject();
    m_aTag.pop_back();
}

void CContextLoad::EndTraverseObject(const pnlString &, TreeEntry& rEnt)
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
        pnlString buf;
        int i;
        
        buf << '\n';
        for(i = 0; i < m_Indices.size(); ++i, buf << ' ');

        buf << "<" << m_aTag.back();
        for(i = 0; i < m_aAttrName.size(); ++i)
        {
            buf << " " << m_aAttrName[i].c_str() << "=\"" << m_aValue[i].c_str() << "\"";
        }
        
        m_File << buf.c_str() << ">\n";
        for(i = 0; i < m_Indices.size(); ++i, buf << ' ');

        m_File << "    " << m_Text.c_str();
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
        m_File << "</" << m_aTag.back().c_str() << '>';
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

void CContextLoad::BeginTraverseObject(const pnlString &typeName, TreeEntry& rEntry)
{
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
            break;
        }
    }
}
