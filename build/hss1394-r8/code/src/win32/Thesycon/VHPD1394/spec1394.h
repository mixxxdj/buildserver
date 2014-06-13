/************************************************************************
 *
 *  Module:       spec1394.h
 *  Long name:    IEEE 1394 Specification 
 *  Description:  definitions taken from the IEEE 1394 spec
 *
 *  Runtime Env.: 
 *  Author(s):    Frank Senf
 *  Company:      Thesycon GmbH, Ilmenau
 ************************************************************************/

#ifndef _SPEC1394_H_
#define _SPEC1394_H_


///////////////////////////////////////////////////////////////////
// general constants from IEEE1394 specification
///////////////////////////////////////////////////////////////////

#define SPEC1394_ISO_FRAMES_PER_SECOND  8000

/*
#define SPEC1394_SPEED_FLAGS_100    0x00000001
#define SPEC1394_SPEED_FLAGS_200    0x00000002
#define SPEC1394_SPEED_FLAGS_400    0x00000004
*/


// pack the following structures
#include <PSHPACK1.H>

///////////////////////////////////////////////////////////////////
// configuration ROM related defines
///////////////////////////////////////////////////////////////////

//
// node address (contains node number and bus number)
//
typedef struct _SPEC1394_NODE_ADDR {
  USHORT NodeNmb;
  USHORT BusNmb;
} SPEC1394_NODE_ADDR;


//
// textual leaf information (from configuration ROM)
//
typedef struct _SPEC1394_TEXTUAL_LEAF_INFO {
  USHORT  LeafCRC;        // CRC of text string
  USHORT  LeafLength;     // length of text string
  ULONG   LeafSpecID;     // ID of specification describing LeafLangID
  ULONG   LeafLangID;     // language ID of LeafData
  UCHAR   LeafData[1];    // variable sized data
} SPEC1394_TEXTUAL_LEAF_INFO;


//
// configuration ROM base and complete structure
//
typedef struct _SPEC1394_CONFIG_ROM_HEAD
{
  ULONG Information;      // first 4 bytes of configuration ROM
  ULONG BusName;          // same for all IEEE1394 devices
  ULONG BusInfoBlockCaps; // capabilities of the device
  ULONG NodeID[2];        // 64bit unique ID
  ULONG RootDirFirst;     // first 4 bytes of root directory
} SPEC1394_CONFIG_ROM_HEAD;

#define SPEC1394_MAX_CONFROM_DATA_QUADS   250

typedef struct _SPEC1394_CONFIG_ROM
{
  SPEC1394_CONFIG_ROM_HEAD  Head;
  ULONG                     DirAndLeaveData[SPEC1394_MAX_CONFROM_DATA_QUADS];
} SPEC1394_CONFIG_ROM;


//
// IEEE1394 cycle time format
//
typedef union _SPEC1394_CYCLE_TIME
{
  struct {
    unsigned int CycleOffset:12;  // bits 0-11,  updated by 24,576 MHz (carry at 3071 -> 0)
    unsigned int CycleCount:13;   // bits 12-24, updated by each carry of CycleOffset (carry at 7999 -> 0)
    unsigned int CycleSeconds:7;  // bits 25-31, updated by each carry of CycleCount (carry at 127 -> 0)
  } bits;
  unsigned int CycleTime; // bits 0-31
} SPEC1394_CYCLE_TIME;


//
// primary self ID packet layout
//
typedef struct _SPEC1394_SELF_ID_PACKET
{
  // Byte 0
  unsigned long   Phys_ID:6;        // pysical ID of node
  unsigned long   Packet_ID:2;      // SID packet identifier (01b)
  // Byte 1
  unsigned long   Gap_Count:6;      // current value of PHYs gap count register
  unsigned long   Link_Active:1;    // one indicates active link and transaction layer
  unsigned long   Zero:1;           // zero for primary SID packets
  // Byte 2
  unsigned long   Power_Class:3;    // power class, see specification
  unsigned long   Contender:1;      // one indicates node is contender for iso. ressource manager
  unsigned long   Delay:2;          // maximum repeater delay (00->144ns, other values rsvd.)
  unsigned long   Speed:2;          // nodes speed capabilities (00->100Mb, 01->200Mb, 10->400Mb)
  // Byte 3
  unsigned long   More_Packets:1;   // indicates that more SID packets from this node will follow
  unsigned long   Initiated_Rst:1;  // this node initiated current bus reset
  unsigned long   Port2:2;          // port status: 00->not present, 01->not connected
  unsigned long   Port1:2;          //              10->connected to parent
  unsigned long   Port0:2;          //              11->connected to child

} SPEC1394_SELF_ID_PACKET;


//
// additional self ID packet layout
//
typedef struct _SPEC1394_SELF_ID_MORE_PACKET
{
  // Byte 0
  unsigned long   Phys_ID:6;
  unsigned long   Packet_ID:2;
  // Byte 1
  unsigned long   PortA:2;
  unsigned long   Rsvd1:2;
  unsigned long   Sequence:2;       // sequence number of SID packet (1, 2, or 3)
  unsigned long   One:1;            // one for additional SID packets
  unsigned long   Rsvd2:1;
  // Byte 2
  unsigned long   PortE:2;
  unsigned long   PortD:2;
  unsigned long   PortC:2;
  unsigned long   PortB:2;
  // Byte 3
  unsigned long   More_Packets:1;
  unsigned long   Rsvd3:1;
  unsigned long   PortH:2;
  unsigned long   PortG:2;
  unsigned long   PortF:2;

} SPEC1394_SELF_ID_MORE_PACKET;


//
// PHY configuration packet layout
//
typedef struct _SPEC1394_PHY_CONFIG_PACKET
{
  union {
    unsigned long     Quadlet;
    struct {
      // first quadlet
      // Byte 0
      unsigned long   NewRootID:6;      // ID of node which shall become root
                                        //  only valid if ForceRoot is set
      unsigned long   PacketID:2;       // PHY config packet identifier (00b)
      // Byte 1
      unsigned long   NewGapCount:6;    // new gap count value
                                        //  only valid if UpdateGapCount is set
      unsigned long   UpdateGapCount:1; // 1 means update gap count
      unsigned long   ForceRoot:1;      // 1 means force a node to become root
      // Byte 2
      unsigned long   Rsvd1:8;
      // Byte 3
      unsigned long   Rsvd2:8;
    };
  } u;
  
  // second quadlet
  unsigned long   Inverse;

} SPEC1394_PHY_CONFIG_PACKET;
    

//
// topology map layout
//
#define SPEC1394_MAX_SID_FIELDS   253

typedef struct _SPEC1394_TOPOLOGY_MAP
{
  USHORT                  Length;
  USHORT                  CRC;
  ULONG                   GenerationCount;
  USHORT                  NodeCount;
  USHORT                  SelfIDCount;
  SPEC1394_SELF_ID_PACKET SelfIDArray[SPEC1394_MAX_SID_FIELDS];

} SPEC1394_TOPOLOGY_MAP;


//
// speed map layout
//
#define SPEC1394_SPEED_CODE_FIELDS    4032

typedef struct _SPEC1394_SPEED_MAP
{
  USHORT  Length;
  USHORT  CRC;
  ULONG   GenerationCount;
  UCHAR   SpeedCode[SPEC1394_SPEED_CODE_FIELDS];

} SPEC1394_SPEED_MAP;


// restore packing
#include <POPPACK.H>


#endif  // _SPEC1394_H_

/*************************** EOF **************************************/
