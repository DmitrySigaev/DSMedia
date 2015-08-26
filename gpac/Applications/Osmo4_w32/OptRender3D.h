#if !defined(AFX_OPTRENDER3D_H__314D0852_A64F_416D_8C53_ECA3079DD8C0__INCLUDED_)
#define AFX_OPTRENDER3D_H__314D0852_A64F_416D_8C53_ECA3079DD8C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptRender3D.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptRender3D dialog

class COptRender3D : public CDialog
{
// Construction
public:
	COptRender3D(CWnd* pParent = NULL);   // standard constructor

	void SaveOptions();

// Dialog Data
	//{{AFX_DATA(COptRender3D)
	enum { IDD = IDD_OPT_RENDER3D };
	CButton	m_NoBackCull;
	CButton	m_BitmapPixels;
	CButton	m_DisableTXRect;
	CButton	m_RasterOutlines;
	CButton	m_EmulPow2;
	CButton	m_PolyAA;
	int		m_Wireframe;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptRender3D)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COptRender3D)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTRENDER3D_H__314D0852_A64F_416D_8C53_ECA3079DD8C0__INCLUDED_)
