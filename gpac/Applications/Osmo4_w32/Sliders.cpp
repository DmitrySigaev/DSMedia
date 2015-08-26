// Sliders.cpp : implementation file
//

#include "stdafx.h"
#include "osmo4.h"
#include "Sliders.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Sliders dialog


Sliders::Sliders(CWnd* pParent /*=NULL*/)
	: CDialog(Sliders::IDD, pParent)
{
	//{{AFX_DATA_INIT(Sliders)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_grabbed = 0;
}


void Sliders::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Sliders)
	DDX_Control(pDX, IDC_SLIDER, m_PosSlider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Sliders, CDialog)
	//{{AFX_MSG_MAP(Sliders)
	ON_WM_HSCROLL()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Sliders message handlers


void Sliders::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	
	WinGPAC *app = GetApp();
	if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER) {
		switch (nSBCode) {
		case TB_LINEUP:
		case TB_LINEDOWN:
		case TB_PAGEUP:
		case TB_PAGEDOWN:
		case TB_THUMBPOSITION:
		case TB_THUMBTRACK:
		case TB_TOP:
		case TB_BOTTOM:
			m_grabbed = 1;
			break;
		case TB_ENDTRACK:
			if (!app->can_seek || !app->m_isopen) {
				m_PosSlider.SetPos(0);
			} else {
				u32 seek_to = m_PosSlider.GetPos();
				app->PlayFromTime(seek_to);
			}
			m_grabbed = 0;
			break;
		}
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void Sliders::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	if (!m_PosSlider.m_hWnd) return;
	RECT rc;
	m_PosSlider.GetClientRect(&rc);
	rc.right = rc.left + cx;
	m_PosSlider.SetWindowPos(this, rc.left, rc.top, rc.right, rc.bottom, SWP_NOZORDER | SWP_NOMOVE);
}

/*we sure don't want to close this window*/
void Sliders::OnClose() 
{
}
