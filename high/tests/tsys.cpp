/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      tsys.cpp                                                    //
//                                                                         //
//  Purpose:   Starts tests                                                //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "trsapi.h"
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include "pnlHigh.hpp"
#include "Tokens.hpp"
#include "TestPnlHigh.hpp"
#include "pnlException.hpp"
#include "pnlFakePtr.hpp"

#if defined(WIN32) && defined(_DEBUG)
#define CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#define FUNC_ENTITY(A) A, #A

void initTestsDBNWrappers();
void initTestsTokens();
void initTestsGaussianBNet();
void initTestsLIMID();
void initTestsMRF();
void initTestsNumeration();
void initTestsSoftMax();
void initTestsCondGaussian();


struct {
    void (*pFunc)();
    const char *fname;
    int testByDefault;
} aFunction[] = {
  
    FUNC_ENTITY(initTestsSoftMax),		               1,
    FUNC_ENTITY(initTestsDBNWrappers),				   1,
	FUNC_ENTITY(initTestsLIMID),                       1,
	FUNC_ENTITY(initTestsGaussianBNet),                1,
	FUNC_ENTITY(initTestsNumeration),	               1,
	FUNC_ENTITY(initTestsCondGaussian),                1,
	FUNC_ENTITY(initTestsTokens),					   1,
	FUNC_ENTITY(initTestsMRF),                         1,

    0, 0 // don't delete this line!
};

const int nInitFunction = sizeof(aFunction)/sizeof(aFunction[0]);

static void processFile(FILE *fp, int *v)
{
    int i;
    char buf[90];
    
    memset(v, 0, nInitFunction*sizeof(int));
    
    for(;;)
    {
        if(fscanf(fp, "%80s\n", buf) != 1)
        {
            break;
        }
        
        if(!buf[0] || buf[0] == '#')
        {
            continue;
        }
        
        for(i = 0; aFunction[i].fname; ++i)
        {
            if(strcmp(buf, aFunction[i].fname) == 0
                || (memcmp(aFunction[i].fname, "init", 4) == 0
                && strcmp(buf, aFunction[i].fname + 4) == 0))
            {
                v[i] = 1;
                break;
            }
        }
    }
}

static void processList(int *pArgc, char **argv, int *v)
{
    const char *fname;
    int nshift;
    
    if(argv[1][2])
    {
        fname = argv[1] + 2;
        nshift = 1;
    }
    else if(*pArgc > 2)
    {
        fname = argv[2];
        nshift = 2;
    }
    else
    {
        fprintf(stderr, "processList(): bad arguments\n");
        return;
    }
    
    FILE *fp;
    bool bNeedfclose;
    
    if(*fname == '-' && !fname[1])
    {
        bNeedfclose = false;
        fp = stdin;
    }
    else
    {
        bNeedfclose = true;
        if((fp = fopen(fname, "r")) == NULL)
        {
            perror(fname);
            fprintf(stderr, "processList(): arguments isn't handled");
            return;
        }
    }
    
    for(int i = 1 + nshift; i <= *pArgc; ++i)
    {
        argv[i - nshift] = argv[i];
    }
    *pArgc -= nshift;
    
    processFile(fp, v);
    
    if(bNeedfclose)
    {
        fclose(fp);
    }
}

int main(int argc, char* argv[])
{
    int abFuncRun[nInitFunction];
    int i;
  
    for(i = nInitFunction; --i >= 0;)
    {
        abFuncRun[i] = aFunction[i].testByDefault;
    }
    
    if(argc > 1 && argv[1][0] == '-' && tolower(argv[1][1]) == 's')
    {
        processList(&argc, argv, abFuncRun);
    }

    if(argc > 1 && argv[1][0] == '-' && tolower(argv[1][1]) == 'a')
    {
	for(i = 0; i < nInitFunction - 1; ++i)
	{
	    if(argv[1][1] == 'a' || aFunction[i].testByDefault)
	    {
		    printf("%s\n", aFunction[i].fname);
	    }
	}

	return 1;
    }
    
    for(i = nInitFunction; --i >= 0;)
    {
        if(abFuncRun[i] && aFunction[i].fname)
        {
            aFunction[i].pFunc();
        }
    }

    try
    {
	trsRun(argc, argv);
   }
    
    catch(pnl::CException ex)
    {
        std::cout << ex.GetMessage() << std::endl;
	fprintf(stderr, "\n###### PNL's exception: %s\n\n", ex.GetMessage());
        return 1;
    }

    catch(...)
    {
	fprintf(stderr, "\n###### got unknown exception\n");
        return 2;
    }

#if defined(_DEBUG) && defined(WIN32)
    _CrtDumpMemoryLeaks( );
#endif
    
    return 0;
}
