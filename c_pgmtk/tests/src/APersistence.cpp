/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      APersistence.cpp                                            //
//                                                                         //
//  Purpose:   TRS test of persistence                                     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include "pnlContext.hpp"
#include "pnlContextPersistence.hpp"
#include "pnlExampleModels.hpp"

PNL_USING

static char  func_name[] = "testPersistence";
static char* test_desc = "Check saving/loading";
static char* test_class = "Algorithm";

static int iTest = 0;
static int nFailed = 0;

static void CompareFiles(const char *name1, const char *name2)
{
    FILE *fp1 = fopen(name1, "r");
    FILE *fp2 = fopen(name2, "r");
    bool bError = false;

    if(fp1 && fp2)
    {
	char buf[8192];
	int nByte;

	for(;;)
	{
	    nByte = fread(buf, 1, sizeof(buf)/2, fp1);
	    if(fread(buf + sizeof(buf)/2, 1, sizeof(buf)/2, fp2) != nByte)
	    {
		trsWrite(TO_CON, "%i: file differs by sizes", iTest);
		++nFailed;
		bError = true;
		break;
	    }
	    if(nByte <= 0)
	    {
		break;
	    }
	    if(memcmp(buf, buf + sizeof(buf)/2, nByte))
	    {
		trsWrite(TO_CON, "%i: file differs by content", iTest);
		bError = true;
		++nFailed;
		break;
	    }
	}
    }
    else
    {
        trsWrite(TO_CON, "Can't open one of the files: '%s', '%s'", name1, name2);
    }

    if(fp1)
    {
	fclose(fp1);
    }
    if(fp2)
    {
	fclose(fp2);
    }

    if(!bError)
    {
	remove(name1);
	remove(name2);
    }
}

static void CheckModelPersistence(CGraphicalModel *pModel, const char *name)
{
    std::stringstream name1, name2;

    name1 << "testmodel" << iTest << ".xml";
    name2 << "testmodel" << iTest << "_.xml";
    ++iTest;
    {
	CContextPersistence xml;
	
	xml.Put(pModel, name);
	if(!xml.SaveAsXML(name1.str().c_str()))
	{
	    trsWrite(TO_CON, "%i: Saving of model '%s' is failed", iTest, name);
	    nFailed++;
	    delete pModel;
	    return;
	}
    }
    
    {
	CContextPersistence xml;
	
	bool bSuccess = xml.LoadXML(name1.str().c_str());

	if(!bSuccess)
	{
	    trsWrite(TO_CON, "%i: Loading of model '%s' is failed", iTest, name);
errorL1:    nFailed++;
	    delete pModel;
	    return;
	}

	CBNet *pLoadModel = static_cast<CBNet*>(xml.Get(name));
	CGraph *pLoadGraph = pLoadModel->GetGraph();
	CGraph *pGraph = pModel->GetGraph();
	float maxDiff = 0;
	const float tolerance = 0.00001f;
	int nFactorFault = 0;
	
	if(*pLoadGraph != *pGraph || !pLoadModel->IsValid())
	{
	    trsWrite(TO_CON, "%i(%s): Bad model", iTest, name);
errorL2:    delete pLoadModel;
	    goto errorL1;
	}

	for(int i = pModel->GetNumberOfFactors(); --i >= 0; )
	{
	    float diff;

	    if(!pModel->GetFactor(i)->IsFactorsDistribFunEqual(
		pLoadModel->GetFactor(i), tolerance, 1, &diff))
	    {
		++nFactorFault;
		if(maxDiff < diff)
		{
		    maxDiff = diff;
		}
	    }
	}

	if(nFactorFault)
	{
	    trsWrite(TO_CON, "%i: Loading of model '%s' is failed:\n"
		"%i factors loaded with error[s]: maximal difference = %lg",
		iTest, name, double(maxDiff));
	    goto errorL2;
	}

	xml.SaveAsXML(name2.str().c_str());
	delete pLoadModel;
    }

    delete pModel;

    CompareFiles(name1.str().c_str(), name2.str().c_str());
}

static void CheckModelPersistenceEx(CGraphicalModel *pModel, const char *name)
{
    try
    {
	CheckModelPersistence(pModel, name);
    }
    catch(pnl::CException ex)
    {
	trsWrite(TO_CON, "%i:### PNL's exception on model '%s': %s\n\n",
	    iTest, name, ex.GetMessage());
        nFailed++;
    }
}

int testPersistence()
{
/* 1 */	CheckModelPersistenceEx(pnlExCreateVerySimpleGauMix(), "VerySimpleGauMix");
/* 2 */	CheckModelPersistenceEx(pnlExCreateKjaerulfsBNet(), "KjaerulfBNet");
/* 3 */	CheckModelPersistenceEx(pnlExCreateWaterSprinklerBNet(), "WaterSprinklerBNet");
/* 4 */	CheckModelPersistenceEx(pnlExCreateAsiaBNet(), "AsiaBNet");
/* 5 */	CheckModelPersistenceEx(pnlExCreateRndArHMM(), "RndArHMM");
/* 6 */	CheckModelPersistenceEx(pnlExCreateBatNetwork(), "BatNetwork");
/* 7 */	CheckModelPersistenceEx(pnlExCreateWasteBNet(), "WasteBNet");
/* 8 */	CheckModelPersistenceEx(pnlExCreateWaterSprinklerSparseBNet(), "WaterSprinklerSparseBNet");
/* 9 */	CheckModelPersistenceEx(pnlExCreateSimpleGauMix(), "SimpleGauMix");
/*10 */	CheckModelPersistenceEx(pnlExCreateCondGaussArBNet(), "CondGaussArBNet");
/*11 */	CheckModelPersistenceEx(pnlExCreateBNetFormSMILE(), "BNetFormSMILE");
/*12 */	CheckModelPersistenceEx(pnlExCreateSingleGauMix(), "SingleGauMix");

    if(nFailed)
    {
	std::stringstream message;

	message << nFailed << " models from " << iTest << " saved with errors";
	return trsResult( TRS_FAIL, message.str().c_str());
    }

    return trsResult(TRS_OK, "No errors");
}

void initAPersistence()
{
    trsReg( func_name, test_desc, test_class, testPersistence );
}
