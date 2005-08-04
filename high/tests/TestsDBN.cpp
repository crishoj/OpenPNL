/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      testsDBN.cpp				                                   //
//                                                                         //
//  Purpose:   Provides DBN wrappers testing					           //
//													                       //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlHigh.hpp"
#include "DBN.hpp"  
#include "test_conf.hpp"

PNLW_USING

static char func_name[] = "testDBNWrappers";

static char* test_desc = "Provide all tests for DBN wrappers";

static char* test_class = "Algorithm";
 


int testDBNEvidences()
// this test provides smoothing with DBN
{
	int ret = TRS_OK;
	// node values
	TokArr aChoice = "True False MayBe";
	// DBN creation
	DBN *pDBN; 
	pDBN = new DBN();
	if (pDBN == NULL)
    {
        ret = TRS_FAIL;
        return trsResult( ret, ret == TRS_OK ? "No errors" : 
        "FAIL Net creation");
    }
    else
    {
        std::cout<<"Net creation OK"<<std::endl; 
    };
	
	// prior slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-0", aChoice); 
	pDBN->AddNode(discrete ^ "House-0", aChoice);
	pDBN->AddNode(discrete ^ "Flat-0", aChoice);
	// 1-st slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-1", aChoice); 
	pDBN->AddNode(discrete ^ "House-1", aChoice);
	pDBN->AddNode(discrete ^ "Flat-1", aChoice);
	pDBN->AddNode(discrete ^ "BadNode-0", aChoice);
	pDBN->DelNode("BadNode-0");
	// edges creation
	
	pDBN->AddArc("Street-0","House-0");
	pDBN->AddArc("Street-0","Flat-0");
	
	pDBN->AddArc("Street-1"," House-1");
	pDBN->AddArc("Street-1","Flat-1");
	pDBN->AddArc("Street-0","Street-1"); 
	// setting Boyen-Koller inference algorithm
	pDBN->SetProperty("InferenceAlgorithm","Boyen-Koller");
	// setting number of slices
	pDBN->SetNumSlices(4);
	// evidences creation
    pDBN->EditEvidence("Street-0^True Flat-0^False");
	pDBN->ClearEvid();
	pDBN->GenerateEvidences("30 40 23 45");
	pDBN->ClearEvidBuf();
	pDBN->AddEvidToBuf("Street-0^True Flat-0^False");
	pDBN->AddEvidToBuf("House-1^True Flat-1^False");
	pDBN->AddEvidToBuf("Street-2^True Flat-2^False");
	pDBN->AddEvidToBuf("House-3^True Flat-3^False");
	// saving evidences
	pDBN->SaveEvidBuf("evidencesBuffer1.csv");
	// loading evidences
	pDBN->LoadEvidBuf("evidencesBuffer1.csv");
	// setting inference property: Smoothing
	pDBN->SetProperty("Inference","Smoothing");
	TokArr  tmpJPD;
	tmpJPD = pDBN->GetJPD("Street-3 House-3");
	
#ifdef PRINT_RESULT
	printf("%s",String(tmpJPD).c_str());
#endif
	// model changing
	pDBN->DelArc("Street-0" ,"Flat-0");
	pDBN->DelArc("Street-1","Flat-1");
	pDBN->AddArc("House-0","Flat-0");
	pDBN->AddArc("House-1","Flat-1");

	pDBN->SetProperty("InferenceAlgorithm","1_5SliceJunctionTree");
	// getting request
	tmpJPD = pDBN->GetJPD("Street-0");
	// saving evidences buffer
	pDBN->SaveEvidBuf("evidencesBuffer2.csv");
	
#ifdef PRINT_RESULT
	printf("%s",String(tmpJPD).c_str());
#endif
	// free memory
	delete pDBN;
	return ret;
}

int testDBNSmothing()
// this test provides smoothing with DBN
{
	int ret = TRS_OK;
	// node values
	TokArr aChoice = "True False MayBe";
	// DBN creation
	DBN *pDBN; 
	pDBN = new DBN();
	// prior slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-0", aChoice); 
	pDBN->AddNode(discrete ^ "House-0", aChoice);
	pDBN->AddNode(discrete ^ "Flat-0", aChoice);
	// 1-st slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-1", aChoice); 
	pDBN->AddNode(discrete ^ "House-1", aChoice);
	pDBN->AddNode(discrete ^ "Flat-1", aChoice);
	// edges creation
	pDBN->AddArc("Street-0","House-0");
	pDBN->AddArc("Street-0","Flat-0");
	pDBN->AddArc("Street-1"," House-1");
	pDBN->AddArc("Street-1","Flat-1");
	pDBN->AddArc("Street-0","Street-1"); // setting interface nodes
	// setting number of slices
	pDBN->SetNumSlices(4);
	//evidences creation
	pDBN->EditEvidence("Street-0^True Flat-0^False");
	pDBN->CurEvidToBuf();
	pDBN->EditEvidence("House-1^True Flat-1^False");
	pDBN->CurEvidToBuf();
	pDBN->EditEvidence("Street-2^True Flat-2^False");
	pDBN->CurEvidToBuf();
	pDBN->EditEvidence("House-3^True Flat-3^False");
	pDBN->CurEvidToBuf();
	// setting inference property: Smoothing
	pDBN->SetProperty("Inference","Smoothing");
	// getting request 
	TokArr  tmpJPD = pDBN->GetJPD("Street-3 House-3");

#ifdef PRINT_RESULT
	printf("%s",String(tmpJPD).c_str());
#endif
	// free memory
	delete pDBN;
	return ret;
}

int testDBNTopologicalSort1()
// this test provides smoothing with DBN
{
	int ret = TRS_OK;;
	// node values
	TokArr aChoice = "True False MayBe";
	// DBN creation
	DBN *pDBN; 
	pDBN = new DBN();

	pDBN->AddNode(discrete ^ "Street-1", aChoice); 
	pDBN->AddNode(discrete ^ "House-1", aChoice);
	pDBN->AddNode(discrete ^ "Flat-1", aChoice);

	pDBN->AddNode(discrete ^ "Street-0", aChoice); 
	pDBN->AddNode(discrete ^ "House-0", aChoice);
	pDBN->AddNode(discrete ^ "Flat-0", aChoice);

	// edges creation
	pDBN->AddArc("Street-0","House-0");
	pDBN->AddArc("Street-0","Flat-0");
	pDBN->AddArc("Street-1"," House-1");
	pDBN->AddArc("Street-1","Flat-1");
	pDBN->AddArc("Street-0","Street-1"); // setting interface nodes
	// setting number of slices
	pDBN->SetNumSlices(4);
	//evidences creation
	pDBN->EditEvidence("Street-0^True Flat-0^False");
	pDBN->CurEvidToBuf();
	pDBN->EditEvidence("House-1^True Flat-1^False");
	pDBN->CurEvidToBuf();
	pDBN->EditEvidence("Street-2^True Flat-2^False");
	pDBN->CurEvidToBuf();
	pDBN->EditEvidence("House-3^True Flat-3^False");
	pDBN->CurEvidToBuf();
	// setting inference property: Smoothing
	pDBN->SetProperty("Inference","Smoothing");
	// getting request 
	TokArr  tmpJPD = pDBN->GetJPD("Street-3 House-3");

#ifdef PRINT_RESULT
	printf("%s",String(tmpJPD).c_str());
#endif
	// free memory
	delete pDBN;
	return ret;
}

int testDBNTopologicalSort2()
// this test provides smoothing with DBN
{
	int result = TRS_OK;
	// node values
	TokArr aChoice = "True False MayBe";
	// DBN creation
	DBN *pDBN; 
	pDBN = new DBN();
	
	pDBN->AddNode(discrete ^ "Street-0", aChoice);
	pDBN->AddNode(discrete ^ "Street-1", aChoice);
	pDBN->AddNode(discrete ^ "House-0", aChoice);
	pDBN->AddNode(discrete ^ "House-1", aChoice);
	pDBN->AddNode(discrete ^ "Flat-0", aChoice);
	pDBN->AddNode(discrete ^ "Flat-1", aChoice);

	// edges creation
	pDBN->AddArc("Street-0","House-0");
	pDBN->AddArc("Street-0","Flat-0");
	pDBN->AddArc("Street-1"," House-1");
	pDBN->AddArc("Street-1","Flat-1");
	pDBN->AddArc("Street-0","Street-1"); // setting interface nodes
	// setting number of slices
	pDBN->SetNumSlices(4);
	//evidences creation
	pDBN->EditEvidence("Street-0^True Flat-0^False");
	pDBN->CurEvidToBuf();
	pDBN->EditEvidence("House-1^True Flat-1^False");
	pDBN->CurEvidToBuf();
	pDBN->EditEvidence("Street-2^True Flat-2^False");
	pDBN->CurEvidToBuf();
	pDBN->EditEvidence("House-3^True Flat-3^False");
	pDBN->CurEvidToBuf();
	// setting inference property: Smoothing
	pDBN->SetProperty("Inference","Smoothing");
	// getting request 
	TokArr  tmpJPD = pDBN->GetJPD("Street-3 House-3");

#ifdef PRINT_RESULT
	printf("%s",String(tmpJPD).c_str());
#endif
	// free memory
	delete pDBN;
	return result;
}

int testDBNFixLagSmothing()
// this test provides fixlagsmoothing with DBN
{
	int result = TRS_OK;
	// node values
	TokArr aChoice = "True False MayBe";
	// DBN creation
	DBN *pDBN; 
	pDBN = new DBN();
	// prior slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-0", aChoice); 
	pDBN->AddNode(discrete ^ "House-0", aChoice);
	pDBN->AddNode(discrete ^ "Flat-0", aChoice);
	// 1-st slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-1", aChoice); 
	pDBN->AddNode(discrete ^ "House-1", aChoice);
	pDBN->AddNode(discrete ^ "Flat-1", aChoice);
	// edges creation
	pDBN->AddArc("Street-0","House-0");
	pDBN->AddArc("Street-0","Flat-0");
	pDBN->AddArc("Street-1"," House-1");
	pDBN->AddArc("Street-1","Flat-1");
	pDBN->AddArc("Street-0","Street-1"); // setting interface nodes
	// setting number of slices
	pDBN->SetNumSlices(4);
	//evidences creation
	pDBN->EditEvidence("Street-0^True Flat-0^False");
	pDBN->CurEvidToBuf();
	pDBN->EditEvidence("House-1^True Flat-1^False");
	pDBN->CurEvidToBuf();
	pDBN->EditEvidence("Street-2^True Flat-2^False");
	pDBN->CurEvidToBuf();
	pDBN->EditEvidence("House-3^True Flat-3^False");
	pDBN->CurEvidToBuf();
	// setting inference property:FixLagSmoothing
	pDBN->SetProperty("Inference","fix");
	// getting request 
	TokArr  tmpJPD = pDBN->GetJPD("Street-0 House-0");
#ifdef PRINT_RESULT
	printf("%s",String(tmpJPD).c_str());
#endif
	// free memory
	delete pDBN;
	return result;
}

int testDBNFiltering()
{
	int result = TRS_OK;
	// node values
	TokArr aChoice = "True False MayBe";
	// DBN creation
	DBN *pDBN; 
	pDBN = new DBN();
	// prior slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-0", aChoice); 
	pDBN->AddNode(discrete ^ "House-0", aChoice);
	pDBN->AddNode(discrete ^ "Flat-0", aChoice);
	// 1-st slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-1", aChoice); 
	pDBN->AddNode(discrete ^ "House-1", aChoice);
	pDBN->AddNode(discrete ^ "Flat-1", aChoice);
	// edges creation
	pDBN->AddArc("Street-0","House-0");
	pDBN->AddArc("Street-0","Flat-0");
	pDBN->AddArc("Street-1"," House-1");
	pDBN->AddArc("Street-1","Flat-1");
	pDBN->AddArc("Street-0","Street-1"); // setting interface nodes
	// setting number of slices
	pDBN->SetNumSlices(4);
	//evidences creation
	pDBN->EditEvidence("Street-0^True Flat-0^False");
	pDBN->CurEvidToBuf();
	pDBN->EditEvidence("House-1^True Flat-1^False");
	pDBN->CurEvidToBuf();
	pDBN->EditEvidence("Street-2^True Flat-2^False");
	pDBN->CurEvidToBuf();
	pDBN->EditEvidence("House-3^True Flat-3^False");
	pDBN->CurEvidToBuf();
	// setting inference property:Filtering
	pDBN->SetProperty("Inference","filt");
	// getting request 
	TokArr  tmpJPD = pDBN->GetJPD("Street-0 Flat-0");
#ifdef PRINT_RESULT
	printf("%s",String(tmpJPD).c_str());
#endif
	// free memory
	delete pDBN;
	return result;
}

int testDBNMPE()
{
	int result = TRS_OK;
	// node values
	TokArr aChoice = "True False MayBe";
	// DBN creation
	DBN *pDBN; 
	pDBN = new DBN();
	// prior slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-0", aChoice); 
	pDBN->AddNode(discrete ^ "House-0", aChoice);
	pDBN->AddNode(discrete ^ "Flat-0", aChoice);
	// 1-st slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-1", aChoice); 
	pDBN->AddNode(discrete ^ "House-1", aChoice);
	pDBN->AddNode(discrete ^ "Flat-1", aChoice);
	// edges creation
	pDBN->AddArc("Street-0","House-0");
	pDBN->AddArc("Street-0","Flat-0");
	pDBN->AddArc("Street-1"," House-1");
	pDBN->AddArc("Street-1","Flat-1");
	pDBN->AddArc("Street-0","Street-1"); // setting interface nodes
	// setting number of slices
	pDBN->SetNumSlices(4);
	//evidences creation
	pDBN->EditEvidence("Street-0^True Flat-0^False");
	pDBN->CurEvidToBuf();
	pDBN->EditEvidence("House-1^True Flat-1^False");
	pDBN->CurEvidToBuf();
	pDBN->EditEvidence("Street-2^True Flat-2^False");
	pDBN->CurEvidToBuf();
	pDBN->EditEvidence("House-3^True Flat-3^False");
	pDBN->CurEvidToBuf();

        pDBN->SaveEvidBuf("evidences.csv");
	//  inference property must be Viterbi
	pDBN->SetProperty("Inference","viter");
	// getting request 
	TokArr  tmpMPE = pDBN->GetMPE("Street-1 Flat-1");
#ifdef PRINT_RESULT
	printf("%s",String(tmpMPE).c_str());
#endif
	// free memory
	delete pDBN;
	return result;
} 

int testDBNLearning()
{
	int result = TRS_OK;
	// node values
	TokArr aChoice = "True False MayBe";
	// DBN creation
	DBN *pDBN; 
	pDBN = new DBN();
	// prior slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-0", aChoice); 
	pDBN->AddNode(discrete ^ "House-0", aChoice);
	pDBN->AddNode(discrete ^ "Flat-0", aChoice);
	// 1-st slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-1", aChoice); 
	pDBN->AddNode(discrete ^ "House-1", aChoice);
	pDBN->AddNode(discrete ^ "Flat-1", aChoice);
	// edges creation
	pDBN->AddArc("Street-0","House-0");
	pDBN->AddArc("Street-0","Flat-0");
	pDBN->AddArc("Street-1"," House-1");
	pDBN->AddArc("Street-1","Flat-1");
	pDBN->AddArc("Street-0","Street-1"); // setting interface nodes
	// setting number of slices
	pDBN->SetNumSlices(4);
	//evidences generation
	pDBN->GenerateEvidences("4 6 5 6");

        pDBN->SaveEvidBuf("evidences1.csv");
        pDBN->LoadEvidBuf("evidences1.csv");
	// learning execution
	pDBN->LearnParameters();
	float value = pDBN->GetEMLearningCriterionValue();
	// free memory
	delete pDBN;
	return result;
}


int testDBNSaveLoad()
{
	int result = TRS_OK;
	// node values
	TokArr aChoice = "True False MayBe";
	// DBN creation
	DBN *pDBN; 
	pDBN = new DBN();
	// prior slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-0", aChoice); 
	pDBN->AddNode(discrete ^ "House-0", aChoice);
	pDBN->AddNode(discrete ^ "Flat-0", aChoice);
	// 1-st slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-1", aChoice); 
	pDBN->AddNode(discrete ^ "House-1", aChoice);
	pDBN->AddNode(discrete ^ "Flat-1", aChoice);
	// edges creation
	pDBN->AddArc("Street-0","House-0");
	pDBN->AddArc("Street-0","Flat-0");
	pDBN->AddArc("Street-1"," House-1");
	pDBN->AddArc("Street-1","Flat-1");
	pDBN->AddArc("Street-0","Street-1"); // setting interface nodes
	// setting number of slices
	pDBN->SetNumSlices(4);  
    pDBN->SaveNet("dbn.xml");
    DBN *newDBN;
	newDBN = new DBN();
    newDBN->LoadNet("dbn.xml");
     // free memory
	delete pDBN;
    delete newDBN;

	return result;
}


int testDBN()
{
    int result = TRS_OK;
    // node values
    TokArr aChoice = "True False MayBe";
    // DBN creation
    DBN *pDBN; 
    pDBN = new DBN();
    // prior slice nodes creation 
    pDBN->AddNode(discrete ^ "Street-0", aChoice); 
    pDBN->AddNode(discrete ^ "House-0", aChoice);
    pDBN->AddNode(discrete ^ "Flat-0", aChoice);
    // 1-st slice nodes creation 
    pDBN->AddNode(discrete ^ "Street-1", aChoice); 
    pDBN->AddNode(discrete ^ "House-1", aChoice);
    pDBN->AddNode(discrete ^ "Flat-1", aChoice);
    // edges creation
    pDBN->AddArc("Street-0","House-0");
    pDBN->AddArc("Street-0","Flat-0");
    pDBN->AddArc("Street-1"," House-1");
    pDBN->AddArc("Street-1","Flat-1");
    pDBN->AddArc("Street-0","Street-1"); // setting interface nodes
	// setting number of slices

    pDBN->SetNumSlices(2);
    pDBN->SetPTabular("Street-0^True Street-0^False Street-0^MayBe", "0.3 0.5 0.2");
    pDBN->SetPTabular("House-0^True House-0^False House-0^MayBe", "0.3 0.2 0.5", "Street-0^True"); 
    pDBN->SetPTabular("House-0^True House-0^False House-0^MayBe", "0.3 0.2 0.5", "Street-0^False"); 
    pDBN->SetPTabular("House-0^True House-0^False House-0^MayBe", "0.3 0.2 0.5", "Street-0^MayBe"); 
    
    pDBN->SetPTabular("Flat-0^True Flat-0^False Flat-0^MayBe", "0.5 0.3 0.2", "Street-0^True"); 
    pDBN->SetPTabular("Flat-0^True Flat-0^False Flat-0^MayBe", "0.5 0.3 0.2", "Street-0^False"); 
    pDBN->SetPTabular("Flat-0^True Flat-0^False Flat-0^MayBe", "0.5 0.3 0.2", "Street-0^MayBe"); 

    pDBN->SetPTabular("Street-1^True Street-1^False Street-1^MayBe", "0.1 0.4 0.5", "Street-0^True"); 
    pDBN->SetPTabular("Street-1^True Street-1^False Street-1^MayBe", "0.1 0.4 0.5", "Street-0^False"); 
    pDBN->SetPTabular("Street-1^True Street-1^False Street-1^MayBe", "0.1 0.4 0.5", "Street-0^MayBe"); 

    pDBN->SetPTabular("House-1^True House-1^False House-1^MayBe", "0.3 0.2 0.5", "Street-1^True"); 
    pDBN->SetPTabular("House-1^True House-1^False House-1^MayBe", "0.3 0.2 0.5", "Street-1^False"); 
    pDBN->SetPTabular("House-1^True House-1^False House-1^MayBe", "0.3 0.2 0.5", "Street-1^MayBe"); 

    pDBN->SetPTabular("Flat-1^True Flat-1^False Flat-1^MayBe", "0.5 0.3 0.2", "Street-1^True"); 
    pDBN->SetPTabular("Flat-1^True Flat-1^False Flat-1^MayBe", "0.5 0.3 0.2", "Street-1^False"); 
    pDBN->SetPTabular("Flat-1^True Flat-1^False Flat-1^MayBe", "0.5 0.3 0.2", "Street-1^MayBe"); 

    TokArr st0 = pDBN->GetPTabular("Street-0");
    TokArr h0 = pDBN->GetPTabular("House-0");
    TokArr f0 = pDBN->GetPTabular("Flat-0");
    TokArr st1 = pDBN->GetPTabular("Street-1");
    TokArr h1 = pDBN->GetPTabular("House-1");
    TokArr f1 = pDBN->GetPTabular("Flat-1");

#ifdef PRINT_RESULT
    printf("%s\n",String(st0).c_str());
    printf("%s\n",String(h0).c_str());
    printf("%s\n",String(f0).c_str());
    printf("%s\n",String(st1).c_str());
    printf("%s\n",String(h1).c_str());
    printf("%s\n",String(f1).c_str());
#endif
    TokArr st0p = pDBN->GetParents("Street-0");
    TokArr h0p = pDBN->GetParents("House-0");
    TokArr f0p = pDBN->GetParents("Flat-0");
    TokArr st1p = pDBN->GetParents("Street-1");
    TokArr h1p = pDBN->GetParents("House-1");
    TokArr f1p = pDBN->GetParents("Flat-1");

#ifdef PRINT_RESULT
    printf("\nParents\n");
    printf("%s\n",String(st0p).c_str());
    printf("%s\n",String(h0p).c_str());
    printf("%s\n",String(f0p).c_str());
    printf("%s\n",String(st1p).c_str());
    printf("%s\n",String(h1p).c_str());
    printf("%s\n",String(f1p).c_str());
#endif
    TokArr st0c = pDBN->GetChildren("Street-0");
    TokArr h0c = pDBN->GetChildren("House-0");
    TokArr f0c = pDBN->GetChildren("Flat-0");
    TokArr st1c = pDBN->GetChildren("Street-1");
    TokArr h1c = pDBN->GetChildren("House-1");
    TokArr f1c = pDBN->GetChildren("Flat-1");

#ifdef PRINT_RESULT
    printf("\nChildren\n");
    printf("%s\n",String(st0c).c_str());
    printf("%s\n",String(h0c).c_str());
    printf("%s\n",String(f0c).c_str());
    printf("%s\n",String(st1c).c_str());
    printf("%s\n",String(h1c).c_str());
    printf("%s\n",String(f1c).c_str());
#endif    
    
    TokArr st0n = pDBN->GetNeighbors("Street-0");
    TokArr h0n = pDBN->GetNeighbors("House-0");
    TokArr f0n = pDBN->GetNeighbors("Flat-0");    
    TokArr st1n = pDBN->GetNeighbors("Street-1");
    TokArr h1n = pDBN->GetNeighbors("House-1");
    TokArr f1n = pDBN->GetNeighbors("Flat-1");

#ifdef PRINT_RESULT        
    printf("\nNeighbors\n");
    printf("%s\n",String(st0n).c_str());
    printf("%s\n",String(h0n).c_str());
    printf("%s\n",String(f0n).c_str());
    printf("%s\n",String(st1n).c_str());
    printf("%s\n",String(h1n).c_str());
    printf("%s\n",String(f1n).c_str());
#endif

    // free memory
    delete pDBN;
    return result;
} 

int testNodeType()
{
    int result = TRS_OK;
	// node values
	TokArr aChoice = "True False MayBe";
	// DBN creation
	DBN *pDBN; 
	pDBN = new DBN();
	// prior slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-0", aChoice); 
	pDBN->AddNode(discrete ^ "House-0", aChoice);
	pDBN->AddNode(discrete ^ "Flat-0", aChoice);
	// 1-st slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-1", aChoice); 
	pDBN->AddNode(discrete ^ "House-1", aChoice);
	pDBN->AddNode(discrete ^ "Flat-1", aChoice);
	// edges creation
	pDBN->AddArc("Street-0","House-0");
	pDBN->AddArc("Street-0","Flat-0");
	pDBN->AddArc("Street-1"," House-1");
	pDBN->AddArc("Street-1","Flat-1");
	pDBN->AddArc("Street-0","Street-1"); // setting interface nodes
	// setting number of slices
	pDBN->SetNumSlices(4);  
    TokArr n0t = pDBN->GetNodeType("Street-0");
    TokArr n1t = pDBN->GetNodeType("House-2");
    TokArr n2t = pDBN->GetNodeType("Flat-1");
    TokArr n3t = pDBN->GetNodeType("Street-3");
    TokArr n5t = pDBN->GetNodeType("House-0");
    TokArr n6t = pDBN->GetNodeType("Flat-3");

    printf("\nNodes types\n");
    printf("%s\n",  String(n0t).c_str());
    printf("%s\n",String(n1t).c_str());
    printf("%s\n",String(n2t).c_str());
    printf("%s\n",String(n3t).c_str());
    printf("%s\n",String(n5t).c_str());
    printf("%s\n",String(n6t).c_str());
     // free memory
	delete pDBN;

	return result;
}

int testUnRoll()
{
    int result = TRS_OK;
	// node values
	TokArr aChoice = "True False MayBe";
	// DBN creation
	DBN *pDBN; 
	pDBN = new DBN();
	// prior slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-0", aChoice); 
	pDBN->AddNode(discrete ^ "House-0", aChoice);
	pDBN->AddNode(discrete ^ "Flat-0", aChoice);
	// 1-st slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-1", aChoice); 
	pDBN->AddNode(discrete ^ "House-1", aChoice);
	pDBN->AddNode(discrete ^ "Flat-1", aChoice);
	// edges creation
	pDBN->AddArc("Street-0","House-0");
	pDBN->AddArc("Street-0","Flat-0");
	pDBN->AddArc("Street-1"," House-1");
	pDBN->AddArc("Street-1","Flat-1");
	pDBN->AddArc("Street-0","Street-1"); // setting interface nodes
	// setting number of slices
	pDBN->SetNumSlices(4); 
	
	BayesNet *pBNet;
    pBNet = pDBN->Unroll(); 

	pBNet->AddEvidToBuf("Street-0^True");

	TokArr jpdres = pBNet->GetJPD("House-0");

    printf("%s",String(jpdres).c_str());
	
     // free memory
	delete pDBN;
	delete pBNet;

	return result;
}

int testPropertiesDBN()
{
	int res = TRS_OK;
    DBN dbn;
	//Adding new network property
	dbn.SetProperty("date","20july2005");
	//Network proprty value request
	String value = dbn.GetProperty("date");
	printf("\n%s\n",value.c_str());
	return res;
}


int testPNLObjectsRequestsDBN()
{
    int result = TRS_OK;
	// node values
	TokArr aChoice = "True False MayBe";
	// DBN creation
	DBN *pDBN; 
	pDBN = new DBN();
	// prior slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-0", aChoice); 
	pDBN->AddNode(discrete ^ "House-0", aChoice);
	pDBN->AddNode(discrete ^ "Flat-0", aChoice);
	// 1-st slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-1", aChoice); 
	pDBN->AddNode(discrete ^ "House-1", aChoice);
	pDBN->AddNode(discrete ^ "Flat-1", aChoice);
	// edges creation
	pDBN->AddArc("Street-0","House-0");
	pDBN->AddArc("Street-0","Flat-0");
	pDBN->AddArc("Street-1"," House-1");
	pDBN->AddArc("Street-1","Flat-1");
	pDBN->AddArc("Street-0","Street-1"); // setting interface nodes
	// setting number of slices
	pDBN->SetNumSlices(4); 

	pDBN->AddEvidToBuf("Street-0^True Flat-0^False");
	pDBN->AddEvidToBuf("House-1^True Flat-1^False");
	pDBN->AddEvidToBuf("Street-2^True Flat-2^False");
	pDBN->AddEvidToBuf("House-3^True Flat-3^False");

	pnl::CDBN& pnlDBN = pDBN->Model();
	pnl::CDynamicInfEngine& pnlInfEngine = pDBN->Inference();
	pnl::CDynamicLearningEngine& pnlLearningEngine = pDBN->Learning();
	pnl::pEvidencesVecVector pnlEvidences = pDBN->GetPNLEvidences();
	 // free memory
	delete pDBN;

	return result;
	
}

int testDBNWrappers()
{
	int res = TRS_OK;
	res = testNodeType() & res;
	res = testUnRoll() & res;
	res = testPropertiesDBN() & res;
	res = testPNLObjectsRequestsDBN() & res;
	res = testDBNTopologicalSort1() & res;
	res = testDBNTopologicalSort2() & res;
	res = testDBNEvidences() & res;
	res = testDBNSmothing() & res;
	res = testDBNFixLagSmothing() & res;
	res = testDBNFiltering() & res;
	res = testDBNMPE() & res;
	res = testDBNLearning() & res;
	res = testDBNSaveLoad() & res;
	res = testDBN() & res;

	return trsResult( res, res == TRS_OK ? "No errors"
	: "Bad test on DBN wrappers");
}

void initTestsDBNWrappers()
{
    trsReg(func_name, test_desc, test_class, testDBNWrappers);
}