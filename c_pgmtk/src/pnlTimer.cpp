#include "pnlConfig.hpp"
#include "pnlTimer.hpp"

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
