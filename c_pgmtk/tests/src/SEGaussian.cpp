/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AGaussianInference.cpp                                      //
//                                                                         //
//  Purpose:   Test on inference for Gaussian BNet model                   //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include <float.h>

static char func_name[] = "testSEGaussian";

static char* test_desc = "Check some methods of Gaussian";

static char* test_class = "Side effects";

int testSEGaussian()
{
    PNL_USING
   
    int i;
    int ret = TRS_OK;
    float eps = 1e-4f;
    //create Gaussian distribution (inside the potential) and try to multiply
    //is by Delta
    
    //create Model Domain
    int nSimNodes = 3;
    CNodeType simNT = CNodeType(0, 2);
    CModelDomain* pSimDomain = CModelDomain::Create( nSimNodes, simNT );
    //create 2 potentials
    intVector dom;
    dom.assign(3,0);
    dom[1] = 1;
    dom[2] = 2;
    floatVector mean;
    mean.assign(6, 0);
    floatVector cov;
    cov.assign(36, 0.1f);
    for( i = 0; i < 6; i++ )
    {
        mean[i] = 1.1f*i;
        cov[i*7] = 2.0f;
    }

	CGaussianPotential* pPot = CGaussianPotential::Create( dom, pSimDomain, 1,
        mean, cov, 1.0f );

    //create gaussian CPD with gaussian parent
    const pConstNodeTypeVector* pTypes = pPot->GetArgType();
    //create data weigth
    floatVector weights1;
    weights1.assign(4, 1.0f);
    floatVector weights2;
    weights2.assign(4, 2.0f);
    const float* weights[] = { &weights1.front(), &weights2.front()};
    CGaussianDistribFun* pGauCPD = CGaussianDistribFun::CreateInMomentForm( 0,
        3, &pTypes->front(), &mean.front(), &cov.front(), weights );

    pPot->Dump();
    pPot->Normalize();
    //try to get multiplied delta
    intVector pos;
    floatVector valuesDelta;
    intVector offsets;
    pPot->GetMultipliedDelta(&pos, &valuesDelta, &offsets);
    if( pos.size() != 0 )
    {
        ret = TRS_FAIL;
    }


    //enter evidence to the pot and after that expand and compare results
    //create evidence object
/*    valueVector obsVals;
    obsVals.assign(6,Value(0) );
    obsVals[0].SetFlt(1.0f);
    obsVals[1].SetFlt(1.5f);
    obsVals[2].SetFlt(2.0f);
    obsVals[3].SetFlt(2.5f);
    obsVals[4].SetFlt(3.0f);
    obsVals[5].SetFlt(3.5f);
    CEvidence* pEvid = CEvidence::Create( pSimDomain, 3, &dom.front(), obsVals );
    CPotential* pShrPot = pPot->ShrinkObservedNodes( pEvid );*/
    
    
    CGaussianPotential* pCanonicalPot = CGaussianPotential::Create( dom,
        pSimDomain, 0, mean, cov, 1.0f );
    CMatrix<float>* matrK = pCanonicalPot->GetMatrix(matK);
    intVector multIndex;
    multIndex.assign(2,5);
    matrK->SetElementByIndexes( 3.0f, &multIndex.front() );
    CMatrix<float>* matrH = pCanonicalPot->GetMatrix(matH);
    multIndex[0] = 0;
    matrH->SetElementByIndexes(0.0f, &multIndex.front());
    pPot->ConvertToSparse();
    pPot->ConvertToDense();
    //create other Gaussian potential for division
    i = 1;
    floatVector meanSmall;
    meanSmall.assign(2, 1.0f);
    floatVector covSmall;
    covSmall.assign(4, 0.0f);
    covSmall[0] = 1.0f;
    covSmall[3] = 1.0f;
    CGaussianPotential* pSmallPot = CGaussianPotential::Create( &i, 1,
        pSimDomain, 1, &meanSmall.front(), &covSmall.front(), 1.0f );
    //divide by distribution in moment form
    (*pCanonicalPot) /= (*pSmallPot);

    //create big unit function distribution and marginalize it
    CGaussianPotential* pBigUnitPot = 
        CGaussianPotential::CreateUnitFunctionDistribution(dom, pSimDomain, 1);
	CGaussianPotential* pCloneBigUniPot = static_cast<CGaussianPotential*>(
		pBigUnitPot->CloneWithSharedMatrices());
	if( !pCloneBigUniPot->IsFactorsDistribFunEqual(pBigUnitPot, eps) )
	{
		ret = TRS_FAIL;
	}
    CPotential* pMargUniPot = pBigUnitPot->Marginalize(&dom.front(), 1, 0);
    (*pBigUnitPot) /= (*pSmallPot);
    (*pBigUnitPot) *= (*pSmallPot);
    
    //check if there are some problems
    static_cast<CGaussianDistribFun*>(pBigUnitPot->GetDistribFun())->CheckCanonialFormValidity();

    if( pBigUnitPot->IsDistributionSpecific() != 1 )
    {
        ret = TRS_FAIL;
    }

    (*pBigUnitPot) /= (*pCanonicalPot);
    (*pBigUnitPot) *= (*pCanonicalPot);

    static_cast<CGaussianDistribFun*>(pBigUnitPot->GetDistribFun())->CheckCanonialFormValidity();

    if( pBigUnitPot->IsDistributionSpecific() != 1 )
    {
        ret = TRS_FAIL;
    }

    static_cast<CGaussianDistribFun*>(pSmallPot->GetDistribFun())->
        UpdateCanonicalForm();
    (*pPot) /= (*pSmallPot);
    pSmallPot->SetCoefficient(0.0f,1);
    pSmallPot->Dump();

    //create canonical potential without coefficient
    i = 0;

    CDistribFun* pCopyPotDistr = pPot->GetDistribFun()->ConvertCPDDistribFunToPot();
    CGaussianPotential* pCanSmallPot = CGaussianPotential::Create( &i, 1,
        pSimDomain, 0, &meanSmall.front(), &covSmall.front(), 1.0f );
    CGaussianPotential* pCanPotCopy = 
        static_cast<CGaussianPotential*>(pCanSmallPot->Clone());
    CGaussianPotential* pCanPotCopy1 = 
        static_cast<CGaussianPotential*>(pCanPotCopy->CloneWithSharedMatrices());

    (*pPot) /= (*pCanSmallPot);
    (*pPot) *= (*pCanSmallPot);
    //can compare results, if we want
    CDistribFun* pMultDivRes = pPot->GetDistribFun();
    float diff = 0;
    if( !pMultDivRes->IsEqual(pCopyPotDistr, eps,1, &diff) )
    {
        ret = TRS_FAIL;
        std::cout<<"the diff is "<<diff<<std::endl;
    }
    delete pCopyPotDistr;
    
    //create delta distribution
    floatVector deltaMean;
    deltaMean.assign( 2, 1.5f );
    i = 0;
    CGaussianPotential* pDeltaPot = CGaussianPotential::CreateDeltaFunction( 
        &i, 1, pSimDomain, &deltaMean.front(), 1 );
    
    CGaussianDistribFun* pDeltaDistr = static_cast<CGaussianDistribFun*>(
        pDeltaPot->GetDistribFun());
    pDeltaDistr->CheckMomentFormValidity();
    pDeltaDistr->CheckCanonialFormValidity();
    pDeltaPot->Dump();


    //multiply some potential by delta
    (*pCanSmallPot) *= (*pDeltaPot);

    (*pPot) *= (*pDeltaPot);
    //(*pPot) *= (*pDeltaPot);
    
    pPot->GetMultipliedDelta(&pos, &valuesDelta, &offsets);
    (*pCanonicalPot) *= (*pDeltaPot);
    //marginalize this distribFun multiplied by delta
    intVector margDims;
    margDims.assign(2,1);
    margDims[1] = 2;
    CPotential* pMargPot = pPot->Marginalize( &margDims.front(), 2 );
    i = 0;
    CPotential* pSmallMargPot = pPot->Marginalize(&i, 1);
    //marginalize in canonical form
    CPotential* pMargCanPot = pCanonicalPot->Marginalize( &margDims.front(), 2 );
    CPotential* pSmallCanPot = pCanonicalPot->Marginalize(&i,1);
    
    //create unit function distribution in canonical form
    i = 0;
    CGaussianPotential* pUnitPot = 
        CGaussianPotential::CreateUnitFunctionDistribution( &i, 1, pSimDomain,
        1);
    pUnitPot->Dump();
    CGaussianDistribFun* pUnitDistr = static_cast<CGaussianDistribFun*>(
        pUnitPot->GetDistribFun());
    pUnitDistr->CheckCanonialFormValidity();
    pUnitDistr->CheckMomentFormValidity();

    (*pPot) *= (*pUnitPot);

    if( pUnitPot->IsFactorsDistribFunEqual(pBigUnitPot, eps, 1) )
    {
        ret = TRS_FAIL;
    }

    deltaMean.resize(6);
    deltaMean[2] = 2.5f;
    deltaMean[3] = 2.5f;
    deltaMean[4] = 3.5f;
    deltaMean[5] = 3.5f;
    CGaussianPotential* pBigDeltaPot = CGaussianPotential::CreateDeltaFunction( 
        dom, pSimDomain, deltaMean, 1 );
    CGaussianPotential* pCloneBigDeltaPot = static_cast<CGaussianPotential*>(
		pBigDeltaPot->CloneWithSharedMatrices());
    //we can shrink observed nodes in this potential
    valueVector vals;
    vals.resize(2);
    vals[0].SetFlt(1.5f);
    vals[1].SetFlt(1.5f);
    i = 0;
    CEvidence* pDeltaEvid = CEvidence::Create( pSimDomain, 1, &i,vals ); 
    CGaussianPotential* pShrGauDeltaPot = static_cast<CGaussianPotential*>(
        pBigDeltaPot->ShrinkObservedNodes( pDeltaEvid ));
    delete pDeltaEvid;
    pShrGauDeltaPot->Dump();
    delete pShrGauDeltaPot;
    CPotential* pSmallDeltaMarg = pBigDeltaPot->Marginalize( &dom.front(), 1, 0 );
    pSmallDeltaMarg->Normalize();
    pDeltaPot->Normalize();
    if( !pSmallDeltaMarg->IsFactorsDistribFunEqual( pDeltaPot, eps, 1 ) )
    {
        ret = TRS_FAIL;
    }
    //call operator = for delta distributions
    (*pSmallDeltaMarg) = (*pDeltaPot);

    //call operator = for canonical and delta distribution
    (*pCanPotCopy) = (*pUnitPot);

    //call operator = for canonical and unit distribution
    (*pCanPotCopy1) = (*pDeltaPot);

    (*pUnitPot) = (*pUnitPot);
    

    (*pPot) *= (*pBigDeltaPot);

    (*pCloneBigDeltaPot) *= (*pDeltaPot);

	if( !pCloneBigDeltaPot->IsFactorsDistribFunEqual(pBigDeltaPot, eps) )
	{
		ret = TRS_FAIL;
	}

    //we can create the matrix which will be almost delta distribution,
    //but covariance matrix will contain almost zeros
    floatVector almostDeltaCov;
    almostDeltaCov.assign(4, 0.0f);
    almostDeltaCov[0] = 0.00001f;
    almostDeltaCov[3] = 0.00001f;
    i = 0;
    CGaussianPotential* pAlmostDeltaPot = CGaussianPotential::Create( &i, 1, 
        pSimDomain, 1, &deltaMean.front(), &almostDeltaCov.front(), 1.0f );
    if( !pDeltaPot->IsFactorsDistribFunEqual( pAlmostDeltaPot, eps, 0 ) )
    {
        ret = TRS_FAIL;
    }
    if( !pAlmostDeltaPot->IsFactorsDistribFunEqual( pDeltaPot, eps, 0 ) )
    {
        ret = TRS_FAIL;
    }
	(*pCloneBigUniPot ) = ( *pPot );
	if( !(pCloneBigUniPot->IsFactorsDistribFunEqual(pPot, eps)) )
	{
		ret = TRS_FAIL;
	}
	(*pCloneBigDeltaPot) = (*pPot);
	if( !(pCloneBigDeltaPot->IsFactorsDistribFunEqual(pPot, eps))  )
	{
		ret = TRS_FAIL;
	}

    delete pCanPotCopy;
    delete pCanPotCopy1;
    delete pAlmostDeltaPot;
    delete pMargUniPot;
	delete pCloneBigUniPot;
    delete pBigUnitPot;
    delete pCanSmallPot;
    delete pDeltaPot; 
    delete pUnitPot;
	delete pCloneBigDeltaPot;
    delete pBigDeltaPot;
    delete pMargCanPot;
    delete pSmallCanPot;
    delete pMargPot;
    delete pSmallMargPot;
    delete pCanonicalPot;
    //delete pShrPot;
    //delete pEvid;
	delete pGauCPD;
    delete pPot;
    delete pSimDomain;
    return trsResult( ret, ret == TRS_OK ? "No errors" : 
                        "Bad test on SEGaussian");

}

void initSEGaussian()
{
    trsReg(func_name, test_desc, test_class, testSEGaussian);
}

