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
//  Purpose:   Front-end for persistence                                   //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlContextPersistence.hpp"
#include "pnlContextLoad.hpp"
#include "pnlContextSave.hpp"
#include "pnlPersistence.hpp"
#include "pnlObjHandler.hpp"
#include "pnlXMLRead.hpp"

PNL_USING

extern CPersistenceZoo *GetZoo();

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

bool CContextPersistence::SaveViaWriter(CXMLWriter *writer) const
{
    CContextSaveXML xml(writer);

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

bool CContextPersistence::LoadXMLToContainer(CXMLContainer *container, const std::string &filename)
{
    CContextLoadXML xml(filename);

    return xml.SwallowXMLToContainer(container);
}

