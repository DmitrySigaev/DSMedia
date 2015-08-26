// OptDecoder.cpp : implementation file
//

#include "stdafx.h"
#include "osmo4.h"
#include "OptDecoder.h"
#include <gpac/m4_decoder.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// OptDecoder dialog


OptDecoder::OptDecoder(CWnd* pParent /*=NULL*/)
	: CDialog(OptDecoder::IDD, pParent)
{
	//{{AFX_DATA_INIT(OptDecoder)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void OptDecoder::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(OptDecoder)
	DDX_Control(pDX, IDC_VIDEC_LIST, m_Video);
	DDX_Control(pDX, IDC_AUDEC_LIST, m_Audio);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(OptDecoder, CDialog)
	//{{AFX_MSG_MAP(OptDecoder)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// OptDecoder message handlers



BOOL OptDecoder::OnInitDialog() 
{
	const char *sOpt;
	CDialog::OnInitDialog();
	
	WinGPAC *gpac = GetApp();
	
	/*audio dec enum*/
	while (m_Audio.GetCount()) m_Audio.DeleteString(0);
	sOpt = IF_GetKey(gpac->m_config, "Systems", "DefAudioDec");
	u32 count = PM_GetPluginsCount(gpac->m_plugins);
	BaseDecoder *ifce;
	s32 select = 0;
	s32 to_sel = 0;
	for (u32 i=0; i<count; i++) {
		if (!PM_LoadInterface(gpac->m_plugins, i, M4MEDIADECODERINTERFACE, (void **) &ifce)) continue;

		if (ifce->CanHandleStream(ifce, M4ST_AUDIO, 0, NULL, 0, 0)) {
			if (sOpt && !stricmp(ifce->plugin_name, sOpt)) select = to_sel;
			m_Audio.AddString(ifce->plugin_name);
			to_sel++;
		}
		PM_ShutdownInterface(ifce);
	}
	m_Audio.SetCurSel(select);

	/*video dec enum*/
	while (m_Video.GetCount()) m_Video.DeleteString(0);
	sOpt = IF_GetKey(gpac->m_config, "Systems", "DefVideoDec");
	count = PM_GetPluginsCount(gpac->m_plugins);
	select = 0;
	to_sel = 0;
	for (i=0; i<count; i++) {
		if (!PM_LoadInterface(gpac->m_plugins, i, M4MEDIADECODERINTERFACE, (void **) &ifce)) continue;

		if (ifce->CanHandleStream(ifce, M4ST_VISUAL, 0, NULL, 0, 0)) {
			if (sOpt && !stricmp(ifce->plugin_name, sOpt)) select = to_sel;
			m_Video.AddString(ifce->plugin_name);
			to_sel++;
		}
		PM_ShutdownInterface(ifce);
	}
	m_Video.SetCurSel(select);
	return TRUE;  
}

void OptDecoder::SaveOptions()
{
	WinGPAC *gpac = GetApp();
	char str[100];
	m_Audio.GetWindowText(str, 100);
	IF_SetKey(gpac->m_config, "Systems", "DefAudioDec", str);
	m_Video.GetWindowText(str, 100);
	IF_SetKey(gpac->m_config, "Systems", "DefVideoDec", str);

}
