// OptVideo.cpp : implementation file
//

#include "stdafx.h"
#include "Osmo4.h"
#include "OptVideo.h"

#include <gpac/m4_avhw.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptVideo dialog


COptVideo::COptVideo(CWnd* pParent /*=NULL*/)
	: CDialog(COptVideo::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptVideo)
	//}}AFX_DATA_INIT
}


void COptVideo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptVideo)
	DDX_Control(pDX, IDC_SWITCH_RES, m_SwitchRes);
	DDX_Control(pDX, IDC_VIDEO_LIST, m_Videos);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptVideo, CDialog)
	//{{AFX_MSG_MAP(COptVideo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptVideo message handlers

BOOL COptVideo::OnInitDialog() 
{
	CDialog::OnInitDialog();

	WinGPAC *gpac = GetApp();
	char *sOpt;
	
	sOpt = IF_GetKey(gpac->m_config, "Video", "SwitchResolution");
	if (sOpt && !stricmp(sOpt, "yes")) {
		m_SwitchRes.SetCheck(1);
	} else {
		m_SwitchRes.SetCheck(0);
	}

	
	u32 count = PM_GetPluginsCount(gpac->m_plugins);
	void *ifce;
	s32 to_sel = 0;
	s32 select = 0;
	/*video drivers enum*/
	while (m_Videos.GetCount()) m_Videos.DeleteString(0);
	sOpt = IF_GetKey(gpac->m_config, "Video", "DriverName");

	for (u32 i=0; i<count; i++) {
		if (!PM_LoadInterface(gpac->m_plugins, i, M4_VIDEO_OUTPUT_INTERFACE, &ifce)) continue;
		if (sOpt && !stricmp(((BaseInterface *)ifce)->plugin_name, sOpt)) select = to_sel;
		m_Videos.AddString(((BaseInterface *)ifce)->plugin_name);
		PM_ShutdownInterface(ifce);
		to_sel++;
	}
	m_Videos.SetCurSel(select);

	return TRUE;
	            
}

void COptVideo::SaveOptions()
{
	WinGPAC *gpac = GetApp();
	char str[50];

	IF_SetKey(gpac->m_config, "Video", "SwitchResolution", m_SwitchRes.GetCheck() ? "yes" : "no");
	m_Videos.GetWindowText(str, 50);
	IF_SetKey(gpac->m_config, "Video", "DriverName", str);
}

