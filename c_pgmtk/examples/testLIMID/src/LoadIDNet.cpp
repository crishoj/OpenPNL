/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      LoadIDNet.cpp                                               //
//                                                                         //
//  Purpose:   Loading Influence Diagram net                               //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "LoadIDNet.h"
#include "pnlContextPersistence.hpp"

PNL_USING
using namespace std;

const string nameOfIDModel = "Model";

// ----------------------------------------------------------------------------
CIDNet* LoadIDNetFromXML(const string fname)
{
  CIDNet* pIDNet = NULL;

  CContextPersistence ContextLoad;
  if (!ContextLoad.LoadXML(fname))
  {
    cout << "\nfile " << fname.c_str() << "isn't correct as XML";
    std::cout.flush();
    return NULL;
  }
  pIDNet = static_cast<CIDNet*>(ContextLoad.Get(nameOfIDModel.c_str()));
  if (NULL == pIDNet)
  {
    cout << "\nfile " << fname.c_str() << 
            "isn't containing an object with name: " << nameOfIDModel.c_str();
    std::cout.flush();
    return NULL;
  }

  return pIDNet;
}
// ----------------------------------------------------------------------------

int SaveIDNetAsXML(CIDNet* pIDNet, const string fname)
{
  CContextPersistence ContextSave;
  ContextSave.Put(pIDNet, nameOfIDModel.c_str());
  if (!ContextSave.SaveAsXML(fname))
  {
    cout << "\nobject can't saved to file " << fname.c_str();
    std::cout.flush();
    return 0;
  }

  return 1;
}
// end of file ----------------------------------------------------------------
