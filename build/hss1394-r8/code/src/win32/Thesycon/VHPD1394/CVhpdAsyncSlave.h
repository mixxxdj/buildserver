/************************************************************************
 *
 *  Module:       CVhpdAsyncSlave.h
 *  Long name:    CVhpdAsyncSlave class
 *  Description:  CVhpdAsyncSlave class definition
 *
 *  Runtime Env.: Win32, Part of VhpdLib
 *  Author(s):    Frank Senf, Udo Eberhardt
 *  Company:      Thesycon GmbH, Ilmenau
 ************************************************************************/

#ifndef __CVHPDASYNCSLAVE_H__
#define __CVHPDASYNCSLAVE_H__

// get base class definition
#include "CVhpd.h"
#include "CVhpdThread.h"



//
// CVhpdAsyncSlave
//
// This class implements basic functions for access handling to the local
// address space.
// Worker thread and data processing has to be implemented by derived classes.
//
//
class CVhpdAsyncSlave : public CVhpd
{
public:
  // standard constructor
  CVhpdAsyncSlave();
  // destructor, should always be declared as virtual
  virtual ~CVhpdAsyncSlave();

  // initialize this instance 
  DWORD SetupSlave(
          VHPD_ADDR_RANGE_MODE Mode,
          VHPD_UINT64 StartAddress,
          DWORD Length,
          VHPD_UINT64 TriggerAddress,
          DWORD AllowedAccess,
          DWORD RequestedNotification,
          DWORD NumberOfBuffers,
          DWORD SizeOfBuffer);

	 // de-initialize this instance
  void DeleteSlave(void);

  // get pointer to embedded buffer pool
  CVhpdBufPool* BufferPool(void)
      { return &mBufPool; }


  // start address of associated address range
  VHPD_UINT64 mStartAddress;
  // length of associated address range
  ULONG mRangeLength;
  // access to this address completes the buffer
  // (only used for TX and RX slave)
  VHPD_UINT64 mTriggerAddress;
  // types of access allowed
  DWORD mAllowedAccess;
  // types of notification requested
  DWORD mRequestedNotification;

  // number of buffers in associated pool
  DWORD mNumberOfBuffers;
  // buffer size
  DWORD mSizeOfBuffer;

  // error counter
  DWORD mErrorCounter;
  DWORD mMaxErrorCount;


// implementation
protected:      

  // associated buffer pool
  CVhpdBufPool mBufPool;

  // list of pending buffers
  CVhpdBuf* mFirstPending;
  CVhpdBuf* mLastPending;


};  // class CVhpdAsyncSlave

#endif // __CVHPDASYNCSLAVE_H__
 
/*************************** EOF **************************************/
