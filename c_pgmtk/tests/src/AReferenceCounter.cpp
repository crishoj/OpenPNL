/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AReferenceCounter.cpp                                       //
//                                                                         //
//  Purpose:   Test on reference counter object functionality              //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "AReferenceCounter.h"
//#include "pnl_dll.hpp"

PNL_USING

static char func_name[] = "testRefCounter";

static char* test_desc = "Initialize a refcounter";

static char* test_class = "Algorithm";

int testRefCounter()
{

	int ret = TRS_OK;
//    int i;
	void* pObj;
	void* pObj1;
	
	pObj = (void*) rand();
pos:	pObj1 = (void*) rand();
	if (pObj == pObj1) goto pos;
//	printf("pObj = %p\n pObj1 = %p\n", pObj, pObj1);
/*	void* pObj = (void*)0xdeadbeef;
	void* pObj1= (void*)0xbabadeda;
	void* pObj2= (void*)0xbafadadf; 
*/

	float data2[12]={0.1f,  0.2f,  0.3f,  0.4f,  0.5f,  0.6f,
				0.7f,  0.8f,  0.9f,  0.8f,  0.7f,  0.6f};
	int range2[2]={3, 4};

	testRefCounter1 *tm2 = new testRefCounter1(2, range2, data2);
		//static_cast<testRefCounter1*>
		//( CNumericDenseMatrix<float>::Create(2, range2, data2) );

	tm2->AddRef(pObj1);
	tm2->AddRef(pObj);
	int s2=tm2->GetListSize();
	if (s2 != 2) 
	{ret = TRS_FAIL; }
//	printf (" s2 = %d , 2 is right \n", s2);
	

	s2=tm2->GetListSize();
	if (s2 != 2) 
	{ret = TRS_FAIL; }
//	printf (" s2 = %d , 2 is right \n", s2);
	
	tm2->Release(pObj1);
	
	s2=tm2->GetListSize();
	if (s2 !=1 ) 
	{ ret = TRS_FAIL;}
//	printf("Release is completed , s2 = %d \n",s2);

//	tm2->Release(pObj2);
	tm2->Release(pObj);
	//delete tm2;
	
/*	tm2->Release(pObj2);  
	//there should be assertion, it will be done after replacement 
	//assertion in initial code with a error number 
*/
	if (CHECK_DESTR == 0) {ret = TRS_FAIL;}
//	printf ("CHECK_DESTR= %d,   1 is right", CHECK_DESTR);
	
//	delete pObj;
//	delete pObj1;
    return trsResult( ret, ret == TRS_OK ? "No errors" : "Bad matrix data");
	
}

void initARefCounter()
{
    trsReg(func_name, test_desc, test_class, testRefCounter);
}
