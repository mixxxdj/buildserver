///////////////////////////////////////////////////////////////////////////////
//! \file 1394API_VHPD1394.cpp
//!
//! C++ API for low-level 1394 asynchronous services. This implementation runs
//! on top of the VHPD1394 driver interface classes from Thesycon.
//! 
//! Created by Don Goodeve (don@bearanascence.com), 5Jun2008, 17:09PST
//! this file Copyright (C) 2009, Stanton Group.
//! $Id: 1394API_VHPD1394.cpp 1561 2009-04-16 06:34:40Z don $
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
#include <windows.h>
#include <process.h>			// Win32 threads

#include "vhpd1394.h"			// vhpd interface definitions
#include "ams1394defs.h"
#include "ams1394ioctl.h"		// Driver IOCTL definitions

#include "CVhpd.h"				// VHPD1394 Interface classes
#include "CVhpdDataSlave.h"
#include "CVhpdNotifySlave.h"

#include "ConfigRom.h"			// Configuration ROM parsing routines
#include "1394API.h"


//*** Local macros
//*****************************************************************************
#define assert(x)	_ASSERT(x)

#define MAX1394BUS	(64)			// Maximum nodes on a single 1394 bus

#define RESPONSETIMEOUT			(2)		// 2msec should be plenty...
#define kHousekeeperInterval	(200)	// Check state every ...
#define kBusResetSettleTime		(500)	// After detected change to let dust settle...
#define kPostResetSettleTime	(1000)	// After generation has settled, before rescan.


//*** Local Types
//*****************************************************************************


//*** Local class definitions
//*****************************************************************************
namespace hss1394 {
	// RealLock
	// Portable lock for mutexing/monitoring.
	//-----------------------------------------------------------------------------
	class RealLock : public Lock {
		private:
			HANDLE mhSemaphore;
			RealLock(void) {
				mhSemaphore = CreateSemaphore(NULL, 1, 1, NULL);
			}

		public:
			virtual ~RealLock() {
				CloseHandle(mhSemaphore);
			}

			virtual void Enter(void) {
				WaitForSingleObject(mhSemaphore, INFINITE);
			}

			virtual void Exit(void) {
				ReleaseSemaphore(mhSemaphore, 1, NULL);
			}

		friend Lock *CreateLock(void);
	};

	// CAsyncHandler
	// <: CVhpdNotifySlave <: CVhpd.
	// Embedding of a CVhpd device object, extended to provide asynchronous handling
	// support.
	//-----------------------------------------------------------------------------
	class CAsyncHandler : public CVhpdNotifySlave {
		public:
			CAsyncHandler(bool bStreaming);
			//CAsyncHandler(uint8 uNodeId, Handler *pHandler);
			~CAsyncHandler();

			// Open device, including setup for async reception.
			DWORD Open(int DeviceNumber, HDEVINFO DeviceList, const GUID* InterfaceGuid);

			// Attach a handler, return the previous handler (if any)
			Handler *AttachHandler(Handler *pHandler);

			// Deprecated 4Nov2008
			// Returns true if async handler object was initialized without error
			//bool IsOk(void);

			// Get monitoring data. Not reset.
			void GetInfo(unsigned int &uBytesTxd, unsigned int &uBytesRxd);

			// Deprecated 4Nov2008
			// Overlap testing. Returns true if there is a conflict.
			//bool Overlap(AsyncHandler *pHandler);

			// Device # testing
			uint8 NodeId(void);

			// Address testing.
			uint48 Address(void);

		protected:
			// CVhpdNotifySlave overload. 
			virtual BOOL ProcessNotification(CVhpdBuf* Buf);

		protected:
			// Handler functionality is initialized disabled.
			// 'Enable' must be called to permit through-flow.
			void Enable(void);
			void Disable(void);
		
		private:
			// Member variables
			//bool mbOk;
			bool mbOpen;
			uint8 muNodeAddr;
			bool mbActive;
			bool mbStreamingDevice;
			volatile bool mbHandlerActive;
			volatile bool mbCallIsActive;
			uint48 muAddress;
			uint   muBytes;
			unsigned int muBytesTxd;
			unsigned int muBytesRxd;
			Handler *mpHandler;

		friend class _1394API;
	};

	// MyNodeInfo::
	// Node information class.
	//-----------------------------------------------------------------------------
	class MyNodeInfo : public NodeInfo {
		protected:	
			uint16 muBusId;			// !!!! 1394 Bus Identifier

			// Friends-only access
			MyNodeInfo(uint64 uGUID, const std::string sName, const std::string sMfr, 
					   uint8 uNetworkId, bool bInstalled);

		public:

		// _1394API is the creator of these objects.
		friend static void localPushNodeInfo(
			int iTestNode, int iLocalNodeId, 
			const VHPD_LOCAL_HOST_INFO_DATA &tHostInfoData, 
			std::list<const NodeInfo *> &lList
		);
	};

};	// namespace hss1394


//*** Local Variables
//*****************************************************************************
// sp1394Devices, sb1394DeviceIsStreaming.
// Array of all available VHPD1394 devices, indexed by node # on bus. Also
// array of flags to indicate 'streaming' devices. Entries in this array
// are only valid if the corresponding sp1394Devices[] entry is valid.
//-----------------------------------------------------------------------------
static hss1394::CAsyncHandler *sp1394Devices[MAX1394BUS];

// spBusResetHandler
// Array of all available VHPD1394 devices
//-----------------------------------------------------------------------------
static hss1394::Handler *spBusResetHandler = NULL;

// stDriverID, stVirtualDeviceDriverID
// GUID for installed driver. Currently using VHPD1394 default GUID. To be
// customized for actual vendor.
//-----------------------------------------------------------------------------
const GUID stDriverStreamingID = ST1394OX_IID;
const GUID stDriverControlOnlyID = ST1394OX_C_IID;
const GUID stVirtualDeviceDriverID = ST1394OX_V_IID;

// spVirtualDevice
// Default VHPD device - provides access to bus resources even if there
// are no other 'installed' devices present. Permits bus scanning etc...
//-----------------------------------------------------------------------------
CVhpd *spVirtualDevice = NULL;

// sbActive
// True if _1394API class is currently active (between successful ::Start 
// and ::Stop calls).
//-----------------------------------------------------------------------------
static bool sbActive = false;

// sbHousekeeperScan
// Synchronization variable - set true to keep housekeeper alive.
//-----------------------------------------------------------------------------
static bool sbHousekeeperScan = false;

// sbHousekeeperRunning
// Signal variable to indicate housekeeper thread is active.
//-----------------------------------------------------------------------------
static bool sbHousekeeperRunning = false;

// sbReconfigurationActive, suCallsInProcess
// Signal variables providing an interlock on access to sp1394Devices
// array. This array is reconfigured on bus-reset events. The interlocks
// are required to prevent calls through bad pointers and the resultant
// chaos... Note that suCallsInProcess is only changed through windows
// atomic mechanisms (InterlockedXXX).
//-----------------------------------------------------------------------------
static volatile bool sbReconfigurationActive = false;
static LONG volatile siCallsInProcess = 0;

// sbDevicesInfoValid
// Signal variable to indicate housekeeper has completed its first scan
// cycle.
//-----------------------------------------------------------------------------
static bool sbHousekeeperFirstCycleComplete = false;

// suValidGeneration
// Generation for which spVirtualDevice and sp1394Devices are deemed to be
// valid. 0x0 is uninitialized, 0xffffffff is an error state.
//-----------------------------------------------------------------------------
static hss1394::uint suValidGeneration = 0x0;

// stErrorCode
// Current class error status code.
//-----------------------------------------------------------------------------
hss1394::_1394API::TErrorCode stErrorCode = hss1394::_1394API::kNoError;

// shDeviceListSemaphore
// Windows semaphore used to protect spDeviceList from concurrent access.
//-----------------------------------------------------------------------------
HANDLE shDeviceListSemaphore;

// suDefaultHandlerAddress, suDefaultHandlerBytes
// Default (ie. only) address used for an address space handler and its size.
//-----------------------------------------------------------------------------
static const hss1394::uint48 suDefaultHandlerAddress(0xcafe, 0x00000000);
static const hss1394::uint32 suDefaultHandlerBytes = MAX1394BUS;


//*** Local functions
//*****************************************************************************
// localDeviceAccessEntry
// Call before attempting to use sp1394Devices[] to prevent it disappearing
// underneath the call - wait-free interlock. If this call returns false, the
// interlock has failed - access to sp1394Devices should be prevented.
//-----------------------------------------------------------------------------
static bool localDevicesAccessEntry(void) {
	InterlockedIncrement(&siCallsInProcess);			// I am attempting to run
	if (false == sbReconfigurationActive) {				// Ok - all is well
		return true;									//    continue...
	} else {
		InterlockedDecrement(&siCallsInProcess);		// Uh-oh, reconfiguration - I must quit
		return false;									//    tell caller to cease and desist
	}
}

// localDevicesAccessExit
// Call once I have finished using sp1394Devices[] - corresponding with initial
// call to localDeviceAccessEntry.
//-----------------------------------------------------------------------------
static void localDevicesAccessExit(void) {
	// If this goes through zero, something is up
	InterlockedDecrement(&siCallsInProcess);	
}

// localWaitForExclusiveDevicesAccess
// Called by the (single) reconfigurer of the sp1394Devices structure to ensure
// that all is safe before I change sp1394Devices.
//-----------------------------------------------------------------------------
static void localWaitForExclusiveDevicesAccess(void) {
	sbReconfigurationActive = true;
	while(siCallsInProcess > 0) {
		hss1394::SleepCPU(2);
	}
}

// localReleaseExclusiveDevicesAccess
// Called by the (single) reconfigurer of the sp1394Devices structure after
// reconfiguration has completed.
//-----------------------------------------------------------------------------
static void localReleaseExclusiveDevicesAccess(void) {
	sbReconfigurationActive = false;
}


// localConstructDevicesList
// Scan the bus and create an array of devices that we can talk to. Entry
// to this function should eb protected.
//-----------------------------------------------------------------------------
static void localConstructDevicesList(bool bOnlyOpenVirtualDevice = false) {
	if (false == bOnlyOpenVirtualDevice) {
		memset(sp1394Devices, 0x0, sizeof(sp1394Devices));

		// Obtain streaming devices
		HDEVINFO hStreamingDeviceList = CVhpd::CreateDeviceList(&stDriverStreamingID);	// Get streaming devices list
		for(int i=0; i<MAX1394BUS; i++) {
			hss1394::CAsyncHandler *pDevice = new hss1394::CAsyncHandler(true);
			DWORD uStatus = pDevice->Open(i, hStreamingDeviceList, &stDriverStreamingID);
			if (VHPD_STATUS_SUCCESS == uStatus) {
				sp1394Devices[pDevice->NodeId()] = pDevice;
			} else {
				delete pDevice; pDevice = NULL;
				if (VHPD_STATUS_NO_SUCH_DEV_INSTANCE == uStatus) {
					break;
				}
			}
		}
		CVhpd::DestroyDeviceList(hStreamingDeviceList);

		// Obtain control-only devices
		HDEVINFO hControlOnlyDeviceList = CVhpd::CreateDeviceList(&stDriverControlOnlyID);	// Get control-only devices list
		for(int i=0; i<MAX1394BUS; i++) {
			hss1394::CAsyncHandler *pDevice = new hss1394::CAsyncHandler(false);
			DWORD uStatus = pDevice->Open(i, hControlOnlyDeviceList, &stDriverControlOnlyID);
			if (VHPD_STATUS_SUCCESS == uStatus) {
				sp1394Devices[pDevice->NodeId()] = pDevice;
			} else {
				delete pDevice; pDevice = NULL;
				if (VHPD_STATUS_NO_SUCH_DEV_INSTANCE == uStatus) {
					break;
				}
			}
		}
		CVhpd::DestroyDeviceList(hStreamingDeviceList);
	}

	// Utility access - we require an open CVhpd object. Attempt to open the 
	// virtual device, which should exist and be installed. Only attempt to
	// create if not already present.
	if (NULL == spVirtualDevice) {
		HDEVINFO hVirtualDeviceList = CVhpd::CreateDeviceList(&stVirtualDeviceDriverID);	// Get virtual devices list
		for(int i=0; i<MAX1394BUS; i++) {
			CVhpd *pDevice = new CVhpd();
			if (VHPD_STATUS_SUCCESS == pDevice->Open(i, hVirtualDeviceList, &stVirtualDeviceDriverID)) {
				spVirtualDevice = pDevice;
				break;
			} else {
				delete pDevice; pDevice = NULL;
			}
		}
		CVhpd::DestroyDeviceList(hVirtualDeviceList);

#ifndef VIRTUAL_DEVICE_IDENTIFICATION_WORKING
		// To ensure we have a device to use, if spVirtualDevice is *still* NULL, use
		// the first available device in the list instead... if this fails we
		// are hooped.
		if (NULL == spVirtualDevice) {
			for(int i=0; i<MAX1394BUS; i++) {
				if (sp1394Devices[i] != NULL) {
					spVirtualDevice = sp1394Devices[i];
					break;
				}
			}
		}
	}
#endif
	// Have now found all the devices we can open

	// Determine the 'valid' generation number for this data
	suValidGeneration = hss1394::_1394API::GetGeneration();
}


// localCleanUpDevices
// Clean up all devices, with the possible exception of spVirtualDevice. Entry
// to this function should be protected.
//-----------------------------------------------------------------------------
static void localCleanUpDevices(bool bRemoveUniqueVirtualDevice = true) {
	using namespace hss1394;

	// Clean up virtual device - if it is not one of the actual devices
	if (spVirtualDevice != NULL) {
		bool bVirtualDeviceUnique = true;
		for(int i=0; i<MAX1394BUS; i++) {
			if (spVirtualDevice == sp1394Devices[i]) {
				bVirtualDeviceUnique = false;
				break;
			}
		}
		if (true == bVirtualDeviceUnique) {
			if (true == bRemoveUniqueVirtualDevice) {
				// Reclaim storage if I am the only reference
				spVirtualDevice->Close();
				delete spVirtualDevice; 
				spVirtualDevice = NULL;
			}
		} else {
			// Kill the pointer to non-unique device that is going to be removed
			spVirtualDevice = NULL;	
		}
	}

	// Dispose of all other devices
	for(int i=0; i<MAX1394BUS; i++) {
		if (sp1394Devices[i] != NULL) {
			sp1394Devices[i]->Close();
			delete sp1394Devices[i]; sp1394Devices[i] = NULL;
		}
	}
}


// localAbortAllIO
// Force all IO Buffers currently in transit to be aborted.
//-----------------------------------------------------------------------------
static void localAbortAllIO(void) {
	// Ensure all current IO is aborted NOW
	for(int i=0; i<MAX1394BUS; i++) {
		if (sp1394Devices[i] != NULL) {
			sp1394Devices[i]->AbortIoBuffers();
		}
	}
	if (spVirtualDevice != NULL) {	// Case 459 15Jan2008
		spVirtualDevice->AbortIoBuffers();
	}
}


// localHousekeeperThread
// Entrypoint for housekeeper thread. The job of the housekeeper is to detect
// changes on the 1394 bus and respond appropriately. The appropriate response
// is rebuilding the device list and ensuring that all active devices are
// tracked.
//-----------------------------------------------------------------------------
static void localHousekeeperThread(void *pArg) {
	using namespace hss1394;
	sbHousekeeperRunning = true;
	sbHousekeeperFirstCycleComplete = false;

	// And into scan cycle
	while(true == sbHousekeeperScan) {
		// Get current generation and check the error code to see if we can trust
		// the result. If not - should attempt a restart.
		uint uNewGeneration = _1394API::GetGeneration();

		// On generation change, or previous failure to restart _1394API:: retry
		if ((suValidGeneration != uNewGeneration) || (NULL == spVirtualDevice)) {
			// Something has changed - call the handler.
			Handler *pHandler = spBusResetHandler;
			if (pHandler != NULL) {
				pHandler->BusResetStarted();
			}

			localAbortAllIO();

			// Wait for kBusResetSettleTime *after* the bus generation number has stabilized
			do {
				uNewGeneration = _1394API::GetGeneration();
				Sleep(kBusResetSettleTime);
			}while (uNewGeneration != _1394API::GetGeneration());

			// At this point the bus generation may have stabilized for better than kBusResetSettleTime
			// (ie. 500msec) - however if we immediately try to re-open the devices on the bus we
			// find that some that have just been removed still exist - therefore we wait another
			// kPostResetSettleTime (ie. 1000msec) to get the right answers from the driver layer
			// when we scan the bus and try to open the new device set.
			SleepCPU(kPostResetSettleTime);		// Required...
			
			localWaitForExclusiveDevicesAccess(); 
			{
				bool bWasOk = (spVirtualDevice != NULL);			
				localCleanUpDevices(false);		// Do *not* remove virtual device
				localConstructDevicesList();	// suValidGeneration set here
				sbActive = (spVirtualDevice != NULL);	// Flag validity of devices information.
			}
			localReleaseExclusiveDevicesAccess();

			// Something has changed - call the handler.
			pHandler = spBusResetHandler;
			if (pHandler != NULL) {
				pHandler->BusResetCompleted(suValidGeneration);
			}

			uNewGeneration = suValidGeneration;	// Sync with data prepared in localConstructDevicesList

		}
		sbHousekeeperFirstCycleComplete = true;
		Sleep(kHousekeeperInterval);
	}

	// Tell the system I am leaving...
	localCleanUpDevices(true);
	sbHousekeeperRunning = false;
	_endthread();
}

// localKillHousekeeper
// Shut down the houskeeper thread. Returns when the thread is terminated.
//-----------------------------------------------------------------------------
static void localKillHousekeeper(void) {
	sbHousekeeperScan = false;
	while(true == sbHousekeeperRunning) {
		hss1394::SleepCPU(10);
	}
}

// localStartHousekeeper
// Start the housekeeper thread.
//-----------------------------------------------------------------------------
static void localStartHousekeeper(void) {
	if (false == sbHousekeeperRunning) {
		sbActive = true;	// Enable interface, used by localHousekeeperThread
		sbHousekeeperScan = true;
		sbHousekeeperFirstCycleComplete = false;
		_beginthread(localHousekeeperThread, 0, NULL);
	}
}


// localReadBlock
// Read from the configuration ROM, returning the number of quadlets successfully
// read.
//-----------------------------------------------------------------------------
#define kReadUnit	(4)
static hss1394::uint localReadBlock(hss1394::uint uBusId, hss1394::uint uNodeId, hss1394::uint64 uAddress,
									hss1394::uint32 *pBuffer, hss1394::uint uBufferWords) {
	using namespace hss1394;
	// check buffer
	if ((NULL == pBuffer) || (0 == uBufferWords)) {
		return 0;
	}

	uint uQuadletsRead = 0;
	uint8 *pWritePoint = (uint8*)pBuffer;
	uint uBytesRemaining = uBufferWords*sizeof(uint32);
	uint uBytesInBuffer = 0;
	VHPD_ASYNC_READ tRead;
	tRead.DestinationAddress.HighPart = uAddress.mu32High;
	tRead.DestinationAddress.LowPart  = uAddress.mu32Low;
	tRead.BlockSize = 0;
	tRead.Flags = 0x0;
	while(uBytesRemaining > 0) {
		DWORD uBytesRead;
		DWORD uThisRead = (uBytesRemaining>kReadUnit)?kReadUnit:uBytesRemaining;
		if (VHPD_STATUS_SUCCESS == spVirtualDevice->AsyncReadSync(&tRead, pWritePoint, uThisRead, &uBytesRead, 1000)) {
			tRead.DestinationAddress.QuadPart += uBytesRead;
			uBytesInBuffer += uBytesRead;
			uBytesRemaining -= uBytesRead;
			pWritePoint += uBytesRead;
		} else {
			break;
		}
	}

	return (uBytesInBuffer/sizeof(uint32));
}


//*** Functions
//*****************************************************************************
namespace hss1394 {
	// Deschedule this thread for the specified time. Useful cross-platform
	// code.
	void SleepCPU(uint uMilliseconds) {
		if (uMilliseconds > 0) {
			Sleep(uMilliseconds);	// Windows call
		}
	}

	// Factory for Lock objects. 
	Lock *CreateLock(void) {
		return new RealLock();
	}

	// Millisecond wallclock timer. Implemented in 1394API_<platform>.cpp
	uint GetMilliseconds(void) {
		return GetTickCount();
	}
};


//*** Method definitions
//*****************************************************************************
namespace hss1394 {

	// CAsyncHandler::
	//-----------------------------------------------------------------------------
	// ::CAsyncHandler
	//-----------------------------------------------------------------------------
	CAsyncHandler::CAsyncHandler(bool bStreaming) :
		mbStreamingDevice(bStreaming), mbOpen(false), mbActive(false),
		muNodeAddr(0xff), muBytesRxd(0), muBytesTxd(0), mpHandler(NULL),
		muAddress(suDefaultHandlerAddress), muBytes(suDefaultHandlerBytes),
		mbHandlerActive(false), mbCallIsActive(false) {
	
	}

	// ::Open
	// Open device. Delegates to CVhpd::Open
	//-----------------------------------------------------------------------------
	DWORD CAsyncHandler::Open(int DeviceNumber, HDEVINFO DeviceList, const GUID* InterfaceGuid) {
		mbOpen = false;

		// Open device
		DWORD uResult = CVhpd::Open(DeviceNumber, DeviceList, InterfaceGuid);
		if (uResult != VHPD_STATUS_SUCCESS) {
			return uResult;
		}

		// Retrieve network node number (address)
		VHPD_GET_ADDR_FROM_DO tAddr;
		tAddr.Flags = 0x0;
		if (VHPD_STATUS_SUCCESS == GetNodeAddress(&tAddr)) {
			muNodeAddr = (uint8)tAddr.NodeAddr.NodeNmb;
		} else {
			return 0xbad0add0;	// ie. bad network address returned.
		}

		// Device is open - now enable FIFO mode
		VHPD_UINT64 u64StartAddress;
		const uint16 uBusNumber = 0x3ff;
		u64StartAddress.HighPart = muAddress.mu16High;
		u64StartAddress.LowPart = muAddress.mu32Low;

		// Setup handler
		unsigned long ulStatus = SetupFifoMode(			  // Use this code for fifo store mode
				u64StartAddress,
				muBytes,
				128,
				sizeof(VHPD_ADRRNG_NOTIFICATION)  // size in bytes of a notification buffer
		);

		// Complete initialization and set status flag
		bool bOk = (VHPD_STATUS_SUCCESS == ulStatus);
		if (true == bOk) {
			bOk = (StartThread()!=0)?true:false;
		}
		if (true == bOk) {
			EnableAddressRange();
			SleepCPU(20);	// Give handler thread time to become ready.
		}

		mbOpen = bOk;
		return (true==mbOpen)?VHPD_STATUS_SUCCESS:VHPD_STATUS_FAILED;
	}

	// ::AttachHandler
	// Attach a handler to the device. Device must be open. Returns previous
	// handler or NULL. Call with NULL to remove current handler.
	//-----------------------------------------------------------------------------
	Handler *CAsyncHandler::AttachHandler(Handler *pHandler) {
		Handler *pReturn = mpHandler;
		mpHandler = pHandler;
		return pReturn;
	}

	// ::~CAsyncHandler
	//-----------------------------------------------------------------------------
	CAsyncHandler::~CAsyncHandler() {
		if (true == mbOpen) {
			Disable();
			ShutdownThread();
			Delete();
			Close();
		}
		// DOES NOT DELETE mpHandler - this is deliberate. This class does not
		// own the storage.
	}

	// ::Enable
	//-----------------------------------------------------------------------------
	void CAsyncHandler::Enable(void) {
		mbActive = true;
	}

	// ::Disable
	//-----------------------------------------------------------------------------
	void CAsyncHandler::Disable(void) {
		mbActive = false;
		while (true == mbCallIsActive) {
			// Wait for mbCallIsActive to become false - safe shutdown interlock.
			SleepCPU(5);
		}
	}

	// ::GetInfo
	// Get monitoring data. No reset.
	//-----------------------------------------------------------------------------
	void CAsyncHandler::GetInfo(unsigned int &uBytesTxd, unsigned int &uBytesRxd) {
		uBytesRxd = muBytesRxd;
		uBytesTxd = muBytesTxd;
	}

	// ::Address
	// Address testing.
	//-----------------------------------------------------------------------------
	uint48 CAsyncHandler::Address(void) {
		return muAddress;
	}

	// ::Device
	// Device # testing.
	//-----------------------------------------------------------------------------
	uint8 CAsyncHandler::NodeId(void) {
		return muNodeAddr;
	}

	// ::ProcessNotification
	// Handle write notification. Strong assumption that this method is invoked from
	// a single thread per device and hence the signal variable mbCallIsActive will
	// behave as expected.
	//-----------------------------------------------------------------------------
	BOOL CAsyncHandler::ProcessNotification(CVhpdBuf *pBuffer) {
		if (true == mbActive) {
			BOOL bHandled = TRUE;
			// Note interlock behaviour on mbCallIsActive during 'Disable' call.
			mbCallIsActive = true; {
				// If not active, silent failure.		
				VHPD_ADRRNG_NOTIFICATION *pNotification = (VHPD_ADRRNG_NOTIFICATION*)pBuffer->Buffer();
				muBytesRxd += pNotification->Length;

				hss1394::Handler *pHandler = mpHandler;
				if (pHandler != NULL) {
					pHandler->WriteRequest((const uint8*)pNotification->Data, pNotification->Length);
				} else {
					bHandled = FALSE;
				}
			}
			mbCallIsActive = false;
			return TRUE;	// Any failure to handle is silent to caller
		
		} else {
			return TRUE;	// Silent (to caller) failure
		}
	}

	// NodeInfo::
	//*************************************************************************
	// Private constructor (friends only)
	MyNodeInfo::MyNodeInfo(uint64 uGUID, const std::string sName, 
					   const std::string sMfr, uint8 uNetworkId, bool bInstalled) :
		NodeInfo(uGUID, sName, sMfr, uNetworkId), 
		muBusId(0x3ff) {
		setInstalled(bInstalled);
	}


	// _1394API::
	// Static object for accessing 1394 asynchronous services.
	//-----------------------------------------------------------------------------

	// Start the API. Initialize all internal data.
	//-----------------------------------------------------------------------------
	bool _1394API::Start(void) {
		sbHousekeeperScan = true;	// Keeps thread running

		if (false == sbActive) {
			shDeviceListSemaphore = CreateSemaphore(NULL, 1, 1, NULL);
			
			// Do initial device list build before housekeeper is running. Kill housekeeper
			// if required.
			localKillHousekeeper();
			
			// Now I know housekeeper is not running - construct initial devices list.
			localWaitForExclusiveDevicesAccess();
			{
				localConstructDevicesList();
			}
			localReleaseExclusiveDevicesAccess();

			// Start housekeeper. 
			sbActive = true;
			localStartHousekeeper();

			// Check state of devices
			bool bSuccess;
			localDevicesAccessEntry();
			{
				if (NULL == spVirtualDevice) {
					// If there is no default device - we cannot start!
					sbActive = false;
					stErrorCode = kNoDefaultDevice;	
					bSuccess = false;

				} else {
					// Success return
					stErrorCode = kNoError;
					bSuccess = true;
				}
			}
			localDevicesAccessExit();
			return bSuccess;

		} else {
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
		WaitForSingleObject(shDeviceListSemaphore, INFINITE);	// Prevent concurrent access to device list

		sbActive = false;			// I am shutting down
		sbHousekeeperScan = false;	// Tell housekeeper to stop

		ReleaseSemaphore(shDeviceListSemaphore, 1, NULL);
		CloseHandle(shDeviceListSemaphore); shDeviceListSemaphore = 0x0;

		while(true == sbHousekeeperRunning) {
			SleepCPU(10);	// Wait for housekeeper to shut down and clean up
		}

		return true;
	}

	// Cause a bus reset to occur. This will imply that any registered bus
	// reset handler will be called.
	//-----------------------------------------------------------------------------
	bool _1394API::BusReset(void) {
		bool bSuccess;
		VHPD_BUS_RESET tResetStruct;
		tResetStruct.Flags = 0x0 | VHPD_FLAG_RESET_FORCE_ROOT;

		localDevicesAccessEntry();
		{
			if (spVirtualDevice != NULL) {
				spVirtualDevice->ForceBusReset(&tResetStruct);
				bSuccess = true;
			} else {
				stErrorCode = kNoDefaultDevice;		
				bSuccess = false;
			}
		}
		localDevicesAccessExit();

		SleepCPU(2000);	// Settling time

		return bSuccess;
	}

	// Return the current bus generation number. Note that this increments on every
	// bus reset event, wherever that is initiated on the bus.
	//-----------------------------------------------------------------------------
	uint _1394API::GetGeneration(void) {
		VHPD_GENERATION_COUNT tCount;
		bool bSuccess;

		localDevicesAccessEntry();
		{
			if (spVirtualDevice != NULL) {
				bSuccess = (VHPD_STATUS_SUCCESS == spVirtualDevice->GetResetGenerationCount(&tCount));
			} else {
				stErrorCode = kNoDefaultDevice;		
				bSuccess = false;
			}
		}
		localDevicesAccessExit();

		if (true == bSuccess) {
			return tCount.Count;
		} else {
			return 0xffffffff;
		}
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


	// !fn
	// Helper function for _1394API::GetNetworkInfo below - inner scanning function.
	// Devices structures must be protected (localDevicesAccess[Entry/Exit] during
	// this call.
	//-----------------------------------------------------------------------------
	static void localPushNodeInfo(int iTestNode, int iLocalNodeId, 
								  const VHPD_LOCAL_HOST_INFO_DATA &tHostInfoData, std::list<const NodeInfo *> &lList) {
		if (sp1394Devices[iTestNode] != NULL) {
			// There is an installed device (VHPD1394) - query it to obtain data
			CVhpd *pThisDevice = sp1394Devices[iTestNode];
			// Get Node address
			uint8 uNetworkId = (uint8)iTestNode;

			// Is this device real or virtual? Only handle real devices.
			uint8 pGUID[8];
			if (VHPD_STATUS_SUCCESS == pThisDevice->GetDeviceUniqueId(pGUID)) {
				// Get name and manufacturer strings
				char pBuffer[128];
				pThisDevice->GetDeviceModelString(pBuffer, sizeof(pBuffer));
				std::string sName(pBuffer);
				pThisDevice->GetDeviceVendorString(pBuffer, sizeof(pBuffer));
				std::string sMfr(pBuffer);

				uint64 uGUID;
				uGUID.mu32High = (pGUID[0]<<24)|(pGUID[1]<<16)|(pGUID[2]<<8)|pGUID[3];
				uGUID.mu32Low = (pGUID[4]<<24)|(pGUID[5]<<16)|(pGUID[6]<<8)|pGUID[7];

				NodeInfo *pNodeInfo = new MyNodeInfo(uGUID, sName, sMfr, uNetworkId, true);
				lList.push_back(pNodeInfo);
			}

		} else {
			// There is no installed device with this id - is there anything there?
			if (iTestNode < (USHORT)tHostInfoData.HCInfTopologyMap.TopologyMap.SelfIDCount) {
				// There is something here - is it connected
				SPEC1394_SELF_ID_PACKET tIdPacket = tHostInfoData.HCInfTopologyMap.TopologyMap.SelfIDArray[iTestNode];
				if (tIdPacket.Link_Active != 0) {
					// Yes it is - pull the data in
					uint uBusId = 0x3FF;
					uint uNodeId = tIdPacket.Phys_ID;
					if (uNodeId != iLocalNodeId) {
						// Not the local node - so keep going!
						// Read the config rom from the device and pass to interpreter
						uint64 uGUID(0x0, 0x0);
						std::string sName("Unknown");
						std::string sMfr("Unknown");
						uint32 puConfigRom[0x200];		// 2KB = .5K 32-bit words
						uint64 uAddress;
						uAddress.mu32High = ( ((((uBusId&0x3FF)<<6)|(uNodeId&0x3F)) << 16) | 0xffff );
						uAddress.mu32Low = 0xf0000400;
						uint32 uWordsRead = localReadBlock(uBusId, uNodeId, uAddress, puConfigRom, 0x80);	// Was 0x200
						// If read successful - interpret it - else mark as 'unknown' node
						if (0x0 == uWordsRead) {
							// Attempt check for blank Oxford device to at least provide some ID
							uint32 u32Buffer;
							uint64 uOxfordAddress;
							uOxfordAddress.mu32High = ( ((((uBusId&0x3FF)<<6)|(uNodeId&0x3F)) << 16) | 0xffff );
							uOxfordAddress.mu32Low = 0xf0090020;
							if (localReadBlock(uBusId, uNodeId, uOxfordAddress, &u32Buffer, sizeof(u32Buffer)) != 0) {
								uint8 pBuffer[5];
								memcpy(pBuffer, (const uint8*)&u32Buffer, sizeof(u32Buffer));
								pBuffer[4] = 0x0;
								sName = (const char *)pBuffer;
								sMfr = "Oxford";

							} else {
								// This is an unknown unknown - all data left at defaults.
							}

						} else {
							// Interpret contents of configuration ROM
							ParseConfigRom(puConfigRom, uWordsRead, sName, sMfr, uGUID);
						}

						// Found out all I can about the node - populate the data structure
						NodeInfo *pNodeInfo = new MyNodeInfo(uGUID, sName, sMfr, uNodeId, false);
						lList.push_back(pNodeInfo);
					}
				}
			}
		}
	}

	// ::GetNetworkInfo
	// Return a description of the network as a list of NodeInfo objects. No 
	// information should be inferred from the ordering of the return list. After
	// using the list, the caller should release the list by calling ::ReleaseNetworkInfo.
	//-----------------------------------------------------------------------------
	std::list<const NodeInfo *> _1394API::GetNetworkInfo(void) {
		DWORD Status;
		std::list<const NodeInfo *> lList;

		if (false == sbActive) {
			stErrorCode = kNotStarted;
			return lList;
		}

		localDevicesAccessEntry();
		{
			if (spVirtualDevice != NULL) {
				// Get local node data
				VHPD_GET_ADDR_FROM_DO tLocalNodeAdr;
				ZeroMemory(&tLocalNodeAdr, sizeof(VHPD_GET_ADDR_FROM_DO));
				tLocalNodeAdr.Flags = VHPD_FLAG_USE_LOCAL_NODE;
				Status = spVirtualDevice->GetNodeAddress(&tLocalNodeAdr);

				// Get network topology data
				VHPD_GET_LOCAL_HOST_INFO tHostInfo;
				memset(&tHostInfo, 0x0, sizeof(VHPD_GET_LOCAL_HOST_INFO));
				VHPD_LOCAL_HOST_INFO_DATA tHostInfoData;
				memset(&tHostInfoData, 0x0, sizeof(VHPD_LOCAL_HOST_INFO_DATA));
				tHostInfo.Type = TopologyMap;
				Status = spVirtualDevice->GetLocalHostInfo(&tHostInfo, &tHostInfoData); 
				unsigned short uSIDCount = (USHORT)tHostInfoData.HCInfTopologyMap.TopologyMap.SelfIDCount;

				// Scan through all devices and bus locations looking for active nodes
				for(int i=0; i<MAX1394BUS; i++) {
					localPushNodeInfo(i, tLocalNodeAdr.NodeAddr.NodeNmb, tHostInfoData, lList);
				}

			} else /* spVirtualDevice == NULL */ {
				stErrorCode = kVirtualDeviceNotOpen;
			}

		}
		localDevicesAccessExit();
		return lList;
	}

	// ::ReleaseNetworkInfo
	// Return storage responsibility for the list returned by GetNetworkInfo to
	// _1394API implementation. This is a required call after using the list
	// provided by ::GetNetworkInfo.
	//-----------------------------------------------------------------------------
	void _1394API::ReleaseNetworkInfo(std::list<const NodeInfo*> &lDevicesList) {
		std::list<const NodeInfo *>::iterator tIter;
		for(tIter = lDevicesList.begin(); tIter != lDevicesList.end(); tIter++) {
			delete (*tIter); (*tIter) = NULL;
		}
		lDevicesList.clear();
	}

	// ::GetSelfId
	// Return my network Id.
	//-----------------------------------------------------------------------------
	uint8 _1394API::GetSelfId(void) {
		if (false == sbActive) {
			stErrorCode = kNotStarted;
			return 0xFF;
		}

		uint8 uReturn;
		localDevicesAccessEntry();
		{
			if (spVirtualDevice != NULL) {
				VHPD_GET_ADDR_FROM_DO tNodeInfo;
				tNodeInfo.Flags = VHPD_FLAG_USE_LOCAL_NODE;
				spVirtualDevice->GetNodeAddress(&tNodeInfo);
				uReturn = (uint8)tNodeInfo.NodeAddr.NodeNmb;

			}  else {
				stErrorCode = kNoDefaultDevice;		
				uReturn = 0;
			}
		}
		localDevicesAccessExit();

		return uReturn;
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
		if (uNodeId > MAX1394BUS) {
			stErrorCode = kBadNodeId;
			return false;
		}

		// Prepare all the data required.
		unsigned long uBytesTransferred; uBytesTransferred = 0;
		VHPD_ASYNC_READ tAsyncRead;
		memset(&tAsyncRead,0,sizeof(tAsyncRead));
		const uint16 uBusId = 0x3FF;
		const uint16 uNetworkAddress = (((uBusId&0x3FF)<<6) | uNodeId&0x3F);
		tAsyncRead.DestinationAddress.HighPart = (uNetworkAddress<<16) | uAddress.mu16High;
		tAsyncRead.DestinationAddress.LowPart = uAddress.mu32Low;
		tAsyncRead.BlockSize = 0;
		tAsyncRead.Flags = 0x0;
		memset(pData, 0x0, uSize);

		bool bReturn;
		localDevicesAccessEntry();
		{
			CVhpd *pDevice = spVirtualDevice;
			if (pDevice != NULL) {
				// Perform read using device for this node
				DWORD uErrorCode = pDevice->AsyncReadSync(&tAsyncRead, pData, uSize, &uBytesTransferred, RESPONSETIMEOUT);
				if (VHPD_STATUS_SUCCESS == uErrorCode) {
					bReturn = true;
				} else {
					stErrorCode = kAsyncTimeout;
					bReturn = false;
				}

			} else {
				// Attempt read using default device
				stErrorCode = kNonExistentNode;
				bReturn = false;
			}
		}
		localDevicesAccessExit();

		return bReturn;
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
		if (uNodeId > MAX1394BUS) {
			stErrorCode = kBadNodeId;
			return false;
		}

		bool bReturn;
		localDevicesAccessEntry();
		{
			CVhpd *pDevice = spVirtualDevice;
			if (pDevice != NULL) {
				VHPD_ASYNC_WRITE tWrite;
				const uint16 uBusId = 0x3FF;
				const uint16 uNetworkAddress = (((uBusId&0x3FF)<<6) | uNodeId&0x3F);
				tWrite.DestinationAddress.HighPart = (uNetworkAddress<<16) | uAddress.mu16High;
				tWrite.DestinationAddress.LowPart = uAddress.mu32Low;
				tWrite.BlockSize = 0;
				tWrite.Flags = 0x0;
				uint8 pBuffer[256];	// Signature of AsyncWriteSync has problems... (void *, not const void *)
				if (uSize > sizeof(pBuffer)) {
					stErrorCode = kIntermediateBufferTooSmall;
					bReturn = false;
				} else {
					memcpy(pBuffer, pData, uSize);
					if (VHPD_STATUS_SUCCESS == 
						pDevice->AsyncWriteSync(&tWrite, pBuffer, uSize, NULL, RESPONSETIMEOUT)) {
						bReturn = true;
					} else {
						stErrorCode = kAsyncTimeout;
						SleepCPU(1);	// Backoff
						bReturn = false;
					}
				}
			} else {
				stErrorCode = kNonExistentNode;
				bReturn = false;
			}
		}
		localDevicesAccessExit();

		return bReturn;
	}

	// Add an address handler object for both read and write. The call returns the 48-bit
	// 1394 base address of the handler. Note this is the only cross-platform (mac/PC) means
	// of doing this, as OSX does not allow pseudo address spaces created on 1394 to be
	// targetted. Only one handler is permitted to be created via this interface.
	// (Since I wrote this I found the method on the Mac - CreatePseudoAddressSpaceInitialUnits).
	//-----------------------------------------------------------------------------
	uint48 _1394API::AddHandler(uint8 uNodeId, Handler *pUserHandler, bool &bSuccess) {
		uint48 uReturn;
		uReturn.mu16Guard = 0;
		uReturn.mu16High = 0;
		uReturn.mu32Low = 0;
		bSuccess = false;

		if (false == sbActive) {
			stErrorCode = kNotStarted;
			return uReturn;
		}
		if (NULL == pUserHandler) {
			stErrorCode = kNoHandlerSpecified;
			return uReturn;
		}
		if (uNodeId > MAX1394BUS) {
			stErrorCode = kBadNodeId;
			return uReturn;
		}

		localDevicesAccessEntry();
		{
			if (NULL == sp1394Devices[uNodeId]) {
				stErrorCode = kNoDevice;
			} else if (sp1394Devices[uNodeId]->mpHandler != NULL) {
				stErrorCode = kHandlerAlreadyInstalled;
			} else {
				bSuccess = true;
				sp1394Devices[uNodeId]->AttachHandler(pUserHandler);
				sp1394Devices[uNodeId]->Enable();
				uReturn = sp1394Devices[uNodeId]->Address();
			}
		}
		localDevicesAccessExit();

		return uReturn;
	}

	// Remove the handler (if any) on a specified node and return it to the
	// caller. If there is no such handler, the return value will be NULL.
	//-----------------------------------------------------------------------------
	Handler *_1394API::RemoveHandler(uint8 uNodeId) {
		uint48 uAddress = suDefaultHandlerAddress;
		if (false == sbActive) {
			stErrorCode = kNotStarted;
			return NULL;
		}
		if (uNodeId > MAX1394BUS) {
			stErrorCode = kBadNodeId;
			return NULL;
		}

		Handler *pReturn = NULL;
		localDevicesAccessEntry();
		{
			if (NULL == sp1394Devices[uNodeId]) {
				stErrorCode = kNoDevice;
			} else {
				sp1394Devices[uNodeId]->Disable();
				pReturn =  sp1394Devices[uNodeId]->AttachHandler(NULL);
			}
		}
		localDevicesAccessExit();

		return pReturn;
	}

};	// namespace hss1394



