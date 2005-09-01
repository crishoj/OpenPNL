// Car.h: interface for the CCar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAR_H__A38D5A5C_89F3_4772_BAAF_3AE1A0BFE725__INCLUDED_)
#define AFX_CAR_H__A38D5A5C_89F3_4772_BAAF_3AE1A0BFE725__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MovingObject.h"

class CCar : public CMovingObject  
{
public:
	static int GetDefaultSpeed();
	CCar();
	virtual ~CCar();

        virtual void Show();

	virtual MovingObjectTypes GetType() {return Car;};

protected:
    virtual bool CanIGoForward(CField *pField);
};

#endif // !defined(AFX_CAR_H__A38D5A5C_89F3_4772_BAAF_3AE1A0BFE725__INCLUDED_)
