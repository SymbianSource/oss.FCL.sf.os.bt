// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

#ifndef TAVSRCSTREAMER_H
#define TAVSRCSTREAMER_H

#include "tavsrc.h"
#include "tavsrcTimer.h"
#include "tavsrcStreamerUser.h"

#include <f32file.h>
#include <rtp.h>

static const TInt KLowTidemark = 2;
static const TInt KSendBucketSize = 2;
	
#ifdef __WINS__
_LIT(KSBCFileRoot, "c:\\");
_LIT(KSBCFiles, "c:\\*.sbc");
#else
//For reference boards, use the MMC drive, because C: is formatted every reboot
_LIT(KSBCFileRoot, "e:\\");
_LIT(KSBCFiles, "e:\\*.sbc");
#endif

class CProgressBar;
class CStreamerUI;
class CActiveSockWriter;
class CActiveSockReader;

class TSbcTrackFrameInfo
	{
public:
	TInt iFrameSize;
	TUint iFrameCount;
	};

// used to keep track of the sizes of the SBC frames to allow going backwards through
// an SBC file on a frame by frame basis. This is require to support VBR where the
// frame sizes can change for a single track.
class CSbcTrackInfo : public CBase
	{
public:
	~CSbcTrackInfo();
	
	TInt GetLastFrameSize();
	TInt AddNewFrame(TInt aFrameSize);
	TInt RemoveLastFrame();
	void Reset();
	
private:
	RArray<TSbcTrackFrameInfo> iFrameInfo;
	};

class CActiveStreamer : public CBase, private MAdaptiveHighResPeriodicClient
	{
public:
	static CActiveStreamer*	NewL(RSocketArray aSockets,
								 CConsoleBase& aConsole,
								 MActiveStreamerUser& aUser,
								 TUint aDisplayMode,
								 TBool aPreloadFile);

	~CActiveStreamer();
	
	static void RTPCallbackL(CActiveStreamer* aStreamer, const TRtpEvent& aEvent);
	
	// streamer operations
	void Stream(TBool aIsSink);
	void Suspend();
	void ReStream();
	void Stop();
	void Pause();
	void Faster();
	void Slower();
	void Backward();
	void Forward();
	void NextTrack();
	void PrevTrack();

private:
	enum TDisplayMode
		{
		EStatusCommandWindows = 0x00,
		EStreamerInfoWindow = 0x01,
		EProgressBarWindow = 0x02,
		EPlaylistWindow = 0x04,
		EChunkyIconWindow = 0x08,
		};
	
private:
	CActiveStreamer(CConsoleBase& aConsole, MActiveStreamerUser& aUser, TUint aDisplayMode, TBool aPreloadFile);

	void TimerEvent(CAdaptiveHighResPeriodic& aTimer);
	void TimerError(CAdaptiveHighResPeriodic& aTimer, TInt aError);
	void StartSinkL();
	
	void ConstructL(RSocketArray aSockets);
	TInt LoadFile();
	void DoTimerEvent();
	void DestroyBucket();
	void InitL();

	void CreateBucketL();
	void FillBucket();
	void Drip();
	void DrawBucket();
	void CheckJammed();

	void UpdateFrameInfo();
	
private:
	CConsoleBase& iConsole;
	TInt iProgressBarPos;
	CProgressBar* iProgressBar;
	CStreamerUI* iStreamerUI;
	CConsoleBase* iStreamingInfoConsole;
	CAdaptiveHighResPeriodic* iTimer;
	
	RFile iFile;
	RFs iRFs;	
	CDir* iFiles;
	TInt iCurrentFile;
	RSocketArray iSockets;
	TInt iMTU;
	TInt iFileSize;
	TTime iStartTime;
	
	RRtpSession iSession;
	RRtpSendSource iSendSource;
	RRtpReceiveSource iReceiveSource;
	
	RArray<RRtpSendPacket> iSendPackets; // used like a queue sometimes
	TRtpEventType iEventType;
	TBool iRTPCanSend;

	TInt iSBCFrameBytesPerRTP;
	TInt iSBCFrameSize;
	TInt iSBCFrameInterval;
	TInt iNumSBCFramesInRTP;
	TInt iSBCBitrate;
	TInt iSendClockInterval;		// microsecs
	TInt iNominalSendClockInterval;// microsecs
	TBool iBonusDrip;
	TTime iStartedTime;			// for "absolute" timing style
		
	// checking for jammed streamer, and leaky mbufs
	TInt iBucketAppearsJammed;
	TBool iBucketJammed;
	TTime iLastPacketSentTime;
	TInt iPreviousFillLevel;
	
	TInt iPos;
	TInt iFailedSend;
	TInt iSent;
	TInt iFillLevel;
	
	HBufC8* iFileBuf;
	
	// current SEP configuration
	TInt iNumChannels;
	TInt iChMode;
	TInt iNumSubbands;
	TInt iBlkLen;
	TInt iBitPool;
	TInt iFreq;
	TInt iAllocMethod;
	
	MActiveStreamerUser& iUser;	
	
	TUint iDisplayMode;
	TBool iPreloadFile;
	
	CSbcTrackInfo iSbcTrackInfo;
	
	TInt iSbcFrameRate;
	
	TBool iDirectionForward;
	};

#endif // TAVSRCSTREAMER_H
