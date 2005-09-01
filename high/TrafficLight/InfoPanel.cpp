// InfoPanel.cpp : implementation file
//

#include "stdafx.h"
#include "TrafficLight.h"
#include "InfoPanel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInfoPanel

IMPLEMENT_DYNCREATE(CInfoPanel, CFormView)

CInfoPanel::CInfoPanel()
	: CFormView(CInfoPanel::IDD)
{
	//{{AFX_DATA_INIT(CInfoPanel)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    pNature = NULL;
    m_Hours = 0;
    m_Minutes = 0;
    m_MaxSliderValue = 100;
}

CInfoPanel::~CInfoPanel()
{
    pNature->PauseProcessing();
    pNature->SetInfoPanel(NULL);
}

void CInfoPanel::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInfoPanel)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInfoPanel, CFormView)
	//{{AFX_MSG_MAP(CInfoPanel)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BUTTON_START, OnButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInfoPanel diagnostics

#ifdef _DEBUG
void CInfoPanel::AssertValid() const
{
	CFormView::AssertValid();
}

void CInfoPanel::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CInfoPanel message handlers

void CInfoPanel::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CFormView::OnShowWindow(bShow, nStatus);
	
    CSliderCtrl *pSlider = static_cast<CSliderCtrl *>(GetDlgItem(IDC_SLIDER_TIME));
    pSlider->SetRangeMin(0);
    pSlider->SetRangeMax(m_MaxSliderValue);
    pSlider->SetTicFreq(1);

    DisableControls();

    CButton *pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_SMART));
    pButton->SetCheck(1);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_SUNNY));
    pButton->SetCheck(1);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_WORKDAY));
    pButton->SetCheck(1);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_CHECK_TIME));
    pButton->SetCheck(1);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_CHECK_DAY));
    pButton->SetCheck(1);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_CHECK_WEATHER));
    pButton->SetCheck(1);
}

void CInfoPanel::OnButtonStart() 
{
	// TODO: Add your control notification handler code here
    
    pNature->StartProcessing();

    DisableControls();
}

void CInfoPanel::OnButtonStop() 
{
	// TODO: Add your control notification handler code here
    
    pNature->PauseProcessing();

    EnableControls();    
}

Days CInfoPanel::GetDay()
{
    CButton *pButton;

    if ((pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_WORKDAY)))->GetCheck() == 0)
    {
	return Saturday;
    };

    return Monday;
};

Weather CInfoPanel::GetWeather()
{
    CButton *pButton;

    if ((pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_SUNNY)))->GetCheck() == 1)
    {
	return Sunny;
    };

    if ((pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_FOGGY)))->GetCheck() == 1)
    {
	return Foggy;
    };

    if ((pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_CLOUDY)))->GetCheck() == 1)
    {
	return Cloudy;
    };

    Weather GeneratedWeather = static_cast<Weather>(rand() % 3);

    switch (GeneratedWeather)
    {
    case Sunny:
        pButton = static_cast<CButton*>(GetDlgItem(IDC_RADIO_SUNNY));
        break;
    case Foggy:
        pButton = static_cast<CButton*>(GetDlgItem(IDC_RADIO_FOGGY));
        break;
    case Cloudy:
        pButton = static_cast<CButton*>(GetDlgItem(IDC_RADIO_CLOUDY));
        break;
    };

    pButton->SetCheck(1);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_AUTO_WEATHER));
    pButton->SetCheck(0);

    Invalidate(FALSE);

    return GeneratedWeather;
};

void CInfoPanel::SetDay(Days day)
{
    CButton *pButton;
    int DlgIndex0;
    int DlgIndex1;

    switch (day)
    {
    case Monday:
    case Tuesday:
    case Wednesday:
    case Thursday:
    case Friday:
	DlgIndex0 = IDC_RADIO_HOLIDAY;
	DlgIndex1 = IDC_RADIO_WORKDAY;
	break;

    case Saturday:
    case Sunday:
	DlgIndex0 = IDC_RADIO_WORKDAY;
	DlgIndex1 = IDC_RADIO_HOLIDAY;
	break;
    };

    pButton = static_cast<CButton *>(GetDlgItem(DlgIndex1));
    pButton->SetCheck(1);

    pButton = static_cast<CButton *>(GetDlgItem(DlgIndex0));
    pButton->SetCheck(0);
}

void CInfoPanel::GetTime(int &HoursOut, int &MinutesOut)
{
    HoursOut = m_Hours;
    MinutesOut = m_Minutes;
}

void CInfoPanel::OnPaint() 
{
    CPaintDC dc(this); // device context for painting
    
    // TODO: Add your message handler code here
    
    // Do not call CFormView::OnPaint() for painting messages
    CString str;
    int delta = -1;
    str.Format("%d:%d %s", ((delta = m_Hours % 12)==0)?(12):(delta), m_Minutes, (m_Hours >= 12)?("PM"):("AM"));

    CButton *pButton = static_cast<CButton *>(GetDlgItem(IDC_STATIC_TIME));
    pButton->SetWindowText(str.GetBuffer(10));

    int CyclesPerDay = pNature->GetCyclesPerDay();
    int Tics;
    pNature->GetTicsByTime(m_Hours, m_Minutes, Tics);

    CSliderCtrl *pSlider = static_cast<CSliderCtrl *>(GetDlgItem(IDC_SLIDER_TIME));
    pSlider->SetPos(static_cast<float>(Tics)/CyclesPerDay*m_MaxSliderValue);
}

void CInfoPanel::SetTime(int Hours, int Minutes)
{
    m_Hours = Hours;
    m_Minutes = Minutes;
}

void CInfoPanel::EnableControls()
{
    CButton *pButton = static_cast<CButton *>(GetDlgItem(IDC_BUTTON_START));
    pButton->EnableWindow();
	
    pButton = static_cast<CButton *>(GetDlgItem(IDC_BUTTON_STOP));
    pButton->EnableWindow(FALSE);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_WORKDAY));
    pButton->EnableWindow();

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_HOLIDAY));
    pButton->EnableWindow();

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_SUNNY));
    pButton->EnableWindow();

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_FOGGY));
    pButton->EnableWindow();

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_CLOUDY));
    pButton->EnableWindow();

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_AUTO_WEATHER));
    pButton->EnableWindow();

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_SMART));
    pButton->EnableWindow();

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_ORDINARY));
    pButton->EnableWindow(); 
    
    CSliderCtrl *pSlider = static_cast<CSliderCtrl *>(GetDlgItem(IDC_SLIDER_TIME));
    pSlider->EnableWindow();

    pButton = static_cast<CButton *>(GetDlgItem(IDC_CHECK_TIME));
    pButton->EnableWindow();

    pButton = static_cast<CButton *>(GetDlgItem(IDC_CHECK_DAY));
    pButton->EnableWindow();

    pButton = static_cast<CButton *>(GetDlgItem(IDC_CHECK_WEATHER));
    pButton->EnableWindow();
}

void CInfoPanel::DisableControls()
{
    CButton *pButton = static_cast<CButton *>(GetDlgItem(IDC_BUTTON_START));
    pButton->EnableWindow(FALSE);
	
    pButton = static_cast<CButton *>(GetDlgItem(IDC_BUTTON_STOP));
    pButton->EnableWindow();

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_WORKDAY));
    pButton->EnableWindow(FALSE);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_HOLIDAY));
    pButton->EnableWindow(FALSE);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_SUNNY));
    pButton->EnableWindow(FALSE);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_FOGGY));
    pButton->EnableWindow(FALSE);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_CLOUDY));
    pButton->EnableWindow(FALSE);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_AUTO_WEATHER));
    pButton->EnableWindow(FALSE);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_SMART));
    pButton->EnableWindow(FALSE);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_ORDINARY));
    pButton->EnableWindow(FALSE); 
    
    CSliderCtrl *pSlider = static_cast<CSliderCtrl *>(GetDlgItem(IDC_SLIDER_TIME));
    pSlider->EnableWindow(FALSE);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_CHECK_TIME));
    pButton->EnableWindow(FALSE);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_CHECK_DAY));
    pButton->EnableWindow(FALSE);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_CHECK_WEATHER));
    pButton->EnableWindow(FALSE);
}

void CInfoPanel::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    // TODO: Add your message handler code here and/or call default   
    CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
    
    if ((nSBCode == SB_THUMBPOSITION)||(nSBCode == SB_THUMBTRACK))
    {
	int Hours;
	int Minutes;
	int Tics;

	Tics = static_cast<float>(nPos)/m_MaxSliderValue*(pNature->GetCyclesPerDay());

	pNature->GetTimeByTics(Tics, Hours, Minutes);

	m_Hours = Hours;
	m_Minutes = Minutes;
	Invalidate(FALSE);
    };
}

TrafficLightType CInfoPanel::GetTrafficLightType()
{
    CButton *pButton;

    if ((pButton = static_cast<CButton *>(GetDlgItem(IDC_RADIO_SMART)))->GetCheck() == 1)
    {
	return Smart;
    };

    return Ordinary;
}

void CInfoPanel::SetTrafficLightDistribution(float Green, float Red, float Yellow)
{
    CString str;
    CStatic *pStatic = static_cast<CStatic *>(GetDlgItem(IDC_STATIC_GREEN));
    str.Format("%.3f", Green);
    pStatic->SetWindowText(str);

    str = "";

    pStatic = static_cast<CStatic *>(GetDlgItem(IDC_STATIC_RED));
    str.Format("%.3f", Red);
    pStatic->SetWindowText(str);

    str = "";

    pStatic = static_cast<CStatic *>(GetDlgItem(IDC_STATIC_YELLOW));
    str.Format("%.3f", Yellow);
    pStatic->SetWindowText(str);
}

void CInfoPanel::SetNumbersOfCarsAndHumans(int NCars, int NHumans)
{
    CString str;
    CStatic *pStatic = static_cast<CStatic *>(GetDlgItem(IDC_STATIC_CARS));
    str.Format("%i", NCars);
    pStatic->SetWindowText(str);

    pStatic = static_cast<CStatic *>(GetDlgItem(IDC_STATIC_humans));
    str.Format("%i", NHumans);
    pStatic->SetWindowText(str);
}

void CInfoPanel::SetSpeedOfCarsAndHumans(int CarSpeed, int HumanSpeed)
{
    CString str;
    CStatic *pStatic = static_cast<CStatic *>(GetDlgItem(IDC_STATIC_SCARS));
    str.Format("%i", CarSpeed);
    pStatic->SetWindowText(str);

    pStatic = static_cast<CStatic *>(GetDlgItem(IDC_STATIC_SHUMANS));
    str.Format("%i", HumanSpeed);
    pStatic->SetWindowText(str);
}

void CInfoPanel::SetSpeedOfCarAndHumanAppearance(double CarSpeedAppearance, double HumanSpeedAppearance)
{
    CString str;
    CStatic *pStatic = static_cast<CStatic *>(GetDlgItem(IDC_STATIC_CARST));
    str.Format("%.3f", CarSpeedAppearance);
    pStatic->SetWindowText(str);

    pStatic = static_cast<CStatic *>(GetDlgItem(IDC_STATIC_HUMANST));
    str.Format("%.3f", HumanSpeedAppearance);
    pStatic->SetWindowText(str);
}

void CInfoPanel::GetObservation(bool &DayObserved, bool &TimeObserved, bool &WeatherObserved)
{
    CButton *pButton = static_cast<CButton *>(GetDlgItem(IDC_CHECK_TIME));
    TimeObserved = (pButton->GetCheck()==1);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_CHECK_DAY));
    DayObserved = (pButton->GetCheck()==1);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_CHECK_TIME));
    TimeObserved = (pButton->GetCheck()==1);

    pButton = static_cast<CButton *>(GetDlgItem(IDC_CHECK_WEATHER));
    WeatherObserved = (pButton->GetCheck()==1);
}
