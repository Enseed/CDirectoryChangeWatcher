// TestFilterPatternDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DirWatcher.h"
#include "TestFilterPatternDlg.h"
#include <shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestFilterPatternDlg dialog


CTestFilterPatternDlg::CTestFilterPatternDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestFilterPatternDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestFilterPatternDlg)
	m_strTestPath = _T("");
	m_strTestPattern = _T("");
	//}}AFX_DATA_INIT
}


void CTestFilterPatternDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestFilterPatternDlg)
	DDX_Text(pDX, IDC_EDIT_TEST_PATH, m_strTestPath);
	DDX_Text(pDX, IDC_EDIT_TEST_PATTERN, m_strTestPattern);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTestFilterPatternDlg, CDialog)
	//{{AFX_MSG_MAP(CTestFilterPatternDlg)
	ON_BN_CLICKED(IDC_BTN_TEST, OnBtnTest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestFilterPatternDlg message handlers

void CTestFilterPatternDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CTestFilterPatternDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

void CTestFilterPatternDlg::OnBtnTest() 
{
	if(!UpdateData(TRUE) )
		return;

	CString str;

	if( !(m_strTestPath.IsEmpty() && m_strTestPattern.IsEmpty()) )
	{
			str.Format(_T("%s"), PathMatchSpec(m_strTestPath, m_strTestPattern)? _T("TRUE") : _T("FALSE"));
	}
		
	SetDlgItemText(IDC_STATIC_TEST_RESULTS, str);
	
}
