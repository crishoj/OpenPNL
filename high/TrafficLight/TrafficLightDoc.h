// TrafficLightDoc.h : interface of the CTrafficLightDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRAFFICLIGHTDOC_H__2C5818C1_367F_4008_8759_DBECB7788494__INCLUDED_)
#define AFX_TRAFFICLIGHTDOC_H__2C5818C1_367F_4008_8759_DBECB7788494__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CTrafficLightDoc : public CDocument
{
protected: // create from serialization only
	CTrafficLightDoc();
	DECLARE_DYNCREATE(CTrafficLightDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTrafficLightDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTrafficLightDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CNature *GetNature() {return m_pNature;};
	CField  *GetField()  {return m_pField;};

protected:

    CNature *m_pNature;
    CField  *m_pField;

// Generated message map functions
protected:
	//{{AFX_MSG(CTrafficLightDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRAFFICLIGHTDOC_H__2C5818C1_367F_4008_8759_DBECB7788494__INCLUDED_)
