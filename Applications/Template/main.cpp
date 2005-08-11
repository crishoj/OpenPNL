#include "pnl_dll.hpp"
PNL_USING

int main (int argc, char * argv[])
{
    int ret;
	CBNet *pBNet = pnlExCreateWasteBNet();
    pEvidencesVector pEv;
    pBNet->GenerateSamples( &pEv, 1 );
    
    float ll = pBNet->ComputeLogLik(pEv[0]);
    delete pBNet;
    delete pEv[0];

    /////////////////////////////////////////////////////////
    
    
    
    const int numOfNds = 4;
    const int numOfNodeTypes = 1;
    const int numOfClqs = 4;
    
    intVector clqSizes( numOfClqs, 2 );
    
    int clq0[] = { 0, 1 };
    int clq1[] = { 1, 2 };
    int clq2[] = { 2, 3 };
    int clq3[] = { 3, 0 };
    
    const int *clqs[] = { clq0, clq1, clq2, clq3 };
    
    CNodeType nodeType( 1, 2 );
    
    intVector nodeAssociation( numOfNds, 0 );
    
    CMNet *pMNet = CMNet::Create( numOfNds, numOfNodeTypes, &nodeType,
        &nodeAssociation.front(), numOfClqs, &clqSizes.front(), clqs );
    
    pMNet->AllocFactors();
    
    float data0[]  = { 0.79f, 0.21f, 0.65f, 0.35f };
    float data1[]  = { 0.91f, 0.09f, 0.22f, 0.78f };
    float data2[]  = { 0.45f, 0.55f, 0.24f, 0.76f };
    float data3[]  = { 0.51f, 0.49f, 0.29f, 0.71f };
    
    float *data[] = { data0, data1, data2, data3 };
    
   
    int i;
    for( i = 0 ; i < numOfClqs; ++i )
    {
        pMNet->AllocFactor(i);
        
        pMNet->GetFactor(i)->AllocMatrix( data[i], matTable );
    }
    
    pEvidencesVector pEvMNet;
    pMNet->GenerateSamples( &pEvMNet, 1 );
    
    ll = 0.0f;
    
    ll= pMNet->ComputeLogLik( pEvMNet[0] );
    delete pMNet;
    delete pEvMNet[0];
    //////////////////////////////////////////////////////////////////////////
    CBNet *pSCBNet = pnlExCreateScalarGaussianBNet();
    CEvidence *pEvSCBNet;
    valueVector vlsSCBNet;
    vlsSCBNet.resize(3);
    vlsSCBNet[0].SetFlt(0.1f);
    vlsSCBNet[1].SetFlt(1.0f);
    vlsSCBNet[2].SetFlt(0.0f);
    int nObsNdsSCBNet = 3;
    int obsNdsSCBNet[] = {0, 1, 3};
    
    pEvSCBNet = CEvidence::Create(pSCBNet, nObsNdsSCBNet, obsNdsSCBNet, vlsSCBNet);

    
    CJtreeInfEngine *pInfSCBNet = CJtreeInfEngine::Create(pSCBNet);
    pInfSCBNet->SetJTreeRootNode(1);
    pInfSCBNet->EnterEvidence(pEvSCBNet);
    ll = pInfSCBNet->GetLogLik();
    
    delete pSCBNet;
    delete pEvSCBNet;
    //CJtreeInfEngine::Release(&pInfSCBNet);
    delete pInfSCBNet;

    
    //////////////////////////////////////////////////////////////////////////
    
    CBNet *pWSBnet = pnlExCreateWaterSprinklerBNet();
    CEvidence *pEvWS;
    valueVector vls;
    vls.resize(2);
    vls[0].SetInt(0);
    vls[1].SetInt(1);
    
    int nObsNds = 2;
    int obsNds[] = {0, 3};
    
    pEvWS = CEvidence::Create(pWSBnet, nObsNds, obsNds, vls);

    const float llFromMatlab = -1.2928f;
     
    
    CJtreeInfEngine *pInf = CJtreeInfEngine::Create(pWSBnet);
    pInf->SetJTreeRootNode(1);
    pInf->EnterEvidence(pEvWS);
    ll = pInf->GetLogLik();
    if( fabs(ll - llFromMatlab) >0.001f )
    {
	ret = 1;
    }

    
    delete pWSBnet;
    delete pEvWS;
    //CJtreeInfEngine::Release(&pInf);
    delete pInf;
    
    
    return ret;
}