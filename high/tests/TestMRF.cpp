#include "pnlHigh.hpp"
#include "MRF.hpp"   

PNLW_USING


int testMRF()
{
    int result = -1;
    // MRF creation
    MRF net; 

    net.AddNode("discrete^X", "true false"); 
    net.AddNode("discrete^Y", "true false");
    net.AddNode("discrete^Z", "true false");
    net.AddNode("discrete^W", "true false");

    net.SetClique("X Y");
    net.SetClique("Y Z");
    net.SetClique("Z W");
    net.SetClique("X W");

    net.SetPTabular("X^true Y^true", "0.1");
    net.SetPTabular("X^true Y^false", "0.3"); 
    net.SetPTabular("X^false Y^true", "0.25");
    net.SetPTabular("X^false Y^false", "0.35"); 

    TokArr XY = net.GetPTabular("X Y");
    TokArr XtrueY = net.GetPTabular("X^true Y");
    TokArr XtrueYfalse = net.GetPTabular("X^true Y^false");

    printf("%s\n",String(XY).c_str());
    printf("%s\n",String(XtrueY).c_str());
    printf("%s\n",String(XtrueYfalse).c_str());

    return result;
} 