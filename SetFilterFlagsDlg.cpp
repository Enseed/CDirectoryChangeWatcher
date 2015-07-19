// SetFilterFlagsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DirWatcher.h"
#include "SetFilterFlagsDlg.h"
#include "DirectoryChanges.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetFilterFlagsDlg dialog


CSetFilterFlagsDlg::CSetFilterFlagsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetFilterFlagsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetFilterFlagsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_dwFilterFlags = 0UL;
}


void CSetFilterFlagsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetFilterFlagsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetFilterFlagsDlg, CDialog)
	//{{AFX_MSG_MAP(CSetFilterFlagsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetFilterFlagsDlg message handlers

BOOL CSetFilterFlagsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	//
	//	Include/Exclude filter options
	//
	if( m_dwFilterFlags & CDirectoryChangeWatcher::FILTERS_DONT_USE_FILTERS )
		CheckDlgButton(IDC_RADIO_DONT_USE_FILTERS, BST_CHECKED);
	else
	if( m_dwFilterFlags & CDirectoryChangeWatcher::FILTERS_CHECK_FULL_PATH )
		CheckDlgButton(IDC_RADIO_CHECK_FULL_PATH, BST_CHECKED);
	else
	if( m_dwFilterFlags & CDirectoryChangeWatcher::FILTERS_CHECK_PARTIAL_PATH)
		CheckDlgButton(IDC_RADIO_CHECK_PARTIAL_PATH, BST_CHECKED);
	else{
		//the default:
		CheckDlgButton(IDC_RADIO_CHECK_FILENAME_ONLY, BST_CHECKED);
	}

	//
	//	Options for CDirectoryChangeHander::On_FilterNotification
	//	and On_WatchStarted/On_WatchStarted
	//
	if( m_dwFilterFlags & CDirectoryChangeWatcher::FILTERS_DONT_USE_HANDLER_FILTER)
		CheckDlgButton(IDC_RADIO_DONT_TEST_HANDLER, BST_CHECKED);
	else
	if( m_dwFilterFlags & CDirectoryChangeWatcher::FILTERS_TEST_HANDLER_FIRST)
		CheckDlgButton(IDC_RADIO_TEST_HANDLER_BEFORE, BST_CHECKED);
	else
	{
		//the default
		CheckDlgButton(IDC_RADIO_TEST_HANDLER_AFTER, BST_CHECKED);
	}

	if( !(m_dwFilterFlags & CDirectoryChangeWatcher::FILTERS_NO_WATCHSTART_NOTIFICATION))
		CheckDlgButton(IDC_CHECK_WATCHSTARTED, BST_CHECKED);
	if( !(m_dwFilterFlags & CDirectoryChangeWatcher::FILTERS_NO_WATCHSTOP_NOTIFICATION))
		CheckDlgButton(IDC_CHECK_WATCHSTOPPED, BST_CHECKED);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSetFilterFlagsDlg::OnOK() 
{
	m_dwFilterFlags = 0UL;
	if( IsDlgButtonChecked(IDC_RADIO_DONT_USE_FILTERS) == BST_CHECKED)	
		m_dwFilterFlags = CDirectoryChangeWatcher::FILTERS_DONT_USE_FILTERS;
	else
	if( IsDlgButtonChecked(IDC_RADIO_CHECK_FULL_PATH) == BST_CHECKED)
		m_dwFilterFlags = CDirectoryChangeWatcher::FILTERS_CHECK_FULL_PATH;
	else
	if( IsDlgButtonChecked(IDC_RADIO_CHECK_PARTIAL_PATH) == BST_CHECKED)
		m_dwFilterFlags = CDirectoryChangeWatcher::FILTERS_CHECK_PARTIAL_PATH;
	else
	{
		m_dwFilterFlags = CDirectoryChangeWatcher::FILTERS_CHECK_FILE_NAME_ONLY;
	}

	if( IsDlgButtonChecked(IDC_RADIO_DONT_TEST_HANDLER) == BST_CHECKED)
		m_dwFilterFlags |= CDirectoryChangeWatcher::FILTERS_DONT_USE_HANDLER_FILTER;
	else
	if( IsDlgButtonChecked(IDC_RADIO_TEST_HANDLER_BEFORE) == BST_CHECKED)
		m_dwFilterFlags |= CDirectoryChangeWatcher::FILTERS_TEST_HANDLER_FIRST;
	else
	{
		//else nothing... there's no flag for test after... it's the default behavior
	}

	if( IsDlgButtonChecked(IDC_CHECK_WATCHSTARTED) != BST_CHECKED )
		m_dwFilterFlags |= CDirectoryChangeWatcher::FILTERS_NO_WATCHSTART_NOTIFICATION;
	if( IsDlgButtonChecked(IDC_CHECK_WATCHSTOPPED) != BST_CHECKED)
		m_dwFilterFlags |= CDirectoryChangeWatcher::FILTERS_NO_WATCHSTOP_NOTIFICATION;


	CDialog::OnOK();
}
