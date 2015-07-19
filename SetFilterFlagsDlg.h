#if !defined(AFX_SETFILTERFLAGSDLG_H__7A31B257_E112_4546_868C_3B5907100164__INCLUDED_)
#define AFX_SETFILTERFLAGSDLG_H__7A31B257_E112_4546_868C_3B5907100164__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetFilterFlagsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSetFilterFlagsDlg dialog

class CSetFilterFlagsDlg : public CDialog
{
// Construction
public:
	DWORD m_dwFilterFlags;
	CSetFilterFlagsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetFilterFlagsDlg)
	enum { IDD = IDD_SET_FILTER_FLAGS_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetFilterFlagsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetFilterFlagsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETFILTERFLAGSDLG_H__7A31B257_E112_4546_868C_3B5907100164__INCLUDED_)
