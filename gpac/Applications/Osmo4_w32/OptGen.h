#if !defined(AFX_OPTGEN_H__F3DA11AB_B868_402D_8146_788C7914A839__INCLUDED_)
#define AFX_OPTGEN_H__F3DA11AB_B868_402D_8146_788C7914A839__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptGen.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptGen dialog

class COptGen : public CDialog
{
// Construction
public:
	COptGen(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COptGen)
	enum { IDD = IDD_OPT_GEN };
	CButton	m_LookForSubs;
	CButton	m_ViewXMT;
	CButton	m_NoConsole;
	CButton	m_Loop;
	//}}AFX_DATA

	void SaveOptions();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptGen)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COptGen)
	virtual BOOL OnInitDialog();
	afx_msg void OnFileassoc();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTGEN_H__F3DA11AB_B868_402D_8146_788C7914A839__INCLUDED_)
