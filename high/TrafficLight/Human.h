// Human.h: interface for the CHuman class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HUMAN_H__B0FFA262_C7E6_4ED5_9F86_FE4A62E66AEA__INCLUDED_)
#define AFX_HUMAN_H__B0FFA262_C7E6_4ED5_9F86_FE4A62E66AEA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MovingObject.h"

class CHuman : public CMovingObject  
{
public:
	static int GetDefaultSpeed();
	CHuman();
	virtual ~CHuman();

        virtual void Show();

	virtual MovingObjectTypes GetType() {return Human;};

protected:
    virtual bool CanIGoForward(CField *pField);
};

#endif // !defined(AFX_HUMAN_H__B0FFA262_C7E6_4ED5_9F86_FE4A62E66AEA__INCLUDED_)
