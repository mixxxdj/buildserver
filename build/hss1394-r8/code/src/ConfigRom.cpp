///////////////////////////////////////////////////////////////////////////////
//! \file ConfigRom.cpp
//!
//! Given a data array, interpret as a standard configuration ROM per
//! ISO/IEC 13213:1994. Does *not* perform CRC checks.
//! 
//! Created by Don Goodeve (don@bearanascence.com), 5Jul2008, 17:46PST
//! this file Copyright (C) 2009, Stanton Group.
//! $Id: ConfigRom.cpp 1561 2009-04-16 06:34:40Z don $
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
#include <list>
#include "ConfigRom.h"


//*** Macros
//*****************************************************************************
//#ifndef __BIG_ENDIAN__
//#define GetByte(uWord,uByte)		((uWord>>(uByte<<3))&0xFF)
//#else
//#define GetByte(uWord,uByte)		((uWord>>((3-uByte)<<3))&0xFF)
//#endif

//#define GetBit(uWord,uByte,uBit)	(((GetByte(uWord,uByte)&(1<<uBit)) != 0)?1:0);


//! defines for keys (key_type and key_value)
//-----------------------------------------------------------------------------
#define KEY_VENDOR_ID                   (0x03)
#define KEY_NODE_CAPABILITIES			(0x0C)
#define KEY_POWER_REQUIREMENTS			(0x30)
#define KEY_TEXTUAL_LEAF                (0x81)
#define KEY_NODE_UNIQUE_ID_LEAF_OFFSET  (0x8D)
#define KEY_UNIT_DIRECTORY              (0xD1)
#define KEY_UNIT_SPEC_ID                (0x12)
#define KEY_UNIT_SOFTWARE_VERSION       (0x13)
#define KEY_MODEL_ID                    (0x17)

#define kInvalid						(0xffffffff)


//*** Useful constants
//*****************************************************************************
//! Interpretation of max record size field codes.
//-----------------------------------------------------------------------------
static const hss1394::uint16 mpMaxRecSizeFromCode[] = {
	0xFFFF, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 0xFFFF
};


//*** Local functions
//*****************************************************************************
#ifdef _WIN32_
	// Win32 implementation of OSX core foundation library call to swap
	// word to little-endian byte order. Trivial (identity).
	__inline hss1394::uint32 CFSwapInt32HostToLittle(hss1394::uint32 uWord) {
		return uWord;
	}
#else // Assume OSX
	// Mac - use runtime libraries
	#include <CoreFoundation/CFByteOrder.h>
#endif

//! Accounting for endian-ness, return the uByte-th byte in little-endian order from the
//! passed 32-bit word.
__inline hss1394::uint8 localSelectByteFrom32(hss1394::uint32 uWord, hss1394::uint8 uByte) {
	if (uByte <= 3) {
		uWord = CFSwapInt32HostToLittle(uWord);
		return ((uWord>>(uByte<<3))&0xff);
	} else {
		return 0x0;
	}
}

//! Accounting for endian-ness, return the uBit-th bit from the uByte-th byte
//! in little-endian order from the passed 32-bit word.
__inline hss1394::uint8 localSelectBitFrom32(hss1394::uint32 uWord, hss1394::uint8 uByte, hss1394::uint8 uBit) {
	return ((localSelectByteFrom32(uWord, uByte)&(1<<uBit)) != 0)?1:0;
}


//*** Local classes
//*****************************************************************************
namespace hss1394 {
	//! Leaf node of configuration ROM data structure.
	class ROMLeaf {
		public:
			bool mbValid;
			uint32 muLanguageSpecifier;
			uint32 muLanguageId;
			std::string msText;
			uint8 muType;

		public:
			ROMLeaf(uint8 uType, const uint32 *puData, uint32 uWordsValid) :
				muType(uType) {
				mbValid = true;
				if (uWordsValid <= 0) {
					mbValid = false;
					return;
				}
				uint16 uLength = (localSelectByteFrom32(puData[0],0)<<8) + 
								 localSelectByteFrom32(puData[0],1);
				if (uWordsValid < (uint)(uLength+1)) {
					mbValid = false;
					return;
				}
				muLanguageSpecifier = (localSelectByteFrom32(puData[1],1)<<16) + 
									  (localSelectByteFrom32(puData[1],2)<<8) + 
									  localSelectByteFrom32(puData[2],3);
				muLanguageId = puData[2];
				for(uint uQuadlet=3; uQuadlet<(uint)(uLength+1); uQuadlet++) {
					const char *pText = (const char*)&puData[uQuadlet];
					for(uint uChar=0; uChar<4; uChar++) {
						msText += pText[uChar];
					}
				}
			}
	};

	//! Configuration ROM directory structure
	class ROMDirectory  {
		public:
			bool mbValid;
			uint32 muNodeCapabilities;
			float mfNodePowerRequired;
			uint32 muVendorId;
			uint32 muUnitSpecId;
			uint32 muUnitSoftwareVersion;
			uint32 muModelId;
			std::list<ROMDirectory*> mlDirList;
			std::list<ROMLeaf*> mlLeafList;

		public:
			ROMDirectory(const uint32 *puData, uint32 uWordsValid) :
				muNodeCapabilities(kInvalid), mfNodePowerRequired(0.0f), muVendorId(kInvalid),
				muUnitSpecId(kInvalid), muUnitSoftwareVersion(kInvalid), muModelId(kInvalid) {
				mbValid = true;
				if (uWordsValid <= 1) {
					mbValid = false;
					return;
				}

				// Extract directory length field
				uint uDirLength = (localSelectByteFrom32(puData[0],0)<<8)+
								  (localSelectByteFrom32(puData[0],1));
				if ((uDirLength+1)>uWordsValid) {
					mbValid = false;
					return;
				}

				uint8 muLastKey = 0x00;
				for(uint uQuadlet=1; uQuadlet<(uDirLength+1); uQuadlet++) {
					uint8 uKey = localSelectByteFrom32(puData[uQuadlet], 0);
					uint32 uDatum = (localSelectByteFrom32(puData[uQuadlet],1)<<16) + 
									(localSelectByteFrom32(puData[uQuadlet],2)<<8) + 
									(localSelectByteFrom32(puData[uQuadlet],3));
					uint32 uOffset = (uQuadlet+uDatum);
					switch(uKey) {
						case KEY_NODE_CAPABILITIES: {
							muNodeCapabilities = uDatum;
							break;
						}	
						case KEY_POWER_REQUIREMENTS: {
							mfNodePowerRequired = (uDatum*10.0f);	// In Deciwatts
							break;
						}	
						case KEY_NODE_UNIQUE_ID_LEAF_OFFSET: {
							if (uWordsValid>uOffset) {
								ROMLeaf *pNewLeaf = new ROMLeaf(uKey, &puData[uOffset], uWordsValid-uOffset);
								mlLeafList.push_back(pNewLeaf);
							}
							break;
						}	
						case KEY_VENDOR_ID: {
							muVendorId = uDatum;
							break;
						}
						case KEY_TEXTUAL_LEAF: {
							if (uWordsValid>uOffset) {
								ROMLeaf *pNewLeaf = new ROMLeaf(muLastKey, &puData[uOffset], uWordsValid-uOffset);
								mlLeafList.push_back(pNewLeaf);
							}
							break;
						}
						case KEY_UNIT_DIRECTORY: {
							if (uWordsValid>uOffset) {
								ROMDirectory *pNewDir = new ROMDirectory(&puData[uOffset], uWordsValid-uOffset);
								mlDirList.push_back(pNewDir);
							}
							break;
						}
						case KEY_UNIT_SPEC_ID: {
							muUnitSpecId = uDatum;
							break;
						}
						case KEY_UNIT_SOFTWARE_VERSION: {
							muUnitSoftwareVersion = uDatum;
							break;
						}
						case KEY_MODEL_ID: {
							muModelId = uDatum;
							break;
						}			
						default: {
							break;
						}
					}
					muLastKey = uKey;
				}
			}

			~ROMDirectory() {
				for(std::list<ROMDirectory*>::iterator tDirIterator = mlDirList.begin();
					tDirIterator != mlDirList.end();
					tDirIterator++) {
						delete (*tDirIterator); (*tDirIterator) = NULL;
				}
				mlDirList.clear();

				for(std::list<ROMLeaf*>::iterator tLeafIterator = mlLeafList.begin();
					tLeafIterator != mlLeafList.end();
					tLeafIterator++) {
						delete (*tLeafIterator); (*tLeafIterator) = NULL;
				}
				mlLeafList.clear();
			}
	};

	//! Configuration ROM root structure
	class ROMRoot {
		public:
			bool mbValid;
			bool mbMinimal;
			bool mbIs1394;
			uint32 muVendorId;
			uint48 muChipId;
			uint8 muIrmc;
			uint8 muCmc;
			uint8 muIsc;
			uint8 muBmc;
			uint8 muCyc_Clk_Acc; 
			uint16 muMaxRecordSize;
			ROMDirectory *mpRootDirectory;

		public:
			ROMRoot(const uint32 *puConfigRom, uint32 uWordsValid) :
				muVendorId(0x0), muIrmc(0x0), muCmc(0x0), muIsc(0x0), muBmc(0x0),
				muCyc_Clk_Acc(0x0), muMaxRecordSize(0x0), mpRootDirectory(NULL) {
				muChipId.mu16High = 0x0; muChipId.mu32Low = 0x0;

				// Invalid data trap
				mbValid = true;
				if (uWordsValid <= 0) {
					mbValid = false;
					return;
				}

				// Minimal ROM format handling
				mbMinimal = false;
				uint8 uInfoLength = localSelectByteFrom32(*puConfigRom,0);
				if (0x1 == uInfoLength) {
					// Minimal rom format
					mbMinimal = true;				
					muVendorId = (localSelectByteFrom32(*puConfigRom,1)<<16) + 
								 (localSelectByteFrom32(*puConfigRom,2)<<8) + 
								 localSelectByteFrom32(*puConfigRom,3);
					return;
				}

				// General ROM format handling continues...
				// Bus Information Block (puConfigRom[1] ... puConfigRom[uInfoLength])
				if ((uWordsValid < ((uint)1+uInfoLength)) || (uInfoLength<(uint)4)) {
					// Trap insufficient data
					mbValid = false;
					return;
				}
				
				// Extract relevant data out of bus information block
				// Bus Information Block Word 0
				mbIs1394 = (0 == strncmp((const char*)&puConfigRom[1], "1394", 4));	
				
				// Bus Information Block Word 1
				muIrmc = localSelectBitFrom32(puConfigRom[2],0,7);				
				muCmc  = localSelectBitFrom32(puConfigRom[2],0,6);
				muIsc  = localSelectBitFrom32(puConfigRom[2],0,5);
				muBmc  = localSelectBitFrom32(puConfigRom[2],0,4);
				muCyc_Clk_Acc = localSelectByteFrom32(puConfigRom[2],1);
				uint8 uMaxRecCode = (localSelectByteFrom32(puConfigRom[2],2)>>4)&0x0F;
				muMaxRecordSize = mpMaxRecSizeFromCode[uMaxRecCode];

				// Bus Information Block Word 2
				muVendorId = (localSelectByteFrom32(puConfigRom[3],0)<<16)+
							 (localSelectByteFrom32(puConfigRom[3],1)<<8)+
							 localSelectByteFrom32(puConfigRom[3],2);
				muChipId.mu16High = localSelectByteFrom32(puConfigRom[3],3);

				// Bus Information Block Word 3
				muChipId.mu32Low = (localSelectByteFrom32(puConfigRom[4],0)<<24)+
								   (localSelectByteFrom32(puConfigRom[4],1)<<16)+
								   (localSelectByteFrom32(puConfigRom[4],2)<<8)+
								   localSelectByteFrom32(puConfigRom[4],3);

				// Parse root directory
				mpRootDirectory = new ROMDirectory(&puConfigRom[1+uInfoLength], uWordsValid-(1+uInfoLength));
			}

			~ROMRoot() {
				if (mpRootDirectory != NULL) {
					delete mpRootDirectory; mpRootDirectory = NULL;
				}
			}
	};
};


//*** Global functions
//*****************************************************************************
namespace hss1394 {
	// ParseConfigRom
	//! Given an array of data, attempts to extract the GUID, vendor and
	//! device information strings.
	//-------------------------------------------------------------------------
	bool ParseConfigRom(
		const uint32 *puConfigRom, uint32 uWordsRead, 
		std::string &sName, std::string &sMfr, uint64 &uGUID) {

		// Parse (recursive) the ROM
		ROMRoot *pRoot = new ROMRoot(puConfigRom, uWordsRead);
		if (true == pRoot->mbValid) {
			uGUID.mu32High = (pRoot->muVendorId<<16)+(pRoot->muChipId.mu16High);
			uGUID.mu32Low = pRoot->muChipId.mu32Low;

			// Look for Vendor ID leaf in root directory
			if (pRoot->mpRootDirectory != NULL) {
				for(std::list<ROMLeaf*>::iterator tLeafIter = pRoot->mpRootDirectory->mlLeafList.begin();
					tLeafIter != pRoot->mpRootDirectory->mlLeafList.end();
					tLeafIter++) {
					if ((KEY_VENDOR_ID == (*tLeafIter)->muType) && (true == (*tLeafIter)->mbValid)) {
						sMfr = (*tLeafIter)->msText;
						break;
					}
				}
				// Look for Model ID leaf in unit directory
				if (false == pRoot->mpRootDirectory->mlDirList.empty()) {
					ROMDirectory *pUnitDirectory = pRoot->mpRootDirectory->mlDirList.front();
					for(std::list<ROMLeaf*>::iterator tLeafIter = pUnitDirectory->mlLeafList.begin();
						tLeafIter != pUnitDirectory->mlLeafList.end();
						tLeafIter++) {
						if ((KEY_MODEL_ID == (*tLeafIter)->muType) && (true == (*tLeafIter)->mbValid)) {
							sName = (*tLeafIter)->msText;
							break;
						}
					}
				}
			}
		}

		// Endpiece...
		bool bGood = (pRoot->mbValid);
		delete pRoot; pRoot = NULL;
		return bGood;
	}

};




