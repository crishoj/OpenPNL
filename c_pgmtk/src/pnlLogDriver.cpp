/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlLogDriver.cpp                                            //
//                                                                         //
//  Purpose:   Implementation of log's writing out:                        //
//               base class and derived classes                            //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#pragma warning (push, 3 )
#include <sstream>
#include <fstream>
#pragma warning(pop)
#include "pnlLogMultiplexor.hpp"
#include "pnlLogDriver.hpp"

PNL_USING

LogDriver::LogDriver(int levelMask, int serviceMask)
{
    m_iInMultiplexor = LogMultiplexor::StdMultiplexor().AttachDriver(this);
    if(levelMask && serviceMask)
    {
        m_aLevel.push_back(levelMask);
        m_aService.push_back(serviceMask);
    }
}

LogDriver::~LogDriver()
{
    LogMultiplexor::StdMultiplexor().DetachDriver(this, m_iInMultiplexor);
}

bool
LogDriver::isAllowedWriting(int level, int service) const
{
    for(int i = m_aLevel.size(); --i >= 0;)
    {
        if((m_aLevel[i] & level) && (m_aService[i] & service))
        {
            return true;
        }
    }
    return false;
}

int
LogDriver::GetIMatching(int *piMatching, int level, int service)
{
    *piMatching = -1;
    for(int i = m_aLevel.size(); --i >= 0;)
    {
        if(m_aLevel[i] == level)
        {
            if(m_aService[i] == service)
            {
                *piMatching = i;
                return 1;
            }
            else
            {
                *piMatching = i;
            }
        }
        else if(m_aService[i] == service)
        {
            *piMatching = i;
        }
    }

    return (*piMatching < 0) ? 0:2;
}

void
LogDriver::Configure(EConfCmd command, int level, int service)
{
    int iMatching, i;
    bool bChanged = false;

    switch(command)
    {
    case eSET:
        m_aLevel.resize(1);
        m_aService.resize(1);
        m_aLevel[0] = level;
        m_aService[0] = service;
        LogMultiplexor::StdMultiplexor().DriverReconfigured(this);
        break;
    case eADD:
        switch(GetIMatching(&iMatching, level, service))
        {
        case 0:
            m_aLevel.push_back(level);
            m_aService.push_back(service);
            LogMultiplexor::StdMultiplexor().DriverReconfigured(this);
            break;
        case 1:
            break;
        case 2:// partially
            if(m_aLevel[iMatching] == level)
            {
                m_aService[iMatching] |= service;
            }
            else
            {
                m_aLevel[iMatching] |= level;
            }
            LogMultiplexor::StdMultiplexor().DriverReconfigured(this);
            break;
        }
        break;
        case eDELETE:
            for(i = m_aLevel.size(); --i >= 0;)
            {
                if(!(m_aLevel[i] & level) && !(m_aService[i] & service))
                {
                    continue;
                }

                bChanged = true;
                if((m_aLevel[i] & level) == m_aLevel[i])
                {
                    if((m_aService[i] & service) == m_aService[i])
                    {
                        m_aLevel.erase(m_aLevel.begin() + i);
                        m_aService.erase(m_aService.begin() + i);
                    }
                    else
                    {
                        m_aService[i] &= ~service;
                    }
                    continue;
                }
                else if((m_aService[i] & service) == m_aService[i])
                {
                    m_aLevel[i] &= ~level;
                    continue;
                }

                m_aLevel.push_back(m_aLevel[i] & (~level));
                m_aService.push_back(m_aService[i]);
                m_aLevel[i] &= level;
                m_aService[i] &= ~service;
            }
            if(bChanged)
            {
                LogMultiplexor::StdMultiplexor().DriverReconfigured(this);
            }
            break;
        default:
            break;
    }
}

void
LogDriver::WriteConfiguration(const char *prefix, const LogDriver &driver)
{
    char buf[256];

    if(strlen(prefix) > 180)
    {
        return;
    }

    for(int i = driver.m_aLevel.size(); --i >= 0;)
    {
        sprintf(buf, "%s: %i(level)*%i(serv)\n", prefix, driver.m_aLevel[i], driver.m_aService[i]);
        WriteString(buf);
    }
}

LogDrvStream::LogDrvStream(const char *pFilename, int levelMask, int serviceMask)
:LogDriver(levelMask, serviceMask)
{
    m_bDeleteOnExit = false;
    m_pStream = new std::ofstream(pFilename, std::ios::out);
    m_bDeleteOnExit = true;
}

void LogDrvStream::Redirect(const char *fname)
{
    SetStream(new std::ofstream(fname, std::ios::out));
    m_bDeleteOnExit = true;
}

void LogDrvStream::Redirect(std::ostream* pStream)
{
    SetStream(pStream);
    m_bDeleteOnExit = false;
}

void LogDrvStream::ForgiveStream()
{
    if(m_pStream)
    {
        m_pStream->flush();
    }

    if(m_bDeleteOnExit)
    {
        delete m_pStream;
    }
}

void LogDrvStream::WriteString(const char* pStr, int strLen)
{
    if(pStr[strLen] == 0)
    {
        (*m_pStream) << pStr;
    }
    else if(strLen > 0)
    {
        for(;--strLen >= 0; ++pStr)
        {
            (*m_pStream) << *pStr;
        }
    }

    m_pStream->flush();
}

void LogDrvSystem::Configure(EConfCmd, int /* level = eLOG_ALL */,
                             int /* service = eLOGSRV_ALL */)
{
    return;
}
