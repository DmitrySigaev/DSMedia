#if !defined(AFX_OPTSYSTEMS_H__E765697B_EC9B_4DD7_8B0D_D94501AEEB2E__INCLUDED_)
#define AFX_OPTSYSTEMS_H__E765697B_EC9B_4DD7_8B0D_D94501AEEB2E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptSystems.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptSystems dialog

class COptSystems : public CDialog
{
// Construction
public:
	COptSystems(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COptSystems)
	enum { IDD = IDD_OPT_SYSTEMS };
	CButton	m_ForceDuration;
	CComboBox	m_Threading;
	CButton	m_BifsAlwaysDrawn;
	CComboBox	m_Lang;
	//}}AFX_DATA

	void SaveOptions();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptSystems)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COptSystems)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTSYSTEMS_H__E765697B_EC9B_4DD7_8B0D_D94501AEEB2E__INCLUDED_)
