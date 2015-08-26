#if !defined(AFX_OPTVIDEO_H__95241F99_544A_4FB9_8063_2D2177AC064F__INCLUDED_)
#define AFX_OPTVIDEO_H__95241F99_544A_4FB9_8063_2D2177AC064F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptVideo.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptVideo dialog

class COptVideo : public CDialog
{
// Construction
public:
	COptVideo(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COptVideo)
	enum { IDD = IDD_OPT_VIDEO };
	CButton	m_SwitchRes;
	CComboBox	m_Videos;
	//}}AFX_DATA


	void SaveOptions();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptVideo)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COptVideo)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTVIDEO_H__95241F99_544A_4FB9_8063_2D2177AC064F__INCLUDED_)
