/************************************************************************
 *
 *  Module:       CVhpdBuf.cpp
 *  Long name:    CVhpdBuf class
 *  Description:  implementation of I/O buffer and buffer pool
 *
 *  Runtime Env.: Win32, Part of VhpdLib
 *  Author(s):    Frank Senf, Udo Eberhardt
 *  Company:      Thesycon GmbH, Ilmenau
 ************************************************************************/

#include "CVhpdBuf.h"

// unicode is not supported by VHPDLIB
#ifdef UNICODE
#undef UNICODE
#endif



//
// CVhpdBuf
//

CVhpdBuf::CVhpdBuf()
{
  Init();
} 
  

CVhpdBuf::CVhpdBuf(void* Buf, DWORD BufferSize)
{
  Init();
  mBufferMem = Buf;
  mBufferSize = BufferSize;
} 


CVhpdBuf::CVhpdBuf(DWORD BufferSize)
{
  Init();
  if ( BufferSize != 0 ) {
    mBufferMem = ::VirtualAlloc(
                    NULL,           // LPVOID lpAddress, region to reserve or commit
                    BufferSize,     // SIZE_T dwSize, size of region
                    MEM_COMMIT,     // DWORD flAllocationType, type of allocation
                    PAGE_READWRITE  // DWORD flProtect, type of access protection
                    );
    mBufferSize = BufferSize;
    mBufferMemAllocated = TRUE;
  }
} 


CVhpdBuf::~CVhpdBuf()
{
  if ( mOverlapped.hEvent != NULL ) {
    ::CloseHandle(mOverlapped.hEvent);
    mOverlapped.hEvent = NULL;
  }
  if ( mBufferMemAllocated && mBufferMem!=NULL ) {
    ::VirtualFree(
        mBufferMem,   // LPVOID lpAddress, address of region
        0,            // SIZE_T dwSize, size of region
        MEM_RELEASE   // DWORD dwFreeType, operation type
        );
    mBufferMemAllocated = FALSE;
    mBufferMem = NULL;
    mBufferSize = 0;
  }
}


// basic initialization
// NOTE: Must not be called if buffer memory is already allocated
void CVhpdBuf::Init(void)
{
  mNumberOfBytesToTransfer = 0;
  mBytesTransferred = 0;
  mStatus = 0;
  mNext = NULL;
  mOwnerContext1 = NULL;
  mOwnerContext2 = NULL;
  mBufferMemAllocated = FALSE;
  mBufferMem = NULL;  //lint !e672
  mBufferSize = 0;

  ZeroMemory(&mOverlapped,sizeof(mOverlapped));
  mOverlapped.hEvent=::CreateEvent(NULL ,FALSE ,FALSE ,NULL); 
} 


void CVhpdBuf::SetBufferMemory(void* BufferMem, DWORD BufferSize)
{ 
  if ( mBufferMemAllocated && mBufferMem!=NULL ) {
    delete[] (char*)mBufferMem;
    mBufferMemAllocated = FALSE;
    mBufferMem = NULL;
  }
  mBufferMem = BufferMem;   //lint !e672
  mBufferSize = BufferSize;
}



//
// CVhpdBufPool
//

CVhpdBufPool::CVhpdBufPool()
{
  mHead = NULL;
  mCount = 0;
  mNumberOfBuffers = 0;
  mSizeOfBuffer = 0;
  mBufArray = NULL;
  mBufferMemory = NULL;

  InitializeCriticalSection(&mCritSect);
}


CVhpdBufPool::~CVhpdBufPool()
{
  Free();
  DeleteCriticalSection(&mCritSect);

} //lint !e1740



BOOL CVhpdBufPool::Allocate(DWORD BufferCount, DWORD BufferSize, BOOL AlignBuffers /*=TRUE*/)
{
  DWORD i;
  CVhpdBuf *buf;
  DWORD PageSize;
  DWORD BufferSizeAfterAlignement;
  DWORD Size;
  BOOL succ = FALSE;

  EnterCriticalSection(&mCritSect);

  if ( mBufArray!=NULL ) {
    // is already allocated
  } else {
    
    if ( BufferCount==0 ) {
      // invalid parameter
    } else {

      // allocate array of buffer descriptors
      mBufArray = new CVhpdBuf[BufferCount];
      if ( mBufArray==NULL ) {
        // failed
      } else {
        
        if ( BufferSize==0 ) {
          // no buffer memory needed
          // success
          succ = TRUE;
        } else {
          if ( AlignBuffers ) {
            // align size of a buffer to a multiple of memory pages
            // each buffer start address will be aligned to a page boundary
            // retrieve the size of a single memory page
            SYSTEM_INFO SysInfo;
            ZeroMemory(&SysInfo, sizeof(SysInfo));
            GetSystemInfo(&SysInfo);
            PageSize = SysInfo.dwPageSize;
            // round up buffer size to a multiple of memory pages
            BufferSizeAfterAlignement = ((BufferSize+(PageSize-1))/PageSize) * PageSize;
          } else {
            // no alignement required
            BufferSizeAfterAlignement = BufferSize;
          }
          Size = BufferSizeAfterAlignement * BufferCount;
          mBufferMemory = (unsigned char*)
            ::VirtualAlloc(
                NULL,           // LPVOID lpAddress, region to reserve or commit
                Size,           // SIZE_T dwSize, size of region
                MEM_COMMIT,     // DWORD flAllocationType, type of allocation
                PAGE_READWRITE  // DWORD flProtect, type of access protection
                );
          if ( mBufferMemory==NULL ) {
            // failed
            delete[] mBufArray;
            mBufArray = NULL;
          } else {
            // set memory to zero
            ZeroMemory(mBufferMemory,Size);
            // init all buffer desc
            for (i=0;i<BufferCount;i++) {
              buf = &mBufArray[i];
              // start of buffer is always aligned to memory page
              buf->mBufferMem = mBufferMemory + (i*BufferSizeAfterAlignement);
              // usable size of a buffer is set to the one requested when calling Allocate()
              buf->mBufferSize = BufferSize;
            }
            // success
            succ = TRUE;
          }
        }
      }
    }
  }

  if ( succ ) {
    // save parameters
    mCount = BufferCount;
    mNumberOfBuffers = BufferCount;
    mSizeOfBuffer = BufferSize;
    // add all buffer desc to list
    for (i=0;i<BufferCount;i++) {
      buf = &mBufArray[i];  //lint !e613
      buf->mNext = mHead;
      mHead = buf;
    }
  }

  LeaveCriticalSection(&mCritSect);

  return succ;
}



void CVhpdBufPool::Free(void)
{
  EnterCriticalSection(&mCritSect);

  mHead = NULL;
  mCount = 0;

  if (mBufArray != NULL) {
    delete [] mBufArray;
    mBufArray=NULL;
  }

  if (mBufferMemory != NULL) {
    ::VirtualFree(
        mBufferMemory,  // LPVOID lpAddress, address of region
        0,              // SIZE_T dwSize, size of region
        MEM_RELEASE     // DWORD dwFreeType, operation type
        );
    mBufferMemory=NULL;
  }

  mNumberOfBuffers = 0;
  mSizeOfBuffer = 0;

  LeaveCriticalSection(&mCritSect);

} //lint !e1740


int CVhpdBufPool::CurrentCount(void)
{
  int count;

  EnterCriticalSection(&mCritSect);
  count = mCount;
  LeaveCriticalSection(&mCritSect);

  return count;
}


CVhpdBuf* CVhpdBufPool::Get(void)
{
  CVhpdBuf *buf;

  EnterCriticalSection(&mCritSect);

  buf = mHead;
  if (buf == NULL) {
    // pool is empty
  } else {
    // remove first element
    mHead = buf->mNext;
    buf->mNext = NULL;
    mCount--;
  }

  LeaveCriticalSection(&mCritSect);

  return buf;
}



void CVhpdBufPool::Put(CVhpdBuf* Buf)
{
  EnterCriticalSection(&mCritSect);

  Buf->mNext = mHead;
  mHead = Buf;
  mCount++;

  LeaveCriticalSection(&mCritSect);
}

 

/*************************** EOF **************************************/
