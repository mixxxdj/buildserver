/************************************************************************
 *
 *  Module:       CVhpdBuf.h
 *  Long name:    CVhpdBuf class
 *  Description:  definition of I/O buffer and buffer pool
 *
 *  Runtime Env.: Win32, Part of VhpdLib
 *  Author(s):    Frank Senf, Udo Eberhardt
 *  Company:      Thesycon GmbH, Ilmenau
 ************************************************************************/

#ifndef __CVHPDBUF_H__
#define __CVHPDBUF_H__

// for shorter and faster windows.h
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

// windows main header file
#include <windows.h>


//
// CVhpdBuf
//
// This class is used as a descriptor for data buffers processed
// by CVhpd and derived classes.
//
class CVhpdBuf
{
  friend class CVhpdBufPool;
  friend class CVhpd;

  // make copy constructor and assignment operator inaccessible
private:
  CVhpdBuf(const CVhpdBuf& );
  CVhpdBuf& operator=(const CVhpdBuf& );

public:
  // construct and do not associate a buffer.
  CVhpdBuf();
  // construct and associate existing buffer
  CVhpdBuf(void* Buf, DWORD BufferSize);
  // construct and allocate a buffer internally (using VirtualAlloc)
  // this buffer will be freed automatically on destruction
  CVhpdBuf(DWORD BufferSize);


  // destructor
  ~CVhpdBuf();

  // associate this descriptor with the specified memory block
  void SetBufferMemory(void* BufferMem, DWORD BufferSize);

  // get buffer start address (pointer to first byte of buffer)
  void* Buffer(void)  { return mBufferMem; }

  // get size of buffer in bytes
  DWORD Size(void)    { return mBufferSize; }

  // get pointer to embedded OVERLAPPED structure
  OVERLAPPED* Overlapped(void)    { return &mOverlapped; }

  // get handle of OVERLAPPED.hEvent event object
  HANDLE OverlappedEvent(void)    { return mOverlapped.hEvent; }

  // number of bytes to transfer from/to buffer
  // must be set *before* the buffer is submitted to the driver
  DWORD mNumberOfBytesToTransfer;

  // number of bytes transferred from/to buffer
  // will be set *after* the overlapped I/O operation was completed
  DWORD mBytesTransferred;

  // current status of the buffer or final status of the overlapped I/O operation
  // final status will be set *after* the overlapped I/O operation is completed
  DWORD mStatus;
  // context pointers that may be used by the current owner of the buffer
  // for any purpose
  void* mOwnerContext1;

  void* mOwnerContext2;

  // link pointer, may be used to build a chain of buffer objects
  CVhpdBuf* mNext;                


// implementation:
protected:

  // member initialization, must not be called if buffer memory is allocated
  void Init(void);

  void* mBufferMem;         // pointer to the buffer
  DWORD mBufferSize;        // size of the allocated buffer in bytes
  OVERLAPPED mOverlapped;   // overlapped struct used for asynch. IO
  BOOL mBufferMemAllocated; // TRUE: buffer was allocated by the CVhpdBuf

};  // class CVhpdBuf



//
// CVhpdBufPool
//
// This class is used to manage a pool of CVhpdBuf objects
// It provides functions to allocate an initial number of buffers, to get a
// buffer from the pool and to return a buffer to the pool
//
class CVhpdBufPool
{
public:
  // standard constructor
  CVhpdBufPool();
  // destructor
  ~CVhpdBufPool();

  // allocate buffer descriptors and associated buffer memory
  // If SizeOfBuffer is set to zero, only descriptors will be allocated.
  // returns TRUE for success, FALSE if a memory allocation fails.
  BOOL Allocate(DWORD NumberOfBuffers, DWORD SizeOfBuffer, BOOL AlignBuffers =TRUE);

  // free buffer descriptors and optionally the associated buffer memory
  void Free(void);

  // returns the current number of buffers in pool
  int CurrentCount(void);

  // get number of buffers allocated
  DWORD NumberOfBuffers(void)   { return mNumberOfBuffers; }

  // get size of single buffer, as allocated
  DWORD SizeOfBuffer(void)    { return mSizeOfBuffer; }

  // get a buffer from pool
  // returns NULL if the pool is empty
  CVhpdBuf* Get(void);

  // put a buffer back to pool
  void Put(CVhpdBuf* Buf);

// implementation:
protected:
  // head pointer of buffer chain
  CVhpdBuf* mHead;
  // current number of buffers in pool
  int mCount;

  // number of buffers allocated
  DWORD mNumberOfBuffers;
  // size of a single buffer
  // null if no buffer memory is allocated
  DWORD mSizeOfBuffer;

  // lock that synchronizes pool access
  CRITICAL_SECTION mCritSect;

  // pointer to allocated descriptors
  CVhpdBuf* mBufArray;
  // pointer to allocated buffer memory
  // NULL if no buffer memory was allocated
  unsigned char* mBufferMemory;

};  // class CVhpdBufPool

#endif // __CVHPDBUF_H__
 
/*************************** EOF **************************************/
