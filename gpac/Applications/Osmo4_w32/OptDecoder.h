#if !defined(AFX_OPTDECODER_H__78873139_7125_41FA_8E7A_4691E77F1E49__INCLUDED_)
#define AFX_OPTDECODER_H__78873139_7125_41FA_8E7A_4691E77F1E49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptDecoder.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// OptDecoder dialog

class OptDecoder : public CDialog
{
// Construction
public:
	OptDecoder(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(OptDecoder)
	enum { IDD = IDD_OPT_DECODER };
	CComboBox	m_Video;
	CComboBox	m_Audio;
	//}}AFX_DATA

	void SaveOptions();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(OptDecoder)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(OptDecoder)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTDECODER_H__78873139_7125_41FA_8E7A_4691E77F1E49__INCLUDED_)
