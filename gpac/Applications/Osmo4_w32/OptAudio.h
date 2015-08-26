#if !defined(AFX_OPTAUDIO_H__ECBF9E6C_541E_4AF6_BA63_6B946AC803CB__INCLUDED_)
#define AFX_OPTAUDIO_H__ECBF9E6C_541E_4AF6_BA63_6B946AC803CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptAudio.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptAudio dialog

class COptAudio : public CDialog
{
// Construction
public:
	COptAudio(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COptAudio)
	enum { IDD = IDD_OPT_AUDIO };
	CButton	m_Notifs;
	CComboBox	m_DriverList;
	CButton	m_AudioResync;
	CEdit	m_AudioFPS;
	CSpinButtonCtrl	m_SpinFPS;
	CButton	m_ForceConfig;
	CSpinButtonCtrl	m_AudioSpin;
	CEdit	m_AudioEdit;
	//}}AFX_DATA

	void SaveOptions();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptAudio)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COptAudio)
	virtual BOOL OnInitDialog();
	afx_msg void OnForceAudio();
	afx_msg void OnSelchangeDriverList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTAUDIO_H__ECBF9E6C_541E_4AF6_BA63_6B946AC803CB__INCLUDED_)
