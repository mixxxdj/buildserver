///////////////////////////////////////////////////////////////////////////////
//! \file 1394API.cpp
//!
//! Cross-platform class implementations used in 1394API layer.
//! 
//! Created by Don Goodeve (don@bearanascence.com), 18Sep2008, 13:01PST
//! this file Copyright (C) 2009, Stanton Group.
//! $Id: 1394API.cpp 1561 2009-04-16 06:34:40Z don $
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


//*** Includes
//*****************************************************************************
#include <string>			// STL string class
#include <list>				// STL list class
#include "HSS1394Types.h"
#include "1394API.h"


//*** Types
//*****************************************************************************

//*** Functions
//*****************************************************************************

//*** Method definitions
//*****************************************************************************
namespace hss1394 {

	// NodeInfo::
	// Abstract Node information class.
	//*************************************************************************
	// Null constructor
	NodeInfo::NodeInfo(void) :
		msName("Unset"), msMfr("Unset"), muNetworkId(0xFF), mbInstalled(false) {
	}
	
	// Flat constructor
	NodeInfo::NodeInfo(uint64 uGUID, std::string sName, std::string sMfr, uint8 uNetworkId) :
		muGUID(uGUID), msName(sName), msMfr(sMfr), muNetworkId(uNetworkId), mbInstalled(false) {
	}

	// Copy constructor
	NodeInfo::NodeInfo(const NodeInfo *pObject) {
		if (pObject != NULL) {
			muGUID = pObject->muGUID;
			msName = pObject->msName; 
			msMfr = pObject->msMfr;
			muNetworkId = pObject->muNetworkId;
			mbInstalled = pObject->mbInstalled;
		} else {
			muGUID.mu32High = 0x00000000;
			muGUID.mu32Low = 0x00000000;
			muNetworkId = 0x00;
			msName = "Uninitialized";
			msMfr = "Uninitialized";
			mbInstalled = false;
		}
	}

	// Protected - set installed status
	void NodeInfo::setInstalled(bool bInstalled) {
		mbInstalled = bInstalled;
	}

	// Returns true if all necassary driver installation for this device is complete.
	bool NodeInfo::isInstalled(void) const {
		return mbInstalled;
	}

	// Return Global Unique Identifier of node
	uint64 NodeInfo::getGUID(void) const {
		return muGUID;
	}

	// Return textual name of node
	const std::string &NodeInfo::getName(void) const {
		return msName;
	}

	// Return textual name of manufacturer
	const std::string &NodeInfo::getMfr(void) const {
		return msMfr;
	}

	// Return Network ID of node. Note that this can change across bus reset events.
	uint8 NodeInfo::getNetworkId(void) const {
		return muNetworkId;
	}

	// Returns true if pNode refers to the same actual node. Useful across bus resets
	// to allow record replacement to occur.
	bool NodeInfo::idMatch(const NodeInfo *pNode) const {
		uint64 uGUID = pNode->getGUID();
		return ((uGUID.mu32Low == muGUID.mu32Low) && 
				(uGUID.mu32High == muGUID.mu32High));
	}
	
	
	// Handler::
	// Default implementation of handler class methods. User should override as
	// required.
	//*************************************************************************
	// Null Bus Reset handlers.
	void Handler::BusResetStarted(void) {
	}

	// Null Bus Reset handlers.
	bool Handler::BusResetCompleted(uint uGeneration) {
		return false;
	}

	// Null read request handler.
	bool Handler::ReadRequest(uint8 *pBytes, uint uSize) {
		return false;
	}

	// Null write request handler.
	bool Handler::WriteRequest(const uint8 *pBytes, uint uSize) {
		return false;
	}

};



