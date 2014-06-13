//*****************************************************************************
// Oxford1394Definitions.h
//
// 1394 register locations used by the Oxford firmware device inquiry and
// updating protocol.
//
// Created on 17Dec2008, 10:20PST by don@bearanascence.com
// $Id: HSS1394.h 1362 2008-11-19 08:22:07Z don $
// ----------------------------------------------------------------------------
// This file Copyright (C) 2008, Beara Nascence Inc. All Rights Reserved.
//*****************************************************************************

#ifndef _OXFORD1394DEFINITIONS_
#define _OXFORD1394DEFINITIONS_

#include "HSS1394Types.h"


// 1394 Registers
//-----------------------------------------------------------------------------
const hss1394::uint48 SCS_UNIT_TYPE(0xffff, 0xf0090100);

const hss1394::uint48 FLASH_PORT(0xFFFF, 0xF0080000);
// WR:  This register is used for flash port programming of the devices
//      At present this is the only method supported for the UF922
// RD:  Reads of this address are not supported for FW900 or FW911
//      For the UF922 a read returns the value

const hss1394::uint48 UPLOAD_MODE_CTRL(0xFFFF, 0xF0090010);
// WR:  The main firmware recognises a write to this address with this code
// as a request to switch into upload mode (When UPLOAD_MODE_ENTER_CODE is
// written) or to enter force flash mode (when FORCE_FLASH_ENTER_CODE is
// written)
// RD:  Reads of this address are not supported

const hss1394::uint48 FLASH_UPLOAD_CTRL(0xFFFF, 0xF0090008);
//????

const hss1394::uint48 CSR_VERSION(0xFFFF, 0xF0050000);
// WR:  Writes to this address are not supported
// RD:  This returns a quadlet which identifies the chip identity and
//      also gives the firmware revision
//      Note that the format of this quadlet is different for all chips
//      and very strange for the 900

const hss1394::uint48 CSR_EXTRA_VERSION(0xFFFF,0xF0060000);
// WR:  Writes to this address are not supported
// RD:  This returns a quadlet which identifies the fix number (upper 16 bits)
//      and internal version number (lower 16 bits) for FW912 and related chips

const hss1394::uint48 HSS1394_DISABLE_WATCHDOG(0xc007, 0xdedadada);
// WR: Writing 0x10000000 will disable the off-chip watchdog, allowing operation
//     without having to reset the watchdog in code. Only implemented in main
//     firmware code (via HSS1394 protocol).

const hss1394::uint48 FLASH_UPLOAD_DATA(0xFFFF, 0xF0090004);
// WR:  The quadlet data is written to the flash LSB at lowest address.
//      The flash address is automatically incremented from that
//      originaly specified in the write to UPLOAD_START_PORT
// RD:  A read of this address will return the checksum of all data written
//      to the flash since the write to UPLOAD_START_PORT. This read also
//      signifies the end of the upload process and will reset the ARM
//      (unless the start address was set to that of the config ROM)

const hss1394::uint48 FLASH_COMMAND_SET_CTRL(0xFFFF, 0xF009001C);
// WR:  A write to this address specifies a non-standard command set
// for the bootstrap loader to use
// RD:  Reads of this address are not supported

const hss1394::uint48 FLASH_ERASE_CTRL(0xFFFF, 0xF0090000);
// WR:  The sector addressed by the quadlet data is sent an erase request
// RD:  Returns 0x00000000 while erasing and 0x00000001 when erase complete

const hss1394::uint48 ADDITIONAL_CONFIG_DATA(0xFFFF, 0xF0090014);
//????

// Magic numbers and sequences
//-----------------------------------------------------------------------------
const hss1394::uint32 FORCE_FLASH_ENTER_CODE	= 'Frce';
const hss1394::uint32 ERASE_ALL_SECTORS			= 0xFFFFFFFF;
const hss1394::uint32 CONTINUE_EXTRACTING		= 0xFFFFFFFF;
const hss1394::uint32 UPLOAD_MODE_ENTER_CODE	= 'Edit';
const hss1394::uint32 UPLOADER_RUNNING_FW971	= 0x39373100;  // "971\0"
const hss1394::uint32 UPLOAD_CONTROL_RESET		= 0x80000000;  // Writing this resets ARM
const hss1394::uint32 CODE_OXFW912_FLASH_UNLOCK = 0x554C434B; // 'ULCK' Unlocks the flash
const hss1394::uint32 SIG_FW971					= 0x97100000;
const hss1394::uint32 GET_TIMESTAMP_CODE		= 0xFFFFFFF0;

const hss1394::uint32 OXFW912_UNLOCK_SEQ[] = {
	0x4000308C, 0x6800308C, 0x40003080, 0x68003080,
	0x40003088, 0x68003088, 0x40000860, 0x68000860,
	0x4000086C, 0x6800086C, 0x40000864, 0x68000864,
	0x40000868, 0x68000868, 0x00000000 
};

const hss1394::uint32 OXFW912_LOCK_SEQ[] = {
	0x4000308C, 0x6800308C, 0x40003080, 0x68003080, 
	0x40003088, 0x68003088, 0x40000860, 0x68000860,
	0x4000086C, 0x6800086C, 0x40000864, 0x68000864,
	0x40000828, 0x68000828, 0x00000000
};

const hss1394::uint32 ARM_RESET_SEQ[] = {
	0x80000000, 0x00000000
};

const hss1394::uint32 OXFW912_ERASE_SEQ[] = {
	0x00000020, 0x30000020, 0x4000AAAA,		// Byte address in v[2]
	0x000000D0, 0x300000D0,		// Wait 50 ms after this quad
	0x000000E0,	0x300000E0,		// Wait 60 us after this quad
	0x00000000
};

const hss1394::uint32 OXFW912_ERASE_ALL_SEQ[] = { 
	0x00000030, 0x30000030, 0x00000030, 0x30000030,		// Wait 50 ms after this quad
	0x000000E0, 0x300000E0,		// Wait 60 us after this quad
	0x00000000
};

const hss1394::uint32 OXFW912_PROG_SEQ[] = {
	0x00000010, 0x30000010, 0x4000AAAA,     // Byte address
	0x0001DDDD,     // DATA - upper 16 bits of data
	0x3000DDDD,     // DATA - lower 16 bits of data
	0x00000000 
};


#endif // _OXFORD1394DEFINITIONS_


