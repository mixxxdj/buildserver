/************************************************************************
 *
 *  Module:       CVhpdAsyncSlave.cpp
 *  Long name:    CVhpdAsyncSlave class
 *  Description:  CVhpdAsyncSlave class implementation
 *
 *  Runtime Env.: Win32, Part of VhpdLib
 *  Author(s):    Frank Senf, Udo Eberhardt
 *  Company:      Thesycon GmbH, Ilmenau
 ************************************************************************/

#include "CVhpdAsyncSlave.h"

// unicode is not supported by VHPDLIB
#ifdef UNICODE
#undef UNICODE
#endif


// standard constructor
CVhpdAsyncSlave::CVhpdAsyncSlave()
{
  mNumberOfBuffers = 0;
  mSizeOfBuffer = 0;

  mStartAddress.QuadPart = 0;
  mRangeLength = 0;
  mTriggerAddress.QuadPart = 0;
  mAllowedAccess = 0;
  mRequestedNotification = 0;

  mFirstPending = NULL;
  mLastPending = NULL;

  mErrorCounter = 0;
  mMaxErrorCount = 5;
}


// destructor
CVhpdAsyncSlave::~CVhpdAsyncSlave()
{
  DeleteSlave();

} //lint !e1740


DWORD CVhpdAsyncSlave::SetupSlave(
          VHPD_ADDR_RANGE_MODE Mode,
          VHPD_UINT64 StartAddress,
          DWORD Length,
          VHPD_UINT64 TriggerAddress,
          DWORD AllowedAccess,
          DWORD RequestedNotification,
          DWORD NumberOfBuffers,
          DWORD SizeOfBuffer
        )
{
  VHPD_ALLOC_ADDR_RANGE AdrRng;
  DWORD Status;

  // allocate buffers
  if ( !mBufPool.Allocate(NumberOfBuffers, SizeOfBuffer) ) {
    return VHPD_STATUS_NO_MEMORY;
  }
  // save buffer pool parameters
  mNumberOfBuffers = NumberOfBuffers;
  mSizeOfBuffer = SizeOfBuffer;

  // allocate address range
  // setup input structure
  ZeroMemory(&AdrRng, sizeof(VHPD_ALLOC_ADDR_RANGE));
  AdrRng.Mode = Mode;
  AdrRng.Desc.StartAddress.QuadPart = StartAddress.QuadPart;
  AdrRng.Desc.Length = Length;
  AdrRng.TriggerAddress.QuadPart = TriggerAddress.QuadPart;
  AdrRng.AccessTypes = AllowedAccess;
  AdrRng.NotificationTypes = RequestedNotification;

  // call device driver through base class
  Status = AllocateAddressRange(&AdrRng);
  if ( Status != VHPD_STATUS_SUCCESS ) {
    mBufPool.Free();
  } else {
    // save address range parameters
    mStartAddress.QuadPart = StartAddress.QuadPart;
    mRangeLength = Length;
    mTriggerAddress.QuadPart = TriggerAddress.QuadPart;
    mAllowedAccess = AllowedAccess;
    mRequestedNotification = RequestedNotification;
  }

  return Status;
}


void CVhpdAsyncSlave::DeleteSlave(void)
{
  // delete address range object in device driver
  FreeAddressRange();
  // save address range parameters
  mStartAddress.QuadPart = 0;
  mRangeLength = 0;
  mTriggerAddress.QuadPart = 0;
  mAllowedAccess = 0;
  mRequestedNotification = 0;
  
  // free memory for associated buffer pool
  mBufPool.Free();
  // invalidate buffer pool parameters
  mNumberOfBuffers = 0;
  mSizeOfBuffer = 0;
}


/*************************** EOF **************************************/
