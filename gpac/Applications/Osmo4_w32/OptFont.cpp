// OptFont.cpp : implementation file
//

#include "stdafx.h"
#include "Osmo4.h"
#include "OptFont.h"
#include <gpac/m4_graphics.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptFont dialog


COptFont::COptFont(CWnd* pParent /*=NULL*/)
	: CDialog(COptFont::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptFont)
	//}}AFX_DATA_INIT
}


void COptFont::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptFont)
	DDX_Control(pDX, IDC_TEXTURE_TEXT, m_UseTexture);
	DDX_Control(pDX, IDC_FONT_LIST, m_Fonts);
	DDX_Control(pDX, IDC_BROWSE_FONT, m_BrowseFont);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptFont, CDialog)
	//{{AFX_MSG_MAP(COptFont)
	ON_BN_CLICKED(IDC_BROWSE_FONT, OnBrowseFont)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptFont message handlers

BOOL COptFont::OnInitDialog() 
{
	u32 i;
	void *ifce;
	
	CDialog::OnInitDialog();
	
	WinGPAC *gpac = GetApp();
	char *sOpt;

	/*video drivers enum*/
	while (m_Fonts.GetCount()) m_Fonts.DeleteString(0);
	sOpt = IF_GetKey(gpac->m_config, "FontEngine", "DriverName");
	s32 to_sel = 0;
	s32 select = 0;
	u32 count = PM_GetPluginsCount(gpac->m_plugins);
	for (i=0; i<count; i++) {
		if (!PM_LoadInterface(gpac->m_plugins, i, M4_FONT_RASTER_INTERFACE, &ifce)) continue;
		if (sOpt && !stricmp(((BaseInterface *)ifce)->plugin_name, sOpt)) select = to_sel;
		m_Fonts.AddString(((BaseInterface *)ifce)->plugin_name);
		PM_ShutdownInterface(ifce);
		to_sel++;
	}
	m_Fonts.SetCurSel(select);
	

	sOpt = IF_GetKey(gpac->m_config, "FontEngine", "FontDirectory");
	if (sOpt) m_BrowseFont.SetWindowText(sOpt);


	sOpt = IF_GetKey(gpac->m_config, "FontEngine", "UseTextureText");
	m_UseTexture.SetCheck( (sOpt && !stricmp(sOpt, "yes")) ? 1 : 0);

	return TRUE;  
}



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

void COptFont::OnBrowseFont() 
{
	BROWSEINFO brw;
	LPMALLOC pMalloc;
	LPITEMIDLIST ret;
	char dir[MAX_PATH];

	if (NOERROR == ::SHGetMalloc(&pMalloc) ) {

		m_BrowseFont.GetWindowText(szCacheDir, MAX_PATH);

		memset(&brw, 0, sizeof(BROWSEINFO));
		brw.hwndOwner = this->GetSafeHwnd();
		brw.pszDisplayName = dir;
		brw.lpszTitle = "Select Font Directory...";
		brw.ulFlags = 0L;
		brw.lpfn = LocCbck;

		ret = SHBrowseForFolder(&brw);
		if (ret != NULL) {
			if (::SHGetPathFromIDList(ret, dir)) {
				m_BrowseFont.SetWindowText(dir);
			}
			pMalloc->Free(ret);
		}
		pMalloc->Release();
	}
}


void COptFont::SaveOptions()
{
	WinGPAC *gpac = GetApp();
	char str[MAX_PATH];
		
	m_Fonts.GetWindowText(str, 50);
	IF_SetKey(gpac->m_config, "FontEngine", "DriverName", str);
	m_BrowseFont.GetWindowText(str, 50);
	IF_SetKey(gpac->m_config, "FontEngine", "FontDirectory", str);
	IF_SetKey(gpac->m_config, "FontEngine", "UseTextureText", m_UseTexture.GetCheck() ? "yes" : "no");
}
