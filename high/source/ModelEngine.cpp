#include "ModelEngine.hpp"

PNLW_BEGIN

ModelEngine::~ModelEngine()
{
    int i;

    for(i = m_apSpy.size(); --i >= 0;)
    {
	m_apSpy[i]->StopSpyTo(this);
    }

    for(i = m_apSpyTo.size(); --i >= 0;)
    {
	StopSpyTo(m_apSpyTo[i]);
    }
}

void ModelEngine::StopSpyTo(ModelEngine *pSpy)
{
    static const char fname[] = "ModelEngine::StopSpyTo";
    int i;
    for(i = pSpy->m_apSpy.size(); --i >= 0; )
    {
	if(pSpy->m_apSpy[i] == this)
	{
	    pSpy->m_apSpy[i] = pSpy->m_apSpy.back();
	    pSpy->m_apSpy.pop_back();
	    break;
	}
    }
    if(i < 0)
    {
	ThrowInternalError("Object not found", fname);
    }
    for(i = m_apSpyTo.size(); --i >= 0; )
    {
	if(m_apSpyTo[i] == pSpy)
	{
	    m_apSpyTo[i] = m_apSpyTo.back();
	    m_apSpyTo.pop_back();
	    break;
	}
    }
    if(i < 0)
    {
	ThrowInternalError("Object not found", fname);
    }
}

PNLW_END
