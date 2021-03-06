// GPAC.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Osmo4.h"

#include "MainFrm.h"
#include "OpenUrl.h"
#include <gpac/m4_network.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// WinGPAC

BEGIN_MESSAGE_MAP(WinGPAC, CWinApp)
	//{{AFX_MSG_MAP(WinGPAC)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILEOPEN, OnOpenFile)
	ON_COMMAND(ID_FILE_STEP, OnFileStep)
	ON_COMMAND(ID_OPEN_URL, OnOpenUrl)
	ON_COMMAND(ID_FILE_RELOAD, OnFileReload)
	ON_COMMAND(ID_FILE_PLAY, OnFilePlay)
	ON_UPDATE_COMMAND_UI(ID_FILE_PLAY, OnUpdateFilePlay)
	ON_UPDATE_COMMAND_UI(ID_FILE_STEP, OnUpdateFileStep)
	ON_COMMAND(ID_FILE_STOP, OnFileStop)
	ON_UPDATE_COMMAND_UI(ID_FILE_STOP, OnUpdateFileStop)
	ON_COMMAND(ID_SWITCH_RENDER, OnSwitchRender)
	ON_COMMAND(ID_RELOAD_TERMINAL, OnReloadTerminal)
	ON_UPDATE_COMMAND_UI(ID_FILE_RELOAD, OnUpdateFileStop)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// WinGPAC construction

WinGPAC::WinGPAC()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only WinGPAC object

WinGPAC theApp;


Bool is_supported_file(LPINIFILE cfg, const char *fileName, Bool disable_no_ext)
{
	char szExt[20], *ext, mimes[1000];
	u32 keyCount, i;

	ext = strrchr(fileName, '/');
	if (ext) {
		ext = strchr(fileName, '.');
		/*fixme - a proper browser should check mime & co here*/
		if (!ext) return strstr(fileName, "http://") ? 0 : 1;
	}
	
	ext = strrchr(fileName, '.');
	/*this may be anything so let's try*/
	if (!ext) {
		return !disable_no_ext;
	}

	strcpy(szExt, ext+1);
	ext =strrchr(szExt, '#');
	if (ext) ext[0] = 0;
	strlwr(szExt);

	keyCount = IF_GetKeyCount(cfg, "MimeTypes");
	for (i=0; i<keyCount; i++) {
		char *sKey;
		sKey = (char *) IF_GetKeyName(cfg, "MimeTypes", i);
		if (!sKey) continue;
		sKey = IF_GetKey(cfg, "MimeTypes", sKey);
		strcpy(mimes, sKey+1);
		sKey = strchr(mimes, '"');
		if (!sKey) continue;
		sKey[0] = 0;
		sKey = mimes;
		while (sKey) {
			if (!strnicmp(sKey, szExt, strlen(szExt))) return 1;
			sKey = strchr(sKey, ' ');
			if (sKey) sKey+=1;
		}
	}
	if (!strstr(fileName, "http://")) return 1;
	/*looks like a regular web link_*/
	return 0;
}

Bool Osmo4_EventProc(void *priv, M4Event *evt)
{
	u32 dur;
	WinGPAC *gpac = (WinGPAC *) priv;
	CMainFrame *pFrame = (CMainFrame *) gpac->m_pMainWnd;
	/*shutdown*/
	if (!pFrame) return 0;

	switch (evt->type) {
	case M4E_DURATION:
		dur = (u32) (1000 * evt->duration.duration);
		if (dur<1100) dur = 0;
		pFrame->m_pPlayList->SetDuration((u32) evt->duration.duration );
		gpac->max_duration = dur;
		if (!dur) {
			gpac->can_seek = 0;
			pFrame->m_Sliders.m_PosSlider.EnableWindow(FALSE);
		} else {
			gpac->can_seek = 1;
			pFrame->m_Sliders.m_PosSlider.EnableWindow(TRUE);
			pFrame->m_Sliders.m_PosSlider.SetRangeMin(0);
			pFrame->m_Sliders.m_PosSlider.SetRangeMax(dur);
		}
		break;

	case M4E_MESSAGE:
		if (!evt->message.service || !strcmp(evt->message.service, (LPCSTR) pFrame->m_pPlayList->GetURL() )) {
			pFrame->console_service = "main service";
		} else {
			pFrame->console_service = evt->message.service;
		}
		if (evt->message.error!=M4OK) {
			if (evt->message.error<M4OK || !gpac->m_NoConsole) {
				pFrame->console_err = evt->message.error;
				pFrame->console_message = evt->message.message;
				gpac->m_pMainWnd->PostMessage(WM_CONSOLEMSG, 0, 0);

				/*any error before connection confirm is a service connection error*/
				if (!gpac->m_isopen) pFrame->m_pPlayList->SetDead();
			}
			return 0;
		}
		if (gpac->m_NoConsole) return 0;

		/*process user message*/
		pFrame->console_err = M4OK;
		pFrame->console_message = evt->message.message;
		gpac->m_pMainWnd->PostMessage(WM_CONSOLEMSG, 0, 0);
		break;

	case M4E_SCENESIZE:
		gpac->orig_width = evt->size.width;
		gpac->orig_height = evt->size.height;
		if (gpac->m_term) 
			pFrame->PostMessage(WM_SETSIZE, evt->size.width, evt->size.height);
		break;

	case M4E_CONNECT:
		pFrame->BuildStreamList(1);
		if (!evt->connect.is_connected) gpac->max_duration = 0;
		gpac->m_isopen = evt->connect.is_connected;
		pFrame->m_wndToolBar.SetButtonInfo(5, ID_FILE_PLAY, TBBS_BUTTON, gpac->m_isopen ? 4 : 3);
		pFrame->m_Sliders.m_PosSlider.SetPos(0);
		pFrame->SetProgTimer(1);
		break;

	case M4E_QUIT:
		pFrame->PostMessage(WM_CLOSE, 0L, 0L);
		break;
	case M4E_VKEYDOWN:
		if (gpac->can_seek && evt->key.key_states & M4KM_ALT) {
			s32 res;
			switch (evt->key.m4_vk_code) {
			case M4VK_LEFT:
				res = M4T_GetCurrentTimeInMS(gpac->m_term) - 5*gpac->max_duration/100;
				if (res<0) res=0;
				gpac->PlayFromTime(res);
				break;
			case M4VK_RIGHT:
				res = M4T_GetCurrentTimeInMS(gpac->m_term) + 5*gpac->max_duration/100;
				if ((u32) res>=gpac->max_duration) res = 0;
				gpac->PlayFromTime(res);
				break;
			case M4VK_DOWN:
				res = M4T_GetCurrentTimeInMS(gpac->m_term) - 1000;
				if (res<0) res=0;
				gpac->PlayFromTime(res);
				break;
			case M4VK_UP:
				res = M4T_GetCurrentTimeInMS(gpac->m_term) + 1000;
				if ((u32) res>=gpac->max_duration) res = 0;
				gpac->PlayFromTime(res);
				break;
			}
		}
		break;
	case M4E_NAVIGATE:
		/*fixme - a proper browser would require checking mime type & co*/
		//if (!is_supported_file(gpac->m_config, evt->navigate.to_url, 0)) return 0;
		/*store URL since it may be destroyed, and post message*/
		gpac->m_navigate_url = evt->navigate.to_url;
		pFrame->PostMessage(WM_NAVIGATE, NULL, NULL);
		return 1;
	case M4E_VIEWPOINTS:
		pFrame->BuildViewList();
		return 0;
	case M4E_STREAMLIST:
		pFrame->BuildStreamList(0);
		return 0;
	}
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// WinGPAC initialization

BOOL WinGPAC::InitInstance()
{
	// Standard initialization

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	SetRegistryKey(_T("GPAC"));

	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;
	pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL);
	m_pMainWnd->DragAcceptFiles();

	unsigned char config_path[MAX_PATH];
	strcpy((char *) config_path, AfxGetApp()->m_pszHelpFilePath);
	while (config_path[strlen((char *) config_path)-1] != '\\') config_path[strlen((char *) config_path)-1] = 0;

	Bool first_launch = 0;
	/*init config and plugins*/
	m_config = NewIniFile((const char *) config_path, "GPAC.cfg");
	if (!m_config) {
		first_launch = 1;
		/*create blank config file in the exe dir*/
		unsigned char config_file[MAX_PATH];
		strcpy((char *) config_file, (const char *) config_path);
		strcat((char *) config_file, "GPAC.cfg");
		FILE *ft = fopen((const char *) config_file, "wt");
		fclose(ft);
		m_config = NewIniFile((const char *) config_path, "GPAC.cfg");
		if (!m_config) {
			MessageBox(NULL, "GPAC Configuration file not found", "Fatal Error", MB_OK);
			m_pMainWnd->PostMessage(WM_CLOSE);
		}
	}

	m_config_dir = config_path;

	char *str = IF_GetKey(m_config, "General", "PluginsDirectory");
	m_plugins = NewPluginManager((const unsigned char *) str, m_config);
	if (!m_plugins) {
		char *sOpt;
		/*inital launch*/
		m_plugins = NewPluginManager((const unsigned char *) config_path, m_config);
		if (m_plugins) {
			IF_SetKey(m_config, "General", "PluginsDirectory", (const char *) config_path);

			sOpt = IF_GetKey(m_config, "Render2D", "GraphicsDriver");
			if (!sOpt) IF_SetKey(m_config, "Render2D", "GraphicsDriver", "gdip_rend");

			sOpt = IF_GetKey(m_config, "General", "CacheDirectory");
			if (!sOpt) {
				unsigned char str_path[MAX_PATH];
				sprintf((char *) str_path, "%scache", config_path);
				IF_SetKey(m_config, "General", "CacheDirectory", (const char *) str_path);
			}
			/*setup UDP traffic autodetect*/
			IF_SetKey(m_config, "Network", "AutoReconfigUDP", "yes");
			IF_SetKey(m_config, "Network", "UDPNotAvailable", "no");
			IF_SetKey(m_config, "Network", "UDPTimeout", "10000");
			IF_SetKey(m_config, "Network", "BufferLength", "3000");

			/*first launch, register all files ext*/
			u32 i;
			for (i=0; i<PM_GetPluginsCount(m_plugins); i++) {
				NetClientPlugin *ifce;
				if (!PM_LoadInterface(m_plugins, i, M4STREAMINGCLIENT, (void **) &ifce)) continue;
				if (ifce) {
					ifce->CanHandleURL(ifce, "test.test");
					PM_ShutdownInterface(ifce);
				}
			}
		}

		/*check audio config on windows, force config*/
		sOpt = IF_GetKey(m_config, "Audio", "ForceConfig");
		if (!sOpt) {
			IF_SetKey(m_config, "Audio", "ForceConfig", "yes");
			IF_SetKey(m_config, "Audio", "NumBuffers", "8");
			IF_SetKey(m_config, "Audio", "BuffersPerSecond", "16");
		}
		/*by default use GDIplus, much faster than freetype on font loading*/
		IF_SetKey(m_config, "FontEngine", "DriverName", "gdip_rend");

	}	
	if (! PM_GetPluginsCount(m_plugins) ) {
		MessageBox(NULL, "No plugins available - system cannot work", "Fatal Error", MB_OK);
		m_pMainWnd->PostMessage(WM_CLOSE);
	}

	/*setup font dir*/
	str = IF_GetKey(m_config, "FontEngine", "FontDirectory");
	if (!str) {
		::GetWindowsDirectory((char*)config_path, MAX_PATH);
		if (config_path[strlen((char*)config_path)-1] != '\\') strcat((char*)config_path, "\\");
		strcat((char *)config_path, "Fonts");
		IF_SetKey(m_config, "FontEngine", "FontDirectory", (const char *) config_path);
	}

	/*check video driver, if none or raw_out use dx_hw by default*/
	str = IF_GetKey(m_config, "Video", "DriverName");
	if (!str || !stricmp(str, "raw_out")) {
		IF_SetKey(m_config, "Video", "DriverName", "dx_hw");
	}
	/*setup user*/
	memset(&m_user, 0, sizeof(M4User));
	m_user.config = m_config;
	m_user.plugins = m_plugins;
	m_user.opaque = this;
	m_user.os_window_handler = pFrame->m_pWndView->m_hWnd;
	m_user.EventProc = Osmo4_EventProc;

	m_reset = 0;
	m_prev_time = 0;
	orig_width = 320;
	orig_height = 240;

	m_term = NewMPEG4Term(&m_user);
	if (! m_term) {
		MessageBox(NULL, "Cannot load MPEG-4 Terminal", "Fatal Error", MB_OK);
		m_pMainWnd->PostMessage(WM_CLOSE);
		return TRUE;
	}
	SetOptions();
	UpdateRenderSwitch();

	pFrame->PostMessage(WM_SETSIZE, orig_width, orig_height);
	pFrame->m_Address.ReloadURLs();

	m_reconnect_time = 0;

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	if (! cmdInfo.m_strFileName.IsEmpty()) {
		pFrame->m_pPlayList->QueueURL(cmdInfo.m_strFileName);
		pFrame->m_pPlayList->PlayNext();
	}
	return TRUE;
}


void WinGPAC::ReloadTerminal()
{
	Bool reconnect = m_isopen;
	CMainFrame *pFrame = (CMainFrame *) m_pMainWnd;
	pFrame->console_err = M4OK;
	pFrame->console_message = "Reloading MPEG-4 Terminal";
	m_pMainWnd->PostMessage(WM_CONSOLEMSG, 0, 0);

	m_reconnect_time = 0;
	if (can_seek) m_reconnect_time = M4T_GetCurrentTimeInMS(m_term);

	M4T_Delete(m_term);
	m_term = NewMPEG4Term(&m_user);
	if (!m_term) {
		MessageBox(NULL, "Fatal Error !!", "Couldn't change renderer", MB_OK);
		m_pMainWnd->PostMessage(WM_DESTROY);
		return;
	}
	if (reconnect) m_pMainWnd->PostMessage(WM_OPENURL);
	pFrame->console_message = "MPEG-4 Terminal reloaded";
	m_pMainWnd->PostMessage(WM_CONSOLEMSG, 0, 0);
	UpdateRenderSwitch();
}


/////////////////////////////////////////////////////////////////////////////
// WinGPAC message handlers





/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnGogpac();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_BN_CLICKED(IDC_GOGPAC, OnGogpac)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void WinGPAC::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	CString str = "GPAC/Osmo4 - version " M4_VERSION;
	SetWindowText(str);
	return TRUE;  
}

void CAboutDlg::OnGogpac() 
{
	ShellExecute(NULL, "open", "http://gpac.sourceforge.net", NULL, NULL, SW_SHOWNORMAL);
}

/////////////////////////////////////////////////////////////////////////////
// WinGPAC message handlers


int WinGPAC::ExitInstance() 
{

	M4T_Delete(m_term);
	PM_Delete(m_plugins);
	IF_Delete(m_config);
	return CWinApp::ExitInstance();
}

void WinGPAC::SetOptions()
{
	char *sOpt = IF_GetKey(m_config, "General", "Loop");
	m_Loop = (sOpt && !stricmp(sOpt, "yes")) ? 1 : 0;
	sOpt = IF_GetKey(m_config, "General", "LookForSubtitles");
	m_LookForSubtitles = (sOpt && !stricmp(sOpt, "yes")) ? 1 : 0;
	sOpt = IF_GetKey(m_config, "General", "ConsoleOff");
	m_NoConsole = (sOpt && !stricmp(sOpt, "yes")) ? 1 : 0;
	sOpt = IF_GetKey(m_config, "General", "ViewXMT");
	m_ViewXMTA  = (sOpt && !stricmp(sOpt, "yes")) ? 1 : 0;
}


void WinGPAC::OnOpenUrl() 
{
	COpenUrl url;
	if (url.DoModal() != IDOK) return;

	((CMainFrame *) m_pMainWnd)->m_pPlayList->Truncate();
	((CMainFrame *) m_pMainWnd)->m_pPlayList->QueueURL(url.m_url);
	((CMainFrame *) m_pMainWnd)->m_pPlayList->PlayNext();
}


CString WinGPAC::GetFileFilter()
{
	u32 keyCount, i;
	CString sFiles;
	CString sExts;
	CString supportedFiles;

	supportedFiles = "All Known Files|*.m3u;*.pls";

	sExts = "";
	sFiles = "";
	keyCount = IF_GetKeyCount(m_config, "MimeTypes");
	for (i=0; i<keyCount; i++) {
		const char *sMime;
		Bool first;
		char *sKey;
		char szKeyList[1000], sDesc[1000];
		sMime = IF_GetKeyName(m_config, "MimeTypes", i);
		if (!sMime) continue;
		CString sOpt;
		sKey = IF_GetKey(m_config, "MimeTypes", sMime);
		/*remove plugin name*/
		strcpy(szKeyList, sKey+1);
		sKey = strrchr(szKeyList, '\"');
		if (!sKey) continue;
		sKey[0] = 0;
		/*get description*/
		sKey = strrchr(szKeyList, '\"');
		if (!sKey) continue;
		strcpy(sDesc, sKey+1);
		sKey[0] = 0;
		sKey = strrchr(szKeyList, '\"');
		if (!sKey) continue;
		sKey[0] = 0;

		/*if same description for # mime types skip (means an old mime syntax)*/
		if (sFiles.Find(sDesc)>=0) continue;
		/*if same extensions for # mime types skip (don't polluate the file list)*/
		if (sExts.Find(szKeyList)>=0) continue;

		sExts += szKeyList;
		sExts += " ";
		sFiles += sDesc;
		sFiles += "|";

		first = 1;

		sOpt = CString(szKeyList);
		while (1) {
			
			int pos = sOpt.Find(' ');
			CString ext = (pos==-1) ? sOpt : sOpt.Left(pos);
			/*WATCHOUT: we do have some "double" ext , eg .wrl.gz - these are NOT supported by windows*/
			if (ext.Find(".")<0) {
				if (!first) {
					sFiles += ";";
				} else {
					first = 0;
				}
				sFiles += "*.";
				sFiles += ext;

				CString sext = ext;
				sext += ";";
				if (supportedFiles.Find(sext)<0) {
					supportedFiles += ";*.";
					supportedFiles += ext;
				}
			}

			if (sOpt==ext) break;
			CString rem;
			rem.Format("%s ", (LPCTSTR) ext);
			sOpt.Replace((LPCTSTR) rem, "");
		}
		sFiles += "|";
	}
	supportedFiles += "|";
	supportedFiles += sFiles;
	supportedFiles += "M3U Playlists|*.m3u|ShoutCast Playlists|*.pls|All Files |*.*|";
	return supportedFiles;
}

void WinGPAC::OnOpenFile() 
{
	CString sFiles = GetFileFilter();
	u32 nb_items;
	
	/*looks like there's a bug here, main filter isn't used correctly while the others are*/
	CFileDialog fd(TRUE,NULL,NULL, OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST , sFiles);
	fd.m_ofn.nMaxFile = 25000;
	fd.m_ofn.lpstrFile = (char *) malloc(sizeof(char) * fd.m_ofn.nMaxFile);
	fd.m_ofn.lpstrFile[0] = 0;

	if (fd.DoModal()!=IDOK) {
		free(fd.m_ofn.lpstrFile);
		return;
	}

	CMainFrame *pFrame = (CMainFrame *) m_pMainWnd;

	nb_items = 0;
	POSITION pos = fd.GetStartPosition();
	while (pos) {
		CString file = fd.GetNextPathName(pos);
		nb_items++;
	}
	/*if several items, act as playlist (replace playlist), otherwise as browser (lost all "next" context)*/
	if (nb_items==1) 
		pFrame->m_pPlayList->Truncate();
	else
		pFrame->m_pPlayList->Clear();

	pos = fd.GetStartPosition();
	while (pos) {
		CString file = fd.GetNextPathName(pos);
		pFrame->m_pPlayList->QueueURL(file);
	}
	free(fd.m_ofn.lpstrFile);
	pFrame->m_pPlayList->PlayNext();
}


void WinGPAC::Pause()
{
	if (!m_isopen) return;
	m_paused = !m_paused;
	M4T_Pause(m_term, m_paused);

}

void WinGPAC::OnMainPause() 
{
	Pause();	
}

void WinGPAC::OnFileStep() 
{
	M4T_Pause(m_term, 2);
	m_paused = 1;
	((CMainFrame *) m_pMainWnd)->m_wndToolBar.SetButtonInfo(5, ID_FILE_PLAY, TBBS_BUTTON, 3);
}
void WinGPAC::OnUpdateFileStep(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_isopen && !m_reset);	
}

void WinGPAC::PlayFromTime(u32 time)
{
	if (m_paused) Pause();
	M4T_PlayFromTime(m_term, time);
	m_reset = 0;
}


void WinGPAC::OnFileReload() 
{
	M4T_CloseURL(m_term);
	m_pMainWnd->PostMessage(WM_OPENURL);
}

void WinGPAC::OnFilePlay() 
{
	if (m_isopen) {
		Pause();
		if (m_reset) {
			m_reset = 0;
			PlayFromTime(0);
			((CMainFrame *)m_pMainWnd)->SetProgTimer(1);
		}
		((CMainFrame *) m_pMainWnd)->m_wndToolBar.SetButtonInfo(5, ID_FILE_PLAY, TBBS_BUTTON, m_paused ? 3 : 4);
	} else {
		((CMainFrame *) m_pMainWnd)->m_pPlayList->PlayNext();
	}
}

void WinGPAC::OnUpdateFilePlay(CCmdUI* pCmdUI) 
{
	if (m_isopen) {
		pCmdUI->Enable(TRUE);	
		if (pCmdUI->m_nID==ID_FILE_PLAY) {
			pCmdUI->SetText(m_isopen ? (m_paused ? "Resume\tCtrl+P" : "Pause\tCtrl+P") : "Play/Pause\tCtrl+P");
		}
	} else {
		pCmdUI->Enable(((CMainFrame *)m_pMainWnd)->m_pPlayList->HasValidEntries() );	
		pCmdUI->SetText("Play\tCtrl+P");
	}
}

void WinGPAC::OnFileStop() 
{
	CMainFrame *pFrame = (CMainFrame *) m_pMainWnd;
	if (m_reset) return;
	if (!m_paused) Pause();
	m_reset = 1;
	pFrame->m_Sliders.m_PosSlider.SetPos(0);
	pFrame->SetProgTimer(0);
	pFrame->m_wndToolBar.SetButtonInfo(5, ID_FILE_PLAY, TBBS_BUTTON, 3);
}

void WinGPAC::OnUpdateFileStop(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_isopen);	
}

void WinGPAC::OnSwitchRender() 
{
	char *opt = IF_GetKey(m_config, "Rendering", "RendererName");
	if (!stricmp(opt, "GPAC 2D Renderer"))
		IF_SetKey(m_config, "Rendering", "RendererName", "GPAC 3D Renderer");
	else
		IF_SetKey(m_config, "Rendering", "RendererName", "GPAC 2D Renderer");

	ReloadTerminal();
}

void WinGPAC::UpdateRenderSwitch()
{
	char *opt = IF_GetKey(m_config, "Rendering", "RendererName");
	if (!stricmp(opt, "GPAC 3D Renderer"))
		((CMainFrame *) m_pMainWnd)->m_wndToolBar.SetButtonInfo(12, ID_SWITCH_RENDER, TBBS_BUTTON, 10);
	else
		((CMainFrame *) m_pMainWnd)->m_wndToolBar.SetButtonInfo(12, ID_SWITCH_RENDER, TBBS_BUTTON, 9);
}

void WinGPAC::OnReloadTerminal() 
{
	ReloadTerminal();
}

