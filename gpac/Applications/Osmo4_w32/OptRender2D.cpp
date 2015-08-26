// OptRender2D.cpp : implementation file
//

#include "stdafx.h"
#include "osmo4.h"
#include "OptRender2D.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptRender2D dialog


COptRender2D::COptRender2D(CWnd* pParent /*=NULL*/)
	: CDialog(COptRender2D::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptRender2D)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void COptRender2D::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptRender2D)
	DDX_Control(pDX, IDC_FORMAT_YUV, m_YUVFormat);
	DDX_Control(pDX, IDC_YUV, m_NoYUV);
	DDX_Control(pDX, IDC_ZOOM_SCALABLE, m_Scalable);
	DDX_Control(pDX, IDC_DIRECTRENDER, m_DirectRender);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptRender2D, CDialog)
	//{{AFX_MSG_MAP(COptRender2D)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptRender2D message handlers

BOOL COptRender2D::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	WinGPAC *gpac = GetApp();
	char *sOpt;

	sOpt = IF_GetKey(gpac->m_config, "Render2D", "DirectRender");
	if (sOpt && !stricmp(sOpt, "yes")) {
		m_DirectRender.SetCheck(1);
	} else {
		m_DirectRender.SetCheck(0);
	}
	sOpt = IF_GetKey(gpac->m_config, "Render2D", "ScalableZoom");
	if (sOpt && !stricmp(sOpt, "no")) {
		m_Scalable.SetCheck(0);
	} else {
		m_Scalable.SetCheck(1);
	}
	
	sOpt = IF_GetKey(gpac->m_config, "Render2D", "DisableYUV");
	if (sOpt && !stricmp(sOpt, "yes")) {
		m_NoYUV.SetCheck(1);
	} else {
		m_NoYUV.SetCheck(0);
	}
	
	SetYUV();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptRender2D::SetYUV()
{
	WinGPAC *gpac = GetApp();
	u32 yuv_format = M4T_GetOption(gpac->m_term, M4O_YUVFormat);
	if (!yuv_format) {
		m_YUVFormat.SetWindowText("(No YUV used)");
	} else {
		char fmt[5], str[100];
		MP4TypeToString(yuv_format, fmt);
		fmt[4] = 0;
		sprintf(str, "(%s used)", fmt);
		m_YUVFormat.SetWindowText(str);
	}
}

void COptRender2D::SaveOptions()
{
	WinGPAC *gpac = GetApp();
	IF_SetKey(gpac->m_config, "Render2D", "DirectRender", m_DirectRender.GetCheck() ? "yes" : "no");
	IF_SetKey(gpac->m_config, "Render2D", "ScalableZoom", m_Scalable.GetCheck() ? "yes" : "no");

	IF_SetKey(gpac->m_config, "Render2D", "DisableYUV", m_NoYUV.GetCheck() ? "yes" : "no");
}

