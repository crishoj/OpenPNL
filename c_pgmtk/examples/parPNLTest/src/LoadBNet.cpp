/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  File:      LoadBNet.cpp                                                //
//                                                                         //
//  Purpose:   Loading an BNet                                             //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "LoadBNet.h"
#include "pnlContextPersistence.hpp"
#include <map>

PNL_USING
using namespace std;

const string nameOfModel = "Model";
const string nameOfEvidence = "my_ev";

CGraphicalModel* LoadGrModelFromXML(const string& xmlname, vector<CEvidence*>* pEv)
{
    CGraphicalModel* pGM = NULL;
    
    CContextPersistence ContextLoad;
    if ( !ContextLoad.LoadXML(xmlname) )
    {
        cout << "\nfile " << xmlname.c_str() << "isn't correct as XML";
        return NULL;
    }
    pGM = static_cast<CGraphicalModel*>(ContextLoad.Get(nameOfModel.c_str()));    
    if (NULL == pGM)
    {
        cout << "\nfile " << xmlname.c_str() << 
            "isn't containing an object with name: " << nameOfModel.c_str();
        return NULL;
    }

    if (pEv)
    {
        CEvidence* ev = NULL;
        
        pEv->clear();
        ev = static_cast<CEvidence*>(
            ContextLoad.Get(string(nameOfEvidence + "0").c_str()));
        if (ev) 
            pEv->push_back(ev);
    }
    
    return pGM;
}

int SaveGrModelAsXML(CGraphicalModel* pGM, const string& xmlname)
{
    CContextPersistence ContextSave;
    ContextSave.Put(pGM, nameOfModel.c_str());
    if ( !ContextSave.SaveAsXML(xmlname) )
    {
        cout << "\nobject can't saved to file " << xmlname.c_str();
        return 0;
    }
    
    return 1;
}
