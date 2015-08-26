// OptSystems.cpp : implementation file
//

#include "stdafx.h"
#include "Osmo4.h"
#include "OptSystems.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptSystems dialog


COptSystems::COptSystems(CWnd* pParent /*=NULL*/)
	: CDialog(COptSystems::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptSystems)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void COptSystems::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptSystems)
	DDX_Control(pDX, IDC_FORCE_DURATION, m_ForceDuration);
	DDX_Control(pDX, IDC_DEC_THREAD, m_Threading);
	DDX_Control(pDX, IDC_BIFSDROP, m_BifsAlwaysDrawn);
	DDX_Control(pDX, IDC_LANG, m_Lang);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptSystems, CDialog)
	//{{AFX_MSG_MAP(COptSystems)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptSystems message handlers




/*ISO 639-2 code names (complete set in /doc directory)*/
#define NUM_LANGUAGE	59
static char *Languages[118] = 
{
"Albanian","alb",
"Arabic","ara",
"Armenian","arm",
"Azerbaijani","aze",
"Basque","baq",
"Belarusian","bel",
"Bosnian","bos",
"Breton","bre",
"Bulgarian","bul",
"Catalan","cat",
"Celtic (Other)","cel",
"Chinese","chi",
"Croatian","scr",
"Czech","cze",
"Danish","dan",
"Dutch","dut",
"English","eng",
"Esperanto","epo",
"Estonian","est",
"Fijian","fij",
"Finnish","fin",
"French","fre",
"Georgian","geo",
"German","ger",
"Greek, Modern (1453-)","gre",
"Haitian","hat",
"Hawaiian","haw",
"Hebrew","heb",
"Indonesian","ind",
"Iranian (Other)","ira",
"Irish","gle",
"Italian","ita",
"Japanese","jpn",
"Korean","kor",
"Kurdish","kur",
"Latin","lat",
"Lithuanian","lit",
"Luxembourgish","ltz",
"Macedonian","mac",
"Mongolian","mon",
"Norwegian","nor",
"Occitan (post 1500)","oci",
"Persian","per",
"Philippine (Other)","phi" ,
"Polish","pol",
"Portuguese","por",
"Russian","rus",
"Serbian","srp",
"Slovak","slo",
"Slovenian","slv",
"Somali","som",
"Spanish","spa",
"Swedish","swe",
"Tahitian","tah",
"Thai","tha",
"Tibetan","tib",
"Turkish","tur",
"Undetermined","und",
"Vietnamese","vie",
};


BOOL COptSystems::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	WinGPAC *gpac = GetApp();
	char *sOpt;

	sOpt = IF_GetKey(gpac->m_config, "Systems", "Language");
	if (!sOpt) sOpt = "eng";
	s32 select = 0;
	while (m_Lang.GetCount()) m_Lang.DeleteString(0);
	for (s32 i = 0; i<NUM_LANGUAGE; i++) {
		m_Lang.AddString(Languages[2*i]);
		if (sOpt && !stricmp(sOpt, Languages[2*i + 1])) select = i;
	}
	m_Lang.SetCurSel(select);

	
	/*system config*/
	sOpt = IF_GetKey(gpac->m_config, "Systems", "ThreadingPolicy");
	select = 0;
	while (m_Threading.GetCount()) m_Threading.DeleteString(0);
	m_Threading.AddString("Single Thread");
	m_Threading.AddString("Mutli Thread");
	if (sOpt && !stricmp(sOpt, "Multi")) select = 1;
	m_Threading.AddString("Free");
	if (sOpt && !stricmp(sOpt, "Free")) select = 2;
	m_Threading.SetCurSel(select);


	sOpt = IF_GetKey(gpac->m_config, "Systems", "ForceSingleClock");
	if (sOpt && !stricmp(sOpt, "yes")) {
		m_ForceDuration.SetCheck(1);
	} else {
		m_ForceDuration.SetCheck(0);
	}
	sOpt = IF_GetKey(gpac->m_config, "Systems", "AlwaysDrawBIFS");
	if (sOpt && !stricmp(sOpt, "yes")) {
		m_BifsAlwaysDrawn.SetCheck(1);
	} else {
		m_BifsAlwaysDrawn.SetCheck(0);
	}

	
	return TRUE; 
}


void COptSystems::SaveOptions()
{
	WinGPAC *gpac = GetApp();

	s32 sel = m_Lang.GetCurSel();
	IF_SetKey(gpac->m_config, "Systems", "Language", Languages[2*sel + 1]);
	sel = m_Threading.GetCurSel();
	IF_SetKey(gpac->m_config, "Systems", "ThreadingPolicy", (sel==0) ? "Single" : ( (sel==1) ? "Multi" : "Free"));
	IF_SetKey(gpac->m_config, "Systems", "ForceSingleClock", m_ForceDuration.GetCheck() ? "yes" : "no");
	IF_SetKey(gpac->m_config, "Systems", "AlwaysDrawBIFS", m_BifsAlwaysDrawn.GetCheck() ? "yes" : "no");
}


