#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <algorithm>

#include "tCreateIncineratorBNet.hpp"

// this file must be last included file
#include "tUtil.h"

PNL_USING

static char func_name[] = "testGibbsWithAnnealingInference";

static char* test_desc = "Provide Gibbs With Annealing Inference";

static char* test_class = "Algorithm";

CMNet* myCreateDiscreteMNet();

int GibbsWithAnnealingForBNet(int seed, float eps);

float 
Temp1( float c, float s ) 
{
    return float(c / (log( s*0.001 + 1)+1));
}

int testGibbsWithAnnealingInference()
{

	int ret = TRS_OK;
    int seed = pnlTestRandSeed();
	//seed=1066386455;

    std::cout<<"seed"<<seed<<std::endl;
    pnlSeed(seed);
 /////////////////////////////////////////////////////////////////////////////

    float eps = -1.0f;
    while( eps <= 0 )
    {
	trssRead( &eps, "1.5e-1f", "accuracy in test" );
    }

    ret = ret && GibbsWithAnnealingForBNet( seed,  eps);
    
    return trsResult( ret, ret == TRS_OK ? "No errors" :
    "Bad test on Gibbs Sampling Inference" );

}

int GibbsWithAnnealingForBNet(int seed, float eps)
{

    int ret = 1;
	std::cout<<std::endl<<"Gibbs for Incenerator BNet"<< std::endl;

    CBNet *pBnet;    
	pBnet = pnlExCreateAsiaBNet();
	pEvidencesVector evidences;
	pBnet->GenerateSamples( &evidences, 1 );

	
    CNaiveInfEngine *pNaiveInf;
    CGibbsWithAnnealingInfEngine *pAnnealingInf;
	
	
	const int ndsToToggle1[] = { 0, 2, 3, 4, 5 };
	
    evidences[0]->ToggleNodeState( 5, ndsToToggle1 );
	
    const int querySz1 = 8;
    const int query1[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	
    pNaiveInf = CNaiveInfEngine::Create(pBnet);
    pNaiveInf->EnterEvidence( evidences[0], 1 );
    pNaiveInf->MarginalNodes( query1,querySz1 );
	
    pAnnealingInf = CGibbsWithAnnealingInfEngine::Create( pBnet );
	
    pAnnealingInf->EnterEvidence( evidences[0] );
    pAnnealingInf->MarginalNodes( query1, querySz1 );
	
    const CEvidence *pEv1 = pAnnealingInf->GetMPE();
    const CEvidence *pEv2 = pNaiveInf->GetMPE();
    std::cout<<"result of gibbs"<<std::endl<<std::endl;
    pEv1->Dump();
    std::cout<<"result of naive"<<std::endl;
    pEv2->Dump();
	intVector ObsNodes1;
	pConstValueVector ObsValues1;
	pEv1->GetObsNodesWithValues( &ObsNodes1, &ObsValues1);
	intVector ObsNodes2;
	pConstValueVector ObsValues2;
	pEv1->GetObsNodesWithValues( &ObsNodes2, &ObsValues2);
	int i;
	for( i = 0; i < ObsValues1.size(); i++ )
	{
		if(ObsValues1[i]->IsDiscrete())
		{
			if(ObsValues1[i]->GetInt() != ObsValues2[i]->GetInt() )
			{
				ret = 0;
				break;
			}
		}
		else
		{
			if( fabs (ObsValues1[i]->GetFlt() - ObsValues2[i]->GetFlt() ) > eps )
			{
				ret = 0;
				break;
			}
			
		}
	}
	
	
	
    
    delete evidences[0];
    delete pNaiveInf;
    delete pAnnealingInf;
    delete pBnet;

    return ret;

    ///////////////////////////////////////////////////////////////////////////////

}



void initAGibbsWithAnnealingInference()
{
    trsReg(func_name, test_desc, test_class, testGibbsWithAnnealingInference);
}
