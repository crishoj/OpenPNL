#include "ModelEngine.hpp"

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
