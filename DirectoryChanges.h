// DirectoryChanges.h: interface for the 
// CDirectoryChangeWatcher and CDirectoryChangeHandler classes.
//
//  Uses an io completion port and ReadDirectoryChangesW -- this code will only work on 
//	Windows NT/2000/XP.
//
//	The directory being watched must be a directory on a Windows NT/2000/XP machine
//
//
//	These classes are based on the FWatch sample program in the SDK.
//
//
//	If you get a compile time error that ReadDirectoryChangesW is an undeclared identifier,
//  you'll need to #define _WIN32_WINNT 0x400 in stdafx.h.
//
//
/*******************************************************************
//		*** COPYRIGHT NOTICE ****
//
//	Copyright 2001. Wes Jones (wesj@hotmail.com)
//
//	This code is free for use under the following conditions:
//
//		You may not claim authorship of this code.
//		You may not sell or distrubute this code without author's permission.
//		You are not permitted to sell this code in it's compiled, non-compiled, executable, or any other form. 
//		Executable code excepted in the case that it is not sold separately from an application which 
//		uses this it. This means you can use this code for your applications as you see fit, but this code may not be sold in any form
//		to others for use in their applications.
//		This copyright notice may not be removed.
//		
//
//		If this code was NOT obtained by downloading it from www.codeproject.com,
//		or given to you by a friend or coworker, please tell me, & let me know how you got it. There a plenty of lazy bastards that 
//		collect source code from the internet, and then sell it as part of 
//		a 'Programmer's Library'.  Using this code for such a purpose is strictly prohibited.
//
//		If you'd like to pay me to turn this into an ActiveX/COM object so you 
//		can use it in a Visual Basic application, feel free to contact me with an offer,
//		and I will create it for you. Otherwise, here is the source code, and you may make your own
//		ActiveX/COM object, providing that it is not sold separately.
//
//  No guarantees or warranties are expressed or implied. 
//	This code may contain bugs.
//	Warning: May contain matter. If this should come into contact with anti-matter, a violent explosion may occur.
*******************************************************************/

//  Please let me know of any bugs, bug fixes, or enhancements made to this code.
//  If you have ideas for this, and/or tips or admonitions, I would be glad to hear them.
//
//	See notes at top of DirectoryChanges.cpp modification history and more info.
//
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIRECTORYCHANGES_H__02E53FDE_CB22_4176_B6D7_DA3675D9F1A6__INCLUDED_)
#define AFX_DIRECTORYCHANGES_H__02E53FDE_CB22_4176_B6D7_DA3675D9F1A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Wrapper/OSA/OSThread/OSThread.h>
#include <Generic/AutoPtr/AutoPtr.h>
#include <Generic/Base/win/Windows.h>
#include <vector>

//#include <afxmt.h>
//#include <afxtempl.h>

#define READ_DIR_CHANGE_BUFFER_SIZE 4096

class CFileNotifyInformation;//helper class 
class CDirectoryChangeWatcher;
class CDelayedDirectoryChangeHandler;//helper class...used in implementation

class CCriticalSection
{
public:
	class CSingleLock
	{
	public:
		CSingleLock(CCriticalSection *cs)
		{
			_cs = cs;
		}
		~CSingleLock()
		{
			_cs->Unlock();
		}
	private:
		CSingleLock(const CSingleLock&);
		CSingleLock& operator=(const CSingleLock&);
	private:
		CCriticalSection *_cs;
	};

public:
	CCriticalSection()
	{
		Init();
	}

	void Init()
	{
		InitializeCriticalSection(&mCS);
	}


	~CCriticalSection()
	{
		DeleteCriticalSection(&mCS);
	}

	BOOL Lock()
	{
		EnterCriticalSection(&mCS);
		return TRUE;
	}

	BOOL Unlock()
	{
		LeaveCriticalSection(&mCS);
		return TRUE;
	}

private:
	CCriticalSection(const CCriticalSection &);
	CCriticalSection& operator=(const CCriticalSection &);

private:
	CRITICAL_SECTION mCS;
};

class CDirectoryChangeHandler
/***********************************
 A class to handle changes to files in a directory.  
 The virtual On_Filexxx() functions are called whenever changes are made to a watched directory that is being handled by this object...
 The On_Filexxx() functions execute in the context of the main thread if true is passed to the constructor of CDirectoryChangeWatcher,
 OR they fire in the context of a worker thread if false is passed to the constructor of CDirectoryChangeWatcher

  NOTES: 
		A CDirectoryChangeHandler can only be used by ONE CDirectoryChangeWatcher object,
		but a CDirectoryChangeWatcher object may use multiple CDirectoryChangeHandler objects.

	When this object is destroyed, whatever directories that it was being used to handle directory changes for
	will automatically be 'unwatched'.

	The class is reference counted.  The reference count is increased every time it is used 
	in a (successfull) call to CDirectoryChangeWatcher::WatchDirectory() and is decremented whenever
	the directory becomes 'unwatched'.

   The only notifications are File Added, Removed, Modified, and Renamed.
   Even though the CDirectoryChangeWatcher::WatchDirectory (which'll call ReadDirectoryChangesW()) function allows you to specify flags 
   to watch for changes to last access time, last write time, attributes changed, etc,
   these changes all map out to On_FileModified() which doesn't specify the type of modification.


  NOTE:   The CDirectoryChangeHandler::On_Filexxx() functions
		  are called in the context of the main thread, the thread that called CDirectoryChangeWatcher::WatchDirectory(),
		  if you pass true to the constructor of CDirectoryChangeWatcher. This is accomplished via a hidden window,
		  and REQUIRES that your app has a message pump. 
		  For console applications, or applications w/out a message pump, you can pass false to the constructor
		  of CDirectoryChangeWatcher, and these notifications will fire in the context of a worker thread.  By passing false
		  to the constructor of CDirectoryChangeWatcher, you do NOT NEED a message pump in your application.



************************************/
{
public:

	CDirectoryChangeHandler();
	virtual ~CDirectoryChangeHandler();

	//this class is reference counted
	long AddRef();
	long Release();
	long CurRefCnt()const;


	BOOL UnwatchDirectory();//causes CDirectoryChangeWatcher::UnwatchDirectory() to be called.
	
	const std::wstring & GetChangedDirectoryName() const { return m_strChangedDirectoryName;}//WARNING: don't use this, this function will be removed in a future release.
								//returns the directory name where the change occured.  This contains
							   //the last directory to have changed if the same CDirectoryChangeHandler is
							   //being used to watch multiple directories. It will return an empty string
							   //if no changes have been made to a directory yet.   It will always be the 
							   //name of the currently changed directory(as specified in CDirectoryChangeWatcher::WatchDirectory())
							   //if called in the context of one of the
							   //On_Filexxx() functions.
protected:
	//
	//	Override these functions:
	//	These functions are called when the directory to watch has had a change made to it
	virtual void On_FileAdded(const std::wstring & strFileName); //=0;
			//
			//	On_FileAdded()
			//
			//	This function is called when a file in one of the watched folders(or subfolders)
			//	has been created.
			//
			//	For this function to execute you'll need to specify FILE_NOTIFY_CHANGE_FILE_NAME or FILE_NOTIFY_CHANGE_DIR_NAME(for directories)
			//  when you call CDirectoryChangeWatcher::WatchDirectory()
			//
	virtual void On_FileRemoved(const std::wstring & strFileName);// = 0;
			//
			//	On_FileRemoved()
			//
			//	This function is called when a file in one of the watched folders(or subfolders)
			//	has been deleted(or moved to another directory)
			//
			//	For this function to execute you'll need to specify FILE_NOTIFY_CHANGE_FILE_NAME or FILE_NOTIFY_CHANGE_DIR_NAME(for directories)
			//  when you call CDirectoryChangeWatcher::WatchDirecotry()
			//

	virtual void On_FileNameChanged(const std::wstring & strOldFileName, const std::wstring & strNewFileName);// = 0;
			//
			//	On_FileNameChanged()
			//
			//	This function is called when a file in one of the watched folders(or subfolders)
			//	has been renamed.
			//
			//
			//	You'll need to specify FILE_NOTIFY_CHANGE_FILE_NAME (or FILE_NOTIFY_CHANGE_DIR_NAME(for directories))
			//	when you call CDirectoryChangeWatcher::WatchDirectory()
			//
			//	

	virtual void On_FileModified(const std::wstring & strFileName);// = 0;
			//
			//	On_FileModified()
			//
			//	This function is called whenever an attribute specified by the watch
			//	filter has changed on a file in the watched directory or 
			//
			//	Specify any of the following flags when you call CDirectoryChangeWatcher::WatchDirectory()
			//  
			//
			//	FILE_NOTIFY_CHANGE_ATTRIBUTES
			//	FILE_NOTIFY_CHANGE_SIZE 
			//	FILE_NOTIFY_CHANGE_LAST_WRITE 
			//	FILE_NOTIFY_CHANGE_LAST_ACCESS
			//	FILE_NOTIFY_CHANGE_CREATION (* See Note # 1* )
			//	FILE_NOTIFY_CHANGE_SECURITY
			//
			//	
			//	General Note)  Windows tries to optimize some of these notifications.  You may not get 
			//				   a notification every single time a file is accessed for example.  
			//				   There's a MS KB article or something on this(sorry forgot which one).
			//
			//	Note #1	)   This notification isn't what you might think(FILE_NOTIFY_CHANGE_CREATION). 
			//				See the help files for ReadDirectoryChangesW...
			//				This notifies you of a change to the file's 
			//				creation time, not when the file is created.  
			//				Use FILE_NOTIFY_CHANGE_FILE_NAME to know about newly created files.
			//

	virtual void On_UnknownChange();
			//
			//	On_UnknownChange()
			//
			//	This function is called when the change in the directory was not specified.
			//	This might be the case when the watched directory is on a network drive and
			//	the file server does not specify details about the changed that occured.
			//	(ex. Linux Samba server or Mac OS X windows sharing)
			//	This call should not occur in the local file system or from a network drive
			//	managed by a Windows operating system.


	virtual void On_ReadDirectoryChangesError(DWORD dwError, const std::wstring & strDirectoryName);
			//
			//	On_ReadDirectoryChangesError()
			//
			//	This function is called when ReadDirectoryChangesW() fails during normal
			//	operation (ie: some time after you've called CDirectoryChangeWatcher::WatchDirectory())
			//
			//
			//	NOTE:  *** READ THIS *** READ THIS *** READ THIS *** READ THIS ***
			//
			//	NOTE: If this function has been called, the watched directory has been automatically unwatched.
			//			You will not receive any further notifications for that directory until 
			//			you call CDirectoryChangeWatcher::WatchDirectory() again.
			//
			//	On_WatchStopped() will not be called.


	virtual void On_WatchStarted(DWORD dwError, const std::wstring& strDirectoryName);
			//
			//	void On_WatchStarted()
			//
			//	This function is called when a directory watch has begun.  
			//	It will be called whether CDirectoryChangeWatcher::WatchDirectory() is successful or not. Check the dwError parameter.
			//
			//	PARAMETERS:
			//	DWORD dwError					 -- 0 if successful, else it's the return value of GetLastError() 
			//										indicating why the watch failed.
			//	const CString & strDirectoryName -- The full path and name of the directory being watched.
	
	virtual void On_WatchStopped(const std::wstring & strDirectoryName);
			//
			//	void On_WatchStopped()
			//
			//	This function is called when a directory is unwatched (except on the case of the direcotry being unwatched due to an error)
			//
			//	WARNING:  *** READ THIS *** READ THIS *** READ THIS *** READ THIS ***
			//
			//	This function MAY be called before the destructor of CDirectoryChangeWatcher 
			//	finishes.  
			//
			//	Be careful if your implementation of this fuction
			//	interacts with some sort of a window handle or other object(a class, a file, etc.).  
			//	It's possible that that object/window handle will NOT be valid anymore the very last time
			//	that On_WatchStopped() is called.  
			//	This scenario is likely if the CDirectoryChangeWatcher instance is currently watching a
			//	directory, and it's destructor is called some time AFTER these objects/windows
			//	your change handler interacts with have been destroyed.
			//	
			//	If your CDirectoryChangeHandler derived class interacts w/ a window or other
			//	object, it's a good idea to unwatch any directories before the object/window is destroyed.
			//	Otherwise, place tests for valid objects/windows in the implementation of this function.
			//
			//  Failure to follow either tip can result in a mysterious RTFM error, or a 'Run time errors'
			//

//
	//
	//	End Override these functions (ie: don't worry about the rest of this class)
	//
	
	void SetChangedDirectoryName(const std::wstring & strChangedDirName);//please don't use this function, it will be removed in future releases.
	
private:
	long m_nRefCnt;
	
	std::wstring m_strChangedDirectoryName;//will be removed in a future release.

	friend class CDirectoryChangeWatcher;
	friend class CDelayedDirectoryChangeHandler;
	//
	//	This class keeps a reference to the CDirectoryChangeHandler 
	//	that was used when an object of this type is passed 
	//	to CDirectoryChangeWatcher::WatchDirectory().
	//
	//	This way, when the CDirectoryChangeWatcher object is destroyed(or if CDirectoryChangeHandler::UnwatchDirectory() is called)
	//	AFTER CDirectoryChangeWatcher::UnwatchDirecotry() or CDirectoryChangeWatcher::UnwatchAllDirectories() is called 
	//	the directory(or direcotries) that this 
	//	CDirectoryChangeWatcher object is handling will be automatically unwatched
	//	If the CDirectoryChangeWatcher object is destroyed before the CDirectoryChangeHandler objects 
	//	that are being used with that watcher are destroyed, the reference counting prevents
	//	this class from referencing a destructed object.
	//	Basically, neither class needs to worry about the lifetime of the other(CDirectoryChangeWatcher && CDirectoryChangeHandler)
	//

	long  ReferencesWatcher(CDirectoryChangeWatcher * pDirChangeWatcher);
	long  ReleaseReferenceToWatcher(CDirectoryChangeWatcher * pDirChangeWatcher);
	CDirectoryChangeWatcher * m_pDirChangeWatcher;
	long   m_nWatcherRefCnt; //<-- when this reaches 0, m_pDirChangeWatcher is set to NULL
	CCriticalSection m_csWatcher;
};

///////////////////////////////////////////////////////////

class CDirectoryChangeWatcher  
/***************************************
	A class to monitor a directory for changes made to files in it, or it's subfolders.
	The class CDirectoryChangeHandler handles the changes. You derive a class from CDirectoryChangeHandler to handle them.


	This class uses the Win32 API ReadDirectoryChangesW() to watch a directory for file changes.

	Multiple directories can be watched simultaneously using a single instance of CDirectoryChangeWatcher.
	Single or multiple instances of CDirectoryChangeHandler object(s) can be used to handle changes to watched directories.
	Directories can be added and removed from the watch dynamically at run time without destroying 
	the CDirectoryChangeWatcher object (or CDirectoryChangeHandler object(s).

	This class uses a worker thread, an io completion port, and ReadDirectoryChangesW() to monitor changes to a direcory (or subdirectories).
	There will always only be a single thread no matter how many directories are being watched(per instance of CDirectoryChangeHandler)

	THREAD ISSUES:
    This class uses worker threads.
	Notifications (calling CDirectoryChangeHandler's virtual functions) are executed 
	in the context of either the main thread, OR in a worker thread.

	The 'main' thread is the thread that first calls CDirectoryChangeWatcher::WatchDirectory().
	For notifications to execute in the main thread, it's required that the calling thread(usually the main thread) 
	has a message pump in order to process the notifications.

	For applications w/out a message pump, notifications are executed in the context of a worker thread.

	See the constructor for CDirectoryChangeWatcher.

  
****************************************/
{
public:
	//ctor/dtor
	CDirectoryChangeWatcher();//see comments in ctor .cpp file.
	virtual ~CDirectoryChangeWatcher();

	//
	//	Starts a watch on a directory:
	//
	DWORD	WatchDirectory(const std::wstring & strDirToWatch, 
						   DWORD dwChangesToWatchFor, 
						   CDirectoryChangeHandler * pChangeHandler, 
						   BOOL bWatchSubDirs = FALSE,
						   LPCTSTR szIncludeFilter = NULL,
						   LPCTSTR szExcludeFilter = NULL);

	BOOL	IsWatchingDirectory (const std::wstring & strDirName)const;
	int		NumWatchedDirectories()const; //counts # of directories being watched.

	
	BOOL	UnwatchDirectory(const std::wstring & strDirToStopWatching);//stops watching specified directory.
	BOOL	UnwatchAllDirectories();//stops watching ALL watched directories.

protected:
	
	virtual void On_ThreadInitialize(){}//All file change notifications has taken place in the context of a worker thread...do any thread initialization here..
	virtual void On_ThreadExit(){}//do thread cleanup here
	
private:
	friend class CDirectoryChangeHandler;
	BOOL UnwatchDirectory(CDirectoryChangeHandler * pChangeHandler);//called in CDirectoryChangeHandler::~CDirectoryChangeHandler()


	void static MonitorDirectoryChanges(LPVOID lpvThis );//the worker thread that monitors directories.

public:
	class CDirWatchInfo;

private:
	void BailOutBecauseOfError(CDirWatchInfo * pdi);



public:
	class CDirWatchInfo 
		//this class is used internally by CDirectoryChangeWatcher
		//to help manage the watched directories
	{
	private:
		CDirWatchInfo();		//private & not implemented
		CDirWatchInfo & operator=(const CDirWatchInfo & rhs);//so that they're aren't accidentally used. -- you'll get a linker error
	public:
		CDirWatchInfo(HANDLE hDir, const std::wstring & strDirectoryName, 
					  CDirectoryChangeHandler * pChangeHandler, 
					  DWORD dwChangeFilter, BOOL bWatchSubDir
					  );
	private:
		~CDirWatchInfo( );//only I can delete myself....use DeleteSelf()
	public:
		void DeleteSelf(CDirectoryChangeWatcher * pWatcher);
		
		DWORD StartMonitor(HANDLE hCompPort);
		BOOL UnwatchDirectory( HANDLE hCompPort );
	protected:
		BOOL SignalShutdown( HANDLE hCompPort );
		BOOL WaitForShutdown();			   
	public:
		BOOL LockProperties() { return m_cs.Lock(); }
		BOOL UnlockProperties(){ return m_cs.Unlock();	}

		CDelayedDirectoryChangeHandler* GetChangeHandler() const;
		CDirectoryChangeHandler * GetRealChangeHandler() const;//the 'real' change handler is your CDirectoryChangeHandler derived class.
		CDirectoryChangeHandler * SetRealDirectoryChangeHandler(CDirectoryChangeHandler * pChangeHandler);
		
		BOOL CloseDirectoryHandle();
		
		//CDirectoryChangeHandler * m_pChangeHandler;
		CDelayedDirectoryChangeHandler * m_pChangeHandler;
		HANDLE      m_hDir;//handle to directory that we're watching
		DWORD		m_dwChangeFilter;
		BOOL		m_bWatchSubDir;
		std::wstring     m_strDirName;//name of the directory that we're watching
		CHAR        m_Buffer[ READ_DIR_CHANGE_BUFFER_SIZE ];//buffer for ReadDirectoryChangesW
		DWORD       m_dwBufLength;//length or returned data from ReadDirectoryChangesW -- ignored?...
		DWORD		m_dwBufReadOffset;
		OVERLAPPED  m_Overlapped;
		DWORD		m_dwReadDirError;//indicates the success of the call to ReadDirectoryChanges()
		CCriticalSection m_cs;
		HANDLE		m_StartStopEvent;
		enum eRunningState{
			 RUNNING_STATE_NOT_SET, RUNNING_STATE_START_MONITORING, RUNNING_STATE_STOP, RUNNING_STATE_STOP_STEP2,
			  RUNNING_STATE_STOPPED, RUNNING_STATE_NORMAL
			 };
		eRunningState m_RunningState;
		};//end nested class CDirWatchInfo 

	void ProcessChangeNotifications(IN CFileNotifyInformation & notify_info, 
									IN CDirWatchInfo * pdi,
									OUT DWORD & ref_dwReadBuffer_Offset);
	friend class CDirWatchInfo;//so that CDirWatchInfo can call the following function.
	long ReleaseReferenceToWatcher(CDirectoryChangeHandler * pChangeHandler);

	BOOL	UnwatchDirectoryBecauseOfError(CDirWatchInfo * pWatchInfo);//called in case of error.
	int		AddToWatchInfo(CDirWatchInfo * pWatchInfo);
	//
	//	functions for retrieving the directory watch info based on different parameters
	//
	CDirWatchInfo *	GetDirWatchInfo(IN const std::wstring & strDirName, OUT int & ref_nIdx)const;
	CDirWatchInfo *	GetDirWatchInfo(IN CDirWatchInfo * pWatchInfo, OUT int & ref_nIdx)const;
	CDirWatchInfo * GetDirWatchInfo(IN CDirectoryChangeHandler * pChangeHandler, OUT int & ref_nIdx)const;
	

	HANDLE m_hCompPort;	//i/o completion port
	sd::AutoPtr<OSThread> m_hThread;	//MonitorDirectoryChanges() thread handle
	std::vector<CDirWatchInfo*> m_DirectoriesToWatch; //holds info about the directories that we're watching.
	CCriticalSection m_csDirWatchInfo;

	DWORD	m_dwFilterFlags;//options for determining the behavior of the filter tests.
	
};


#endif // !defined(AFX_DIRECTORYCHANGES_H__02E53FDE_CB22_4176_B6D7_DA3675D9F1A6__INCLUDED_)
