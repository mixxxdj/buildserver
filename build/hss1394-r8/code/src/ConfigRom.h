///////////////////////////////////////////////////////////////////////////////
//! \file ConfigRom.h
//!
//! Given a data array, interpret as a standard configuration ROM per
//! ISO/IEC 13213:1994. Does *not* perform CRC checks.
//! 
//! Created by Don Goodeve (don@bearanascence.com), 5Jul2008, 17:46PST
//! this file Copyright (C) 2009, Stanton Group.
//! $Id: ConfigRom.h 1561 2009-04-16 06:34:40Z don $
//-----------------------------------------------------------------------------
//! GNU Lesser Public License:
//! This program is free software: you can redistribute it and/or modify
//! it under the terms of the GNU Lesser General Public License as published 
//! by the Free Software Foundation, either version 3 of the License, or
//! (at your option) any later version.
//!
//! This program is distributed in the hope that it will be useful,
//! but WITHOUT ANY WARRANTY; without even the implied warranty of
//! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//! GNU Lesser General Public License for more details.
//!
//! You should have received a copy of the GNU Lesser General Public License
//! along with this program.  If not, see <http://www.gnu.org/licenses/>.
///////////////////////////////////////////////////////////////////////////////

#ifndef _CONFIGROM_
#define _CONFIGROM_


//*** Includes
//*****************************************************************************
#include <string>			// STL string class
#include "HSS1394Types.h"


//*** Types
//*****************************************************************************


//*** Global functions
//*****************************************************************************
namespace hss1394 {
	// ParseConfigRom
	// Given an array of data, attempts to extract the GUID, vendor and
	// device information strings.
	//-------------------------------------------------------------------------
	bool ParseConfigRom(const uint32 *puConfigRom, uint32 uWordsRead, 
						std::string &sName, std::string &sMfr, uint64 &uGUID);

};

#endif // _CONFIGROM_


