// DirWatcherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DirWatcher.h"
#include "DirWatcherDlg.h"
#include "FolderDialog.h"
#include "TestFilterPatternDlg.h"
#include "SetFilterFlagsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//helpers
CString GetLastErrorMessageString(DWORD dwLastError )
{
	LPVOID lpMsgBuf = NULL;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
				  dwLastError, 
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
							   (LPTSTR) &lpMsgBuf, 0, NULL );

	CString str = (LPCTSTR)lpMsgBuf;

	if( lpMsgBuf != NULL )
		LocalFree(lpMsgBuf);	

	return str;
}
/////////////////////////////////////////////////////////////////////////////
// CDirWatcherDlg dialog

CDirWatcherDlg::CDirWatcherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDirWatcherDlg::IDD, pParent)
	,m_DirWatcher( true )
	,m_DirChangeHandler( m_lstChanges )
{
	//{{AFX_DATA_INIT(CDirWatcherDlg)
	m_strDirectoryToMonitor = _T("");
	m_strDirectoryToMonitor2 = _T("");
	m_strExcludeFilter1 = _T("");
	m_strExcludeFilter2 = _T("");
	m_strIncludeFilter1 = _T("");
	m_strIncludeFilter2 = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDirWatcherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDirWatcherDlg)
	DDX_Control(pDX, IDC_LIST_CHANGES2, m_lstChanges2);
	DDX_Control(pDX, IDC_LIST_CHANGES, m_lstChanges);
	DDX_Text(pDX, IDC_EDIT_DIR_TO_MONITOR, m_strDirectoryToMonitor);
	DDV_MaxChars(pDX, m_strDirectoryToMonitor, 260);
	DDX_Text(pDX, IDC_EDIT_DIR_TO_MONITOR2, m_strDirectoryToMonitor2);
	DDV_MaxChars(pDX, m_strDirectoryToMonitor2, 260);
	DDX_Text(pDX, IDC_EDIT_EXCLUDE_FILTER1, m_strExcludeFilter1);
	DDX_Text(pDX, IDC_EDIT_EXCLUDE_FILTER2, m_strExcludeFilter2);
	DDX_Text(pDX, IDC_EDIT_INCLUDE_FILTER1, m_strIncludeFilter1);
	DDX_Text(pDX, IDC_EDIT_INCLUDE_FILTER2, m_strIncludeFilter2);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDirWatcherDlg, CDialog)
	//{{AFX_MSG_MAP(CDirWatcherDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_MONITOR, OnBtnMonitor)
	ON_BN_CLICKED(IDC_BTN_MONITOR2, OnBtnMonitor2)
	ON_BN_CLICKED(IDC_BTN_CLEAR1, OnBtnClear1)
	ON_BN_CLICKED(IDC_BTN_CLEAR2, OnBtnClear2)
	ON_BN_CLICKED(IDC_BTN_BROWSE1, OnBtnBrowse1)
	ON_BN_CLICKED(IDC_BTN_BROWSE2, OnBtnBrowse2)
	ON_BN_CLICKED(IDC_BTN_TEST_PATTERNS, OnBtnTestPatterns)
	ON_BN_CLICKED(IDC_BTN_SET_FILTER_FLAGS, OnBtnSetFilterFlags)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDirWatcherDlg message handlers

//#include <shlwapi.h>
//#pragma comment( lib, "shlwapi.lib")

BOOL CDirWatcherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	//check all the check boxes (I went into resource.h to make sure the values were in order)
	for(UINT i = IDC_CHECK_FILE_NAME1; i < IDC_CHECK_SECURITY2 + 1; ++i)
	{
		CheckDlgButton(i, BST_CHECKED);
	}

	//
	//	Set the default filter options:
	//
	CheckDlgButton(IDC_RADIO_CHECK_FILENAME_ONLY, BST_CHECKED);
	CheckDlgButton(IDC_RADIO_TEST_HANDLER_AFTER,  BST_CHECKED);

//	TRACE(_T("PathMatchSpec: %s\n"), PathMatchSpec(_T("C:\\Program Files\\FolderName\\FileName.txt"), _T("*Fol*\\*.*"))? _T("TRUE") : _T("FALSE"));
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDirWatcherDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDirWatcherDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CDirWatcherDlg::OnBtnMonitor() 
{
	// TODO: Add your control notification handler code here

	DWORD dwChangeFilter = GetChangeFlags1();
	BOOL bWatchSubDir = (BOOL)(IsDlgButtonChecked(IDC_CHECK_SUBDIR1) == BST_CHECKED);
	
	if( dwChangeFilter == 0 ){
		MessageBox(_T("You must select one or more change filters in order to monitor a directory"));
		return;
	}
	if( m_DirWatcher.IsWatchingDirectory( m_strDirectoryToMonitor) )
		m_DirWatcher.UnwatchDirectory( m_strDirectoryToMonitor );

	GetDlgItemText(IDC_EDIT_DIR_TO_MONITOR, m_strDirectoryToMonitor);
	if( m_DirWatcher.GetFilterFlags() & CDirectoryChangeWatcher::FILTERS_DONT_USE_FILTERS )
	{
		m_strIncludeFilter1.Empty();
		m_strExcludeFilter1.Empty();
	}
	else
	{
		GetDlgItemText(IDC_EDIT_INCLUDE_FILTER1, m_strIncludeFilter1);
		GetDlgItemText(IDC_EDIT_EXCLUDE_FILTER1, m_strExcludeFilter1);

		if( m_strIncludeFilter1 == _T("*.*") )
		{
			MessageBox(_T("For performance reasons, don't use *.* as the include filter. An empty string means the same thing."));
			SetDlgItemText(IDC_EDIT_INCLUDE_FILTER1, _T(""));
			m_strIncludeFilter1.Empty();
		}
	}
	

	DWORD dwWatch = 0;
	if( ERROR_SUCCESS != (dwWatch = m_DirWatcher.WatchDirectory(m_strDirectoryToMonitor, 
													dwChangeFilter,
													&m_DirChangeHandler,
													bWatchSubDir,
													m_strIncludeFilter1,
													m_strExcludeFilter1)) )
	{
		MessageBox(_T("Failed to start watch:\n") + GetLastErrorMessageString( dwWatch ) );
	}

	
}

typedef struct {
	UINT nCtrlId;
	UINT nFlag;
} FLAG_INFO;

DWORD CDirWatcherDlg::GetChangeFlags1()
{

	DWORD dwChangeNotifyFlags = 0UL;
	FLAG_INFO arFlagInfo[]={
		{IDC_CHECK_FILE_NAME1,		FILE_NOTIFY_CHANGE_FILE_NAME},
		{IDC_CHECK_DIR_NAME1,		FILE_NOTIFY_CHANGE_DIR_NAME},
		{IDC_CHECK_LAST_WRITE1,		FILE_NOTIFY_CHANGE_LAST_WRITE},
		{IDC_CHECK_LAST_ACCESS1,	FILE_NOTIFY_CHANGE_LAST_ACCESS},
		{IDC_CHECK_SIZE1,			FILE_NOTIFY_CHANGE_SIZE},
		{IDC_CHECK_ATTRIBUTES1,		FILE_NOTIFY_CHANGE_ATTRIBUTES},
		{IDC_CHECK_LAST_WRITE1,		FILE_NOTIFY_CHANGE_LAST_WRITE},
		{IDC_CHECK_LAST_ACCESS1,	FILE_NOTIFY_CHANGE_LAST_ACCESS},
		{IDC_CHECK_CREATION1,		FILE_NOTIFY_CHANGE_CREATION},
		{IDC_CHECK_SECURITY1,		FILE_NOTIFY_CHANGE_SECURITY}
	};

	for(int i = 0; i < sizeof(arFlagInfo)/sizeof(arFlagInfo[0]); ++i)
	{
		dwChangeNotifyFlags |= (IsDlgButtonChecked(arFlagInfo[i].nCtrlId) == BST_CHECKED)? arFlagInfo[i].nFlag : 0;
	}

	return dwChangeNotifyFlags;
}

DWORD CDirWatcherDlg::GetChangeFlags2()
{
	DWORD dwChangeNotifyFlags = 0UL;
	FLAG_INFO arFlagInfo[]={
		{IDC_CHECK_FILE_NAME2,		FILE_NOTIFY_CHANGE_FILE_NAME},
		{IDC_CHECK_DIR_NAME2,		FILE_NOTIFY_CHANGE_DIR_NAME},
		{IDC_CHECK_LAST_WRITE2,		FILE_NOTIFY_CHANGE_LAST_WRITE},
		{IDC_CHECK_LAST_ACCESS2,	FILE_NOTIFY_CHANGE_LAST_ACCESS},
		{IDC_CHECK_SIZE2,			FILE_NOTIFY_CHANGE_SIZE},
		{IDC_CHECK_ATTRIBUTES2,		FILE_NOTIFY_CHANGE_ATTRIBUTES},
		{IDC_CHECK_LAST_WRITE2,		FILE_NOTIFY_CHANGE_LAST_WRITE},
		{IDC_CHECK_LAST_ACCESS2,	FILE_NOTIFY_CHANGE_LAST_ACCESS},
		{IDC_CHECK_CREATION2,		FILE_NOTIFY_CHANGE_CREATION},
		{IDC_CHECK_SECURITY2,		FILE_NOTIFY_CHANGE_SECURITY}
	};

	for(int i = 0; i < sizeof(arFlagInfo)/sizeof(arFlagInfo[0]); ++i)
	{
		dwChangeNotifyFlags |= (IsDlgButtonChecked(arFlagInfo[i].nCtrlId) == BST_CHECKED)? arFlagInfo[i].nFlag : 0;
	}

	return dwChangeNotifyFlags;	
}
void CDirWatcherDlg::OnBtnMonitor2() 
{
	DWORD dwChangeFilter = GetChangeFlags1();
	BOOL bWatchSubDir = (BOOL)(IsDlgButtonChecked(IDC_CHECK_SUBDIR1) == BST_CHECKED);
	
	if( dwChangeFilter == 0 ){
		MessageBox(_T("You must select one or more change filters in order to monitor a directory"));
		return;
	}
	if( m_DirWatcher.IsWatchingDirectory( m_strDirectoryToMonitor2) )
		m_DirWatcher.UnwatchDirectory( m_strDirectoryToMonitor2 );

	GetDlgItemText(IDC_EDIT_DIR_TO_MONITOR2, m_strDirectoryToMonitor2);
	if( m_DirWatcher.GetFilterFlags() & CDirectoryChangeWatcher::FILTERS_DONT_USE_FILTERS )
	{
		// not using filters...
		m_strIncludeFilter2.Empty();
		m_strExcludeFilter2.Empty();
	}
	else
	{
		GetDlgItemText(IDC_EDIT_INCLUDE_FILTER2, m_strIncludeFilter2);
		GetDlgItemText(IDC_EDIT_EXCLUDE_FILTER2, m_strExcludeFilter2);

		if( m_strIncludeFilter2 == _T("*.*") )
		{
			MessageBox(_T("For performance reasons, don't use *.* as the include filter. An empty string means the same thing."));
			SetDlgItemText(IDC_EDIT_INCLUDE_FILTER2, _T(""));
			m_strIncludeFilter2.Empty();
		}
	}


	//
	//	This demonstrates that the CDirectoryChangeHandler objects are 
	//	reference counted(reference count is incremented/decremented by CDirecotryChangeWatcher)
	//	and that they will be automatically deleted
	//	when the directory is unwatched.
	//

	CDirectoryChangeHandler_ListBox * pHandler = new CDirectoryChangeHandler_ListBox( m_lstChanges2 );

	DWORD dwWatch;
	if( ERROR_SUCCESS != (dwWatch = m_DirWatcher.WatchDirectory(m_strDirectoryToMonitor2, 
													dwChangeFilter,
													pHandler,
													bWatchSubDir,
													m_strIncludeFilter2,
													m_strExcludeFilter2)) )
	{
		MessageBox(_T("Failed to start watch:\n") + GetLastErrorMessageString( dwWatch ));
	}

	if( pHandler )
		pHandler->Release();
	
}

void CDirWatcherDlg::OnBtnClear1() 
{
	m_lstChanges.ResetContent();

}

void CDirWatcherDlg::OnBtnClear2() 
{
	
	m_lstChanges2.ResetContent();
}

void CDirWatcherDlg::OnBtnBrowse1() 
{
	GetDlgItemText(IDC_EDIT_DIR_TO_MONITOR, m_strDirectoryToMonitor);

	CFolderDialog dlg(_T("Select the Directory you wish to monitor..."), m_strDirectoryToMonitor, this);

	if( dlg.DoModal() == IDOK )
	{
		SetDlgItemText(IDC_EDIT_DIR_TO_MONITOR, dlg.GetFolderPath());
	}
	
}

void CDirWatcherDlg::OnBtnBrowse2() 
{
	GetDlgItemText(IDC_EDIT_DIR_TO_MONITOR2, m_strDirectoryToMonitor2);
	
	CFolderDialog dlg(_T("Select the Directory you wish to monitor..."), m_strDirectoryToMonitor2, this);

	if( dlg.DoModal() == IDOK )
	{
		SetDlgItemText(IDC_EDIT_DIR_TO_MONITOR2, dlg.GetFolderPath());
	}
}

void CDirWatcherDlg::OnCancel() 
{

	
	CDialog::OnCancel();
}

void CDirWatcherDlg::OnOK() 
{
	
	//CDialog::OnOK();//do nothing...we don't want the dlg to close when the enter key is pressed....
}

void CDirWatcherDlg::EnableFilterEditCtrls(BOOL bEnable)
{
	UINT nIDs[] = {
		IDC_EDIT_INCLUDE_FILTER1, IDC_EDIT_INCLUDE_FILTER2,
		IDC_EDIT_EXCLUDE_FILTER1, IDC_EDIT_EXCLUDE_FILTER2
	};
	for(UINT i(0); i < sizeof(nIDs) / sizeof(nIDs[0]); ++i)
	{
		::EnableWindow(GetDlgItem(nIDs[i])->GetSafeHwnd(), bEnable);
	}	
}

void CDirWatcherDlg::OnBtnTestPatterns() 
{

	CTestFilterPatternDlg dlg(this);
	dlg.DoModal();
}

void CDirWatcherDlg::OnBtnSetFilterFlags() 
{
	CSetFilterFlagsDlg dlg(this);

	dlg.m_dwFilterFlags = m_DirWatcher.GetFilterFlags();

	if( dlg.DoModal() == IDOK )
	{
		//
		//	The next time a watch is started, these filter flags
		//	will be in effect.
		//
		m_DirWatcher.SetFilterFlags( dlg.m_dwFilterFlags );

		if( dlg.m_dwFilterFlags & CDirectoryChangeWatcher::FILTERS_DONT_USE_FILTERS )
			EnableFilterEditCtrls( FALSE );
		else
			EnableFilterEditCtrls( TRUE );
	}
	
}
