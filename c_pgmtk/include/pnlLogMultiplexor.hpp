/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlLogMultiplexor.hpp                                       //
//                                                                         //
//  Purpose:   Log system configuring class                                //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLLOGMULTIPLEXOR_HPP__
#define __PNLLOGMULTIPLEXOR_HPP__

#include "pnlConfig.hpp"
#include "pnlTypeDefs.hpp"
#ifndef __PNLLOGDRIVER_HPP__
#include "pnlLogDriver.hpp"
#endif

PNL_BEGIN

// FORWARDS
class Log;

// Multiplexor. Designed to be singleton object
// Distributes messages between drivers in compliance with 'level' and 'service'
// Configures all drivers (except LogDrvSystem) via Configure()
class PNL_API LogMultiplexor
{
public: // USER INTERFACE
    void Configure(LogDriver::EConfCmd command, int level = eLOG_ALL,
        int service = eLOGSRV_ALL);
    void WriteConfiguration() const;
    
    static LogMultiplexor& StdMultiplexor() { return *s_pStdMultiplexor; }
    static void SetStdMultiplexor(LogMultiplexor *pMultiplexor)
    { s_pStdMultiplexor = pMultiplexor; }
    
public: // PUBLIC INTERFACE FOR LOGGING SUBSYSTEM (Log, LogDriver)
    LogMultiplexor(): m_iUpdate(0) {}
    ~LogMultiplexor();
    
    int AttachDriver(LogDriver* pDriver);
    int AttachLogger(Log* pLog);
    void DetachDriver(LogDriver* pDriver, int iDriver);
    void DetachLogger(Log* pLog, int iLogger);
    int iUpdate() const { return m_iUpdate; }
    bool GetBDenyOutput(int *piUpdate, int level, int service);
    void DriverReconfigured(LogDriver *pDriver);
    void WriteString(int level, int service, const char* pStr, int strLen = -1);
    
private:
    LogMultiplexor& operator=(const LogMultiplexor&)
    { return *this; } // deny copying
    
private: // DATA
    static LogMultiplexor *s_pStdMultiplexor;
    
    pnlVector<LogDriver*> m_apDriver;
    pnlVector<Log*> m_apHead;
    int m_iUpdate;
};

PNL_END

#endif // include guard
