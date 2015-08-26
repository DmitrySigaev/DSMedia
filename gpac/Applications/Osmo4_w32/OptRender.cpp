// OptRender.cpp : implementation file
//

#include "stdafx.h"
#include "Osmo4.h"
#include "OptRender.h"
#include <gpac/m4_graphics.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptRender dialog


COptRender::COptRender(CWnd* pParent /*=NULL*/)
	: CDialog(COptRender::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptRender)
	//}}AFX_DATA_INIT
}


void COptRender::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptRender)
	DDX_Control(pDX, IDC_DRAW_BOUNDS, m_DrawBounds);
	DDX_Control(pDX, IDC_GD_LIST, m_Graphics);
	DDX_Control(pDX, IDC_USE_RENDER3D, m_Use3DRender);
	DDX_Control(pDX, IDC_AA_LIST, m_AntiAlias);
	DDX_Control(pDX, IDC_FORCE_SIZE, m_ForceSize);
	DDX_Control(pDX, IDC_FAST_RENDER, m_FastRender);
	DDX_Control(pDX, IDC_BIFS_RATE, m_BIFSRate);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptRender, CDialog)
	//{{AFX_MSG_MAP(COptRender)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptRender message handlers



#define NUM_RATES 11
static char *BIFSRates[11] = 
{
	"5.0",
	"7.5",
	"10.0",
	"12.5",
	"15.0",
	"24.0",
	"25.0",
	"30.0",
	"50.0",
	"60.0",
	"100.0",
};



BOOL COptRender::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	WinGPAC *gpac = GetApp();
	char *sOpt;
	
	sOpt = IF_GetKey(gpac->m_config, "Rendering", "RendererName");
	m_Use3DRender.SetCheck( (sOpt && strstr(sOpt, "3D")) ? 1 : 0);

	sOpt = IF_GetKey(gpac->m_config, "Rendering", "ForceSceneSize");
	m_ForceSize.SetCheck( (sOpt && !stricmp(sOpt, "yes")) ? 1 : 0);

	sOpt = IF_GetKey(gpac->m_config, "Rendering", "FrameRate");
	if (!sOpt) sOpt = "30.0";
	s32 select = 0;
	while (m_BIFSRate.GetCount()) m_BIFSRate.DeleteString(0);
	for (s32 i = 0; i<NUM_RATES; i++) {
		m_BIFSRate.AddString(BIFSRates[i]);
		if (sOpt && !stricmp(sOpt, BIFSRates[i]) ) select = i;
	}
	m_BIFSRate.SetCurSel(select);
	
	sOpt = IF_GetKey(gpac->m_config, "Rendering", "FastRender");
	m_FastRender.SetCheck( (sOpt && !stricmp(sOpt, "yes")) ? 1 : 0);
	
	sOpt = IF_GetKey(gpac->m_config, "Rendering", "AntiAlias");
	while (m_AntiAlias.GetCount()) m_AntiAlias.DeleteString(0);

	m_AntiAlias.AddString("None");
	m_AntiAlias.AddString("Text only");
	m_AntiAlias.AddString("Complete");
	select = 2;
	if (sOpt && !stricmp(sOpt, "Text")) select = 1;
	else if (sOpt && !stricmp(sOpt, "None")) select = 0;
	m_AntiAlias.SetCurSel(select);

	/*graphics driver enum*/
	while (m_Graphics.GetCount()) m_Graphics.DeleteString(0);
	sOpt = IF_GetKey(gpac->m_config, "Rendering", "GraphicsDriver");
	s32 count = PM_GetPluginsCount(gpac->m_plugins);
	void *ifce;
	select = 0;
	u32 to_sel = 0;
	for (i=0; i<count; i++) {
		if (!PM_LoadInterface(gpac->m_plugins, i, M4_GRAPHICS_2D_INTERFACE, &ifce)) continue;
		if (sOpt && !stricmp(((BaseInterface *)ifce)->plugin_name, sOpt)) select = to_sel;
		m_Graphics.AddString(((BaseInterface *)ifce)->plugin_name);
		PM_ShutdownInterface(ifce);
		to_sel++;
	}
	m_Graphics.SetCurSel(select);


	m_DrawBounds.AddString("None");
	m_DrawBounds.AddString("Box/Rect");
	m_DrawBounds.AddString("AABB Tree");
	sOpt = IF_GetKey(gpac->m_config, "Rendering", "BoundingVolume");
	if (sOpt && !stricmp(sOpt, "Box")) m_DrawBounds.SetCurSel(1);
	else if (sOpt && !stricmp(sOpt, "AABB")) m_DrawBounds.SetCurSel(2);
	else m_DrawBounds.SetCurSel(0);

	return TRUE;  
}


Bool COptRender::SaveOptions()
{
	char str[50];
	WinGPAC *gpac = GetApp();

	IF_SetKey(gpac->m_config, "Rendering", "FastRender", m_FastRender.GetCheck() ? "yes" : "no");
	IF_SetKey(gpac->m_config, "Rendering", "ForceSceneSize", m_ForceSize.GetCheck() ? "yes" : "no");

	s32 sel = m_BIFSRate.GetCurSel();
	IF_SetKey(gpac->m_config, "Rendering", "FrameRate", BIFSRates[sel]);

	sel = m_AntiAlias.GetCurSel();
	IF_SetKey(gpac->m_config, "Rendering", "AntiAlias", (sel==0) ? "None" : ( (sel==1) ? "Text" : "All"));

	sel = m_DrawBounds.GetCurSel();
	IF_SetKey(gpac->m_config, "Rendering", "BoundingVolume", (sel==2) ? "AABB" : (sel==1) ? "Box" : "None");

	m_Graphics.GetWindowText(str, 50);
	IF_SetKey(gpac->m_config, "Rendering", "GraphicsDriver", str);

	char *opt;
	opt = IF_GetKey(gpac->m_config, "Rendering", "RendererName");
	if (!opt || strstr(opt, "2D")) {
		if (!m_Use3DRender.GetCheck()) return 0;
		IF_SetKey(gpac->m_config, "Rendering", "RendererName", "GPAC 3D Renderer");
		return 1;
	}
	if (m_Use3DRender.GetCheck()) return 0;
	IF_SetKey(gpac->m_config, "Rendering", "RendererName", "GPAC 2D Renderer");
	return 1;
}

