#include "pnlConfig.hpp"
#include "pnlTimer.hpp"
//#define WINDOWS_TIMER

#ifndef WIN32
#undef WINDOWS_TIMER
#endif

#ifdef WINDOWS_TIMER
#include <windows.h>
#endif

PNL_USING

//////////////////////////////////////////////////////////////////////////

CTimerAndLogWriter* CTimerAndLogWriter::Create()
{
    CTimerAndLogWriter* pTimer = new CTimerAndLogWriter();

    PNL_CHECK_IF_MEMORY_ALLOCATED(pTimer);

    return pTimer;
}
//////////////////////////////////////////////////////////////////////////

void CTimerAndLogWriter::Release(CTimerAndLogWriter** pTimer)
{
    delete *pTimer;

    *pTimer = NULL;
}
//////////////////////////////////////////////////////////////////////////

CTimerAndLogWriter::CTimerAndLogWriter()
                    : m_divider(" ------------------------------------------------ "),
                      m_timeLog( "timing: ", PNL_TIMERLOG_LEVEL_AND_SERVICE )
{
}
//////////////////////////////////////////////////////////////////////////

CTimerAndLogWriter::~CTimerAndLogWriter()
{
}
//////////////////////////////////////////////////////////////////////////


CTimer::CTimer()
{
    m_Duration = m_DurationLast = 0.0;
#ifdef WINDOWS_TIMER
    QueryPerformanceFrequency(((LARGE_INTEGER*)m_InnerUnionBuf) + 0);
#endif
}

void CTimer::Start()
{
#ifdef WINDOWS_TIMER
    QueryPerformanceCounter(((LARGE_INTEGER*)m_InnerUnionBuf) + 1);
#else
    *((clock_t*)m_InnerUnionBuf) = clock();
#endif
}

void CTimer::Stop()
{
#ifdef WINDOWS_TIMER
    LARGE_INTEGER endT;
    QueryPerformanceCounter(&endT);
    m_DurationLast = double(
	endT.QuadPart - ((LARGE_INTEGER*)m_InnerUnionBuf)[1].QuadPart
	)/((LARGE_INTEGER*)m_InnerUnionBuf)[0].QuadPart;
#else
    clock_t endT = clock();

    m_DurationLast = double(endT - *((clock_t*)m_InnerUnionBuf))/CLOCKS_PER_SEC;
#endif

    m_Duration += m_DurationLast;
}


#ifdef PNL_RTTI
const CPNLType CTimerAndLogWriter::m_TypeInfo = CPNLType("CTimerAndLogWriter", &(CPNLBase::m_TypeInfo));

#endif