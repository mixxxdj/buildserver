/************************************************************************
 *
 *  Module:       ams1394stats.h
 *  Long name:    Device Driver Statistics
 *                
 *  Description:
 *    Declares the programming interface provided by the 
 *    device driver.
 *                
 *
 *  Runtime Env.: 
 *    Windows XP Kernel, Win32
 *
 *  Author(s):   
 *    Udo Eberhardt,  Udo.Eberhardt@thesycon.de
 *    Frank Senf,     Frank.Senf@thesycon.de
 *                
 *  Companies:
 *    Thesycon GmbH, Germany      http://www.thesycon.de
 *                
 ************************************************************************/

// struct alignment = 1 byte
#include <pshpack1.h>



//
// This file can be used with C++ only.
//
#ifndef __cplusplus
#error This file can be used with C++ only.
#endif


//
// ISO stream statistics
//

struct AmsIsoStreamStatistics
{
  // isochronous channel number, 0..63
  //unsigned long IsoChannel;

  // standard statistics

  // counter: number of non-empty isochronous packets processed
  unsigned long infDataPackets;
  // counter: number of empty isochronous packets processed (CIP header only, no data blocks)
  unsigned long infEmptyPackets;
  // data block size in quadlets
  unsigned long infDataBlockSize;
  // sampling frequency in 1/s
	unsigned long infSamplingFreq;
  // SYT_INTERVAL 
  unsigned long infSytInterval;


  // extended statistics
  // These will be maintained by the debug build of the driver only.

  // minimum/maximum packet size, in bytes, processed so far
  // does not include the isochronous packet header
  long extMinPacketSize;
  long extMaxPacketSize;
  // minimum/maximum number of data blocks per (non-empty) packet
  long extMinDataBlocksPerPacket;
  long extMaxDataBlocksPerPacket;
  // minimum/maximum/current callback interval, in microsecs
  long extMinCallbackInterval;
  long extMaxCallbackInterval;
  long extCurrentCallbackInterval;


  // error statistics
  
  // counter: the stream was restarted because of a watchdog request
  unsigned long errWatchdogRestart;

};


struct AmsIsoStreamStatisticsIn : public AmsIsoStreamStatistics
{

  // standard statistics

	// current state of the isochronous packet parser
  unsigned long infReceiverState;


  // error statistics

  // an isochronous packet is missing
  unsigned long errPacketMissing;
  // a data block sequence (DBC) error has been detected
  unsigned long errDataBlockSequenceError;
  // isochronous packet data length is smaller than the expected minimum 
  // which is 8 bytes (CIP header)
  unsigned long errPacketTooShort;
  // isochronous packet data length is greater than the expected maximum
  unsigned long errPacketTooLong;
  // the CIP header does not contain the expected fixed values (FMT)
  unsigned long errInvalidCipHeader;
  // the FDF code is unsupported 
  unsigned long errUnsupportedFormat;
  // the sampling frequency (SFC) has changed
  unsigned long errSamplingFreqMismatch;
  // the data block size has changed (number of channels changed)
  unsigned long errDataBlockSizeMismatch;

};


struct AmsIsoStreamStatisticsOut : public AmsIsoStreamStatistics
{
  // standard statistics

  // extended statistics

  // error statistics

};


//
// statistical information per ASIO host instance
//
struct AmsAsioHostStatistics
{
  // general information and standard statistics

  // number of channels currently acquired by ASIO
  unsigned long infInChannelsAcquired;
  unsigned long infOutChannelsAcquired;
	// set to true if streaming is enabled for all channels
  unsigned long infChannelsEnabled;
  // buffer switch interval currently set
  unsigned long infSwitchInterval_spl;    // in terms of samples
  unsigned long infSwitchInterval_us;     // in microseconds
  // scheduling priority of ASIO worker thread, -1 if unknown
  long infAsioThreadPriority;


	// ERROR:
  // ASIO worker thread has missed at least one switch event
  unsigned long errThreadMissedEvents;
  // Number of events missed by the ASIO worker thread
  unsigned long errThreadDelayed;


	// EXTENDED (maintained by the debug build only):
  // minimum/maximum/current interval between two successive ASIO buffer switch events
  long infMinSwitchInterval;     // in microseconds
  long infMaxSwitchInterval;     // in microseconds
  long infCurrentSwitchInterval; // in microseconds

};



#define AMS_MAX_IN_STREAM_STATISTICS_INFO				1
#define AMS_MAX_OUT_STREAM_STATISTICS_INFO      1
#define AMS_MAX_ASIO_HOST_STATISTICS_INFO				8

//
// Driver statistics info
//
struct AmsFullStatisticsInfo {
  
  // one statistics struct per incoming stream currently active in the driver.
  unsigned long NumberOfInStreams;
  // array that contains NumberOfInStreams elements
  AmsIsoStreamStatisticsIn InStreamStatistics[AMS_MAX_IN_STREAM_STATISTICS_INFO];

  // one statistics struct per outgoing stream currently active in the driver.
  unsigned long NumberOfOutStreams;
  // array that contains NumberOfOutStreams elements
  AmsIsoStreamStatisticsOut OutStreamStatistics[AMS_MAX_OUT_STREAM_STATISTICS_INFO];

  // one statistics struct per ASIO host instance
  unsigned long NumberOfAsioHosts;
  // array that contains NumberOfAsioHosts elements
  AmsAsioHostStatistics AsioHostStatistics[AMS_MAX_ASIO_HOST_STATISTICS_INFO];

};




// restore previous alignment
#include <poppack.h>
