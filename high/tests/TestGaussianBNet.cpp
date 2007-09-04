#include "test_conf.hpp"
#include "pnlException.hpp"
#include "pnl_dll.hpp"


PNLW_USING
using namespace std;



static char func_name[] = "testGaussianBNet";

static char* test_desc = "Provide some tests of Gaussian network";

static char* test_class = "Algorithm";

BayesNet *VerySimpleModel()
{
    // NodeA -> NodeB -> NodeC
    BayesNet *net;
    net = new BayesNet();

    net->AddNode(continuous^"NodeB NodeA NodeC", "dim1");
//    net->AddNode(continuous^"NodeA NodeB NodeC", "dim1");
    net->AddArc("NodeA", "NodeB");
    net->AddArc("NodeB", "NodeC");

    net->SetPGaussian("NodeA", "1.0", "10.0");
    net->SetPGaussian("NodeB", "0.5", "7.0", "0.1");
    net->SetPGaussian("NodeC", "0.8", "3.5", "0.4");

    return net;
}

void TestGaussianModelCreate(bool DeleteNet)
{
    BayesNet *net = VerySimpleModel();

    if( net->GetGaussianMean("NodeA")[0].FltValue() != 1.0f ||
        net->GetGaussianCovar("NodeA")[0].FltValue() != 10.0f ||
        net->GetGaussianMean("NodeB")[0].FltValue() != 0.5f ||
        net->GetGaussianCovar("NodeB")[0].FltValue() != 7.0f ||
        net->GetGaussianWeights("NodeB", "NodeA")[0].FltValue() != 0.1f ||
        net->GetGaussianMean("NodeC")[0].FltValue() != 0.8f ||
        net->GetGaussianCovar("NodeC")[0].FltValue() != 3.5f ||
        net->GetGaussianWeights("NodeC", "NodeB")[0].FltValue() != 0.4f )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Setting or getting gaussian parameters is wrong");
    }

    net->DelArc("NodeB", "NodeC");
    net->AddArc("NodeC", "NodeA");

    net->SetPGaussian("NodeA", "0.6", "12.0", "0.35");

    if( net->GetGaussianMean("NodeA")[0].FltValue() != 0.6f ||
        net->GetGaussianCovar("NodeA")[0].FltValue() != 12.0f ||
        net->GetGaussianWeights("NodeA", "NodeC")[0].FltValue() != 0.35f )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Setting or getting gaussian parameters is wrong");
    }

    net->AddNode(continuous ^ "NodeD", "dim1");// trvs - previous version - discrete node
    net->AddArc("NodeC", "NodeD");
    net->AddArc("NodeD", "NodeB");// trvs - previous version: net->AddArc("NodeD", "NodeA");

    net->SetPGaussian("NodeB", "0.12", "3.0", "0.21 0.9");

    if( net->GetGaussianMean("NodeB")[0].FltValue() != 0.12f ||
        net->GetGaussianCovar("NodeB")[0].FltValue() != 3.0f)
    {
        PNL_THROW(pnl::CAlgorithmicException, "Setting or getting gaussian parameters is wrong");
    }
    float ba = net->GetGaussianWeights("NodeB", "NodeA")[0].FltValue();
    float bd = net->GetGaussianWeights("NodeB", "NodeD")[0].FltValue();
    
    if((ba != 0.21f || bd != 0.9f) && (ba != 0.9f || bd != 0.21f))
    {
        PNL_THROW(pnl::CAlgorithmicException, "Setting or getting gaussian parameters is wrong");
    }

    net->DelNode("NodeA");

    if( net->GetGaussianMean("NodeB")[0].FltValue() != 1.0f ||
        net->GetGaussianCovar("NodeB")[0].FltValue() != 1.0f ||
        net->GetGaussianWeights("NodeB", "NodeD")[0].FltValue() != 0.0f )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Setting or getting gaussian parameters is wrong");
    }


    if (DeleteNet)
    {
	delete net;
    };

    cout << "TestGaussianModelCreate is completed successfully" << endl;
}

BayesNet *PolytreeModel()
{
    // NodeA  NodeB 
    //     \ /
    //    NodeC
    //     / \
    // NodeD  NodeE

    BayesNet *net;
    net = new BayesNet();

    //net->AddNode(continuous^"NodeA NodeC NodeE", "dim1 dim2");
    //net->AddNode(continuous^"NodeB NodeD", "dim1");
    net->AddNode(continuous^"NodeA", "dim1 dim2");
    net->AddNode(continuous^"NodeB", "dim1");
    net->AddNode(continuous^"NodeC", "dim1 dim2");
    net->AddNode(continuous^"NodeD", "dim1");
    net->AddNode(continuous^"NodeE", "dim1 dim2");

    net->AddArc("NodeA NodeB", "NodeC");
    net->AddArc("NodeC", "NodeD NodeE");

    net->SetPGaussian("NodeA", "1.0 0.0", "4.0 1.0 1.0 4.0");
    net->SetPGaussian("NodeB", "1.0", "1.0");
    net->SetPGaussian("NodeC", "0.0 0.0", "2.0 1.0 1.0 1.0", "1.0 2.0 1.0 0.0 2.0 1.0");
    net->SetPGaussian("NodeD", "0.0", "1.0", "1.0 1.0");
    net->SetPGaussian("NodeE", "0.0 0.0", "1.0 0.0 0.0 1.0", "1.0 0.0 1.0 1.0");

    return net;
}

void TestGaussianInference(bool DeleteNet)
{
    BayesNet *net = PolytreeModel();
    float eps = 1e-3f;

    net->SetProperty("Inference", "naive");

    TokArr Ajpd, Bjpd, Cjpd, Ejpd, Djpd;
    Tok AjpdMean, AjpdCov, BjpdMean, BjpdCov, CjpdMean, CjpdCov, DjpdMean, DjpdCov, EjpdMean, EjpdCov;
    Cjpd = net->GetJPD("NodeC");
    CjpdMean = Cjpd[0];
    CjpdCov = Cjpd[1];
    if( CjpdMean.FltValue(0).fl - 3.0f > eps ||
        CjpdMean.FltValue(1).fl - 2.0f > eps ||
        CjpdCov.FltValue(0).fl - 30.0f > eps ||
        CjpdCov.FltValue(1).fl - 9.0f > eps ||
        CjpdCov.FltValue(2).fl - 9.0f > eps ||
        CjpdCov.FltValue(3).fl - 6.0f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    Djpd = net->GetJPD("NodeD");
    //cout << String(Djpd) << endl;
    DjpdMean = Djpd[0];
    DjpdCov = Djpd[1];
    if( DjpdMean.FltValue(0).fl - 5.0f > eps ||
        DjpdCov.FltValue(0).fl - 55.0f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    Ejpd = net->GetJPD("NodeE");
    //cout << String(Ejpd) << endl;
    EjpdMean = Ejpd[0];
    EjpdCov = Ejpd[1];
    if( EjpdMean.FltValue(0).fl - 3.0f > eps ||
        EjpdMean.FltValue(1).fl - 5.0f > eps ||
        EjpdCov.FltValue(0).fl - 31.0f > eps ||
        EjpdCov.FltValue(1).fl - 39.0f > eps ||
        EjpdCov.FltValue(2).fl - 39.0f > eps ||
        EjpdCov.FltValue(3).fl - 55.0f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    // add evidence
    net->EditEvidence("NodeE^dim1^5.0 NodeE^dim2^5.0");
    Cjpd = net->GetJPD("NodeC");
    //cout << String(Cjpd) << endl;
    CjpdMean = Cjpd[0];
    CjpdCov = Cjpd[1];
    if( CjpdMean.FltValue(0).fl - 4.4022f > eps ||
        CjpdMean.FltValue(1).fl - 1.0217f > eps ||
        CjpdCov.FltValue(0).fl - 0.7011f > eps ||
        CjpdCov.FltValue(1).fl + 0.4891f > eps ||
        CjpdCov.FltValue(2).fl + 0.4891f > eps ||
        CjpdCov.FltValue(3).fl - 1.1087f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    Djpd = net->GetJPD("NodeD");
    //cout << String(Djpd) << endl;
    DjpdMean = Djpd[0];
    DjpdCov = Djpd[1];
    if( DjpdMean.FltValue(0).fl - 5.4239f > eps ||
        DjpdCov.FltValue(0).fl - 1.8315f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    Ejpd = net->GetJPD("NodeE");
    //cout << String(Ejpd) << endl;
    EjpdMean = Ejpd[0];
    EjpdCov = Ejpd[1];
    if( EjpdMean.FltValue(0).fl - 5.0f > eps ||
        EjpdMean.FltValue(1).fl - 5.0f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    Ajpd = net->GetJPD("NodeA");
    AjpdMean = Ajpd[0];
    AjpdCov = Ajpd[1];
    if( AjpdMean.FltValue(0).fl - 0.3478f > eps ||
        AjpdMean.FltValue(1).fl - 1.1413f > eps ||
        AjpdCov.FltValue(0).fl - 1.8261f > eps ||
        AjpdCov.FltValue(1).fl + 0.1957f > eps ||
        AjpdCov.FltValue(2).fl + 0.1957f > eps ||
        AjpdCov.FltValue(3).fl - 1.0924f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    Bjpd = net->GetJPD("NodeB");
    //cout << String(Bjpd) << endl;
    BjpdMean = Bjpd[0];
    BjpdCov = Bjpd[1];
    if( BjpdMean.FltValue(0).fl - 0.9239f > eps ||
        BjpdCov.FltValue(0).fl - 0.8315f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    //net->EditEvidence("NodeB^dim1^0.0 NodeD^dim1^10.0");
    net->EditEvidence("NodeB^0.0 NodeD^10.0");

    Cjpd = net->GetJPD("NodeC");
    //cout << String(Cjpd) << endl;
    CjpdMean = Cjpd[0];
    CjpdCov = Cjpd[1];
    if( CjpdMean.FltValue(0).fl - 4.9964f > eps ||
        CjpdMean.FltValue(1).fl - 2.4444f > eps ||
        CjpdCov.FltValue(0).fl - 0.6738f > eps ||
        CjpdCov.FltValue(1).fl + 0.5556f > eps ||
        CjpdCov.FltValue(2).fl + 0.5556f > eps ||
        CjpdCov.FltValue(3).fl - 0.8889f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    Ajpd = net->GetJPD("NodeA");
    //cout << String(Ajpd) << endl;
    AjpdMean = Ajpd[0];
    AjpdCov = Ajpd[1];
    if( AjpdMean.FltValue(0).fl - 2.2043f > eps ||
        AjpdMean.FltValue(1).fl - 1.2151f > eps ||
        AjpdCov.FltValue(0).fl - 1.2903f > eps ||
        AjpdCov.FltValue(1).fl + 0.4839f > eps ||
        AjpdCov.FltValue(2).fl + 0.4839f > eps ||
        AjpdCov.FltValue(3).fl - 0.8056f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    net->SetProperty("Inference", "pearl");
    net->ClearEvid();

    Cjpd = net->GetJPD("NodeC");
    //cout << String(Cjpd) << endl;
    CjpdMean = Cjpd[0];
    CjpdCov = Cjpd[1];
    if( CjpdMean.FltValue(0).fl - 3.0f > eps ||
        CjpdMean.FltValue(1).fl - 2.0f > eps ||
        CjpdCov.FltValue(0).fl - 30.0f > eps ||
        CjpdCov.FltValue(1).fl - 9.0f > eps ||
        CjpdCov.FltValue(2).fl - 9.0f > eps ||
        CjpdCov.FltValue(3).fl - 6.0f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    Djpd = net->GetJPD("NodeD");
    //cout << String(Djpd) << endl;
    DjpdMean = Djpd[0];
    DjpdCov = Djpd[1];
    if( DjpdMean.FltValue(0).fl - 5.0f > eps ||
        DjpdCov.FltValue(0).fl - 55.0f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    Ejpd = net->GetJPD("NodeE");
    //cout << String(Ejpd) << endl;
    EjpdMean = Ejpd[0];
    EjpdCov = Ejpd[1];
    if( EjpdMean.FltValue(0).fl - 3.0f > eps ||
        EjpdMean.FltValue(1).fl - 5.0f > eps ||
        EjpdCov.FltValue(0).fl - 31.0f > eps ||
        EjpdCov.FltValue(1).fl - 39.0f > eps ||
        EjpdCov.FltValue(2).fl - 39.0f > eps ||
        EjpdCov.FltValue(3).fl - 55.0f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    net->EditEvidence("NodeE^dim1^5.0 NodeE^dim2^5.0");
    Cjpd = net->GetJPD("NodeC");
    //cout << String(Cjpd) << endl;
    CjpdMean = Cjpd[0];
    CjpdCov = Cjpd[1];
    if( CjpdMean.FltValue(0).fl - 4.4022f > eps ||
        CjpdMean.FltValue(1).fl - 1.0217f > eps ||
        CjpdCov.FltValue(0).fl - 0.7011f > eps ||
        CjpdCov.FltValue(1).fl + 0.4891f > eps ||
        CjpdCov.FltValue(2).fl + 0.4891f > eps ||
        CjpdCov.FltValue(3).fl - 1.1087f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    Djpd = net->GetJPD("NodeD");
    //cout << String(Djpd) << endl;
    DjpdMean = Djpd[0];
    DjpdCov = Djpd[1];
    if( DjpdMean.FltValue(0).fl - 5.4239f > eps ||
        DjpdCov.FltValue(0).fl - 1.8315f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    Ejpd = net->GetJPD("NodeE");
    //cout << String(Ejpd) << endl;
    EjpdMean = Ejpd[0];
    EjpdCov = Ejpd[1];
    if( EjpdMean.FltValue(0).fl - 5.0f > eps ||
        EjpdMean.FltValue(1).fl - 5.0f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    Ajpd = net->GetJPD("NodeA");
    AjpdMean = Ajpd[0];
    AjpdCov = Ajpd[1];
    if( AjpdMean.FltValue(0).fl - 0.3478f > eps ||
        AjpdMean.FltValue(1).fl - 1.1413f > eps ||
        AjpdCov.FltValue(0).fl - 1.8261f > eps ||
        AjpdCov.FltValue(1).fl + 0.1957f > eps ||
        AjpdCov.FltValue(2).fl + 0.1957f > eps ||
        AjpdCov.FltValue(3).fl - 1.0924f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    Bjpd = net->GetJPD("NodeB");
    //cout << String(Bjpd) << endl;
    BjpdMean = Bjpd[0];
    BjpdCov = Bjpd[1];
    if( BjpdMean.FltValue(0).fl - 0.9239f > eps ||
        BjpdCov.FltValue(0).fl - 0.8315f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    net->EditEvidence("NodeB^0.0 NodeD^10.0");

    Cjpd = net->GetJPD("NodeC");
    //cout << String(Cjpd) << endl;
    CjpdMean = Cjpd[0];
    CjpdCov = Cjpd[1];
    if( CjpdMean.FltValue(0).fl - 4.9964f > eps ||
        CjpdMean.FltValue(1).fl - 2.4444f > eps ||
        CjpdCov.FltValue(0).fl - 0.6738f > eps ||
        CjpdCov.FltValue(1).fl + 0.5556f > eps ||
        CjpdCov.FltValue(2).fl + 0.5556f > eps ||
        CjpdCov.FltValue(3).fl - 0.8889f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    Ajpd = net->GetJPD("NodeA");
    //cout << String(Ajpd) << endl;
    AjpdMean = Ajpd[0];
    AjpdCov = Ajpd[1];
    if( AjpdMean.FltValue(0).fl - 2.2043f > eps ||
        AjpdMean.FltValue(1).fl - 1.2151f > eps ||
        AjpdCov.FltValue(0).fl - 1.2903f > eps ||
        AjpdCov.FltValue(1).fl + 0.4839f > eps ||
        AjpdCov.FltValue(2).fl + 0.4839f > eps ||
        AjpdCov.FltValue(3).fl - 0.8056f > eps )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Naive inference without evidences is wrong");
    }

    if (DeleteNet)
    {
        delete net;
    };

    cout << "TestGaussianInference is completed successfully" << endl;
}

void TestGaussianParamLearning(bool DeleteNet)
{
    BayesNet *net = PolytreeModel();
    BayesNet *netToLearn = PolytreeModel();
    float eps = 1e-1f;

    int nEvid = 5000;
    netToLearn->GenerateEvidences(nEvid);

    netToLearn->LearnParameters();

    String nodes[] = {"NodeA", "NodeB", "NodeC", "NodeD", "NodeE"};

    int nNodes = 5;
    int i, j;
    TokArr LearnParam, Param;
    for(i = 0; i < nNodes; i++)
    {
        LearnParam = netToLearn->GetGaussianMean(nodes[i]);
        Param = net->GetGaussianMean(nodes[i]);
        if(LearnParam[0].fload.size() != Param[0].fload.size())
        {
            PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
        }
        for(j = 0; j < LearnParam[0].fload.size(); j++)
        {
            if( LearnParam[0].FltValue(j).fl - Param[0].FltValue(j).fl > eps)
            {
                PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
            }
        }

        LearnParam = netToLearn->GetGaussianCovar(nodes[i]);
        Param = net->GetGaussianCovar(nodes[i]);
        if(LearnParam[0].fload.size() != Param[0].fload.size())
        {
            PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
        }
        for(j = 0; j < LearnParam[0].fload.size(); j++)
        {
            if( LearnParam[0].FltValue(j).fl - Param[0].FltValue(j).fl > eps)
            {
                PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
            }
        }
    }

    if (DeleteNet)
    {
	delete net;
	delete netToLearn;
    };
    
    cout << "TestGaussianParamLearning is completed successfully" << endl;
}

void TestGetLogLik(bool DeleteNet)
{
    BayesNet *net = PolytreeModel();
    BayesNet *netToLearn = PolytreeModel();
    float eps = 1e-2f;

    int nEvid = 500;
    netToLearn->GenerateEvidences(nEvid);

    int i, j;

    float logLikBest = 0.0f;
    TokArr bufLiks = netToLearn->GetEvidBufLogLik();
    for(i = 0; i < nEvid; i++)
    {
	logLikBest += bufLiks[i].FltValue();
    }

    netToLearn->SetProperty("Learning", "em");
    netToLearn->LearnParameters();

    float logLikRes = netToLearn->GetEMLearningCriterionValue();

    cout << "Best logarithm of likelihood = " << logLikBest << endl;

    cout << "Result logarithm of likelihood = " << logLikRes << endl;
    if( fabs(logLikRes) < fabs(logLikBest) )
    {
	// compare logLikRes and logLikBest with tolerance eps:
	// check that exponent parts are equal and compare fixed 
	// point parts with tolerance eps

	int exp1 = 0;
	while(floor(fabs(logLikRes)) > 0)
	{
	    logLikRes /= 10.0;
	    exp1++;
	}
	int exp2 = 0;
	while(floor(fabs(logLikBest)) > 0)
	{
	    logLikBest /= 10.0;
	    exp2++;
	}
	if(exp1 != exp2)
	{
            PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
	}
	if(fabs(logLikBest) - fabs(logLikRes) > eps)
	{
            PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
	}
    }

    if (DeleteNet)
    {
	delete net;
	delete netToLearn;
    }

    cout << "TestGetLogLik is completed successfully" << endl;
}

void SimpleModel()
{
    BayesNet net;
    net.AddNode("continuous^x0", "dim1"); 
    net.AddNode("continuous^x1", "dim1"); 
    net.AddNode("continuous^x2", "dim1"); 
    net.AddNode("continuous^x3", "dim1"); 
    net.AddNode("continuous^x4", "dim1");
    
    net.AddArc("x0", "x2");
    net.AddArc("x1", "x2");
    net.AddArc("x2", "x3");
    net.AddArc("x2", "x4");
    
    net.SetPGaussian("x0", "1.0", "4.0");
    net.SetPGaussian("x1", "1.0", "1.0");
        
    net.SetPGaussian("x2", "0.0", "2.0", "1.0 2.0");
    net.SetPGaussian("x3", "0.0", "4.0", "1.1");
    net.SetPGaussian("x4", "-0.8", "1.2", "2.0");
    
    net.AddEvidToBuf("x0^dim1^0.4");

    net.LearnParameters();
    TokArr MeanX2 = net.GetGaussianMean("x2");
    float mean = MeanX2[0].FltValue();

    TokArr CovarX2 = net.GetGaussianCovar("x2");
    float cov = CovarX2[0].FltValue();

    net.SetProperty("Inference", "jtree");
    const TokArr X3Marg = net.GetJPD("x3");
    Tok x3Mean = X3Marg[0];
    Tok x3Cov = X3Marg[1];

    cout<< X3Marg <<"\n";
    float x3M = x3Mean.FltValue(0).fl;
    float x3C = x3Cov.FltValue(0).fl;

    TokArr X3MPE = net.GetMPE("x3");

    cout<< X3MPE <<"\n";
    float x3MPE = X3MPE[0].FltValue(0).fl;
}

void TestsPnlHigh::Test2Bnets()
{
    TestGaussianModelCreate(false);
    TestGaussianInference(false);
    TestGaussianParamLearning(false);
}

int testGaussianBNet()
{
	int res = TRS_OK;
	try 
    {
        TestGaussianModelCreate();// del arc
        TestGaussianInference();
        TestGaussianParamLearning();// too long
        TestGetLogLik();
    }
    catch(pnl::CException e)
    {
        std::cout << e.GetMessage();
        res = TRS_FAIL;
    }


	return trsResult( res, res == TRS_OK ? "No errors"
	: "Bad test on Tokens");
}

void initTestsGaussianBNet()
{
    trsReg(func_name, test_desc, test_class, testGaussianBNet);
}
