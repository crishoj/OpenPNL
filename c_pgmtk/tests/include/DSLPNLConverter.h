#ifndef DSLPNLCONVERTER_H
#define DSLPNLCONVERTER_H

#include "pnl_dll.hpp"
#include "network.h"

//#define DSLPNL_DEBUG

PNL_USING

class DSLPNLConverter  
{
private:

    DSL_idArray theIds; // Stores mapping of DSL_nodeIds, indexed by node numbers
    
    CGraph*      CreateCGraph(DSL_network& dslNet);
    CBNet*       CreateCBNet(DSL_network& dslNet, CGraph* pnlGraph);
    void         CreateFactors(DSL_network& dslNet, CBNet* pnlNet);
    
public:

    CBNet*       CreateBNet(DSL_network &dslNet);
    DSL_idArray& GetMapping() {return theIds;}
    CEvidence*   CreateEvidence(DSL_network& dslNet, CBNet* pnlNet);
    
    DSLPNLConverter();
    virtual ~DSLPNLConverter();
};

#endif // DSLPNLCONVERTER_H
