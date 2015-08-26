// Options.cpp : implementation file
//

#include "stdafx.h"
#include "Osmo4.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptions dialog


COptions::COptions(CWnd* pParent /*=NULL*/)
	: CDialog(COptions::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptions)
	//}}AFX_DATA_INIT
}


void COptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptions)
	DDX_Control(pDX, IDC_SELECT, m_Selector);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptions, CDialog)
	//{{AFX_MSG_MAP(COptions)
	ON_BN_CLICKED(IDC_SAVEOPT, OnSaveopt)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_SELECT, OnSelchangeSelect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL COptions::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_general.Create(IDD_OPT_GEN, this);
	m_systems.Create(IDD_OPT_SYSTEMS, this);
	m_render.Create(IDD_OPT_RENDER, this);
	m_render2d.Create(IDD_OPT_RENDER2D, this);
	m_render3d.Create(IDD_OPT_RENDER3D, this);
	m_decoder.Create(IDD_OPT_DECODER, this);
	m_audio.Create(IDD_OPT_AUDIO, this);
	m_video.Create(IDD_OPT_VIDEO, this);
	m_http.Create(IDD_OPT_HTTP, this);
	m_font.Create(IDD_OPT_FONT, this);
	m_stream.Create(IDD_OPT_STREAM, this);

	m_Selector.AddString("General");
	m_Selector.AddString("MPEG-4 Systems");
	m_Selector.AddString("Media Decoders");
	m_Selector.AddString("Rendering");
	m_Selector.AddString("Renderer 2D");
	m_Selector.AddString("Renderer 3D");
	m_Selector.AddString("Video Output");
	m_Selector.AddString("Audio Output");
	m_Selector.AddString("Text Engine");
	m_Selector.AddString("File Download");
	m_Selector.AddString("Real-Time Streaming");

	HideAll();
	m_Selector.SetCurSel(0);
	m_general.ShowWindow(SW_SHOW);

	return TRUE;  
}


/////////////////////////////////////////////////////////////////////////////
// COptions message handlers

void COptions::HideAll()
{
	m_general.ShowWindow(SW_HIDE);
	m_systems.ShowWindow(SW_HIDE);
	m_render.ShowWindow(SW_HIDE);
	m_render2d.ShowWindow(SW_HIDE);
	m_render3d.ShowWindow(SW_HIDE);
	m_audio.ShowWindow(SW_HIDE);
	m_video.ShowWindow(SW_HIDE);
	m_http.ShowWindow(SW_HIDE);
	m_font.ShowWindow(SW_HIDE);
	m_stream.ShowWindow(SW_HIDE);
	m_decoder.ShowWindow(SW_HIDE);
}

void COptions::OnSelchangeSelect() 
{
	HideAll();
	switch (m_Selector.GetCurSel()) {
	case 0: m_general.ShowWindow(SW_SHOW); break;
	case 1: m_systems.ShowWindow(SW_SHOW); break;
	case 2: m_decoder.ShowWindow(SW_SHOW); break;
	case 3: m_render.ShowWindow(SW_SHOW); break;
	case 4: m_render2d.ShowWindow(SW_SHOW); break;
	case 5: m_render3d.ShowWindow(SW_SHOW); break;
	case 6: m_video.ShowWindow(SW_SHOW); break;
	case 7: m_audio.ShowWindow(SW_SHOW); break;
	case 8: m_font.ShowWindow(SW_SHOW); break;
	case 9: m_http.ShowWindow(SW_SHOW); break;
	case 10: m_stream.ShowWindow(SW_SHOW); break;
	}
}

void COptions::OnSaveopt() 
{
	Bool need_reload;
	m_general.SaveOptions();
	m_systems.SaveOptions();
	m_decoder.SaveOptions();
	need_reload = m_render.SaveOptions();
	m_render2d.SaveOptions();
	m_render3d.SaveOptions();
	m_audio.SaveOptions();
	m_video.SaveOptions();
	m_http.SaveOptions();
	m_font.SaveOptions();
	m_stream.SaveOptions();

	WinGPAC *gpac = GetApp();
	if (!need_reload) {
		M4T_SetOption(gpac->m_term, M4O_ReloadConfig, 1);
	} else {
		gpac->ReloadTerminal();
	}
	m_render2d.SetYUV();
}

void COptions::OnClose() 
{
	DestroyWindow();
}

void COptions::OnDestroy() 
{
	CDialog::OnDestroy();
	delete this;
	((CMainFrame *)GetApp()->m_pMainWnd)->m_pOpt = NULL;
}
