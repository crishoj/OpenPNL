#ifndef __MODELENGINE_HPP__
#define __MODELENGINE_HPP__

#include <map>
#ifndef __PNLHIGHCONFIG_HPP__
#include "pnlHighConf.hpp"
#endif

PNLW_BEGIN

class ModelEngine;

class PNLHIGH_API Message
{
ALLOW_TESTING
public:
    typedef enum
    {	eChangeNState		= 1
    ,	eChangeParentNState	= 2
    ,	eMSGDelNode		= 4
    ,	eInit			= 8
    ,	eChangeName		= 16
    ,	eSetModelInvalid	= 32
    };

    Message(int messageId, ModelEngine *sender, int i = -1):
    m_MessageId(messageId), m_Sender(sender), m_IntArg(i) {}

    int MessageId()	    const { return m_MessageId; }
    ModelEngine &Sender()   const { return *m_Sender; }
    int IntArg()	    const { return m_IntArg; }
    virtual ~Message() {}

private:
    int m_MessageId;
    ModelEngine *m_Sender;
    int m_IntArg;
};

// ModelEngine is base class for all interacting entities under cover
// of user class (Graph object or Distribution object for example)
//
// Purpose of ModelEngine - to deliver message about model changing to all
// objects who interested in it
class PNLHIGH_API ModelEngine
{
ALLOW_TESTING
public:
    ModelEngine(): m_MaskMessage(0) {}
    virtual ~ModelEngine();
    void Notify(int message, int iNode = -1)
    {
	Message msg(message, this, iNode);
	MaskAddOrDelete(message, true);// temporary mask message
	for(int i = m_apSpy.size(); --i >= 0;)
	{
	    ModelEngine &engine = *m_apSpy[i];

	    if(message & engine.InterestedIn() & (~engine.Mask()))
	    {
		engine.DoNotify(msg);
	    }
	}
	MaskAddOrDelete(message, false);// unmask message, previously masked
    }
    void SpyTo(ModelEngine *pSpy)
    {
	pSpy->m_apSpy.push_back(this);
	m_apSpyTo.push_back(pSpy);
    }
    void StopSpyTo(ModelEngine *pSpy);

protected:
    virtual void DoNotify(const Message &msg) = 0;
    virtual int InterestedIn() const { return 127; /* all messages */ }
    // With masks we can temporary stop handling of some messages
    void MaskAddOrDelete(int mask, bool bAdd)
    {
	if(bAdd)
	{
	    m_MaskMessage |= mask;
	}
	else
	{
	    m_MaskMessage &= ~mask;
	}
    }

    int Mask() const
    {
	return m_MaskMessage;
    }

private:
    Vector<ModelEngine*> m_apSpy;
    Vector<ModelEngine*> m_apSpyTo;
    int m_MaskMessage;
};

PNLW_END

#endif //__MODELENGINE_HPP__
