/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlContext.hpp                                              //
//                                                                         //
//  Purpose:   Base class for traversal classes                            //
//             (for example saving/loading classes)                        //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __PNLCONTEXT_HPP__
#define __PNLCONTEXT_HPP__

#ifndef __PNLEXCEPTION_HPP__
#include "pnlException.hpp"
#endif

#ifndef __PNLSTRING_HPP__
#include "pnlString.hpp"
#endif

PNL_BEGIN

// FORWARDS
class CPNLBase;
class CObjHandler;

#ifndef ASSERT
#define ASSERT(A)     if( (A) != true ) { PNL_THROW( CBadArg, #A " must be true"); }
#endif

class PNL_API CContext
{
public: // USER INTERFACE
    CContext();
    virtual ~CContext()
    {
	for(int i = 0; i < m_paAutoDelete.size(); ++i)
	{
	    delete m_paAutoDelete[i];
	}
    }

    void Put(CPNLBase *pObj, const char *name, bool bAutoDelete = false);
    CPNLBase *Get(const char *name);

public: // INTERFACE FOR CLASSES DERIVED FROM CPersistence
    void AutoDelete(CPNLBase *pObj)
    {
	m_paAutoDelete.push_back(pObj);
    }

public: // INTERFACE FOR INNER USE
    void BeginTraverse(bool bEnumeration = false);
    void SetObjectHandler(CObjHandler *pHandler)
    {
        m_pObjectHandler = pHandler;
    }

    void GetRootObjects(const CContext *pContext)
    {
        m_Tree.resize(1);
	m_Tree[0].reserve(pContext->m_Tree[0].size());
	for(int i = 0; i < pContext->m_Tree[0].size(); ++i)
	{
	    m_Tree[0].push_back(pContext->m_Tree[0][i].Clone());
	}
    }
    void SetAutoDeleteRootObjects()
    {
	for(int i = 0; i < m_Tree[0].size(); ++i)
	{
	    if(m_Tree[0][i].m_pObject)
	    {
		AutoDelete(m_Tree[0][i].m_pObject);
	    }
	}
    }

protected: // DATA TYPES
    template<typename Type> class TreeLeafIndex
    {
    public:
        TreeLeafIndex(pnlVector<pnlVector<Type> >& aaEntry, int major, int minor)
            : m_paaEntry(&aaEntry), m_Major(major), m_Minor(minor)
        {}
        
        TreeLeafIndex()
            : m_paaEntry(0), m_Major(0), m_Minor(-1)
        {}
        
        Type& operator()() const
        {
            return (*m_paaEntry)[m_Major][m_Minor];
        }
#ifdef PNL_VC7
	bool operator<(const TreeLeafIndex &nt) const 
	{
	    return (m_Major < nt.m_Major) ? true:
	    ((m_Major > nt.m_Major) ? false:m_Minor < nt.m_Minor);
	}
	bool operator==(const TreeLeafIndex &nt) const 
	{
	    return (m_Major == nt.m_Major) && (m_Minor == nt.m_Minor);
	}
#endif

        int m_Major;
        int m_Minor;

    private:
        pnlVector<pnlVector<Type> > *m_paaEntry;
    };

    struct TreeEntry
    {
	TreeEntry()
	    : m_iSubTree(-1), m_pObject(0), m_pUser(0), m_bDup(false)
	{}

        TreeEntry(const pnlString& name, int iSubTree, CPNLBase *pObject)
            : m_Name(name), m_iSubTree(iSubTree), m_pObject(pObject),
	    m_pUser(0), m_bDup(false)
        {}
        
        TreeEntry(const pnlString& name, int iSubTree, void *pUser)
            : m_Name(name), m_iSubTree(iSubTree), m_pObject(0),
	    m_pUser(pUser), m_bDup(false)
        {}
        
        TreeEntry(const TreeEntry &te): m_Name(te.m_Name), m_iSubTree(te.m_iSubTree),
	    m_pObject(te.m_pObject), m_PathToReplacing(te.m_PathToReplacing),
	    m_bDup(te.m_bDup), m_pUser(te.m_pUser)
	{}

        const TreeEntry &operator=(const TreeEntry &te)
        {
            if(this != &te)
            {
                m_Name = te.m_Name;
                m_iSubTree = te.m_iSubTree;
                m_pObject = te.m_pObject;
		m_PathToReplacing = te.m_PathToReplacing;
		m_bDup = te.m_bDup;
		m_pUser = te.m_pUser;
            }

            return *this;
        }

#ifdef PNL_VC7
	bool operator<(const TreeEntry &nt) const
	{
	    return m_Name < nt.m_Name;
	}
	bool operator==(const TreeEntry &nt) const
	{
	    return m_Name == nt.m_Name;
	}
#endif

	TreeEntry Clone() const
	{
	    return TreeEntry(m_Name, -1, m_pObject);
	}

        pnlString m_Name;
        int m_iSubTree;
        CPNLBase *m_pObject;
	pnlVector<TreeLeafIndex<TreeEntry> > m_PathToReplacing;
	bool m_bDup;
        void *m_pUser;
    };

protected: // FUNCTIONS
    TreeEntry& Current()
    {
        return m_Indices.back()();
    }

    const TreeEntry& Current() const
    {
        return m_Indices.back()();
    }

    virtual void GetObjWithObjTypeName(CPNLBase **ppObj,
        pnlString *pObjTypeName, const TreeEntry &rEntry) const
    {
	if(rEntry.m_pObject)
	{
	    *ppObj = rEntry.m_pObject;
	}
	else
	{
	    const_cast<TreeEntry&>(rEntry).m_bDup = true;
	    if(rEntry.m_PathToReplacing.size() == 0 ||
		rEntry.m_PathToReplacing.back()().m_pObject == 0)
	    {
		PNL_THROW(CInternalError, "Both of pointer to object and "
		    "pointer to replacing object are nulls");
	    }
	    *ppObj = rEntry.m_PathToReplacing.back()().m_pObject;
	}
        pObjTypeName->resize(0);
    }
    bool IsEnumeration() { return m_bEnumeration; }
    void Traverse(int iTree);

    virtual void BeginTraverseObject(const pnlString &, TreeEntry&) {}
    virtual void   EndTraverseObject(const pnlString &, TreeEntry&) {}
    virtual bool HandleObjectBeforeInterior() const { return true; }

protected: // DATA
    pnlVector<TreeLeafIndex<TreeEntry> > m_Indices;
    pnlVector<pnlVector<TreeEntry> > m_Tree;
    CObjHandler *m_pObjectHandler;
    pnlVector<CPNLBase*> m_paAutoDelete;
    bool m_bEnumeration;
    std::map<CPNLBase *, int, less<void*> > m_MapUniqueObj;
    pnlVector<pnlVector<TreeLeafIndex<TreeEntry> > > m_HeapUniqueObj;
    int m_iAccelTree;
    std::map<pnlString, int> m_Accelerator;

private:
    const CContext& operator=(const CContext&) { return *this; } // deny copying
};

PNL_END

#endif // include guard
