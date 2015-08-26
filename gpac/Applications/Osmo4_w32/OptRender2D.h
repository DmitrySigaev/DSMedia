#if !defined(AFX_OPTRENDER2D_H__2B769B41_D78E_4A53_918F_B6FF003B066C__INCLUDED_)
#define AFX_OPTRENDER2D_H__2B769B41_D78E_4A53_918F_B6FF003B066C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptRender2D.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptRender2D dialog

class COptRender2D : public CDialog
{
// Construction
public:
	COptRender2D(CWnd* pParent = NULL);   // standard constructor

	void SaveOptions();
	void SetYUV();

// Dialog Data
	//{{AFX_DATA(COptRender2D)
	enum { IDD = IDD_OPT_RENDER2D };
	CStatic	m_YUVFormat;
	CButton	m_NoYUV;
	CButton	m_Scalable;
	CButton	m_DirectRender;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptRender2D)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COptRender2D)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTRENDER2D_H__2B769B41_D78E_4A53_918F_B6FF003B066C__INCLUDED_)
