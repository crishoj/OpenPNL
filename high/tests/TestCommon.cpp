#include "test_conf.hpp"
#include "pnlException.hpp"
#include "pnl_dll.hpp"

using namespace std;

const string nameOfModel = "Model";
const string nameOfEvidence = "my_ev";

std::ostream &operator<<(std::ostream &str, const TokArr &ta)
{
    return str << String(ta);
}

pnl::CGraphicalModel* LoadGrModelFromXML(const string& xmlname, vector<pnl::CEvidence*>* pEv)
{
    pnl::CGraphicalModel* pGM = NULL;
    
    pnl::CContextPersistence ContextLoad;
    if ( !ContextLoad.LoadXML(xmlname) )
    {
        cout << "\nfile " << xmlname.c_str() << "isn't correct as XML";
        return NULL;
    }
    pGM = static_cast<pnl::CGraphicalModel*>(ContextLoad.Get(nameOfModel.c_str()));    
    if (NULL == pGM)
    {
        cout << "\nfile " << xmlname.c_str() << 
            "isn't containing an object with name: " << nameOfModel.c_str();
        return NULL;
    }

    if (pEv)
    {
        pnl::CEvidence* ev = NULL;
        
        pEv->clear();
        ev = static_cast<pnl::CEvidence*>(
            ContextLoad.Get(string(nameOfEvidence + "0").c_str()));
        if (ev) 
            pEv->push_back(ev);
    }
    
    return pGM;
}

