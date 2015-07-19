#include "Precompiled.h"
// DelayedDirectoryChangeHandler.cpp: implementation of the CDelayedDirectoryChangeHandler2 class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "DirectoryChanges.h"
#include "DelayedDirectoryChangeHandler.h"
#include <process.h>//for _beginthreadex

#include <shlwapi.h>				 // for PathMatchSpec
#pragma comment( lib, "shlwapi.lib") // function



#define UWM_DELAYED_DIRECTORY_NOTIFICATION (WM_APP+1024)


HINSTANCE GetInstanceHandle()
{
	return (HINSTANCE)GetModuleHandle(NULL);
	// ASSERT( AfxGetInstanceHandle() == (HINSTANCE)GetModuleHandle(NULL) ); <-- true for building .exe's 
	//NOTE: In Dll's using shared MFC, AfxGetInstanceHandle() != (HINSTANCE)GetModuleHandle(NULL)...
	//don't know if this is the case for dll's using static MFC
}
static inline bool IsEmptyString(LPCTSTR sz)
{
	return (bool)(sz==NULL || *sz == 0);
}
/*********************************************************
  PathMatchSpec() requires IE 4.0 or greater on NT...
  if running on NT 4.0 w/ out IE 4.0, then uses this function instead.

  Based on code by Jack Handy:
  http://www.codeproject.com/string/wildcmp.asp

  Changed slightly to match the PathMatchSpec signature, be unicode compliant & to ignore case by myself.
  
*********************************************************/

#define _TESTING_WILDCMP_ONLY_ 

BOOL STDAPICALLTYPE wildcmp(const std::wstring & str, const std::wstring & w ) 
{
	const WCHAR *string = str.c_str();
	const WCHAR *wild = w.c_str();

	const WCHAR *cp, *mp;
	cp = mp = NULL;
	
	while ((*string) && (*wild != _T('*'))) 
	{
		if ((_toupper(*wild) != _toupper(*string)) && (*wild != _T('?'))) 
		{
			return FALSE;
		}
		wild++;
		string++;
	}
		
	while (*string) 
	{
		if (*wild == _T('*')) 
		{
			if (!*++wild) 
			{
				return TRUE;
			}
			mp = wild;
			cp = string+1;
		} 
		else 
		if ((_toupper(*wild) == _toupper(*string)) || (*wild == _T('?'))) 
		{
			wild++;
			string++;
		} 
		else 
		{
			wild = mp;
			string = cp++;
		}
	}
		
	while (*wild == _T('*')) 
	{
		wild++;
	}
	return (!*wild)? TRUE : FALSE;
}

//////////////////////////////////////////////////////////////////////////
//
//CDirChangeNotification member functions:
//
CDirChangeNotification::CDirChangeNotification(CDelayedDirectoryChangeHandler *	pDelayedHandler, HANDLE hEventWhenDone)
:m_pDelayedHandler( pDelayedHandler )
,m_szFileName1()
,m_szFileName2()
,m_dwError(0UL)
,m_notificationDoneEvent(hEventWhenDone)
{
	ASSERT( pDelayedHandler );
}


CDirChangeNotification::~CDirChangeNotification()
{
	if (m_notificationDoneEvent)
	{
		::SetEvent(m_notificationDoneEvent);
		m_notificationDoneEvent = NULL;
	}
}

void CDirChangeNotification::DispatchNotificationFunction()
{
	ASSERT( m_pDelayedHandler );
	if( m_pDelayedHandler )
		m_pDelayedHandler->DispatchNotificationFunction( this );

	// at this point, "this" has been deleted by DispatchNotificationFunction
}

void CDirChangeNotification::PostOn_FileAdded(const std::wstring &szFileName)
{
	ASSERT( !szFileName.empty() );
	m_eFunctionToDispatch	= eOn_FileAdded;
	m_szFileName1			= szFileName;
	//
	// post the message so it'll be dispatch by another thread.
	PostNotification();

}
void CDirChangeNotification::PostOn_FileRemoved(const std::wstring &szFileName)
{
	ASSERT( !szFileName.empty() );
	m_eFunctionToDispatch	= eOn_FileRemoved;
	m_szFileName1			= szFileName;
	//
	// post the message so it'll be dispatched by another thread.
	PostNotification();
	
}
void CDirChangeNotification::PostOn_FileNameChanged(const std::wstring &szOldName, const std::wstring &szNewName)
{
	ASSERT( !szOldName.empty() && !szNewName.empty());

	m_eFunctionToDispatch	= eOn_FileNameChanged;
	m_szFileName1			= szOldName;
	m_szFileName2			= szNewName;
	//
	// post the message so it'll be dispatched by another thread.
	PostNotification();
	
}

void CDirChangeNotification::PostOn_UnknownChange()
{
	m_eFunctionToDispatch = eOn_UnknownChange;
	m_szFileName1.clear();
	m_szFileName2.clear();

	//
	// post the message so it'll be dispatched by another thread.
	PostNotification();
}

void CDirChangeNotification::PostOn_FileModified(const std::wstring &szFileName)
{
	ASSERT( !szFileName.empty() );

	m_eFunctionToDispatch	= eOn_FileModified;
	m_szFileName1			= szFileName;
	//
	// post the message so it'll be dispatched by another thread.
	PostNotification();
}

void CDirChangeNotification::PostOn_ReadDirectoryChangesError(DWORD dwError, const std::wstring &szDirectoryName)
{
	ASSERT( !szDirectoryName.empty() );

	m_eFunctionToDispatch = eOn_ReadDirectoryChangesError;
	m_dwError			  = dwError;
	m_szFileName1		  = szDirectoryName;
	//
	// post the message so it'll be dispatched by the another thread.
	PostNotification();
}

void CDirChangeNotification::PostOn_WatchStarted(DWORD dwError, const std::wstring &szDirectoryName)
{
	ASSERT( !szDirectoryName.empty() );

	m_eFunctionToDispatch = eOn_WatchStarted;
	m_dwError			  =	dwError;
	m_szFileName1		  = szDirectoryName;

	PostNotification();
}

void CDirChangeNotification::PostOn_WatchStopped(const std::wstring & szDirectoryName)
{
	ASSERT( !szDirectoryName.empty() );

	m_eFunctionToDispatch = eOn_WatchStopped;
	m_szFileName1		  = szDirectoryName;

	PostNotification();
}

void CDirChangeNotification::PostNotification()
{
	ASSERT( m_pDelayedHandler );
	if( m_pDelayedHandler )
		m_pDelayedHandler->PostNotification( this );
}

static LRESULT CALLBACK DelayedNotificationWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//
//	This is the wndproc for the notification window
//
//	it's here to dispatch the notifications to the client
//
{
	if( message == UWM_DELAYED_DIRECTORY_NOTIFICATION )
	{
		CDirChangeNotification * pNotification = reinterpret_cast<CDirChangeNotification*>(lParam);
		ASSERT(  pNotification );
		if( pNotification )
		{
			DWORD dwEx(0);
			__try{
				pNotification->DispatchNotificationFunction();
			}
			__except(dwEx = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER){
				//An exception was raised:
				//
				//	Likely cause: there was a problem creating the CDelayedDirectoryChangeHandler::m_hWatchStoppedDispatchedEvent object
				//	and the change handler object was deleted before the notification could be dispatched to this function.
				//
				//  or perhaps, somebody's implementation of an overridden function caused an exception
				TRACEF(_T("Following exception occurred: %d -- File: %s Line: %d\n"), dwEx, _T(__FILE__), __LINE__);
			}
		}
		
		return 0UL;
	}
	else
		return DefWindowProc(hWnd,message,wParam,lParam);
}



/////////////////////////////////////////////////////////
//	CDelayedNoticationThread
//
long	CDelayedNotificationThread::s_nRefCnt = 0L;
HANDLE	CDelayedNotificationThread::s_hThread = NULL;
DWORD	CDelayedNotificationThread::s_dwThreadID = 0UL;

void CDelayedNotificationThread::PostNotification(CDirChangeNotification * pNotification)
{
	ASSERT( s_hThread != NULL );
	ASSERT( s_dwThreadID != 0 );

	if(
		!PostThreadMessage(s_dwThreadID, 
						   UWM_DELAYED_DIRECTORY_NOTIFICATION, 
						   0, 
						   reinterpret_cast<LPARAM>(pNotification))
	  )
	{
		//Note, this can sometimes fail.
		//Will fail if: s_dwThreadID references a invalid thread id(the thread has died for example)
		// OR will fail if the thread doesn't have a message queue.
		//
		//	This was failing because the thread had not been fully started by the time PostThreadMessage had been called
		//
		//Note: if this fails, it creates a memory leak because
		//the CDirChangeNotification object that was allocated and posted
		//to the thread is actually never going to be dispatched and then deleted.... it's
		//hanging in limbo.....

		//
		//	The fix for this situation was to force the thread that starts
		//	this worker thread to wait until the worker thread was fully started before
		//	continueing.  accomplished w/ an event... also.. posting a message to itself before signalling the 
		//  'spawning' thread that it was started ensured that there was a message pump
		//  associated w/ the worker thread by the time PostThreadMessage was called.
		TRACEF(_T("PostThreadMessage() failed while posting to thread id: %d! GetLastError(): %d%s\n"), s_dwThreadID, GetLastError(), GetLastError() == ERROR_INVALID_THREAD_ID? _T("(ERROR_INVALID_THREAD_ID)") : _T(""));
	}
}

bool CDelayedNotificationThread::StartThread()
{
	TRACEF(_T("CDelayedNotificationThread::StartThread()\n"));
	ASSERT( s_hThread == NULL 
		&&	s_dwThreadID == 0 );
	s_hThread = (HANDLE)_beginthreadex(NULL,0, 
								ThreadFunc, this, 0, (UINT*) &s_dwThreadID);
	if( s_hThread )
		WaitForThreadStartup();

	return s_hThread == NULL ? false : true;

}

bool CDelayedNotificationThread::StopThread()
{
	TRACEF(_T("CDelayedNotificationThread::StopThread()\n"));
	if( s_hThread != NULL 
	&&	s_dwThreadID != 0 )
	{
		PostThreadMessage(s_dwThreadID, WM_QUIT, 0,0);

		WaitForSingleObject(s_hThread, INFINITE);
		CloseHandle(s_hThread);
		s_hThread	 = NULL;
		s_dwThreadID = 0UL;
		return true;
	}
	return true;//already shutdown
}

UINT __stdcall CDelayedNotificationThread::ThreadFunc(LPVOID lpvThis)
{
	//UNREFERENCED_PARAMETER( lpvThis );
	//
	//	Implements a simple message pump
	//
	CDelayedNotificationThread * pThis = reinterpret_cast<CDelayedNotificationThread*>(lpvThis);
	ASSERT( pThis );

	//
	//	Insure that this thread has a message queue by the time another
	//	thread gets control and tries to use PostThreadMessage
	//	problems can happen if someone tries to use PostThreadMessage
	//	in between the time pThis->SignalThreadStartup() is called,
	//	and the first call to GetMessage();

	::PostMessage(NULL, WM_NULL, 0,0);//if this thread didn't have a message queue before this, it does now.


	//
	//
	//	Signal that this thread has started so that StartThread can continue.
	//
	if( pThis ) pThis->SignalThreadStartup();

	TRACEF(_T("CDelayedNotificationThread::ThreadFunc() ThreadID: %d -- Starting\n"), GetCurrentThreadId());
	MSG msg;
	do{
		while( GetMessage(&msg, NULL, 0,0) )//note GetMessage() can return -1, but only if i give it a bad HWND.(HWND for another thread for example)..i'm not giving an HWND, so no problemo here.
		{
			if( msg.message == UWM_DELAYED_DIRECTORY_NOTIFICATION )
			{
				CDirChangeNotification * pNotification = 
								reinterpret_cast<CDirChangeNotification *>( msg.lParam );
				DWORD dwEx(0UL);

				__try{
				if( pNotification )
					pNotification->DispatchNotificationFunction();
				}
				__except(dwEx = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER){
				//An exception was raised:
				//
				//	Likely causes: 
				//		* There was a problem creating the CDelayedDirectoryChangeHandler::m_hWatchStoppedDispatchedEvent object
				//			and the change handler object was deleted before the notification could be dispatched to this function.
				//
				//		* Somebody's implementation of an overridden virtual function caused an exception
				TRACEF(_T("The following exception occurred: %d -- File: %s Line: %d\n"), dwEx, _T(__FILE__), __LINE__);
				}
			}
			else
			if( msg.message == WM_QUIT )
			{
				break;
			}
		}
	}while( msg.message != WM_QUIT );
	TRACEF(_T("CDelayedNotificationThread::ThreadFunc() exiting. ThreadID: %d\n"), GetCurrentThreadId());
	return 0;
}

long CDelayedNotificationThread::AddRef()
{
	if( InterlockedIncrement(&s_nRefCnt) == 1 )
	{
		VERIFY( StartThread() );
	}
	return s_nRefCnt;
}
long CDelayedNotificationThread::Release()
{
	if( InterlockedDecrement(&s_nRefCnt) <= 0 )
	{
		s_nRefCnt = 0;
		VERIFY( StopThread() );
	}
	return s_nRefCnt;
}

///////////////////////////////////////////////////////
//static member data for CDelayedDirectoryChangeHandler
HINSTANCE CDelayedDirectoryChangeHandler::s_hShlwapi_dll = NULL;//for the PathMatchSpec() function
BOOL CDelayedDirectoryChangeHandler::s_bShlwapi_dllExists = TRUE;
long CDelayedDirectoryChangeHandler::s_nRefCnt_hShlwapi = 0L;

///////////////////////////////////////////////////////
//construction destruction
CDelayedDirectoryChangeHandler::CDelayedDirectoryChangeHandler(CDirectoryChangeHandler * pRealHandler)
: m_pDelayNotifier( NULL )
 ,m_pRealHandler( pRealHandler )
 ,m_hWatchStoppedDispatchedEvent(NULL)
 {


	ASSERT( m_pRealHandler ); 


	m_hWatchStoppedDispatchedEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);//AUTO-RESET, not initially signalled
	ASSERT( m_hWatchStoppedDispatchedEvent );
	
	// Notifications will be executed in this worker thread.
	//
	m_pDelayNotifier = new CDelayedNotificationThread();
}

CDelayedDirectoryChangeHandler::~CDelayedDirectoryChangeHandler()
{
	if( m_pRealHandler )
		delete m_pRealHandler, m_pRealHandler = NULL;
	if( m_pDelayNotifier )
		delete m_pDelayNotifier, m_pDelayNotifier = NULL;

	if( m_hWatchStoppedDispatchedEvent )
		CloseHandle(m_hWatchStoppedDispatchedEvent), m_hWatchStoppedDispatchedEvent = NULL;
}



CDirChangeNotification * CDelayedDirectoryChangeHandler::GetNotificationObject(HANDLE *outWaitEvent)
//
//	Maybe in future I'll keep a pool of these 
//	objects around to increase performance...
//	using objects from a cache will be faster 
//	than allocated and destroying a new one each time.
//	
//  
{
	HANDLE waitEvent = 0;
	if (outWaitEvent)
	{
		waitEvent = ::CreateEvent(0, false, 0, NULL);
		*outWaitEvent = waitEvent;
		// the wait event will have to be released by the caller (::CloseHandle)
	}

	ASSERT( m_pRealHandler );
	return new CDirChangeNotification(this, waitEvent);//helps support FILTERS_CHECK_PARTIAL_PATH
}

void CDelayedDirectoryChangeHandler::DisposeOfNotification(CDirChangeNotification * pNotification)
{
	delete pNotification;
}

//These functions are called when the directory to watch has had a change made to it
void CDelayedDirectoryChangeHandler::On_FileAdded(const std::wstring & strFileName)
{
	CDirChangeNotification * p = GetNotificationObject();
	ASSERT( p );
	if( p )
		p->PostOn_FileAdded( strFileName.c_str() );
}

void CDelayedDirectoryChangeHandler::On_FileRemoved(const std::wstring & strFileName)
{
	CDirChangeNotification * p = GetNotificationObject();
	ASSERT( p );
	if( p )
		p->PostOn_FileRemoved( strFileName );
}

void CDelayedDirectoryChangeHandler::On_FileModified(const std::wstring & strFileName)
{
	CDirChangeNotification * p = GetNotificationObject();
	ASSERT( p );
	if( p ) p->PostOn_FileModified( strFileName );
}

void CDelayedDirectoryChangeHandler::On_FileNameChanged(const std::wstring & strOldFileName, const std::wstring & strNewFileName)
{
	CDirChangeNotification * p = GetNotificationObject();	
	ASSERT( p );
	if( p ) p->PostOn_FileNameChanged( strOldFileName, strNewFileName );
}

void CDelayedDirectoryChangeHandler::On_UnknownChange()
{
	CDirChangeNotification * p = GetNotificationObject();
	ASSERT(p);
	if (p) p->PostOn_UnknownChange();
}

void CDelayedDirectoryChangeHandler::On_ReadDirectoryChangesError(DWORD dwError, const std::wstring & strDirectoryName)
{
	HANDLE waitForExecution;
	CDirChangeNotification * p = GetNotificationObject(&waitForExecution);
	ASSERT( p );
	if( p )
	{
		p->PostOn_ReadDirectoryChangesError( dwError, strDirectoryName );
		::WaitForSingleObject(waitForExecution, INFINITE);
		::CloseHandle(waitForExecution);
	}
}

void CDelayedDirectoryChangeHandler::On_WatchStarted(DWORD dwError, const std::wstring& strDirectoryName)
{
	CDirChangeNotification * p = GetNotificationObject();

	if( p )
		p->PostOn_WatchStarted(dwError, strDirectoryName);
}

void CDelayedDirectoryChangeHandler::On_WatchStopped(const std::wstring & strDirectoryName)
{
	CDirChangeNotification * p = GetNotificationObject();

	if( p ){
		if( m_hWatchStoppedDispatchedEvent )
			::ResetEvent(m_hWatchStoppedDispatchedEvent);

		p->PostOn_WatchStopped( strDirectoryName );

		//	Wait that this function has been dispatched to the other thread
		//	before continuing.  This object may be getting deleted
		//	soon after this function returns, and before the function can be
		//	dispatched to the other thread....
		WaitForOnWatchStoppedDispatched();
	}
}


void CDelayedDirectoryChangeHandler::PostNotification(CDirChangeNotification * pNotification)
{
	if( m_pDelayNotifier )
		m_pDelayNotifier->PostNotification( pNotification );
}

inline bool IsNonFilterableEvent( CDirChangeNotification::eFunctionToDispatch eEvent)
// Helper function
//	For filtering events..... these functions can not be filtered out.
//
{
	if(	eEvent == CDirChangeNotification::eOn_WatchStarted 
	||	eEvent == CDirChangeNotification::eOn_WatchStopped
	||	eEvent == CDirChangeNotification::eOn_ReadDirectoryChangesError )
	{
		return true;
	}
	else
		return false;
}

void CDelayedDirectoryChangeHandler::DispatchNotificationFunction(CDirChangeNotification * pNotification)
/*****************************************************
	This function is called when we want the notification to execute.

	
******************************************************/
{
	ASSERT( m_pRealHandler );
	ASSERT( pNotification );
	if( pNotification && m_pRealHandler )
	{
		//
		//	Allow the client to ignore the notification
		//
		//
		switch( pNotification->m_eFunctionToDispatch )
		{
		case CDirChangeNotification::eOn_FileAdded:
			
			m_pRealHandler->On_FileAdded( pNotification->m_szFileName1 ); 
			break;
			
		case CDirChangeNotification::eOn_FileRemoved:
			
			m_pRealHandler->On_FileRemoved( pNotification->m_szFileName1 );
			break;
			
		case CDirChangeNotification::eOn_FileNameChanged:
			
			m_pRealHandler->On_FileNameChanged( pNotification->m_szFileName1, pNotification->m_szFileName2 );
			break;
			
		case CDirChangeNotification::eOn_UnknownChange:

			m_pRealHandler->On_UnknownChange();
			break;

		case CDirChangeNotification::eOn_FileModified:
			
			m_pRealHandler->On_FileModified( pNotification->m_szFileName1 );
			break;
			
		case CDirChangeNotification::eOn_ReadDirectoryChangesError:
			
			m_pRealHandler->On_ReadDirectoryChangesError( pNotification->m_dwError, pNotification->m_szFileName1 );
			break;
			
		case CDirChangeNotification::eOn_WatchStarted:
	
			m_pRealHandler->On_WatchStarted(pNotification->m_dwError, pNotification->m_szFileName1);
			break;
	
		case CDirChangeNotification::eOn_WatchStopped:
			
			try{
				//
				//	The exception handler is just in case of the condition described in DirectoryChanges.h
				//	in the comments for On_WatchStopped()
				//
				m_pRealHandler->On_WatchStopped(pNotification->m_szFileName1);

			}catch(...){
				MessageBeep( 0xffff );
				MessageBeep( 0xffff );
		#ifdef DEBUG
				MessageBox(NULL,_T("An RTFM Exception was raised in On_WatchStopped() -- see Comments for CDirectoryChangeHandler::On_WatchStopped() in DirectoryChanges.h."), _T("Programmer Note(DEBUG INFO):"), MB_ICONEXCLAMATION | MB_OK);
		#endif
			}
			//
			//	Signal that the On_WatchStopped() function has been dispatched.
			//
			if( m_hWatchStoppedDispatchedEvent )
				SetEvent(m_hWatchStoppedDispatchedEvent);
			break;
		case CDirChangeNotification::eFunctionNotDefined:
		default:
			break;
		}//end switch()
	}	

	if( pNotification )						 //		
		DisposeOfNotification(pNotification);// deletes or releases the notification object from memory/use
											 //
}

BOOL CDelayedDirectoryChangeHandler::WaitForOnWatchStoppedDispatched( )
//
//	When shutting down, m_pRealHandler->On_WatchStopped() will be called.
//	Because it's possible that this object will be deleted before the notification
//	can be dispatched to the other thread, we have to wait until we know that it's been executed
//	before returning control.
//
//	This function signals that the function has been dispatched to the other
//	thread and it will be safe to delete this object once this has returned.
//
{
	ASSERT( m_hWatchStoppedDispatchedEvent );
	DWORD dwWait = WAIT_FAILED;
	if( m_hWatchStoppedDispatchedEvent )
	{

		//
		//	The function will be dispatched to another thread...
		//	just wait for the event to be signalled....
		do{
			dwWait	= WaitForSingleObject(m_hWatchStoppedDispatchedEvent, 5000);//wait five seconds
			if( dwWait != WAIT_OBJECT_0 )
			{
				TRACEF(_T("WARNING: Possible Deadlock detected! ThreadID: %d File: %s Line: %d\n"), GetCurrentThreadId(), _T(__FILE__), __LINE__);
			}
		}while( dwWait != WAIT_OBJECT_0 );
	}
	else
	{
		TRACEF(_T("WARNING: Unable to wait for notification that the On_WatchStopped function has been dispatched to another thread.\n"));
		TRACEF(_T("An Exception may occur shortly.\n"));
		TRACEF(_T("File: %s Line: %d"), _T( __FILE__ ), __LINE__);
	
	}


	return (dwWait == WAIT_OBJECT_0 );
}

void CDelayedDirectoryChangeHandler::SetChangedDirectoryName(const std::wstring & strChangedDirName)
{
	ASSERT( m_pRealHandler );
	CDirectoryChangeHandler::SetChangedDirectoryName(strChangedDirName);
	if( m_pRealHandler )
		m_pRealHandler->SetChangedDirectoryName( strChangedDirName );
}
const std::wstring & CDelayedDirectoryChangeHandler::GetChangedDirectoryName() const
{
	if( m_pRealHandler )
		return m_pRealHandler->GetChangedDirectoryName();
	return CDirectoryChangeHandler::GetChangedDirectoryName();
}