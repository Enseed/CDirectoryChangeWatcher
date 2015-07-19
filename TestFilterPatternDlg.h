#if !defined(AFX_TESTFILTERPATTERNDLG_H__352FCF19_B6D6_45F4_9470_E4D0D033CF17__INCLUDED_)
#define AFX_TESTFILTERPATTERNDLG_H__352FCF19_B6D6_45F4_9470_E4D0D033CF17__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TestFilterPatternDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTestFilterPatternDlg dialog

class CTestFilterPatternDlg : public CDialog
{
// Construction
public:
	CTestFilterPatternDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTestFilterPatternDlg)
	enum { IDD = IDD_DLG_TEST_PATTERNS };
	CString	m_strTestPath;
	CString	m_strTestPattern;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestFilterPatternDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTestFilterPatternDlg)
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnBtnTest();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTFILTERPATTERNDLG_H__352FCF19_B6D6_45F4_9470_E4D0D033CF17__INCLUDED_)
