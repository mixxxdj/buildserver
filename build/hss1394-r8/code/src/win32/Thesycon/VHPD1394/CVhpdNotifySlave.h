/************************************************************************
 *
 *  Module:       CVhpdNotifySlave.h
 *  Long name:    CVhpdNotifySlave class
 *  Description:  CVhpdNotifySlave class definition
 *
 *  Runtime Env.: Win32, Part of VhpdLib
 *  Author(s):    Frank Senf, Udo Eberhardt
 *  Company:      Thesycon GmbH, Ilmenau
 ************************************************************************/

#ifndef __CVHPDNOTIFYSLAVE_H__
#define __CVHPDNOTIFYSLAVE_H__

// get base class definition
#include "CVhpdAsyncSlave.h"



//
// CVhpdNotifySlave
//
// This class implements generic access handling to the local address space
// using a worker thread.
//
//
class CVhpdNotifySlave : public CVhpdAsyncSlave, public CVhpdThread
{
public:
  // standard constructor
  CVhpdNotifySlave();
  // destructor, should always be declared as virtual
  virtual ~CVhpdNotifySlave();

  // initialize instance (set mode and call base class)
  DWORD Setup(
          VHPD_UINT64 StartAddress,
          DWORD Length,
          VHPD_UINT64 TriggerAddress,
          DWORD AllowedAccess,
          DWORD RequestedNotification,
          DWORD NumberOfBuffers,
          DWORD SizeOfBuffer);

	DWORD SetupFifoMode(
					VHPD_UINT64 StartAddress,
					DWORD Length,
					DWORD NumberOfBuffers,
					DWORD SizeOfBuffer
					);


  // de-initialize instance
  void Delete(void);

// implementation
protected:      

  // function that is called after a buffer is completed by the driver
  // has to be overloaded by derived classes
  // returns FALSE if notification processing should be stopped, TRUE to continue
  virtual BOOL ProcessNotification(CVhpdBuf* Buf) =0;
  // this function is called with each buffer that is completed with error
  // can be overloaded by derived classes
  virtual void BufErrorHandler(CVhpdBuf* Buf);

  // overloaded functions from base class CVhpdThread
  virtual void ThreadRoutine(void);

  virtual void TerminateThread(void);

};  // class CVhpdNotifySlave


#endif // __CVHPDNOTIFYSLAVE_H__
 
/*************************** EOF **************************************/
