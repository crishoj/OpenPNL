#if !defined(AFX_INFOPANEL_H__AAFDBA21_4DE0_43F3_98D2_9D8684BF2BDD__INCLUDED_)
#define AFX_INFOPANEL_H__AAFDBA21_4DE0_43F3_98D2_9D8684BF2BDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InfoPanel.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInfoPanel form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "Nature.h"

class CInfoPanel : public CFormView
{
protected:
	CInfoPanel();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CInfoPanel)

// Form Data
public:
	//{{AFX_DATA(CInfoPanel)
	enum { IDD = IDD_INFOPANEL };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	void GetObservation(bool &DayObserved, bool &TimeObserved, bool &WeatherObserved);
	void SetSpeedOfCarAndHumanAppearance(double CarSpeedAppearance, double HumanSpeedAppearance);
	void SetSpeedOfCarsAndHumans(int CarSpeed, int HumanSpeed);
	void SetNumbersOfCarsAndHumans(int NCars, int NHumans);
	void SetTrafficLightDistribution(float Green, float Red, float Yellow);
	TrafficLightType GetTrafficLightType();
	void DisableControls();
	void EnableControls();
	void SetTime(int Hours, int Minutes);
	void GetTime(int &HoursOut, int &MinutesOut);
	void SetDay(Days day);
    void SetNature(CNature *_pNature)
    {pNature = _pNature;};

    Days GetDay();
    Weather GetWeather();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInfoPanel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_MaxSliderValue;
	int m_Minutes;
	int m_Hours;
	virtual ~CInfoPanel();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

    CNature *pNature;

	// Generated message map functions
	//{{AFX_MSG(CInfoPanel)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnButtonStart();
	afx_msg void OnButtonStop();
	afx_msg void OnPaint();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INFOPANEL_H__AAFDBA21_4DE0_43F3_98D2_9D8684BF2BDD__INCLUDED_)
