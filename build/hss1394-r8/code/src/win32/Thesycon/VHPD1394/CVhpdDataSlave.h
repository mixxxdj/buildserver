/************************************************************************
 *
 *  Module:       CVhpdDataSlave.h
 *  Long name:    CVhpdDataSlave class
 *  Description:  CVhpdDataSlave class definition
 *
 *  Runtime Env.: Win32, Part of VhpdLib
 *  Author(s):    Frank Senf, Udo Eberhardt
 *  Company:      Thesycon GmbH, Ilmenau
 ************************************************************************/

#ifndef __CVHPDDATASLAVE_H__
#define __CVHPDDATASLAVE_H__

// get base class definition
#include "CVhpdAsyncSlave.h"



//
// CVhpdDataSlave
//
// This class implements receiving data written to the local address space
// or transmitting data read from the local address space by
// other nodes on the IEEE 1394 bus using a worker thread.
// Data processing is implemented by a derived class.
//
//
class CVhpdDataSlave : public CVhpdAsyncSlave, public CVhpdThread
{
public:
  // standard constructor
  CVhpdDataSlave();
  // destructor, should always be declared as virtual
  virtual ~CVhpdDataSlave();

  // initialize instance (set mode and call base class)
  DWORD Setup(
          VHPD_UINT64 StartAddress,
          DWORD Length,
          VHPD_UINT64 TriggerAddress,
          DWORD AllowedAccess,
          DWORD RequestedNotification,
          DWORD NumberOfBuffers,
          DWORD SizeOfBuffer);

  // de-initialize instance
  void Delete(void);


// implementation
protected:      

  // this function is called with each buffer before it is submitted
  // it can be overloaded by derived classes
  // returns FALSE if buffer processing should be stopped, TRUE to continue
  virtual BOOL PreProcessBuffer(CVhpdBuf* Buf);

  // function that is called after a buffer is completed by the driver
  // it can be overloaded by derived classes
  // returns FALSE if buffer processing should be stopped, TRUE to continue
  // this function should implement error detection
  virtual BOOL PostProcessBuffer(CVhpdBuf* Buf);

  // overloaded functions from base class
  virtual void ThreadRoutine(void);

  virtual void TerminateThread(void);


};  // class CVhpdDataSlave


#endif // __CVHPDDATASLAVE_H__
 
/*************************** EOF **************************************/
