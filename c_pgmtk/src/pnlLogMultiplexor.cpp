/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlLogMultiplexor.cpp                                       //
//                                                                         //
//  Purpose:   Implementation of log multiplexor                           //
//             (configuration and multiplexing)                            //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlLogDriver.hpp"
#include "pnlLogMultiplexor.hpp"

PNL_USING

LogMultiplexor* LogMultiplexor::s_pStdMultiplexor = 0;

void
LogMultiplexor::DriverReconfigured(LogDriver *pDriver)
{
    ++m_iUpdate;
}

LogMultiplexor::~LogMultiplexor()
{
    int i;

    for(i = m_apHead.size(); --i >= 0;)
    {
        if(m_apHead[i])
        {
            assert(0 && "All Log()s must be detached before deleting Multiplexor");
        }
    }

    for(i = m_apDriver.size(); --i >= 0;)
    {
        delete m_apDriver[i];
    }
}

int
LogMultiplexor::AttachDriver(LogDriver* pDriver)
{
    m_apDriver.push_back(pDriver);
    ++m_iUpdate;
    return m_apDriver.size() - 1;
}

int
LogMultiplexor::AttachLogger(Log* pLog)
{
    m_apHead.push_back(pLog);
    ++m_iUpdate;
    return m_apHead.size() - 1;
}

void
LogMultiplexor::DetachDriver(LogDriver* pDriver, int iDriver)
{
    assert(iDriver < m_apDriver.size() && m_apDriver[iDriver] == pDriver);

    ++m_iUpdate;
    m_apDriver[iDriver] = NULL;
}

void
LogMultiplexor::DetachLogger(Log* pLog, int iLogger)
{
    assert(iLogger < m_apHead.size() && m_apHead[iLogger] == pLog);

    ++m_iUpdate;
    m_apHead[iLogger] = NULL;
}

bool
LogMultiplexor::GetBDenyOutput(int *piUpdate, int level, int service)
{
    *piUpdate = iUpdate();
    for(int i = m_apDriver.size(); --i >= 0;)
    {
        if(m_apDriver[i] && m_apDriver[i]->isAllowedWriting(level, service))
        {
            return false;
        }
    }
    return true;
}

void LogMultiplexor::Configure(LogDriver::EConfCmd command, int level, int service)
{
    for(int i = m_apDriver.size(); --i >= 0;)
    {
        if(m_apDriver[i])
	{
	    m_apDriver[i]->Configure(command, level, service);
	}
    }
}

void LogMultiplexor::WriteString(int level, int service, const char *str, int strLen)
{
    for(int i = m_apDriver.size(); --i >= 0;)
    {
        if(m_apDriver[i] && m_apDriver[i]->isAllowedWriting(level, service))
        {
            m_apDriver[i]->WriteString(str, strLen);
        }
    }
}

void LogMultiplexor::WriteConfiguration() const
{
    int sz = m_apDriver.size();

    for(int i = sz*sz; --i >= 0;)
    {
	if(m_apDriver[i/sz] && m_apDriver[i%sz])
	{
	    m_apDriver[i/sz]->WriteConfiguration((i/sz == (i % sz))
		? "config of this":"config of another", *m_apDriver[i % sz]);
	}
    }
}
