#if !defined(AFX_OPTIONS_H__5C839953_58C0_4D9D_89CE_2820C7686C1B__INCLUDED_)
#define AFX_OPTIONS_H__5C839953_58C0_4D9D_89CE_2820C7686C1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Options.h : header file
//


#include "OptGen.h"
#include "OptSystems.h"
#include "OptRender.h"
#include "OptRender2D.h"
#include "OptRender3D.h"
#include "OptAudio.h"
#include "OptVideo.h"
#include "OptHTTP.h"
#include "OptFont.h"
#include "OptStream.h"
#include "OptDecoder.h"


/////////////////////////////////////////////////////////////////////////////
// COptions dialog

class COptions : public CDialog
{
// Construction
public:
	COptions(CWnd* pParent = NULL);   // standard constructor
	BOOL Create(CWnd * pParent)
	{
		return CDialog::Create( COptions::IDD, pParent);
	}

// Dialog Data
	//{{AFX_DATA(COptions)
	enum { IDD = IDD_OPTIONS };
	CComboBox	m_Selector;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	COptGen m_general;
	COptSystems m_systems;
	COptRender m_render;
	COptRender2D m_render2d;
	COptRender3D m_render3d;
	COptAudio m_audio;
	OptDecoder m_decoder;
	COptVideo m_video;
	COptHTTP m_http;
	COptFont m_font;
	COptStream m_stream;

	void HideAll();

	// Generated message map functions
	//{{AFX_MSG(COptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnSaveopt();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeSelect();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONS_H__5C839953_58C0_4D9D_89CE_2820C7686C1B__INCLUDED_)
