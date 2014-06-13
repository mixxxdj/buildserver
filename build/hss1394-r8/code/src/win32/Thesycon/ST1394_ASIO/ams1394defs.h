/************************************************************************
 *
 *  Module:       ams1394defs.h
 *  Long name:    Device Driver API
 *                (general definitions)
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

#ifndef __AMS1394DEFS_H__
#define __AMS1394DEFS_H__


//
// Version of the software interface exported by the driver.
//
// This will be incremented if changes are made at the programming interface level.
// The following convention exists:
// If changes are made to the programming interface that are compatible with
// previous versions then the minor version number (low order byte) will be incremented.
// If changes are made that cause an incompatibility with previous versions of the 
// interface then the major version number (high order byte) will be incremented.
//
#define ST1394OX_INTERFACE_VERSION_MJ   1
#define ST1394OX_INTERFACE_VERSION_MN   00
#define ST1394OX_INTERFACE_VERSION     ( (ST1394OX_INTERFACE_VERSION_MJ<<8) | ST1394OX_INTERFACE_VERSION_MN )



//
// The software interface exposed by the driver for physical devices that support audio data streaming
// is unambiguously identified by the ST1394OX_IID GUID.
//
#ifndef ST1394OX_IID
#define ST1394OX_IID         { 0x96a5413a, 0x9630, 0x4c55, { 0xa9, 0x4b, 0x33, 0x1a, 0xd4, 0x7, 0x2b, 0x36 } }
#endif
#define ST1394OX_IID_STR     "{96A5413A-9630-4c55-A94B-331AD4072B36}"
#define ST1394OX_IID_STR_W  L"{96A5413A-9630-4c55-A94B-331AD4072B36}"


//
// Default driver interface name (static device object name exported by the driver).
//
#define ST1394OX_DRIVER_INTERFACE_NAME		ST1394OX_IID_STR
#define ST1394OX_DRIVER_INTERFACE_NAME_W	ST1394OX_IID_STR_W



///////////////////////////////////////////////////////////////////
// Status Codes
///////////////////////////////////////////////////////////////////

#define  AMS_STATUS_SUCCESS                 0x00000000L
#define  AMS_STATUS_FAILED                  0xE0000001L
#define  AMS_STATUS_NO_MEMORY               0xE0000002L
#define  AMS_STATUS_UNKNOWN                 0xE0000003L
#define  AMS_STATUS_INTERNAL_ERROR          0xE0000004L
#define  AMS_STATUS_TIMEOUT                 0xE0000005L
#define  AMS_STATUS_ABORTED                 0xE0000006L
#define  AMS_STATUS_NO_MORE_ITEMS           0xE0000007L

#define  AMS_STATUS_NOT_SUPPORTED           0xE0000010L
#define  AMS_STATUS_NOT_POSSIBLE            0xE0000011L
#define  AMS_STATUS_NOT_ALLOWED             0xE0000012L
#define  AMS_STATUS_NOT_OPEN                0xE0000013L
#define  AMS_STATUS_VERSION_MISMATCH        0xE0000014L

#define  AMS_STATUS_REJECTED                0xE0000020L
#define  AMS_STATUS_ALREADY_DONE            0xE0000021L
#define  AMS_STATUS_NEGATIVE_RESPONSE       0xE0000022L
#define  AMS_STATUS_WRONG_DIRECTION         0xE0000023L

#define  AMS_STATUS_INVALID_PARAMETER       0xE0000030L
#define  AMS_STATUS_INVALID_BUFFER_SIZE     0xE0000031L
#define  AMS_STATUS_INVALID_SAMPLE_SIZE     0xE0000032L
#define  AMS_STATUS_INVALID_TYPE            0xE0000033L
#define  AMS_STATUS_INVALID_SAMPLE_RATE     0xE0000034L
#define  AMS_STATUS_INVALID_IOCTL           0xE0000035L
#define  AMS_STATUS_INVALID_INBUFFER_SIZE   0xE0000036L
#define  AMS_STATUS_INVALID_OUTBUFFER_SIZE  0xE0000037L

#define  AMS_STATUS_NOT_FOUND               0xE0000040L
#define  AMS_STATUS_IN_USE                  0xE0000041L
#define  AMS_STATUS_NO_CONNECTION           0xE0000042L
#define  AMS_STATUS_CHANNEL_CONFIG_CHANGED  0xE0000043L

#define  AMS_STATUS_DEVICE_REMOVED          0xE0000050L

#define  AMS_STATUS_REGISTRY_ACCESS_FAILED  0xE0000060L

#define  AMS_STATUS_ALLOC_BUFFER_FAILED     0xE0000070L
#define  AMS_STATUS_ALLOC_IRPRES_FAILED     0xE0000071L
#define  AMS_STATUS_ALLOC_CHANNEL_FAILED    0xE0000072L
#define  AMS_STATUS_ALLOC_BANDWIDTH_FAILED  0xE0000073L
#define  AMS_STATUS_ALLOC_ISORES_FAILED     0xE0000074L
#define  AMS_STATUS_ATTACH_BUFFER_FAILED    0xE0000075L
#define  AMS_STATUS_START_CHANNEL_FAILED    0xE0000076L
#define  AMS_STATUS_CHANNEL_CHANGED         0xE0000077L

#define  AMS_STATUS_UNSUPPORTED_SAMPLE_RATE 0xE0000080L



///////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////

// maximum number of characters in name strings, including terminating null
#define AMS_MAX_NAME_STRING_LENGTH        32

// max number of rates per sample rate set
#define AMS_MAX_SAMPLE_RATE_COUNT         16

// max number of devices
#define AMS_MAX_DEVICE_COUNT              16

// max number of channels per channel set
#define AMS_MAX_CHANNELS_PER_SET          32



//
// Sample format
//
typedef enum tagAmsSampleFormat
{
  AmsSampleFormat_24BitLPCM = 0,	// 24 bit linear audio, corresponds to ASIOSTInt32LSB
  AmsSampleFormat_32BitFP,				// IEEE 754 32 bit floating point (Intel), corresponds to ASIOSTFloat32LSB

  AvsSampleFormatOther = 1000,		// used for Midi and other formats

  AmsSampleFormat_Force32bitEnum = 2000000000 // dummy value, defined to force sizeof(enum)==4

} AmsSampleFormat;


//
// Direction of a channel
//
typedef enum tagAmsDirection
{
  AmsDirection_Unknown = 0,
  // data flows from network to application
  AmsDirection_Input,
  // data flows from application to network
  AmsDirection_Output,

  AmsDirection_Force32bitEnum = 2000000000 // dummy value, defined to force sizeof(enum)==4

} AmsDirection;


//
// Type of a channel
//
typedef enum tagAmsChannelType
{
  // type unknown
  AmsChannelType_Unknown = 0,    
  // raw audio channel, LPCM or floating point format
  AmsChannelType_RawAudio,
  // SPDIF channel
  AmsChannelType_SPDIF,
  // MIDI channel
  AmsChannelType_MIDI,

  // dummy value, defined to force sizeof(enum)==4
  AmsChannelType_Force32bitEnum = 2000000000 

} AmsChannelType;


//
// Streaming state
//
typedef enum tagAmsStreamingState
{
  // type unknown
  AmsStreamingState_Unknown = 0,

  // stopped
  AmsStreamingState_Stopped,
  // started
  AmsStreamingState_Started,

  // error
  AmsStreamingState_Error,

  // dummy value, defined to force sizeof(enum)==4
  AmsStreamingState_Force32bitEnum = 2000000000 

} AmsStreamingState;



//
// Type of notification event
//
typedef enum tagAmsEventType
{
  // this value is reserved
  AmsEvtType_Undefined = 0,

  // a bus reset was handled by the driver
  AmsEvtType_BusReset,

  // the isochronous streaming has been started
  AmsEvtType_StreamingStarted,
  // the isochronous streaming has been stopped
  AmsEvtType_StreamingStopped,


  AmsEvtType_Force32bitEnum = 2000000000 // dummy value, defined to force sizeof(enum)==4

} AmsEventType;



///////////////////////////////////////////////////////////////////
// Types
///////////////////////////////////////////////////////////////////

// status code returned by the driver
// holds either a driver-defined code (see AMS_STATUS_xxx) or a Windows error code
typedef unsigned long AMS_STATUS;


// unique identifier for an object
typedef unsigned __int64 AmsObjectId;


// We define a union to encapsulate 32 bit and 64 bit pointers.
// For both x86 and x64 code the size of this union is 8 bytes.
typedef union tagGenericEmbeddedPointer {

	// pointer type, size varies
	void* pointer;

	// handle type, declared as void*, size varies
	HANDLE handle;

	// 32 bit pointer type used by the 64 bit driver if the calling process is 32 bit
	void* __ptr32 pointer32;
	// 64 bit pointer type, size is always 8 bytes
	// forces sizeof(GenericEmbeddedPointer) == 8
	void* __ptr64 pointer64;

// lint does not understand __ptr32 and __ptr64
// force struct size == 8 bytes
#ifdef _lint
	unsigned char just_for_lint[8];
#endif

} GenericEmbeddedPointer;




///////////////////////////////////////////////////////////////////
// Support Macros
///////////////////////////////////////////////////////////////////

//
// Define the device type value. Note that values used by Microsoft
// are in the range 0-32767, and 32768-65535 are reserved for use
// by customers.
//
#define FILE_DEVICE_AMS1394       0x8282

//
// Macro used to generate IOCTL codes.
// Note that function codes 0-2047 are reserved for Microsoft, and
// 2048-4095 are reserved for customers.
//
#define AMS1394_IOCTL_BASE       0x800

#define AMS1394_IOCTL_CODE(FnCode,Method)   \
   ( (ULONG)CTL_CODE(                       \
      (ULONG)FILE_DEVICE_AMS1394,           \
      (ULONG)(AMS1394_IOCTL_BASE+(FnCode)), \
      (ULONG)(Method),                      \
      (ULONG)FILE_ANY_ACCESS                \
      ) )




#endif  // __AMS1394DEFS_H__

/*************************** EOF **************************************/
