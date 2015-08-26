// OptAudio.cpp : implementation file
//

#include "stdafx.h"
#include "Osmo4.h"
#include "OptAudio.h"

#include <gpac/m4_avhw.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptAudio dialog


COptAudio::COptAudio(CWnd* pParent /*=NULL*/)
	: CDialog(COptAudio::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptAudio)
	//}}AFX_DATA_INIT
}


void COptAudio::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptAudio)
	DDX_Control(pDX, IDC_AUDIO_NOTIFS, m_Notifs);
	DDX_Control(pDX, IDC_DRIVER_LIST, m_DriverList);
	DDX_Control(pDX, IDC_AUDIO_RESYNC, m_AudioResync);
	DDX_Control(pDX, IDC_AUDIO_FPS, m_AudioFPS);
	DDX_Control(pDX, IDC_SPIN_FPS, m_SpinFPS);
	DDX_Control(pDX, IDC_FORCE_AUDIO, m_ForceConfig);
	DDX_Control(pDX, IDC_SPIN_AUDIO, m_AudioSpin);
	DDX_Control(pDX, IDC_EDIT_AUDIO, m_AudioEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptAudio, CDialog)
	//{{AFX_MSG_MAP(COptAudio)
	ON_BN_CLICKED(IDC_FORCE_AUDIO, OnForceAudio)
	ON_CBN_SELCHANGE(IDC_DRIVER_LIST, OnSelchangeDriverList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptAudio message handlers

BOOL COptAudio::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_AudioSpin.SetBuddy(& m_AudioEdit);
	m_SpinFPS.SetBuddy(& m_AudioFPS);

	WinGPAC *gpac = GetApp();
	char *sOpt;

	sOpt = IF_GetKey(gpac->m_config, "Audio", "ForceConfig");
	if (sOpt && !stricmp(sOpt, "yes")) {
		m_ForceConfig.SetCheck(1);
	} else {
		m_ForceConfig.SetCheck(0);
	}
	sOpt = IF_GetKey(gpac->m_config, "Audio", "NumBuffers");
	if (sOpt) {
		m_AudioEdit.SetWindowText(sOpt);
	} else {
		m_AudioEdit.SetWindowText("6");
	}
	sOpt = IF_GetKey(gpac->m_config, "Audio", "BuffersPerSecond");
	if (sOpt) {
		m_AudioFPS.SetWindowText(sOpt);
	} else {
		m_AudioFPS.SetWindowText("15");
	}

	OnForceAudio();

	sOpt = IF_GetKey(gpac->m_config, "Audio", "NoResync");
	if (sOpt && !stricmp(sOpt, "yes")) {
		m_AudioResync.SetCheck(1);
	} else {
		m_AudioResync.SetCheck(0);
	}
	
	/*driver enum*/
	while (m_DriverList.GetCount()) m_DriverList.DeleteString(0);
	sOpt = IF_GetKey(gpac->m_config, "Audio", "DriverName");
	u32 count = PM_GetPluginsCount(gpac->m_plugins);
	BaseInterface *ifce;
	s32 select = 0;
	s32 to_sel = 0;
	for (u32 i=0; i<count; i++) {
		if (!PM_LoadInterface(gpac->m_plugins, i, M4_AUDIO_OUTPUT_INTERFACE, (void **) &ifce)) continue;
		if (sOpt && !stricmp(ifce->plugin_name, sOpt)) select = to_sel;
		m_DriverList.AddString(ifce->plugin_name);
		PM_ShutdownInterface(ifce);
		to_sel++;
	}
	m_DriverList.SetCurSel(select);

	m_Notifs.ShowWindow(SW_HIDE);
	if (sOpt && strstr(sOpt, "DirectSound")) m_Notifs.ShowWindow(SW_SHOW);

	sOpt = IF_GetKey(gpac->m_config, "Audio", "DisableNotification");
	if (sOpt && !stricmp(sOpt, "yes")) 
		m_Notifs.SetCheck(1);
	else
		m_Notifs.SetCheck(0);

	return TRUE;  
}


void COptAudio::SaveOptions()
{
	WinGPAC *gpac = GetApp();
	char str[50];

	IF_SetKey(gpac->m_config, "Audio", "ForceConfig", m_ForceConfig.GetCheck() ? "yes" : "no");
	IF_SetKey(gpac->m_config, "Audio", "NoResync", m_AudioResync.GetCheck() ? "yes" : "no");

	m_AudioEdit.GetWindowText(str, 20);
	IF_SetKey(gpac->m_config, "Audio", "NumBuffers", str);
	m_AudioFPS.GetWindowText(str, 20);
	IF_SetKey(gpac->m_config, "Audio", "BuffersPerSecond", str);

	m_DriverList.GetWindowText(str, 50);
	IF_SetKey(gpac->m_config, "Audio", "DriverName", str);

	if (strstr(str, "DirectSound")) {
		IF_SetKey(gpac->m_config, "Audio", "DisableNotification", m_Notifs.GetCheck() ? "yes" : "no");
	}

}

void COptAudio::OnForceAudio() 
{
	BOOL en = m_ForceConfig.GetCheck();

	m_AudioSpin.EnableWindow(en);
	m_AudioEdit.EnableWindow(en);
	m_SpinFPS.EnableWindow(en);
	m_AudioFPS.EnableWindow(en);
}

void COptAudio::OnSelchangeDriverList() 
{
	char str[50];
	m_DriverList.GetWindowText(str, 50);
	if (strstr(str, "DirectSound")) {
		m_Notifs.ShowWindow(SW_SHOW);
	} else {
		m_Notifs.ShowWindow(SW_HIDE);
	}
}
