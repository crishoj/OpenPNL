/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AParameterLearningDBN.cpp                                   //
//                                                                         //
//  Purpose:   Test on factor estimation of DBN using EM algorithm      //
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
#include "tCreateKjaerulffDBN.hpp"
#include "tConfig.h"
// this file must be last included file
#include "tUtil.h"

PNL_USING

static char func_name[] = "testLearnParamDBN";

static char* test_desc = "Provide Factor Learning for DBN";

static char* test_class = "Algorithm";


int GetDataToCompare(CDBN *pDBN, pnlVector<float*> * data2Compare);
int CreateEvidences(CDBN *pDBN, pEvidencesVecVector *myEvidencesForDBN);
int testLearnParamDBN()
{
    int ret = TRS_OK;
    float eps = -1.0f;

    while( eps <= 0 )
    {
	trssRead( &eps, "1e-1f", "accuracy in test");
    }

    int maxIter = -1;
    while( maxIter < 1 )
    {
	trsiRead( &maxIter, "5", "maximum number of iterations");
    }
    //////////////////////////////////////////////////////////////////////////
    int seed = pnlTestRandSeed();

    std::cout<<"seed"<<seed<<std::endl;

    srand( seed );

    CDBN *pDBN = tCreateKjaerulffDBN();
    pnlVector< pnlVector<CEvidence*> > myEvidencesForDBN;
    if( !CreateEvidences(pDBN, &myEvidencesForDBN))
    {
	ret = TRS_FAIL;
	return trsResult( ret, ret == TRS_OK ? "No errors" :
	"Bad test on LearnParamDBN, not file with cases");
    }

    pnlVector<float*> data2Compare;
    if( !GetDataToCompare(pDBN, &data2Compare) )
    {
	ret = FALSE;
	return trsResult( ret, ret == TRS_OK ? "No errors" :
	"Bad test on LearnParamDBN, not file with cases");

    }
    C1_5SliceJtreeInfEngine *pJTree =
	C1_5SliceJtreeInfEngine::Create(pDBN);
    CEMLearningEngineDBN *pLearn =
	CEMLearningEngineDBN::Create(pDBN);
    int numOfTimeSeries = myEvidencesForDBN.size();


    intVector numberOfSlices;
    numberOfSlices.resize(numOfTimeSeries);
    int series;
    for(series = 0; series < numOfTimeSeries; series++)
    {
	numberOfSlices[series] = (myEvidencesForDBN[series]).size();
    }

    CEvidence *** evFinal;
    evFinal = new CEvidence** [numOfTimeSeries];
    for(series = 0; series < numOfTimeSeries; series++)
    {
	(evFinal[series]) = new CEvidence*[numberOfSlices[series]];
	int slice;
	for(slice = 0; slice < numberOfSlices[series]; slice++)
	{
	    (evFinal[series])[slice] = (myEvidencesForDBN[series])[slice];
	}
    }

    pLearn->SetData(numOfTimeSeries, &numberOfSlices.front(), evFinal);
    pLearn->SetMaxIterEM(maxIter);
    pLearn->Learn();
    /////////////////////////////////////////////////////////////////////////
    CBNet *pItogBnet = static_cast<CBNet *>(pDBN->GetStaticModel());
    int nnodes = pItogBnet->GetNumberOfNodes();
    int dataLength;
    const float* data;
    float delta;
    int node;
    for( node = 0; node < nnodes; node++ )
    {
	static_cast<CNumericDenseMatrix<float>*>(pItogBnet->GetFactor(node)->
	    GetMatrix(matTable))->GetRawData(&dataLength, &data);
	int coord;
	for(coord = 0; coord < dataLength; coord++)
	{
	    delta = (float)fabs(data[coord] - (data2Compare[node])[coord]);

	    if( delta > eps )
	    {
		ret = FALSE;
	    }
	}

    }


    /////////////////////////////////////////////////////////////////////////
    delete (pDBN);
    for(series = 0; series < numOfTimeSeries; series++)
    {

	int slice;
	for(slice = 0; slice < numberOfSlices[series]; slice++)
	{
	    delete (evFinal[series])[slice];
	}
	delete [](evFinal[series]);
    }
    delete []evFinal;

    for( node = 0; node < nnodes; node++ )
    {
	delete [](data2Compare[node]);
    }
    delete pJTree;
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    return trsResult( ret, ret == TRS_OK ? "No errors" :
    "Bad test on Learnin DBN");

}

void initAFactorLearningDBN()
{
    trsReg(func_name, test_desc, test_class, testLearnParamDBN);
}


int CreateEvidences(CDBN *pDBN,
		    pnlVector< pnlVector<CEvidence*> > *myEvidencesForDBN)
{
    //////////////////////////////////////////////////////////////////////
    // Create values for evidence in every slice from t=0 to t=nTimeSlice
    //////////////////////////////////////////////////////////////////////////
    /*
    ifstream inFile;
        char *argv = "../c_pgmtk/tests/testdata/casesDBN";
        ///////////////////////////////////////////////////////////////////////////

        inFile.open(argv);
        if( !inFile)
        {
	    argv = "../testdata/casesDBN";
	    inFile.clear();
	    inFile.open(argv);
	    if (!inFile)
	    {
    	        printf( "can't open file %s\n", argv );
    	        return TRS_FAIL;
	    }
        }

        intVector nslicesVector;
        int nslices = 0;
        //int val;

        char val;
        while(inFile.get(val))
        {
    	//inFile >> val;
    	switch(val)
    	{
    	case EOF: break;
    	case '\t':
    	    {
    		nslices++;
    		break;
    	    }
    	case '\n':
    	    {
    		nslicesVector.push_back(nslices);
    		nslices = 0;
    		break;
    	    }

    	}


        }

        //////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////
        inFile.clear();
        inFile.seekg(0);
        if( !inFile)
        {
    	printf( "can't open file %s\n", argv );
    	return TRS_FAIL;

        }

        int numOfNodes = pDBN->GetNumberOfNodes();
        int nTimeSeries = nslicesVector.size();
        pnlVector<pnlVector<pnlVector<int> > > SeriesOfEv;
        SeriesOfEv.resize(nTimeSeries);
        int series;
        for(series = 0; series <nTimeSeries; series++)
        {
    	(SeriesOfEv[series]).resize(nslicesVector[series]);
        }
        int x;
        int i, j;
        i=0;
        j=0;
        while(inFile.get(val))
        {
    	if(isdigit(val))
    	{
    	    x = atoi(&val);

    	    ((SeriesOfEv[i])[j]).push_back(x);

    	    if(((SeriesOfEv[i])[j]).size() == numOfNodes)
    	    {
    		j++;
    	    }
    	    if(j == (SeriesOfEv[i]).size())
    	    {
    		j=0;
    		i++;
    	    }


    	}
        }

        inFile.close();

        ////////////////////////////////////////////////////////////////////////////
        //Create Evidence for DBN and unrolled BNet for smoothing procedure
        ////////////////////////////////////////////////////////////////////////////


        (*myEvidencesForDBN).resize(nTimeSeries);

        for(series = 0; series < nTimeSeries; series++)
        {
    	((*myEvidencesForDBN)[series]).resize(nslicesVector[series]);
        }



        intVector obsNodes;
        obsNodes.resize(numOfNodes);

        for( i = 0; i < numOfNodes; i++ )
        {
    	obsNodes[i] = i;
        }


        CEvidence *ev;

        for( series = 0; series < nTimeSeries; series++)
        {
    	int slice;
    	for(slice = 0; slice < nslicesVector[series]; slice++)
    	{
    	    int sz = (SeriesOfEv[series])[slice].size();
    	    valueVector dbnValues(sz);
    	    for ( i = 0; i < sz; i++ )
    	    {
    		dbnValues[i].SetInt(((SeriesOfEv[series])[slice])[i]);
    	    }

    	    ev = CEvidence::Create(pDBN->GetModelDomain(), obsNodes, dbnValues);


	((*myEvidencesForDBN)[series])[slice] = ev;
	}
	}

	  ////////////////////////////////////////////////////////////////////////////
	  return 1;
*/
    char *argv = "../c_pgmtk/tests/testdata/MyCasesDBN";

    bool isOk = CEvidence::Load( argv, myEvidencesForDBN, pDBN->GetModelDomain());
    
    if (!isOk)
    {
        argv = "../testdata/MyCasesDBN";
        isOk = CEvidence::Load( argv, myEvidencesForDBN, pDBN->GetModelDomain());
    }

    return isOk ? 1 : 0;
}


int GetDataToCompare(CDBN *pDBN, pnlVector<float*> *data2Compare)
{
    int ret = TRUE;
    /////////////////////////////////////////////////////////////////////////
    //load data from file
    /////////////////////////////////////////////////////////////////////////
    ifstream inFile;
    char *argv = "../c_pgmtk/tests/testdata/apostparams";
    inFile.open(argv);
    if( !inFile)
    {
        argv = "../testdata/apostparams";
	inFile.clear();
        inFile.open(argv);
        if (!inFile)
	{
	    printf( "can't open file %s\n", argv );
	    return TRS_FAIL;
	}
    }
    float data;
    floatVector dataVector;


    do{
	inFile >> data;
	dataVector.push_back(data);
    }while(inFile);
    inFile.close();
    /////////////////////////////////////////////////////////////////////////

    CBNet* pBNet = static_cast<CBNet*>(pDBN->GetStaticModel());
    int nnodes = pBNet->GetNumberOfNodes();

    (*data2Compare).resize(nnodes);

    float* tmpData;
    int offset = 0;
    for( int node = 0; node < nnodes; node++)
    {
	int dataLength;
	const float* data;
	static_cast<CNumericDenseMatrix<float>*>(pBNet->GetFactor(node)->
	    GetMatrix(matTable))->GetRawData(&dataLength, &data);
	tmpData = new float[dataLength];

	for( int pos = 0; pos < dataLength; pos ++, offset++)
	{
	    tmpData[pos] = dataVector[offset];

	}

	(*data2Compare)[node] = tmpData;
    }

    return ret;
}
