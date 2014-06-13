//*****************************************************************************
// Endian.h
//
// Conversion functions for 32-bit data to convert between big-endian and
// native formats. Cross-platform (PC/Mac Intel/PPC).
//
// Created on 5Jul2008, 17:46PST by don@bearanascence.com
// $Id: ConfigRom.h 1281 2008-10-22 19:28:26Z don $
// ----------------------------------------------------------------------------
// This file Copyright (C) 2008, Beara Nascence Inc. All Rights Reserved.
//*****************************************************************************

#ifndef _ENDIAN_
#define _ENDIAN_


//*** Includes
//*****************************************************************************
#ifndef _WIN32_
#include <CoreServices/CoreServices.h>
#endif


//*** Types
//*****************************************************************************


//*** Global functions
//*****************************************************************************
// bigEndianToNative32, nativeToBigEndian32
// Swap endian-ness of input 32-bit word.
//-----------------------------------------------------------------------------
#ifdef _WIN32_
static inline hss1394::uint32 endSwap(hss1394::uint32 uQuad) {
	return (((uQuad&0xff)<<24) | ((uQuad&0xff00)<<8) | ((uQuad&0xff0000)>>8) | ((uQuad&0xff000000)>>24));
}
#define bigEndianToNative32(x)	endSwap(x)
#define nativeToBigEndian32(x)	endSwap(x)
#else
#define bigEndianToNative32(x)	EndianU32_BtoN(x)
#define nativeToBigEndian32(x)	EndianU32_NtoB(x)
#endif

#endif // _ENDIAN_


