/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlLogDriver.hpp                                            //
//                                                                         //
//  Purpose:   Base and derived classes. Writes log to a file              //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __PNLLOGDRIVER_HPP__
#define __PNLLOGDRIVER_HPP__

#pragma warning (push, 3 )
#include <fstream>
#pragma warning(pop)
#include "pnlConfig.hpp"
#include "pnlTypeDefs.hpp"

PNL_BEGIN

// level
enum
{   eLOG_RESULT  = 1
,   eLOG_SYSERR  = 2
,   eLOG_PROGERR = 4   // program error
,   eLOG_WARNING = 8
,   eLOG_NOTICE  = 16
,   eLOG_INFO    = 32
,   eLOG_DEBUG   = 64
,   eLOG_ALL     = 127
};

// service
enum
{   eLOGSRV_LOG   = 1 // log system
,   eLOGSRV_EXCEPTION_HANDLING = 2
,   eLOGSRV_TEST1 = 4 // for testing purposes only. Don't use it!
,   eLOGSRV_TEST2 = 8 // for testing purposes only. Don't use it!
,   eLOGSRV_PNL_POTENTIAL = 16
,   eLOGSRV_PNL   = eLOGSRV_PNL_POTENTIAL
,   eLOGSRV_ALL   = 31
};

class PNL_API LogDriver
{
public: // TYPES
    enum EConfCmd
    {   eADD
    ,   eDELETE
    ,   eSET
    };
    
public: // USER INTERFACE
    LogDriver(int levelMask = 0, int serviceMask = 0);
    virtual ~LogDriver();
    virtual void Configure(EConfCmd command, int level = eLOG_ALL,
        int service = eLOGSRV_ALL);

    // dump configure of `driver` to this
    void WriteConfiguration(const char *prefix, const LogDriver &driver);

public: // FUNCTIONS FOR USING BY LOGGING SUBSYSTEM
    virtual void WriteString(const char* pStr, int strLen = -1) = 0;
    bool isAllowedWriting(int level, int service) const;

private:
    int GetIMatching(int *piMatching, int level, int service);
    LogDriver& operator=(const LogDriver&)
    { return *this; } // deny copying

private:
    intVector m_aLevel;
    intVector m_aService;
    int m_iInMultiplexor;
};

// LogDriver associated with stream
class PNL_API LogDrvStream: public LogDriver
{
public:
    LogDrvStream(std::ostream* pStream, int levelMask = 0, int serviceMask = 0)
        :LogDriver(levelMask, serviceMask),
        m_pStream(pStream), m_bDeleteOnExit(false)
    {}

    LogDrvStream(const char *pFilename, int levelMask = 0, int serviceMask = 0);

    void Redirect(const char *fname);
    void Redirect(std::ostream* pStream);

    virtual ~LogDrvStream()
    {
        ForgiveStream();
    }

    virtual void WriteString(const char* pStr, int strLen = 0);

private:
    LogDrvStream& operator=(const LogDrvStream&)
    { return *this; } // deny copying

    void SetStream(std::ostream *pStream)
    {
        ForgiveStream();
        m_pStream = pStream;
    }

    void ForgiveStream();

private:
    std::ostream *m_pStream;
    bool m_bDeleteOnExit;
};

// Same as LogDrvStream but configured via ConfigureSystem()
class PNL_API LogDrvSystem: public LogDrvStream
{
public:
    LogDrvSystem(std::ostream* pStream, int levelMask = eLOG_ALL,
        int serviceMask = eLOGSRV_ALL)
        :LogDrvStream(pStream, levelMask, serviceMask)
    {}

    LogDrvSystem(const char *pFilename, int levelMask = eLOG_ALL,
        int serviceMask = eLOGSRV_ALL)
        :LogDrvStream(pFilename, levelMask, serviceMask)
    {}

    virtual void Configure(EConfCmd command, int level = eLOG_ALL,
        int service = eLOGSRV_ALL);
    
    void ConfigureSystem(EConfCmd command, int level = eLOG_ALL,
        int service = eLOGSRV_ALL);

private:
    LogDrvSystem& operator=(const LogDrvSystem&)
    { return *this; } // deny copying
};

PNL_END

#endif // include guard
