/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      ALog.cpp                                                    //
//                                                                         //
//  Purpose:   Test of Log subsystem                                       //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include "pnlLog.hpp"
#include "pnlLogDriver.hpp"
#include "trsapi.h"

PNL_USING

int testLog();

void initALog()
{
    trsReg("testLog", "Open two streams and output to it thru pnl::Log interface",
        "subsystem", testLog);
}


int testLog()
{
    LogDrvSystem ldSystem("LogDrvSystem.log", eLOG_ALL, eLOGSRV_ALL);
    LogDrvStream ldStream("LogDrvStream.log",
        eLOG_RESULT|eLOG_SYSERR|eLOG_PROGERR|eLOG_WARNING,
        eLOGSRV_TEST1|eLOGSRV_TEST2);
    
    LogMultiplexor::StdMultiplexor().WriteConfiguration();
    
    {
        Log logH1("Testing Log: ", eLOG_RESULT, eLOGSRV_LOG);
        
        logH1 << "Log Started\n";
	logH1 << "Testing Types\n";
        
        logH1.SetService(eLOGSRV_TEST1);
        logH1 << "Printing numbers: " << (int)18 << ' ' << 18.1818 << '\n';

	int x;

	logH1 << "void*,void* : " << (void*)0 << ", " << (void*)&x << '\n';
	logH1 << "int* : " << &x << '\n';
        
        ldStream.Configure(LogDriver::eDELETE);
        
        logH1 << "Output only to systemLog\n\n";
    }
    
    Log logH2("Testing Log2: ", eLOG_RESULT, eLOGSRV_TEST2);
    
    logH2 << "Output2 only to systemLog\n\n";
    ldStream.Configure(LogDriver::eADD, eLOG_ALL, eLOGSRV_TEST1|eLOGSRV_TEST2);
    logH2 << "Output2 to Test2\n";
    logH2.SetService(eLOGSRV_TEST1);
    // check for big buffer printing (by default buffer is 1024 bytes length)
    {
        char buf[6144];
        
        for(int i = sizeof(buf); --i >= 0;)
        {
            buf[i] = '*';
            if(!(i % 1536))
            {
                buf[i] = '\n';
            }
        }
        buf[sizeof(buf) - 1] = 0;
        logH2.printf("Test printf only to systemLog: '%s', '%i', '%lg'\nwriting long string test: %s\n",
            "%s testing", 123, (double)123.456, buf);
    }
    
    ldStream.Configure(LogDriver::eDELETE, eLOG_ALL, eLOGSRV_TEST2);
    LogMultiplexor::StdMultiplexor().WriteConfiguration();
    
    {// alias test
        Log("Testing alias: ", eLOG_ALL, eLOGSRV_ALL).Register("alias::test");
        Log("Testing alias default: ", eLOG_ALL, eLOGSRV_TEST2).Register("");
        
        Log("alias::test1") << "Should be in syslog only\n";
        Log("alias::test") << "Should be anywhere\n";
        Log("") << "Should be in syslog only\n";
        
        Log("Testing alias default: ", eLOG_ALL, eLOGSRV_TEST1).Register("");
        Log("") << "Should be anywhere\n";
    }
    
    return TRS_OK;
}

