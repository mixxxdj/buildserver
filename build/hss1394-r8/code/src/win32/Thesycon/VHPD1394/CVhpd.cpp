/************************************************************************
 *
 *  Module:       CVhpd.cpp
 *  Long name:    CVhpd class
 *  Description:  CVhpd base device class implementation
 *
 *  Runtime Env.: Win32, Part of VhpdLib
 *  Author(s):    Frank Senf, Udo Eberhardt
 *  Company:      Thesycon GmbH, Ilmenau
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "CVhpd.h"

// unicode is not supported by VHPDLIB
#ifdef UNICODE
#undef UNICODE
#endif

// static members
CSetupApiDll CVhpd::smSetupApi;


// standard constructor
CVhpd::CVhpd()
{
  // initialize members
  mFileHandle = NULL;
  ZeroMemory(&mOverlapped,sizeof(mOverlapped));
  InitializeCriticalSection(&mCritSect);

  mCheckedBuildDetected = FALSE;
  mDemoVersionDetected = FALSE;
  mIsVirtualDevice = FALSE;
  mIsRunningOnT1394bus = FALSE;

  mDevDetail=NULL;
}


// destructor
CVhpd::~CVhpd()
{
  // close file handle, free mDevDetail
  Close();

  // free resources
  DeleteCriticalSection(&mCritSect);

} //lint !e1740




//static
HDEVINFO _VHPD_LIB_CALL
CVhpd::CreateDeviceList(const GUID *InterfaceGuid)
{
  HDEVINFO h;

  // make sure the setupapi dll is loaded
  if ( !smSetupApi.Load() ) {
    return NULL;
  }

  h = (smSetupApi.SetupDiGetClassDevs)(
        (GUID*)InterfaceGuid,                 // LPGUID ClassGuid, 
        NULL,                                 // PCTSTR Enumerator, 
        NULL,                                 // HWND hwndParent, 
        DIGCF_DEVICEINTERFACE | DIGCF_PRESENT // DWORD Flags
        );
  return ( (h==INVALID_HANDLE_VALUE) ? NULL : h );
}


//static
void _VHPD_LIB_CALL
CVhpd::DestroyDeviceList(HDEVINFO DeviceList)
{
  // make sure the setupapi dll is loaded
  if ( !smSetupApi.Load() ) {
    return;
  }

  if ( DeviceList!=NULL ) {
    (smSetupApi.SetupDiDestroyDeviceInfoList)(DeviceList);
  }
}



DWORD CVhpd::Open(int DeviceNumber, HDEVINFO DeviceList, const GUID *InterfaceGuid)
{
  DWORD Status;
  HANDLE h;
  char *Name;
  SP_DEVICE_INTERFACE_DATA  DevData;
  DWORD len;
  BOOL succ;

  if ( mFileHandle!=NULL ) {
    // this instance is already connected to the driver
    // we do not support successive Open() calls without an intermediate Close()
    return VHPD_STATUS_DEVICE_ALREADY_OPEN;
  }

  // check parameters
  if ( (DeviceList==NULL) || (InterfaceGuid==NULL) ) {
    return VHPD_STATUS_INVALID_FUNCTION_PARAM;
  }

  // make sure the setupapi dll is loaded
  if ( !smSetupApi.Load() ) {
    return VHPD_STATUS_LOAD_SETUP_API_FAILED;
  }

  // delete old detail data if any
  if ( mDevDetail!=NULL ) {
    delete[] (char*)mDevDetail;
    mDevDetail = NULL;
  }

  // enumerate the interface
  // get the device information for the given device number
  ZeroMemory(&DevData,sizeof(DevData));
  DevData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
  succ = (smSetupApi.SetupDiEnumDeviceInterfaces)(DeviceList, NULL, (GUID*)InterfaceGuid, DeviceNumber, &DevData );
  if (!succ) {
    Status = GetLastError();
    if ( Status==ERROR_NO_MORE_ITEMS ) {
      Status = VHPD_STATUS_NO_SUCH_DEV_INSTANCE;
    }
    return Status;
  }
  // get the length of the detailed information, allocate buffer
  (smSetupApi.SetupDiGetDeviceInterfaceDetail)(DeviceList, &DevData, NULL, 0, &len, NULL);
  mDevDetail = (SP_INTERFACE_DEVICE_DETAIL_DATA*) new char[len];  //lint !e433 !e826
  if ( mDevDetail==NULL ) {
    return VHPD_STATUS_MEM_ALLOC_FAILED;
  }
  // now get the  detailed device information
  ZeroMemory(mDevDetail,len);
  mDevDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
  succ = (smSetupApi.SetupDiGetDeviceInterfaceDetail)(DeviceList, &DevData, mDevDetail, len, &len, NULL);
  if (!succ) {
    return GetLastError();
  }
  // set device name
  Name = mDevDetail->DevicePath;

  // try to open the device driver
  h = ::CreateFile(
          Name,
          GENERIC_READ | GENERIC_WRITE,       // access mode
          FILE_SHARE_WRITE | FILE_SHARE_READ, // share mode
          NULL,                               // security desc.
          OPEN_EXISTING,                      // how to create
          FILE_FLAG_OVERLAPPED,               // file attributes
          NULL                                // template file
          );
  if ( h == INVALID_HANDLE_VALUE ) {
    Status = VHPD_STATUS_DEVICE_NOT_FOUND;
  } else {
    // save handle
    mFileHandle = h;

    // init the event (auto-reset type)
    mOverlapped.hEvent = ::CreateEvent(NULL ,FALSE ,FALSE ,NULL); 
    if ( mOverlapped.hEvent == NULL ) {
      Status = VHPD_STATUS_NO_MEMORY;
      Close();
    } else {

      // now get version info
      VHPD_DRIVER_INFO info;
      Status = GetDriverInfo(&info);
      if ( Status != VHPD_STATUS_SUCCESS ) {
        // failed
        Close();
      } else {
        mCheckedBuildDetected = (info.Flags&VHPD_INFOFLAG_CHECKED_BUILD) ? TRUE : FALSE;
        mDemoVersionDetected = (info.Flags&VHPD_INFOFLAG_DEMO_VERSION) ? TRUE : FALSE;

        // check the API version
        // major version currently used must match to the driver installed
        // minor version currently used must be equal or lower than the version of the driver installed
        // if changes are made at API level that makes the new API incompatible to the previous
        // version the major number will change
        if ( ((info.IFVersion>>8) != VHPD_IF_VERSION_MJ) ||
             ((info.IFVersion&0xFF) < VHPD_IF_VERSION_MN) ) {
          // wrong version
          Status = VHPD_STATUS_VERSION_MISMATCH;
          Close();
        } else {
          // ok, determine the type of device, virtual or real
          VHPD_DEVICE_PARAMETERS dev_params;
          Status = GetDeviceParameters(&dev_params);
          if ( Status != VHPD_STATUS_SUCCESS ) {
            // failed
            Close();
          } else {
            mIsVirtualDevice = (dev_params.Flags&VHPD_FLAG_VIRTUAL_DEVICE) ? TRUE : FALSE;
            mIsRunningOnT1394bus = (dev_params.Flags&VHPD_FLAG_ON_T1394BUS) ? TRUE : FALSE;
          }
        }
      }
    }
  }
  
  return Status;
}



DWORD CVhpd::OpenBusDevice(int BusNumber)
{
  DWORD Status;
  HANDLE h;

  if ( mFileHandle!=NULL ) {
    // this instance is already connected to the driver
    // we do not support successive Open() calls without an intermediate Close()
    return VHPD_STATUS_DEVICE_ALREADY_OPEN;
  }

  // build the device name
  // NOTE: Bus devices are named devices and do not register a device interface,
  //  so we do not use the enumeration of the device interface (recommended for WDM)
  //  but opens the device directly using it's name.
  //  If a host controller (bus) is removed from system we don't get informed on this event
  //  but instead of this further calls with the device handle will fail.
  char BusDevName[120];
  ::sprintf(BusDevName,"\\\\.\\VHPD1394BusDevice%d",BusNumber);
  char* Name = BusDevName;

  // try to open the device driver
  h = ::CreateFile(
          Name,
          GENERIC_READ | GENERIC_WRITE,       // access mode
          FILE_SHARE_WRITE | FILE_SHARE_READ, // share mode
          NULL,                               // security desc.
          OPEN_EXISTING,                      // how to create
          FILE_FLAG_OVERLAPPED,               // file attributes
          NULL                                // template file
          );
  if ( h == INVALID_HANDLE_VALUE ) {
    Status = VHPD_STATUS_DEVICE_NOT_FOUND;
  } else {
    // save handle
    mFileHandle = h;

    // init the event (auto-reset type)
    mOverlapped.hEvent = ::CreateEvent(NULL ,FALSE ,FALSE ,NULL); 
    if ( mOverlapped.hEvent == NULL ) {
      Status = VHPD_STATUS_NO_MEMORY;
      Close();
    } else {

      // now get version info
      VHPD_DRIVER_INFO info;
      Status = GetDriverInfo(&info);
      if ( Status != VHPD_STATUS_SUCCESS ) {
        // failed
        Close();
      } else {
        mCheckedBuildDetected = (info.Flags&VHPD_INFOFLAG_CHECKED_BUILD) ? TRUE : FALSE;
        mDemoVersionDetected = (info.Flags&VHPD_INFOFLAG_DEMO_VERSION) ? TRUE : FALSE;

        // check the API version
        // major version currently used must match to the driver installed
        // minor version currently used must be equal or lower than the version of the driver installed
        // if changes are made at API level that makes the new API incompatible to the previous
        // version the major number will change
        if ( ((info.IFVersion>>8) != VHPD_IF_VERSION_MJ) ||
             ((info.IFVersion&0xFF) < VHPD_IF_VERSION_MN) ) {
          // wrong version
          Status = VHPD_STATUS_VERSION_MISMATCH;
          Close();
        } else {
          // ok
          Status = VHPD_STATUS_SUCCESS;
        }
      }
      
      // reset device type
      mIsVirtualDevice = FALSE;
      mIsRunningOnT1394bus = FALSE;
    }
  }

  return Status;
}


void CVhpd::Close(void)
{
  if ( mFileHandle != NULL ) {
    ::CloseHandle(mFileHandle);
    mFileHandle = NULL;
  }

  if ( mOverlapped.hEvent != NULL ) {
    ::CloseHandle(mOverlapped.hEvent);
    mOverlapped.hEvent = NULL;
  }

  // delete detail info if any
  if ( mDevDetail!=NULL ) {
    delete[] (char*)mDevDetail;
    mDevDetail = NULL;
  }

}



const char*
CVhpd::GetDevicePathName(void) {
  if ( mDevDetail!=NULL ) {
    return ( mDevDetail->DevicePath );
  } else {
    return NULL;
  }
}



DWORD CVhpd::GetDriverInfo(VHPD_DRIVER_INFO* DriverInfo)
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_VHPD_GET_DRIVER_INFO,
              NULL,
              0,
              DriverInfo,
              sizeof(VHPD_DRIVER_INFO),
              NULL
              );

  return Status;
}


DWORD CVhpd::GetDeviceParameters(VHPD_DEVICE_PARAMETERS* DevParam)
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_VHPD_GET_DEVICE_PARAMETERS,
              NULL,
              0,
              DevParam,
              sizeof(VHPD_DEVICE_PARAMETERS),
              NULL
              );

  return Status;
}


DWORD CVhpd::SetDeviceParameters(const VHPD_DEVICE_PARAMETERS* DevParam)
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_VHPD_SET_DEVICE_PARAMETERS,
              DevParam,
              sizeof(VHPD_DEVICE_PARAMETERS),
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CVhpd::QueryRateCounter(VHPD_QUERY_RATE_COUNTER* RateCounter)
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_VHPD_QUERY_RATE_COUNTER,
              NULL,
              0,
              RateCounter,
              sizeof(VHPD_QUERY_RATE_COUNTER),
              NULL
              );

  return Status;
}


DWORD CVhpd::GetDeviceInfoParams(VHPD_CONFIG_INFO_PARAMS* ConfigParams)
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_VHPD_GET_CONFIG_INFO_PARAMS,
              NULL,
              0,
              ConfigParams,
              sizeof(VHPD_CONFIG_INFO_PARAMS),
              NULL
              );

  return Status;
}


DWORD CVhpd::GetDeviceInfoBlock(const VHPD_CONFIG_INFO_BLOCK_DESC* BlockDesc,
                                unsigned char* BlockBuf,
                                DWORD* BlockBufLength
                                )
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_VHPD_GET_CONFIG_INFO_BLOCK,
              BlockDesc,
              sizeof(VHPD_CONFIG_INFO_BLOCK_DESC),
              BlockBuf,
              *BlockBufLength,
              BlockBufLength
              );

  return Status;
}


DWORD CVhpd::GetDeviceVendorString(char* VendorString, DWORD BufLength)
{
  // check buffer and size
  if ( (VendorString == NULL) || (BufLength < 1) ) {
    return 0;
  }

  // reset buffer
  ZeroMemory(VendorString, BufLength);

  // ask driver for vendor string size and contents
  VHPD_CONFIG_INFO_PARAMS params;
  ZeroMemory(&params,sizeof(params));

  DWORD Status = IoctlSync(
                  IOCTL_VHPD_GET_CONFIG_INFO_PARAMS,
                  NULL,
                  0,
                  &params,
                  sizeof(VHPD_CONFIG_INFO_PARAMS),
                  NULL
                  );
  if ( Status != VHPD_STATUS_SUCCESS ) {
    // error
    return 0;
  }
  if ( params.VendorLeafBufSize < 4 ) {
    // no or invalid vendor string available for this device
    return 0;
  }
  // vendor string available, allocate storage
  char* buf = new char[params.VendorLeafBufSize];
  if (buf == NULL) {
    return 0; 
  }

  // get vendor leaf
  VHPD_CONFIG_INFO_BLOCK_DESC desc;
  desc.BlockType = VendorLeaf;
  Status = IoctlSync(
              IOCTL_VHPD_GET_CONFIG_INFO_BLOCK,
              &desc,
              sizeof(VHPD_CONFIG_INFO_BLOCK_DESC),
              buf,
              params.VendorLeafBufSize,
              NULL
              );
  if ( Status != VHPD_STATUS_SUCCESS ) {
    // error
    delete[] buf;
    return 0;
  }
  // extract length in bytes
  unsigned long length = buf[0];
  length <<= 8;
  length |= buf[1]; // length in quadlets
  length *=4; // length in bytes
  length -= 8;  // length of raw string not including format specification
  // check length for plausibility
  if ( (length+12) > params.VendorLeafBufSize ) {
    // error
    delete[] buf;
    return 0;
  }

  // get language specifier id
  unsigned long LanguageSpecID = buf[4];LanguageSpecID <<= 8;
  LanguageSpecID |= buf[5];LanguageSpecID <<= 8;
  LanguageSpecID |= buf[6];LanguageSpecID <<= 8;
  LanguageSpecID |= buf[7];

  // get language id
  unsigned long LanguageID = buf[8];LanguageID <<= 8;
  LanguageID |= buf[9];LanguageID <<= 8;
  LanguageID |= buf[10];LanguageID <<= 8;
  LanguageID |= buf[11];

  // check language specifier id and language id
  // 0/0 defines Limited ASCII

  // check for ASCII
  if ( (LanguageSpecID == 0x0) && (LanguageID == 0x0) ) {      

    // check length
    if ( length > BufLength ) {
      length = BufLength;
    }
    // string starts at offset 12
    CopyMemory(VendorString,&buf[12],length);
    // ensure zero termination
    VendorString[BufLength-1] = 0;

  } else {
    // no ASCII, check for Microsoft unicode
    if ( (LanguageSpecID == 0x80000000) && (LanguageID == 0x0409) ) {
      length /= 2;
      // check length
      if ( length > BufLength ) {
        length = BufLength;
      }
      // convert unicode string to ascii
      wcstombs( VendorString, (const wchar_t *)&buf[12], length );
      // ensure zero termination
      VendorString[BufLength-1] = 0;

    } else {
      // not supported
      length = 0;
    }

  }

  // cleanup local resources
  delete[] buf;
  return length;

}


DWORD CVhpd::GetDeviceModelString(char* ModelString, DWORD BufLength)
{
  // reset
  ModelString[0] = 0;

  // check buffer and size
  if ( (ModelString == NULL) || (BufLength < 1) ) {
    return 0;
  }

  // ask driver for model string size and contents
  VHPD_CONFIG_INFO_PARAMS params;
  ZeroMemory(&params,sizeof(params));

  DWORD Status = IoctlSync(
                  IOCTL_VHPD_GET_CONFIG_INFO_PARAMS,
                  NULL,
                  0,
                  &params,
                  sizeof(VHPD_CONFIG_INFO_PARAMS),
                  NULL
                  );
  if ( Status != VHPD_STATUS_SUCCESS ) {
    // error
    return 0;
  }

  if ( params.ModelLeafBufSize < 4 ) {
    // no or invalid model string available for this device
    return 0;
  }
  // model string available, allocate storage
  char* buf = new char[params.ModelLeafBufSize];
  if (buf == NULL) {
    return 0; 
  }

  // get model leaf
  VHPD_CONFIG_INFO_BLOCK_DESC desc;
  desc.BlockType = ModelLeaf;
  Status = IoctlSync(
              IOCTL_VHPD_GET_CONFIG_INFO_BLOCK,
              &desc,
              sizeof(VHPD_CONFIG_INFO_BLOCK_DESC),
              buf,
              params.ModelLeafBufSize,
              NULL
              );
  if ( Status != VHPD_STATUS_SUCCESS ) {
    // error
    delete[] buf;
    return 0;
  }
  // extract length in bytes
  unsigned long length = buf[0];
  length <<= 8;
  length |= buf[1]; // length in quadlets
  length *=4; // length in bytes
  length -= 8;  // length of raw string not including format specification
  // check length for plausibility
  if ( (length+12) > params.ModelLeafBufSize ) {
    // error
    delete[] buf;
    return 0;
  }
  
  // get language specifier id
  unsigned long LanguageSpecID = buf[4];LanguageSpecID <<= 8;
  LanguageSpecID |= buf[5];LanguageSpecID <<= 8;
  LanguageSpecID |= buf[6];LanguageSpecID <<= 8;
  LanguageSpecID |= buf[7];

  // get language id
  unsigned long LanguageID = buf[8];LanguageID <<= 8;
  LanguageID |= buf[9];LanguageID <<= 8;
  LanguageID |= buf[10];LanguageID <<= 8;
  LanguageID |= buf[11];

  // check language specifier id and language id
  // 0/0 defines Limited ASCII

  // check for ASCII
  if ( (LanguageSpecID == 0x0) && (LanguageID == 0x0) ) {      

    // check length
    if ( length > BufLength ) {
      length = BufLength;
    }
    // string starts at offset 12
    CopyMemory(ModelString,&buf[12],length);
    // ensure zero termination
    ModelString[BufLength-1] = 0;

  } else {
    // no ASCII, check for Microsoft unicode
    if ( (LanguageSpecID == 0x80000000) && (LanguageID == 0x0409) ) {
      length /= 2;
      // check length
      if ( length > BufLength ) {
        length = BufLength;
      }
      // convert unicode string to ascii
      wcstombs( ModelString, (const wchar_t *)&buf[12], length );
      // ensure zero termination
      ModelString[BufLength-1] = 0;

    } else {
      // not supported
      length = 0;
    }

  }

  // cleanup local resources
  delete[] buf;
  return length;

}


DWORD CVhpd::GetDeviceUniqueId(
  unsigned char UID[8]
  )
{
  // call driver to get the bus info block for the current device
  VHPD_CONFIG_INFO_PARAMS params;
  ZeroMemory(&params,sizeof(params));

  DWORD Status = IoctlSync(
                  IOCTL_VHPD_GET_CONFIG_INFO_PARAMS,
                  NULL,
                  0,
                  &params,
                  sizeof(VHPD_CONFIG_INFO_PARAMS),
                  NULL
                  );
  if ( Status != VHPD_STATUS_SUCCESS ) {
    // error
    return Status;
  }

  // copy node ID to user buffer, bytes are ordered as in the config ROM
  CopyMemory(UID,params.ConfigRomHead.NodeID,8);

  return VHPD_STATUS_SUCCESS;

}


DWORD CVhpd::SetDeviceXmitProperties(const VHPD_DEVICE_XMIT_PROPERTIES* XmitProps)
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_VHPD_SET_DEVICE_XMIT_PROPERTIES,
              XmitProps,
              sizeof(VHPD_DEVICE_XMIT_PROPERTIES),
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CVhpd::GetLocalHostInfo(const VHPD_GET_LOCAL_HOST_INFO* HcInfParams,
                                      VHPD_LOCAL_HOST_INFO_DATA* HcInfData)
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_VHPD_GET_LOCAL_HOST_INFO,
              HcInfParams,
              sizeof(VHPD_GET_LOCAL_HOST_INFO),
              HcInfData,
              sizeof(VHPD_LOCAL_HOST_INFO_DATA),
              NULL
              );

  return Status;
}


DWORD CVhpd::SetLocalHostProperties(
                const VHPD_LOCAL_HOST_PROPERTIES* HCProps,
                const void* Buffer /*=NULL*/,
                DWORD BufferSize /*=0*/
                )
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_VHPD_SET_LOCAL_HOST_PROPERTIES,
              HCProps,
              sizeof(VHPD_LOCAL_HOST_PROPERTIES),
              (void*)Buffer,
              BufferSize,
              NULL
              );

  return Status;
}


DWORD CVhpd::GetNodeAddress(VHPD_GET_ADDR_FROM_DO* GetNodeAdr)
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_VHPD_GET_ADDR_FROM_DEVICE_OBJECT,
              GetNodeAdr,
              sizeof(VHPD_GET_ADDR_FROM_DO),
              GetNodeAdr,
              sizeof(VHPD_GET_ADDR_FROM_DO),
              NULL
              );

  return Status;
}


DWORD CVhpd::ForceBusReset(const VHPD_BUS_RESET* BusResetParams)
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_VHPD_BUS_RESET,
              BusResetParams,
              sizeof(VHPD_BUS_RESET),
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CVhpd::WaitForResetNotification(void)
{
  OVERLAPPED Overlapped;
  DWORD BytesTransferred = 0;
  DWORD Status;
  BOOL succ;

  // check if the device driver was opened
  if ( mFileHandle == NULL ) {
    return VHPD_STATUS_DEVICE_NOT_OPEN;
  }

  // init Overlapped object for asynchronous IO call
  ZeroMemory(&Overlapped,sizeof(Overlapped));
  Overlapped.hEvent=::CreateEvent(NULL ,FALSE ,FALSE ,NULL); 


  // call the device driver (asynchronous)
  succ = DeviceIoControl(
            mFileHandle,                        // driver handle
            IOCTL_VHPD_WAIT_FOR_RESET_NOTIFY,   // IOCTL code
            NULL,                               // input buffer
            0,                                  // input buffer size
            NULL,                               // output buffer
            0,                                  // output buffer size
            &BytesTransferred,                  // pointer to number of bytes written
            &Overlapped                         // pointer to overlapped structure
            );

  if ( succ ) {
    // the operation was completed with success
    // this should not happen, set Status to FAILED
    Status = VHPD_STATUS_FAILED;
  } else {
    Status = GetLastError();
    if ( Status == ERROR_IO_PENDING ) {
      // the operation is pending, wait for completion
      succ = GetOverlappedResult(
                mFileHandle,
                &Overlapped,
                &BytesTransferred,  // byte count
                TRUE                // wait flag
                );
      if ( succ ) {
        // the operation was completed with success
        // this should not happen, set Status to FAILED
        Status = VHPD_STATUS_FAILED;
      } else {
        Status = GetLastError();
      }
    }
  }
  // close Overlapped event
  if ( Overlapped.hEvent != NULL ) {
    CloseHandle(Overlapped.hEvent);
    Overlapped.hEvent = NULL;
  }

  return Status;
}


DWORD CVhpd::GetResetGenerationCount(VHPD_GENERATION_COUNT* GenerationCount)
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_VHPD_GET_GENERATION_COUNT,
              NULL,
              0,
              GenerationCount,
              sizeof(VHPD_GENERATION_COUNT),
              NULL
              );

  return Status;
}


DWORD CVhpd::SendPhyConfigPacket(const VHPD_SEND_PHY_CONFIG_PACKET* SendPCP)
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_VHPD_SEND_PHY_CONFIG_PACKET,
              SendPCP,
              sizeof(VHPD_SEND_PHY_CONFIG_PACKET),
              NULL,
              0,
              NULL
              );

  return Status;
}


BOOL CVhpd::AsyncWrite(const VHPD_ASYNC_WRITE* Write, CVhpdBuf* Buf)
{
  BOOL succ;

  // check if the device driver was opened
  if ( mFileHandle == NULL ) {
    Buf->mStatus = VHPD_STATUS_DEVICE_NOT_OPEN;
    return FALSE;
  }

  // call the device driver (asynchronous)
  succ = DeviceIoControl(
            mFileHandle,                    // driver handle
            IOCTL_VHPD_ASYNC_WRITE,         // IOCTL code
            (void*)Write,                   // input buffer
            sizeof(VHPD_ASYNC_WRITE),       // input buffer size
            Buf->Buffer(),                  // output buffer
            Buf->mNumberOfBytesToTransfer,  // output buffer size
            &Buf->mBytesTransferred,        // pointer to number of bytes written
            &Buf->mOverlapped               // pointer to overlapped structure
            );
  if ( succ ) {
    // the operation was completed with success
    Buf->mStatus = VHPD_STATUS_SUCCESS;
  } else {
    Buf->mStatus = GetLastError();
    if ( Buf->mStatus == ERROR_IO_PENDING ) {
      // the operation is pending
      succ = TRUE;
    }
  }

  return succ;
}


BOOL CVhpd::AsyncRead(const VHPD_ASYNC_READ* Read, CVhpdBuf* Buf)
{
  BOOL succ;

  // check if the driver was opened
  if ( mFileHandle == NULL ) {
    Buf->mStatus = VHPD_STATUS_DEVICE_NOT_OPEN;
    return FALSE;
  }

  // call the device driver (asynchronous)
  succ = DeviceIoControl(
            mFileHandle,                    // driver handle
            IOCTL_VHPD_ASYNC_READ,          // IOCTL code
            (void*)Read,                    // input buffer
            sizeof(VHPD_ASYNC_READ),        // input buffer size
            Buf->Buffer(),                  // output buffer
            Buf->mNumberOfBytesToTransfer,  // output buffer size
            &Buf->mBytesTransferred,        // pointer to number of bytes read
            &Buf->mOverlapped               // pointer to overlapped structure
            );
  if ( succ ) {
    // the operation was completed with success
    Buf->mStatus = VHPD_STATUS_SUCCESS;
  } else {
    Buf->mStatus = GetLastError();
    if ( Buf->mStatus == ERROR_IO_PENDING ) {
      // the operation is pending
      succ = TRUE;
    }
  }

  return succ;
}


BOOL CVhpd::AsyncLock(const VHPD_ASYNC_LOCK* Lock, CVhpdBuf* Buf)
{
  BOOL succ;

  // check if the driver was opened
  if ( mFileHandle == NULL ) {
    Buf->mStatus = VHPD_STATUS_DEVICE_NOT_OPEN;
    return FALSE;
  }

  // call the device driver (asynchronous)
  succ = DeviceIoControl(
            mFileHandle,                    // driver handle
            IOCTL_VHPD_ASYNC_LOCK,          // IOCTL code
            (void*)Lock,                    // input buffer
            sizeof(VHPD_ASYNC_LOCK),        // input buffer size
            Buf->Buffer(),                  // output buffer
            Buf->mNumberOfBytesToTransfer,  // output buffer size
            &Buf->mBytesTransferred,        // pointer to number of bytes read
            &Buf->mOverlapped               // pointer to overlapped structure
            );
  if ( succ ) {
    // the operation was completed with success
    Buf->mStatus = VHPD_STATUS_SUCCESS;
  } else {
    Buf->mStatus = GetLastError();
    if ( Buf->mStatus == ERROR_IO_PENDING ) {
      // the operation is pending
      succ = TRUE;
    }
  }

  return succ;
}


DWORD CVhpd::WaitForCompletion(CVhpdBuf* Buf, DWORD Timeout/*=INFINITE*/)
{
  DWORD Status;
  DWORD err;
  BOOL succ;
  
  if ( Buf->mStatus == VHPD_STATUS_SUCCESS ) {
    // the operation was already completed with success
    Status = VHPD_STATUS_SUCCESS;
  } else {
    if ( Buf->mStatus == ERROR_IO_PENDING ) {   //note: ERROR_IO_PENDING==0x3E5
      // the operation is pending, wait for completion
      err = ::WaitForSingleObject(Buf->mOverlapped.hEvent,Timeout); 
      if ( err == WAIT_TIMEOUT ) {
        // timeout on wait, we do not change the status of the buffer!
        Status = VHPD_STATUS_WAIT_TIMEOUT;
      } else {
        // operation was completed, get final status
        succ = ::GetOverlappedResult(
                    mFileHandle,
                    &Buf->mOverlapped,        // overlapped structure
                    &Buf->mBytesTransferred,  // byte count
                    FALSE                     // wait flag
                    );
        if ( succ ) {
          // success
          Buf->mStatus = VHPD_STATUS_SUCCESS;
        } else {
          // error
          Buf->mStatus = GetLastError();
        }
        Status = Buf->mStatus;
      }
    } else {
      // the operation was already completed with an error
      Status = Buf->mStatus;
    }
  }
  
  return Status;
}



DWORD CVhpd::GetCompletionStatus(CVhpdBuf* Buf)
{
  DWORD Status;
  BOOL succ;

  if ( Buf->mStatus == ERROR_IO_PENDING ) {
    // the operation is pending, get completion status (non-blocking)
    succ = ::GetOverlappedResult(
                mFileHandle,
                &Buf->mOverlapped,        // overlapped structure
                &Buf->mBytesTransferred,  // byte count
                FALSE                     // wait flag
                );
    if ( succ ) {
      // success
      Status = Buf->mStatus = VHPD_STATUS_SUCCESS;
    } else {
      // error
      Status = GetLastError();
      if ( (Status==ERROR_IO_INCOMPLETE) || (Status==ERROR_IO_PENDING) ) {
        // still pending
        Status = ERROR_IO_PENDING;
      } else {
        // completed with error
        Buf->mStatus = Status;
      }
    }
  } else {
    // already completed
    Status = Buf->mStatus;
  }
  
  return Status;
}





DWORD CVhpd::AsyncWriteSync(
        const VHPD_ASYNC_WRITE* Write,
        void* Buffer,
        DWORD BytesToWrite,
        DWORD* BytesTransferred /*=NULL*/,
        DWORD Timeout /*=INFINITE*/ )
{
  DWORD status;

  // check if the driver was opened
  if ( mFileHandle == NULL ) {
    return VHPD_STATUS_DEVICE_NOT_OPEN;
  }

  // init buffer descriptor
  CVhpdBuf Buf(Buffer,BytesToWrite);
  Buf.mNumberOfBytesToTransfer = BytesToWrite;

  // submit the buffer
  AsyncWrite(Write,&Buf);
  // wait for the transfer to complete
  status = WaitForCompletion(&Buf,Timeout);
  if ( status == VHPD_STATUS_WAIT_TIMEOUT ) {
    // a timeout is occurred
    // we must cancel the request because the 
    // Buf.OVERLAPPED struct is valid only in this function
    CancelThreadRequests();
    // now wait for the buffer to complete (infinite)
    WaitForCompletion(&Buf);
  }
  
  // return the number of bytes transferred
  if ( BytesTransferred!=NULL ) {
    *BytesTransferred = Buf.mBytesTransferred;
  }
  
  return status;
}


DWORD CVhpd::AsyncReadSync(
        const VHPD_ASYNC_READ* Read,
        void* Buffer,
        DWORD BytesToRead,
        DWORD* BytesTransferred /*=NULL*/,
        DWORD Timeout /*=INFINITE*/ )
{
  DWORD status;

  // check if the driver was opened
  if ( mFileHandle == NULL ) {
    return VHPD_STATUS_DEVICE_NOT_OPEN;
  }

  // init buffer descriptor
  CVhpdBuf Buf(Buffer,BytesToRead);
  Buf.mNumberOfBytesToTransfer = BytesToRead;

  // submit the buffer
  AsyncRead(Read,&Buf);
  // wait for the transfer to complete
  status = WaitForCompletion(&Buf,Timeout);
  if ( status == VHPD_STATUS_WAIT_TIMEOUT ) {
    // a timeout is occurred
    // we must cancel the request because the 
    // Buf.OVERLAPPED struct is valid only in this function
    CancelThreadRequests();
    // now wait for the buffer to complete (infinite)
    WaitForCompletion(&Buf);
  }
  
  // return the number of bytes transferred
  if ( BytesTransferred!=NULL ) {
    *BytesTransferred = Buf.mBytesTransferred;
  }
  
  return status;
}


DWORD CVhpd::GetPingTime(VHPD_PING_TIME* PingTime)
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_VHPD_GET_PING_TIME,
              NULL,
              0,
              PingTime,
              sizeof(VHPD_PING_TIME),
              NULL
              );

  return Status;
}


DWORD CVhpd::AsyncLockSync(
        const VHPD_ASYNC_LOCK* Lock,
        VHPD_ASYNC_LOCK_DATA* LockData,
        DWORD Timeout /*=INFINITE*/ )
{
  DWORD status;

  // check if the driver was opened
  if ( mFileHandle == NULL ) {
    return VHPD_STATUS_DEVICE_NOT_OPEN;
  }

  // init buffer descriptor
  CVhpdBuf Buf(LockData,sizeof(VHPD_ASYNC_LOCK_DATA));
  Buf.mNumberOfBytesToTransfer = sizeof(VHPD_ASYNC_LOCK_DATA);

  // submit the buffer
  AsyncLock(Lock,&Buf);
  // wait for the transfer to complete
  status = WaitForCompletion(&Buf,Timeout);
  if ( status == VHPD_STATUS_WAIT_TIMEOUT ) {
    // a timeout is occurred
    // we must cancel the request because the 
    // Buf.OVERLAPPED struct is valid only in this function
    CancelThreadRequests();
    // now wait for the buffer to complete (infinite)
    WaitForCompletion(&Buf);
  }
  
  return status;
}


DWORD CVhpd::AllocateAddressRange(const VHPD_ALLOC_ADDR_RANGE* AllocAdrRng)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ALLOC_ADDR_RANGE,
              AllocAdrRng,
              sizeof(VHPD_ALLOC_ADDR_RANGE),
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CVhpd::FreeAddressRange(void)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_FREE_ADDR_RANGE,
              NULL,
              0,
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CVhpd::EnableAddressRange(void)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ENABLE_ADDR_RANGE,
              NULL,
              0,
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CVhpd::DisableAddressRange(void)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_DISABLE_ADDR_RANGE,
              NULL,
              0,
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CVhpd::AddressRangeWriteBuffer(
                const VHPD_ADRRNG_RW_DATA_BUFFER* AdrRngRwBuffer,
                CVhpdBuf* Buf
                )
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ADRRNG_WRITE_DATA_BUFFER,
              AdrRngRwBuffer,
              sizeof(VHPD_ADRRNG_RW_DATA_BUFFER),
              Buf->Buffer(),
              Buf->mNumberOfBytesToTransfer,
              NULL
              );

  return Status;
}


DWORD CVhpd::AddressRangeReadBuffer(
                const VHPD_ADRRNG_RW_DATA_BUFFER* AdrRngRwBuffer,
                CVhpdBuf* Buf
                )
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ADRRNG_READ_DATA_BUFFER,
              AdrRngRwBuffer,
              sizeof(VHPD_ADRRNG_RW_DATA_BUFFER),
              Buf->Buffer(),
              Buf->mNumberOfBytesToTransfer,
              &Buf->mBytesTransferred
              );

  return Status;
}


BOOL CVhpd::GetAddressRangeNotification(
              CVhpdBuf* Buf
              )
{
  BOOL succ;

  // check if the device driver was opened
  if ( mFileHandle == NULL ) {
    Buf->mStatus = VHPD_STATUS_DEVICE_NOT_OPEN;
    return FALSE;
  }

  // call the device driver (asynchronous)
  succ = DeviceIoControl(
            mFileHandle,                        // driver handle
            IOCTL_VHPD_GET_ADRRNG_NOTIFICATION, // IOCTL code
            NULL,                               // input buffer
            0,                                  // input buffer size
            Buf->Buffer(),                      // output buffer
            Buf->mNumberOfBytesToTransfer,      // output buffer size
            &Buf->mBytesTransferred,            // pointer to number of bytes written
            &Buf->mOverlapped                   // pointer to overlapped structure
            );
  if ( succ ) {
    // the operation was completed with success
    Buf->mStatus = VHPD_STATUS_SUCCESS;
  } else {
    Buf->mStatus = GetLastError();
    if ( Buf->mStatus == ERROR_IO_PENDING ) {
      // the operation is pending
      succ = TRUE;
    }
  }

  return succ;
}


DWORD CVhpd::GetAddressRangeLastAccess(VHPD_ADRRNG_LAST_ACCESS* LastAccess)
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_VHPD_GET_ADRRNG_LAST_ACCESS,
              NULL,
              0,
              LastAccess,
              sizeof(VHPD_ADRRNG_LAST_ACCESS),
              NULL
              );

  return Status;
}


BOOL CVhpd::AddressRangeSubmitBuffer(
              CVhpdBuf* Buf
              )
{
  BOOL succ;

  // check if the driver was opened
  if ( mFileHandle == NULL ) {
    Buf->mStatus = VHPD_STATUS_DEVICE_NOT_OPEN;
    return FALSE;
  }

  // call the device driver (asynchronous)
  succ = DeviceIoControl(
            mFileHandle,                              // driver handle
            IOCTL_VHPD_ADRRNG_SUBMIT_BUFFER,          // IOCTL code
            NULL,                                     // input buffer
            0,                                        // input buffer size
            Buf->Buffer(),                            // output buffer
            Buf->mNumberOfBytesToTransfer,            // output buffer size
            &Buf->mBytesTransferred,                  // pointer to number of bytes written
            &Buf->mOverlapped                         // pointer to overlapped structure
            );
  if ( succ ) {
    // the operation was completed with success
    Buf->mStatus = VHPD_STATUS_SUCCESS;
  } else {
    Buf->mStatus = GetLastError();
    if ( Buf->mStatus == ERROR_IO_PENDING ) {
      // the operation is pending
      succ = TRUE;
    }
  }

  return succ;
}


DWORD CVhpd::IsoQueryCycleTime(VHPD_ISOCH_CYCLE_TIME* CycleTime)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ISOCH_QUERY_CYCLE_TIME,
              NULL,
              0,
              CycleTime,
              sizeof(VHPD_ISOCH_CYCLE_TIME),
              NULL
              );

  return Status;
}


DWORD CVhpd::IsoQueryResources(VHPD_ISOCH_RESOURCES* Res)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ISOCH_QUERY_RESOURCES,
              Res,
              sizeof(VHPD_ISOCH_RESOURCES),
              Res,
              sizeof(VHPD_ISOCH_RESOURCES),
              NULL
              );

  return Status;
}


DWORD CVhpd::IsoAllocBandwidth(VHPD_ISOCH_ALLOC_BW* AllocBw)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ISOCH_ALLOC_BANDWIDTH,
              AllocBw,
              sizeof(VHPD_ISOCH_ALLOC_BW),
              AllocBw,
              sizeof(VHPD_ISOCH_ALLOC_BW),
              NULL
              );

  return Status;
}


DWORD CVhpd::IsoFreeBandwidth(void)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ISOCH_FREE_BANDWIDTH,
              NULL,
              0,
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CVhpd::IsoSetBandwidth(const VHPD_ISOCH_SET_CHAN_BW* SetBw)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ISOCH_SET_CHANNEL_BANDWIDTH,
              SetBw,
              sizeof(VHPD_ISOCH_SET_CHAN_BW),
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CVhpd::IsoAllocChannel(VHPD_ISOCH_ALLOC_CHAN* AllocChan)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ISOCH_ALLOC_CHANNEL,
              AllocChan,
              sizeof(VHPD_ISOCH_ALLOC_CHAN),
              AllocChan,
              sizeof(VHPD_ISOCH_ALLOC_CHAN),
              NULL
              );

  return Status;
}


DWORD CVhpd::IsoFreeChannel(void)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ISOCH_FREE_CHANNEL,
              NULL,
              0,
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CVhpd::IsoAllocResources(const VHPD_ISOCH_ALLOC_RES* AllocRes)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ISOCH_ALLOC_RESOURCES,
              AllocRes,
              sizeof(VHPD_ISOCH_ALLOC_RES),
              NULL,
              0,
              NULL
              );

  return Status;
}

DWORD CVhpd::IsoAllocResourcesEx(const VHPD_ISOCH_ALLOC_RES_EX* AllocResEx)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ISOCH_ALLOC_RESOURCES_EX,
              AllocResEx,
              sizeof(VHPD_ISOCH_ALLOC_RES_EX),
              NULL,
              0,
              NULL
              );

  return Status;
}

DWORD CVhpd::IsoFreeResources(void)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ISOCH_FREE_RESOURCES,
              NULL,
              0,
              NULL,
              0,
              NULL
              );

  return Status;
}



DWORD CVhpd::IsoStartTalk(const VHPD_ISOCH_TALK_LISTEN* TalkListen /*=NULL*/)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ISOCH_TALK,
              TalkListen,
              (TalkListen==NULL) ? 0 : sizeof(VHPD_ISOCH_TALK_LISTEN),
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CVhpd::IsoStartListen(const VHPD_ISOCH_TALK_LISTEN* TalkListen /*=NULL*/)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ISOCH_LISTEN,
              TalkListen,
              (TalkListen==NULL) ? 0 : sizeof(VHPD_ISOCH_TALK_LISTEN),
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CVhpd::IsoStop(void)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ISOCH_STOP,
              NULL,
              0,
              NULL,
              0,
              NULL
              );

  return Status;
}


BOOL CVhpd::IsoSubmitWriteBuffer(
      CVhpdBuf* Buf, 
      const VHPD_ISOCH_SUBMIT_BUFFER* Submit /*=NULL*/
      )
{
  BOOL succ;

  // check if the driver was opened
  if ( mFileHandle == NULL ) {
    Buf->mStatus = VHPD_STATUS_DEVICE_NOT_OPEN;
    return FALSE;
  }

  // call the device driver (asynchronous)
  succ = DeviceIoControl(
            mFileHandle,                          // driver handle
            IOCTL_VHPD_ISOCH_SUBMIT_WRITE_BUFFER, // IOCTL code
            (void*)Submit,                        // input buffer
            (Submit==NULL) ? 0 : sizeof(VHPD_ISOCH_SUBMIT_BUFFER), // input buffer size
            Buf->Buffer(),                        // output buffer
            Buf->mNumberOfBytesToTransfer,        // output buffer size
            &Buf->mBytesTransferred,              // pointer to number of bytes written
            &Buf->mOverlapped                     // pointer to overlapped structure
            );
  if ( succ ) {
    // the operation was completed with success
    Buf->mStatus = VHPD_STATUS_SUCCESS;
  } else {
    Buf->mStatus = GetLastError();
    if ( Buf->mStatus == ERROR_IO_PENDING ) {
      // the operation is pending
      succ = TRUE;
    }
  }

  return succ;
}


BOOL CVhpd::IsoSubmitReadBuffer(
      CVhpdBuf* Buf, 
      const VHPD_ISOCH_SUBMIT_BUFFER* Submit /*=NULL*/
      )
{
  BOOL succ;

  // check if the driver was opened
  if ( mFileHandle == NULL ) {
    Buf->mStatus = VHPD_STATUS_DEVICE_NOT_OPEN;
    return FALSE;
  }

  // call the device driver (asynchronous)
  succ = DeviceIoControl(
            mFileHandle,                          // driver handle
            IOCTL_VHPD_ISOCH_SUBMIT_READ_BUFFER,  // IOCTL code
            (void*)Submit,                        // input buffer
            (Submit==NULL) ? 0 : sizeof(VHPD_ISOCH_SUBMIT_BUFFER), // input buffer size
            Buf->Buffer(),                        // output buffer
            Buf->mNumberOfBytesToTransfer,        // output buffer size
            &Buf->mBytesTransferred,              // pointer to number of bytes written
            &Buf->mOverlapped                     // pointer to overlapped structure
            );
  if ( succ ) {
    // the operation was completed with success
    Buf->mStatus = VHPD_STATUS_SUCCESS;
  } else {
    Buf->mStatus = GetLastError();
    if ( Buf->mStatus == ERROR_IO_PENDING ) {
      // the operation is pending
      succ = TRUE;
    }
  }

  return succ;
}


DWORD CVhpd::IsoAttachSharedBuffer(
        const VHPD_ISOCH_ATTACH_SHARED_BUFFER* Attach
        )
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ISOCH_ATTACH_SHARED_BUFFER,
              Attach,
              sizeof(VHPD_ISOCH_ATTACH_SHARED_BUFFER),
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CVhpd::IsoDetachSharedBuffer(void)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ISOCH_DETACH_SHARED_BUFFER,
              NULL,
              0,
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CVhpd::IsoAcknowledgeSharedBuffer(void)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ISOCH_SHARED_BUFFER_ACK,
              NULL,
              0,
              NULL,
              0,
              NULL
              );

  return Status;
}

DWORD CVhpd::AbortIoBuffers(void)
{
  DWORD Status;
  Status = IoctlSync(
              IOCTL_VHPD_ABORT_IO_BUFFERS,
              NULL,
              0,
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CVhpd::IsoWriteSync(
        void* Buffer,
        DWORD BytesToWrite,
        DWORD* BytesTransferred /*=NULL*/,
        DWORD Timeout /*=INFINITE*/,
        const VHPD_ISOCH_SUBMIT_BUFFER* Submit /*=NULL*/
        )
{
  DWORD Status;

  // check if the driver was opened
  if ( mFileHandle == NULL ) {
    return VHPD_STATUS_DEVICE_NOT_OPEN;
  }

  // init buffer descriptor
  CVhpdBuf Buf(Buffer,BytesToWrite);
  Buf.mNumberOfBytesToTransfer = BytesToWrite;

  // submit the buffer
  IsoSubmitWriteBuffer(&Buf,Submit);

  // start the data transfer
  Status = IsoStartTalk();
  if ( Status != VHPD_STATUS_SUCCESS ) {
    // we must cancel the request because the 
    // Buf.OVERLAPPED struct is valid only in this function
    CancelThreadRequests();
    // now wait for the buffer to complete (infinite)
    WaitForCompletion(&Buf);
    // data transfer failed
    return Status;
  }

  // wait for the transfer to complete
  Status = WaitForCompletion(&Buf,Timeout);
  if ( Status == VHPD_STATUS_WAIT_TIMEOUT ) {
    // a timeout is occurred
    // we must cancel the request because the 
    // Buf.OVERLAPPED struct is valid only in this function
    CancelThreadRequests();
    // now wait for the buffer to complete (infinite)
    WaitForCompletion(&Buf);
  }

  // stop data transfer
  IsoStop();
  
  // return the number of bytes transferred
  if ( BytesTransferred != NULL ) {
    *BytesTransferred = Buf.mBytesTransferred;
  }
  
  return Status;
}


DWORD CVhpd::IsoReadSync(
        void* Buffer,
        DWORD BytesToRead,
        DWORD* BytesTransferred /*=NULL*/,
        DWORD Timeout /*=INFINITE*/,
        const VHPD_ISOCH_SUBMIT_BUFFER* Submit /*=NULL*/
        )
{
  DWORD Status;

  // check if the driver was opened
  if ( mFileHandle == NULL ) {
    return VHPD_STATUS_DEVICE_NOT_OPEN;
  }

  // init buffer descriptor
  CVhpdBuf Buf(Buffer,BytesToRead);
  Buf.mNumberOfBytesToTransfer = BytesToRead;

  // submit the buffer
  IsoSubmitReadBuffer(&Buf,Submit);

  // start the data transfer
  Status = IsoStartListen();
  if ( Status != VHPD_STATUS_SUCCESS ) {
    // we must cancel the request because the 
    // Buf.OVERLAPPED struct is valid only in this function
    CancelThreadRequests();
    // now wait for the buffer to complete (infinite)
    WaitForCompletion(&Buf);
    // data transfer failed
    return Status;
  }

  // wait for the transfer to complete
  Status = WaitForCompletion(&Buf,Timeout);
  if ( Status == VHPD_STATUS_WAIT_TIMEOUT ) {
    // a timeout is occurred
    // we must cancel the request because the 
    // Buf.OVERLAPPED struct is valid only in this function
    CancelThreadRequests();
    // now wait for the buffer to complete (infinite)
    WaitForCompletion(&Buf);
  }

  // stop data transfer
  IsoStop();
  
  // return the number of bytes transferred
  if ( BytesTransferred != NULL ) {
    *BytesTransferred = Buf.mBytesTransferred;
  }
  
  return Status;
}



DWORD CVhpd::IoctlSync(
        DWORD IoctlCode,
        const void* InBuffer,
        DWORD InBufferSize,
        void* OutBuffer,
        DWORD OutBufferSize,
        DWORD* BytesReturned
        )
{
  DWORD Status;
  DWORD BytesRet = 0;
  BOOL succ;

  // check if the driver was opened
  if ( mFileHandle == NULL ) {
    return VHPD_STATUS_DEVICE_NOT_OPEN;
  }

  // IOCTL requests must be serialized
  // because there is only one event object per instance
  EnterCriticalSection(&mCritSect);

  // call the device driver
  succ = DeviceIoControl(
            mFileHandle,        // driver handle
            IoctlCode,          // IOCTL code
            (void*)InBuffer,    // input buffer
            InBufferSize,       // input buffer size
            OutBuffer,          // output buffer
            OutBufferSize,      // output buffer size
            &BytesRet,          // number of bytes returned
            &mOverlapped        // overlapped structure (async.)
            );
  if ( succ ) {
    // ioctl completed successfully
    Status = VHPD_STATUS_SUCCESS;
  } else {
    Status = GetLastError();
    if ( Status == ERROR_IO_PENDING ) {
      // the operation is pending, wait for completion
      succ = GetOverlappedResult(
                mFileHandle,
                &mOverlapped,
                &BytesRet,  // byte count
                TRUE        // wait flag
                );
      if ( succ ) {
        // completed successfully
        Status = VHPD_STATUS_SUCCESS;
      } else {
        Status = GetLastError();
      }
    }
  }

  LeaveCriticalSection(&mCritSect);

  if ( BytesReturned != NULL ) {
    *BytesReturned = BytesRet;
  }

  return Status;
}


BOOL CVhpd::CancelThreadRequests(void)
{
  // cancel all outstanding requests that were
  // issued by the calling thread on this handle
  return ::CancelIo(mFileHandle);
}


struct _ErrorCodeTable {
  DWORD Code;
  const char *String;
};


// static
char* _VHPD_LIB_CALL
CVhpd::ErrorText(char* StringBuffer, DWORD StringBufferSize, DWORD ErrorCode)
{
  // string table
  static const struct _ErrorCodeTable ErrorTable[] = {
    {VHPD_STATUS_SUCCESS                , "No error."},
    {VHPD_STATUS_FAILED                 , "Operation failed."},
    {VHPD_STATUS_NOT_SUPPORTED          , "Operation not supported."},
    {VHPD_STATUS_VERSION_MISMATCH       , "API Version does not match."},
    {VHPD_STATUS_TIMEOUT                , "Driver responded timeout status."},
    {VHPD_STATUS_REJECTED               , "Operation rejected."},
    {VHPD_STATUS_DEMO_EXPIRED           , "DEMO version has expired! Please reboot!"},
    {VHPD_STATUS_NO_MEMORY              , "Out of memory."},
    {VHPD_STATUS_POOL_EMPTY             , "Packet pool empty."},
    {VHPD_STATUS_DEVICE_NOT_PRESENT     , "Device is removed."},
    {VHPD_STATUS_INVALID_DEVICE_STATE   , "Device is not in working state."},
    {VHPD_STATUS_INVALID_POWER_STATE    , "Requested power state transition not allowed."},
    {VHPD_STATUS_POWER_DOWN             , "Operation rejected, device is powered down."},
    {VHPD_STATUS_INVALID_PARAMETER      , "Invalid parameter."},
    {VHPD_STATUS_INVALID_IOCTL          , "Invalid IO control code."},
    {VHPD_STATUS_INVALID_BYTE_COUNT     , "Requested byte count doesn´t match buffer size."},
    {VHPD_STATUS_INVALID_BUFFER_SIZE    , "Size of buffer is invalid."},
    {VHPD_STATUS_INVALID_USER_BUFFER    , "User buffer is invalid."},
    {VHPD_STATUS_INVALID_EVENT_HANDLE   , "Event handle is invalid."},
    {VHPD_STATUS_INVALID_INBUFFER       , "Input buffer too small."},
    {VHPD_STATUS_INVALID_OUTBUFFER      , "Output buffer too small."},

    {VHPD_STATUS_BOUND_TO_THIS          , "Handle already used by an object of this type."},
    {VHPD_STATUS_BOUND_TO_OTHER         , "Handle already used by an object of another type."},
    {VHPD_STATUS_NOT_BOUND              , "Handle not associated with required object."},
    {VHPD_STATUS_ALREADY_IN_USE         , "Requested resource already used by another object."},

    {VHPD_STATUS_BUS_RESET_OCCURED      , "Bus reset detected."},
    {VHPD_STATUS_INVALID_GEN_COUNT      , "Invalid reset generation count."},

    {VHPD_STATUS_INVALID_CONFIG_TYPE    , "Invalid configuration info type."},
    {VHPD_STATUS_INVALID_INFO_TYPE      , "Invalid local host info type."},
    {VHPD_STATUS_INVALID_PROP_TYPE      , "Invalid local host property type."},
    {VHPD_STATUS_INVALID_LOCK_TYPE      , "Unsupported type of lock operation."},
    {VHPD_STATUS_INVALID_SPEED          , "Unsupported speed selected."},

    {VHPD_STATUS_DEVICE_NOT_FOUND       , "Device not found."},
    {VHPD_STATUS_DEVICE_NOT_OPEN        , "Device not open."},
    {VHPD_STATUS_WAIT_TIMEOUT           , "Request timeout interval has expired."},
    {VHPD_STATUS_DEVICE_ALREADY_OPEN    , "Device already open."},
    {VHPD_STATUS_LOAD_SETUP_API_FAILED  , "Loading 'setupapi.dll' failed."},
    {VHPD_STATUS_INVALID_FUNCTION_PARAM , "Invalid parameter in function call."},
    {VHPD_STATUS_NO_SUCH_DEV_INSTANCE   , "No such device instance."},
    {VHPD_STATUS_MEM_ALLOC_FAILED       , "Memory allocation failed."},


  };


  static int Size = sizeof(ErrorTable)/sizeof(struct _ErrorCodeTable);
  const char* ErrorString = "Windows system error code.";
  int i;

  if ( (StringBuffer==NULL) || (StringBufferSize==0) ) {
    return StringBuffer;
  }

  for (i=0;i<Size;i++) {
    if (ErrorTable[i].Code == ErrorCode) {
      ErrorString=ErrorTable[i].String;
      break;
    }
  }

  // print error message to string buffer
  _snprintf(StringBuffer,StringBufferSize,"Error code 0x%08X: %s",ErrorCode,ErrorString); 
  // make sure the string is zero-terminated
  StringBuffer[StringBufferSize-1] = 0;

  return StringBuffer;
}


//
// copy maxcount characters from src to dst, including the terminating null
// make sure dst is null-terminated
// convert UNICODE to ASCII
// returns number of chars copied, not counting the terminating null
//
unsigned int CVhpd::StringCopyUNICODEtoASCII(
  char* dst,
  const WCHAR* src,
  unsigned int maxcount
  )
{
  unsigned int n = 0;

  while ( (*src) && (n<(maxcount-1)) ) {
    *dst++ = (char)*src++;
    n++;
  }
  *dst = '\0';

  return n;

} // StringCopyUNICODEtoASCII


/*************************** EOF **************************************/
