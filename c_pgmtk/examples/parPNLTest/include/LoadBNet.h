/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  File:      LoadBNet.h                                                  //
//                                                                         //
//  Purpose:   Loading an BNet                                             //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __LOADBNET_H__
#define __LOADBNET_H__

#include "pnl_dll.hpp"
#include "pnlGraphicalModel.hpp"
#include <string>

PNL_USING
using namespace std;

CGraphicalModel* LoadGrModelFromXML(const string& xmlname, 
    vector<CEvidence*>* pEv = NULL);
int SaveGrModelAsXML(CGraphicalModel* pGM, const string& xmlname);

#endif // __LOADBNET_H__
