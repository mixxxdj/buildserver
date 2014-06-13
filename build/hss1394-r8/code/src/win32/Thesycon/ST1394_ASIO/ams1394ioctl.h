/************************************************************************
 *
 *  Module:       ams1394ioctl.h
 *  Long name:    Device Driver API
 *                (common requests)
 *  Description:
 *    Declares the programming interface provided by the 
 *    device driver.
 *                
 *
 *  Runtime Env.: 
 *    Windows XP Kernel, Win32
 *
 *  Author(s):   
 *    Udo Eberhardt,  Udo.Eberhardt@thesycon.de
 *    Frank Senf,     Frank.Senf@thesycon.de
 *                
 *  Companies:
 *    Thesycon GmbH, Germany      http://www.thesycon.de
 *                
 ************************************************************************/

#ifndef __AMS1394IOCTL_H__
#define __AMS1394IOCTL_H__



///////////////////////////////////////////////////////////////////
// IOCTL Requests
///////////////////////////////////////////////////////////////////

// Retrieve driver version information.
#define IOCTL_GENERIC_GET_DRIVER_INFO                      ST1394OX_IOCTL_CODE(0x001,METHOD_BUFFERED)
// InBuffer:  NULL
// OutBuffer: GenericDriverInfo


// Retrieve current debug settings.
#define IOCTL_GENERIC_GET_DEBUG_SETTINGS                   ST1394OX_IOCTL_CODE(0x002,METHOD_BUFFERED)
// InBuffer:  NULL
// OutBuffer: GenericDriverDebugSettings


// Modify current debug settings.
#define IOCTL_GENERIC_SET_DEBUG_SETTINGS                   ST1394OX_IOCTL_CODE(0x003,METHOD_BUFFERED)
// InBuffer:  GenericDriverDebugSettings
// OutBuffer: NULL


// Save current debug settings permanently in the registry.
#define IOCTL_GENERIC_SAVE_DEBUG_SETTINGS                  ST1394OX_IOCTL_CODE(0x004,METHOD_BUFFERED)
// InBuffer:  NULL
// OutBuffer: NULL


// Query driver statistics.
#define IOCTL_ST1394OX_QUERY_STATISTICS										 ST1394OX_IOCTL_CODE(0x010,METHOD_BUFFERED)
// InBuffer:  GenericFullStatisticsQuery
// OutBuffer: AmsFullStatisticsInfo



// Register a notification event.
#define IOCTL_ST1394OX_REGISTER_NOTIFICATION_EVENT         ST1394OX_IOCTL_CODE(0x020,METHOD_BUFFERED)
// InBuffer:  AmsRegisterNotificationEvent
// OutBuffer: NULL

// Unregister the notification event that was registered previously.
#define IOCTL_ST1394OX_UNREGISTER_NOTIFICATION_EVENT       ST1394OX_IOCTL_CODE(0x021,METHOD_BUFFERED)
// InBuffer:  NULL
// OutBuffer: NULL

// Retrieve a notification message.
#define IOCTL_ST1394OX_GET_NOTIFICATION                    ST1394OX_IOCTL_CODE(0x022,METHOD_BUFFERED)
// InBuffer:  NULL
// OutBuffer: AmsNotificationInfo


// Retrieve the current list of devices.
#define IOCTL_ST1394OX_GET_DEVICE_LIST                     ST1394OX_IOCTL_CODE(0x030,METHOD_BUFFERED)
// InBuffer:  NULL
// OutBuffer: AmsDeviceList

// Retrieve current status information.
#define IOCTL_ST1394OX_GET_STATUS_INFO                     ST1394OX_IOCTL_CODE(0x031,METHOD_BUFFERED)
// InBuffer:  NULL
// OutBuffer: AmsStatusInfo

// Set the sample rate.
#define IOCTL_ST1394OX_SET_SAMPLE_RATE                     ST1394OX_IOCTL_CODE(0x032,METHOD_BUFFERED)
// InBuffer:  AmsSetSampleRate
// OutBuffer: NULL

// Start streaming data path.
#define IOCTL_ST1394OX_START_STREAMING										 ST1394OX_IOCTL_CODE(0x033,METHOD_BUFFERED)
// InBuffer:  NULL
// OutBuffer: NULL

// Stop streaming data path.
#define IOCTL_ST1394OX_STOP_STREAMING											 ST1394OX_IOCTL_CODE(0x034,METHOD_BUFFERED)
// InBuffer:  NULL
// OutBuffer: NULL

// Select the device that shall be used by streaming data path.
#define IOCTL_ST1394OX_SELECT_DEVICE											 ST1394OX_IOCTL_CODE(0x035,METHOD_BUFFERED)
// InBuffer:  AmsSelectDevice
// OutBuffer: NULL

// Set size of buffers used in streaming data path.
#define IOCTL_ST1394OX_SET_BUFFER_SIZE 										 ST1394OX_IOCTL_CODE(0x040,METHOD_BUFFERED)
// InBuffer:  AmsSetBufferSize
// OutBuffer: NULL




///////////////////////////////////////////////////////////////////
// Data structures and associated types
///////////////////////////////////////////////////////////////////

// struct alignment = 1 byte
#include <pshpack1.h>


//
// driver version info
//
typedef struct tagGenericDriverInfo
{
  // current version of programming interface exported by the device driver
  unsigned int interfaceVersionMajor;
  unsigned int interfaceVersionMinor;

  // current version of device driver implementation
  unsigned int driverVersionMajor;
  unsigned int driverVersionMinor;
  unsigned int driverVersionSub;

  // additional information, encoded as bit flags
  unsigned int flags;
// the device driver is a debug build
#define GENERIC_INFOFLAG_CHECKED_BUILD    0x00000001

} GenericDriverInfo;


//
// driver debug settings
//
typedef struct tagGenericDriverDebugSettings
{
  // trace mask
  unsigned int traceMask;

} GenericDriverDebugSettings;



//
// input parameters when statistics is queried
//
typedef struct tagGenericFullStatisticsQuery
{
	// flag: true forces to clear statistic values after query
	unsigned int resetStatistics;

} GenericFullStatisticsQuery;



//
// AmsRegisterNotificationEvent
//
typedef struct tagAmsRegisterNotificationEvent
{
  // handle that identifies a Win32 Event object to be registered
  // Note: The event object should be of auto-reset type.
  HANDLE eventHandle;

} AmsRegisterNotificationEvent;


//
// AmsNotificationInfo
//
typedef struct tagAmsNotificationInfo
{
  // type of event
  AmsEventType eventType;

  // additional information, meaning depends on event type
  ULONG information[2];

} AmsNotificationInfo;


//
// AmsDeviceProperties
//
typedef struct tagAmsDeviceProperties
{
  // unique device ID
  AmsObjectId deviceID;
  
  // 8 byte EUI as stored in 1394 ConfigRom
  // VendorId[2..0] = deviceEui[7..5]
  // DeviceId[4..0] = deviceEui[4..0]
  UCHAR deviceEui[8];

} AmsDeviceProperties;

//
// AmsDeviceList
//
typedef struct tagAmsDeviceList
{
  // unique device ID of device that is currently selected
  // zero if no device is selected
  AmsObjectId selectedDeviceID;

  // number of valid devices in list
  // set to zero if no devices are available
  ULONG deviceCount;

  // array of AmsDeviceProperties structures, one for each valid device
  // the array contains deviceCount valid entries
  // the array does not contain AmsDeviceProperties structures for virtual devices
  AmsDeviceProperties deviceProperties[AMS_MAX_DEVICE_COUNT];

} AmsDeviceList;


//
// AmsBufferSize
// this determines the size of the driver-internal streaming buffer
//
typedef enum tagAmsBufferSize
{
	BufferSize_Minimum	= 0,
	BufferSize_Small,
	BufferSize_Standard, // this is the default
	BufferSize_Large,
	BufferSize_ExtraLarge,
	BufferSize_Maximum,

	BufferSize_last,	// for internal use only

  // dummy value, defined to force sizeof(enum)==4
  BufferSize_Force32bitEnum = 2000000000 

} AmsBufferSize;

//
// AmsBufferSizeAsio
// this determines the size of the preferred ASIO switch interval,
// the ASIO switch interval is always a multiple of the streaming buffer size
//
typedef enum tagAmsBufferSizeAsio
{
	BufferSizeAsio_Standard, // this is the default
	BufferSizeAsio_Large,
	BufferSizeAsio_Maximum,

	BufferSizeAsio_last,	// for internal use only

  // dummy value, defined to force sizeof(enum)==4
  BufferSizeAsio_Force32bitEnum = 2000000000 

} AmsBufferSizeAsio;

typedef struct tagAmsSetBufferSize
{
  // size of streaming buffer (one of the predefined values)
  AmsBufferSize bufferSize;
  
  // multiplier to calculate preferred ASIO switch interval
  AmsBufferSizeAsio bufferSizeAsio;

} AmsSetBufferSize;


//
// AmsSampleRateList
//
typedef struct tagAmsSampleRateList
{
  // number of rates in the sample rate set
  // set to zero if the set is empty
  ULONG count;

  // Array of sample rates. The array contains Count valid entries.
  // Each rate is specified in terms of samples per second.
  ULONG sampleRate[AMS_MAX_SAMPLE_RATE_COUNT];

} AmsSampleRateList;


//
// AmsStatusInfo
//
typedef struct tagAmsStatusInfo
{
  // current streaming state
  AmsStreamingState streamingState;

  // sample rate, in samples per second
  ULONG currentSampleRate;

  // current size of streaming buffer (one of the predefined values)
  AmsBufferSize currentBufferSize;

  // current size of the preferred ASIO switch interval (one of the predefined values)
  AmsBufferSizeAsio currentBufferSizeAsio;

  // list of currently supported sample rates
  AmsSampleRateList supportedSampleRates;

} AmsStatusInfo;


//
// AmsSetSampleRate
//
typedef struct tagAmsSetSampleRate
{
  // sample rate, in samples per second
  ULONG sampleRate;

} AmsSetSampleRate;


//
// AmsSelectDevice
//
typedef struct tagAmsSelectDevice
{
  // unique device ID, one of the IDs returned in AmsDeviceList.deviceProperties.deviceID
  AmsObjectId deviceID;

} AmsSelectDevice;



// restore previous alignment
#include <poppack.h>



///////////////////////////////////////////////////////////////////
// Support Macros
///////////////////////////////////////////////////////////////////

//
// Define the device type value. Note that values used by Microsoft
// are in the range 0-32767, and 32768-65535 are reserved for use
// by customers.
//
#define FILE_DEVICE_ST1394OX     0x8456

//
// Macro used to generate IOCTL codes.
// Note that function codes 0-2047 are reserved for Microsoft, and
// 2048-4095 are reserved for customers.
//
#define ST1394OX_IOCTL_BASE       0x800

#define ST1394OX_IOCTL_CODE(FnCode,Method)  \
   ( (ULONG)CTL_CODE(                       \
      (ULONG)FILE_DEVICE_ST1394OX,          \
      (ULONG)(ST1394OX_IOCTL_BASE+(FnCode)),\
      (ULONG)(Method),                      \
      (ULONG)FILE_ANY_ACCESS                \
      ) )


#endif  // __AMS1394IOCTL_H__

/*************************** EOF **************************************/
