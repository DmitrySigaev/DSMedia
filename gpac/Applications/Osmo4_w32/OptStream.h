#if !defined(AFX_OPTSTREAM_H__CB500431_216C_4E77_AB91_C2A08AEDCDF5__INCLUDED_)
#define AFX_OPTSTREAM_H__CB500431_216C_4E77_AB91_C2A08AEDCDF5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptStream.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptStream dialog

class COptStream : public CDialog
{
// Construction
public:
	COptStream(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COptStream)
	enum { IDD = IDD_OPT_STREAM };
	CEdit	m_RebufferLen;
	CButton	m_Rebuffer;
	CEdit	m_Buffer;
	CEdit	m_Timeout;
	CButton	m_Reorder;
	CButton	m_UseRTSP;
	CComboBox	m_Port;
	//}}AFX_DATA


	void SaveOptions();

	void CheckRebuffer();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptStream)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COptStream)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangePort();
	afx_msg void OnRtsp();
	afx_msg void OnRebuffer();
	afx_msg void OnUpdateRebufferLen();
	afx_msg void OnUpdateBuffer();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTSTREAM_H__CB500431_216C_4E77_AB91_C2A08AEDCDF5__INCLUDED_)
