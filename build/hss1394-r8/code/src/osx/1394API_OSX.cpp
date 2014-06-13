///////////////////////////////////////////////////////////////////////////////
//! \file 1394API_OSX.cpp
//!
//! C++ API for low-level 1394 asynchronous services. This implementation uses
//! OSX IOKit firewire support for OSX10.5 and later.
//! 
//! Created by Don Goodeve (don@bearanascence.com), 5Jun2008, 17:09PST
//! this file Copyright (C) 2009, Stanton Group.
//! $Id: 1394API_OSX.cpp 1561 2009-04-16 06:34:40Z don $
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
#include <string>				// STL string class
#include <list>					// STL list class

#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>		// Carbon multiprocessing support
#include <mach/mach_time.h>		// Mach timing utilities
#include <unistd.h>

#include <IOKit/IOKitLib.h>					// Kernel interface/1394 support
#include <IOKit/firewire/IOFireWireLib.h>
#include <IOKit/firewire/IOFireWireFamilyCommon.h>

#include <libkern/OSAtomic.h>	// Kernel wait-free/atomic support

#include <pthread.h>			// POSIX threading support
#include <mach/mach_time.h>		// Time functions

#include "ConfigRom.h"			// Configuration ROM parsing routines
#include "1394API.h"


//*** Local macros
//*****************************************************************************
#ifndef assert
	#define assert(x)	_ASSERT(x)
#endif

#define k					(1024)

#define MAX1394BUS			(64)		// Maximum nodes on a single 1394 bus

#define kAddressSpaceSize	(32)		// 32 bytes
#define kInputBufferSpace	(32*k)		// 512msec seconds worth at 2kHz

#define kMaxUnservicedPackets	(2*k)	// 1024msec worth at 2kHz


//*** Local Types
//*****************************************************************************
namespace hss1394 {
	
	// NetworkNode
	// Specialization of public NodeInfo class. Uses the services of the base
	// class and extends with actual platform specific device information.
	// Provides a degree of abstraction of a node.
	//-------------------------------------------------------------------------
	class NetworkNode : public NodeInfo {
		protected:
			UInt8 muNodeId;
			bool mbValid;
		
		public:
			NetworkNode(UInt8 uNodeId);
			bool isValid(void);
	};
	
	// Packet
	// Structured type holding a HSS1394 packet payload.
	//-------------------------------------------------------------------------
	class Packet {
		public:
			uint  muSourceNode;
			uint  muSize;
			uint8 *mpPayload;
			
			Packet(uint uSourceNode, const uint8 *pData, uint uSize) :
				muSourceNode(uSourceNode), muSize(uSize) {
				mpPayload = new uint8[muSize];
				memcpy(mpPayload, pData, muSize);
			}
			
			~Packet() {
				if (mpPayload != NULL) {
					delete []mpPayload; mpPayload = NULL;
				}
			}
	};
}


//*** Local class definitions
//*****************************************************************************
namespace hss1394 {
	// RealLock
	// Portable lock for mutexing/monitoring.
	//-------------------------------------------------------------------------
	class RealLock : public Lock {

		private:
			MPSemaphoreID mtSemaphore;
			
		public:
			RealLock(void) {
				MPCreateSemaphore(
					(MPSemaphoreCount)1,
					(MPSemaphoreCount)1, 
					&mtSemaphore
				);
			}

			virtual ~RealLock() {
				MPDeleteSemaphore(mtSemaphore);
			}

			virtual void Enter(void) {
				MPWaitOnSemaphore(mtSemaphore, kDurationForever);
			}

			virtual void Exit(void) {
				MPSignalSemaphore(mtSemaphore);
			}

		friend Lock *CreateLock(void);
	};

};	// namespace hss1394


//*** Local Variables
//*****************************************************************************
// sbActive
// True if class is currently active (between successful ::Start and ::Stop calls).
//-----------------------------------------------------------------------------
static bool sbActive = false;

// suBusGeneration
// Current bus generation.
//-----------------------------------------------------------------------------
static UInt32 suBusGeneration = 0xffffffff;

// suLocalNodeId
// Network Id of local node.
//-----------------------------------------------------------------------------
static UInt16 suLocalNodeId = 0xffff;

// stErrorCode
// Current class error status code.
//-----------------------------------------------------------------------------
hss1394::_1394API::TErrorCode stErrorCode = hss1394::_1394API::kNoError;

// spBusResetHandler
// Array of all available VHPD1394 devices
//-----------------------------------------------------------------------------
static hss1394::Handler *spBusResetHandler = NULL;

// slPacketList, smPacketListMutex, scPacketListCondition
// List of arriving data packets. The kernel-service thread is separated
// from the application service thread by using this list (queue). Also
// the protection mutex and signalling condition variable.
//-----------------------------------------------------------------------------
static std::list<hss1394::Packet*> slPacketList;
pthread_mutex_t smPacketListMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t scPacketListCondition = PTHREAD_COND_INITIALIZER;

// sbRunUpcallThread, sbUpcallThreadRunning
// Thread marshalling incomming packets to their handlers (decoupling kernel
// service from application service).
//-----------------------------------------------------------------------------
static volatile bool sbRunUpcallThread = false;
static volatile bool sbUpcallThreadRunning = false;

// spDispatchRunLoopReference, sbDipatchThreadRunning
// Thread/dispatch structure variables - asynchronous traffic handling.
//-----------------------------------------------------------------------------
static CFRunLoopRef spDispatchRunLoopReference = 0;
static bool sbDispatchThreadRunning = false;
	
// cfPlugInInterface, stLocalNodeServiceAddress, spLocalNodeDeviceInterface
// Local 1394 Node service address and interface reference.
//-----------------------------------------------------------------------------
IOCFPlugInInterface **scfPlugInInterface = NULL;
io_service_t stLocalNodeServiceAddress = NULL;
IOFireWireLibDeviceRef spLocalNodeDeviceInterface = NULL;
io_object_t stLocalNodeDevice = 0;

// sbBusResetActive
// Set true whilst a bus reset event is in process.
//-----------------------------------------------------------------------------
static bool sbBusResetActive = false;

// stAllocatedAddress, slHandlersList
// Address space handler variables.
//-----------------------------------------------------------------------------
static FWAddress stAllocatedAddress;
static std::list<std::pair<UInt16, hss1394::Handler*> > slHandlersList;
static hss1394::RealLock stHandlersListLock;


//*** Local functions
//*****************************************************************************
// localBusResetHandler
// Called by firewire when a bus reset is in process.
//-----------------------------------------------------------------------------
static void localBusResetHandler(IOFireWireLibDeviceRef rIterface, FWClientCommandID tCommandId) {
	sbBusResetActive = true;
	hss1394::Handler *pHandler = spBusResetHandler;
	if (pHandler != NULL) {
		pHandler->BusResetStarted();
	}
}

// localBusResetDoneHandler
// Called by firewire when a bus reconfiguration has completed.
//-----------------------------------------------------------------------------
static void localBusResetDoneHandler(IOFireWireLibDeviceRef rIterface, FWClientCommandID tCommandId) {
	static uint suLastGeneration = 0x0;
//	static bool sbResetForced = false;

	// Update bus generation parameter. Only proceed to BusReset handling iff there has been a
	// generation change detected.
	(*rIterface)->GetBusGeneration(rIterface, &suBusGeneration);
	if(suBusGeneration == suLastGeneration) {
		return;
	}
	suLastGeneration = suBusGeneration;
	
	// Recover the (new) localNode Id
	UInt16 uNodeId;
	(*rIterface)->GetLocalNodeIDWithGeneration(rIterface, suBusGeneration, &uNodeId);
	suLocalNodeId = uNodeId & 0x3f;	// Only lower 6 bits required
	
	// Recovery - call any registered bus reset handler
	sbBusResetActive = false;
	hss1394::Handler *pHandler = spBusResetHandler;
	if (pHandler != NULL) {
		pHandler->BusResetCompleted(suBusGeneration);
	}
	
	// Correctly handled
	(*rIterface)->ClientCommandIsComplete(rIterface, tCommandId, kIOReturnSuccess);
}

// localWaitForBusResetCompletion
// Stall until bus reset event has completed (if one in process).
//-----------------------------------------------------------------------------
static void localWaitForBusResetCompletion(void) {
	while(true == sbBusResetActive) {
		hss1394::SleepCPU(10);
	}
}


// localCloseBus
// Clean up the bus devices data structures. Closes and deletes all open
// devices.
//-----------------------------------------------------------------------------
static bool localCloseBus(void) {
	if (spLocalNodeDeviceInterface != NULL) {
		(*spLocalNodeDeviceInterface)->Close(spLocalNodeDeviceInterface);
		(*spLocalNodeDeviceInterface)->RemoveCallbackDispatcherFromRunLoop(spLocalNodeDeviceInterface);
		(*spLocalNodeDeviceInterface)->Release(spLocalNodeDeviceInterface);
		spLocalNodeDeviceInterface = NULL;
		IOObjectRelease(stLocalNodeDevice);
		stLocalNodeDevice = 0;
		if (scfPlugInInterface != NULL) {
			IODestroyPlugInInterface(scfPlugInInterface);
			scfPlugInInterface = NULL;
		}
	}
	stAllocatedAddress.nodeID = 0x0;
	stAllocatedAddress.addressHi = 0x0;
	stAllocatedAddress.addressLo = 0x0;
	
	return true;
}


// localOpenBus
// Open 1394 local node to give access to the 1394 bus services. Can then
// make calls through (*spLocalNodeDeviceInterface) IOFireWireLibDeviceRef.
//-----------------------------------------------------------------------------
static bool localOpenBus(void) {
	using namespace hss1394;
	
	// Open the local node device. This is all we need access to to talk to all nodes
	// on the bus. This is a different model from the PC where we needed to open all
	// node devices. Here, effectively we are opening the bus as a device and using
	// its services directly.
	IOReturn tError	= kIOReturnSuccess;
	mach_port_t	tMasterPort ;
	tError = ::IOMasterPort(MACH_PORT_NULL, &tMasterPort);
	
	if (kIOReturnSuccess == tError) {
		stLocalNodeDevice = IOServiceGetMatchingService(tMasterPort, IOServiceMatching("IOFireWireLocalNode"));
		SInt32 theScore ;
		tError = ::IOCreatePlugInInterfaceForService(
													 stLocalNodeDevice,
													 kIOFireWireLibTypeID, kIOCFPlugInInterfaceID,
													 &scfPlugInInterface, &theScore);
	}
	
	if (kIOReturnSuccess == tError) {
		(*scfPlugInInterface)->QueryInterface(scfPlugInInterface, 
											  CFUUIDGetUUIDBytes(kIOFireWireDeviceInterfaceID),
											  (void**)&spLocalNodeDeviceInterface);
	}
	
	if (kIOReturnSuccess == tError) {
		tError = (*spLocalNodeDeviceInterface)->Open(spLocalNodeDeviceInterface);
	}
	
	if (kIOReturnSuccess == tError) {
		(*spLocalNodeDeviceInterface)->GetBusGeneration(spLocalNodeDeviceInterface, &suBusGeneration);
		UInt16 uNodeId;
		(*spLocalNodeDeviceInterface)->GetLocalNodeIDWithGeneration(spLocalNodeDeviceInterface, suBusGeneration, &uNodeId);
		suLocalNodeId = uNodeId & 0x3f;	// Only lower 6 bits required
		
		return true;
	}
	
	return false;	// I get here, something failed.
}


// local1394WriteHandler (producer in context of localDispatchThread RunLoop)
// Callback for arrival/processing of 1394 packet.
//-----------------------------------------------------------------------------
static
#ifdef OSX10_4
UInt32 local1394WriteHandler(
							 IOFireWireLibPseudoAddressSpaceRef pAddressSpace,
							 FWClientCommandID tCommandID,
							 UInt32 uDataLength, void* pData,
							 UInt16 uNodeId, UInt32 uAddressHi, 
							 UInt32 uAddressLo, UInt32 refCon)
#else
UInt32 local1394WriteHandler(
							 IOFireWireLibPseudoAddressSpaceRef pAddressSpace,
							 FWClientCommandID tCommandID,
							 UInt32 uDataLength, void* pData,
							 UInt16 uNodeId, UInt32 uAddressHi, 
							 UInt32 uAddressLo, void* refCon)
#endif
{
	// Remove Bus number data (ignored)
	uNodeId &= 0x3f;	
	
	// Enqueue packet, signal listening thread. If too many packets are outstanding,
	// silently discard the packet (limit resource usage).
	pthread_mutex_lock(&smPacketListMutex);
	if (slPacketList.size() < kMaxUnservicedPackets) {
		slPacketList.push_back(new hss1394::Packet(uNodeId, (uint8_t*)pData, uDataLength));
		pthread_cond_signal(&scPacketListCondition);		
	}
	pthread_mutex_unlock(&smPacketListMutex);
	
	// Accept the traffic
	(*pAddressSpace)->ClientCommandIsComplete(pAddressSpace, tCommandID, kIOReturnSuccess);
	return 0;
}

// local1394ReadHandler
// Callback for arrival/processing of 1394 packet.
//-----------------------------------------------------------------------------
static
UInt32 local1394ReadHandler(
	IOFireWireLibPseudoAddressSpaceRef pAddressSpace,
	FWClientCommandID tCommandID,
	UInt32 uPacketLen, UInt32 uPacketOffset,
	UInt16 uSrcNodeId, UInt32 uDestAddressHi, UInt32 uDestAddressLo, void* refCon) {
	
	// Do Nothin
	
	// Accept the traffic
	(*pAddressSpace)->ClientCommandIsComplete(pAddressSpace, tCommandID, kIOReturnSuccess);
	return 0;
}

// local1394SkippedPacketHandler
// Callback for arrival/processing of 1394 packet.
//-----------------------------------------------------------------------------
static
void local1394SkippedPacketHandler(
	IOFireWireLibPseudoAddressSpaceRef pAddressSpace,
	FWClientCommandID tCommandID,
	UInt32 uSkippedPacketCount) {
	
	printf("!!!!!SKIPPED PACKET!!!!! (%d reported)\n", uSkippedPacketCount);
	(*pAddressSpace)->ClientCommandIsComplete(pAddressSpace, tCommandID, kIOReturnSuccess);
}


// localDispatchThread
// POSIX thread function for 1394 traffic dispatcher.
//-----------------------------------------------------------------------------
static void *localDispatchThread(void *pArg) {
	// Get dispatch thread run loop reference
	spDispatchRunLoopReference = CFRunLoopGetCurrent();

	// Flag thread is now running
	sbDispatchThreadRunning = true;
	
	// Configure 1394 callbacks mechanism into my run loop
	(*spLocalNodeDeviceInterface)->AddCallbackDispatcherToRunLoop(spLocalNodeDeviceInterface, spDispatchRunLoopReference);

	// Install bus reset handlers
	(*spLocalNodeDeviceInterface)->SetBusResetHandler(spLocalNodeDeviceInterface, localBusResetHandler);
	(*spLocalNodeDeviceInterface)->SetBusResetDoneHandler(spLocalNodeDeviceInterface, localBusResetDoneHandler);
	(*spLocalNodeDeviceInterface)->TurnOnNotification(spLocalNodeDeviceInterface);
	
	// Ask user client to create a pseudo address space for receiving packets
	char *pBuffer = NULL; // new char[kAddressSpaceSize];
	IOFireWireLibPseudoAddressSpaceRef pPseudoAddressSpace = 0;
	pPseudoAddressSpace = (*spLocalNodeDeviceInterface)->CreatePseudoAddressSpace(
				spLocalNodeDeviceInterface, kAddressSpaceSize,
				(void*)pPseudoAddressSpace, kInputBufferSpace, pBuffer, 
				kFWAddressSpaceNoReadAccess,
				CFUUIDGetUUIDBytes(kIOFireWirePseudoAddressSpaceInterfaceID)
		);
		
	if (pPseudoAddressSpace != 0) {
		(*pPseudoAddressSpace)->GetFWAddress(pPseudoAddressSpace, &stAllocatedAddress);
		(*pPseudoAddressSpace)->SetWriteHandler(pPseudoAddressSpace, local1394WriteHandler);
		(*pPseudoAddressSpace)->SetSkippedPacketHandler(pPseudoAddressSpace, local1394SkippedPacketHandler);
		if (true == (*pPseudoAddressSpace)->TurnOnNotification(pPseudoAddressSpace)) {
			CFRunLoopRun() ;	// Terminated by external call to CFRunLoopStop (localStopDispatchThread)
		} else {
			stErrorCode = hss1394::_1394API::kCannotTurnOnNotification;
		}
	}
	
	// Post-run clean up
	(*spLocalNodeDeviceInterface)->RemoveCallbackDispatcherFromRunLoop(spLocalNodeDeviceInterface);
	delete []pBuffer;
	sbDispatchThreadRunning = false;
	return 0;	// Loop exit
}
	
	
// localUpcallThread (consumer)
// POSIX thread function for dispatching upcalls. This decouples the
// kernel-service thread from the application servicing thread. The aim is
// to ensure minimal kernel service time, even when application delays occur.
//-----------------------------------------------------------------------------
static void *localUpcallThread(void *pArg) {
	sbUpcallThreadRunning = true;
	
	while(true == sbRunUpcallThread) {
		hss1394::Packet *pPacket = NULL;
		hss1394::Handler *pHandler = NULL;
		
		// Packet queue access -> pPacket
		pthread_mutex_lock(&smPacketListMutex);
		do {
			if (false == slPacketList.empty()) {
				// Remove packet from queue - and we are done				
				pPacket = slPacketList.front();
				slPacketList.pop_front();
				
			} else if (true == sbRunUpcallThread) {
				// Wait on condition variable for the next enqueue event
				pthread_cond_wait(&scPacketListCondition, &smPacketListMutex);
			}
		}while((NULL == pPacket) && (true == sbRunUpcallThread));
		pthread_mutex_unlock(&smPacketListMutex);
		
		if (pPacket != NULL) {
//#define DEBUG_REPORT	(1)
#ifdef DEBUG_REPORT
			// DEBUG dump packet to stdout
			static uint uStartTime = hss1394::GetMilliseconds();
			static uint uWeirdCount = 0;
			static uint uErrorCount = 0;
			static uint uLastErrorTimestamp = 0;
			static uint uRxCount = 0;
			uRxCount++;
			if ((pPacket->muSize != 5) || (pPacket->mpPayload[0] != 0x00)) {
				printf("1394API BAD PACKET!!!! (%d): ", ++uWeirdCount);
				uErrorCount++;
				uint uNow = hss1394::GetMilliseconds();
				uint uDelta = uNow - uLastErrorTimestamp;
				uLastErrorTimestamp = uNow;
				printf("delta %d msec: ", uDelta);
				for(int i=0; i<pPacket->muSize; i++) {
					printf("%02x ", pPacket->mpPayload[i]);
				}
				printf("\n");
			} else {
				uWeirdCount = 0;
			}
			if ((uRxCount % 2000) == 0) {
				printf("1394API %dmsec: %d packets received ok, %d errors\n", hss1394::GetMilliseconds()-uStartTime, uRxCount, uErrorCount);
			}
			//	printf("%08d: ", hss1394::GetMilliseconds());
			//	for(int i=0; i<uDataLength; i++) {
			//		printf("%02x ", ((const hss1394::uint8*)pData)[i]);
			//	}
			//	printf("\n");
			// end of DEBUG
#endif // DEBUG_REPORT			
			
			// Process packet - search for associated handler
			stHandlersListLock.Enter();
			for(std::list<std::pair<UInt16,hss1394::Handler*> >::const_iterator tIter = slHandlersList.begin();
				tIter != slHandlersList.end();
				tIter++) {
				if (pPacket->muSourceNode == (*tIter).first) {
					pHandler = (*tIter).second;
					break;
				}
			}
			stHandlersListLock.Exit();
			
			// Pass packet to handler (if found)
			if (pHandler != NULL) {
				pHandler->WriteRequest(pPacket->mpPayload, pPacket->muSize);
				// Finished with packet
				delete pPacket; pPacket = NULL;
			}
			
		} // (pPacket != NULL)
	} // (while (true == sbRunUpcallThread)
	sbUpcallThreadRunning = false;
	
	return 0;	// Thread exit
}


// localStartPacketHandler
// The packet handler consists of a kernel service thread and an upcall thread.
// These threads share a common list data structure protected by mutex and
// using a condition variable for signalling data arrival. This routine
// initializes the data structures and starts the packet handler subsystem.
//-----------------------------------------------------------------------------
static bool localStartPacketHandler(void) {
	// Initialize shared data

	// Start upcall thread
	sbRunUpcallThread = true;
		
	// Create and start the upcall (consumer) pthread using POSIX routines.
	pthread_attr_t  tAttr;
	pthread_t tPosixThreadID;
	int iError;
	iError = pthread_attr_init(&tAttr);
	iError |= pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_DETACHED);
	iError |= pthread_create(&tPosixThreadID, &tAttr, &localUpcallThread, NULL);
	iError |= pthread_attr_destroy(&tAttr);
	
    // Create and start the dispatch (producer) pthread.
	iError = pthread_attr_init(&tAttr);
	iError |= pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_DETACHED);
    iError |= pthread_create(&tPosixThreadID, &tAttr, &localDispatchThread, NULL);
	iError |= pthread_attr_destroy(&tAttr);
	
	// If returns true, thread is now running.
	return (0 == iError);
}

// localStopPacketHandler
// Cleanly kill the packet handling subsystem.
//-----------------------------------------------------------------------------
static bool localStopPacketHandler(void) {
	// Stop servicing of kernel (localDispatchThread)
	if (true == sbDispatchThreadRunning) {
		CFRunLoopStop(spDispatchRunLoopReference);
		while(true == sbDispatchThreadRunning) {
			hss1394::SleepCPU(10);
		}
	}
	
	// Stop upcall thread
	sbRunUpcallThread = false;	// Tell thread to stop
	while(true == sbUpcallThreadRunning) {
		pthread_cond_signal(&scPacketListCondition);
		hss1394::SleepCPU(10);
	}
	
	// Clean associated resources
	while(false == slPacketList.empty()) {
		hss1394::Packet *pPacket = slPacketList.front();
		slPacketList.pop_front();
		delete pPacket; pPacket = NULL;
	}
	
	return true;
}


//*** Global Functions
//*****************************************************************************
namespace hss1394 {
	// Deschedule this thread for the specified time. Useful cross-platform
	// code.
	void SleepCPU(uint uMilliseconds) {
		AbsoluteTime tEntryTime = UpTime();
		AbsoluteTime tExpiryTime = AddDurationToAbsolute(kDurationMillisecond*uMilliseconds, tEntryTime);
		MPDelayUntil(&tExpiryTime);
	}

	// Factory for Lock objects. 
	Lock *CreateLock(void) {
		return new RealLock();
	}
	
	uint GetMilliseconds(void) {
		uint64_t tNow = mach_absolute_time();
		Nanoseconds tAbsTimeNsec = AbsoluteToNanoseconds(*(AbsoluteTime*)&tNow);
		uint64_t uAbsTimeMsec = (*(uint64_t*)&tAbsTimeNsec) / 1000000;
		uAbsTimeMsec &= 0xFFFFFFFF;
		return uAbsTimeMsec;
	}
};


//*** Method definitions
//*****************************************************************************
namespace hss1394 {

#define K	(1024)

#define kConfigROMReadWords	(0x100)

	// NetworkNode::
	// Node abstraction class.
	//-------------------------------------------------------------------------
	// ::NetworkNode
	// Open device, populate data, close device. Will stall if a bus reset is in process.
	NetworkNode::NetworkNode(UInt8 uNodeId) :
	muNodeId(uNodeId), mbValid(false) {
		muNetworkId = uNodeId;
		mbInstalled = true;
					
		UInt32 pConfigROM[kConfigROMReadWords] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		UInt32 *pConfigROMQuadletPtr = pConfigROM;
		UInt16 uFullNodeId = 0xffc0 | (uNodeId & 0x3F);
		FWAddress cConfigROMAddr(0xffff, 0xf0000400, uFullNodeId);
		
		// Wait for stable bus
		localWaitForBusResetCompletion();	// Stalls during a bus reset event
		
		// Attempt to read first quadlet of device configuration ROM. Fail = no device there...
		if (spLocalNodeDeviceInterface != NULL) {
			IOReturn tError	= (*spLocalNodeDeviceInterface)->ReadQuadlet(
				spLocalNodeDeviceInterface, 0x0,
				&cConfigROMAddr, pConfigROMQuadletPtr, true, suBusGeneration);		
			if (0 == tError) {
				// Read as far as I can before failure.
				uint uWordsRemaining = kConfigROMReadWords;
				do {
					pConfigROMQuadletPtr++;
					cConfigROMAddr.addressLo += 4;
					tError	= (*spLocalNodeDeviceInterface)->ReadQuadlet(
						spLocalNodeDeviceInterface, 0x0,
						&cConfigROMAddr, pConfigROMQuadletPtr, true, suBusGeneration);
					uWordsRemaining--;
				}while((0 == tError) && (uWordsRemaining > 0));
				
				// Interpret what I have read
				uint uWordsRead = kConfigROMReadWords - uWordsRemaining;
				ParseConfigRom((uint32*)pConfigROM, uWordsRead, msName, msMfr, muGUID);
				mbValid = true;
				
			} else {
				stErrorCode = _1394API::kConfigROMReadFailed;			
				// Attempt check for blank Oxford device to at least provide some ID
				UInt32 u32Buffer;
				FWAddress cOxfordAddress(0xffff, 0xf0090020, uFullNodeId);
				tError = (*spLocalNodeDeviceInterface)->ReadQuadlet(
					spLocalNodeDeviceInterface, 0x0,
					&cOxfordAddress, &u32Buffer, true, suBusGeneration);
				if (0 == tError) {
					uint8 pBuffer[5];
					u32Buffer = u32Buffer;
					memcpy(pBuffer, (const uint8*)&u32Buffer, sizeof(u32Buffer));
					pBuffer[4] = 0x0;
					msName = (const char *)pBuffer;
					msMfr = "Oxford";

					mbValid = true;
					
				} else {
					// This is an unknown unknown - all data left at defaults.
				}
			}
		}
	}
	
	// ::isValid
	// Has node creation completed successfully?
	bool NetworkNode::isValid(void) {
		return mbValid;
	}
	

	// _1394API::
	// Static object for accessing 1394 asynchronous services.
	//-------------------------------------------------------------------------

	// Start the API. Initialize all internal data.
	//-------------------------------------------------------------------------
	bool _1394API::Start(void) {
		if (false == sbActive) {
			// Build initial devices information
			bool bOk = localOpenBus();
			
			// Passed - all now going...
			if (true == bOk) {
				// Start the 1394 packet handling subsystem
				localStartPacketHandler();
				sbActive = true;
				return true;

			} else {
				stErrorCode = kCannotOpenLocalNode;
				return false;
			}

		} else {
			// Try to not open twice
			stErrorCode = kAlreadyStarted;
			return false;
		}
	}

	// Cease operation of the API class. After this call all calls become
	// non-functional until another ::Start call is issued.
	// Correct synchronization on shDeviceListSemaphore/sbActive is
	// if (true == sbActive) {
	// 	 // ... _1394API function code
	// }
	// This works as sbActive is only changed inside the Start/Stop critical
	// section and hence is atomic wrt. the state of sp1394Devices. If sbActive
	// is detected as true therefore - the list is valid. If it is false, the
	// list is invalid.
	//-----------------------------------------------------------------------------
	bool _1394API::Stop(void) {
		if (true == sbActive) {
			localStopPacketHandler();
			localCloseBus();
			sbActive = false;
			return true;
		} else {
			stErrorCode = kNotStarted;
			return false;
		}
	}

	// Cause a bus reset to occur. This will imply that any registered bus
	// reset handler will be called.
	//-----------------------------------------------------------------------------
	bool _1394API::BusReset(void) {
		if (false == sbActive) {
			stErrorCode = kNotStarted;
			return false;
		}

		IOReturn tStatus = (*spLocalNodeDeviceInterface)->BusReset(spLocalNodeDeviceInterface);

		SleepCPU(2000);	// Settling time

		return (tStatus != 0);
	}

	// Return the current bus generation number. Note that this increments on every
	// bus reset event, wherever that is initiated on the bus. This returns the
	// cached value. This is updated by the BusResetDone handler.
	//-----------------------------------------------------------------------------
	uint _1394API::GetGeneration(void) {
		if (false == sbActive) {
			stErrorCode = kNotStarted;
			return false;
		}
		
		return suBusGeneration;
	}

	// Add a bus reset handler. If the argument is NULL, any existing reset handler
	// will be removed.
	//-----------------------------------------------------------------------------
	bool _1394API::AddBusResetHandler(Handler *pHandler) {
		spBusResetHandler = pHandler;
		return true;
	}

	// Return a code for the last error encountered in the API and reset the error
	// code to TErrorCode::kNoError
	//-----------------------------------------------------------------------------
	_1394API::TErrorCode _1394API::GetLastError(void) {
		TErrorCode tError = stErrorCode;
		stErrorCode = kNoError;
		return tError;
	}

	// Return a description of the network as a list of NodeInfo objects. The storage
	// of these objects is the responsibility of the caller. No information should be
	// implied from the ordering of the return list.
	//-----------------------------------------------------------------------------
	std::list<const NodeInfo*> _1394API::GetNetworkInfo(void) {
		std::list<const NodeInfo*> lDevicesList;
		
		// Scan all possible nodes on bus. Build lDevicesList from those
		// that can be found.
		for(UInt8 uNode = 0; uNode < /*0x40*/10; uNode++) {
			if (uNode != suLocalNodeId) {
				NetworkNode *pNode = new NetworkNode(uNode);
//const char *pName = (pNode->getName()).c_str();
//uint64 uGUID = pNode->getGUID();
//printf("\t\t ::GetNetworkInfo %d(%s)=> 0x%04x%08x<%s>\n", uNode, pNode->isValid()?"good":"bad",uGUID.u32High&0xFFFF, uGUID.u32Low, pName);				
				if (true == pNode->isValid()) {
					lDevicesList.push_back((const NodeInfo*)pNode);
				} else {
					delete pNode; pNode = NULL;
					break;	// End of valid devices - quit now
				}
			}
		}

		return lDevicesList;
	}

	// Release the network info structure returned from ::GetNetworkInfo.
	//-----------------------------------------------------------------------------
	void _1394API::ReleaseNetworkInfo(std::list<const NodeInfo*> &lDevicesList) {
		std::list<const NodeInfo *>::iterator tIter;
		for(tIter = lDevicesList.begin(); tIter != lDevicesList.end(); tIter++) {
			delete (*tIter); (*tIter) = NULL;
		}
		lDevicesList.clear();
	}

	// Return my network Id.
	//-----------------------------------------------------------------------------
	uint8 _1394API::GetSelfId(void) {
		if (false == sbActive) {
			stErrorCode = kNotStarted;
			return 0xFF;
		}
		return suLocalNodeId;
	}

	// ::ReadBlockAsync
	// Perform an asynchronous read across the network to the specified node. Blocks until
	// the read completes either populating pData or returning 'false'. If uSize is 4 
	// performs a 'quadlet read'.
	//-----------------------------------------------------------------------------
	bool _1394API::ReadBlockAsync(uint8 uNodeId, uint48 uAddress, uint8 *pData, uint uSize) {
		if (false == sbActive) {
			stErrorCode = kNotStarted;
			return false;
		}
		
		UInt16 uFullNodeId = 0xffc0 | (uNodeId & 0x3F);
		FWAddress cTarget1394Address(uAddress.mu16High, uAddress.mu32Low, uFullNodeId);
		UInt32 u32Size = uSize;
		IOReturn tError	= (*spLocalNodeDeviceInterface)->Read(
								spLocalNodeDeviceInterface, 0x0, &cTarget1394Address, 
								(void*)pData, &u32Size, true, suBusGeneration);
						
		return (0 ==  tError);
	}

	// ::WriteBlockAsync
	// Perform an asynchronous write across the network to the specified node. If the write
	// does not complete, returns 'false' and a failure code is generated. If uSize is 4 performs
	// a 'quadlet write'.
	//-----------------------------------------------------------------------------
	bool _1394API::WriteBlockAsync(uint8 uNodeId, uint48 uAddress, const uint8 *pData, uint uSize) {
		if (false == sbActive) {
			stErrorCode = kNotStarted;
			return false;
		}
		UInt16 uFullNodeId = 0xffc0 | (uNodeId & 0x3F);
		FWAddress cTarget1394Address(uAddress.mu16High, uAddress.mu32Low, uFullNodeId);
		UInt32 u32Size = uSize;
		//void				(*SetMaxRetryCount)(IOFireWireLibCommandRef self, UInt32 count ); ???
		IOReturn tError	= (*spLocalNodeDeviceInterface)->Write(
								spLocalNodeDeviceInterface, 0x0, &cTarget1394Address, 
								(const void*)pData, &u32Size, true, suBusGeneration);
						
		return (0 ==  tError);
	}

	// Add an address handler object for both read and write. The size of the address space
	// to be covered is specified. It is an error for pHandler to be NULL.
	//-----------------------------------------------------------------------------
	uint48 _1394API::AddHandler(uint8 uNodeId, Handler *pUserHandler, bool &bSuccess) {
		uint48 uReturn;
		uReturn.mu16Guard = 0x0;
		uReturn.mu16High = 0x0;
		uReturn.mu32Low = 0x0;
		bSuccess = false;		// Failure hypothesis until all is done...

		if (false == sbActive) {
			stErrorCode = kNotStarted;
			return uReturn;
		}
		if (NULL == pUserHandler) {
			stErrorCode = kNoHandlerSpecified;
			return uReturn;
		}
		
		// See if there is an entry for this node, if so replace it - if not add a new
		// entry.
		stHandlersListLock.Enter(); 
		{
			bool bFound = false;
			for(std::list<std::pair<UInt16,Handler*> >::iterator tIter = slHandlersList.begin();
			    tIter != slHandlersList.end(); tIter++) {
				if ((*tIter).first == uNodeId) {
					// Found in list, modify entry
					bFound = true;
					//Handler *pOldHandler = (*tIter).second;
					(*tIter).second = pUserHandler;
				}
			}
			if (false == bFound) {
				std::pair<UInt16,Handler*> tIdHandlerPair(uNodeId, pUserHandler);
				slHandlersList.push_front(tIdHandlerPair);			
			}
		}
		stHandlersListLock.Exit();

		// Prepare return data
		uReturn.mu16Guard = stAllocatedAddress.nodeID;
		uReturn.mu16High = stAllocatedAddress.addressHi;
		uReturn.mu32Low = stAllocatedAddress.addressLo;
		
		bSuccess = true;	// Phew! - we made it!
		return uReturn;
	}

	// Remove the handler (if any) on a specified node and return it to the caller. If there
	// is no such handler, the return value will be NULL.
	//-----------------------------------------------------------------------------
	Handler *_1394API::RemoveHandler(uint8 uNodeId) {
		if (false == sbActive) {
			stErrorCode = kNotStarted;
			return NULL;
		}
		
		// See if there is an entry for this node, if so remove it.
		Handler *pOldHandler = NULL;
		stHandlersListLock.Enter();
		{
			for(std::list<std::pair<UInt16,Handler*> >::iterator tIter = slHandlersList.begin();
				tIter != slHandlersList.end(); tIter++) {
				if ((*tIter).first == uNodeId) {
					// Found in list, remove entry
					pOldHandler = (*tIter).second;
					(*tIter).second = NULL;
					slHandlersList.erase(tIter);
					break;
				}
			}
		}
		stHandlersListLock.Exit();

		if (NULL == pOldHandler) {
			stErrorCode = kNoSuchHandler;
			return NULL;
		} else {
			return pOldHandler;
		}
	}


};	// namespace hss1394


