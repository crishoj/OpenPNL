#include "pnlConfig.hpp"
#include "pnlLogUsing.hpp"
#include "pnlLog.hpp"
#include "pnlLogDriver.hpp"
#include "pnlLogMultiplexor.hpp"

PNL_BEGIN

class LogUsing {
public:
    LogUsing()
    {
	if(&LogMultiplexor::StdMultiplexor() == 0)
	{
	    m_pMultiplexor = new LogMultiplexor;
	    LogMultiplexor::SetStdMultiplexor(m_pMultiplexor);
	}
	else
	{
	    m_pMultiplexor = 0;
	}

	m_pCOutLogDriver = new LogDrvStream(&std::cout,
	    eLOG_RESULT|eLOG_PROGERR|eLOG_SYSERR,
	    eLOGSRV_LOG|eLOGSRV_PNL);
	m_pLogPotential = new Log("", eLOG_RESULT, eLOGSRV_PNL_POTENTIAL);
#if defined(WIN32) && defined(_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
    }

    ~LogUsing()
    {
	delete m_pLogPotential;
	delete m_pCOutLogDriver;
	delete m_pMultiplexor;
    }

    LogMultiplexor *m_pMultiplexor;
    LogDrvStream *m_pCOutLogDriver;
    Log *m_pLogPotential;
};

static LogUsing logUsing;

void SetDump(const char *fname)
{
    logUsing.m_pCOutLogDriver->Redirect(fname);
}

Log *LogPotential()
{
    return logUsing.m_pLogPotential;
}

PNL_END
