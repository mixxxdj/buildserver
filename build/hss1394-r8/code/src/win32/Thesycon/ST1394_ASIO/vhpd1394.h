/************************************************************************
 *
 *  Module:       vhpd1394.h
 *  Long name:    VHPD1394 Driver Interface
 *  Description:  Defines the programming interface of the 
 *                VHPD1394 driver
 *
 *  Runtime Env.: Win32, WDM
 *  Author(s):    Frank Senf, Udo Eberhardt
 *  Company:      Thesycon GmbH, Ilmenau
 ************************************************************************/

#ifndef _VHPD1394_H_
#define _VHPD1394_H_

#include "spec1394.h"

//
// The current interface version number.
// This will be incremented if changes are made at the programming interface level.
// The following convention exists:
// If changes are made to the programming interface that are compatible with
// previous versions then the minor version number (low order byte) will be incremented.
// If changes are made that cause an incompatibility with previous versions of the 
// interface then the major version number (high order byte) will be incremented.
// 
// Applications should call IOCTL_VHPD_GET_DRIVER_INFO and check if the driver currently
// running supports the expected interface version. The major version number must
// match the expected number exactly. The minor version number needs to be greater than
// or equal to the expected number.
//
#define VHPD_IF_VERSION_MJ   1
#define VHPD_IF_VERSION_MN   80
#define VHPD_IF_VERSION     ((VHPD_IF_VERSION_MJ<<8)|VHPD_IF_VERSION_MN)


//
// The software interface exposed by the driver for physical devices that support audio data streaming
// is unambiguously identified by the ST1394OX_IID GUID.
//
#ifndef ST1394OX_IID
#define ST1394OX_IID         { 0x96a5413a, 0x9630, 0x4c55, { 0xa9, 0x4b, 0x33, 0x1a, 0xd4, 0x7, 0x2b, 0x36 } }
#endif

//
// The software interface exposed by the driver for physical devices that do not support audio data streaming
// is unambiguously identified by the ST1394OX_C_IID GUID.
//
#define ST1394OX_C_IID       { 0x31048fa3, 0xdc4d, 0x45b5, { 0x99, 0x38, 0xb2, 0xe8, 0x30, 0xf9, 0x57, 0xd9 } }

//
// The software interface exposed by the driver for virtual devices
// is unambiguously identified by the ST1394OX_V_IID GUID.
//
#define ST1394OX_V_IID       { 0x8eadf3ad, 0x5463, 0x485a, { 0x8b, 0x2a, 0xb8, 0x85, 0xc4, 0x8, 0x93, 0x5e } }



///////////////////////////////////////////////////////////////////
// Status Codes
///////////////////////////////////////////////////////////////////

#define  VHPD_STATUS_SUCCESS                0x00000000L
#define  VHPD_STATUS_FAILED                 0xE0000001L
#define  VHPD_STATUS_NOT_SUPPORTED          0xE0000010L
#define  VHPD_STATUS_VERSION_MISMATCH       0xE0000011L
#define  VHPD_STATUS_NOT_SUPPORTED_ON_VDEV  0xE0000012L
#define  VHPD_STATUS_TIMEOUT                0xE0000020L
#define  VHPD_STATUS_REJECTED               0xE0000030L
#define  VHPD_STATUS_DEMO_EXPIRED           0xE0000040L
#define  VHPD_STATUS_VENDOR_INVALID         0xE0000050L
#define  VHPD_STATUS_NO_MEMORY              0xE0000100L
#define  VHPD_STATUS_POOL_EMPTY             0xE0000110L
#define  VHPD_STATUS_DEVICE_NOT_PRESENT     0xE0000200L
#define  VHPD_STATUS_INVALID_DEVICE_STATE   0xE0000210L
#define  VHPD_STATUS_INVALID_POWER_STATE    0xE0000220L
#define  VHPD_STATUS_POWER_DOWN             0xE0000280L
#define  VHPD_STATUS_INVALID_PARAMETER      0xE0000300L
#define  VHPD_STATUS_INVALID_IOCTL          0xE0000302L
#define  VHPD_STATUS_INVALID_BYTE_COUNT     0xE0000304L
#define  VHPD_STATUS_INVALID_BUFFER_SIZE    0xE0000305L
#define  VHPD_STATUS_INVALID_USER_BUFFER    0xE0000306L
#define  VHPD_STATUS_INVALID_EVENT_HANDLE   0xE0000307L
#define  VHPD_STATUS_INVALID_INBUFFER       0xE0000310L
#define  VHPD_STATUS_INVALID_OUTBUFFER      0xE0000320L

#define  VHPD_STATUS_BOUND_TO_THIS          0xE0000340L
#define  VHPD_STATUS_BOUND_TO_OTHER         0xE0000341L
#define  VHPD_STATUS_NOT_BOUND              0xE0000342L
#define  VHPD_STATUS_ALREADY_IN_USE         0xE0000343L

#define  VHPD_STATUS_BUS_RESET_OCCURED      0xE0000350L
#define  VHPD_STATUS_INVALID_GEN_COUNT      0xE0000351L

#define  VHPD_STATUS_INVALID_CONFIG_TYPE    0xE0000400L
#define  VHPD_STATUS_INVALID_INFO_TYPE      0xE0000401L
#define  VHPD_STATUS_INVALID_PROP_TYPE      0xE0000402L
#define  VHPD_STATUS_INVALID_LOCK_TYPE      0xE0000403L
#define  VHPD_STATUS_INVALID_SPEED          0xE0000404L
#define  VHPD_STATUS_DISCONTINUITY          0xE0000500L


#define  VHPD_STATUS_DEVICE_NOT_FOUND       0xE0008001L
#define  VHPD_STATUS_DEVICE_NOT_OPEN        0xE0008002L
#define  VHPD_STATUS_WAIT_TIMEOUT           0xE0008003L
#define  VHPD_STATUS_DEVICE_ALREADY_OPEN    0xE0008004L
#define  VHPD_STATUS_LOAD_SETUP_API_FAILED  0xE0008005L
#define  VHPD_STATUS_INVALID_FUNCTION_PARAM 0xE0008008L
#define  VHPD_STATUS_NO_SUCH_DEV_INSTANCE   0xE0008010L
#define  VHPD_STATUS_MEM_ALLOC_FAILED       0xE0008020L


#define  VHPD_STATUS_RETURNED_FROM_BUSDRV   0xE8000000L



///////////////////////////////////////////////////////////////////
// IOCTL requests supported by the VHPD1394 driver
///////////////////////////////////////////////////////////////////

//
// general driver requests
//

#define IOCTL_VHPD_GET_DRIVER_INFO              _VHPD_IOCTL_CODE(1,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: VHPD_DRIVER_INFO

#define IOCTL_VHPD_GET_DEVICE_PARAMETERS        _VHPD_IOCTL_CODE(2,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: VHPD_DEVICE_PARAMETERS

#define IOCTL_VHPD_SET_DEVICE_PARAMETERS        _VHPD_IOCTL_CODE(3,METHOD_BUFFERED)
// InBuffer:  VHPD_DEVICE_PARAMETERS
// OutBuffer: none

#define IOCTL_VHPD_QUERY_RATE_COUNTER           _VHPD_IOCTL_CODE(4,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: VHPD_QUERY_RATE_COUNTER



//
// general requests for the 1394 bus
//

#define IOCTL_VHPD_BUS_RESET                    _VHPD_IOCTL_CODE(5,METHOD_BUFFERED)
// InBuffer:  VHPD_BUS_RESET
// OutBuffer: none

#define IOCTL_VHPD_WAIT_FOR_RESET_NOTIFY        _VHPD_IOCTL_CODE(6,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: none

#define IOCTL_VHPD_GET_GENERATION_COUNT         _VHPD_IOCTL_CODE(7,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: VHPD_GENERATION_COUNT

#define IOCTL_VHPD_GET_CONFIG_INFO_PARAMS       _VHPD_IOCTL_CODE(9,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: VHPD_CONFIG_INFO_PARAMS

#define IOCTL_VHPD_GET_CONFIG_INFO_BLOCK        _VHPD_IOCTL_CODE(10,METHOD_BUFFERED)
// InBuffer:  VHPD_CONFIG_INFO_BLOCK_DESC
// OutBuffer: information block, variable sized

#define IOCTL_VHPD_SET_DEVICE_XMIT_PROPERTIES   _VHPD_IOCTL_CODE(11,METHOD_BUFFERED)
// InBuffer:  VHPD_DEVICE_XMIT_PROPERTIES
// OutBuffer: none

#define IOCTL_VHPD_GET_ADDR_FROM_DEVICE_OBJECT  _VHPD_IOCTL_CODE(12,METHOD_BUFFERED)
// InBuffer:  VHPD_GET_ADDR_FROM_DO
// OutBuffer: VHPD_GET_ADDR_FROM_DO

#define IOCTL_VHPD_GET_LOCAL_HOST_INFO          _VHPD_IOCTL_CODE(13,METHOD_BUFFERED)
// InBuffer:  VHPD_GET_LOCAL_HOST_INFO
// OutBuffer: VHPD_LOCAL_HOST_INFO_DATA

#define IOCTL_VHPD_SET_LOCAL_HOST_PROPERTIES    _VHPD_IOCTL_CODE(14,METHOD_IN_DIRECT)
// InBuffer:  VHPD_LOCAL_HOST_PROPERTIES
// OutBuffer: config ROM extension for Type=ModifyCROM

#define IOCTL_VHPD_SEND_PHY_CONFIG_PACKET       _VHPD_IOCTL_CODE(15,METHOD_BUFFERED)
// InBuffer:  VHPD_SEND_PHY_CONFIG_PACKET
// OutBuffer: none



//
// asynchronous transactions
//

#define IOCTL_VHPD_ASYNC_WRITE                  _VHPD_IOCTL_CODE(20,METHOD_IN_DIRECT)
// InBuffer:  VHPD_ASYNC_WRITE
// OutBuffer: data buffer

#define IOCTL_VHPD_ASYNC_READ                   _VHPD_IOCTL_CODE(21,METHOD_OUT_DIRECT)
// InBuffer:  VHPD_ASYNC_READ
// OutBuffer: data buffer

#define IOCTL_VHPD_GET_PING_TIME                _VHPD_IOCTL_CODE(22,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: VHPD_PING_TIME

#define IOCTL_VHPD_ASYNC_LOCK                   _VHPD_IOCTL_CODE(23,METHOD_BUFFERED)
// InBuffer:  VHPD_ASYNC_LOCK
// OutBuffer: VHPD_ASYNC_LOCK_DATA

#define IOCTL_VHPD_ALLOC_ADDR_RANGE             _VHPD_IOCTL_CODE(25,METHOD_BUFFERED)
// InBuffer:  VHPD_ALLOC_ADDR_RANGE
// OutBuffer: none

#define IOCTL_VHPD_FREE_ADDR_RANGE              _VHPD_IOCTL_CODE(26,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: none

#define IOCTL_VHPD_ENABLE_ADDR_RANGE            _VHPD_IOCTL_CODE(27,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: none

#define IOCTL_VHPD_DISABLE_ADDR_RANGE           _VHPD_IOCTL_CODE(28,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: none

#define IOCTL_VHPD_GET_ADRRNG_NOTIFICATION      _VHPD_IOCTL_CODE(29,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: VHPD_ADRRNG_NOTIFICATION

#define IOCTL_VHPD_ADRRNG_WRITE_DATA_BUFFER     _VHPD_IOCTL_CODE(30,METHOD_IN_DIRECT)
// InBuffer:  VHPD_ADRRNG_RW_DATA_BUFFER
// OutBuffer: data to be written


#define IOCTL_VHPD_ADRRNG_READ_DATA_BUFFER      _VHPD_IOCTL_CODE(31,METHOD_OUT_DIRECT)
// InBuffer:  VHPD_ADRRNG_RW_DATA_BUFFER
// OutBuffer: data read


#define IOCTL_VHPD_ADRRNG_SUBMIT_BUFFER   _VHPD_IOCTL_CODE(32,METHOD_OUT_DIRECT)
// InBuffer:  none
// OutBuffer: data buffer


#define IOCTL_VHPD_GET_ADRRNG_LAST_ACCESS      _VHPD_IOCTL_CODE(33,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: VHPD_ADRRNG_LAST_ACCESS



//
// isochronous operations
//

#define IOCTL_VHPD_ISOCH_QUERY_CYCLE_TIME       _VHPD_IOCTL_CODE(35,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: VHPD_ISOCH_CYCLE_TIME

#define IOCTL_VHPD_ISOCH_QUERY_RESOURCES        _VHPD_IOCTL_CODE(36,METHOD_BUFFERED)
// InBuffer:  VHPD_ISOCH_RESOURCES
// OutBuffer: VHPD_ISOCH_RESOURCES

#define IOCTL_VHPD_ISOCH_ALLOC_BANDWIDTH        _VHPD_IOCTL_CODE(37,METHOD_BUFFERED)
// InBuffer:  VHPD_ISOCH_ALLOC_BW
// OutBuffer: VHPD_ISOCH_ALLOC_BW

#define IOCTL_VHPD_ISOCH_FREE_BANDWIDTH         _VHPD_IOCTL_CODE(38,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: none

#define IOCTL_VHPD_ISOCH_SET_CHANNEL_BANDWIDTH  _VHPD_IOCTL_CODE(39,METHOD_BUFFERED)
// InBuffer:  VHPD_ISOCH_SET_CHAN_BW
// OutBuffer: none

#define IOCTL_VHPD_ISOCH_ALLOC_CHANNEL          _VHPD_IOCTL_CODE(40,METHOD_BUFFERED)
// InBuffer:  VHPD_ISOCH_ALLOC_CHAN
// OutBuffer: VHPD_ISOCH_ALLOC_CHAN

#define IOCTL_VHPD_ISOCH_FREE_CHANNEL           _VHPD_IOCTL_CODE(41,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: none

#define IOCTL_VHPD_ISOCH_ALLOC_RESOURCES        _VHPD_IOCTL_CODE(42,METHOD_BUFFERED)
// InBuffer:  VHPD_ISOCH_ALLOC_RES
// OutBuffer: none

#define IOCTL_VHPD_ISOCH_FREE_RESOURCES         _VHPD_IOCTL_CODE(43,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: none

#define IOCTL_VHPD_ISOCH_SUBMIT_WRITE_BUFFER    _VHPD_IOCTL_CODE(44,METHOD_IN_DIRECT)
// InBuffer:  VHPD_ISOCH_SUBMIT_BUFFER (optional)
// OutBuffer: data buffer

#define IOCTL_VHPD_ISOCH_SUBMIT_READ_BUFFER     _VHPD_IOCTL_CODE(45,METHOD_OUT_DIRECT)
// InBuffer:  VHPD_ISOCH_SUBMIT_BUFFER (optional)
// OutBuffer: data buffer

#define IOCTL_VHPD_ISOCH_TALK                   _VHPD_IOCTL_CODE(46,METHOD_BUFFERED)
// InBuffer:  VHPD_ISOCH_TALK_LISTEN (optional)
// OutBuffer: none

#define IOCTL_VHPD_ISOCH_LISTEN                 _VHPD_IOCTL_CODE(47,METHOD_BUFFERED)
// InBuffer:  VHPD_ISOCH_TALK_LISTEN (optional)
// OutBuffer: none

#define IOCTL_VHPD_ISOCH_STOP                   _VHPD_IOCTL_CODE(48,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: none

#define IOCTL_VHPD_ABORT_IO_BUFFERS             _VHPD_IOCTL_CODE(49,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: none


#define IOCTL_VHPD_ISOCH_ATTACH_SHARED_BUFFER   _VHPD_IOCTL_CODE(50,METHOD_BUFFERED)
// InBuffer:  VHPD_ISOCH_ATTACH_SHARED_BUFFER
// OutBuffer: none

#define IOCTL_VHPD_ISOCH_DETACH_SHARED_BUFFER   _VHPD_IOCTL_CODE(51,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: none


#define IOCTL_VHPD_ISOCH_SHARED_BUFFER_ACK      _VHPD_IOCTL_CODE(52,METHOD_BUFFERED)
// InBuffer:  none
// OutBuffer: none


#define IOCTL_VHPD_ISOCH_ALLOC_RESOURCES_EX     _VHPD_IOCTL_CODE(53,METHOD_BUFFERED)
// InBuffer:  VHPD_ISOCH_ALLOC_RES_EX
// OutBuffer: none




///////////////////////////////////////////////////////////////////
// interface structures
///////////////////////////////////////////////////////////////////

// pack the following structures                                                     
#include <PSHPACK1.H>

//
// constants
//
#define VHPD_ISOCH_MIN_CHAN           0
#define VHPD_ISOCH_MAX_CHAN           63
#define VHPD_ISOCH_ANY_CHAN           ((ULONG)-1)

//
// flags
//
#define VHPD_INFOFLAG_CHECKED_BUILD   0x00000001
#define VHPD_INFOFLAG_DEMO_VERSION    0x00000002
#define VHPD_INFOFLAG_VENDOR_SPECIFIC 0x00000004

#define VHPD_FLAG_BUS_RESET_ON_CLOSE  0x00000002
#define VHPD_FLAG_MONITOR_RESET       0x00000004
#define VHPD_FLAG_VIRTUAL_DEVICE      0x00000008
#define VHPD_FLAG_ON_T1394BUS         0x00000010

#define VHPD_FLAG_NONINCREMENTING     0x00000010
#define VHPD_FLAG_NO_STATUS           0x00000020
#define VHPD_FLAG_PING_PACKET         0x00000040
#define VHPD_FLAG_BROADCAST           0x00000080

#define VHPD_FLAG_RESET_FORCE_ROOT    0x00000100

#define VHPD_FLAG_USE_LOCAL_NODE      0x00000200

#define VHPD_FLAG_HCINFO_UNIQUE_ID    0x00001000

#define VHPD_FLAG_HCINFO_CAPS         0x00002000
#define VHPD_FLAG_HCCAPS_PACKBASED    0x00000001
#define VHPD_FLAG_HCCAPS_STRBASED     0x00000002
#define VHPD_FLAG_HCCAPS_ISOSTRIP     0x00000004
#define VHPD_FLAG_HCCAPS_STRTONCY     0x00000008
#define VHPD_FLAG_HCCAPS_ISORET       0x00000010
#define VHPD_FLAG_HCCAPS_ISOINS       0x00000020
#define VHPD_FLAG_HCCAPS_ISOCIPSTRIP  0x00000040

#define VHPD_FLAG_HC_ADD_CONF_ROM     0x00000100
#define VHPD_FLAG_HC_REM_CONF_ROM     0x00000200

#define VHPD_FLAG_ISOCH_TALKING       0x00000001
#define VHPD_FLAG_ISOCH_LISTENING     0x00000002
#define VHPD_FLAG_ISOCH_START_ON_TIME 0x00000010
#define VHPD_FLAG_ISOCH_PACK_BASED    0x00000020
#define VHPD_FLAG_ISOCH_MULTICHANNEL  0x00000040
#define VHPD_FLAG_ISOCH_VAR_PAYLOAD   0x00000080

#define VHPD_FLAG_ISOCH_SYNCH_ON_SY   0x00010000
#define VHPD_FLAG_ISOCH_SYNCH_ON_TAG  0x00020000
#define VHPD_FLAG_ISOCH_SYNCH_ON_TIME 0x00040000
#define VHPD_FLAG_ISOCH_SYTAG_FIRST   0x00080000
#define VHPD_FLAG_ISOCH_TIMESTAMP     0x00100000
#define VHPD_FLAG_ISOCH_PRIO_TIME     0x00200000
#define VHPD_FLAG_ISOCH_HEADER_SG     0x00400000



typedef enum _VHPD_DEVICE_POWER_STATE {
  DevicePowerStateD0 = 0,
  DevicePowerStateD1,
  DevicePowerStateD2,
  DevicePowerStateD3,
  DevicePowerState_Force32bitEnum = 2000000000    // dummy value to force sizeof(enum)==4
} VHPD_DEVICE_POWER_STATE;


typedef enum _VHPD_BUS_SPEED {
  BusSpeedRsvd = 0,
  BusSpeed100 = 1,
  BusSpeed200 = 2,
  BusSpeed400 = 4,
  BusSpeed800 = 8,
  BusSpeed1600 = 16,
  BusSpeedFastest = 1024,
  BusSpeed_Force32bitEnum = 2000000000    // dummy value to force sizeof(enum)==4
} VHPD_BUS_SPEED;


//
// unsigned 64 bit integer
//
// compiler complains about 'nonstandard extension used : nameless struct/union'
#pragma warning(disable:4201)
typedef union _VHPD_UINT64 {
  struct {
    ULONG LowPart;
    ULONG HighPart;
  };
  unsigned __int64 QuadPart;
} VHPD_UINT64;
#pragma warning(default:4201)    



//
// GET_DRIVER_INFO
//
typedef struct _VHPD_DRIVER_INFO {
  USHORT IFVersion;     // hi byte = major, lo byte = minor
  USHORT DriverVersion; // hi byte = major, lo byte = minor
  ULONG  DriverBuildNumber;
  ULONG  Flags;         // VHPD_INFOFLAG_XXX
} VHPD_DRIVER_INFO;



//
// GET_DEVICE_PARAMETERS
// SET_DEVICE_PARAMETERS
//
typedef struct _VHPD_DEVICE_PARAMETERS {
  ULONG Flags;
  ULONG RequestTimeout;
} VHPD_DEVICE_PARAMETERS;



//
// QUERY_RATE_COUNTER
//
typedef struct _VHPD_QUERY_RATE_COUNTER {
  ULONG FilterLength;
  ULONG FilterDelay;      // ms
  ULONG CurrentMeanValue; // bytes
} VHPD_QUERY_RATE_COUNTER;



//
// BUS_RESET
//
typedef struct _VHPD_BUS_RESET {
  ULONG Flags;
} VHPD_BUS_RESET;


//
// GET_GENERATION_COUNT
//
typedef struct _VHPD_GENERATION_COUNT {
  ULONG Count;
} VHPD_GENERATION_COUNT;


//
// GET_CONFIG_INFO_PARAMS
//
typedef struct _VHPD_CONFIG_INFO_PARAMS {
  SPEC1394_CONFIG_ROM_HEAD  ConfigRomHead;
  VHPD_UINT64               UnitDirAddress;
  VHPD_UINT64               UnitDepDirAddress;
  ULONG                     UnitDirBufSize;
  ULONG                     UnitDepDirBufSize;
  ULONG                     VendorLeafBufSize;
  ULONG                     ModelLeafBufSize;
} VHPD_CONFIG_INFO_PARAMS;


typedef enum _VHPD_CONFIG_INFO_BLOCK_TYPE {
  UnitDirectory = 0,
  UnitDependentDirectory,
  VendorLeaf,
  ModelLeaf,
  ConfigInfoBlockType_Force32bitEnum = 2000000000   // dummy value to force sizeof(enum)==4
} VHPD_CONFIG_INFO_BLOCK_TYPE;

//
// GET_CONFIG_INFO_BLOCK
//
typedef struct _VHPD_CONFIG_INFO_BLOCK_DESC {
  VHPD_CONFIG_INFO_BLOCK_TYPE BlockType;
} VHPD_CONFIG_INFO_BLOCK_DESC;


//
// SET_DEVICE_XMIT_PROPERTIES
//
typedef struct _VHPD_DEVICE_XMIT_PROPERTIES {
  VHPD_BUS_SPEED Speed;
} VHPD_DEVICE_XMIT_PROPERTIES;


//
// GET_ADDR_FROM_DEVICE_OBJECT
//
typedef struct _VHPD_GET_ADDR_FROM_DO {
  SPEC1394_NODE_ADDR NodeAddr;
  ULONG Flags;
} VHPD_GET_ADDR_FROM_DO;


typedef enum _VHPD_HC_INFO_TYPE {
  UniqueID = 0,
  Capabilities,
  PowerSupplied,
  ConfigRom,
  TopologyMap,
  SpeedMap,
  HcInfoType_Force32bitEnum = 2000000000    // dummy value to force sizeof(enum)==4
} VHPD_HC_INFO_TYPE;

//
// GET_LOCAL_HOST_INFO
//
typedef struct _VHPD_GET_LOCAL_HOST_INFO {
  VHPD_HC_INFO_TYPE Type;
} VHPD_GET_LOCAL_HOST_INFO;

//
// GET_LOCAL_HOST_INFO
//
typedef union _VHPD_LOCAL_HOST_INFO_DATA {
  struct {
    VHPD_UINT64 UniqueId;
  } HCInfUniqueID;

  struct {
    ULONG Capabilities;
    ULONG MaxAsyncReadRequest;
    ULONG MaxAsyncWriteRequest;
  } HCInfCapabilities;

  struct {
    ULONG deciWattsSupplied;
    ULONG Voltage;
  } HCInfPowerSupplied;

  struct {
    SPEC1394_CONFIG_ROM ConfigRom;
  } HCInfConfigRom;

  struct {
    SPEC1394_TOPOLOGY_MAP TopologyMap;
  } HCInfTopologyMap;

  struct {
    SPEC1394_SPEED_MAP SpeedMap;
  } HCInfSpeedMap;

} VHPD_LOCAL_HOST_INFO_DATA;


typedef enum _VHPD_HC_PROP_TYPE {
  NoCycleStarts = 0,    // not yet implemented by bus driver
  GapCountLowerBound,
  ModifyCROM,
  HcPropType_Force32bitEnum = 2000000000    // dummy value to force sizeof(enum)==4
} VHPD_HC_PROP_TYPE;

//
// SET_LOCAL_HOST_PROPERTIES
//
typedef struct _VHPD_LOCAL_HOST_PROPERTIES {
  VHPD_HC_PROP_TYPE Type;
  union {
/*
    struct {
    // not yet implemented by bus driver
    } HCPropNoCycleStarts;
*/

  struct {
    ULONG GapCountLowerBound;
  } HCPropGapCountLowerBound;

    struct {
      ULONG Flags;
    } HCPropModifyCROM;
  } u;

} VHPD_LOCAL_HOST_PROPERTIES;


//
// SEND_PHY_CONFIG_PACKET
//
typedef struct _VHPD_SEND_PHY_CONFIG_PACKET {
  SPEC1394_PHY_CONFIG_PACKET PhyConfigPacket;
} VHPD_SEND_PHY_CONFIG_PACKET;


//
// ASYNC_WRITE
//
typedef struct _VHPD_ASYNC_WRITE {
  VHPD_UINT64 DestinationAddress;
  ULONG BlockSize;    // 0 --> max. block size
  ULONG Flags;
} VHPD_ASYNC_WRITE;


//
// ASYNC_READ
//
typedef struct _VHPD_ASYNC_READ {
  VHPD_UINT64 DestinationAddress;
  ULONG BlockSize;    // 0 --> max. block size
  ULONG Flags;
} VHPD_ASYNC_READ;


//
// GET_PING_TIME
//
typedef struct _VHPD_PING_TIME {
  ULONG Time;
} VHPD_PING_TIME;


typedef enum _VHPD_LOCK_TYPE {
  MaskSwap = 0,     // new_value = data_value | (old_value & ~arg_value)
  CompareSwap,      // if (old_value == arg_value) new_value = data_value
  FetchAdd,         // new_value = old_value + data_value
  LittleAdd,        // (little) new_value = (little) old_value + (little) data_value
  BoundedAdd,       // if (old_value != arg_value) new_value = old_value + data_value
  WrapAdd,          // new_value = (old_value != arg_value) ? old_value + data_value : data_value
  LockType_Force32bitEnum = 2000000000    // dummy value to force sizeof(enum)==4
} VHPD_LOCK_TYPE;

//
// ASYNC_LOCK
//
typedef struct _VHPD_ASYNC_LOCK {
  VHPD_LOCK_TYPE LockType;
  VHPD_UINT64 DestinationAddress;
  USHORT NmbOfArgQuadlets;    // 0, 1, 2
  USHORT NmbOfDataQuadlets;   // 1, 2

  // lock transactions can be performed as 32bit or 64bit operation
  ULONG Arg[2];
  ULONG Data[2];

} VHPD_ASYNC_LOCK;

typedef struct _VHPD_ASYNC_LOCK_DATA {
  // lock transaction always return the data values
  // at DestinationAddress immediately prior performing the lock
  ULONG DataReturned[2];
} VHPD_ASYNC_LOCK_DATA;



#define VHPD_FLAG_ADRRNG_ACCESS_READ    0x00000001
#define VHPD_FLAG_ADRRNG_ACCESS_WRITE   0x00000002
#define VHPD_FLAG_ADRRNG_ACCESS_LOCK    0x00000004
#define VHPD_FLAG_ADRRNG_ACCESS_BC      0x00000008

#define VHPD_FLAG_ADRRNG_NOTIFY_READ    0x00000010
#define VHPD_FLAG_ADRRNG_NOTIFY_WRITE   0x00000020
#define VHPD_FLAG_ADRRNG_NOTIFY_LOCK    0x00000040

#define VHPD_FLAG_ADRRNG_NEW_ACCESS     0x80000000


// this corresponds to the max length of the payload of an asynchronous packet
#define VHPD_NOTIFY_DATA_BYTES_RET      4096


typedef enum _VHPD_ADDR_RANGE_MODE {
  BufferStoreMode = 0,
  BufferQueueMode,
  FifoStoreMode,
  AddrRangeMode_Force32bitEnum = 2000000000   // dummy value to force sizeof(enum)==4
} VHPD_ADDR_RANGE_MODE;

typedef struct _VHPD_ADDR_RANGE_DESC {
  VHPD_UINT64 StartAddress;
  ULONG Length;
} VHPD_ADDR_RANGE_DESC;


//
// ALLOCATE_ADDR_RANGE
//
typedef struct _VHPD_ALLOC_ADDR_RANGE {
  VHPD_ADDR_RANGE_MODE Mode;

  VHPD_ADDR_RANGE_DESC Desc;

  ULONG AccessTypes;
  ULONG NotificationTypes;

  VHPD_UINT64 TriggerAddress;

} VHPD_ALLOC_ADDR_RANGE;


//
// GET_ADRRNG_NOTIFICATION
//
typedef struct _VHPD_ADRRNG_NOTIFICATION {
  VHPD_UINT64 StartAddress;
  ULONG Offset;
  ULONG Length;
  ULONG NotificationEvent;
  UCHAR Data[VHPD_NOTIFY_DATA_BYTES_RET];
} VHPD_ADRRNG_NOTIFICATION;


//
// GET_ADRRNG_LAST_ACCESS
//
typedef struct _VHPD_ADRRNG_LAST_ACCESS {
  ULONG Offset;
  ULONG Length;
  ULONG NotificationEvent;
} VHPD_ADRRNG_LAST_ACCESS;


//
// ADRRNG_WRITE_DATA_BUFFER
// ADRRNG_READ_DATA_BUFFER
//
typedef struct _VHPD_ADRRNG_RW_DATA_BUFFER {
  ULONG Offset;
} VHPD_ADRRNG_RW_DATA_BUFFER;


//
// ISOCH_QUERY_CYCLE_TIME
//
typedef struct _VHPD_ISOCH_CYCLE_TIME {
  SPEC1394_CYCLE_TIME CycleTime;
} VHPD_ISOCH_CYCLE_TIME;


//
// ISOCH_QUERY_RESOURCES
//
typedef struct _VHPD_ISOCH_RESOURCES {
  VHPD_BUS_SPEED Speed;
  ULONG AvailableBytesPerFrame;
  VHPD_UINT64 AvailableChannels;
} VHPD_ISOCH_RESOURCES;


//
// ISOCH_ALLOC_BANDWIDTH
//
typedef struct _VHPD_ISOCH_ALLOC_BW {
  ULONG ReqBytesPerFrame;
  VHPD_BUS_SPEED ReqSpeed;
  ULONG RemBytesPerFrame;
  VHPD_BUS_SPEED SelSpeed;
} VHPD_ISOCH_ALLOC_BW;


//
// ISOCH_SET_CHANNEL_BANDWIDTH
//
typedef struct _VHPD_ISOCH_SET_CHAN_BW {
  ULONG ReqBytesPerFrame;
} VHPD_ISOCH_SET_CHAN_BW;


//
// ISOCH_ALLOC_CHANNEL
//
typedef struct _VHPD_ISOCH_ALLOC_CHAN {
  ULONG ReqChannel; 
  ULONG ChannelGot;
  VHPD_UINT64 RemChannels;
} VHPD_ISOCH_ALLOC_CHAN;


//
// ISOCH_ALLOCATE_RESOURCES
//
typedef struct _VHPD_ISOCH_ALLOC_RES {
  ULONG Flags;
  ULONG UsedChannel;
  VHPD_BUS_SPEED UsedSpeed;
  ULONG ExpMaxBytesPerFrame;
  ULONG MaxNmbOfBuffersUsed;
  ULONG MaxBufferSizeUsed;
  ULONG QuadsToStrip;
} VHPD_ISOCH_ALLOC_RES;


//
// ISOCH_ALLOCATE_RESOURCES_EX (extended for multichannel receive)
//
typedef struct _VHPD_ISOCH_ALLOC_RES_EX {
  ULONG Flags;
  ULONG UsedChannel;
  VHPD_BUS_SPEED UsedSpeed;
  ULONG ExpMaxBytesPerFrame;
  ULONG MaxNmbOfBuffersUsed;
  ULONG MaxBufferSizeUsed;
  ULONG QuadsToStrip;
  VHPD_UINT64 ChannelMask;
} VHPD_ISOCH_ALLOC_RES_EX;



//
// ISOCH_SUBMIT_WRITE_BUFFER
// ISOCH_SUBMIT_READ_BUFFER
//
typedef struct _VHPD_ISOCH_SUBMIT_BUFFER
{
  ULONG Flags;
  ULONG BytesPerFrame;
  ULONG SyField;
  ULONG TagField;
  SPEC1394_CYCLE_TIME CycleTime;
} VHPD_ISOCH_SUBMIT_BUFFER;


//
// ISOCH_TALK
// ISOCH_LISTEN
//
typedef struct _VHPD_ISOCH_TALK_LISTEN
{
  SPEC1394_CYCLE_TIME CycleTime;
} VHPD_ISOCH_TALK_LISTEN;


#define VHPD_SHBUF_SEG_OWNED_BY_DRIVER    0
#define VHPD_SHBUF_SEG_OWNED_BY_APP       1

#define VHPD_SHBUF_MAX_SEGMENTS           64

//
// ISOCH_ATTACH_SHARED_BUFFER
//
typedef struct _VHPD_ISOCH_SHARED_BUFFER_STATUS
{
  volatile LONG SegmentState[VHPD_SHBUF_MAX_SEGMENTS];
  
  volatile ULONG DiscontinuityCounter;
  volatile ULONG ErrorCounter;
  volatile ULONG ErrorStatus;

} VHPD_ISOCH_SHARED_BUFFER_STATUS;

typedef struct _VHPD_ISOCH_ATTACH_SHARED_BUFFER
{
  void* BufferStartAddress;

  ULONG PayloadBytesPerFrame;
  ULONG BytesPerFrameInBuffer;
  ULONG FramesPerSegment;
  ULONG NumberOfSegments;

  HANDLE SignalEvent;

  VHPD_ISOCH_SHARED_BUFFER_STATUS* SharedBufferStatus;

  ULONG Flags;
  ULONG UsedChannel;
  VHPD_BUS_SPEED UsedSpeed;
  ULONG QuadsToStrip;
  ULONG SyField;
  ULONG TagField;

} VHPD_ISOCH_ATTACH_SHARED_BUFFER;


// restore previous packing
#include <POPPACK.H>


///////////////////////////////////////////////////////////////////
// private support macros
///////////////////////////////////////////////////////////////////

//
// Define the device type value. Note that values used by Microsoft
// are in the range 0-32767, and 32768-65535 are reserved for use
// by customers.
//
#define FILE_DEVICE_VHPD1394       0x8099

//
// Macros to generate IOCTL codes.
// Note that function codes 0-2047 are reserved for Microsoft, and
// 2048-4095 are reserved for customers.
//
#define _VHPD_IOCTL_BASE       0x800

#define _VHPD_IOCTL_CODE(FnCode,Method)     \
   ( (ULONG)CTL_CODE(                       \
      (ULONG)FILE_DEVICE_VHPD1394,          \
      (ULONG)(_VHPD_IOCTL_BASE+(FnCode)),   \
      (ULONG)(Method),                      \
      (ULONG)FILE_ANY_ACCESS                \
      ) )


#endif  // _VHPD1394_H_

/*************************** EOF **************************************/
