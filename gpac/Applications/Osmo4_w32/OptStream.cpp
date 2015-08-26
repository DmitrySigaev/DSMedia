// OptStream.cpp : implementation file
//

#include "stdafx.h"
#include "Osmo4.h"
#include "OptStream.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptStream dialog


COptStream::COptStream(CWnd* pParent /*=NULL*/)
	: CDialog(COptStream::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptStream)
	//}}AFX_DATA_INIT
}


void COptStream::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptStream)
	DDX_Control(pDX, IDC_REBUFFER_LEN, m_RebufferLen);
	DDX_Control(pDX, IDC_REBUFFER, m_Rebuffer);
	DDX_Control(pDX, IDC_BUFFER, m_Buffer);
	DDX_Control(pDX, IDC_TIMEOUT, m_Timeout);
	DDX_Control(pDX, IDC_REORDER, m_Reorder);
	DDX_Control(pDX, IDC_RTSP, m_UseRTSP);
	DDX_Control(pDX, IDC_PORT, m_Port);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptStream, CDialog)
	//{{AFX_MSG_MAP(COptStream)
	ON_CBN_SELCHANGE(IDC_PORT, OnSelchangePort)
	ON_BN_CLICKED(IDC_RTSP, OnRtsp)
	ON_BN_CLICKED(IDC_REBUFFER, OnRebuffer)
	ON_EN_UPDATE(IDC_REBUFFER_LEN, OnUpdateRebufferLen)
	ON_EN_UPDATE(IDC_BUFFER, OnUpdateBuffer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptStream message handlers

BOOL COptStream::OnInitDialog() 
{
	CDialog::OnInitDialog();

	WinGPAC *gpac = GetApp();
	char *sOpt;

	while (m_Port.GetCount()) m_Port.DeleteString(0);
	m_Port.AddString("554 (RTSP standard)");
	m_Port.AddString("7070 (RTSP ext)");
	m_Port.AddString("80 (RTSP / HTTP tunnel)");
	m_Port.AddString("8080 (RTSP / HTTP tunnel)");

	sOpt = IF_GetKey(gpac->m_config, "Streaming", "DefaultPort");
	u32 port = 554;
	Bool force_rtsp = 0;;
	if (sOpt) port = atoi(sOpt);
	switch (port) {
	case 8080:
		m_Port.SetCurSel(3);
		force_rtsp = 1;
		break;
	case 80:
		m_Port.SetCurSel(2);
		force_rtsp = 1;
		break;
	case 7070:
		m_Port.SetCurSel(1);
		break;
	default:
		m_Port.SetCurSel(0);
		break;
	}

	Bool use_rtsp = 0;
	sOpt = IF_GetKey(gpac->m_config, "Streaming", "RTPoverRTSP");
	if (sOpt && !stricmp(sOpt, "yes")) use_rtsp = 1;

	if (force_rtsp) {
		m_UseRTSP.SetCheck(1);
		m_UseRTSP.EnableWindow(0);
		m_Reorder.SetCheck(0);
		m_Reorder.EnableWindow(0);
	} else {
		m_UseRTSP.SetCheck(use_rtsp);
		m_UseRTSP.EnableWindow(1);
		m_Reorder.EnableWindow(1);
		sOpt = IF_GetKey(gpac->m_config, "Streaming", "ReorderSize");
		if (sOpt && !stricmp(sOpt, "0")) {
			m_Reorder.SetCheck(0);
		} else {
			m_Reorder.SetCheck(1);
		}
	}

	sOpt = IF_GetKey(gpac->m_config, "Streaming", "RTSPTimeout");
	if (sOpt) {
		m_Timeout.SetWindowText(sOpt);
	} else {
		m_Timeout.SetWindowText("30000");
	}

	sOpt = IF_GetKey(gpac->m_config, "Network", "BufferLength");
	if (sOpt) {
		m_Buffer.SetWindowText(sOpt);
	} else {
		m_Buffer.SetWindowText("3000");
	}

	sOpt = IF_GetKey(gpac->m_config, "Network", "RebufferLength");
	u32 buf_len = 0;
	if (sOpt) buf_len = atoi(sOpt);
	if (buf_len) {
		m_RebufferLen.SetWindowText(sOpt);
		m_Rebuffer.SetCheck(1);
		m_RebufferLen.EnableWindow(1);
	} else {
		m_RebufferLen.SetWindowText("0");
		m_Rebuffer.SetCheck(0);
		m_RebufferLen.EnableWindow(0);
	}

	return TRUE;  
}


void COptStream::OnSelchangePort() 
{
	s32 sel = m_Port.GetCurSel();
	switch (sel) {
	case 3:
	case 2:
		m_UseRTSP.SetCheck(1);
		m_UseRTSP.EnableWindow(0);
		m_Reorder.SetCheck(0);
		m_Reorder.EnableWindow(0);
		break;
	case 1:
	default:
		m_UseRTSP.SetCheck(0);
		m_UseRTSP.EnableWindow(1);
		m_Reorder.SetCheck(1);
		m_Reorder.EnableWindow(1);
		break;
	}
}

void COptStream::OnRtsp() 
{
	if (m_UseRTSP.GetCheck()) {
		m_Reorder.SetCheck(0);
		m_Reorder.EnableWindow(0);
	} else {
		m_Reorder.SetCheck(1);
		m_Reorder.EnableWindow(1);
	}
	
}

void COptStream::CheckRebuffer()
{
	char str[50];
	s32 buf, rebuf;
	m_Buffer.GetWindowText(str, 50);
	buf = atoi(str);
	m_RebufferLen.GetWindowText(str, 50);
	rebuf = atoi(str);
	if (rebuf*2 > buf) {
		rebuf = buf/2;
		sprintf(str, "%d", rebuf);
		m_RebufferLen.SetWindowText(str);
	}
}

void COptStream::OnRebuffer() 
{
	if (!m_Rebuffer.GetCheck()) {
		m_RebufferLen.EnableWindow(0);
	} else {
		m_RebufferLen.EnableWindow(1);
		CheckRebuffer();
	}
}

void COptStream::OnUpdateRebufferLen() 
{
	CheckRebuffer();
}

void COptStream::OnUpdateBuffer() 
{
	CheckRebuffer();	
}

void COptStream::SaveOptions()
{
	WinGPAC *gpac = GetApp();
	Bool force_rtsp = 0;
	s32 sel = m_Port.GetCurSel();
	switch (sel) {
	case 3:
		IF_SetKey(gpac->m_config, "Streaming", "DefaultPort", "8080");
		force_rtsp = 1;
		break;
	case 2:
		IF_SetKey(gpac->m_config, "Streaming", "DefaultPort", "80");
		force_rtsp = 1;
		break;
	case 1:
		IF_SetKey(gpac->m_config, "Streaming", "DefaultPort", "7070");
		break;
	default:
		IF_SetKey(gpac->m_config, "Streaming", "DefaultPort", "554");
		break;
	}

	if (force_rtsp) {
		IF_SetKey(gpac->m_config, "Streaming", "RTPoverRTSP", "yes");
	} else {
		IF_SetKey(gpac->m_config, "Streaming", "RTPoverRTSP", m_UseRTSP.GetCheck() ? "yes" : "no");
		if (!m_UseRTSP.GetCheck()) IF_SetKey(gpac->m_config, "Streaming", "ReorderSize", m_Reorder.GetCheck() ? "30" : "0");
	}

	char str[50];

	m_Timeout.GetWindowText(str, 50);
	IF_SetKey(gpac->m_config, "Streaming", "RTSPTimeout", str);

	m_Buffer.GetWindowText(str, 50);
	IF_SetKey(gpac->m_config, "Network", "BufferLength", str);
	if (m_Rebuffer.GetCheck()) {
		m_RebufferLen.GetWindowText(str, 50);
		IF_SetKey(gpac->m_config, "Network", "RebufferLength", str);
	} else {
		IF_SetKey(gpac->m_config, "Network", "RebufferLength", "0");
	}
}


