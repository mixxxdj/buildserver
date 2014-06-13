/************************************************************************
 *
 *  Module:       CVhpd.h
 *  Long name:    CVhpd class
 *  Description:  CVhpd base device class definition
 *
 *  Runtime Env.: Win32, Part of VhpdLib
 *  Author(s):    Frank Senf, Udo Eberhardt, Mario Guenther
 *  Company:      Thesycon GmbH, Ilmenau
 ************************************************************************/

#ifndef __CVHPD_H__
#define __CVHPD_H__

// for shorter and faster windows.h
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

// windows main header file
#include <windows.h>
// get IOCTL support
#include <winioctl.h>

// get setup API functions
#include "SetupApiDll.h"

// get driver interface definitions
#include "spec1394.h"
#include "vhpd1394.h"

#include "CVhpdBuf.h"

//
// define the calling convention used by VHPDLIB
//
#if (_MSC_VER >= 800)
#define _VHPD_LIB_CALL __stdcall
#else
#define _VHPD_LIB_CALL __cdecl
#endif


//
// CVhpd
//
// This class implements the interface to the vhpd1394.sys device driver.
// It contains all of the general functions that can be executed with any
// IEEE 1394 device.
//
class CVhpd
{
public:

  //
  // standard constructor
  //
  CVhpd();
  
  //
  // destructor, should always be declared as virtual
  //
  virtual ~CVhpd();

  //
  // Create a Windows-internal device list with all matching interfaces.
  // The device interface is identified by InterfaceGuid. A handle for the list
  // is returned in case of success, or NULL is returned in case of error.
  // You can iterate through the device list using Open(), see below.
  // The device list must be freed with DestroyDeviceList().
  //
  static HDEVINFO _VHPD_LIB_CALL
  CreateDeviceList(
    const GUID *InterfaceGuid);
  
  //
  // Use this function to destroy the device list generated with CreateDeviceList
  //
  static void _VHPD_LIB_CALL
  DestroyDeviceList(
    HDEVINFO DeviceList);

  //
  // Open an 1394 device
  // The device list provided in DeviceList must be build using CreateDeviceList() (see above).
  // The GUID that identifies the device interface must be provided in InterfaceGuid.
  // DeviceNumber is used to iterate through the device list. It should start with zero and
  // should be incremented after each call to Open(). If no more instances of the interface 
  // are available VHPD_STATUS_NO_SUCH_DEV_INSTANCE is returned.
  //
  DWORD 
  Open(
    int DeviceNumber,
    HDEVINFO DeviceList,
    const GUID *InterfaceGuid);

  //
  // Open a device instance for a bus device (host controller)
  // BusNumber specifies the bus to open
  // If successful, this CVhpd instance represents a host controller installed
  // in the system and therefore a single IEEE1394 bus.
  // Driver interface IOCTL codes (library functions) supported by a bus device:
  //    IOCTL_VHPD_BUS_RESET (ForceBusReset())
  // A CVhpd instance may either represent a IEEE1394 device or a bus device.
  // Use Close() to close the opened bus device.
  //
  DWORD 
  OpenBusDevice(
    int BusNumber);

  //
  // Close the device.
  //
  void
  Close(void);

  //
  // Returns TRUE if the class instance is attached to a device.
  //
  bool
  IsOpen(void) 
    { return (mFileHandle!=NULL); }

  //
  // return the handle to the vhpd1394 device driver used by the instance
  // or NULL of no handle is opened
  // 
  HANDLE
  GetHandle(void)
      { return mFileHandle; }

  //  
  // Returns a pointer to the pathname associated with this device instance.
  // The pointer is only temporarily valid and should not be stored for later use. 
  // It becomes invalid if the device is closed.
  // If no device is opened, the return value is NULL.
  //
  const char* 
  GetDevicePathName(void);
  
  //  
  // get driver info
  // API version, driver version, build number, ...
  //
  DWORD 
  GetDriverInfo(
    VHPD_DRIVER_INFO *DriverInfo);

  //
  // returns TRUE if a checked build (debug version) of the VHPD1394 driver was detected
  //
  BOOL 
  IsCheckedBuild(void)  { return mCheckedBuildDetected; }

  //
  // returns TRUE if a demo version of the VHPD1394 driver was detected
  //
  BOOL 
  IsDemoVersion(void) { return mDemoVersionDetected; }

  //
  // returns TRUE if the handle is associated with a virtual 1394 device
  //
  BOOL 
  IsVirtualDevice(void) { return mIsVirtualDevice; }

  //
  // returns TRUE if Thesycon's alternative 1394 bus driver T1394bus is used as the bus driver
  //
  BOOL 
  IsRunningOnT1394bus(void) { return mIsRunningOnT1394bus; }

  //
  // get device-related driver parameters
  //
  DWORD 
  GetDeviceParameters(
    VHPD_DEVICE_PARAMETERS *DevParam);

  //
  // set device-related driver parameters
  //
  DWORD 
  SetDeviceParameters(
    const VHPD_DEVICE_PARAMETERS *DevParam);

  //  
  // get value of rate counter associated with this handle
  // can be used to determine actual data transfer rate on this handle
  //
  DWORD 
  QueryRateCounter(
    VHPD_QUERY_RATE_COUNTER *RateCounter);

  //
  // get basic configuration information for actual device
  // this information is hold in the device´s configuration ROM
  //
  DWORD 
  GetDeviceInfoParams(
    VHPD_CONFIG_INFO_PARAMS* ConfigParams);

  //
  // get specific device information block
  //
  DWORD 
  GetDeviceInfoBlock(
    const VHPD_CONFIG_INFO_BLOCK_DESC* BlockDesc,
    unsigned char* BlockBuf,
    DWORD* BlockBufLength);

  //
  // receives the device vendor string in a caller-provided buffer
  // vendor string is contained in device's configuration ROM
  // returns the number of bytes copied to the buffer, 0 if an error occurred
  //
  DWORD
  GetDeviceVendorString(
    char* VendorString,
    DWORD BufLength);

  //
  // receives the device model string in a caller-provided buffer
  // model string is contained in device's configuration ROM
  // returns the number of bytes copied to the buffer, 0 if an error occurred
  //
  DWORD
  GetDeviceModelString(
    char* ModelString,
    DWORD BufLength);

  //
  // return unique ID for device
  // unique ID is a combination of vendor ID (3 byte)
  // and chip ID (5 byte)
  // byte order is the same as in configuration ROM
  //
  DWORD
  GetDeviceUniqueId(
    unsigned char UID[8]
    );


  //
  // set properties for requests to the actual device
  // the following properties can be set:
  //  max. speed for requests to the device
  //
  DWORD 
  SetDeviceXmitProperties(
    const VHPD_DEVICE_XMIT_PROPERTIES* XmitProps);

  //
  // get available configuration information for the local host controller
  // the following types of information are supported:
  //  host unique id, host general capabilities, host transfer capabilities
  //  host power supply parameters, host CONFIG ROM, 
  //  current speed map, current topology map
  //
  DWORD 
  GetLocalHostInfo(
    const VHPD_GET_LOCAL_HOST_INFO* HcInfParams,
    VHPD_LOCAL_HOST_INFO_DATA* HcInfData);

  //
  // set properties of the local host controller
  // the following properties can be set:
  //  gap count lower bound
  //  configuration ROM contents
  //
  DWORD 
  SetLocalHostProperties(
    const VHPD_LOCAL_HOST_PROPERTIES* HCProps,
    const void* Buffer =NULL,   // optional buffer, used to modify configuration ROM contents
    DWORD BufferSize =0
    );
  
  //
  // get the real IEEE1394 address (Busnumber:Nodenumber) for actual device
  // or for local host controller
  //
  DWORD 
  GetNodeAddress(
    VHPD_GET_ADDR_FROM_DO* GetNodeAdr);

  //
  // tell the device to initiate a bus reset
  //
  DWORD 
  ForceBusReset(
  const VHPD_BUS_RESET* BusResetParams);
  
  //
  // tell the driver to notify us in case of bus reset
  // This function blocks until IO call is completed.
  // The IO call returns VHPD_STATUS_BUS_RESET in case of Bus Reset
  // and cancelled status in case of cancel. Other return values should
  // not appear.
  // NOTE: This function never returns VHPD_STATUS_SUCCESS!
  //
  DWORD 
  WaitForResetNotification(void);
  
  //  
  // get the actual value of bus-reset generation from driver
  // GenerationCount parameter is used by async read/write/lock.
  // Vhpd1394.sys supports automatically monitoring GenerationCount
  // (driver configuration registry parameter 'MonitorReset'),
  // applications can decide to monitor this count manually by
  // installing a bus-reset notification, in this case the correct
  // GenerationCount has to be supplied with each read/write/lock call.
  //
  DWORD 
  GetResetGenerationCount(
    VHPD_GENERATION_COUNT* GenerationCount);
  
  //
  // send a PHY configuration packet
  // PHY configuration packets are broadcast packets and may be used
  // to make a special node the root node and to set a new value
  // for gap count parameter.
  //
  DWORD 
  SendPhyConfigPacket(
    const VHPD_SEND_PHY_CONFIG_PACKET* SendPCP);

  //
  // issue a write, read or lock transaction to the current device
  // These calls are always handled asynchronously.
  // These functions return TRUE if the request was successfully
  // submitted to the driver (even in case of request is pending).
  // The return value is FALSE if an error is occurred,
  // the error code is placed in the mStatus member of CVhpdBuf.
  //
  BOOL 
  AsyncWrite(
    const VHPD_ASYNC_WRITE* Write,
    CVhpdBuf* Buf);

  BOOL AsyncRead(
    const VHPD_ASYNC_READ* Read,
    CVhpdBuf* Buf);

  BOOL 
  AsyncLock(
    const VHPD_ASYNC_LOCK* Lock,
    CVhpdBuf* Buf);
  
  //
  // wait for completion of a write, read, or lock operation or an iso buffer
  // After submitting a buffer using one of the functions that are normally handled asynchronously
  // this function may be called with this buffer to wait for completion of the operation
  // WaitForCompletion can be called regardless of the return status
  // of the actual operation. It returns always the final
  // completion status of the operation.
  // Optionally a timeout interval for the wait may be specified.
  // A value of INFINITE means: no timeout (wait until the operation completes).
  // Note: After a status of VHPD_STATUS_TIMEOUT was returned, the operation
  // is still pending and WaitForCompletion can be called again.
  //
  DWORD
  WaitForCompletion(
    CVhpdBuf* Buf, 
    DWORD Timeout =INFINITE);

  //
  // get completion status of buffer
  // After submitting a buffer using one of the functions that are normally handled asynchronously
  // this function may be called with this buffer to query the current status
  // of the operation.
  // GetCompletionStatus can be called regardless of the return status
  // of the actual operation. It returns ERROR_IO_PENDING if the operation
  // is still in progress or the final completion status.
  // Note: After a status of ERROR_IO_PENDING was returned, the operation
  // is still pending and GetCompletionStatus can be called again.
  //
  DWORD
  GetCompletionStatus(
    CVhpdBuf* Buf);

  //
  // write, read, or lock the actual device using synchronous operation
  // The functions transfer data from or to the specified buffer by using
  // asynchronous transfer mode.
  // Optionally a timeout interval for the completion of the transfer may be specified.
  // A value of INFINITE means: no timeout (wait until the operation completes).
  // The AsynReadSync, AsyncWriteSync, and AsyncLockSync functions do not return to 
  // the caller until the data transfer is completed, with success or error, or a timeout
  // error occurs. In the case of a timeout the read or write operation
  // is aborted (cancelled) and the status VHPD_STATUS_WAIT_TIMEOUT is returned.
  //
  // NOTE: There is some overhead when using these sync. functions.
  // Every time the functions are called an event object will be created
  // internally (with CreateEvent) and destroyed again.
  //
  DWORD AsyncWriteSync(
    const VHPD_ASYNC_WRITE* Write,
    void* Buffer,
    DWORD BytesToWrite,
    DWORD* BytesTransferred =NULL,
    DWORD Timeout =INFINITE);

  DWORD 
  AsyncReadSync(
    const VHPD_ASYNC_READ* Read,
    void* Buffer,
    DWORD BytesToRead,
    DWORD* BytesTransferred =NULL,
    DWORD Timeout =INFINITE);

  DWORD AsyncLockSync(
    const VHPD_ASYNC_LOCK* Lock,
    VHPD_ASYNC_LOCK_DATA* LockData,
    DWORD Timeout =INFINITE);

  //
  // get the time elapsed for last performed read operation
  // NOTE: The last performed read operation has to be called with
  // VHPD_FLAG_PING_PACKET flag set. Otherwise, the value returned in
  // PingTime is invalid.
  //
  DWORD 
  GetPingTime(
    VHPD_PING_TIME* PingTime);

  //
  // allocate and setup address range
  // This call allocates necessary resources and set needed parameters
  // to handle access to the local host controller.
  // NOTE: This call does not enable access and notification
  // to/for allocated range.
  //
  DWORD 
  AllocateAddressRange(
    const VHPD_ALLOC_ADDR_RANGE* AllocAdrRng);

  //
  // free address range allocated by AllocateAddressRange()
  //
  DWORD 
  FreeAddressRange(void);

  //
  // enable access and notification to/for allocated range
  //
  DWORD 
  EnableAddressRange(void);

  //
  // disable access and notification to/for allocated range
  //
  DWORD
  DisableAddressRange(void);

  //
  // write data to the buffer associated with the previously allocated
  // address range
  //
  DWORD 
  AddressRangeWriteBuffer(
    const VHPD_ADRRNG_RW_DATA_BUFFER* AdrRngRwBuffer,
    CVhpdBuf* Buf);
  
  //
  // read data from the buffer associated with the previously allocated
  // address range
  //
  DWORD 
  AddressRangeReadBuffer(
    const VHPD_ADRRNG_RW_DATA_BUFFER* AdrRngRwBuffer,
    CVhpdBuf* Buf);

  //
  // setup for receiving notification if a previously allocated address range
  // is accessed
  // This call is always handled asynchronously.
  //
  BOOL 
  GetAddressRangeNotification(
    CVhpdBuf* Buf);


  //
  // query parameters of last access to the current address range
  //
  DWORD 
  GetAddressRangeLastAccess(
    VHPD_ADRRNG_LAST_ACCESS* LastAccess);
  
  //
  // submit a buffer to the driver in order to provide data which may
  // be read or to receive data which may be written from/to local address space
  // by other devices on the IEEE 1394 bus
  // This call is always handled asynchronously.
  // This function returns TRUE if the request was successfully
  // submitted to the driver (even in case of request is pending).
  // The return value is FALSE if an error is occurred,
  // the error code is placed in the mStatus member of CVhpdBuf.
  //
  BOOL 
  AddressRangeSubmitBuffer(
    CVhpdBuf* Buf);

  //
  // query actual cycle time of bus system
  //
  DWORD 
  IsoQueryCycleTime(
    VHPD_ISOCH_CYCLE_TIME* CycleTime);

  //
  // query isochronous resources currently available
  // NOTE: Values returned in Res depends on transfer speed. Therefore
  // this speed has to be supplied with each call.
  //
  DWORD 
  IsoQueryResources(
    VHPD_ISOCH_RESOURCES* Res);

  //
  // allocate isochronous bandwidth
  // Bandwidth to be allocated is in Bytes/sec. Value should be the maximum
  // bandwidth needed by isochronous transfers with current device
  //
  DWORD 
  IsoAllocBandwidth(
    VHPD_ISOCH_ALLOC_BW* AllocBw);

  //
  // free bandwidth which was previously allocated through this handle
  //
  DWORD
  IsoFreeBandwidth(void);

  //
  // adjust amount of bandwidth previously allocated through this handle
  //
  DWORD
  IsoSetBandwidth(
    const VHPD_ISOCH_SET_CHAN_BW* SetBw);

  //
  // allocate channel for transactions with the current device
  //
  DWORD 
  IsoAllocChannel(
    VHPD_ISOCH_ALLOC_CHAN* AllocChan);

  //
  // free channel which was previously allocated through this handle
  //
  DWORD
  IsoFreeChannel(void);

  //
  // allocate isochronous resources
  //
  DWORD 
  IsoAllocResources(
    const VHPD_ISOCH_ALLOC_RES* AllocRes);

  //
  // allocate isochronous resources (extended for multichannel receive)
  //
  DWORD 
  IsoAllocResourcesEx(
    const VHPD_ISOCH_ALLOC_RES_EX* AllocResEx);

  //
  // free isochronous resources which was previously allocated through this handle
  //
  DWORD 
  IsoFreeResources(void);

  //
  // submit data buffer for isochronous talk or listen operation
  // These calls are always handled asynchronously.
  // These functions return TRUE if the request was successfully
  // submitted to the driver (even in case of request is pending).
  // The return value is FALSE if an error is occurred,
  // the error code is placed in the mStatus member of CVhpdBuf.
  //
  BOOL IsoSubmitWriteBuffer(
    CVhpdBuf* Buf, 
    const VHPD_ISOCH_SUBMIT_BUFFER* Submit =NULL);
  
  BOOL IsoSubmitReadBuffer(
    CVhpdBuf* Buf, 
    const VHPD_ISOCH_SUBMIT_BUFFER* Submit =NULL);

  //
  // attach an application buffer to an isochronous channel object
  // allocate related resources
  // buffer will be shared between driver and application
  //
  DWORD
  IsoAttachSharedBuffer(
    const VHPD_ISOCH_ATTACH_SHARED_BUFFER* Attach);

  //
  // release sharing of application buffer
  // release associated resources
  //
  DWORD
  IsoDetachSharedBuffer(void);

  //
  // notify driver on completion of processing of shared buffer
  //
  DWORD
  IsoAcknowledgeSharedBuffer(void);

  //
  // start isochronous talk/listen with the current device
  //
  DWORD 
  IsoStartTalk(
    const VHPD_ISOCH_TALK_LISTEN* TalkListen =NULL);

  //
  // IsoStartListen
  //
  DWORD 
  IsoStartListen(
  const VHPD_ISOCH_TALK_LISTEN* TalkListen =NULL);

  //
  // stop isochronous data transfer with current device, associated
  // with this file handle
  //
  DWORD 
  IsoStop(void);

  //  
  // submit isochronous write or read buffer using synchronous operation
  // The functions transfer data from or to the specified buffer by using
  // isochronous transfer mode.
  // Optionally a timeout interval for the completion of the transfer may be specified.
  // A value of INFINITE means: no timeout (wait until the operation completes).
  // The IsoWriteSync and IsoReadSync functions do not return to the caller until 
  // the data transfer is completed, with success or error, or a timeout
  // error occurs. In the case of a timeout the read or write operation
  // is aborted (cancelled) and the status VHPD_STATUS_WAIT_TIMEOUT is returned.
  //
  // NOTE: There is some overhead when using these sync. functions.
  // Every time the functions are called an event object will be created
  // internally (with CreateEvent) and destroyed again.
  //
  DWORD IsoWriteSync(
    void* Buffer,
    DWORD BytesToWrite,
    DWORD* BytesTransferred =NULL,
    DWORD Timeout =INFINITE,
    const VHPD_ISOCH_SUBMIT_BUFFER* Submit =NULL);

  DWORD IsoReadSync(
    void* Buffer,
    DWORD BytesToRead,
    DWORD* BytesTransferred =NULL,
    DWORD Timeout =INFINITE,
    const VHPD_ISOCH_SUBMIT_BUFFER* Submit =NULL);

  //
  // call a driver IOCTL function and wait for completion
  //
  DWORD 
  IoctlSync(
    DWORD IoctlCode,
    const void* InBuffer,
    DWORD InBufferSize,
    void* OutBuffer,
    DWORD OutBufferSize,
    DWORD* BytesReturned);

  //
  // abort all currently pending IO operations initiated on current file handle
  // NOTE: This call cancels all IO operations submitted by this handle. The call is
  // not thread-specific.
  //
  DWORD 
  AbortIoBuffers(void);
  
  //
  // cancel all outstanding requests that were
  // issued by the calling thread on the file handle
  // Note: Requests issued on the handle by other threads are NOT cancelled.
  //
  BOOL 
  CancelThreadRequests(void);

  //
  // translate an error code into a readable string
  // The function prints into StringBuffer and returns a pointer to StringBuffer.
  //
  static
  char* _VHPD_LIB_CALL
  ErrorText(
    char* StringBuffer, 
    DWORD StringBufferSize, 
    DWORD ErrorCode);

//
// implementation
//
protected:      

  HANDLE mFileHandle;
  OVERLAPPED mOverlapped;
  CRITICAL_SECTION mCritSect;

  BOOL mCheckedBuildDetected;
  BOOL mDemoVersionDetected;
  BOOL mIsVirtualDevice;
  BOOL mIsRunningOnT1394bus;

private:
  SP_INTERFACE_DEVICE_DETAIL_DATA* mDevDetail;

protected:

  // The setupapi dll is loaded only once per process.
  // Therefore, we use a static member.
  static CSetupApiDll smSetupApi;

  unsigned int StringCopyUNICODEtoASCII(char* dst,const WCHAR* src,unsigned int maxcount );
  
};  // class CVhpd


#endif // __CVHPD_H__

 
/*************************** EOF **************************************/
