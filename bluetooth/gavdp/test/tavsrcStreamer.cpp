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

#include "tavsrcStreamer.h"
#include "tavsrcUI.h"
#include "tavsrcUtils.h"

#include <bluetoothav.h>

static const TSize KStreamerConsole(55,12);

using namespace SymbianBluetoothAV;
using namespace SymbianSBC;

//
// class CSbcTrackInfo
//
CSbcTrackInfo::~CSbcTrackInfo()
	{
	iFrameInfo.Close();
	}

TInt CSbcTrackInfo::GetLastFrameSize()
	{
	TInt frameSize = KErrNotFound;
	TInt count = iFrameInfo.Count();
	
	if (count > 0)
		{
		frameSize = iFrameInfo[count - 1].iFrameSize;
		}
	return frameSize;
	}

TInt CSbcTrackInfo::AddNewFrame(TInt aFrameSize)
	{
	TInt rerr = KErrNone;
	TInt count = iFrameInfo.Count();

	if ((count > 0) && (iFrameInfo[count - 1].iFrameSize == aFrameSize))
		{
		// another frame of the same size
		iFrameInfo[count - 1].iFrameCount++;
		}
	else
		{
		// add new frame info
		rerr = iFrameInfo.Append(TSbcTrackFrameInfo());
		if (rerr == KErrNone)
			{
			iFrameInfo[count].iFrameSize = aFrameSize;
			iFrameInfo[count].iFrameCount = 1;
			}
		}
	return rerr;
	}

TInt CSbcTrackInfo::RemoveLastFrame()
	{
	TInt rerr = KErrNotFound;
	TInt count = iFrameInfo.Count();
	
	if (count > 0)
		{
		if (iFrameInfo[count - 1].iFrameCount > 1)
			{
			// remove one of the instances of the last frame size
			iFrameInfo[count - 1].iFrameCount--;
			}
		else
			{
			// remove the last frame info
			iFrameInfo.Remove(count - 1);
			}
		rerr = KErrNone;
		}
	return rerr;	
	}

void CSbcTrackInfo::Reset()
	{
	iFrameInfo.Reset();
	}

//
// class CActiveStreamer
//
CActiveStreamer* CActiveStreamer::NewL(RSocketArray aSockets,
									   CConsoleBase& aConsole,
									   MActiveStreamerUser& aUser,
									   TUint aDisplayMode,
									   TBool aPreloadFile)
	{
	CActiveStreamer* self = new (ELeave) CActiveStreamer (aConsole, aUser, aDisplayMode, aPreloadFile);
	CleanupStack::PushL(self);
	self->ConstructL(aSockets);
	CleanupStack::Pop();
	return self;
	}

CActiveStreamer::CActiveStreamer(CConsoleBase& aConsole, MActiveStreamerUser& aUser, TUint aDisplayMode, TBool aPreloadFile)
: iConsole(aConsole), iUser(aUser), iDisplayMode(aDisplayMode), iPreloadFile(aPreloadFile), iSbcFrameRate(1), iDirectionForward(ETrue)
	{
	iRTPCanSend = ETrue;
	}

CActiveStreamer::~CActiveStreamer()
	{
	delete iTimer;

	iSendSource.Close();
	iSession.Close();
	iSockets[0].Close();
	delete iFiles;
	iFile.Close();
	iRFs.Close();
	
	delete iProgressBar;
	DestroyBucket();
	delete iStreamingInfoConsole;	

	delete iFileBuf;
	iFileBuf = NULL;
	
	delete iStreamerUI;
	iStreamerUI = NULL;	
	}

void CActiveStreamer::Stream(TBool aIsSink)
	{

	if (!aIsSink)
		{
		iStartedTime.UniversalTime();
		iTimer->Start(iNominalSendClockInterval);
		Drip();		
		}
	else
		{
		//as sink, we do want to start up rtp and await its notification of a NewSource
		//Nothing to do at the moment, because when INT is a SNK, 
		//it just needs to get the RTP packets running and wait for notification.
		//This has already been done in constructor of the class.		
		}
	iStreamerUI->Play();
	}

void CActiveStreamer::Suspend()
	{
	iTimer->Cancel(); // stop callbacks to send
	iStreamerUI->Pause();
	}
	
void CActiveStreamer::ReStream()
	{


	iTimer->Start(iNominalSendClockInterval);
	Drip();
	iStreamerUI->Play();
	}

	
void CActiveStreamer::Stop()
	{
	iTimer->Cancel();
	iStreamerUI->Stop();

	iFillLevel = 0;
	iPos = 0;
	}

void CActiveStreamer::Pause()
	{
	iTimer->Cancel();
	iStreamerUI->Pause();
	}

	
void CActiveStreamer::NextTrack()
	{
	iPos=0;
	if (iCurrentFile < iFiles->Count() - 1)
		{
		iCurrentFile++;
		iStreamerUI->Next();
		}
	else
		{
		iCurrentFile = 0;
		iStreamerUI->First();
		}
	TRAPD(err, InitL());		
	if (err)
		{
		iConsole.Printf(_L("InitL failed with error: %d"),err);
		}
	}

void CActiveStreamer::PrevTrack()
	{
	iPos=0;
	if (iCurrentFile>0)
		{
		iCurrentFile--;
		iStreamerUI->Prev();
		}
	TRAPD(err, InitL());		
	if (err)
		{
		iConsole.Printf(_L("InitL failed with error: %d"),err);
		}
	}

void CActiveStreamer::InitL()
	{
	// close current file
	iFile.Close();
	delete iProgressBar;
	iProgressBar = NULL;

	// get file details
	RBuf filename;
	filename.Create(100);
	filename.Append(KSBCFileRoot);
	filename.Append(iFiles->operator[](iCurrentFile).iName);
	
	User::LeaveIfError(iFile.Open(iRFs,filename,EFileRead | EFileShareReadersOnly));
	
	TInt numChannels, chMode, numSubbands, blkLen, bitPool, freq, allocMethod;
	User::LeaveIfError(TTavsrcUtils::GetCodecSettingsFromSBCFile(filename, chMode, numChannels, numSubbands, blkLen, bitPool, freq, allocMethod));
	
	if (iStreamingInfoConsole)
		{
		iStreamingInfoConsole->Printf(_L("\nFirst SBC frame info for: %S...\n"), &filename);
		}

	filename.Close();

	// determine if a re-configuration is required
	if ((iNumChannels != numChannels) || (iFreq != freq) ||
		(iChMode != chMode) || (iBitPool != bitPool) ||
		(iBlkLen != blkLen) || (iNumSubbands != numSubbands) ||
		(iAllocMethod != allocMethod))
		{		
		iNumChannels = numChannels;
		iChMode = chMode;
		iNumSubbands = numSubbands;
		iBlkLen = blkLen;
		iBitPool = bitPool;
		iFreq = freq;
		iAllocMethod = allocMethod;
		
		// configuration is set first time around
		if (iSBCFrameSize != 0)
			{
			TSBCCodecCapabilities cfg;

			TSBCSubbandsBitmask subbands = numSubbands == 8 ? EEightSubbands : EFourSubbands;
			TSBCAllocationMethodBitmask alloc = allocMethod == 0 ? ELoudness : ESNR;
			
			TSBCSamplingFrequencyBitmask freqs(0);
			if (freq == 48000) freqs = E48kHz;
			else if (freq == 44100) freqs = E44100Hz; // note else if now as only select one
			else if (freq == 32000) freqs = E32kHz;
			else if (freq == 16000) freqs = E16kHz;
			
			TSBCChannelModeBitmask chs(0); // set it to anything to prevent warning
			if (chMode == 0) chs=EMono; 
			else if (chMode == 1) chs=EDualChannel; 
			else if (chMode == 2) chs=EStereo; 
			else if (chMode == 3) chs=EJointStereo; 
			
			TSBCBlockLengthBitmask blkLens(0); // set it to anything to prevent warning
			if (blkLen == 4) blkLens = EBlockLenFour;
			else if (blkLen == 8) blkLens = EBlockLenEight;
			else if (blkLen == 12) blkLens = EBlockLenTwelve;
			else if (blkLen == 16) blkLens = EBlockLenSixteen;
				
			cfg.SetSamplingFrequencies(freqs);
			cfg.SetChannelModes(chs);
			cfg.SetBlockLengths(blkLens);
			cfg.SetSubbands(subbands);
			cfg.SetAllocationMethods(alloc);
		
			// reconfig required
			iUser.MediaCodecConfigurationRequired(cfg);
			
			// ensure no more timer events until we have finished reconfiguring
			iTimer->Cancel();
			}

		if (chMode == 0 || chMode == 1)
			{
			iSBCFrameSize = 4+TReal((4*numSubbands*numChannels))/8+TTavsrcUtils::CEIL(TReal(blkLen*numChannels*bitPool)/8);
			}
		else
			{
			TBool join = chMode == 0x03;
			
			iSBCFrameSize = 4+TReal((4*numSubbands*numChannels))/8+TTavsrcUtils::CEIL(TReal((join*numSubbands+blkLen*bitPool))/8);
			}

		iSBCBitrate = 8*iSBCFrameSize*freq/(numSubbands*blkLen);
		TUint64 numerator = TUint64(8000000)*iSBCFrameSize;
		iSBCFrameInterval = (numerator)/iSBCBitrate; //microsecs
		}
	
	iSbcTrackInfo.Reset();
	iSbcTrackInfo.AddNewFrame(iSBCFrameSize);
	
	iDirectionForward = ETrue;
	iSbcFrameRate = 1;
	
	User::LeaveIfError(iFile.Size(iFileSize));

	if (iStreamingInfoConsole)
		{
		iStreamingInfoConsole->Printf(_L("Sampling Frequency: %d Hz\n"), iFreq);
		iStreamingInfoConsole->Printf(_L("Subbands: %d\n"), iNumSubbands);
		iStreamingInfoConsole->Printf(_L("BlkLen: %d\n"), iBlkLen);
		iStreamingInfoConsole->Printf(_L("ChannelMode: %d\n"), iChMode);
		iStreamingInfoConsole->Printf(_L("AllocMethod: %d\n"), iAllocMethod);
		iStreamingInfoConsole->Printf(_L("Bitpool: %d\n"), iBitPool);
		iStreamingInfoConsole->Printf(_L("SBC Frame size: %d bytes\n"), iSBCFrameSize);
		iStreamingInfoConsole->Printf(_L("Bitrate: %d bps\n"), iSBCBitrate);
		}
	
	TInt err = LoadFile();
	if(err==KErrNone)
		{
		if (iDisplayMode & EProgressBarWindow)
			{
			iProgressBar = CProgressBar::NewL(iFileSize);
			}
		FillBucket();
		}
	else
		{
		User::Leave(err);
		}
	// start the timer for this file
	iStartTime.UniversalTime();
	}

void CActiveStreamer::UpdateFrameInfo()
	{
	TInt numChannels, chMode, numSubbands, blkLen, bitPool, freq, allocMethod;
	User::LeaveIfError(TTavsrcUtils::GetCodecSettingsFromSBCFile(iFile, iPos, chMode, numChannels, numSubbands, blkLen, bitPool, freq, allocMethod));

	// determine if a re-configuration is required
	if ((iNumChannels != numChannels)|| (iFreq != freq) ||
		(iChMode != chMode) || (iBitPool != bitPool) ||
		(iBlkLen != blkLen) || (iNumSubbands != numSubbands) ||
		(iAllocMethod != allocMethod))
		{		
		iNumChannels = numChannels;
		iChMode = chMode;
		iNumSubbands = numSubbands;
		iBlkLen = blkLen;
		iBitPool = bitPool;
		iFreq = freq;
		iAllocMethod = allocMethod;
					
		TInt newFrameSize = 0;
		if (chMode == 0 || chMode == 1)
			{
			newFrameSize = 4+TReal((4*numSubbands*numChannels))/8+TTavsrcUtils::CEIL(TReal(blkLen*numChannels*bitPool)/8);
			}
		else
			{
			TBool join = chMode == 0x03;
			
			newFrameSize = 4+TReal((4*numSubbands*numChannels))/8+TTavsrcUtils::CEIL(TReal((join*numSubbands+blkLen*bitPool))/8);
			}

		if (newFrameSize != iSBCFrameSize)
			{
			// work out timer for SBC frame
			iSBCBitrate = 8*newFrameSize*freq/(numSubbands*blkLen);

			TUint64 numerator = TUint64(8000000)*iSBCFrameSize;
			iSBCFrameInterval = (numerator)/iSBCBitrate; //microsecs
			}
		iSBCFrameSize = newFrameSize;
		}	
	}

void CActiveStreamer::TimerEvent(CAdaptiveHighResPeriodic& /*aTimer*/)
	{
	if (iRTPCanSend)
		{
		iSent++;
		}
	else
		{
		// move iPos on anyway?
		iFailedSend++;
		}
	DoTimerEvent();
	}

void CActiveStreamer::TimerError(CAdaptiveHighResPeriodic& /*aTimer*/, TInt aError)
	{
	iConsole.Printf(_L("*ERROR %d*\n"), aError);
	__DEBUGGER();
	}
	

void CActiveStreamer::DoTimerEvent()
	{
	FillBucket();
	Drip();
	CheckJammed();	
	}

void CActiveStreamer::FillBucket()
	{
	// fill up bucket - it may be fully empty or partially full, but top it up in all cases	
	if (iFillLevel < KLowTidemark)
		{
		for (/*iFillLevel*/; iFillLevel < KSendBucketSize; iFillLevel++)
			{
			// get the next RTP packet to send
			RRtpSendPacket& sendPacket = iSendPackets[iFillLevel];
			TDes8& payload = sendPacket.WritePayload();
			payload.Zero();
			payload.Append(0); // update this later with number of frames in packet.
			
			TInt spaceInRtpPacket = iSBCFrameBytesPerRTP;
			TInt framesAdded = 0;
			TInt packetInterval = 0;
			
			TInt nextFrameSize = iDirectionForward ? iSBCFrameSize : iSbcTrackInfo.GetLastFrameSize();
			
			TBool moreFrames = ETrue;
			while ((nextFrameSize > 0) && (nextFrameSize <= spaceInRtpPacket) && (framesAdded <= 15) && moreFrames)
				{
				// add frame
				if (iPreloadFile)
					{
					TPtrC8 ptr(iFileBuf->Des().Ptr()+iPos, nextFrameSize);
					payload.Append(ptr);
					}
				else
					{
					TPtr8 ptr(const_cast<TUint8*>(iFileBuf->Des().Ptr()), 0, nextFrameSize);
					iFile.Read(iPos, ptr);
					payload.Append(ptr);
					}
				framesAdded++;
				packetInterval+=iSBCFrameInterval;			
				spaceInRtpPacket-=nextFrameSize;

				// get next frame information
				for (TInt count = 0; (count != iSbcFrameRate) && moreFrames; count++)
					{
					iPos = iDirectionForward ? iPos + nextFrameSize : iPos - nextFrameSize;

					// determine if we are done with the current file
					if ((iPos >= iFileSize) || (iPos < 0))
						{
						moreFrames = EFalse;
						}
					else 
						{
						TInt err = KErrNone;
						if (iDirectionForward)
							{
							// keep track of the frame sizes as we go, this is used to rewind,
							// i.e. iDirectionForward = EFalse
							if ((err = iSbcTrackInfo.AddNewFrame(iSBCFrameSize)) != KErrNone)
								{
								iConsole.Printf(_L("Error adding SBC frame information: %d\n"), err);
								__DEBUGGER();
								}
							}
						else
							{
							if ((err = iSbcTrackInfo.RemoveLastFrame()) != KErrNone)
								{
								// this should never happen as we always check the length first
								iConsole.Printf(_L("Error removing SBC frame information: %d\n"), err);
								__DEBUGGER();
								}
							}
						UpdateFrameInfo();
						nextFrameSize = iDirectionForward ? iSBCFrameSize : iSbcTrackInfo.GetLastFrameSize();
						}
					}				
				}

			// has the interval changed since last time we set the timer
			if ((iNominalSendClockInterval != packetInterval) && moreFrames)
				{
				// adjust timer
				if (iStreamingInfoConsole)
					{
					iStreamingInfoConsole->Printf(_L("Interval change from: %d to %d\n"), iNominalSendClockInterval, packetInterval);
					}
				iNominalSendClockInterval = packetInterval;
				iTimer->SetInterval(iNominalSendClockInterval);
				}
				
			// write number of SBC frames into packet
			payload[0] = TChar(framesAdded);
						
			//DrawBucket();	//<--- to animate the display
			
			if (iPos > iFileSize)
				{
				// time to do some metrics, and loop back to beginning
				TTime finTime;
				finTime.UniversalTime();
				
				TInt64 secs;
				secs = (finTime.MicroSecondsFrom(iStartTime)).Int64();
				
				TInt bps = (iFileSize*8LL*1000000LL)/secs;
				iConsole.Printf(_L("Looping. fail=%d, sent=%d, bytes=%d, secs=%Ld"), iFailedSend, iSent, iFileSize, secs);
				iConsole.Printf(_L(" bps=%d\n"), bps);

				iFailedSend=0;
				iSent=0;
				iPos=0; // loop
				
				RDebug::Printf("Looping");
				NextTrack();
				
				// restart the timer
				iTimer->Start(iNominalSendClockInterval);
				}
			else if (iPos <= 0)
				{
				PrevTrack();
				}
			else if (iProgressBar)
				{
				iProgressBar->Increment(iPos-iProgressBarPos);
				iProgressBarPos = iPos;
				}
			}
		iPreviousFillLevel = iFillLevel;
		}
	}

void CActiveStreamer::DrawBucket()
	{
	if (iStreamingInfoConsole)
		{
		iStreamingInfoConsole->SetPos(1,1);

		TBuf<KSendBucketSize> bar;
		bar.AppendFill('#',iFillLevel);
		bar.AppendFill('.',KSendBucketSize-iFillLevel);
	
		iStreamingInfoConsole->Printf(bar);
		}
	}
	
void CActiveStreamer::Drip()
	{
	// take head packet
	RRtpSendPacket& packet = iSendPackets[0]; //packet=oldpacket

	// move previous packet to back - it is reusable, so we don't close
	// instead just move to back so that the packet to send is at head
	iSendPackets.Remove(0);
	iSendPackets.Append(packet);

	if (iRTPCanSend && iFillLevel > 0)
		{
		// take oldest packet and give to RTP
		packet.Send();
		iRTPCanSend = EFalse;
		iFillLevel--;
		//DrawBucket();	//<--- to animate the display
		}
	else
		{
		// not ready to send yet but when we are send straight away
		iBonusDrip = ETrue;
		
		// remember this as a fail to measure
		// let code beneath recycle packet
		iFailedSend++;
		}		
	}
	
void CActiveStreamer::ConstructL(RSocketArray aSockets)
	{
	iTimer = CAdaptiveHighResPeriodic::NewL(*this);
	
	if (iDisplayMode & EStreamerInfoWindow)
		{
		iStreamingInfoConsole = Console::NewL(_L("Streamer"), KStreamerConsole);
		}
	
	User::LeaveIfError(iRFs.Connect());
	
	iSockets = aSockets;

	TPckgBuf<TInt> mruBuf;
	iSockets[0].GetOpt(EAvdtpMediaGetMaximumReceivePacketSize, KSolBtAVDTPMedia, mruBuf);
	
	// donate media socket to rtp
	iSession.OpenL(iSockets[0], mruBuf());
	
	// we get all RTP events in one place (could have them separately)
	iSession.RegisterEventCallbackL(ERtpAnyEvent,
									RTPCallbackL,
									this);
									
	iSendSource = iSession.NewSendSourceL();

	iSendSource.RegisterEventCallbackL(ERtpAnyEvent,
									RTPCallbackL,
									this);

	iStreamerUI = CStreamerUI::NewL((iDisplayMode & EPlaylistWindow), (iDisplayMode & EChunkyIconWindow));

	TInt err = iRFs.GetDir(KSBCFiles, KEntryAttNormal, ESortByName, iFiles);	

	// set playlist
	for (TInt i=0; i<iFiles->Count(); i++)
		{
		iStreamerUI->AddTitle(iFiles->operator[](i).iName);
		}
	
	err = aSockets[0].GetOpt(EAvdtpMediaGetMaximumPacketSize, KSolBtAVDTPMedia, iMTU);

	iSBCFrameBytesPerRTP = iMTU - 12 - 1;
	iSendSource.SetDefaultPayloadSize(iSBCFrameBytesPerRTP+1);

	CreateBucketL();
	InitL();
	}
	
void CActiveStreamer::CreateBucketL()
	{
	if (iSendPackets.Count() == KSendBucketSize)
		{
		RDebug::Printf("Bucket already created");
		}
	else
		{
		RDebug::Printf("Creating bucket");
		// create all the RTP send packets now
		for (TInt i=0; i<KSendBucketSize ; i++)
			{
			User::LeaveIfError(iSendPackets.Append(iSendSource.NewSendPacketL()));
			RDebug::Printf("Adding Sendpacket 0x%08x in bucket", &iSendPackets[i]);
			}
		}
	}
	
void CActiveStreamer::DestroyBucket()
	{
	RDebug::Printf("Destroying bucket");

	// rtp bug closing these packets?
	iSendPackets.Reset();
	}
	
/*static*/ void CActiveStreamer::RTPCallbackL(CActiveStreamer* aStreamer, const TRtpEvent& aEvent)
	{
	switch (aEvent.Type())
		{
	case ERtpSendFail:
		if (aStreamer->iStreamingInfoConsole)
			{
			aStreamer->iStreamingInfoConsole->Printf(_L("\n**RTP SEND FAILURE**"));			
			}
		break;
		
	case ERtpSendSucceeded:
		aStreamer->iRTPCanSend = ETrue;	
		if (aStreamer->iBonusDrip)
			{
			aStreamer->Drip();
			aStreamer->iBonusDrip = EFalse;
			}
		break;
		
	case ERtpSourceFail:
		if (aStreamer->iStreamingInfoConsole)
			{
			aStreamer->iStreamingInfoConsole->Printf(_L("\n**RTP SOURCE FAILURE**"));			
			}
		break;
		

	case ERtpNewSource:
		if (aStreamer->iStreamingInfoConsole)
			{
			aStreamer->iStreamingInfoConsole->Printf(_L("\n**NEW SOURCE!\n"));
			}
		aStreamer->StartSinkL();
		break;
		
	case ERtpPacketReceived:
		RRtpPacket packet = aStreamer->iReceiveSource.Packet();
		if (aStreamer->iStreamingInfoConsole)
			{
			aStreamer->iStreamingInfoConsole->Printf(_L("SNK Rxd packet "));
			aStreamer->iStreamingInfoConsole->Printf(_L("SeqNo %d\n"),packet.SequenceNumber());
			}
		break;
		}
	}
	
void CActiveStreamer::StartSinkL()
	{
	iReceiveSource = iSession.NewReceiveSourceL();
	iReceiveSource.RegisterEventCallbackL(ERtpAnyEvent,
										RTPCallbackL,
										this);
	}
	
TInt CActiveStreamer::LoadFile()
	{
	RDebug::Printf("Loading file");
	delete iFileBuf;
	iFileBuf = NULL;
	
	TInt err = KErrNone;

	if (iPreloadFile)
		{
		if (iStreamingInfoConsole)
			{
			iStreamingInfoConsole->Printf(_L("Preloading SBC file\n"));
			}

		// max heap is something or other	
		const TInt KMaxHeap = 4000000;
		TInt size = iFileSize;
		size = Min(iFileSize, KMaxHeap);
		TRAP(err, iFileBuf = HBufC8::NewL(size));	
		if (err)
			{
			return err;
			}
		iFileSize = Min(KMaxHeap, iFileSize);
	
		TPtr8 ptr(const_cast<TUint8*>(iFileBuf->Des().Ptr()), 0, iFileSize);	
	
		const TEntry& entry = iFiles->operator[](iCurrentFile);
		RFile test;
		test.Open(iRFs, entry.iName, EFileRead);
		
		ptr.Zero();
		err=iFile.Read(ptr);
		}
	else
		{
		if (iStreamingInfoConsole)
			{
			iStreamingInfoConsole->Printf(_L("Streaming from file\n"));
			}
		
		// read from file to be more "streaming"-like
		TRAP(err, iFileBuf = HBufC8::NewL(Min(iFileSize, iSBCFrameBytesPerRTP)));
		}
	return err;
	}

void CActiveStreamer::Faster()
	{
	// limit the speed
	if (iSbcFrameRate < 5)
		{
		iSbcFrameRate++;
		}
	}
	
void CActiveStreamer::Slower()
	{
	if (iSbcFrameRate > 1)
		{
		iSbcFrameRate--;		
		}
	}

void CActiveStreamer::Backward()
	{
	iDirectionForward=EFalse;
	}

void CActiveStreamer::Forward()
	{
	iDirectionForward=ETrue;
	}

void CActiveStreamer::CheckJammed()
	{
	if (iFillLevel==iPreviousFillLevel)
		{
		if ((iBucketAppearsJammed++ > 500) && iStreamingInfoConsole)
			{
			iStreamingInfoConsole->Printf(_L("BUCKET JAMMED\n"));
			iStreamingInfoConsole->Printf(_L("iFillLevel %d "),iFillLevel);
			iStreamingInfoConsole->Printf(_L("iRTPCanSend %d "),iRTPCanSend);
			iStreamingInfoConsole->Printf(_L("iFailedSend %d "),iFailedSend);
			iStreamingInfoConsole->Printf(_L("iPos %d "),iPos);
	
			TTime now;
			now.UniversalTime();
			
			TInt millisecs = now.MicroSecondsFrom(iLastPacketSentTime).Int64()/1000;
			iStreamingInfoConsole->Printf(_L("time since last send %d ms "),millisecs);
			}
		}
	else
		{
		iBucketAppearsJammed = 0;
		}
	}
