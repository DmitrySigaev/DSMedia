// OptHTTP.cpp : implementation file
//

#include "stdafx.h"
#include "Osmo4.h"
#include "OptHTTP.h"

//#include <shlobj.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptHTTP dialog


COptHTTP::COptHTTP(CWnd* pParent /*=NULL*/)
	: CDialog(COptHTTP::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptHTTP)
	//}}AFX_DATA_INIT
}


void COptHTTP::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptHTTP)
	DDX_Control(pDX, IDC_RESTART_CACHE, m_RestartFile);
	DDX_Control(pDX, IDC_CLEAN_CACHE, m_CleanCache);
	DDX_Control(pDX, IDC_BROWSE_CACHE, m_CacheDir);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptHTTP, CDialog)
	//{{AFX_MSG_MAP(COptHTTP)
	ON_BN_CLICKED(IDC_BROWSE_CACHE, OnBrowseCache)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptHTTP message handlers


static char szCacheDir[MAX_PATH];

static int CALLBACK LocCbck(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData) 
{
	char dir[MAX_PATH];
	if (uMsg == BFFM_INITIALIZED) {
		strcpy(dir, szCacheDir);
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE,(LPARAM) dir);
	}
	return 0;
}

void COptHTTP::OnBrowseCache() 
{
	BROWSEINFO brw;
	LPMALLOC pMalloc;
	LPITEMIDLIST ret;
	char dir[MAX_PATH];

	if (NOERROR == ::SHGetMalloc(&pMalloc) ) {

		m_CacheDir.GetWindowText(szCacheDir, MAX_PATH);

		memset(&brw, 0, sizeof(BROWSEINFO));
		brw.hwndOwner = this->GetSafeHwnd();
		brw.pszDisplayName = dir;
		brw.lpszTitle = "Select HTTP Cache Directory...";
		brw.ulFlags = 0L;
		brw.lpfn = LocCbck;

		ret = SHBrowseForFolder(&brw);
		if (ret != NULL) {
			if (::SHGetPathFromIDList(ret, dir)) {
				m_CacheDir.SetWindowText(dir);
			}
			pMalloc->Free(ret);
		}
		pMalloc->Release();
	}
}

BOOL COptHTTP::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	WinGPAC *gpac = GetApp();
	char *sOpt;

	sOpt = IF_GetKey(gpac->m_config, "Downloader", "CleanCache");
	if (sOpt && !stricmp(sOpt, "yes")) {
		m_CleanCache.SetCheck(1);
	} else {
		m_CleanCache.SetCheck(0);
	}
	sOpt = IF_GetKey(gpac->m_config, "Downloader", "RestartFiles");
	if (sOpt && !stricmp(sOpt, "yes")) {
		m_RestartFile.SetCheck(1);
	} else {
		m_RestartFile.SetCheck(0);
	}

	sOpt = IF_GetKey(gpac->m_config, "General", "CacheDirectory");
	if (sOpt) m_CacheDir.SetWindowText(sOpt);
	
	return TRUE; 
}


void COptHTTP::SaveOptions()
{
	WinGPAC *gpac = GetApp();

	IF_SetKey(gpac->m_config, "Downloader", "CleanCache", m_CleanCache.GetCheck() ? "yes" : "no");
	IF_SetKey(gpac->m_config, "Downloader", "RestartFiles", m_RestartFile.GetCheck() ? "yes" : "no");

	m_CacheDir.GetWindowText(szCacheDir, MAX_PATH);
	IF_SetKey(gpac->m_config, "General", "CacheDirectory", szCacheDir);
}
