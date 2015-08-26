#if !defined(AFX_OPTFONT_H__F05BF781_A1B6_42BB_8190_EF9C89DE3EAF__INCLUDED_)
#define AFX_OPTFONT_H__F05BF781_A1B6_42BB_8190_EF9C89DE3EAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptFont.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptFont dialog

class COptFont : public CDialog
{
// Construction
public:
	COptFont(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COptFont)
	enum { IDD = IDD_OPT_FONT };
	CButton	m_UseTexture;
	CComboBox	m_Fonts;
	CButton	m_BrowseFont;
	//}}AFX_DATA


	void SaveOptions();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptFont)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COptFont)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowseFont();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTFONT_H__F05BF781_A1B6_42BB_8190_EF9C89DE3EAF__INCLUDED_)
