#if !defined(AFX_OPTHTTP_H__A739F4B0_AFA9_4620_9D0A_E5350FF2DCAA__INCLUDED_)
#define AFX_OPTHTTP_H__A739F4B0_AFA9_4620_9D0A_E5350FF2DCAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptHTTP.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptHTTP dialog

class COptHTTP : public CDialog
{
// Construction
public:
	COptHTTP(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COptHTTP)
	enum { IDD = IDD_OPT_HTTP };
	CButton	m_RestartFile;
	CButton	m_CleanCache;
	CButton	m_CacheDir;
	//}}AFX_DATA


	void SaveOptions();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptHTTP)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COptHTTP)
	afx_msg void OnBrowseCache();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTHTTP_H__A739F4B0_AFA9_4620_9D0A_E5350FF2DCAA__INCLUDED_)
