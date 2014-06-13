/************************************************************************
 *
 *  Module:       CVhpdThread.cpp
 *  Long name:    CVhpdThread class
 *  Description:  Thread base class
 *
 *  Runtime Env.: Win32, Part of VhpdLib
 *  Author(s):    Frank Senf, Udo Eberhardt
 *  Company:      Thesycon GmbH, Ilmenau
 ************************************************************************/

// for shorter and faster windows.h
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#include <process.h>
#include "CVhpdThread.h"

// unicode is not supported by VHPDLIB
#ifdef UNICODE
#undef UNICODE
#endif



CVhpdThread::CVhpdThread()
{
  mThreadPriority = THREAD_PRIORITY_HIGHEST;
  mThreadHandle = NULL;
  mThreadID = 0;
  mTerminateFlag = FALSE;
  mParentWnd = NULL;
}


CVhpdThread::~CVhpdThread()
{
  // thread must have been terminated at this point
#ifdef _DEBUG
  if ( mThreadHandle != NULL ) {
    ::OutputDebugString(
      "CVhpdThread::~CVhpdThread(): Thread is still running! "
      "ShutdownThread() must be called before the object is destroyed!\n");
  }
#endif

}


// called in context of the main thread
BOOL CVhpdThread::StartThread(void)
{
  BOOL succ = FALSE;

  if ( mThreadHandle != NULL ) {
    // already started
  } else {
  
    mTerminateFlag = FALSE;

    // we use _beginthreadex, it is a wrapper for the Win32 API CreateThread
    mThreadHandle = (HANDLE)
      _beginthreadex( 
            NULL,                 // void *security,
            0,                    // unsigned stack_size, 
            beginthread_routine,  // unsigned ( __stdcall *start_address )( void * ),
            this,                 // void *arglist,
            0,                    // unsigned initflag,
            &mThreadID            // unsigned *thrdaddr
            );
    if ( mThreadHandle == NULL ) {
      // failed
    } else {
      // success
      succ = TRUE;
    }
  }

  return succ;
}


// called in context of the main thread
// blocks until the worker thread has been terminated
BOOL CVhpdThread::ShutdownThread(void)
{
  BOOL succ = TRUE;

  //Warning C4312 ('operation' : conversion from 'type1' to 'type2' of greater size)
  //apparently appears due to the fact that the warning system operates on the code 
  //after it has been preprocessed. So the warning system detects a 64-bit portability 
  //problem. We know this is no error and disable the warning for this piece of code. 
  #pragma warning( push )
  #pragma warning( disable : 4312 )
  HANDLE h = InterlockedExchangePointer( &mThreadHandle, 0 );
  #pragma warning( pop )

  if ( h != NULL ) {

    // tell thread to exit
    mTerminateFlag = TRUE;
    
    // loop until thread has terminated
    for (;;) {
      // call virtual member function
      // it should cause the thread to resume and to check mTerminateFlag
      TerminateThread();
      // wait on thread handle, 40 ms timeout
      DWORD err = WaitForSingleObject(h,40);
      if ( err==WAIT_OBJECT_0 ) {
        // handle is signaled, done
        break;
      }
      if ( err==WAIT_TIMEOUT ) {
        // we timed out, loop and wait again
        continue;
      }
      // error during wait
      succ = FALSE;
      break;
    }//for

    // close thread handle
    CloseHandle(h);
  }

  return succ;

}


// system thread routine
unsigned int __stdcall CVhpdThread::beginthread_routine(void *StartContext)
{
  CVhpdThread *This = (CVhpdThread*)StartContext;

  // adjust thread priority
  ::SetThreadPriority(::GetCurrentThread(),This->mThreadPriority);

  // call notification handler for thread entry
  This->OnThreadEntry();
  // call virtual member function
  This->ThreadRoutine();
  // call notification handler for thread exit
  This->OnThreadExit();

  // terminate the thread
  return 0;
}


//
// default implementation of virtual functions
// called in context of the worker thread
// 
void CVhpdThread::OnThreadEntry(void)
{
  // empty
}

void CVhpdThread::OnThreadExit(void)
{
#ifndef _WIN64_
  // send notification message to parent window
  if(mParentWnd != NULL) {
    ::PostMessage(mParentWnd,WM_USER_THREAD_TERMINATED,0,0);
  }
#endif
}

 
/*************************** EOF **************************************/
