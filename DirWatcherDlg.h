// DirWatcherDlg.h : header file
//

#if !defined(AFX_DIRWATCHERDLG_H__ECE3EDC7_6C61_4153_B25B_32A8904A37B1__INCLUDED_)
#define AFX_DIRWATCHERDLG_H__ECE3EDC7_6C61_4153_B25B_32A8904A37B1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CDirWatcherDlg dialog
#include "DirectoryChanges.h"

class CDirectoryChangeHandler_ListBox : public CDirectoryChangeHandler
{
public:
	CDirectoryChangeHandler_ListBox(CListBox & list_box)
		: CDirectoryChangeHandler(),
		m_listBox( list_box ){}
	protected:
	//These functions are called when the directory to watch has had a change made to it
	void On_FileAdded(const CString & strFileName)
	{
		m_listBox.AddString( _T("File Added: ") + strFileName);
	}
	void On_FileRemoved(const CString & strFileName)
	{
		m_listBox.AddString(_T("File Removed: ") + strFileName);
	}
	void On_FileModified(const CString & strFileName)
	{
		m_listBox.AddString(_T("File Modified: ") + strFileName);
	}
	void On_FileNameChanged(const CString & strOldFileName, const CString & strNewFileName)
	{
		m_listBox.AddString(_T("File name changed from: ") + strOldFileName + _T(" to: ") + strNewFileName);
	}

	void On_WatchStarted(DWORD dwError, const CString & strDirectoryName)
	{
		if( dwError == 0 )
			m_listBox.AddString( _T("Directory Watch Started: ") + strDirectoryName);
		else
			m_listBox.AddString( _T("Directory watch failed: ") + strDirectoryName);
	}
	void On_WatchStopped(const CString & strDirectoryName)
	{
		//
		//	NOTE:  Be careful ! On_WatchStopped can be called AFTER this list box has
		//	been destoroyed.  Be carefull whenever interacting w/ windows with this function
		//	because the window may not be valid when this function is executed for the last time.
		//
		//
		if( ::IsWindow( m_listBox.GetSafeHwnd() ) )
			m_listBox.AddString(_T("Stopped watching directory: ") + strDirectoryName);
		else
		{
			TRACE(_T("NOTE: m_listBox is not a valid window! This has probably been called as the result of program shutdown sometime after CDirWatcherDlg::OnBtnMonitor2() was used to watch a direcotry.\n"));
			TRACE(_T("NOTE: Be sure that you are careful in your implementation of this function to watch out for the same condition if you are dealing w/ windows.\n"));
//			MessageBeep(static_cast<UINT>(-1));
//			MessageBeep(static_cast<UINT>(-1));
//			MessageBeep(static_cast<UINT>(-1));
			
			//
			//	See the comments in DirectoryChanges.h for the CDirectoryChangeHandler::On_WatchStopped() function.
			//
			//
		}
	}

	CListBox & m_listBox;

};

class CDirWatcherDlg : public CDialog
{
// Construction
public:
	CDirWatcherDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CDirWatcherDlg)
	enum { IDD = IDD_DIRWATCHER_DIALOG };
	CListBox	m_lstChanges2;
	CListBox	m_lstChanges;
	CString	m_strDirectoryToMonitor;
	CString	m_strDirectoryToMonitor2;
	CString	m_strExcludeFilter1;
	CString	m_strExcludeFilter2;
	CString	m_strIncludeFilter1;
	CString	m_strIncludeFilter2;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDirWatcherDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CDirWatcherDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnMonitor();
	afx_msg void OnBtnMonitor2();
	afx_msg void OnBtnClear1();
	afx_msg void OnBtnClear2();
	afx_msg void OnBtnBrowse1();
	afx_msg void OnBtnBrowse2();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnBtnTestPatterns();
	afx_msg void OnBtnSetFilterFlags();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CDirectoryChangeWatcher			m_DirWatcher;
	CDirectoryChangeHandler_ListBox m_DirChangeHandler;

	DWORD GetChangeFlags1();
	DWORD GetChangeFlags2();


	void EnableFilterEditCtrls(BOOL bEnable);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIRWATCHERDLG_H__ECE3EDC7_6C61_4153_B25B_32A8904A37B1__INCLUDED_)
