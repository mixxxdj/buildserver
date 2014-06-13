///////////////////////////////////////////////////////////////////////////////
//! \file 1394API.cpp
//!
//! C++ API for low-level 1394 asynchronous services. This forms the portability
//! layer of the HSS1394 implementation. See accompanying UML documentation for
//! additional details.
//! 
//! Created by Don Goodeve (don@bearanascence.com), 5Jun2008, 16:20PST
//! this file Copyright (C) 2009, Stanton Group.
//! $Id: 1394API.h 1561 2009-04-16 06:34:40Z don $
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


#ifndef _1394API_
#define _1394API_


//*** Includes
//*****************************************************************************
#include <string>			// STL string class
#include <list>				// STL list class
#include "HSS1394Types.h"


//*** Types
//*****************************************************************************

//*** Functions
//*****************************************************************************
namespace hss1394 {
	//! Deschedule this thread for the specified time. Useful cross-platform
	//! code.
	extern
	void SleepCPU(uint uMilliseconds);

	//! Factory for Lock objects. 
	class Lock;
	extern
	Lock *CreateLock(void);

	//! Millisecond wallclock timer. Implemented in 1394API_<platform>.cpp
	extern
	uint GetMilliseconds(void);
};


//*** Class declarations
//*****************************************************************************
namespace hss1394 {
	// Lock
	//! Portable lock for mutexing/monitoring.
	//-----------------------------------------------------------------------------
	class Lock {
		public:
			virtual ~Lock() { };
			virtual void Enter(void) = 0;
			virtual void Exit(void) = 0;
	};

	// NodeInfo::
	//! Abstract Node information class.
	//-----------------------------------------------------------------------------
	class NodeInfo {
		protected:
			uint64 muGUID;			//!< Node ID
			std::string msName;		//!< Node name
			std::string msMfr;		//!< Node manufacturer
			uint8 muNetworkId;		//!< Node current network ID
			bool mbInstalled;		//!< Flag stating whether node has driver support
			
			NodeInfo(void);		//!< Used by subclass constructors...

			//! Standard constructor
			NodeInfo(uint64 uGUID, std::string sName, std::string sMfr, uint8 uNetworkId);

			//! Set installed status flag
			void setInstalled(bool bInstalled);

		public:
			//! Copy constructor
			NodeInfo(const NodeInfo *pObject);

			//! Returns true if all necassary driver installation for this device is complete.
			virtual bool isInstalled(void) const ;

			//! Return Global Unique Identifier of node
			virtual uint64 getGUID(void) const;

			//! Return textual name of node
			virtual const std::string &getName(void) const;

			//! Return textual name of manufacturer
			virtual const std::string &getMfr(void) const;

			//! Return Network ID of node. Note that this can change across bus reset events.
			virtual uint8 getNetworkId(void) const;

			//! Returns true if pNode refers to the same actual node. Useful across bus resets
			//! to allow record replacement to occur.
			virtual bool idMatch(const NodeInfo *pNode) const;
	};


	// Handler::
	// Handler deals with asynchronous notifications from the 1394 interface. Note
	// that in the implementation asynchrony should be assumed; consideration of
	// concurrent access protection is required.
	//-----------------------------------------------------------------------------
	class Handler {
		public:
			//! Handle bus reset start
			virtual void BusResetStarted(void);
			//! Handle bus reset completion
			virtual bool BusResetCompleted(uint uGeneration);

			//! Handle an asynchronous read request to this handler.
			virtual bool ReadRequest(uint8 *pBytes, uint uSize);

			//! Handle an asynchronous write request to this handler.
			virtual bool WriteRequest(const uint8 *pBytes, uint uSize);
	};


	// _1394API::
	//! Static object for accessing 1394 asynchronous services.
	//-----------------------------------------------------------------------------
	class _1394API {
		public:
			// TErrorCode
			//! Enumerated type of error codes that the API can generate.
			//-------------------------------------------------------------------------
			typedef enum {
				kNoError = 0,
				kExistentiallyChallenged,
				kCannotOpenDefaultDevice,
				kAlreadyStarted,
				kNotStarted,
				kBadNodeId,
				kNoHandlerSpecified,
				kAsyncTimeout,
				kNonExistentNode,
				kIntermediateBufferTooSmall,
				kHandlerAddressConflict,
				kHandlerInitializationFailed,
				kHandlerAllocationFailed,
				kNoSuchHandler,
				kNoDefaultDevice,
				kConfigROMReadFailed,
				kCannotOpenLocalNode,
				kCannotTurnOnNotification,
				kCannotSetWriteHandler,
				kVirtualDeviceNotOpen,
				kNoDevice,
				kHandlerAlreadyInstalled
			}TErrorCode;

			// ::Start
			//! Commence the operation of the API class. Required call before any other
			//! interface calls become functional.
			static bool Start(void);

			// ::Stop
			//! Cease operation of the API class. After this call all calls become
			//! non-functional until another ::Start call is issued.
			static bool Stop(void);

			// ::BusReset
			//! Cause a bus reset to occur. This will imply that any registered bus
			//! reset handler will be called.
			static bool BusReset(void);

			// ::GetGeneration
			//! Return the current bus generation number. Note that this increments on every
			//! bus reset event, wherever that is initiated on the bus.
			static uint GetGeneration(void);

			// ::AddBusResetHandler
			//! Add a bus reset handler. If the argument is NULL, any existing reset handler
			//! will be removed.
			static bool AddBusResetHandler(Handler *pHandler);

			// ::GetLastError
			//! Return a code for the last error encountered in the API and reset the error
			//! code to TErrorCode::kNoError
			static TErrorCode GetLastError(void);

			// ::GetNetworkInfo
			//! Return a description of the network as a list of NodeInfo objects. No 
			//! information should be inferred from the ordering of the return list. After
			//! using the list, the caller should release the list by calling ::ReleaseNetworkInfo.
			static std::list<const NodeInfo *> GetNetworkInfo(void);

			// ::ReleaseNetworkInfo
			//! Return storage responsibility for the list returned by GetNetworkInfo to
			//! _1394API implementation. This is a required call after using the list
			//! provided by ::GetNetworkInfo
			static void ReleaseNetworkInfo(std::list<const NodeInfo *> &lDevicesList);

			// ::GetSelfId
			//! Return my network Id.
			static uint8 GetSelfId(void);

			// ::ReadBlockAsync
			//! Perform an asynchronous read across the network to the specified node. Blocks until
			//! the read completes either populating pData or returning 'false'. If uSize is 4 
			//! performs a 'quadlet read'.
			static bool ReadBlockAsync(uint8 uNodeId, uint48 uAddress, uint8 *pData, uint uSize);

			// ::WriteBlockAsync
			//! Perform an asynchronous write across the network to the specified node. If the write
			//! does not complete, returns 'false' and a failure code is generated. 
			static bool WriteBlockAsync(uint8 uNodeId, uint48 uAddress, const uint8 *pData, uint uSize);

			// ::AddHandler
			//! Add an address handler object for both read and write. The call returns the 48-bit
			//! 1394 base address of the handler. Note this is the only cross-platform (mac/PC) means
			//! of doing this, as OSX does not allow pseudo address spaces created on 1394 to be
			//! targetted. Only one handler is permitted to be created via this interface. On failure,
			//! bSuccess is set to false on return, true otherwise.
			static uint48 AddHandler(uint8 uNodeId, Handler *pHandler, bool &bSuccess);

			// ::RemoveHandler
			//! Remove the handler (if any) for a specified node and return it to the caller.
			static Handler *RemoveHandler(uint8 uNodeId);
	};

};	// namespace hss1394


#endif // _1394API_


