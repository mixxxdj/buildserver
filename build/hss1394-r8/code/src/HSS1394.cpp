///////////////////////////////////////////////////////////////////////////////
//! \file HSS1394.cpp
//!
//! C++ API file for 1394 High-Speed Serial interface. This interface is designed
//! to provide a low-latency data transport between endpoints in an IEEE1394
//! network that are configured with a HSS1394 endpoint. This interface provides
//! management and data passage functionality for the host end of this interface.
//!
//! A HSS1394 network is superimposed upon a 1394 network. A HSS1394 network
//! consists of a set of nodes with a simple common interface allowing them
//! to exchange information with a host node (star network topology) via
//! full-duplex data channels.
//
//! This interface is not thread-safe, and therefore the user should ensure that
//! multiple concurrent invocations through this interface are protected using
//! some external synchronization mechanism.
//! 
//! Created by Don Goodeve (don@bearanascence.com), 28Apr2008, 11:30PST
//! this file Copyright (C) 2009, Stanton Group.
//! $Id: HSS1394.cpp 1561 2009-04-16 06:34:40Z don $
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
#include <string>		// STL string class
#include <vector>		// STL vector template
#include <assert.h>     // C ASSERT macros
#include "HSS1394.h"	// HSS1394 includes
#include "1394API.h"	// Cross-platform 1394 interface layer


//*** Macros
//*****************************************************************************
//! MS16 of HSS1394 node address
#define kDefaultHSS1394AddressHi	(0xc007)
//! LS32 of HSS1394 node address
#define kDefaultHSS1394AddressLo	(0xdedadada)
//! Maximum allowed HSS1394 packet size
#define kHSS1394Bytes				(63)
//! Invalid network ID value
#define kInvalidNetworkId			(0xff)
//! Maximum allowed retries before failing some operations
#define kMaxRetries					(32)

#ifndef assert
	#define assert(x)	_ASSERT(x)
#endif


//*** Local variables
//*****************************************************************************
//! Initialization of static member variable of Node class (declared in 
//! HSS1394.h).
//-----------------------------------------------------------------------------
hss1394::Node *hss1394::Node::mspInstance = NULL;

//! suHSS1394Address
//! Address at which to operate the HSS1394 protocol. Initialized in ::RealNode.
//-----------------------------------------------------------------------------
static hss1394::uint48 suHSS1394Address;


//*** Local classes
//*** Declarations and definitions.
//*****************************************************************************
namespace hss1394 {
	class RealChannel;	// Defined below

	// HSS1394NodeInfo
	//! Contains information about a discovered HSS1394 node on the 1394 net
	//! this is an element of the database of discovered HSS1394 nodes.
	//-------------------------------------------------------------------------
	class HSS1394NodeInfo : public NodeInfo {
		private:
			bool mbLive;					//!< Flag can be set dead to say record should not be used.
			bool mbWasLive;					//!<  Track previous state
			uint16 muHSS1394Version;		//!<  HSS1394 protocol version (of node)
			RealChannel *mpChannel;			//!<  If non-null is the channel to this node
			hss1394::Handler *mpHandler;	//!<  Associated address space handler
			uint8 muNetworkId;				//!<  Actual network id
			uint8 muOldNetworkId;			//!<  If a network id change occurs - this tracks the old value

			//! Definition requires class RealChannel defined below
			void SetChannelConnectionState(bool bActive);

		public:
			//! Definitions require class HSS1394Handler defined below
			HSS1394NodeInfo(const NodeInfo *pNodeInfo);
			~HSS1394NodeInfo();

			//! Return version code of on-node implementation of HSS1394 protocol
			uint16 getHSS1394Version(void) {
				return muHSS1394Version;
			}

			//! Node is 'active' if it is present, and a handler has been successfully set up.
			bool Active(void) {
				return (mbLive && (mpHandler != NULL));
			}

			//! Override of base class function - using local stored value.
			uint8 getNetworkId(void) {
				return muNetworkId;
			}
		
			//! Pull out all node details into a TNodeInfo structure.
			void extractNodeInfo(TNodeInfo &tNodeInfo) {
				tNodeInfo.sName = getName();
				tNodeInfo.uGUID = getGUID();
				tNodeInfo.uProtocolVersion = getHSS1394Version();
			}

		friend class RealNode;
		friend class HSS1394Handler;
	};


	// Message::
	//! A message is a variable-length buffer containing a single message.
	//----------------------------------------------------------------------------
	class Message {
		private:
			uint8 muTag;
			uint8 *mpMessage;
			uint muSize;
		
		public:
			//! Given raw packet data, construct a 'message'
			Message(const uint8 *pData, uint uSize) : 
				mpMessage(NULL), muTag(kUndefined), muSize(0) {
				if ((pData != NULL) && (uSize>0)) {
					muTag = pData[0];
					muSize = (uSize-1);
					if (muSize > 0) {
						mpMessage = new uint8[muSize]; //!!!!
						memcpy(mpMessage, &pData[1], muSize);
					}
				}
			}

			//! Delete internal storage
			~Message() {
				if (mpMessage != NULL) {
					delete []mpMessage;  mpMessage = NULL;
				}
			}

			//! Copy-construct a message from the given offset (defaults to zero)
			Message *tail(uint uOffset) {
				if (uOffset < muSize) {
					return new Message(&(mpMessage[uOffset]), (muSize-uOffset));
				} else {
					return NULL;
				}
			}
		
			//! Truncate a message at the specified offset
			void truncateAt(uint uOffset) {
				if (uOffset < muSize) {
					muSize = uOffset;
				}
			}
		
			//! Append the given message to this message
			void append(Message *pMessage) {
				uint uNewSize = muSize + pMessage->muSize + 1;	// tag byte becomes part of message
				uint8 *pNewMessage = new uint8[uNewSize];
				memcpy(pNewMessage, mpMessage, muSize);
				pNewMessage[muSize] = pMessage->muTag;
				memcpy(&(pNewMessage[muSize+1]), pMessage->mpMessage, pMessage->muSize);
				delete []mpMessage; mpMessage = NULL;
				mpMessage = pNewMessage;
				muSize = uNewSize;
			}
		
			friend class HSS1394Handler;
			friend class RealChannel;
	};

	// RealChannel::
	// Abstraction of a channel endpoint at the host communicating with a
	// remote HSS1394 node. See method documentation in header file HSS1394.h. This
	// subclass is the one that does the work.
	//----------------------------------------------------------------------------
	class RealChannel : public Channel {
		private:
			volatile bool mbRunning;			//!< True if channel is active
			uint muRetries;						//!< Current retry count
			std::list<Message *> mlRxList;		//!< Received message list
			HSS1394NodeInfo *mpNodeInfo;		//!< Associated node information
			ChannelListener *mpChannelCallback;	//!< Channel callback function
			volatile bool mbStallChannelListenerRemoval;	//!< Interlock for removal of callback
			Lock *mpRxListLock;					//!< Lock protecting access to mlRxList

			//! Constructor - called from RealNode
			RealChannel(HSS1394NodeInfo *pNodeInfo);

			//! Destructor - called from HSS1394Handler
			~RealChannel();

			//! Called from RealNode to add a message to this channel's queue
			void InsertMessage(Message *pMessage);

			//! Internal call when node appears/disappears
			void SetConnectionState(bool bActive);

			//! Core send functionality.
			uint SendBytesInternal(uint8 uTagByte, const uint8 *pBuffer, uint uBufferSize, bool bForce);

		public:
			virtual void Stop(void);
			virtual void Restart(void);
			virtual void Flush(void);
			virtual void GetNodeInfo(TNodeInfo &tNodeInfo);
			virtual uint SendChannelBytes(const uint8 *pBuffer, uint uBufferSize, bool bForce);
			virtual uint SendChannelEcho(const uint8 *pBuffer, uint uBufferSize);
			virtual uint GetRetries(void);
			virtual uint ReceiveChannelBytes(uint8 *pBuffer, uint uBufferSize);
			virtual bool ChannelListenerExists(void);
			virtual bool InstallChannelListener(ChannelListener *pListener);
			virtual uint SendUserControl(uint8 uUserTag, const uint8 *pUserData, uint uDataBytes);

		friend class RealNode;			// Construction, destruction
		friend class HSS1394Handler;	// Message passing
		friend class HSS1394NodeInfo;
	};

	// HSS1394Handler::
	//! Asynchronous handler class for channel operations
	//-----------------------------------------------------------------------------
	class HSS1394Handler : public hss1394::Handler {
		private:
			HSS1394NodeInfo *mpOwner;
			Message *mpPrefix;
		
			//! Perform processing on incomming write request. Only accept the packet if the tag
			//! byte appears valid - otherwise reject.
			bool processWriteRequest(Message *pMessage) {
				assert(pMessage != NULL);	// Trap allocation failure
				if (0 == pMessage->muSize) {
					return true;	// Special case - tag only message
				}
		
				switch(pMessage->muTag) {
					case kUserData: {
						// Expect message to start with MIDI status byte - msb set
						if ((pMessage->mpMessage[0]&0x80) != 0x80) {
							return false;	// Reject message
						}
						RealChannel *pChannel = mpOwner->mpChannel;
						if (pChannel != NULL) {
							// User data passed to associated channel
							pChannel->InsertMessage(pMessage);

						} // Silent discard if no channel
						return true;	// Do not reclaim pMessage here
					}
						
					case kDebugData: {
						for(uint i=0; i<pMessage->muSize; i++) {
							fputc(pMessage->mpMessage[i], stdout);
						}
						return true;
					}
						
					default: {
						return false;
					}
				}
				return false;	// Stop compiler complaining...
			}
		
			//! Search through pMessage to see if there is an embedded start of a new message
			//! (tag byte). If so, then return the new message and modify pMessage to remove
			//! these bytes from the end. Otherwise return NULL and pMessage remains unchanged.
			Message *findNewMessageStart(Message *pMessage) {
				assert(pMessage != NULL);
				for(uint i=0; i<pMessage->muSize; i++) {
					if ((kUserData == pMessage->mpMessage[i]) || (kDebugData == pMessage->mpMessage[i])) {
						// Ok - looks like a good candidate; do split
						Message *pSuffix = pMessage->tail(i);
						pMessage->truncateAt(i);
						return pSuffix;
					}
				}
				return NULL;	// Fall through failure case
			}
		
		public:
			//! Simple constructor
			HSS1394Handler(HSS1394NodeInfo *pOwner) : mpOwner(pOwner), mpPrefix(NULL) {
			}

			//! Handle an asynchronous write request to this handler.
			virtual bool WriteRequest(const uint8 *pBytes, uint uSize) {
				if (0 == uSize) {	
					return false;	// Spurious call
				}
				Message *pMessage = new Message(pBytes, uSize);
				bool bReturn = false;
				if (true == processWriteRequest(pMessage)) {
					// Packet is good! Clear stored state and move on.
					bReturn = true;
					if (mpPrefix != NULL) {
						delete mpPrefix; mpPrefix = NULL;				
					}
					
				} else {
					// Tag is bad - prepend last iterations' prefix (if any)
					if (mpPrefix != NULL) {
						mpPrefix->append(pMessage);
						delete pMessage; pMessage = NULL;
						pMessage = mpPrefix; mpPrefix = NULL;
					}
					mpPrefix = findNewMessageStart(pMessage);	// Modifies pMessage
					bReturn = processWriteRequest(pMessage);
				}
				if (false == bReturn) {
					// processWriteRequest now owns the message if it has been accepted
					delete pMessage; pMessage = NULL;
				}
				return bReturn;
			}
						
	};


	// HSS1394NodeInfo::
	//*************************************************************************
	//! Method defined here as it requires the declaration of RealChannel above
	void HSS1394NodeInfo::SetChannelConnectionState(bool bActive) {
		if (mpChannel != NULL) {
			mpChannel->SetConnectionState(bActive);
		}
	}	
				
	//! Methods defined here as they requires the declaration of HSS1394Handler above
	HSS1394NodeInfo::HSS1394NodeInfo(const NodeInfo *pNodeInfo):
		muHSS1394Version(0x0000), mbLive(true), mbWasLive(false), NodeInfo(pNodeInfo), 
		mpChannel(NULL), muNetworkId(pNodeInfo->getNetworkId()), muOldNetworkId(kInvalidNetworkId) {

		// Create new handler object, but do not add via _1394API. Done in caller.
		mpHandler = new HSS1394Handler(this);
	}

	//! Destructor handles removal of handler
	HSS1394NodeInfo::~HSS1394NodeInfo() {
		if (mpHandler != NULL) {
			_1394API::RemoveHandler(this->getNetworkId()); 
			delete mpHandler; mpHandler = NULL;
		}
	}


	// RealNode::
	// Singleton High-speed serial over 1394 host node interface. See method 
	// documentation in header file HSS1394.h. This is the class that does
	// the work.
	//----------------------------------------------------------------------------
	class RealNode : public Node {
		private:
			bool mbOk;								//!< Node 'is good' flag
			Listener *mpConnectionListener;			//!< Called when bus topology has changed
			hss1394::Handler *mpWriteHandler;		//!< Called with incomming data
			std::vector<HSS1394NodeInfo*> mlNodeDB;	//!< Internal database of network information

		protected:
			RealNode(void);				//!< Constructor - dealt with via '::Instance' call
			void ScanBus(void);			//!< Internal - perform bus scan, updating mlNodeDB
			void StopAllChannels(void);	//!< Internal - force all channels to stop

			//! Synchronously ping a node, returns HSS1394 version
			bool Ping(uint8 uNodeId, uint16 *puVersion, bool bReadPing);

			//! Handle installation of HSS1394 Handler against a specific network Id
			bool Configure1394Handler(uint uNodeId, Handler *pHandler);

		public:
			static Node *Instance(void);
			virtual ~RealNode();
			virtual bool IsOk(void);		// Returns true if node initialization is ok.
			virtual void InstallConnectionListener(Listener *pListener);
			virtual uint GetNodeCount(void);
			virtual bool GetNodeInfo(TNodeInfo &tNodeInfo, uint uNode, bool *pbAvailable = NULL, bool *pbInstalled = NULL);
			virtual Channel *OpenChannel(uint uNode);
			virtual bool ReleaseChannel(Channel *pChannel);

		// Intimates...
		friend class RealChannel;
		friend class Node;
		friend class MyBusResetHandler;
	};


	// PingHandler::
	//! Asynchronous handler class for ping operations.
	//-----------------------------------------------------------------------------
	class PingHandler : public hss1394::Handler {
		private:
			bool mbGotResponse;
		
		public:
			uint16 muVersion;

			PingHandler(void) {
				Reset();
			}

			void Reset(void) {
				mbGotResponse = false;
				muVersion = 0x0000;
			}

			//! Handle an asynchronous write request to this handler.
			virtual bool WriteRequest(const uint8 *pBytes, uint uSize) {
				if (4 == uSize) {
					if (kPingResponse == pBytes[0]) {
						mbGotResponse = true;
						muVersion = (pBytes[2]<<8) | pBytes[3];
						return true;
					}
				}
				return false;
			}

			bool GotResponse(void) {
				return mbGotResponse;
			}
	};


	//! MyBusResetHandler -> scBusResetHandler
	//! 1394 bus reset handling functionality, attached to BusReset mechanism in
	//! _1394API.
	//-----------------------------------------------------------------------------
	static class MyBusResetHandler : public Handler {
		friend class RealNode;

		private:
			RealNode *mpRealNodeInstance;	//!< Associated node instance
		
		public:
			//! Handler invoked when bus reset has begun
			virtual void BusResetStarted(void) {
				RealNode *pRealNode = mpRealNodeInstance;
				if (pRealNode != NULL) {
					pRealNode->StopAllChannels();
				}
			}

			//! Handler invoked when bus reset has completed
			virtual bool BusResetCompleted(uint uGeneration) {
				RealNode *pRealNode = mpRealNodeInstance;
				if (pRealNode != NULL) {
					pRealNode->ScanBus();
				}
				return true;
			}
	}scBusResetHandler;


	// RealChannel::
	//*************************************************************************
	//! Constructor of a channel to/from a node
	RealChannel::RealChannel(HSS1394NodeInfo *pNodeInfo): 
		mpNodeInfo(pNodeInfo), mbRunning(true), mpChannelCallback(NULL),
		mbStallChannelListenerRemoval(false), muRetries(0) {
		mpRxListLock = hss1394::CreateLock();
	}

	//! Destructor - called from RealNode
	RealChannel::~RealChannel() {
		mbRunning = false;

		// Reclaims all internal storage.
		Flush();
		
		// Careful delete sequence (race)
		hss1394::Lock *pRxListLock = mpRxListLock;
		mpRxListLock = NULL;
		delete pRxListLock;

		// Note - mpChannelListener is not my property
	}

	//! Inform channel callback of connection state of channel
	void RealChannel::SetConnectionState(bool bActive) {
		if (mpChannelCallback != NULL) {
			if (false == bActive) {
				mpChannelCallback->Disconnected();
			} else {
				mpChannelCallback->Reconnected();
			}
		}
	}

	//! Force channel into stopped state
	void RealChannel::Stop(void) {
		mbRunning = false;
	}

	//! Force channel into running state
	void RealChannel::Restart(void) {
		mbRunning = true;
	}

	//! Return associated node information
	void RealChannel::GetNodeInfo(TNodeInfo &tNodeInfo) {
		mpNodeInfo->extractNodeInfo(tNodeInfo);
	}

	// ::SendChannelBytes
	//! Returns the number of bytes actually sent from pBuffer. There is a known issue with the Oxford
	//! firmware that it does not reliably handle non-quadlet traffic.
	uint RealChannel::SendChannelBytes(const uint8 *pBuffer, uint uBufferSize, bool bForce) {
		return SendBytesInternal(kUserData, pBuffer, uBufferSize, bForce);
	}

	// ::SendChannelEcho
	//! Send a packet to be echoed back (testing purposes). Packet will not be passed to 
	//! application layer on receiving node.
	uint RealChannel::SendChannelEcho(const uint8 *pBuffer, uint uBufferSize) {
		return SendBytesInternal(kEchoAsUserData, pBuffer, uBufferSize, false);
	}

	// ::SendUserControl
	//! Send a user control packet. This is provides a means of protocol embedding for special
	//! purposes on the target (firmware support required).
	uint RealChannel::SendUserControl(uint8 uUserTag, const uint8 *pUserData, uint uDataBytes) {
		uint8 uActualTag = uUserTag + kUserTagBase;
		if (uActualTag > kUserTagTop) {
			return 0;
		}
		return SendBytesInternal(uActualTag, pUserData, uDataBytes, false);
	}

	// ::SendBytesInternal
	//! Core functionality for sending bytes (private).
	uint RealChannel::SendBytesInternal(uint8 uTagByte, const uint8 *pBuffer, uint uBufferSize, bool bForce) {
		assert(mpNodeInfo != NULL);
		if ((true == mbRunning) && (true == mpNodeInfo->Active())) {
			// The node is active - can go ahead
			// Sends constrained to single quadlet size to avoid block size rx
			// problems noted in Oxford firmware.
			uint8 pStagingBuffer[kHSS1394Bytes+1];
			uint uRemainder;
			if (false == bForce) {
				// Send what can fit in one (quadlet) packet
				uRemainder = (uBufferSize<(sizeof(pStagingBuffer)-1))?uBufferSize:(sizeof(pStagingBuffer)-1);
			} else {
				// Iterate to send the lot
				uRemainder = uBufferSize;
			}
			uint uSent = 0;
			while(uRemainder>0) {
				uint uThisSend = (uRemainder>(sizeof(pStagingBuffer)-1))?(sizeof(pStagingBuffer)-1):uRemainder;
				// Invariant: (uThisSend < sizeof(pStagingBuffer))
				pStagingBuffer[0] = uTagByte;
				memcpy(&pStagingBuffer[1], pBuffer, uThisSend);
				if (true == _1394API::WriteBlockAsync(mpNodeInfo->getNetworkId(), suHSS1394Address, pStagingBuffer, (uThisSend+1))) {
					uRemainder -= uThisSend;
					pBuffer += uThisSend;
					uSent += uThisSend;
				} else {
					muRetries++;
					if (muRetries >= kMaxRetries) {
						return uSent;
					}
					SleepCPU(1);	// Back-off
				}
			}
			return uSent;

		} else {
			return 0;
		}
	}

	// ::Flush
	//! Flush all the pending bytes in the receive queue on this channel.
	void RealChannel::Flush(void) {
		// Delete the Rx list
		mpRxListLock->Enter();
			for(std::list<Message *>::iterator tIter = mlRxList.begin(); tIter != mlRxList.end(); tIter++) {
				if ((*tIter) != NULL) {
					delete (*tIter); (*tIter) = NULL;
				}
			}	
			mlRxList.clear();
		mpRxListLock->Exit();
	}

	// ::GetRetries
	//! Return the number of retries current on this channel.
	uint RealChannel::GetRetries(void) {
		uint uReturn = muRetries;
		muRetries = 0;
		return uReturn;
	}

	// ::InsertMessage
	//! Insert a message into the receive-queue.
	void RealChannel::InsertMessage(Message *pMessage) {
		if (NULL == pMessage) {
			return;
		}

		if (true == mbRunning) {
			// All incomming messages go into channel buffer.
			mpRxListLock->Enter();
				mlRxList.push_back(pMessage);
			mpRxListLock->Exit();

			// Flush to channel callback (if installed).
			// There is a race here. If mpChannelCallback is deleted whilst this loop
			// is operating we could be in trouble. If the interface is being used
			// correctly, ::InstallChannelListener(NULL) is called before mpChannelCallback
			// can be deleted. So, we need to ensure that this code completes before
			// ::InstallChannelListener(NULL) is allowed to complete. This is done through
			// the variable mbStallChannelListenerRemoval.
			mbStallChannelListenerRemoval = true;
			ChannelListener *pCallback = mpChannelCallback;
			if (pCallback != NULL) {
				mpRxListLock->Enter();
					while(false == mlRxList.empty()) {
						Message *pMessage = mlRxList.front();
						mlRxList.pop_front();
						pCallback->Process(pMessage->mpMessage, pMessage->muSize);
						delete pMessage; pMessage = NULL;
					}
				mpRxListLock->Exit();
			}
			mbStallChannelListenerRemoval = false;
			
		} else {
			// No processing - but message not deleted here...
			delete pMessage; pMessage = NULL;
		}
	}

	// ::ReceiveChannelBytes
	//! Pulls the first message from mlRxList. uBufferSize should be large enough to absorb
	//! the largest possible HSS1394 message (kHSS1394Bytes-1).
	uint RealChannel::ReceiveChannelBytes(uint8 *pBuffer, uint uBufferSize) {
		if (mpChannelCallback != NULL) {
			return 0;	// If callback is present, this call is a no-op.
		}

		if (false == mbRunning) {
			return 0;	// Channel not running - no can do
		}

		Message *pMessage = NULL;
		mpRxListLock->Enter();
			if (false == mlRxList.empty()) {
				pMessage = mlRxList.front();
				mlRxList.pop_front();
			}
		mpRxListLock->Exit();

		// Ignore the tag field in the message. Already interpreted below in the stack (Handler)
		if (pMessage != NULL) {
			uint uPullSize = (pMessage->muSize<uBufferSize)?pMessage->muSize:uBufferSize;
			memcpy(pBuffer, pMessage->mpMessage, uPullSize);

			// Dispose of message storage here
			delete pMessage; pMessage = NULL;
			return uPullSize;

		} else {
			return 0;
		}
	}

	// ::ChannelListenerExists
	//! Returns true if a callback has been installed.
	bool RealChannel::ChannelListenerExists(void) {
		return (mpChannelCallback != NULL);
	}

	// ::InstallChannelListener
	//! Install a new channel listener. May result in queue data being
	//! flushed into the newly installed listener within the calling thread.
	bool RealChannel::InstallChannelListener(ChannelListener *pListener) {
		if (true == mpNodeInfo->Active()) {
			// Swap old and new
			//ChannelListener *pOldListener = mpChannelCallback;
			mpChannelCallback = pListener;
			
			// Flush buffer into newly installed (non-NULL) listener
			if (mpChannelCallback != NULL) {
				mpRxListLock->Enter();
					// The duration of this lock might be excessive - but this should be a rare event.
					// Considered acceptable for now. Alternative is to re-order so that multiple
					// iterations of lock/unlock are used to complete the operation.
					while (false == mlRxList.empty()) {
						Message *pMessage = mlRxList.front();
						mlRxList.pop_front();
						mpChannelCallback->Process(pMessage->mpMessage, pMessage->muSize);
						delete pMessage; pMessage = NULL;
					}
				mpRxListLock->Exit();

			}

			// Interlock with ::InsertMessage to ensure that the ChannelListener being used
			// does not suddenly vanish whilst it is being used.
			while(true == mbStallChannelListenerRemoval) {
				SleepCPU(2);
			}

			// Success
			return true;

		} else {
			// Node inactive - cannot install.	
			return false;
		}
	}


	// Node::
	//*************************************************************************
	// ::Instance
	//! Return the singleton node instance, constructing on initial call.
	//-------------------------------------------------------------------------
	Node *Node::Instance(void) {
		if (NULL == mspInstance) {
			mspInstance = new RealNode();
		}
		return mspInstance;
	}

	//! Shutdown the current instance - cleanup
	//-------------------------------------------------------------------------
	void Node::Shutdown(void) {
		if (mspInstance != NULL) {
			delete mspInstance; mspInstance = NULL;
		}
	}

	// RealNode::
	//*************************************************************************
	// ::RealNode (local scope only)
	//! Construct a new Node object and its internal database. Install bus reset
	//! handler and perform initial bus scan. 
	//-------------------------------------------------------------------------
	RealNode::RealNode(void) :
		mpConnectionListener(NULL), mbOk(false) {

		// Setup critical implementation variables
		suHSS1394Address.mu16Guard = 0x0;
		suHSS1394Address.mu16High = kDefaultHSS1394AddressHi;
		suHSS1394Address.mu32Low = kDefaultHSS1394AddressLo;
		
		// Start API & add bus reset handler
		scBusResetHandler.mpRealNodeInstance = this;
		_1394API::AddBusResetHandler(&scBusResetHandler);
		if (true == _1394API::Start()) {
			scBusResetHandler.BusResetCompleted(_1394API::GetGeneration());
			mbOk = true;	// Mark ok
		}
	}

	// ::IsOk (local scope only)
	//! Return 'true' if node 'ok' flag remains set.
	//-------------------------------------------------------------------------
	bool RealNode::IsOk(void) {
		return mbOk;
	}

	// ::~RealNode (local scope only)
	//! Destructor.
	//-------------------------------------------------------------------------
	RealNode::~RealNode() {
		// Disable bus reset handler indirecting to me
		scBusResetHandler.mpRealNodeInstance = NULL;

		// Cleanup network topology data
		for(std::vector<HSS1394NodeInfo*>::iterator tDBIter = mlNodeDB.begin(); tDBIter != mlNodeDB.end(); tDBIter++) {
			_1394API::RemoveHandler((*tDBIter)->getNetworkId());
			if ((*tDBIter)->mpChannel != NULL) {
				delete (*tDBIter)->mpChannel; (*tDBIter)->mpChannel = NULL;
			}
			delete (*tDBIter)->mpHandler; (*tDBIter)->mpHandler = NULL;
			delete (*tDBIter); (*tDBIter) = NULL;
		}
		mlNodeDB.clear();

		// Shutdown API
		_1394API::Stop();
	}

	// ::Configure1394Handler 
	//! Install the specified 1394 handler. This may cause a new allocation in
	//! 1394 address space for the handler. The call returns the address. The
	//! HSS1394 ChangeReturnAddress command is used to reconfigure the other
	//! end of the connection to use the correct address.
	//-------------------------------------------------------------------------
	bool RealNode::Configure1394Handler(uint uNodeId, Handler *pHandler) {
		// Configure handler and get 1394 address of handler
		bool bSuccess;
		uint48 u1394Address = _1394API::AddHandler(uNodeId, pHandler, bSuccess);
		if (false == bSuccess) {
			return false;
		}

		// Tell other node to send back to this address
		uint8 pReturnAddress[8];	// Two quadlets for address change command
		// Note that address change command is coded in big-endian byte order.
		pReturnAddress[0] = kChangeAddress;
		pReturnAddress[1] = 0x00;
		pReturnAddress[2] = (u1394Address.mu16High>>8)&0xff;
		pReturnAddress[3] = u1394Address.mu16High&0xff;
		pReturnAddress[4] = (u1394Address.mu32Low>>24)&0xff;
		pReturnAddress[5] = (u1394Address.mu32Low>>16)&0xff;
		pReturnAddress[6] = (u1394Address.mu32Low>>8)&0xff;
		pReturnAddress[7] = u1394Address.mu32Low&0xff;
		_1394API::WriteBlockAsync(uNodeId, suHSS1394Address, 
								  pReturnAddress, sizeof(pReturnAddress));
		return true;
	}

	// ::Ping 
	//! Perform a ping to the passed node. Wait for response. On success returns
	//! true and populates the version number argument with the protocol version
	//! number from the responding node. Can operate in read ping or write ping
	//! modes - both are supported by the protocol.
	//-------------------------------------------------------------------------
	bool RealNode::Ping(uint8 uNodeId, uint16 *puVersion, bool bReadPing) {
		if (true == bReadPing) {
			// Perform read ping sequence
			uint8 pBuffer[4];
			if (true == _1394API::ReadBlockAsync(uNodeId, suHSS1394Address, pBuffer, sizeof(pBuffer))) {
				*puVersion = (pBuffer[2]<<8)+pBuffer[3];
				return true;

			} else {
				// No response			
				return false;
			}
		
		} else {
			// Perform write-ping sequence
			PingHandler cPingHandler;

			// Remove old handler
			Handler *pOldHandler = _1394API::RemoveHandler(uNodeId);

			// Insert temporary (ping) handler.
			bool bOk = Configure1394Handler(uNodeId, &cPingHandler);
			if (true == bOk) {
				// Send ping - succeed or fail.
				const unsigned char pMessage[] = {kPing, 0x00, 0x00, 0x00};
				bOk = _1394API::WriteBlockAsync(uNodeId, suHSS1394Address, pMessage, sizeof(pMessage));
				if (true == bOk) {
					// Await response: timeout at 100msec
					bOk = false;
					for(int i=0; i<20; i++) {
						if (true == cPingHandler.GotResponse()) {
							bOk = true;				
							break;
						}
						SleepCPU(5);
					}
				}
				if (puVersion != NULL) {
					*puVersion = cPingHandler.muVersion;
				}
			}

			// Restore old handler
			_1394API::RemoveHandler(uNodeId);			// Remove cPingHandler
			Configure1394Handler(uNodeId, pOldHandler);	// No-op if Null
			return bOk;
		}
	}

	// ::StopAllChannels
	//! Force all channels into stopped state immediately.
	//-------------------------------------------------------------------------
	void RealNode::StopAllChannels(void) {
		for(std::vector<HSS1394NodeInfo*>::iterator tDBIter = mlNodeDB.begin(); tDBIter != mlNodeDB.end(); tDBIter++) {
			RealChannel *pChannel = (*tDBIter)->mpChannel;
			if (pChannel != NULL) {		// Prevent any channel activity during scan
				pChannel->Stop();
			}
		}
	}

	// ::ScanBus
	//! Perform a scan of the 1394 bus and build/update the HSS1394 nodes list. Nodes that
	//! were once here and are now not present are marked as 'dead' but are *not* removed. They
	//! may come live again...
	//-------------------------------------------------------------------------
	void RealNode::ScanBus(void) {
		// Get raw information on current nodes
		std::list<const NodeInfo *> slNodes = _1394API::GetNetworkInfo();

		// Mark all DB nodes as dead until we prove otherwise. Store old state to observe changes.
		for(std::vector<HSS1394NodeInfo*>::iterator tDBIter = mlNodeDB.begin(); tDBIter != mlNodeDB.end(); tDBIter++) {
			RealChannel *pChannel = (*tDBIter)->mpChannel;
			if (pChannel != NULL) {		// Prevent any channel activity during scan
				pChannel->Stop();
			}
			(*tDBIter)->mbWasLive = (*tDBIter)->mbLive;				// Store old live state to see change
			(*tDBIter)->mbLive = false;								// Until proven otherwise
			(*tDBIter)->muOldNetworkId = (*tDBIter)->muNetworkId;	// Store old so I can see a change			
		}

		bool bIdChangesOccurred = false;	// Set true if any node network Ids changed
		bool bActivityChangesOccurred = false;	// Set true if any nodes disappeared or re-appeared
		// Scan list and update mlNodeDB database with new information
		for(std::list<const NodeInfo *>::const_iterator tScanIter = slNodes.begin(); tScanIter != slNodes.end(); tScanIter++) {
			bool bFoundNodeInDB = false;	// Set true if node found in mlNodeDB
			for(std::vector<HSS1394NodeInfo*>::iterator tDBIter = mlNodeDB.begin(); tDBIter != mlNodeDB.end(); tDBIter++) {
				uint64 uScanGUID = (*tScanIter)->getGUID(); uint64 uDBGUID = (*tDBIter)->getGUID();
				if (uScanGUID == uDBGUID) {		// GUID match found. 
					bFoundNodeInDB = true;		// Indicate this node is already in the database
					(*tDBIter)->mbLive = true;	// Mark node as live
					if ((*tScanIter)->getNetworkId() != (*tDBIter)->getNetworkId()) {
						// Network Id has changed - update database
						(*tDBIter)->muNetworkId = (*tScanIter)->getNetworkId();	// NEW network Id
						bIdChangesOccurred = true;
					}
					break;
				}
			}
			if (false == bFoundNodeInDB) {
				// Ping - test is a HSS node? If so, get version. If not - ignore.
				uint16 uVersion;
				if (true == Ping((*tScanIter)->getNetworkId(), &uVersion, true)) {
					// Is a HSS1394 node - add into database
					HSS1394NodeInfo *pNewNode = new HSS1394NodeInfo(*tScanIter);
					pNewNode->muHSS1394Version = uVersion;
					mlNodeDB.push_back(pNewNode);		// Add new node into database. Handler added below...
					bIdChangesOccurred = true;
				}
			}
		}

		// Remove all handlers and re-install those for live nodes. Do in two waves to avoid clashes.
		for(std::vector<HSS1394NodeInfo*>::iterator tDBIter = mlNodeDB.begin(); tDBIter != mlNodeDB.end(); tDBIter++) {
			if (true == (*tDBIter)->mbWasLive) {
				_1394API::RemoveHandler((*tDBIter)->muOldNetworkId);
			}
			// Check if any nodes have just disappeared or re-appeared
			if ((*tDBIter)->mbWasLive != (*tDBIter)->mbLive) {
				(*tDBIter)->SetChannelConnectionState((*tDBIter)->mbLive);
				bActivityChangesOccurred = true;
			}
		}

		// Re-install cycle
		for(std::vector<HSS1394NodeInfo*>::iterator tDBIter = mlNodeDB.begin(); tDBIter != mlNodeDB.end(); tDBIter++) {
			if ((true == (*tDBIter)->mbLive) && ((*tDBIter)->mpHandler != NULL)) {
				// Need to (re-)install handler. If (re-)install fails, we are *not* live.
				(*tDBIter)->mbLive = Configure1394Handler((*tDBIter)->getNetworkId(), ((*tDBIter)->mpHandler));
				RealChannel *pChannel = (*tDBIter)->mpChannel;
				if (pChannel != NULL) {		// Restart on a non-live channel does not get things going...
					pChannel->Restart();
				}
			}
		}

		// Inform user callback of any change
		if ((true == bIdChangesOccurred) || (true == bActivityChangesOccurred)) {
			Listener *pListener = mpConnectionListener;
			if (pListener != NULL) {
				pListener->Do();
			}
		}

		// Clean up storage - return responsibility
		_1394API::ReleaseNetworkInfo(slNodes);
	}


	// ::InstallConnectionListener
	//! Install node 'listener' callback.
	//-------------------------------------------------------------------------
	void RealNode::InstallConnectionListener(Listener *pListener) {
		mpConnectionListener = pListener;
	}
	
	// ::GetNodeCount
	//! Return number of 'known' nodes. This value bounds the range of nodes
	//! that will be reported by ::GetNodeInfo.
	//-------------------------------------------------------------------------
	uint RealNode::GetNodeCount(void) {
		return (uint)mlNodeDB.size();
	}
	
	// ::GetNodeInfo
	//! Returns either a populated tNodeInfo structure etc... and true if the node
	//! is connected, or false if the node is non-existent or not live.
	//-------------------------------------------------------------------------
	bool RealNode::GetNodeInfo(TNodeInfo &tNodeInfo, uint uNode, bool *pbAvailable, bool *pbInstalled) {
		if (uNode < mlNodeDB.size()) {
			// Node exists! - return information from entry
			assert(mlNodeDB[uNode] != NULL);
			// Note - returning data from node, whether live or not
			if (true == mlNodeDB[uNode]->mbLive) {
				mlNodeDB[uNode]->extractNodeInfo(tNodeInfo);
				if (pbAvailable != NULL) {
					*pbAvailable = ((NULL == mlNodeDB[uNode]->mpChannel) && 
									(true == mlNodeDB[uNode]->Active()));
				}
				if (pbInstalled != NULL) {
					*pbInstalled = mlNodeDB[uNode]->isInstalled();
				}
				return true;
			
			} else {
				// Node is not live - do not report it
				return false;
			}

		} else {
			// Node does not exist!
			return false;
		}
	}
	
	// ::OpenChannel
	//! Search list for node. If possible open a channel and return it OR return the current
	//! open channel. 
	//-------------------------------------------------------------------------
	Channel *RealNode::OpenChannel(uint uNode) {
		if (uNode < mlNodeDB.size()) {
			if (NULL == mlNodeDB[uNode]->mpChannel) {
				mlNodeDB[uNode]->mpChannel = new RealChannel(mlNodeDB[uNode]);
			} else {
				mlNodeDB[uNode]->mpChannel->mbRunning = true;
			}
			return mlNodeDB[uNode]->mpChannel;

		} else {
			// No such node!
			return NULL;
		}
	}
	
	// ::ReleaseChannel
	//! Release this channel. After this call, pChannel is no longer valid.
	//-------------------------------------------------------------------------
	bool RealNode::ReleaseChannel(Channel *pChannel) {
		if (NULL == pChannel) {
			return false;	// Cannot delete non-existent channel
		}

		for(std::vector<HSS1394NodeInfo*>::iterator tDBIter = mlNodeDB.begin(); tDBIter != mlNodeDB.end(); tDBIter++) {
			if ((*tDBIter)->mpChannel == pChannel) {	// Found node - de-activate channel
				(*tDBIter)->mpChannel->mbRunning = false;
				return true;
			}
		}

		return false;	// Channel not found - cannot delete
	}

};	// namespace hss1394

