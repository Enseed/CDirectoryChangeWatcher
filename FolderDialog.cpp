
/* 
DESCRIPTION:
	CFolderDialog  - Folder Selection Dialog Class

NOTES:
	Copyright(C) Armen Hakobyan, 2002
	mailto:armenh@cit.am
	
VERSION HISTORY:
	24 Mar 2002 - First release
*/

#include "stdafx.h"
#include "FolderDialog.h"

#ifndef BFFM_VALIDATEFAILED
#ifndef UNICODE
#define BFFM_VALIDATEFAILED 3
#else
#define BFFM_VALIDATEFAILED 4	
#endif
#endif //!BFFM_VALIDATEFAILED


// CFolderDialog

IMPLEMENT_DYNAMIC(CFolderDialog, CDialog)
CFolderDialog::CFolderDialog(LPCTSTR lpszTitle /*NULL*/,
							 LPCTSTR lpszFolderName /*NULL*/,
							 CWnd* pParentWnd /*NULL*/,
							 UINT uFlags /*BIF_RETURNONLYFSDIRS*/)
			 : CCommonDialog(pParentWnd)
			 , m_hWnd(NULL)			
{
	m_szSelectedPath[0]	= '\0';
	m_szFolderPath[0]	= '\0';
	m_szFolderDisplayName[0]= '\0';
	
	if(lpszFolderName != NULL)
		lstrcpy(m_szSelectedPath, lpszFolderName);
	
	// Fill
	::ZeroMemory(&m_bi, sizeof(BROWSEINFO)); 
	m_bi.hwndOwner = pParentWnd->GetSafeHwnd();
	m_bi.pidlRoot = NULL;
	m_bi.pszDisplayName = m_szFolderDisplayName;
	m_bi.lpszTitle = lpszTitle;
	m_bi.ulFlags = uFlags;
	m_bi.lpfn = BrowseCallbackProc;
	m_bi.lParam = (LPARAM)this;
}

CFolderDialog::~CFolderDialog(void)
{
}

BEGIN_MESSAGE_MAP(CFolderDialog, CCommonDialog)
END_MESSAGE_MAP()

// CFolderDialog message handlers

int CFolderDialog::DoModal(void)
{
	ASSERT_VALID(this);	
	ASSERT(m_bi.lpfn != NULL);
		
	m_bi.hwndOwner = PreModal();	
	INT_PTR nRet   = -1;
	LPITEMIDLIST pItemIDList = ::SHBrowseForFolder(&m_bi);

	if(pItemIDList != NULL)
	{
		if(::SHGetPathFromIDList(pItemIDList, m_szFolderPath))
		{
			IMalloc* pMalloc = NULL;
			if(SUCCEEDED(::SHGetMalloc(&pMalloc)))
			{
				pMalloc->Free(pItemIDList);
				pMalloc->Release();
			}
			nRet = IDOK;
		}
		else
		{
			nRet = IDCANCEL;
		}
	}
	PostModal();
	return nRet;	
}

// Overridables:

void CFolderDialog::OnInitialized(void)
{
	if(lstrlen(m_szSelectedPath))
		SetSelection(m_szSelectedPath);
}

int CFolderDialog::OnValidateFailed(LPCTSTR lpstrFolderPath)
{
	int nRet = AfxMessageBox(_T("The path entered is not valid! Continue ?"), 
				MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
	
	// Return 1 = continue, 0 = EndDialog
	return((nRet == IDYES) ? 0 : 1);
}

void CFolderDialog::OnSelChanged(LPITEMIDLIST /*pItemIDList*/)
{
}

// Callback function used with the SHBrowseForFolder function. 

INT CALLBACK CFolderDialog::BrowseCallbackProc(HWND hWnd, 
							UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	CFolderDialog* pThis = (CFolderDialog*)(lpData);
	pThis->m_hWnd = hWnd;
	int nRet = 0;

	switch(uMsg)
	{
	case BFFM_INITIALIZED:
		pThis->OnInitialized();
		break;
	case BFFM_SELCHANGED:
		pThis->OnSelChanged((LPITEMIDLIST)lParam);
		break;
	case BFFM_VALIDATEFAILED:
		nRet = pThis->OnValidateFailed((LPCTSTR)lParam);
		break;
	}
	pThis->m_hWnd = NULL;
	return nRet;	
}
