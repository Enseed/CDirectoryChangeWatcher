// DirWatcher.h : main header file for the DIRWATCHER application
//

#if !defined(AFX_DIRWATCHER_H__7B9F7CB4_A044_4B1E_80C3_6DCBB8CCCC7F__INCLUDED_)
#define AFX_DIRWATCHER_H__7B9F7CB4_A044_4B1E_80C3_6DCBB8CCCC7F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDirWatcherApp:
// See DirWatcher.cpp for the implementation of this class
//

class CDirWatcherApp : public CWinApp
{
public:
	CDirWatcherApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDirWatcherApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDirWatcherApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIRWATCHER_H__7B9F7CB4_A044_4B1E_80C3_6DCBB8CCCC7F__INCLUDED_)
