// OptRender3D.cpp : implementation file
//

#include "stdafx.h"
#include "osmo4.h"
#include "OptRender3D.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptRender3D dialog


COptRender3D::COptRender3D(CWnd* pParent /*=NULL*/)
	: CDialog(COptRender3D::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptRender3D)
	m_Wireframe = -1;
	//}}AFX_DATA_INIT
}


void COptRender3D::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptRender3D)
	DDX_Control(pDX, IDC_NO_BACKCULL, m_NoBackCull);
	DDX_Control(pDX, IDC_BITMAP_USE_PIXEL, m_BitmapPixels);
	DDX_Control(pDX, IDC_DISABLE_TX_RECT, m_DisableTXRect);
	DDX_Control(pDX, IDC_RASTER_OUTLINE, m_RasterOutlines);
	DDX_Control(pDX, IDC_EMUL_POW2, m_EmulPow2);
	DDX_Control(pDX, IDC_DISABLE_POLY_AA, m_PolyAA);
	DDX_Radio(pDX, IDC_WIRE_NONE, m_Wireframe);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptRender3D, CDialog)
	//{{AFX_MSG_MAP(COptRender3D)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptRender3D message handlers


BOOL COptRender3D::OnInitDialog() 
{
	CDialog::OnInitDialog();
	

	WinGPAC *gpac = GetApp();
	char *sOpt;

	sOpt = IF_GetKey(gpac->m_config, "Render3D", "RasterOutlines");
	m_RasterOutlines.SetCheck((sOpt && !stricmp(sOpt, "yes")) ? 1 : 0);
	sOpt = IF_GetKey(gpac->m_config, "Render3D", "EmulatePOW2");
	m_EmulPow2.SetCheck((sOpt && !stricmp(sOpt, "yes")) ? 1 : 0);
	sOpt = IF_GetKey(gpac->m_config, "Render3D", "PolygonAA");
	m_PolyAA.SetCheck((sOpt && !stricmp(sOpt, "yes")) ? 1 : 0);
	sOpt = IF_GetKey(gpac->m_config, "Render3D", "DisableBackFaceCulling");
	m_NoBackCull.SetCheck((sOpt && !stricmp(sOpt, "yes")) ? 1 : 0);

	sOpt = IF_GetKey(gpac->m_config, "Render3D", "Wireframe");
	if (sOpt && !stricmp(sOpt, "WireOnly")) m_Wireframe = 1;
	else if (sOpt && !stricmp(sOpt, "WireOnSolid")) m_Wireframe = 2;
	else m_Wireframe = 0;
	UpdateData(FALSE);

	sOpt = IF_GetKey(gpac->m_config, "Render3D", "BitmapCopyPixels");
	m_BitmapPixels.SetCheck((sOpt && !stricmp(sOpt, "yes")) ? 1 : 0);
	
	sOpt = IF_GetKey(gpac->m_config, "Render3D", "DisableRectExt");
	m_DisableTXRect.SetCheck((sOpt && !stricmp(sOpt, "yes")) ? 1 : 0);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptRender3D::SaveOptions()
{
	WinGPAC *gpac = GetApp();

	IF_SetKey(gpac->m_config, "Render3D", "RasterOutlines", m_RasterOutlines.GetCheck() ? "yes" : "no");
	IF_SetKey(gpac->m_config, "Render3D", "EmulatePOW2", m_EmulPow2.GetCheck() ? "yes" : "no");
	IF_SetKey(gpac->m_config, "Render3D", "PolygonAA", m_PolyAA.GetCheck() ? "yes" : "no");
	IF_SetKey(gpac->m_config, "Render3D", "DisableBackFaceCulling", m_NoBackCull.GetCheck() ? "yes" : "no");

	UpdateData();
	switch (m_Wireframe) {
	case 2:
		IF_SetKey(gpac->m_config, "Render3D", "Wireframe", "WireOnSolid");
		break;
	case 1:
		IF_SetKey(gpac->m_config, "Render3D", "Wireframe", "WireOnly");
		break;
	case 0:
		IF_SetKey(gpac->m_config, "Render3D", "Wireframe", "WireNone");
		break;
	}
	IF_SetKey(gpac->m_config, "Render3D", "DisableRectExt", m_DisableTXRect.GetCheck() ? "yes" : "no");
	IF_SetKey(gpac->m_config, "Render3D", "BitmapCopyPixels", m_BitmapPixels.GetCheck() ? "yes" : "no");
}
