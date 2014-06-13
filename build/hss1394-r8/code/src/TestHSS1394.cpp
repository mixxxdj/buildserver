///////////////////////////////////////////////////////////////////////////////
//! \file TestHSS1394.cpp
//!
//! Command-line test application for HSS1394 cross-platform library. Performs
//! simple testing, opening channels to all connected devices and displaying
//! information about traffic. Simple illustration of how to use the library.
//! 
//! Created by Don Goodeve (don@bearanascence.com), 11Jun2008
//! this file Copyright (C) 2009, Stanton Group.
//! $Id: TestHSS1394.cpp 1561 2009-04-16 06:34:40Z don $
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
#ifdef _WIN32
 #include <vld.h>	  // Win32 memory activity debugging.
 #include <windows.h> // Windows stuff
#endif

#include <stdio.h>
#include <list>

#include "1394API.h"
#include "HSS1394.h"

#ifdef WIN32
	#include <windows.h>
#endif

static
void localElevatePriority(void) {
#ifdef WIN32
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#endif
}


//*** Local classes
//*****************************************************************************
class TestHandler : public hss1394::Handler {
	public:
		bool mbRxFlag;
		hss1394::uint8 mpRxBuffer[128];
		hss1394::uint muRxSize;

		TestHandler(void) :
			mbRxFlag(false) {
			muRxSize = 0;
		}

		bool WriteRequest(const hss1394::uint8 *pBytes, hss1394::uint uSize) {
			mbRxFlag = true;
			muRxSize = (uSize<sizeof(mpRxBuffer))?uSize:sizeof(mpRxBuffer);
			memcpy(mpRxBuffer, pBytes, muRxSize);
			return true;
		}

		bool ReceivedYet(void) {
			bool bReturn = mbRxFlag;
			if (true == bReturn) {
				mbRxFlag = false;
			}
			return bReturn;
		}

		void DumpBufferHex(void) {
			printf("0x%02x: ", muRxSize);
			for(int i=0; i<(int)muRxSize; i++) {
				printf("0x%02x%c", mpRxBuffer[i], (i<(int)(muRxSize-1))?',':'\n');
			}
		}
};

class RandomGenerator {
	// A fairly simple and reasonably strong RNG
	private:
		hss1394::_u64 muSeed;
		hss1394::uint muErrors;

		void Init(void) {
			muSeed = 0x12345678;		
			muSeed ^= (muSeed>>32)&0xffffffff;
			muSeed *= muSeed;
		}

		hss1394::uint32 RandomWord(void) {
			muSeed ^= (muSeed>>32)&0xffffffff;
			muSeed *= (muSeed & 0xffffffff);
			muSeed <<= (((muSeed & 0xffffffff) % 3) + 4);
			if (0 == ((muSeed>>32)&0xffffffff)) {
				Init();
			}

			return (hss1394::uint32)((muSeed>>32)&0xffffffff);
		}

	public:
		RandomGenerator(void) : muErrors(0) {
			Init();
		}

		char NextCharacter(void) {
			return (char)(RandomWord()&0xFF);
		}

		bool CheckCharacter(char cCheck) {
			if (cCheck != NextCharacter()) {
				muErrors++;
				return false;
			} else {
				return true;
			}
		}

		bool CheckSequence(const hss1394::uint8 *pCheck, int iLength) {
			bool bOk = true;
			for(int i=0; i<iLength; i++) {
				if (false == CheckCharacter(pCheck[i])) {
					bOk = false;
				}
			}
			return bOk;
		}

		void SetBuffer(hss1394::uint8 *pBuffer, int iLength) {
			for(int i=0; i<iLength; i++) {
				pBuffer[i] = NextCharacter();
			}
		}

		int GetErrors(void) {
			return muErrors;
		}
};


//*** Global functions
//*****************************************************************************
static
class MyBusResetHandler : public hss1394::Handler {
	bool BusReset(hss1394::uint uGeneration) {
		printf("\t\t\t!!!!HANDLER!!!! Generation %d   \n\n", uGeneration);
		return true;
	}
}scBusResetHandler;


static void localShowNetworkInfo(void) {
	using namespace hss1394;
	hss1394::uint uNodes = Node::Instance()->GetNodeCount();
	printf("---------------------\n"
		   "Nodes detected = %d\n", uNodes);
	for(hss1394::uint i=0; i<40; i++) {
		TNodeInfo tNodeInfo;
		bool bInstalled;
		if (true == Node::Instance()->GetNodeInfo(tNodeInfo, i, NULL, &bInstalled)) {
			printf("Node %d (%s): Name = <%s>, GUID = 0x%08x%08x, FW[0x%04x]\n",
				   i, (bInstalled)?"installed":"not installed", 
				   tNodeInfo.sName.c_str(), tNodeInfo.uGUID.mu32High, tNodeInfo.uGUID.mu32Low, tNodeInfo.uProtocolVersion);
		}
	}
}

static
class MyBusListener : public hss1394::Listener {
	void Do(void) {
		localShowNetworkInfo();
	}
}scMyBusListener;


// Class containing a trial packet
class TrialData {
	private:
		hss1394::uint muNodeId;
		hss1394::uint muIndex;
		hss1394::uint muBytes;
		hss1394::uint8 *mpBuffer;

	public:
		TrialData(hss1394::uint uNodeId, hss1394::uint uSequence, hss1394::uint uSize) {
			muNodeId = uNodeId;
			muIndex = uSequence;
			muBytes = uSize;		
			mpBuffer = new hss1394::uint8[uSize];
		}

		TrialData(TrialData &tSource) {
			muNodeId = tSource.muNodeId;
			muIndex = tSource.muIndex;
			muBytes = tSource.muBytes;
			mpBuffer = new hss1394::uint8[muBytes];
			memcpy(mpBuffer, tSource.mpBuffer, muBytes);
		}

		~TrialData() {
			delete mpBuffer; mpBuffer = NULL;
		}

		void SetIndex(hss1394::uint uIndex) {
			muIndex = uIndex;
		}

		hss1394::uint GetIndex(void) {
			return muIndex;
		}

		hss1394::uint GetSize(void) {
			return muBytes;
		}

		hss1394::uint8 *GetMutableDataPtr(void) {
			return mpBuffer;
		}

		void Show(void) {
			printf("\t<%d, %d, [%d]>\n", muIndex, muNodeId, muBytes);
		}

		bool operator==(const TrialData &other) const {
			// Only check node ID and Data 
			return ((other.muNodeId == muNodeId) &&
				    (other.muBytes == muBytes) && 
				    (0==strncmp((const char*)other.mpBuffer, (const char*)mpBuffer, muBytes)));
		}

		bool operator!=(const TrialData &other) const {
			// Only check node ID and Data 
			return !((*this) == other);
		}

};

static hss1394::Lock *spPacketListLock;
static std::list<TrialData*> slPacketList;
static std::list<int> slErrorsList;

class MyChannelListener : public hss1394::ChannelListener {
	private:
		int miId;
		int miTotalRx;
		RandomGenerator cChecker;
		int miPacketErrors;
		friend int main(int argc, char *argv[]);

	public:
		bool mbEcho;
		bool mbMatchQueue;

		MyChannelListener(int iId) {
			miPacketErrors = 0;
			miId = iId;
			miTotalRx = 0;
			mbEcho = false;
			mbMatchQueue = false;
		}
	
		virtual void Process(const hss1394::uint8 *pBuffer, hss1394::uint uBufferSize) {
			miTotalRx += uBufferSize;
			if (true == mbEcho) {
				printf("Rx[%d]: <", miId);
				for(hss1394::uint i=0; i<uBufferSize; i++) {
					printf("0x%02x%c", pBuffer[i], (i<(uBufferSize-1))?',':'>');
				}
				printf("\n");
			} else if (true == mbMatchQueue) {
				// Attempt to match to queue
				if (slPacketList.size() != 0) {
					TrialData *pReceivedPacket = new TrialData(miId, 0, uBufferSize);
					memcpy(pReceivedPacket->GetMutableDataPtr(), pBuffer, uBufferSize);
					bool bMatched = false;
					spPacketListLock->Enter();
					{
						while((false == bMatched) && (false == slPacketList.empty())) {
							// Look through the list from the front to find my match
							// any non-matching packet I find is an error
							if ((*slPacketList.front()) != *pReceivedPacket) {
								miPacketErrors++;
								slErrorsList.push_back(slPacketList.front()->GetIndex());
							} else {
								bMatched = true;
							}
							delete slPacketList.front();
							slPacketList.pop_front();
						}
					}
					spPacketListLock->Exit();
					delete pReceivedPacket; pReceivedPacket = NULL;
				}
			}

			// Deck sends a continuous stream of 0xf9 packets for deck position information. This code
			// intercepts the stream and traps any basic errors.
			static int siF9Messages=0;
			static int siF9Errors=0;
			if (pBuffer[0] != 0xf9) {
				printf("\t\t!!!!Non - 0xf9 PACKET AT APP LAYER!!!! ");
				printf("Rx[%d]: <", miId);
				for(hss1394::uint i=0; i<uBufferSize; i++) {
					printf("0x%02x%c", pBuffer[i], (i<(uBufferSize-1))?',':'>');
				}
				printf("\n");
			} else if (uBufferSize != 4) {
				printf("\t\t!!!!Bad 0xf9 packet length - %d\n", uBufferSize);
				siF9Errors++;
			} else {
				siF9Messages++;
			}
			if ((siF9Messages % 2000) == 0) {
				printf("\t\tAPP received %d 0xf9 messages (0xf9 errors = %d)\n", siF9Messages, siF9Errors);
			}
		}

		virtual void Disconnected(void) {
			printf("Rx[%d]: Disconnected", miId);
		}

		virtual void Reconnected(void) {
			printf("Rx[%d]: Reconnected", miId);
		}

		int GetErrors(void) {
			int iReturn = miPacketErrors;
			miPacketErrors = 0;
			return iReturn;
		}

		void ResetStats(void) {
			miPacketErrors = 0;
			miTotalRx = 0;
		}

		bool ExpectingMore(void) {
			return (false == slPacketList.empty());
		}
};


#define kMaxDevices	(10)


// Simple main - just reports received traffic
int main(int callArgc, char *callArgv[]) {
	using namespace hss1394;
	
	// Start
	spPacketListLock = hss1394::CreateLock();
	slPacketList.clear();
	
	printf("HSS1394 Rx Test.\n");	
	Node::Instance()->InstallConnectionListener(&scMyBusListener);
	localShowNetworkInfo();
	
	// Install listeners on all nodes found
	printf("Installing listeners for all network nodes.\n");
	Channel *pChannel[kMaxDevices];
	MyChannelListener *pChannelListener[kMaxDevices];
	for(hss1394::uint uNode=0; uNode<kMaxDevices; uNode++) {
		if ((pChannel[uNode] = Node::Instance()->OpenChannel(uNode)) != NULL) {
			pChannelListener[uNode] = new MyChannelListener(uNode);
			//pChannelListener[uNode]->mbEcho = true;
			if (false == pChannel[uNode]->InstallChannelListener(pChannelListener[uNode])) {
				delete pChannelListener[uNode]; pChannelListener[uNode] = NULL;
				pChannel[uNode] = NULL;
			}
		} else {
			pChannel[uNode] = NULL;
			pChannelListener[uNode] = NULL;
		}
	}

	while(true) {
		printf("Press x<enter> to finish trial.\n");
		int c = getchar(); getchar();
		if ('x' == c) {
			break;
		}
	}
	printf("Exiting! - Bye!\n\n");
	
	// Clean up the HSS1394Node
	Node::Shutdown();
	for(hss1394::uint uNode=0; uNode<kMaxDevices; uNode++) {
		if (pChannelListener[uNode] != NULL) {
			delete pChannelListener[uNode]; pChannelListener[uNode] = NULL;
		}
	}
	
	// Cleanup missing packets list
	while(false == slPacketList.empty()) {
		delete slPacketList.front();
		slPacketList.pop_front();
	}
	
	printf("\nBye!\n");
	
	exit(0);
	return 0;
}