/************************************************************************
 *
 *  Module:       CVhpdNotifySlave.cpp
 *  Long name:    CVhpdNotifySlave class
 *  Description:  CVhpdNotifySlave class implementation
 *
 *  Runtime Env.: Win32, Part of VhpdLib
 *  Author(s):    Frank Senf, Udo Eberhardt
 *  Company:      Thesycon GmbH, Ilmenau
 ************************************************************************/

#include "CVhpdNotifySlave.h"

// unicode is not supported by VHPDLIB
#ifdef UNICODE
#undef UNICODE
#endif


// standard constructor
CVhpdNotifySlave::CVhpdNotifySlave()
{
}


// destructor
CVhpdNotifySlave::~CVhpdNotifySlave()
{
  Delete();
}



DWORD CVhpdNotifySlave::Setup(
        VHPD_UINT64 StartAddress,
        DWORD Length,
        VHPD_UINT64 TriggerAddress,
        DWORD AllowedAccess,
        DWORD RequestedNotification,
        DWORD NumberOfBuffers,
        DWORD SizeOfBuffer
        )
{
  // call base class CVhpdAsyncSlave
  return CVhpdAsyncSlave::SetupSlave(
            BufferStoreMode,
            StartAddress,
            Length,
            TriggerAddress,
            AllowedAccess,
            RequestedNotification,
            NumberOfBuffers,
            SizeOfBuffer);
}


DWORD CVhpdNotifySlave::SetupFifoMode(
        VHPD_UINT64 StartAddress,
        DWORD Length,
        DWORD NumberOfBuffers,
        DWORD SizeOfBuffer
        )
{
	// in FIFO mode, we have to use WRITE access and WRITE notifications

  // call base class CVhpdAsyncSlave
  return CVhpdAsyncSlave::SetupSlave(
            FifoStoreMode,
            StartAddress,
            Length,
            StartAddress,	//TriggerAddress is unused in FIFO mode
            VHPD_FLAG_ADRRNG_ACCESS_WRITE,
            VHPD_FLAG_ADRRNG_NOTIFY_WRITE,
            NumberOfBuffers,
            SizeOfBuffer);
}


void CVhpdNotifySlave::Delete(void)
{
  // call base class CVhpdAsyncSlave
  CVhpdAsyncSlave::DeleteSlave();
}


// thread main routine
void CVhpdNotifySlave::ThreadRoutine(void)
{
  CVhpdBuf* Buf;
  DWORD Status;
  BOOL cont;

  if ( mBufPool.CurrentCount() == 0 ) {
    // no buffers
    return;
  }

  mFirstPending = NULL;
  mLastPending = NULL;
  cont = TRUE;
  mErrorCounter = 0;

  // thread main loop
  for (;;) {
    
    if ( mTerminateFlag ) {
      // thread should be terminated
      break;
    }

    // submit all available notification buffers to the address range object
    while ( (Buf=mBufPool.Get()) != NULL ) {
      // number of bytes to transfer
      Buf->mNumberOfBytesToTransfer = Buf->Size(); 
      // submit buffer (asynchronous)
      GetAddressRangeNotification(Buf);
      // chain buffer to pending list
      if ( mFirstPending == NULL ) {
        mFirstPending = Buf;
      } else {
        mLastPending->mNext = Buf;  //lint !e613
      }
      mLastPending = Buf;
      Buf->mNext = NULL;
    }

    if ( mFirstPending==NULL ) {
      // no pending request, break main loop
      break;
    }

    // wait for completion on the first pending buffer
    Buf = mFirstPending;
    Status = WaitForCompletion(Buf);// INFINITE
    if ( Status != VHPD_STATUS_SUCCESS ) {
      // completed with error
      mErrorCounter++;
      BufErrorHandler(Buf);
    } else {
      // completed with success
      // reset error counter
      mErrorCounter = 0;
      // call virtual function to process notification
      cont = ProcessNotification(Buf);
    }

    // remove buffer from pending list
    mFirstPending = Buf->mNext;
    // put the buffer back to pool
    mBufPool.Put(Buf);

    // break loop if max error count reached
    if ( (mMaxErrorCount!=0) && (mErrorCounter>=mMaxErrorCount) ) {
      break;
    }

    if ( !cont ) {
      // break main loop
      break;
    }

  } // for

  // we have to call CancelThreadRequests() to abort all IO operations
  // issued by this worker thread
  // this is especially required in case the worker thread terminates itself
  CancelThreadRequests();

  // wait for all pending buffers to complete
  while ( (Buf=mFirstPending) != NULL ) {
    WaitForCompletion(Buf);// INFINITE
    mFirstPending = Buf->mNext;
    mBufPool.Put(Buf);
  }

}


// NOTE: called in the context of the main thread
void CVhpdNotifySlave::TerminateThread(void)
{
  // cancel all pending notification buffers
  AbortIoBuffers();
}


void CVhpdNotifySlave::BufErrorHandler(CVhpdBuf* /*Buf*/)
{
  // empty
}


/*************************** EOF **************************************/
