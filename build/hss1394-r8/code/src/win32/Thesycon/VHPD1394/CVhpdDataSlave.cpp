/************************************************************************
 *
 *  Module:       CVhpdDataSlave.cpp
 *  Long name:    CVhpdDataSlave class
 *  Description:  CVhpdDataSlave class implementation
 *
 *  Runtime Env.: Win32, Part of VhpdLib
 *  Author(s):    Frank Senf, Udo Eberhardt
 *  Company:      Thesycon GmbH, Ilmenau
 ************************************************************************/

#include "CVhpdDataSlave.h"

// unicode is not supported by VHPDLIB
#ifdef UNICODE
#undef UNICODE
#endif


// standard constructor
CVhpdDataSlave::CVhpdDataSlave()
{
  // no members to initialize
}


// destructor
CVhpdDataSlave::~CVhpdDataSlave()
{
  Delete();
}


DWORD CVhpdDataSlave::Setup(
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
            BufferQueueMode,
            StartAddress,
            Length,
            TriggerAddress,
            AllowedAccess,
            RequestedNotification,
            NumberOfBuffers,
            SizeOfBuffer);
}


void CVhpdDataSlave::Delete(void)
{
  // call base class CVhpdAsyncSlave
  CVhpdAsyncSlave::DeleteSlave();
}


// thread main routine
void CVhpdDataSlave::ThreadRoutine(void)
{
  CVhpdBuf* Buf;
  BOOL cont;

  if ( mBufPool.CurrentCount() == 0 ) {
    // no buffers
    return;
  }

  // set some members of base class
  mFirstPending = NULL;
  mLastPending = NULL;
  mErrorCounter = 0;

  cont = TRUE;

  // thread main loop
  for (;;) {
    
    if ( mTerminateFlag ) {
      // thread should be terminated
      break;
    }

    // submit all available buffers to the address range object
    while ( (Buf=mBufPool.Get()) != NULL ) {
      // number of bytes to transfer
      Buf->mNumberOfBytesToTransfer = Buf->Size(); 
      // pre-process buffer (e.g. to fill buffer with data in order to be read)
      cont = PreProcessBuffer(Buf);
      if ( cont ) {
        // submit buffer 
        AddressRangeSubmitBuffer(Buf);
        // chain buffer to pending list
        if ( mFirstPending == NULL ) {
          mFirstPending = Buf;
        } else {
          mLastPending->mNext = Buf;  //lint !e613
        }
        mLastPending = Buf;
        Buf->mNext = NULL;
      } else {
        // derived class want's to stop data transfer
        // put buffer back to pool, stop submitting buffers
        mBufPool.Put(Buf);
        break;
      }
    }

    if ( mFirstPending==NULL ) {
      // no pending request, break main loop
      break;
    }

    if ( !cont ) {
      // worker thread should stop, leave main loop
      break;
    }

    // wait for completion on the first pending buffer
    Buf = mFirstPending;
    WaitForCompletion(Buf);// INFINITE
    // call virtual function to detect errors and
    // process data (e.g. store received data)
    cont = PostProcessBuffer(Buf);

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
void CVhpdDataSlave::TerminateThread(void)
{
  // disable address range
  // cancel all pending data buffers
  AbortIoBuffers();

}


//
// default implementation of virtual functions
// NOTE: called in the context of the worker thread
//
BOOL CVhpdDataSlave::PreProcessBuffer(CVhpdBuf* /*Buf*/)
{
  return TRUE;

}


BOOL CVhpdDataSlave::PostProcessBuffer(CVhpdBuf* /*Buf*/)
{
  return TRUE;

}

/*************************** EOF **************************************/
