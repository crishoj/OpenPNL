// TrafficLightView.h : interface of the CTrafficLightView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRAFFICLIGHTVIEW_H__98F60BFC_63A6_4A41_AA33_679A0BD6BA65__INCLUDED_)
#define AFX_TRAFFICLIGHTVIEW_H__98F60BFC_63A6_4A41_AA33_679A0BD6BA65__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CTrafficLightView : public CView
{
protected: // create from serialization only
	CTrafficLightView();
	DECLARE_DYNCREATE(CTrafficLightView)

// Attributes
public:
	CTrafficLightDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTrafficLightView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetProjection();
	BOOL SetupPixelFormat();
	void InitGL();
	virtual ~CTrafficLightView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

    HGLRC    m_hRC;
    HDC m_myhDC;
    int    m_height;
    int    m_width; 

protected:

    int m_alpha, m_betta;
    int m_zvalue;
    int m_x0, m_y0;

// Generated message map functions
protected:
	//{{AFX_MSG(CTrafficLightView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnCancelMode();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in TrafficLightView.cpp
inline CTrafficLightDoc* CTrafficLightView::GetDocument()
   { return (CTrafficLightDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRAFFICLIGHTVIEW_H__98F60BFC_63A6_4A41_AA33_679A0BD6BA65__INCLUDED_)
