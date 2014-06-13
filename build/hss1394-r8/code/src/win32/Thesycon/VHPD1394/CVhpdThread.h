/************************************************************************
 *
 *  Module:       CVhpdThread.h
 *  Long name:    CVhpdThread class
 *  Description:  Thread base class
 *
 *  Runtime Env.: Win32, Part of VhpdLib
 *  Author(s):    Frank Senf, Udo Eberhardt
 *  Company:      Thesycon GmbH, Ilmenau
 ************************************************************************/

#ifndef __CVHPDTHREAD_H__
#define __CVHPDTHREAD_H__


#define WM_USER_THREAD_TERMINATED   (WM_USER+100)


//
// CVhpdThread
//
// This class implements basic functions of a worker thread.
//
class CVhpdThread
{
public:
  // standard constructor
  CVhpdThread();

  // destructor
  virtual ~CVhpdThread();

  // initialize handle to parent window
  // this window will receive a WM_USER_THREAD_TERMINATED message on thread termination
  HWND SetParentWnd(HWND hParent) {mParentWnd = hParent; return hParent;};
  
  // start the worker thread
  // returns TRUE for success, FALSE otherwise
  // thread priority is read from mThreadPriority member
  BOOL
  StartThread(void);
  
  // terminate the worker thread
  // blocks until the thread has been terminated by the operating system
  // returns TRUE for success, FALSE otherwise
  BOOL
  ShutdownThread(void);

  // This notification handler is called by the worker thread before
  // the thread enters the ThreadRoutine() function.
  // may be overloaded by a derived class
  // NOTE: It is called in the context of the worker thread.
  virtual
  void
  OnThreadEntry(void);

  // This notification handler is called by the worker thread before
  // the thread terminates itself.
  // may be overloaded by a derived class
  // NOTE: It is called in the context of the worker thread.
  virtual
  void
  OnThreadExit(void);


// implementation:
protected:

  // the main routine that is executed by the worker thread
  // has to be implemented by derived classes
  virtual
  void
  ThreadRoutine(void) = 0;

  // this routine is called to terminate the thread
  // called by ShutdownThread
  // has to be implemented by a derived classes
  virtual
  void
  TerminateThread(void) = 0;
  

  // system thread routine
  static unsigned int __stdcall beginthread_routine(void *StartContext);

// members
public:

  // priority of the thread
  int mThreadPriority;



protected:

  // thread handle, NULL when invalid
  HANDLE mThreadHandle;

  // thread id
  unsigned int mThreadID;

  // variables used by the thread routine
  volatile BOOL mTerminateFlag;

  // parent window
  HWND mParentWnd;

};  // class CVhpdThread


#endif // __CVHPDTHREAD_H__
 
/*************************** EOF **************************************/
