#include "test_conf.hpp"
#include "pnlException.hpp"
#include "pnl_dll.hpp"

PNLW_USING
using namespace std;

void OilTest()
{
    LIMID *net;

    net = new LIMID();
    TokArr aChoice = "False True";// possible values for nodes
    TokArr aCh = "False True MayBe";// possible values for nodes
    
    TokArr aIncome = "Cost";// possible values for nodes   

    net->AddNode(chance ^"h1", aCh);
    net->AddNode(decision^"d1", aChoice);
    net->AddNode(chance ^ "h2", aCh);
    net->AddNode(decision^"d2", aChoice);

    net->AddNode(value^"u1", aIncome);
    net->AddNode(value^"u2", aIncome);

    net->AddArc("h1", "h2");
    net->AddArc("h1", "u1");
    net->AddArc("d1", "u2");
    net->AddArc("d1", "h2");
    net->AddArc("d2", "u1");
    net->AddArc("h2", "d2");

    net->SetPChance("h1^False h1^True h1^MayBe", "0.5 0.3 0.2");

    net->SetPDecision("d1^False d1^True", "0.5 0.5");

    net->SetPChance("h2^False h2^True h2^MayBe", "0.1 0.3 0.6", "h1^False d1^False");
    net->SetPChance("h2^False h2^True h2^MayBe", "0.333333 0.333333 0.333333", "h1^False d1^True");
    net->SetPChance("h2^False h2^True h2^MayBe", "0.3 0.4 0.3", "h1^True d1^False");
    net->SetPChance("h2^False h2^True h2^MayBe", "0.333333 0.333333 0.333333", "h1^True d1^True");
    net->SetPChance("h2^False h2^True h2^MayBe", "0.5 0.4 0.1", "h1^MayBe d1^False");
    net->SetPChance("h2^False h2^True h2^MayBe", "0.333333 0.333333 0.333333", "h1^MayBe d1^True");
    
    net->SetPDecision("d2^False d2^True", "0.5 0.5", "h2^False");
    net->SetPDecision("d2^False d2^True", "0.5 0.5", "h2^True");
    net->SetPDecision("d2^False d2^True", "0.5 0.5", "h2^MayBe");
 
    net->SetValueCost("u1^Cost", "-70000.0", "h1^False d2^False");
    net->SetValueCost("u1^Cost", "0.0", "h1^False d2^True");
    net->SetValueCost("u1^Cost", "50000.0", "h1^True d2^False");
    net->SetValueCost("u1^Cost", "0.0", "h1^True d2^True");
    net->SetValueCost("u1^Cost", "200000.0", "h1^MayBe d2^False");
    net->SetValueCost("u1^Cost", "0.0", "h1^MayBe d2^True");

    net->SetValueCost("u2^Cost", "-10000.0", "d1^False");
    net->SetValueCost("u2^Cost", "0.0", "d1^True");

    TokArr exp;
    exp = net->GetExpectation();
    if ( exp[0].FltValue() - 20000.0019 > 1e-3f)
    {
        PNL_THROW(pnl::CAlgorithmicException, "Expectation is wrong");
    }

    TokArr politics;
    politics = net->GetPolitics();
    cout << politics;
    cout <<"\n";

    delete net;
    cout << "LIMID OIL is completed successfully" << endl;
}

void TestPigs()
{
    LIMID *net;

    net = new LIMID();
    TokArr aChoice = "False True";// possible values for nodes
    
    TokArr aIncome = "Cost";// possible values for nodes

    net->AddNode(chance^"h1", aChoice);
    net->AddNode(chance^"t1", aChoice);
    net->AddNode(decision^"d1", aChoice);

    net->AddNode(chance^"h2", aChoice);
    net->AddNode(chance^"t2", aChoice);
    net->AddNode(decision^"d2", aChoice);

    net->AddNode(chance^"h3", aChoice);
    net->AddNode(chance^"t3", aChoice);
    net->AddNode(decision^"d3", aChoice);

    net->AddNode(chance^"h4", aChoice);

    net->AddNode(value^"u1", aIncome);
    net->AddNode(value^"u2", aIncome);
    net->AddNode(value^"u3", aIncome);
    net->AddNode(value^"u4", aIncome);
    
    net->AddArc("h1", "h2");
    net->AddArc("h1", "t1");
    net->AddArc("h2", "t2");
    net->AddArc("t1", "d1");
    net->AddArc("t2", "d2");
    net->AddArc("d1", "h2");
    net->AddArc("h3", "t3");
    net->AddArc("t3", "d3");
    net->AddArc("d2", "h3");
    net->AddArc("h2", "h3");
    net->AddArc("h3", "h4");
    net->AddArc("d3", "h4");
    net->AddArc("d1", "u1");
    net->AddArc("d2", "u2");
    net->AddArc("d3", "u3");
    net->AddArc("h4", "u4");

    net->SetPChance("h1^False h1^True", "0.9 0.1");

    net->SetPChance("t1^False t1^True", "0.1 0.9", "h1^False");
    net->SetPChance("t1^False t1^True", "0.8 0.2", "h1^True");

    net->SetPDecision("d1^False d1^True", "0.5 0.5", "t1^False");
    net->SetPDecision("d1^False d1^True", "0.5 0.5", "t1^True");

    net->SetValueCost("u1^Cost", "-100.0", "d1^False");
    net->SetValueCost("u1^Cost", "0.0", "d1^True");

    net->SetPChance("h2^False h2^True", "0.9 0.1", "h1^False d1^False");
    net->SetPChance("h2^False h2^True", "0.8 0.2", "h1^False d1^True");

    net->SetPChance("h2^False h2^True", "0.5 0.5", "h1^True d1^False");
    net->SetPChance("h2^False h2^True", "0.1 0.9", "h1^True d1^True");

    net->SetPChance("t2^False t2^True", "0.1 0.9", "h2^False");
    net->SetPChance("t2^False t2^True", "0.8 0.2", "h2^True");

    net->SetPDecision("d2^False d2^True", "0.5 0.5", "t2^False");
    net->SetPDecision("d2^False d2^True", "0.5 0.5", "t2^True");

    net->SetValueCost("u2^Cost", "-100.0", "d2^False");
    net->SetValueCost("u2^Cost", "0.0", "d2^True");

    net->SetPChance("h3^False h3^True", "0.9 0.1", "h2^False d2^False");
    net->SetPChance("h3^False h3^True", "0.5 0.5", "h2^False d2^True");

    net->SetPChance("h3^False h3^True", "0.8 0.2", "h2^True d2^False");
    net->SetPChance("h3^False h3^True", "0.1 0.9", "h2^True d2^True");

    net->SetPChance("t3^False t3^True", "0.1 0.9", "h3^False");
    net->SetPChance("t3^False t3^True", "0.8 0.2", "h3^True");

    net->SetPDecision("d3^False d3^True", "0.5 0.5", "t3^False");
    net->SetPDecision("d3^False d3^True", "0.5 0.5", "t3^True");

    net->SetValueCost("u3^Cost", "-100.0", "d3^False");
    net->SetValueCost("u3^Cost", "0.0", "d3^True");

    net->SetPChance("h4^False h4^True", "0.9 0.1", "h3^False d3^False");
    net->SetPChance("h4^False h4^True", "0.8 0.2", "h3^False d3^True");

    net->SetPChance("h4^False h4^True", "0.5 0.5", "h3^True d3^False");
    net->SetPChance("h4^False h4^True", "0.1 0.9", "h3^True d3^True");

    net->SetValueCost("u4^Cost", "1000.0", "h4^False");
    net->SetValueCost("u4^Cost", "300.0", "h4^True");

/*    net->SaveNet("pigs.xml");

    LIMID *newNet;
    newNet = new LIMID();

    newNet->LoadNet("pigs.xml");

*/
    TokArr exp;
    exp = net->GetExpectation();
    if ( exp[0].FltValue() - 726.812073 > 1e-3f)
    {
        PNL_THROW(pnl::CAlgorithmicException, "Expectation is wrong");
    }

    TokArr politics;
    politics = net->GetPolitics();
    cout << politics;
    cout <<"\n";

    delete net;
    cout << "LIMID PIGS is completed successfully" << endl;
}

void testRandom1()
{
    // wrapper works
    int i;
    LIMID *net;
    net = new LIMID();

    net->LoadNet("random15.xml");
    
    TokArr exp;
    exp = net->GetExpectation();

    TokArr politics;
    politics = net->GetPolitics();
    delete net;

    pnl::CIDNet *pIDNet = dynamic_cast<pnl::CIDNet*>(LoadGrModelFromXML("random15.xml", NULL));
    pnl::CLIMIDInfEngine *pInfEng = NULL;
    pInfEng = pnl::CLIMIDInfEngine::Create(pIDNet);
  
    pInfEng->DoInference();
    
    pnl::pFactorVector *Vec = pInfEng->GetPolitics();
    float res = pInfEng->GetExpectation();
    
    if (exp[0].FltValue() != res)
    {
       PNL_THROW(pnl::CAlgorithmicException, "Expectation is wrong");
    }
    
    if (Vec->size() != politics.size())
    {
           PNL_THROW(pnl::CAlgorithmicException, "Number of politics is wrong");
    }

    for (i = 0; i < Vec->size(); i++)
    {
        pnl::CMatrix<float> *mat = (*Vec)[i]->GetDistribFun()->GetMatrix(pnl::matTable);
        pnl::CDenseMatrix<float> *dMat = mat->ConvertToDense();
        const pnl::floatVector *matVec = dMat->GetVector();
        for (int j = 0; j< matVec->size(); j++ )
        {
           if ((*matVec)[j] != politics[i].fload[j].fl) 
           {
                PNL_THROW(pnl::CAlgorithmicException, "politics are wrong");
           };
        }
        
    }
    pnl::CLIMIDInfEngine::Release(&pInfEng);
    delete pIDNet;
    cout << "LIMID testRandom1 is completed successfully" << endl;

}
void testRandom2()
{
    // wrapper works
    int i;
    LIMID *net;
    net = new LIMID();

    net->LoadNet("random20.xml");
    
    TokArr exp;
    exp = net->GetExpectation();

    TokArr politics;
    politics = net->GetPolitics();
    delete net;

    // PNL works

    pnl::CIDNet *pIDNet = dynamic_cast<pnl::CIDNet*>(LoadGrModelFromXML("random20.xml", NULL));
    
    pnl::CLIMIDInfEngine *pInfEng = NULL;
    pInfEng = pnl::CLIMIDInfEngine::Create(pIDNet);
  
    pInfEng->DoInference();
    
    pnl::pFactorVector *Vec = pInfEng->GetPolitics();
    float res = pInfEng->GetExpectation();

    if (exp[0].FltValue() != res)
    {
       PNL_THROW(pnl::CAlgorithmicException, "Expectation is wrong");
    }
    
    if (Vec->size() != politics.size())
    {
           PNL_THROW(pnl::CAlgorithmicException, "Number of politics is wrong");
    }

    for (i = 0; i < Vec->size(); i++)
    {
        pnl::CMatrix<float> *mat = (*Vec)[i]->GetDistribFun()->GetMatrix(pnl::matTable);
        pnl::CDenseMatrix<float> *dMat = mat->ConvertToDense();
        const pnl::floatVector *matVec = dMat->GetVector();
        for (int j = 0; j< matVec->size(); j++ )
        {
           if ((*matVec)[j] != politics[i].fload[j].fl) 
           {
                PNL_THROW(pnl::CAlgorithmicException, "politics are wrong");
           };
        }
        
    }
 
    pnl::CLIMIDInfEngine::Release(&pInfEng);
    delete pIDNet;
    cout << "LIMID testRandom2 is completed successfully" << endl;

}
void testRandom3()
{
    // wrapper works
    int i;
    LIMID *net;
    net = new LIMID();

    net->LoadNet("random25.xml");
    
    TokArr exp;
    exp = net->GetExpectation();

    TokArr politics;
    politics = net->GetPolitics();
    delete net;

    // PNL works

    pnl::CIDNet *pIDNet = dynamic_cast<pnl::CIDNet*>(LoadGrModelFromXML("random25.xml", NULL));
    
    pnl::CLIMIDInfEngine *pInfEng = NULL;
    pInfEng = pnl::CLIMIDInfEngine::Create(pIDNet);
  
    pInfEng->DoInference();
    
    pnl::pFactorVector *Vec = pInfEng->GetPolitics();
    float res = pInfEng->GetExpectation();

    if (exp[0].FltValue() != res)
    {
       PNL_THROW(pnl::CAlgorithmicException, "Expectation is wrong");
    }
    
    if (Vec->size() != politics.size())
    {
           PNL_THROW(pnl::CAlgorithmicException, "Number of politics is wrong");
    }

    for (i = 0; i < Vec->size(); i++)
    {
        pnl::CMatrix<float> *mat = (*Vec)[i]->GetDistribFun()->GetMatrix(pnl::matTable);
        pnl::CDenseMatrix<float> *dMat = mat->ConvertToDense();
        const pnl::floatVector *matVec = dMat->GetVector();
        for (int j = 0; j< matVec->size(); j++ )
        {
           if ((*matVec)[j] != politics[i].fload[j].fl) 
           {
                PNL_THROW(pnl::CAlgorithmicException, "politics are wrong");
           };
        }
    }
     
    pnl::CLIMIDInfEngine::Release(&pInfEng);
    delete pIDNet;
    
    cout << "LIMID testRandom3 is completed successfully" << endl;
}


void DelNodes()
{
    LIMID *net;

    net = new LIMID();
    TokArr aChoice = "False True";// possible values for nodes
    TokArr aCh = "False True MayBe";// possible values for nodes
    
    TokArr aIncome = "Cost";// possible values for nodes   

    net->AddNode(chance ^"h1", aCh);
    net->AddNode(decision^"d1", aChoice);
    net->AddNode(chance ^ "h2", aCh);
    net->AddNode(decision^"d2", aChoice);

    net->AddNode(value^"u1", aIncome);
    net->AddNode(value^"u2", aIncome);

    net->AddArc("h1", "h2");
    net->AddArc("h1", "u1");
    net->AddArc("d1", "u2");
    net->AddArc("d1", "h2");
    net->AddArc("d2", "u1");
    net->AddArc("h2", "d2");

    net->SetPChance("h1^False h1^True h1^MayBe", "0.5 0.3 0.2");

    net->SetPDecision("d1^False d1^True", "0.5 0.5");

    net->SetPChance("h2^False h2^True h2^MayBe", "0.1 0.3 0.6", "h1^False d1^False");
    net->SetPChance("h2^False h2^True h2^MayBe", "0.333333 0.333333 0.333333", "h1^False d1^True");
    net->SetPChance("h2^False h2^True h2^MayBe", "0.3 0.4 0.3", "h1^True d1^False");
    net->SetPChance("h2^False h2^True h2^MayBe", "0.333333 0.333333 0.333333", "h1^True d1^True");
    net->SetPChance("h2^False h2^True h2^MayBe", "0.5 0.4 0.1", "h1^MayBe d1^False");
    net->SetPChance("h2^False h2^True h2^MayBe", "0.333333 0.333333 0.333333", "h1^MayBe d1^True");
    
    net->SetPDecision("d2^False d2^True", "0.5 0.5", "h2^False");
    net->SetPDecision("d2^False d2^True", "0.5 0.5", "h2^True");
    net->SetPDecision("d2^False d2^True", "0.5 0.5", "h2^MayBe");
 
    net->SetValueCost("u1^Cost", "-70000.0", "h1^False d2^False");
    net->SetValueCost("u1^Cost", "0.0", "h1^False d2^True");
    net->SetValueCost("u1^Cost", "50000.0", "h1^True d2^False");
    net->SetValueCost("u1^Cost", "0.0", "h1^True d2^True");
    net->SetValueCost("u1^Cost", "200000.0", "h1^MayBe d2^False");
    net->SetValueCost("u1^Cost", "0.0", "h1^MayBe d2^True");

    net->SetValueCost("u2^Cost", "-10000.0", "d1^False");
    net->SetValueCost("u2^Cost", "0.0", "d1^True");

    TokArr h2 = net->GetPChance("h2");
    TokArr d2 = net->GetPDecision("d2");
    TokArr u2 = net->GetValueCost("u2");

    if ((h2[0].FltValue() != 0.1f)||(h2[1].FltValue() != 0.3f) || (h2[2].FltValue() != 0.6f) )
    {
       PNL_THROW(pnl::CAlgorithmicException, "Error get");
    }
    if ((d2[0].FltValue() != 0.5f)||(d2[1].FltValue() != 0.5f) )
    {
       PNL_THROW(pnl::CAlgorithmicException, "Error get");
    }
    if ((u2[0].FltValue() != -10000.0f)||(u2[1].FltValue() != 0.0f)  )
    {
       PNL_THROW(pnl::CAlgorithmicException, "Error get");
    }

    net->DelNode("u2");

    TokArr exp;
    exp = net->GetExpectation();
    cout << exp<< "\n";

    TokArr politics;
    politics = net->GetPolitics();
    cout << politics << "\n";

    net->AddNode(value^"u2", aIncome);
    net->AddArc("d1", "u2");
    net->DelArc("h2", "d2");

    TokArr exp1;
    exp1 = net->GetExpectation();
    cout << exp1<< "\n";

    TokArr politics1;
    politics1 = net->GetPolitics();
    cout << politics1<< "\n";


    delete net;
    cout << "DelNodes is completed successfully" << endl;
}

void LimidTopology()
{
    LIMID *net;

    net = new LIMID();
    TokArr aChoice = "False True";// possible values for nodes
    TokArr aCh = "False True MayBe";// possible values for nodes
    
    TokArr aIncome = "Cost";// possible values for nodes   

    net->AddNode(decision^"d1", aChoice);
    net->AddNode(value^"u1", aIncome);
    net->AddArc("d1", "u1");

    net->AddNode(chance ^"h1", aCh);
    net->AddNode(chance ^ "h2", aCh);
    net->AddArc("h1", "h2");
 
    net->AddArc("h2", "d1");

    net->SetPChance("h1^False h1^True h1^MayBe", "0.1 0.3 0.6");

    net->SetPChance("h2^False h2^True h2^MayBe", "0.333333 0.333333 0.333333", "h1^False");
    net->SetPChance("h2^False h2^True h2^MayBe", "0.3 0.4 0.3", "h1^True");
    net->SetPChance("h2^False h2^True h2^MayBe", "0.5 0.4 0.1", "h1^MayBe");

    net->SetPDecision("d1^False d1^True", "0.5 0.5", "h2^False");
    net->SetPDecision("d1^False d1^True", "0.5 0.5", "h2^True");
    net->SetPDecision("d1^False d1^True", "0.5 0.5", "h2^MayBe");

    net->SetValueCost("u1^Cost", "-10000.0", "d1^False");
    net->SetValueCost("u1^Cost", "20.0", "d1^True");

    TokArr exp;
    exp = net->GetExpectation();
    cout << exp<< "\n";

    delete net;
    cout << "Test LimidTopology is completed successfully" << endl;
}


void PureLimidModel1()
{
    LIMID *net;

    net = new LIMID();
    TokArr aChoice = "False True";// possible values for nodes
    TokArr aCh = "False True MayBe";// possible values for nodes
    
    TokArr aIncome = "Cost";// possible values for nodes   

    net->AddNode(chance ^"h1", aCh);
    net->AddNode(chance ^ "h2", aCh);
    net->AddNode(value^"u1", aIncome);

    net->AddArc("h1", "h2");
    net->AddArc("h2", "u1");

    net->SetPChance("h1^False h1^True h1^MayBe", "0.1 0.3 0.6");

    net->SetPChance("h2^False h2^True h2^MayBe", "0.333333 0.333333 0.333333", "h1^False");
    net->SetPChance("h2^False h2^True h2^MayBe", "0.3 0.4 0.3", "h1^True");
    net->SetPChance("h2^False h2^True h2^MayBe", "0.5 0.4 0.1", "h1^MayBe");

    net->SetValueCost("u1^Cost", "-10000.0", "h2^False");
    net->SetValueCost("u1^Cost", "0.0", "h2^True");
    net->SetValueCost("u1^Cost", "500.0", "h2^MayBe");

    TokArr exp;
    exp = net->GetExpectation();
    cout << exp<< "\n";

    TokArr politics1;
    politics1 = net->GetPolitics();
    cout << politics1<< "\n";

    delete net;
    cout << "Test PureLimidModel without decision nodes is completed successfully" << endl;
}

void PureLimidModel2()
{
    LIMID *net;

    net = new LIMID();
    TokArr aChoice = "False True";// possible values for nodes
    TokArr aCh = "False True MayBe";// possible values for nodes
    
    TokArr aIncome = "Cost";// possible values for nodes   

    net->AddNode(chance ^"h1", aCh);
    net->AddNode(chance ^ "h2", aCh);
    net->AddNode(decision^"d1", aChoice);

    net->AddArc("h1", "h2");
    net->AddArc("h2", "d1");

    net->SetPChance("h1^False h1^True h1^MayBe", "0.1 0.3 0.6");

    net->SetPChance("h2^False h2^True h2^MayBe", "0.333333 0.333333 0.333333", "h1^False");
    net->SetPChance("h2^False h2^True h2^MayBe", "0.3 0.4 0.3", "h1^True");
    net->SetPChance("h2^False h2^True h2^MayBe", "0.5 0.4 0.1", "h1^MayBe");

    net->SetPDecision("d1^False d1^True", "0.5 0.5", "h2^False");
    net->SetPDecision("d1^False d1^True", "0.5 0.5", "h2^True");
    net->SetPDecision("d1^False d1^True", "0.5 0.5", "h2^MayBe");

    TokArr exp;
    exp = net->GetExpectation();
    cout << exp<< "\n";

    TokArr politics1;
    politics1 = net->GetPolitics();
    cout << politics1<< "\n";

    delete net;
    cout << "Test PureLimidModel without value nodes is completed successfully" << endl;
}

void PureLimidModel3()
{
    LIMID *net;

    net = new LIMID();
    TokArr aChoice = "False True";// possible values for nodes
    TokArr aCh = "False True MayBe";// possible values for nodes
    
    TokArr aIncome = "Cost";// possible values for nodes   

    net->AddNode(decision^"d1", aChoice);
    net->AddNode(value^"u1", aIncome);

    net->AddArc("d1", "u1");

    net->SetPDecision("d1^False d1^True", "0.5 0.5");

    net->SetValueCost("u1^Cost", "-10000.0", "d1^False");
    net->SetValueCost("u1^Cost", "0.0", "d1^True");

    TokArr exp;
    exp = net->GetExpectation();
    cout << exp<< "\n";

    TokArr politics1;
    politics1 = net->GetPolitics();
    cout << politics1<< "\n";

    delete net;
    cout << "Test PureLimidModel without chance nodes is completed successfully" << endl;
}