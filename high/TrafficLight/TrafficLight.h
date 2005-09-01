// TrafficLight.h : main header file for the TRAFFICLIGHT application
//

#if !defined(AFX_TRAFFICLIGHT_H__9EEB8EF7_64D8_400A_9080_1788BEBB0B82__INCLUDED_)
#define AFX_TRAFFICLIGHT_H__9EEB8EF7_64D8_400A_9080_1788BEBB0B82__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CTrafficLightApp:
// See TrafficLight.cpp for the implementation of this class
//

class CTrafficLightApp : public CWinApp
{
public:
	CTrafficLightApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTrafficLightApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CTrafficLightApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRAFFICLIGHT_H__9EEB8EF7_64D8_400A_9080_1788BEBB0B82__INCLUDED_)
