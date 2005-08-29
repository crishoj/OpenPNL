#include "test_conf.hpp"
#include "pnlException.hpp"
#include "pnl_dll.hpp"


PNLW_USING
using namespace std;

static char func_name[] = "testLIMID";

static char* test_desc = "Provide some tests for LIMID";

static char* test_class = "Algorithm";

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

    net->SaveNet("pigs.xml");

    LIMID *newNet;
    newNet = new LIMID();

    newNet->LoadNet("pigs.xml");


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

	delete newNet;
    delete net;
    cout << "LIMID PIGS is completed successfully" << endl;
}

void testSaveLoadLIMID()
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

    net->SaveNet("pigs.xml");
 
	

    LIMID *newNet;
    newNet = new LIMID();

    newNet->LoadNet("pigs.xml");


    TokArr exp;
    exp = newNet->GetExpectation();
    if ( exp[0].FltValue() - 726.812073 > 1e-3f)
    {
        PNL_THROW(pnl::CAlgorithmicException, "Expectation is wrong");
    }

    TokArr politics;
    politics = newNet->GetPolitics();
    cout << politics;
    cout <<"\n";

	delete net;
	delete newNet;
}
void testPNLObjectsRequestsLIMID()
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
	
	pnl::CIDNet& pnlNet = net->Model();
	pnl::CLIMIDInfEngine& pnlInf = net->Inference();

	delete net;
}

void testProperties()
{
    LIMID net;
	//Adding new network property
	net.SetProperty("date","20july2005");
	//Network proprty value request
	String value = net.GetProperty("date");
	printf("\n%s\n",value.c_str());
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

    net->AddNode(chance ^"hhh1", aCh);
    net->AddNode(decision^"ddd1", aChoice);
    net->AddNode(chance ^ "hhh2", aCh);
    net->AddNode(decision^"ddd2", aChoice);

    net->AddNode(value^"uuu1", aIncome);
    net->AddNode(value^"uuu2", aIncome);

    net->AddArc("hhh1", "hhh2");
    net->AddArc("hhh1", "uuu1");
    net->AddArc("ddd1", "uuu2");
    net->AddArc("ddd1", "hhh2");
    net->AddArc("ddd2", "uuu1");
    net->AddArc("hhh2", "ddd2");

    net->SetPChance("hhh1^False hhh1^True hhh1^MayBe", "0.5 0.3 0.2");

    net->SetPDecision("ddd1^False ddd1^True", "0.5 0.5");

    net->SetPChance("hhh2^False hhh2^True hhh2^MayBe", "0.1 0.3 0.6", "hhh1^False ddd1^False");
    net->SetPChance("hhh2^False hhh2^True hhh2^MayBe", "0.333333 0.333333 0.333333", "hhh1^False ddd1^True");
    net->SetPChance("hhh2^False hhh2^True hhh2^MayBe", "0.3 0.4 0.3", "hhh1^True ddd1^False");
    net->SetPChance("hhh2^False hhh2^True hhh2^MayBe", "0.333333 0.333333 0.333333", "hhh1^True ddd1^True");
    net->SetPChance("hhh2^False hhh2^True hhh2^MayBe", "0.5 0.4 0.1", "hhh1^MayBe ddd1^False");
    net->SetPChance("hhh2^False hhh2^True hhh2^MayBe", "0.333333 0.333333 0.333333", "hhh1^MayBe ddd1^True");
    
    net->SetPDecision("ddd2^False ddd2^True", "0.5 0.5", "hhh2^False");
    net->SetPDecision("ddd2^False ddd2^True", "0.5 0.5", "hhh2^True");
    net->SetPDecision("ddd2^False ddd2^True", "0.5 0.5", "hhh2^MayBe");
 
    net->SetValueCost("uuu1^Cost", "-70000.0", "hhh1^False ddd2^False");
    net->SetValueCost("uuu1^Cost", "0.0", "hhh1^False ddd2^True");
    net->SetValueCost("uuu1^Cost", "50000.0", "hhh1^True ddd2^False");
    net->SetValueCost("uuu1^Cost", "0.0", "hhh1^True ddd2^True");
    net->SetValueCost("uuu1^Cost", "200000.0", "hhh1^MayBe ddd2^False");
    net->SetValueCost("uuu1^Cost", "0.0", "hhh1^MayBe ddd2^True");

    net->SetValueCost("uuu2^Cost", "-10000.0", "ddd1^False");
    net->SetValueCost("uuu2^Cost", "0.0", "ddd1^True");

    TokArr h2 = net->GetPChance("hhh2");
    TokArr d2 = net->GetPDecision("ddd2");
    TokArr u2 = net->GetValueCost("uuu2");

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

    net->DelNode("uuu2");

    TokArr exp;
    exp = net->GetExpectation();
    cout << exp<< "\n";

    TokArr politics;
    politics = net->GetPolitics();
    cout << politics << "\n";

    net->AddNode(value^"uuu2", aIncome);
    net->AddArc("ddd1", "uuu2");
    net->DelArc("hhh2", "ddd2");

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

    net->AddNode(decision^"d01", aChoice);
    net->AddNode(value^"u01", aIncome);
    net->AddArc("d01", "u01");

    net->AddNode(chance ^"h01", aCh);
    net->AddNode(chance ^ "h02", aCh);
    net->AddArc("h01", "h02");
 
    net->AddArc("h02", "d01");

    net->SetPChance("h01^False h01^True h01^MayBe", "0.1 0.3 0.6");

    net->SetPChance("h02^False h02^True h02^MayBe", "0.333333 0.333333 0.333333", "h01^False");
    net->SetPChance("h02^False h02^True h02^MayBe", "0.3 0.4 0.3", "h01^True");
    net->SetPChance("h02^False h02^True h02^MayBe", "0.5 0.4 0.1", "h01^MayBe");

    net->SetPDecision("d01^False d01^True", "0.5 0.5", "h02^False");
    net->SetPDecision("d01^False d01^True", "0.5 0.5", "h02^True");
    net->SetPDecision("d01^False d01^True", "0.5 0.5", "h02^MayBe");

    net->SetValueCost("u01^Cost", "-10000.0", "d01^False");
    net->SetValueCost("u01^Cost", "20.0", "d01^True");

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

    net->AddNode(chance ^"hh1", aCh);
    net->AddNode(chance ^ "hh2", aCh);
    net->AddNode(value^"uu1", aIncome);

    net->AddArc("hh1", "hh2");
    net->AddArc("hh2", "uu1");

    net->SetPChance("hh1^False hh1^True hh1^MayBe", "0.1 0.3 0.6");

    net->SetPChance("hh2^False hh2^True hh2^MayBe", "0.333333 0.333333 0.333333", "hh1^False");
    net->SetPChance("hh2^False hh2^True hh2^MayBe", "0.3 0.4 0.3", "hh1^True");
    net->SetPChance("hh2^False hh2^True hh2^MayBe", "0.5 0.4 0.1", "hh1^MayBe");

    net->SetValueCost("uu1^Cost", "-10000.0", "hh2^False");
    net->SetValueCost("uu1^Cost", "0.0", "hh2^True");
    net->SetValueCost("uu1^Cost", "500.0", "hh2^MayBe");

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

    net->AddNode(chance ^"h11", aCh);
    net->AddNode(chance ^ "h21", aCh);
    net->AddNode(decision^"d11", aChoice);

    net->AddArc("h11", "h21");
    net->AddArc("h21", "d11");

    net->SetPChance("h11^False h11^True h11^MayBe", "0.1 0.3 0.6");

    net->SetPChance("h21^False h21^True h21^MayBe", "0.333333 0.333333 0.333333", "h11^False");
    net->SetPChance("h21^False h21^True h21^MayBe", "0.3 0.4 0.3", "h11^True");
    net->SetPChance("h21^False h21^True h21^MayBe", "0.5 0.4 0.1", "h11^MayBe");

    net->SetPDecision("d11^False d11^True", "0.5 0.5", "h21^False");
    net->SetPDecision("d11^False d11^True", "0.5 0.5", "h21^True");
    net->SetPDecision("d11^False d11^True", "0.5 0.5", "h21^MayBe");

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

    net->AddNode(decision^"d31", aChoice);
    net->AddNode(value^"u31", aIncome);

    net->AddArc("d31", "u31");

    net->SetPDecision("d31^False d31^True", "0.5 0.5");

    net->SetValueCost("u31^Cost", "-10000.0", "d31^False");
    net->SetValueCost("u31^Cost", "0.0", "d31^True");

    TokArr exp;
    exp = net->GetExpectation();
    cout << exp<< "\n";

    TokArr politics1;
    politics1 = net->GetPolitics();
    cout << politics1<< "\n";

    delete net;
    cout << "Test PureLimidModel without chance nodes is completed successfully" << endl;
}

int testLIMID()
{
	bool bTestOK = TRS_OK;
	bool lastTestRes = TRS_OK;

    
	try 
    {
        lastTestRes = false;
        PureLimidModel1();
    }
    catch(pnl::CException e)
    {
        static const char messageOk[] = "The Influence Diagram hasn't decision nodes.";
        
        if((e.GetCode() == pgmAlgorithmic)
            && strstr(e.GetMessage(), messageOk))
        {
            lastTestRes = true;// this exception must be produced
        }
        else
        {
            std::cout << e.GetMessage()<< "\n";
        }
    }
    bTestOK = bTestOK && lastTestRes;
    
    try 
    {
        PureLimidModel2();
    }
    catch(pnl::CException e)
    {
        static const char messageOk[] = "The Influence Diagram hasn't value nodes.";
        
        if((e.GetCode() == pgmAlgorithmic)
            && strstr(e.GetMessage(), messageOk))
        {
            lastTestRes = true;// this exception must be produced
        }
        else
        {
            std::cout << e.GetMessage()<< "\n";
        }
    }
    bTestOK = bTestOK && lastTestRes;
    
    try 
    {
        PureLimidModel3();
    }
    catch(pnl::CException e)
    {
        static const char messageOk[] = "The Influence Diagram hasn't chance nodes.";
        
        if((e.GetCode() == pgmAlgorithmic)
            && strstr(e.GetMessage(), messageOk))
        {
            lastTestRes = true;// this exception must be produced
        }
        else
        {
            std::cout << e.GetMessage()<< "\n";
        }
    }
    bTestOK = bTestOK && lastTestRes;
    
    try 
    {
        LimidTopology();
        DelNodes();
//        TestPigs();           // does not work under Linux
//		testSaveLoadLIMID();    // does not work under Linux
        OilTest();
        testRandom1();
        testRandom2();
        testRandom3();
		testProperties();
    	testPNLObjectsRequestsLIMID();

    }
    catch(pnl::CException e)
    {
        std::cout << e.GetMessage();
        bTestOK = TRS_FAIL;
    }
    
	return bTestOK;
}

void initTestsLIMID()
{
    trsReg(func_name, test_desc, test_class, testLIMID);
}
