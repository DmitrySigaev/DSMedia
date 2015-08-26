// Playlist.cpp : implementation file
//

#include "stdafx.h"
#include "osmo4.h"
#include "MainFrm.h"
#include "OpenURL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Playlist dialog

PLEntry::PLEntry(CString url)
{
	m_url = strdup(url);
	char *str = strrchr(url, '\\');
	if (!str) str = strrchr(url, '/');
	if (str && strlen(str+1)) {
		m_disp_name = strdup(str+1);
		str = strrchr(m_disp_name, '.');
		if (str) str[0] = 0;
	} else {
		str = strstr(url, "://");
		if (str) {
			str += 3;
			m_disp_name = strdup(str);
		} else {
			m_disp_name = strdup(url);
			str = strrchr(m_disp_name, '.');
			if (str) str[0] = 0;
		}
	}
	m_duration = 0;
	m_bIsDead = 0;
	m_bIsPlaying = 0;
	m_bIsSelected = 0;
}

PLEntry::~PLEntry()
{
	if (m_url) free(m_url);
	if (m_disp_name) free(m_disp_name);

}


static char szCacheDir[MAX_PATH];

Playlist::Playlist()
	: CDialog(Playlist::IDD, NULL)
{
	//{{AFX_DATA_INIT(Playlist)
	//}}AFX_DATA_INIT

	m_entries = NewChain();
	m_cur_entry = -1;
	m_all_dead_entries=-1;
	GetCurrentDirectory(MAX_PATH, szCacheDir);
}

Playlist::~Playlist()
{
	Clear();
	DeleteChain(m_entries);
}

void Playlist::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Playlist)
	DDX_Control(pDX, IDC_FILELIST, m_FileList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Playlist, CDialog)
	//{{AFX_MSG_MAP(Playlist)
	ON_WM_SIZE()
	ON_COMMAND(ID_PL_ADD_FILE, OnPlAddFile)
	ON_COMMAND(ID_PL_REM_FILE, OnPlRemFile)
	ON_COMMAND(ID_PL_UP, OnSelUp)
	ON_COMMAND(ID_PL_DOWN, OnSelDown)
	ON_COMMAND(ID_PL_SAVE, OnPlSave)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_PL_REM_DEAD, OnPlRemDead)
	ON_COMMAND(ID_PL_REM_ALL, OnPlRemAll)
	ON_COMMAND(ID_PL_ADD_DIR, OnPlAddDir)
	ON_COMMAND(ID_PL_ADD_DIR_REC, OnPlAddDirRec)
	ON_COMMAND(ID_PL_ADD_URL, OnPlAddUrl)
	ON_COMMAND(ID_PL_OPEN, OnPlOpen)
	ON_COMMAND(ID_PL_PLAY, OnPlPlay)
	ON_COMMAND(ID_PL_SEL_REV, OnReverseSelection)
	ON_COMMAND(ID_PL_SORT_REV, OnReverseList)
	ON_COMMAND(ID_PL_RANDOM, OnRandomize)
	ON_COMMAND(ID_PL_SORT_TITLE, OnSortTitle)
	ON_COMMAND(ID_PL_SORT_FILE, OnSortFile)
	ON_COMMAND(ID_PL_SORT_DUR, OnSortDuration)
	ON_NOTIFY(NM_RCLICK, IDC_FILELIST, OnRclickFilelist)
	ON_NOTIFY(NM_DBLCLK, IDC_FILELIST, OnDblclkFilelist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Playlist message handlers

BOOL Playlist::OnInitDialog() 
{
	UINT buttonArray[50];
	TBBUTTONINFO bi;
	u32 *ba;
	CDialog::OnInitDialog();

	
	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), TRUE);
	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);

	if (!m_toolBar.CreateEx(this, WS_CHILD | CBRS_TOP | CBRS_FLYBY) ||
		!m_toolBar.LoadBitmap(IDR_PLAYLIST))
	{
		TRACE0("Failed to create toolbar\n");
		return 0;
	}
	
	ba = &buttonArray[0];
	*ba = ID_PL_OPEN; ba++;
	*ba = ID_PL_SAVE; ba++;
	*ba = ID_SEPARATOR; ba++;
	*ba = ID_PL_ADD_FILE; ba++;
	*ba = ID_PL_REM_FILE; ba++;
	*ba = ID_SEPARATOR; ba++;
	*ba = ID_PL_UP; ba++;
	*ba = ID_PL_DOWN; ba++;
	*ba = ID_SEPARATOR; ba++;
	*ba = ID_PL_SORT_FILE; ba++;
	m_toolBar.SetButtons(buttonArray, 9);
	m_toolBar.SetButtonInfo(0, ID_PL_OPEN, TBBS_BUTTON, 0);
	m_toolBar.SetButtonInfo(1, ID_PL_SAVE, TBBS_BUTTON, 1);
	m_toolBar.SetButtonInfo(2, ID_SEPARATOR, TBBS_SEPARATOR, 0);
	m_toolBar.SetButtonInfo(3, ID_PL_ADD_FILE, TBBS_DROPDOWN | TBBS_BUTTON, 2);
	m_toolBar.SetButtonInfo(4, ID_PL_REM_FILE, TBBS_DROPDOWN | TBBS_BUTTON, 3);
	m_toolBar.SetButtonInfo(5, ID_SEPARATOR, TBBS_SEPARATOR, 0);
	m_toolBar.SetButtonInfo(6, ID_PL_UP, TBBS_BUTTON, 4);
	m_toolBar.SetButtonInfo(7, ID_PL_DOWN, TBBS_BUTTON, 5);
//	m_toolBar.SetButtonInfo(8, ID_SEPARATOR, TBBS_SEPARATOR, 0);
	m_toolBar.SetButtonInfo(8, ID_PL_SORT_FILE, TBBS_DROPDOWN | TBBS_BUTTON, 6);

	CToolBarCtrl &ctrl = m_toolBar.GetToolBarCtrl();
	ctrl.SetStyle(TBSTYLE_FLAT | TBSTYLE_DROPDOWN);
	ctrl.SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);

	memset(&bi, 0, sizeof(bi));
	bi.cbSize = sizeof(bi);
	ctrl.GetButtonInfo(3, &bi);
	bi.fsStyle |= TBSTYLE_DROPDOWN;
	ctrl.SetButtonInfo(ID_PL_ADD_FILE, &bi);

	memset(&bi, 0, sizeof(bi));
	bi.cbSize = sizeof(bi);
	ctrl.GetButtonInfo(4, &bi);
	bi.fsStyle |= TBBS_DROPDOWN | TBSTYLE_DROPDOWN;
	ctrl.SetButtonInfo(ID_PL_REM_FILE, &bi);

	memset(&bi, 0, sizeof(bi));
	bi.cbSize = sizeof(bi);
	ctrl.GetButtonInfo(9, &bi);
	bi.fsStyle |= TBSTYLE_DROPDOWN;
	ctrl.SetButtonInfo(ID_PL_SORT_FILE, &bi);

	m_FileList.InsertColumn(0, "", LVCFMT_LEFT, 30, 0);
	m_FileList.InsertColumn(1, "Title", LVCFMT_LEFT, 200, 1);
	m_FileList.InsertColumn(2, "Duration", LVCFMT_LEFT, 200, 2);


	m_toolBar.UpdateWindow();
	m_toolBar.ShowWindow(SW_SHOW);

	SetWindowPos(NULL, 0, 0, 400, 600, SWP_NOZORDER | SWP_NOMOVE);

	PostMessage(WM_NULL);
	return TRUE;  
}

void Playlist::OnSize(UINT nType, int cx, int cy) 
{
	u32 tool_h;
	CDialog::OnSize(nType, cx, cy);
	RECT rc;
	if (!m_toolBar.m_hWnd) return;
	if (!m_FileList.m_hWnd) return;

	m_toolBar.GetClientRect(&rc);
	tool_h = rc.bottom - rc.top;
	m_toolBar.SetWindowPos(this, 0, 0, cx, tool_h, SWP_NOZORDER);		
	m_FileList.SetWindowPos(this, 0, tool_h, cx, cy-tool_h, SWP_NOZORDER);		

	m_FileList.SetExtendedStyle(m_FileList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	m_FileList.SetColumnWidth(0, 30);
	m_FileList.SetColumnWidth(2, 60);
	m_FileList.SetColumnWidth(1, cx-95);
	
}

BOOL Playlist::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{

	if (((LPNMHDR)lParam)->code == TBN_DROPDOWN) {
		RECT rc;
		POINT pt;
		CMenu *pPopup = new CMenu();
		pPopup->CreatePopupMenu();

		m_toolBar.GetWindowRect(&rc);
		pt.y = rc.bottom;
		pt.x = rc.left;

		if ( ((LPNMTOOLBAR)lParam)->iItem == ID_PL_ADD_FILE) {
			pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_PL_ADD_DIR, "Directory...");
			pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_PL_ADD_DIR_REC, "Directory and subfolders...");
			pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_PL_ADD_URL, "URL...");
			m_toolBar.GetToolBarCtrl().GetItemRect(3, &rc);
			pt.x += rc.left;
		} else if ( ((LPNMTOOLBAR)lParam)->iItem == ID_PL_REM_FILE) {
			pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_PL_REM_ALL, "Clear");
			pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_PL_REM_DEAD, "Remove dead entries");

			m_toolBar.GetToolBarCtrl().GetItemRect(4, &rc);
			pt.x += rc.left;
		} else if ( ((LPNMTOOLBAR)lParam)->iItem == ID_PL_SORT_FILE) {
			pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_PL_SORT_TITLE, "Sort Files by title");
			pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_PL_SORT_FILE, "Sort Files by filename");
			pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_PL_SORT_DUR, "Sort Files by duration");
			pPopup->AppendMenu(MF_SEPARATOR,0);
			pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_PL_SORT_REV, "Reverse Playlist");
			pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_PL_RANDOM, "Shuffle Playlist");

			m_toolBar.GetToolBarCtrl().GetItemRect(8, &rc);
			pt.x += rc.left;
		}
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
		delete pPopup;

		return FALSE;
	}
	return CDialog::OnNotify(wParam, lParam, pResult);
}

void Playlist::Clear()
{
	m_FileList.DeleteAllItems();
	while (ChainGetCount(m_entries)) {
		PLEntry *ple = (PLEntry *) ChainGetEntry(m_entries, 0);
		ChainDeleteEntry(m_entries, 0);
		delete ple;
	}
	m_cur_entry = -1;
}

void Playlist::ClearButPlaying()
{
	PLEntry *p = NULL;
	if (m_cur_entry >= 0) {
		p = (PLEntry *) ChainGetEntry(m_entries, m_cur_entry);
		ChainDeleteEntry(m_entries, m_cur_entry);
	}
	Clear();
	if (p) {
		ChainAddEntry(m_entries, p);
		m_cur_entry = 0;
	}
	RefreshList();
}

void Playlist::RefreshList()
{
	u32 i, top_idx;
	char szText[20];

	top_idx = m_FileList.GetTopIndex();
	m_FileList.DeleteAllItems();

	for (i=0; i<ChainGetCount(m_entries); i++) {
		PLEntry *ple = (PLEntry *) ChainGetEntry(m_entries, i);
		m_FileList.InsertItem(i, "");
		m_FileList.SetItemData(i, (u32) ple);

		if (i+1<10) sprintf(szText, "00%d", i+1);
		else if (i+1<100) sprintf(szText, "0%d", i+1);
		else sprintf(szText, "%d", i+1);
		m_FileList.SetItem(i, 0, LVIF_TEXT, szText, 0, 0, 0, 0);

		CString str;
		if (ple->m_bIsDead) { str = "!! DEAD !! "; str += ple->m_disp_name; }
		else if (ple->m_bIsPlaying) { 
			str = ">> "; str += ple->m_disp_name; str += " >>"; 
			if (m_cur_entry<0) m_cur_entry = i;
		}
		else str = ple->m_disp_name;
		m_FileList.SetItem(i, 1, LVIF_TEXT, str, 0, 0, 0, 0);
		
		if (ple->m_duration) {
			u32 h = (u32) (ple->m_duration / 3600);
			u32 m = (u32) (ple->m_duration / 60) - h*60;
			u32 s = (u32) (ple->m_duration) - h*3600 - m*60;
			sprintf(szText, "%02d:%02d:%02d", h, m, s);
			m_FileList.SetItem(i, 2, LVIF_TEXT, szText, 0, 0, 0, 0);
		} else {
			m_FileList.SetItem(i, 2, LVIF_TEXT, "Unknown", 0, 0, 0, 0);
		}

		if (ple->m_bIsSelected) {
			m_FileList.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
			ple->m_bIsSelected = 0;
		}
	}
	
	if (m_cur_entry >= (s32) ChainGetCount(m_entries)-1) m_cur_entry = ChainGetCount(m_entries)-1;
	else {
		s32 last_idx = top_idx + m_FileList.GetCountPerPage();
		m_FileList.EnsureVisible(top_idx, 0);
		if (ChainGetCount(m_entries)<1+ (u32) last_idx) last_idx = ChainGetCount(m_entries)-1;
		m_FileList.EnsureVisible(last_idx, 1);
	}
}

void Playlist::OnPlAddFile() 
{
	WinGPAC *app = GetApp();
	CString sFiles = app->GetFileFilter();
	
	CFileDialog fd(TRUE,NULL,NULL, OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST , sFiles);
	fd.m_ofn.nMaxFile = 25000;
	fd.m_ofn.lpstrFile = (char *) malloc(sizeof(char) * fd.m_ofn.nMaxFile);
	fd.m_ofn.lpstrFile[0] = 0;

	if (fd.DoModal() == IDOK) {
		s32 cur = m_FileList.GetItemCount();
		POSITION pos = fd.GetStartPosition();
		while (pos) {
			QueueURL(fd.GetNextPathName(pos));
		}
	}
	free(fd.m_ofn.lpstrFile);
	m_all_dead_entries=-1;
	RefreshList();
}

void Playlist::OnClose() 
{
	ShowWindow(SW_HIDE);
}

void Playlist::OnPlRemFile() 
{
	if (!m_FileList.GetSelectedCount()) return;
	POSITION pos = m_FileList.GetFirstSelectedItemPosition();
	while (pos != NULL) {
		int nItem = m_FileList.GetNextSelectedItem(pos);
		PLEntry *ple = (PLEntry *) m_FileList.GetItemData(nItem);
		ChainDeleteItem(m_entries, ple);
		if (ple->m_bIsPlaying) m_cur_entry = -1;
		delete ple;
	}
	m_all_dead_entries=-1;
	RefreshList();
}

void Playlist::OnSelUp() 
{
	s32 i;
	if (!m_FileList.GetSelectedCount()) return;
	POSITION pos = m_FileList.GetFirstSelectedItemPosition();
	int nItem = m_FileList.GetNextSelectedItem(pos);
	if (nItem==0) return;

	pos = m_FileList.GetFirstSelectedItemPosition();
	while (pos != NULL) {
		nItem = m_FileList.GetNextSelectedItem(pos);
		PLEntry *ple = (PLEntry *) m_FileList.GetItemData(nItem);
		i = ChainDeleteItem(m_entries, ple);
		assert(i>=1);
		ChainInsertEntry(m_entries, ple, i-1);
		ple->m_bIsSelected = 1;
	}
	RefreshList();
}

void Playlist::OnSelDown() 
{
	s32 i, nItem;
	if (!m_FileList.GetSelectedCount()) return;
	POSITION pos = m_FileList.GetFirstSelectedItemPosition();
	while (pos != NULL) nItem = m_FileList.GetNextSelectedItem(pos);

	if ((u32) nItem + 1 == ChainGetCount(m_entries)) return;

	pos = m_FileList.GetFirstSelectedItemPosition();
	while (pos != NULL) {
		nItem = m_FileList.GetNextSelectedItem(pos);
		PLEntry *ple = (PLEntry *) m_FileList.GetItemData(nItem);
		i = ChainDeleteItem(m_entries, ple);
		ChainInsertEntry(m_entries, ple, i+1);
		ple->m_bIsSelected = 1;
	}
	RefreshList();
}

void Playlist::OnPlRemAll() 
{
	Clear();
	RefreshList();
	m_cur_entry = -1;
}

void Playlist::OnPlRemDead() 
{
	for (u32 i=0; i<ChainGetCount(m_entries); i++) {
		PLEntry *ple = (PLEntry *) ChainGetEntry(m_entries, i);
		if (!ple->m_bIsDead) continue;
		ChainDeleteEntry(m_entries, i);
		i--;
		delete ple;
	}
	m_all_dead_entries=-1;
	RefreshList();
}


static int CALLBACK LocCbck(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData) 
{
	char dir[MAX_PATH];
	if (uMsg == BFFM_INITIALIZED) {
		strcpy(dir, szCacheDir);
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE,(LPARAM) dir);
	}
	return 0;
}
static Bool pl_enum_dir_item(void *cbck, char *item_name, char *item_path)
{
	WinGPAC *app = GetApp();
	Playlist *_this = (Playlist *)cbck;

	if (is_supported_file(app->m_config, item_name, 1)) {
		_this->QueueURL(item_path);
	}
	return 0;
}

static Bool pl_enum_dir_dirs(void *cbck, char *item_name, char *item_path)
{
	DIR_Enum(item_path, 0, pl_enum_dir_item, cbck);
	DIR_Enum(item_path, 1, pl_enum_dir_dirs, cbck);
	return 0;
}


void Playlist::AddDir(Bool do_recurse) 
{
	BROWSEINFO brw;
	LPMALLOC pMalloc;
	LPITEMIDLIST ret;
	char dir[MAX_PATH];

	Bool res = 0;
	if (NOERROR == ::SHGetMalloc(&pMalloc) ) {
		memset(&brw, 0, sizeof(BROWSEINFO));
		brw.hwndOwner = this->GetSafeHwnd();
		brw.pszDisplayName = dir;
		brw.lpszTitle = "Select Directory...";
		brw.ulFlags = 0L;
		brw.lpfn = LocCbck;

		ret = SHBrowseForFolder(&brw);
		if (ret != NULL) {
			if (::SHGetPathFromIDList(ret, dir)) res = 1;
			pMalloc->Free(ret);
		}
		pMalloc->Release();
	}
	if (!res) return;
	strcpy(szCacheDir, dir);

	DIR_Enum(dir, 0, pl_enum_dir_item, this);
	if (do_recurse) DIR_Enum(dir, 1, pl_enum_dir_dirs, this);
	m_all_dead_entries=-1;
	RefreshList();
}
void Playlist::OnPlAddDir() 
{
	AddDir(0);
}
void Playlist::OnPlAddDirRec() 
{
	AddDir(1);
}

void Playlist::OnPlAddUrl() 
{
	COpenUrl url;
	if (url.DoModal() != IDOK) return;
	PLEntry *ple = new PLEntry(url.m_url);
	ChainAddEntry(m_entries, ple);
	m_all_dead_entries=-1;
	RefreshList();
}

void Playlist::OnPlSave() 
{
	Bool save_m3u;
	char szPath[M4_MAX_PATH];
	if (!ChainGetCount(m_entries)) return;
	CFileDialog fd(FALSE,NULL,NULL, OFN_OVERWRITEPROMPT, "M3U Playlists|*.m3u|ShoutCast Playlists|*.pls|");
	if (fd.DoModal() != IDOK) return;

	strcpy(szPath, fd.GetPathName());
	strlwr(szPath);
	save_m3u = (fd.m_ofn.nFilterIndex==1) ? 1 : 0;
	if (save_m3u) {
		if (!strstr(szPath, ".m3u")) {
			strcpy(szPath, fd.GetPathName());
			strcat(szPath, ".m3u");
		} else {
			strcpy(szPath, fd.GetPathName());
		}
	} else {
		if (!strstr(szPath, ".pls")) {
			strcpy(szPath, fd.GetPathName());
			strcat(szPath, ".pls");
		} else {
			strcpy(szPath, fd.GetPathName());
		}
	}

	FILE *out = fopen(szPath, "wt");
	if (!save_m3u) 
		fprintf(out, "[playlist]\nNumberOfEntries=%d\n", ChainGetCount(m_entries));

	for (u32 i=0; i<ChainGetCount(m_entries); i++) {
		PLEntry *ple = (PLEntry *) ChainGetEntry(m_entries, i);
		if (save_m3u) {
			fprintf(out, "%s\n", ple->m_url);
		} else {
			fprintf(out, "File%d=%s\n", i+1, ple->m_url);
			fprintf(out, "Title%d=%s\n", i+1, ple->m_disp_name);
			fprintf(out, "Length%d=%d\n", i+1, ple->m_duration ? ple->m_duration : -1);

		}
	}
	if (!save_m3u) fprintf(out, "Version=2\n");

	fprintf(out, "\n");
	fclose(out);
}

void Playlist::OnPlOpen() 
{
	CFileDialog fd(TRUE,NULL,NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, "M3U & PLS Playlists|*.m3u;*.pls|M3U Playlists|*.m3u|ShoutCast Playlists|*.pls|");
	if (fd.DoModal() != IDOK) return;

	Clear();
	OpenPlayList(fd.GetPathName());
	m_all_dead_entries=-1;
	m_cur_entry = 0;
	Play();
}

void Playlist::OpenPlayList(CString fileName)
{
	FILE *pl;
	PLEntry *ple;
	Bool load_m3u, go;
	char szLine[M4_MAX_PATH];
	
	pl = fopen(fileName, "rt");
	ple = NULL;
	load_m3u = 1;
	while (!feof(pl)) {
		fgets(szLine, M4_MAX_PATH, pl);
		go = 1;
		while (go) {
			switch (szLine[strlen(szLine)-1]) {
			case '\n':
			case '\r':
			case ' ':
				szLine[strlen(szLine)-1] = 0;
				break;
			default:
				go = 0;
				break;
			}
		}
		if (!strlen(szLine)) continue;
		if (!stricmp(szLine, "[playlist]")) {
			load_m3u = 0;
		} else if (load_m3u) {
			ple = new PLEntry(szLine);
			ChainAddEntry(m_entries, ple);
		} else if (!strnicmp(szLine, "file", 4)) {
			char *st = strchr(szLine, '=');
			if (!st) ple = NULL;
			else {
				ple = new PLEntry(st + 1);
				ChainAddEntry(m_entries, ple);
			}
		} else if (ple && !strnicmp(szLine, "Length", 6)) {
			char *st = strchr(szLine, '=');
			s32 d = atoi(st + 1);
			if (d>0) ple->m_duration = d;
		} else if (ple && !strnicmp(szLine, "Title", 5)) {
			char *st = strchr(szLine, '=');
			free(ple->m_disp_name);
			ple->m_disp_name = strdup(st + 6);
		}
	}
}


void Playlist::OnRclickFilelist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (!m_FileList.GetItemCount()) return;

	CMenu *pPopup = new CMenu();	
	pPopup->CreatePopupMenu();

	if (m_FileList.GetSelectedCount()==1) {
		pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_PL_PLAY, "Play");
		pPopup->AppendMenu(MF_SEPARATOR, 0, "");
	}
	pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_PL_SEL_REV, "Inverse Selection");
	if (m_FileList.GetSelectedCount()) pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_PL_REM_FILE, "Remove File(s)");
	if (m_FileList.GetItemCount()>1) {
		pPopup->AppendMenu(MF_SEPARATOR, 0, "");
		pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_PL_SORT_TITLE, "Sort By Title");
		pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_PL_SORT_FILE, "Sort By File Name");
		pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_PL_SORT_DUR, "Sort By Duration");
		pPopup->AppendMenu(MF_SEPARATOR, 0, "");
		pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_PL_SORT_REV, "Reverse List");
		pPopup->AppendMenu(MF_STRING | MF_ENABLED, ID_PL_RANDOM, "Randomize");
	}
	
	POINT pt;
    GetCursorPos(&pt);
	pPopup->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	delete pPopup;

	*pResult = 0;
}

void Playlist::OnReverseSelection() 
{
	u32 i;
	POSITION pos = m_FileList.GetFirstSelectedItemPosition();
	while (pos != NULL) {
		int nItem = m_FileList.GetNextSelectedItem(pos);
		PLEntry *ple = (PLEntry *) m_FileList.GetItemData(nItem);
		ple->m_bIsSelected = 1;
	}

	for (i=0; i<ChainGetCount(m_entries); i++) {
		PLEntry *ple = (PLEntry *) ChainGetEntry(m_entries, i);
		ple->m_bIsSelected = !ple->m_bIsSelected;
	}
	RefreshList();
}

void Playlist::OnReverseList()
{
	u32 count = ChainGetCount(m_entries);
	u32 hcount = count / 2;
	count--;
	for (u32 i=0; i<hcount; i++) {
		PLEntry *ple1 = (PLEntry *) ChainGetEntry(m_entries, i);
		PLEntry *ple2 = (PLEntry *) ChainGetEntry(m_entries, count-i);
		ChainDeleteEntry(m_entries, i);
		ChainInsertEntry(m_entries, ple2, i);
		ChainDeleteEntry(m_entries, count-i);
		ChainInsertEntry(m_entries, ple1, count-i);
	}
	RefreshList();
}

void Playlist::OnRandomize()
{
	Chain *new_entries = NewChain();

	M4_RandInit(0);

	while (ChainGetCount(m_entries)>1) {
		u32 pos = M4_Rand() % (ChainGetCount(m_entries)-1);
		PLEntry *ple = (PLEntry *)ChainGetEntry(m_entries, pos);
		ChainDeleteEntry(m_entries, pos);
		ChainAddEntry(new_entries, ple);
	}
	PLEntry *ple = (PLEntry *)ChainGetEntry(m_entries, 0);
	ChainDeleteEntry(m_entries, 0);
	ChainAddEntry(new_entries, ple);

	DeleteChain(m_entries);
	m_entries = new_entries;
	m_cur_entry = -1;
	RefreshList();
}

void Playlist::Sort(u32 type)
{
	u32 i, j, smallest;
	if (ChainGetCount(m_entries)<=1) return;

	for (i=0; i<ChainGetCount(m_entries)-1; i++) {
		smallest = i;
		for (j=i+1; j<ChainGetCount(m_entries); j++) {
			PLEntry *ple2 = (PLEntry *) ChainGetEntry(m_entries, smallest);
			PLEntry *ple1 = (PLEntry *) ChainGetEntry(m_entries, j);
			s32 test = 0;
			switch (type) {
			case 0:
                test = stricmp(ple1->m_url, ple2->m_url);
				break;
			case 1:
                test = stricmp(ple1->m_disp_name, ple2->m_disp_name);
				break;
			case 2:
                test = ple1->m_duration - ple2->m_duration;
				break;
			}
			if (test<0) smallest = j;
		}
		PLEntry *ple = (PLEntry *)ChainGetEntry(m_entries, smallest);
		ChainDeleteEntry(m_entries, smallest);
		ChainInsertEntry(m_entries, ple, i);
	}
	m_cur_entry = -1;
	RefreshList();
}

void Playlist::OnSortFile() { Sort(0); }
void Playlist::OnSortTitle() { Sort(1); }
void Playlist::OnSortDuration() { Sort(2); }


Bool Playlist::HasValidEntries()
{
	u32 nb_dead = 0;
	if (m_all_dead_entries==-1) {
		for (u32 i=0; i<ChainGetCount(m_entries); i++) {
			PLEntry *ple = (PLEntry *) ChainGetEntry(m_entries, i);
			ple->m_bIsPlaying = 0;
			if (ple->m_bIsDead) nb_dead ++;
		}
		m_all_dead_entries = (nb_dead==ChainGetCount(m_entries)) ? 1 : 0;
	}
	if (m_all_dead_entries==1) return 0;
	return 1;
}

void Playlist::Play()
{
	if (m_cur_entry >= (s32)ChainGetCount(m_entries)) {
		if (!GetApp()->m_Loop) return;
		m_cur_entry = 0;
	}
	if (!HasValidEntries()) return;

	for (u32 i=0; i<ChainGetCount(m_entries); i++) {
		PLEntry *ple = (PLEntry *) ChainGetEntry(m_entries, i);
		ple->m_bIsPlaying = 0;
	}

	PLEntry *ple = (PLEntry *) ChainGetEntry(m_entries, m_cur_entry);
	if (ple->m_bIsDead) {
		m_cur_entry++;
		Play();
	} else {
		ple->m_bIsPlaying = 1;
		GetApp()->m_pMainWnd->PostMessage(WM_OPENURL);
		RefreshList();
	}
}

void Playlist::OnDblclkFilelist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	POSITION pos = m_FileList.GetFirstSelectedItemPosition();
	m_cur_entry = m_FileList.GetNextSelectedItem(pos);
	Play();
	*pResult = 0;
}

void Playlist::OnPlPlay()
{
	POSITION pos = m_FileList.GetFirstSelectedItemPosition();
	m_cur_entry = m_FileList.GetNextSelectedItem(pos);
	Play();
}

void Playlist::Truncate()
{
	if (m_cur_entry<0) return;
	while ((u32) m_cur_entry+1 < ChainGetCount(m_entries)) {
		PLEntry *ple = (PLEntry *) ChainGetEntry(m_entries, m_cur_entry+1);
		ChainDeleteEntry(m_entries, m_cur_entry+1);
		delete ple;
	}
	RefreshList();
}


void Playlist::QueueURL(CString filename)
{
	char *ext = strrchr(filename, '.');
	if (ext && (!stricmp(ext, ".m3u") || !stricmp(ext, ".pls")) ) {
		OpenPlayList(filename);
	} else {
		PLEntry *ple = new PLEntry(filename);
		ChainAddEntry(m_entries, ple);
	}
	m_all_dead_entries=-1;
}

void Playlist::PlayNext()
{
	m_cur_entry++;
	Play();
}

void Playlist::PlayPrev()
{
	if (m_cur_entry>0) {
		m_cur_entry--;
		Play();
	}
}

void Playlist::SetDead()
{
	PLEntry *ple = (PLEntry *) ChainGetEntry(m_entries, m_cur_entry);
	if (ple) {
		ple->m_bIsDead = 1;
		RefreshList();
		m_all_dead_entries=-1;
		if (ple->m_bIsPlaying) PlayNext();
	}
}
void Playlist::SetDuration(u32 duration)
{
	PLEntry *ple = (PLEntry *) ChainGetEntry(m_entries, m_cur_entry);
	if (ple) {
		ple->m_duration = duration;
		RefreshList();
	}
}

CString Playlist::GetDisplayName()
{
	PLEntry *ple = (PLEntry *) ChainGetEntry(m_entries, m_cur_entry);
	if (ple) return CString(ple->m_disp_name);
	return CString("");
}

CString Playlist::GetURL()
{
	PLEntry *ple = (PLEntry *) ChainGetEntry(m_entries, m_cur_entry);
	if (ple) return CString(ple->m_url);
	return CString("");
}

