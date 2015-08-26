#if !defined(AFX_OPTRENDER_H__1CB8EF7C_59EA_49A9_9AE5_76D2F220D29E__INCLUDED_)
#define AFX_OPTRENDER_H__1CB8EF7C_59EA_49A9_9AE5_76D2F220D29E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptRender.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptRender dialog

class COptRender : public CDialog
{
// Construction
public:
	COptRender(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COptRender)
	enum { IDD = IDD_OPT_RENDER };
	CComboBox	m_DrawBounds;
	CComboBox	m_Graphics;
	CButton	m_Use3DRender;
	CComboBox	m_AntiAlias;
	CButton	m_ForceSize;
	CButton	m_FastRender;
	CComboBox	m_BIFSRate;
	//}}AFX_DATA


	Bool SaveOptions();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptRender)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COptRender)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTRENDER_H__1CB8EF7C_59EA_49A9_9AE5_76D2F220D29E__INCLUDED_)
