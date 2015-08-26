// OptGen.cpp : implementation file
//

#include "stdafx.h"
#include "Osmo4.h"
#include "OptGen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptGen dialog


COptGen::COptGen(CWnd* pParent /*=NULL*/)
	: CDialog(COptGen::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptGen)
	//}}AFX_DATA_INIT
}


void COptGen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptGen)
	DDX_Control(pDX, IDC_LOOKFORSUB, m_LookForSubs);
	DDX_Control(pDX, IDC_DUMP_XMT, m_ViewXMT);
	DDX_Control(pDX, IDC_NO_CONSOLE, m_NoConsole);
	DDX_Control(pDX, IDC_LOOP, m_Loop);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptGen, CDialog)
	//{{AFX_MSG_MAP(COptGen)
	ON_BN_CLICKED(IDC_FILEASSOC, OnFileassoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptGen message handlers



BOOL COptGen::OnInitDialog() 
{
	CDialog::OnInitDialog();

	WinGPAC *gpac = GetApp();
	char *sOpt;
	
	sOpt = IF_GetKey(gpac->m_config, "General", "Loop");
	m_Loop.SetCheck((sOpt && !stricmp(sOpt, "yes")) ? 1 : 0);
	sOpt = IF_GetKey(gpac->m_config, "General", "LookForSubtitles");
	m_LookForSubs.SetCheck((sOpt && !stricmp(sOpt, "yes")) ? 1 : 0);
	sOpt = IF_GetKey(gpac->m_config, "General", "ConsoleOff");
	m_NoConsole.SetCheck((sOpt && !stricmp(sOpt, "yes")) ? 1 : 0);
	sOpt = IF_GetKey(gpac->m_config, "General", "ViewXMT");
	m_ViewXMT.SetCheck((sOpt && !stricmp(sOpt, "yes")) ? 1 : 0);
	return TRUE; 
}

void COptGen::SaveOptions()
{
	WinGPAC *gpac = GetApp();

	gpac->m_Loop = m_Loop.GetCheck();
	IF_SetKey(gpac->m_config, "General", "Loop", gpac->m_Loop ? "yes" : "no");
	gpac->m_LookForSubtitles = m_LookForSubs.GetCheck();
	IF_SetKey(gpac->m_config, "General", "LookForSubtitles",  gpac->m_LookForSubtitles ? "yes" : "no");
	gpac->m_NoConsole = m_NoConsole.GetCheck();
	IF_SetKey(gpac->m_config, "General", "ConsoleOff", gpac->m_NoConsole ? "yes" : "no");
	gpac->m_ViewXMTA = m_ViewXMT.GetCheck();
	IF_SetKey(gpac->m_config, "General", "ViewXMT", gpac->m_ViewXMTA ? "yes" : "no");

}

void COptGen::OnFileassoc() 
{
	HKEY hSection;
	unsigned char szDir[MAX_PATH];
	unsigned char cmd[MAX_PATH];
	DWORD ioSize = MAX_PATH;
	DWORD dwDisp;

	RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\GPAC", 0, KEY_READ, &hSection);
	strcpy((char *) szDir, AfxGetApp()->m_pszHelpFilePath);
	while (szDir[strlen((char *) szDir)-1] != '\\') szDir[strlen((char *) szDir)-1] = 0;
	if (!hSection) 
		RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\GPAC", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSection, &dwDisp);

	/*overwrite install dir with current path*/
	RegSetValueEx(hSection, "Install_Dir", 0, REG_SZ, szDir, strlen((const char *) szDir)+1);
	RegCloseKey(hSection);

	/*overwrite .mp4 file associations */
	RegCreateKeyEx(HKEY_CLASSES_ROOT, "mp4file\\DefaultIcon", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSection, &dwDisp);
	strcpy((char *) cmd, (const char *)szDir);
	strcat((char *) cmd, "Osmo4.ico");
	RegSetValueEx(hSection, "", 0, REG_SZ, cmd, strlen((const char *) cmd)+1);
	RegCloseKey(hSection);

	RegCreateKeyEx(HKEY_CLASSES_ROOT, "mp4file\\Shell\\open\\command", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSection, &dwDisp);
	strcpy((char *) cmd, (const char *)szDir);
	strcat((char *) cmd, "Osmo4.exe \"%L\"");
	RegSetValueEx(hSection, "", 0, REG_SZ, cmd, strlen((const char *) cmd)+1);
	RegCloseKey(hSection);

	RegCreateKeyEx(HKEY_CLASSES_ROOT, ".mp4", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSection, &dwDisp);
	RegSetValueEx(hSection, "", 0, REG_SZ, (const unsigned char *) "mp4file", strlen("mp4file")+1);
	RegCloseKey(hSection);
}
