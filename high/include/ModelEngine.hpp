#ifndef __MODELENGINE_HPP__
#define __MODELENGINE_HPP__

#include <map>
#include "pnlHighConf.hpp"

class PNLHIGH_API ModelEngine
{
public:
    typedef enum
    {	eChangeNState		= 1
    ,	eChangeParentNState	= 2
    ,	eDelNode		= 4
    ,	eInit			= 8
    ,	eChangeName		= 16
    };

    ModelEngine() {}
    virtual ~ModelEngine() {}
    void Notify(int message, int iNode)
    {
	for(int i = m_apSpy.size(); --i >= 0;)
	{
	    ModelEngine &engine = *m_apSpy[i];

	    if(message & engine.InterestedIn())
	    {
		engine.DoNotify(message, iNode, this);
	    }
	}
    }
    virtual void DoNotify(int message, int iNode, ModelEngine *pObj) = 0;
    virtual int InterestedIn() const { return 127; /* all messages */ }
    void SpyTo(ModelEngine *pSpy)
    {
	pSpy->m_apSpy.push_back(this);
	m_apSpyTo.push_back(pSpy);
    }
    void StopSpyTo(ModelEngine *pSpy)
    {
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
	for(i = m_apSpyTo.size(); --i >= 0; )
	{
	    if(m_apSpyTo[i] == this)
	    {
		m_apSpyTo[i] = m_apSpyTo.back();
		m_apSpyTo.pop_back();
		break;
	    }
	}
    }

private:
    Vector<ModelEngine*> m_apSpy;
    Vector<ModelEngine*> m_apSpyTo;
};

#endif //__MODELENGINE_HPP__
