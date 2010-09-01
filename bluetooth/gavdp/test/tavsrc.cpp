// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "tavsrc.h"
#include "tavsrcUtils.h"
#include "tavsrcSock.h"
#include "tavsrcController.h"
#include "tavsrcStreamer.h"

#include <es_sock.h>
#include <bt_sock.h>
#include <bluetoothav.h>
#include <btsdp.h>
#include <flogger.h>
#include <btmanclient.h>
#include <remconinterfaceselector.h>
#include <remconcoreapitarget.h>
#include <remcontrackinfotarget.h>
#include <remconabsvoltarget.h>
#include <remconmediainformationtargetobserver.h>
#include <remconinterfaceselector.h>
#include <remconaddress.h>
#include <bacline.h> 
#ifdef DBAWARE
#include "tavsrcmtupdaterdatabaseaware.h"
#else
#include "tavsrcmtupdaterdatabaseunaware.h"
#endif
#include "tavsrcAbsoluteVolume.h"

#ifdef __WINS__
static const TSize KMainConsole(75,30);
static const TSize KLogConsole(75,30);
#else
static const TSize KMainConsole(KConsFullScreen,KConsFullScreen);
static const TSize KLogConsole(KConsFullScreen,KConsFullScreen);
#endif

using namespace SymbianBluetoothAV;
using namespace SymbianSBC;

void CAVTestApp::PreventLowPowerModes()
	{
	// Prevent any other application from setting to low power mode
	TInt err = iPhy.PreventLowPowerModes(EAnyLowPowerMode);
	__LOG(_L("Low power mode prevented, result %d\n"), err);
	}

void CAVTestApp::AllowLowPowerModes()
	{
	// Allow any other application to set to low power mode
	TInt err = iPhy.AllowLowPowerModes(EAnyLowPowerMode);
	__LOG(_L("Low power mode allowed, result %d\n"), err);
	}

void CAVTestApp::EchoStorm()
	{
	//UPF test
	//connect phy
	CBluetoothPhysicalLinks* phys = NULL;
	TRAP_IGNORE(phys = CBluetoothPhysicalLinks::NewL(*this, iSockServ));
	if (phys)
		{
		phys->CreateConnection(iDevAddr);
		}
	}

 void CAVTestApp::HandleCreateConnectionCompleteL(TInt /*aErr*/)
 	{
 	TBuf8<32> echo;
 	RSocket sock;
 	
 	sock.Open(iSockServ, _L("L2CAP"));
 	TL2CAPSockAddr addr;
 	addr.SetBTAddr(iDevAddr);
 	addr.SetPort(1);
 	
 	TRequestStatus status;
 	
 	__LOG(_L("Connecting l2cap"));
 	sock.Connect(addr, status);
 	User::WaitForRequest(status);

	echo.SetMax();
	echo.Fill('!');
	__LOG(_L("Echooing"));
	FOREVER
		{
 		sock.Ioctl(KL2CAPEchoRequestIoctl, status, &echo, KSolBtL2CAP);
 		User::WaitForRequest(status);
		}
 	}
 	
void CAVTestApp::HandleDisconnectCompleteL(TInt /*aErr*/)
 	{ 	
 	}
 	
void CAVTestApp::HandleDisconnectAllCompleteL(TInt /*aErr*/)
	{
	}

void CAVTestApp::CreateStreamerL(RSocketArray aSockets)
	{
	__LOG(_L("Creating new streamer\n"));
	iStreamer = CActiveStreamer::NewL(aSockets,iLogConsole->Console(), *this, iDisplayMode, iPreloadFile);
	__LOG(_L("OK\n"));		
	}

void CAVTestApp::Abort()
	{
	__PRINT(_L("\n (!ABORT!) Enter remote SEID to ABORT"));
	TInt seid = TTavsrcUtils::GetIntFromUser(iActiveConsole->Console());
	__PRINT(_L("\n"));
	__LOG(_L("Aborting SEID &d...\n"),seid);
	iGavdp.AbortStream(TSEID(seid, EFalse));	
	}

void CAVTestApp::SendSecurityControl()
	{
	__PRINT(_L("\n Enter remote SEID to Send Security Control"));
	TInt seid = TTavsrcUtils::GetIntFromUser(iActiveConsole->Console());
	__PRINT(_L("\n"));
	__LOG(_L("Sending security control to SEID %d...\n"),seid);
	
	TBuf8<20> secBuf(_L8("Danger!\n"));
	
	iGavdp.SendSecurityControl(TSEID(seid, EFalse), secBuf);	
	}
	
void CAVTestApp::GetCapabilities()
	{
	if (!iLocalReconfigure) // Don't change the remote SEID if we're reconfiguring
		{
		if (iAutoStream)
			{
			// start at first in shortlisted sep array
			iCurrentSEID = iShortlistedSEIDs[0];
			iShortlistedSEIDs.Remove(0); // so next time around, take head again
			}
		else
			{		
			__PRINT(_L("\n Enter remote SEID to get caps for"));		
			iCurrentSEID = TSEID(TTavsrcUtils::GetIntFromUser(iActiveConsole->Console()), EFalse);
			}
		}
		
	__PRINT(_L("\n"));
	__LOG(_L("Getting Capabilities for SEID %d...\n"),iCurrentSEID.Value());
	
	iSEPCapabilities.ResetAndDestroy(); //clear what's already there
	
	// we're interested in all caps as we are *TEST CODE*
	// some GCs should only register interest in the capabilities they might select
	TAvdtpServiceCategories caps;
	caps.SetCapability(EAllServiceCategories);
	
	iGavdp.GetRemoteSEPCapabilities(iCurrentSEID, caps);
	}
	

void CAVTestApp::GAVDP_SEPCapability(TAvdtpServiceCapability* aCapability)
	{
	TInt err;
	PrettyPrint(*aCapability);
	
	// we own cap, stash it in our RPointerArray for owning and later use
	err = iSEPCapabilities.Append(aCapability);
	
	if(err == KErrNone)
		{
		// check if remote does SBC
		if (aCapability->Category() == EServiceCategoryMediaCodec)
			{
			if (static_cast<TAvdtpMediaCodecCapabilities*>(aCapability)->MediaCodecType() == EAudioCodecSBC)
				{
				iChosenSEP = ETrue;
				__LOG(_L("Found remote suitable SEP with SEID %d\n"), iCurrentSEID.SEID());
				}
			}
		}
	else
		{
		delete aCapability;
		}
	}
				

void CAVTestApp::GAVDP_SEPDiscoveryComplete()
	{
	// complete the operation
	iOperations->EndOperation(KTavsrcOpDiscoverSEPs, KErrNone);
	
	__LOG(_L("GAVDP:SEP Discovery complete.\n"));
	
	if (iAutoStream)
		{
		// go through all the shortlisted SEPs to find the SBC one
		GetCapabilities();
		}
	TestMenu();
	}

void CAVTestApp::GAVDP_SEPCapabilityComplete()
	{
	// complete the operation
	iOperations->EndOperation(KTavsrcOpGetCapabilities, KErrNone);

	__LOG(_L("GAVDP:SEP has no more capabilities.\n"));

	if ((iAutoStream && iChosenSEP) || iLocalReconfigure)
		{
		// configure iCurrentSEID
		TRAPD(err, ConfigureSEPL());
		if (err != KErrNone)
			{
			__LOG(_L("Error configuring remote SEP: %d\n"), err);
			}
		}
	else if (iAutoStream && !iShortlistedSEIDs.Count())
		{
		__LOG(_L("Error: Remote does not have SBC codec available\n"));
		}
	else if (iAutoStream)
		{
		//try next sep
		GetCapabilities();
		}
	TestMenu();
	}

void CAVTestApp::GAVDP_ConnectConfirm(const TBTDevAddr& aAddr)
	{
	// complete the operation
	iOperations->EndOperation(KTavsrcOpConnect, KErrNone);
	
	// update our tiny state...
	iGavdpState = ESigConnected;	
	
	// hack :o)
	iDevAddr = aAddr;
	__LOG(_L("Signalling channel connected = GAVDP ready\n"));
	
	// Open RBTPhysicalLinkAdapter for prevention of low power modes during streaming
	TInt err = iPhy.Open(iSockServ, iDevAddr);
	__LOG(_L("Opened iPhy, result %d\n"), err);

	TestMenu();
	}

void CAVTestApp::GAVDP_SEPDiscovered(const TAvdtpSEPInfo& aSEPInfo)
	{
	// this test code at present doesnt "remember" the SEPs
	switch (aSEPInfo.MediaType())
		{
		case EAvdtpMediaTypeAudio:
			__LOG(_L("Audio"));
			break;
		case EAvdtpMediaTypeVideo:
			__LOG(_L("Video"));
			break;
		case EAvdtpMediaTypeMultimedia:
			__LOG(_L("Multimedia"));
			break;
		}
		
	if (aSEPInfo.IsSink())
		{
		__LOG(_L(" Sink"));
		}
	else 
		{
		__LOG(_L(" Source"));
		}
	
	__LOG(_L(" SEP (SEID %d)"), aSEPInfo.SEID().Value());
	
	if (aSEPInfo.InUse()) 
		{
		__LOG(_L("[In use]"));
		}

	// as we intend streaming audio, we need a free audio sink sep							
	if (aSEPInfo.MediaType()==EAvdtpMediaTypeAudio && aSEPInfo.IsSink() && !aSEPInfo.InUse())
			{
			 __LOG(_L("<-- Shortlisted SEP"));
			(void)iShortlistedSEIDs.Append(aSEPInfo.SEID());
			}
			
	__LOG(_L("\n"));
	TestMenu();
	}
	
	
void CAVTestApp::GAVDP_AbortStreamConfirm()
	{
	// complete the operation
	iOperations->EndOperation(KTavsrcOpAbort, KErrNone);

	__LOG(_L("GAVDP:Aborted\n"));
	delete iStreamer;
	iStreamer = NULL;

	AllowLowPowerModes();

	TestMenu();
	}
	
void CAVTestApp::GAVDP_SecurityControlConfirm(const TDesC8& aResponseData)
	{
	// complete the operation
	iOperations->EndOperation(KTavsrcOpContentProtection, KErrNone);

	__LOG(_L("GAVDP:Security control complete\n"));
	__LOG(_L("GAVDP:Security control rsp=%S\n"), &aResponseData);
	
	TestMenu();
	}
	
void CAVTestApp::GAVDP_StartStreamsConfirm()
	{
	// complete the operation
	iOperations->EndOperation(KTavsrcOpStartStreams, KErrNone);

	__LOG(_L("GAVDP: StartStreamConfirm: now ready for use\n"));
	
	RThread().SetPriority(EPriorityRealTime);
	
	if (iLocalReconfigure)
		{
		iStreamer->ReStream();
		iStreamState = EStreaming;

		iOperations->EndOperation(KTavsrcOpStream, KErrNone);
		iLocalReconfigure = EFalse;
		}
	else if (iAutoStream)
		{
		//this maynot be true as SNK can be INT (but tavsrc only looks for remote SNKs)
		iStreamer->Stream(CurrentSEIDIsSink());
		iStreamState = EStreaming;

		iOperations->EndOperation(KTavsrcOpStream, KErrNone);
		}
		
	iAutoStream = EFalse;	// done
	iLocalReconfigure = EFalse;

	PreventLowPowerModes();

	TestMenu();
	}
	
void CAVTestApp::GAVDP_SuspendStreamsConfirm()
	{
	// complete the operation
	iOperations->EndOperation(KTavsrcOpSuspendStreams, KErrNone);

	iGavdpState = ESuspended;

	__LOG(_L("Suspending stream!\n"));
	iStreamer->Suspend();
	iStreamState = EPaused;
	
	// we might be doing a reconfigure due to a request from the streamer
	if (iLocalReconfigure)
		{
		GetCapabilities();
		}

	AllowLowPowerModes();

	TestMenu();
	}
	
void CAVTestApp::GAVDP_ConfigurationConfirm()
	{ 
	// complete the operation
	if (iRegisteringLocalSEP)
		{
		__LOG(_L("SEP [SEID %d]: Local Configuration complete\n"), iCurrentSEID.SEID());
		iOperations->EndOperation(KTavsrcOpRegisterSEP, KErrNone);
		
		iRegisteringLocalSEP = EFalse;
		
		// time to listen now SEP  registered, ignore error, logged in Listen()
		Listen();		
		}
	else
		{
		__LOG(_L("SEP [SEID %d]: Remote Configuration complete\n"), iCurrentSEID.SEID());
		iOperations->EndOperation(KTavsrcOpConfigureSEP, KErrNone);

		// we might be doing a reconfigure due to a request from the streamer	
		if (iLocalReconfigure)
			{
			StartStreams();
			}
		// if we;re doing autoCSR stuff we choose to go straight to open
		else if (iAutoStream)
			{
			CreateBearers();
			}
		}

	TestMenu();
	}
	
void CAVTestApp::GAVDP_Error(TInt aError, const TDesC8& /*aErrorData*/)
	{
	__LOG(_L("GAVDP: **Error**: %d:\n"), aError);
	
	// complete the outstanding operation with an error
	iOperations->EndOperation(0, aError);
	
	switch (aError)
		{
		case KErrAvdtpBaseError-KErrAvdtpRequestTimeout: 
			{
			__LOG(_L("AVDTP Request timed out\n"));
			break;
			}
		case -6305:
		case KErrDisconnected:
			{
			iPhy.Close();
			__LOG(_L("Closed iPhy\n"));

			__LOG(_L("Signalling disconnected\nTest code is going to Re-listen...\n"));
			aError = Listen();

			iAutoStream = EFalse;
			if (iStreamer)
				{
				iStreamer->Stop();
				}
			iStreamState = EStopped;

			// reset the state
			iOperations->EndOperation(KTavsrcOpDisconnectSrc, aError);
			break;
			}

		// Errors we can't match to a specific command
		case EAvdtpBadHeaderFormat:
		case EAvdtpBadLength:
		case EAvdtpBadACPSEID:
		case EAvdtpBadPayloadFormat:
		case EAvdtpNotSupportedCommand:
		case EAvdtpBadState:
			{
			__LOG(_L("Couldn't determine operation that caused error\n"));
			break;
			}
		
		// Errors on SetConfig/Reconfig
		case EAvdtpSEPInUse:
		case EAvdtpSepNotInUse:
		case EAvdtpBadServCategory:
		case EAvdtpInvalidCapabilities:
		case EAvdtpBadRecoveryType:	
		case EAvdtpBadMediaTransportFormat:	
		case EAvdtpBadReportingFormat:			
		case EAvdtpBadRecoveryFormat:
		case EAvdtpBadRohcFormat:
		case EAvdtpBadCpFormat:
		case EAvdtpBadMultiplexingFormat:
		case EAvdtpUnsupportedConfiguration:	
		case EGavdpBadService:	
		case EGavdpInsufficientResource:
		case EA2dpInvalidCodec:
		case EA2dpNotSupportedCodec:
		case EA2dpInvalidSamplingFrequency:
		case EA2dpNotSupportedSamplingFrequency:
		case EA2dpInvalidChannelMode:
		case EA2dpNotSupportedChannelMode:
		case EA2dpInvalidSubbands:
		case EA2dpNotSupportedSubbands:
		case EA2dpInvalidAllocationMethod:
		case EA2dpNotSupportedAllocationMethod:
		case EA2dpInvalidMinimumBitPoolValue:
		case EA2dpNotSupportedMinimumBitPoolValue:
		case EA2dpInvalidMaximumBitPoolValue:
		case EA2dpNotSupportedMaximumBitPoolValue:
		case EA2dpInvalidLayer:
		case EA2dpNotSupportedLayer:
		case EA2dpNotSupportedCRC:
		case EA2dpNotSupportedMPF:
		case EA2dpNotSupportedVBR:
		case EA2dpInvalidBitRate:
		case EA2dpNotSupportedBitRate:
		case EA2dpInvalidObjectType:
		case EA2dpNotSupportedObjectType:
		case EA2dpInvalidChannels:
		case EA2dpNotSupportedChannels:
		case EA2dpInvalidVersion:
		case EA2dpNotSupportedVersion:
		case EA2dpNotSupportedSUL:
		case EA2dpInvalidBlockLength:
		case EA2dpInvalidCPType:
		case EA2dpInvalidCPFormat:
			{
			__LOG(_L("Error setting configuration\n"));
			break;
			};
		}

	TestMenu();
	}
	
// passive gubbins
void CAVTestApp::GAVDP_ConfigurationStartIndication(TSEID aLocalSEID, TSEID aRemoteSEID)
	{
	// ah - remote is attempting to confuigure us
	// we need to set our state
	__LOG(_L("Remote SEP [SEID %d] is configuring Local SEP [SEID %d]\n"), aRemoteSEID.Value(), aLocalSEID.Value());

	
	// the seid has already been checked, but we could see which of our endpoints this refers to
	iGavdpState = ERemoteReconfiguring;
	iCurrentSEID = aRemoteSEID;
	iCurrentLocalSEID = aLocalSEID;
	}

TInt CAVTestApp::GAVDP_ConfigurationIndication(TAvdtpServiceCapability* aCapability)
	{
	// the new capability proposed by remote
	__LOG(_L("Configuration proposed: category %d\n"), aCapability->Category());
	
	// for this test code we currently accept everything
	//** A REAL GC SHOULD THINK ABOUT WHAT THE REMOTE IS PROPOSING
	PrettyPrint(*aCapability);
		
	TInt ret = KErrNone;
	if (aCapability->Category() == EServiceCategoryRecovery)
		{
		TAvdtpRecoveryCapabilities* recCap = static_cast<TAvdtpRecoveryCapabilities*>(aCapability);
		if (static_cast<TInt>(recCap->RecoveryType()) == 0xff)
			{
			ret = ConvertToSymbianError::AvdtpError(EAvdtpBadRecoveryType);
			}

		}

	
	if (ret == KErrNone)
		{
		delete aCapability;
		}

	return ret;
	}
	
	
TInt CAVTestApp::GAVDP_ConfigurationEndIndication()
	{
	// just accept all we saw
	__LOG(_L("GAVDP: Remote configuration proposals now finished"));
	__LOG(_L(" - we are replying that all is OK\n"));
	
	// real GC should think about what remote has said!!

	return KErrNone;
	}

TInt CAVTestApp::GAVDP_StartIndication(TSEID aLocalSEID)
	{
	__LOG(_L("Start indication for Local SEID %d\n"), aLocalSEID.SEID());
	TInt err = KErrNone;
	
	if(iStreamer)
		{
		__LOG(_L("Starting streamer (passively!)\n"));
		
		TBool sink = EFalse;
		
		for (TInt i=0; i<iLocallyRegisteredSEPs.Count(); i++)
			{
			// see if we are being a sink or source
			if (aLocalSEID==iLocallyRegisteredSEPs[i].SEID())
				{
				sink = iLocallyRegisteredSEPs[i].IsSink();
				}
			}
		iCurrentLocalSEID = aLocalSEID;
		iStreamer->Stream(sink);
		iStreamState = EStreaming;
		
		iOperations->EndOperation(KTavsrcOpStream, KErrNone);

		PreventLowPowerModes();
		}
	else
		{
		__LOG(_L("No streamer! Rejecting start\n"));
		err = KErrNotReady;
		}
	TestMenu();
	return err;
	}
	
void CAVTestApp::GAVDP_AbortIndication(TSEID aSEID)
	{
	__LOG(_L("GAVDP:Stream %d ABORTED by peer\n"), aSEID.SEID());
	
	iOperations->EndOperation(KTavsrcOpAbort, KErrNone);
	
	delete iStreamer;
	iStreamer = NULL;

	iPhy.Close();
	__LOG(_L("Closed iPhy\n"));

	TestMenu();
	}

void CAVTestApp::GAVDP_ReleaseIndication(TSEID aSEID)
	{
	__LOG(_L("GAVDP:Stream %d RELEASED by peer\n"), aSEID.SEID());

	AllowLowPowerModes();
	}

	
TInt CAVTestApp::GAVDP_SuspendIndication(TSEID aSEID)
	{
	__LOG(_L("GAVDP:Stream %d SUSPENDED by peer\n"), aSEID.SEID());

	iStreamer->Suspend();
	// should ask the streamer object what SEID it is streaming really
	
	// as test code we always consume this for now
	iOperations->EndOperation(KTavsrcOpSuspendStreams, KErrNone);
	
	AllowLowPowerModes();
	
	TestMenu();
	return KErrNone;
	}
	
TInt CAVTestApp::GAVDP_SecurityControlIndication(TSEID aSEID, TDes8& aSecurityData)
	{
	__LOG(_L("GAVDP:Got security control data length %d for SEID %d\n"), aSecurityData.Length(), aSEID.Value());

	TBool identical = TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(), _L("Security control data: respond identically?"));
	if (!identical)
		{
		aSecurityData[0]^='!';
		}

	return KErrNone;
	}

void CAVTestApp::GAVDP_BearerReady(RSocket aNewSocket, const TAvdtpSockAddr& aAddr)
	{
	// wrap socket with active wrapper...
	__LOG(_L("Got a bearer, for session %d\n"), aAddr.Session());

	// we'll make a streamer now (easy way to keep this socket)
	// set the PHY to be AV friendly!
	switch(aAddr.Session())
		{
	case EMedia:
		{
		iPendingSockets[0] = aNewSocket;				

		// complete the operation
		iOperations->EndOperation(KTavsrcOpCreateBearers, KErrNone);

		iGavdpState = EOpen;

		// all bearers ready!
		__LOG(_L("Got all bearers, can now start\n"));
		
		RBTPhysicalLinkAdapter phy;
		
		// AV sockets don't foward opts yet so use addr version
		TInt err = phy.Open(iSockServ, iDevAddr);
		TUint16 packets = EPacketsDH1|EPacketsDH3|EPacketsDH5;
		err = phy.RequestChangeSupportedPacketTypes(packets);
		__LOG(_L("Modified PHY, result %d\n"), err);
		TRAP(err, CreateStreamerL(iPendingSockets)); //FIXME arrange array better for different #bearers
		if(err)
			{
			__LOG(_L("Creating streamer failed with err %d, closing socket\n"), err);
			aNewSocket.Close();
			}
				
		if (iAutoStream)
			{
			StartStreams();
			}
		// but not start streaming until manually started
		
		TestMenu();
		break;
		}
	case EReporting:
		{
		iPendingSockets[1] = aNewSocket;
#ifdef CHANNEL_BINDING_DEBUG_CHECK
			TInt err(KErrNone);
			// Create the active socket reader
			TRAP(err,iRepReader = CActiveSockReader::NewL(iPendingSockets[1], EReporting));
			if (err != KErrNone)
				{
				__LOG(_L("Creating active socket reader failed with error %d"),err);
				}
			else
				{
				iRepReader->Start();
				}
			// Create the active socket writer
			TRAP(err,iRepWriter = CActiveSockWriter::NewL(iPendingSockets[1], EReporting));
			if (err != KErrNone)
				{
				__LOG(_L("Creating active socket writer failed with error %d"),err);
				}
			else
				{
				iRepWriter->Send();		
				}
#endif
		}
		break;
		
	case ERecovery:
		{
		iPendingSockets[2] = aNewSocket;
#ifdef CHANNEL_BINDING_DEBUG_CHECK
			TInt err(KErrNone);
			// Create the active socket reader
			TRAP(err,iRecvReader = CActiveSockReader::NewL(iPendingSockets[2], ERecovery));
			if (err != KErrNone)
				{
				__LOG(_L("Creating active socket reader failed with error %d"),err);
				}
			else
				{
				iRecvReader->Start();
				}
			// Create the active socket writer
			TRAP(err,iRecvWriter = CActiveSockWriter::NewL(iPendingSockets[2], ERecovery));
			if (err != KErrNone)
				{
				__LOG(_L("Creating active socket writer failed with error %d"),err);
				}
			else
				{
				iRecvWriter->Send();		
				}
#endif
		}
		break;
	default:
		__DEBUGGER();
		break;
		}
	}
		
void CAVTestApp::PrettyPrint(TAvdtpServiceCapability& aCapability)
	{
	__LOG(_L("Capability = "));
	
	switch (aCapability.Category())
		{
	case EServiceCategoryMediaTransport:
		__LOG(_L("Media Transport\n"));
		break;

	case EServiceCategoryReporting:
		__LOG(_L("Reporting\n"));
		break;

	case EServiceCategoryRecovery:
		__LOG(_L("Recovery\n"));
		break;

	case EServiceCategoryContentProtection:
		__LOG(_L("Content Protection: Type %d\n"), static_cast<TAvdtpContentProtectionCapabilities&>(aCapability).ContentProtectionType());
		break;

	case EServiceCategoryHeaderCompression:
		__LOG(_L("Header Compression\n"));
		break;

	case EServiceCategoryMultiplexing:
		__LOG(_L("Multiplexing\n"));
		break;

	case EServiceCategoryMediaCodec:
		__LOG(_L("Media Codec\n-----------\n"));
		// print name of codec
		TAvdtpMediaCodecCapabilities& codecCaps = static_cast<TAvdtpMediaCodecCapabilities&>(aCapability);
		switch (codecCaps.MediaType())
			{
		case EAvdtpMediaTypeAudio:
			__LOG(_L("Audio:"));
			break;
		case EAvdtpMediaTypeVideo:
			__LOG(_L("Video:"));
			break;
		case EAvdtpMediaTypeMultimedia:
			__LOG(_L("Multimedia:"));
			break;
			}

		if (codecCaps.MediaCodecType() == EAudioCodecSBC) 
			{
			__LOG(_L("SBC\n"));
		
			TSBCCodecCapabilities& sbcCaps = static_cast<TSBCCodecCapabilities&>(aCapability);
			__LOG(_L("Sampling frequencies: "));
			if (sbcCaps.SamplingFrequencies() & E48kHz) __LOG(_L("48kHz "));
			if (sbcCaps.SamplingFrequencies() & E44100Hz) __LOG(_L("44.1kHz "));
			if (sbcCaps.SamplingFrequencies() & E32kHz) __LOG(_L("32kHz "));
			if (sbcCaps.SamplingFrequencies() & E16kHz) __LOG(_L("16kHz"));
			__LOG(_L("\nChannel modes: "));
			if (sbcCaps.ChannelModes() & EMono) __LOG(_L("Mono "));
			if (sbcCaps.ChannelModes() & EStereo) __LOG(_L("Stereo "));
			if (sbcCaps.ChannelModes() & EJointStereo) __LOG(_L("JointStereo "));
			if (sbcCaps.ChannelModes() & EDualChannel) __LOG(_L("DualChannel"));
			__LOG(_L("\nBlockLengths: "));
			if (sbcCaps.BlockLengths() & EBlockLenFour) __LOG(_L("4 "));
			if (sbcCaps.BlockLengths() & EBlockLenEight) __LOG(_L("8 "));
			if (sbcCaps.BlockLengths() & EBlockLenTwelve) __LOG(_L("12 "));
			if (sbcCaps.BlockLengths() & EBlockLenSixteen) __LOG(_L("16"));					
			__LOG(_L("\nSubbands: "));
			if (sbcCaps.Subbands() & EFourSubbands) __LOG(_L("4 "));
			if (sbcCaps.Subbands() & EEightSubbands) __LOG(_L("8"));
			__LOG(_L("\nAllocation: "));
			if (sbcCaps.AllocationMethods() & ELoudness) __LOG(_L("Loudness "));
			if (sbcCaps.AllocationMethods() & ESNR) __LOG(_L("SNR"));
			__LOG(_L("\nMinBitpool: %d"), sbcCaps.MinBitpoolValue());
			__LOG(_L("\nMaxBitpool: %d\n"), sbcCaps.MaxBitpoolValue());
			}
		else 
			{
			TNonSBCCodecCapabilities& nonSbcCaps = static_cast<TNonSBCCodecCapabilities&>(aCapability);
			TPtrC8 codecData = nonSbcCaps.CodecData();
			
			switch (codecCaps.MediaCodecType())
				{
			case EAudioCodecMPEG12Audio:
				__LOG(_L("MPEG1,2 Audio\n"));
				__LOG(_L("Manually parsing caps of length %d\n"), codecData.Length());
				__LOG(_L("Layers: "));
				if (codecData[0] & 0x80) __LOG(_L("mp1"));
				if (codecData[0] & 0x40) __LOG(_L("mp2"));
				if (codecData[0] & 0x20) __LOG(_L("mp3"));
				__LOG(_L("\nCRC protection: "));
				codecData[0] & 0x10 ? __LOG(_L("yes")) : __LOG(_L("no"));
				__LOG(_L("\nChannel modes: "));
				if (codecData[0] & 0x08) __LOG(_L("Mono "));
				if (codecData[0] & 0x04) __LOG(_L("DualChannel "));
				if (codecData[0] & 0x02) __LOG(_L("Stereo "));
				if (codecData[0] & 0x01) __LOG(_L("JointStereo"));
				__LOG(_L("\nMPF: %d"), codecData[1] & 0x40 ? 1 : 0);
				__LOG(_L("\nSampling frequencies: "));
				if (codecData[1] & 0x20) __LOG(_L("16kHz "));
				if (codecData[1] & 0x10) __LOG(_L("22.05kHz "));
				if (codecData[1] & 0x08) __LOG(_L("24kHz "));
				if (codecData[1] & 0x04) __LOG(_L("32kHz "));
				if (codecData[1] & 0x02) __LOG(_L("44.1kHz "));
				if (codecData[1] & 0x01) __LOG(_L("48kHz"));
				__LOG(_L("\nVBR: "));
				codecData[2] & 0x80 ? __LOG(_L("yes")) : __LOG(_L("no"));
				__LOG(_L("\nBit rate index: %b\n"), codecData[3]+((codecData[2] & 0x7f)<<8));
				break;
			case EAudioCodecMPEG24AAC:
				__LOG(_L("MPEG 2,4 AAC\n"));
				__LOG(_L("Not parsing caps of length %d\n"), nonSbcCaps.CodecData().Length());
				break;
			case EAudioCodecATRAC:
				__LOG(_L("ATRAC\n"));
				__LOG(_L("Not parsing caps of length %d\n"), nonSbcCaps.CodecData().Length());
				break;

			default:
				__LOG(_L("Unknown codec, Type %d\n"), codecCaps.MediaCodecType());
				}
			}
		}
	}

TInt CAVTestApp::Listen()
	{
	TInt err = iGavdp.Listen();
	__LOG(_L("Listening (result %d)\n"), err);
	return err;
	}
	
void CAVTestApp::StartStreams()
	{
	iGavdpState = EOpen;
	TSEID seid;
	
	if (iAutoStream)
		{
		seid = iCurrentSEID;
		}
	else
		{
		__PRINT(_L("\n Enter remote SEID to start streaming"));
		seid = TSEID(TTavsrcUtils::GetIntFromUser(iActiveConsole->Console()),EFalse);
		}
	
	__PRINT(_L("\n"));
	__LOG(_L("Starting remoteSEP %d streaming...\n"),seid.SEID());
	
	iGavdp.StartStream(seid);
	}
	
void CAVTestApp::SuspendStreams()
	{
	TSEID seid;
	
	if (iAutoStream)
		{
		seid = iCurrentSEID;
		}
	else
		{
		__PRINT(_L("\n Enter remote SEID to suspend streaming"));
		seid = TSEID(TTavsrcUtils::GetIntFromUser(iActiveConsole->Console()),EFalse);
		}
	
	__PRINT(_L("\n"));
	__LOG(_L("Suspending remoteSEP %d streaming...\n"),seid.SEID());
	
	iGavdp.SuspendStream(seid);
	iGavdpState = EOpen; 
	}

TSBCCodecCapabilities CAVTestApp::InteractiveSBCMediaCodecConfig(TSBCCodecCapabilities& caps)
	{
	TSBCCodecCapabilities res;
	
	TSBCSamplingFrequencyBitmask freqs = caps.SamplingFrequencies();
	TBool resp = EFalse;
	
	if (freqs)
		{
		__PRINT(_L("\nFreqs: Remote Supports: "));
		if (freqs & E48kHz && !resp)
			{
			resp=TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("48kHz Use"));
			if (resp)
				{
				res.SetSamplingFrequencies(E48kHz);
				}
			}
		if (freqs & E44100Hz && !resp)
			{
			resp=TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\n44.1kHz Use"));
			if (resp)
				{
				res.SetSamplingFrequencies(E44100Hz);
				}
			}
		if (freqs & E32kHz && !resp)
			{
			resp=TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\n32kHz Use"));
			if (resp)
				{
				res.SetSamplingFrequencies(E32kHz);
				}
			}
		if (freqs & E16kHz && !resp)
			{
			resp=TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\n16kHz Use"));
			if (resp)
				{
				res.SetSamplingFrequencies(E16kHz);
				}
			}
		}
	
	resp = EFalse;
	TSBCChannelModeBitmask chmodes = caps.ChannelModes();
	if (chmodes)
		{
		__PRINT(_L("\nChModes: remote supports:"));
		if (chmodes & EJointStereo && !resp)
			{
			resp=TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\nJointStereo Use"));
			if (resp)
				{
				res.SetChannelModes(EJointStereo);
				}
			}
		if (chmodes & EStereo && !resp)
			{
			resp=TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\nStereo Use"));
			if (resp)
				{
				res.SetChannelModes(EStereo);
				}
			}
		if (chmodes & EDualChannel && !resp)
			{
			resp=TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\nDual Ch Use"));
			if (resp)
				{
				res.SetChannelModes(EDualChannel);
				}
			}
		if (chmodes & EMono && !resp)
			{
			resp=TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\nMono Use"));
			if (resp)
				{
				res.SetChannelModes(EMono);
				}
			}
		}

	TSBCBlockLengthBitmask blockLens = caps.BlockLengths();
	resp = EFalse;
	
	if (blockLens)
		{
		__PRINT(_L("\nBlockLens: remote supports:"));
		if (blockLens & EBlockLenFour && !resp)
			{
			resp=TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\n4 Use"));
			if (resp)
				{
				res.SetBlockLengths(EBlockLenFour);
				}
			}
		if (blockLens & EBlockLenEight && !resp)
			{
			resp=TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\n8 Use"));
			if (resp)
				{
				res.SetBlockLengths(EBlockLenEight);
				}
			}
		if (blockLens & EBlockLenTwelve && !resp)
			{
			resp=TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\n12 Use"));
			if (resp)
				{
				res.SetBlockLengths(EBlockLenTwelve);
				}
			}
		if (blockLens & EBlockLenSixteen && !resp)
			{
			resp=TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\n16 Use"));
			if (resp)
				{
				res.SetBlockLengths(EBlockLenSixteen);
				}
			}
		}

	resp = EFalse;
	TSBCSubbandsBitmask subbands = caps.Subbands();
	if (subbands)
		{
		__PRINT(_L("\nSubbands: remote supports:"));
		if (subbands & EFourSubbands && !resp)
			{
			resp=TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\n4 Use"));
			if (resp)
				{
				res.SetSubbands(EFourSubbands);
				}
			}
		if (subbands & EEightSubbands && !resp)
			{
			resp=TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\n8 Use"));
			if (resp)
				{
				res.SetSubbands(EEightSubbands);
				}
			}
		}
		
		
	resp = EFalse;
	TSBCAllocationMethodBitmask allocs = caps.AllocationMethods();
	if (allocs)
		{
		__PRINT(_L("\nAllocation methods: remote supports:"));
		if (allocs & ESNR && !resp)
			{
			resp=TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\nSNR Use"));
			if (resp)
				{
				res.SetAllocationMethods(ESNR);
				}
			}
		if (allocs & ELoudness && !resp)
			{
			resp=TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\nLoudness Use"));
			if (resp)
				{
				res.SetAllocationMethods(ELoudness);
				}
			}
		}

	// cat2 test	
	//res.SetSamplingFrequencies(freqs);	
	//res.SetBlockLengths(4);
	//res.SetSubbands(1);
	//res.SetAllocMethods(1);

	// set bitpool to whatever they said - ok to set range - see 4.3.2.6 A2DP
	res.SetMaxBitpoolValue(caps.MaxBitpoolValue());
	res.SetMinBitpoolValue(caps.MinBitpoolValue());

	//dodgy iWish needs to select one
	//res.SetMinBitpoolValue(42);
	//res.SetMaxBitpoolValue(42);
	
	return res;
	}

TBool CAVTestApp::CurrentSEIDIsSink()
	{
	TBool sink = EFalse;
	TUint localSEPIndex = iCurrentLocalSEID.Value();
	
	//Get the current local SEP index in iLocallyRegisteredSEPs, by screening out flags
	localSEPIndex &= 0x3f;

	sink = iLocallyRegisteredSEPs[localSEPIndex-1].IsSink();
	
	return sink;
	}

void CAVTestApp::ConfigureSEPL()
	{
	TInt res;
	
	// user for test code gets to choose "right" local sep to connect to remote sep
	TSEID localSEPtoUse(1, ETrue); // settings for iAutoStream
	if (!iAutoStream)
		{
		__PRINT(_L("Choose local SEP to use for Stream"));
		localSEPtoUse.Set(TTavsrcUtils::GetIntFromUser(iActiveConsole->Console())); // it *is* local		
		}
	//Record the SEID of chosen local SEP
	iCurrentLocalSEID = localSEPtoUse;
	
	TSEID remoteSEPtoUse = iCurrentSEID; // settings for iAutoStream
	if (!iAutoStream)
		{
		__PRINT(_L("\nChoose remote SEP to use for Stream"));
		remoteSEPtoUse.Set(TTavsrcUtils::GetIntFromUser(iActiveConsole->Console()));
		
		if (remoteSEPtoUse != iCurrentSEID)
			{
			__PRINT(_L("\nWARNING: Do not have the capabilities of the selected remote SEP, "));
			__PRINT(_L("\nthis may lead to sending incorrect configuration information. To"));
			__PRINT(_L("\nfix this Get Capabilities for selected remote SEP first and then"));	
			if (!TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),
					_L("\nconfigure. \nDo you still wish to continue this configuration?")))
				{
				User::Leave(KErrAbort);
				}
			}
		}

	// we have to choose which of our seps to bind with the remote
	// this test code assumes the local SEP to use has SEID 1.
	// real code would have better code to fathom a good binding based on caps etc
	res = iGavdp.BeginConfiguringRemoteSEP(remoteSEPtoUse, localSEPtoUse);
	
	__LOG(_L("Begin configuring remote SEP returned %d\n"), res);
	User::LeaveIfError(res);
	
	for (TInt index=0; index<iSEPCapabilities.Count(); index++)		
		{
		TAvdtpServiceCapability* cap = iSEPCapabilities[index];
		
		TAvdtpServiceCategory cat = cap->Category();
		
		TBool use = EFalse;
		
		if (cat==EServiceCategoryMediaTransport)
			{
			if (iAutoStream)
				{
				use = ETrue;
				}
			else
				{
				use = (TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\nSEP Does Media Transport - use?")));
				}
			if (use)
				{
				res = iGavdp.AddSEPCapability(*cap);
				__LOG(_L("completed: %d"),res);
				}
			}
		if (cat==EServiceCategoryReporting)
			{
			if (iAutoStream)
				{
				use = /*ETrue*/EFalse;
				}
			else
				{
				use = TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\nSEP Does Reporting - use?"));
				}
			if (use)
				{
				res = iGavdp.AddSEPCapability(*cap);
				__LOG(_L("completed: %d"),res);
				}
			}
		if (cat==EServiceCategoryRecovery)
			{
			if (iAutoStream)
				{
				use = /*ETrue*/EFalse;
				}
			else
				{
				use = (TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\nSEP Does Recovery - use?")));
				}
			if (use)
				{
				res = iGavdp.AddSEPCapability(*cap);
				__LOG(_L("completed: %d"),res);
				}
			}
		
		if (cat==EServiceCategoryContentProtection)
			{
			if (iAutoStream)
				{
				use = EFalse;
				}
			else
				{
				use = (TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\nSEP Does Content Protection - use?")));			
				}
				
			if (use)
				{
				res = iGavdp.AddSEPCapability(*cap);
				__LOG(_L("completed: %d"),res);
				}
			}
			
		if (cat==EServiceCategoryMediaCodec)
			{
			if (iAutoStream)
				{
				use = ETrue;
				}
			else
				{
				use = (TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\nConfigure Media Codec?")));
				}
			
			if (use)
				{
				if (static_cast<TBluetoothAudioCodecType>(static_cast<TAvdtpMediaCodecCapabilities*>(cap)->MediaCodecType())==EAudioCodecSBC)
					{
					const TSBCCodecCapabilities& available = *static_cast<TSBCCodecCapabilities*>(cap);
					TSBCCodecCapabilities cfg;

					if (iAutoStream || TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(),_L("\nAutomatically Configure Media Codec?")))
						{
						// we might be doing a reconfigure due to a request from the streamer
						if (iLocalReconfigure)
							{
							cfg = iReconfigInfo;
							}
						else
							{
							// CSR board ignores the configuration
							// which makes playlisting quite easy
							// will need to fix this to playlist to boards that do care
							TInt err, numChannels, chMode, numSubbands, blkLen, bitPool, freq, allocMethod;
							err = TTavsrcUtils::GetCodecSettingsFromSBCFile(iFilename, chMode, numChannels, numSubbands, blkLen, bitPool, freq, allocMethod);
							
							if (err != KErrNone)
								{
								__LOG(_L("Problem accessing SBC file: %d\n"), err);
								__LOG(_L("Warning - Codec settings not obtained\n"));
								}
							else
								{
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
								
								}
							}
							
						// use the available bitpool
						cfg.SetMaxBitpoolValue(available.MaxBitpoolValue());
						cfg.SetMinBitpoolValue(available.MinBitpoolValue());
						}
					else if (use && !iAutoStream)
						{
						cfg = InteractiveSBCMediaCodecConfig(*static_cast<TSBCCodecCapabilities*>(cap));
						}
					
						
					res = iGavdp.AddSEPCapability(cfg);
					__LOG(_L("Add SEP Capability completed: %d"),res);
					}
				else
					{
					__LOG(_L("MP3 codec, choosing fixed configuration...\n"));
					// assume mp3 for now
					// testing with blueant, just choose stereo, 44.1khz
					TBuf8<4> mp3Cfg;
					mp3Cfg.SetLength(4);
					mp3Cfg[0]=0x32;
					mp3Cfg[1]=0x02;
					mp3Cfg[2]=0x00;
					mp3Cfg[3]=0x02;
					
					TNonSBCCodecCapabilities mp3codecCaps(EAvdtpMediaTypeAudio,EAudioCodecMPEG12Audio);
					mp3codecCaps.SetCodecData(mp3Cfg);
					res = iGavdp.AddSEPCapability(mp3codecCaps);
					}
				}
			}
		}// for			

	__LOG(_L("\nCommiting configuration...\n"));
	iGavdp.CommitSEPConfiguration();
	}


TInt CAVTestApp::StartSrc()
	{
	TBool failed = EFalse;
	
	// register source/sink records in sdp
	__LOG(_L("Registering A2DP Sink SDP Record"));
	TRAPD(err, TTavsrcUtils::RegisterSinkSDPRecordL(iSdpDB, iSnkHandle, ETrue, ETrue, ETrue, ETrue));
	if (err != KErrNone)
		{
		__LOG(_L("Failed to Register A2DP Sink SDP Record: %d"), err);
		__DEBUGGER();
		failed = ETrue;
		}
	
	__LOG(_L("Registering A2DP Source SDP Record"));
	TRAP(err, TTavsrcUtils::RegisterSourceSDPRecordL(iSdpDB, iSrcHandle, ETrue, ETrue, ETrue, ETrue));
	if (err != KErrNone)
		{
		__LOG(_L("Failed to Register A2DP Source SDP Record: %d"), err);
		__DEBUGGER();
		failed = ETrue;
		}

	__LOG(_L("Opening GAVDP Session"));
	err = iGavdp.Open(*this, iSockServ);
	if (err == KErrNone)
		{
		RHostResolver hostResolver;
		err = hostResolver.Open( iSockServ, KBTAddrFamily, KBTLinkManager);
		if (err == KErrNone)
			{
			err = hostResolver.SetHostName(_L("Boom Box!")); 
			hostResolver.Close();
			}
		iActiveConsole->Console().Printf(_L("Set Host Name (ret:%d)\r\n"),err);
		
		iGavdpState = EIdle;
		}
	else
		{
		__LOG(_L("Failed to Open GAVDP Session: %d"), err);
		__DEBUGGER();
		failed = ETrue;
		}
	err = RegisterSEP();
	if (err != KErrNone)
		{
		__LOG(_L("Failed to Register SEP: %d"), err);
		__DEBUGGER();
		failed = ETrue;
		}
	
	TRAP(err, CreateRemConInterfacesL());
	if (err != KErrNone)
		{
		__LOG(_L("Failed to Register RemCon interfaces: %d"), err);
		__DEBUGGER();
		failed = ETrue;
		}
	
	if(failed)
		{
		return KErrGeneral;
		}
    return KErrNone;
	}
	
void CAVTestApp::StopSrc()
	{
	// Close RBTPhysicalLinkAdapter
	iPhy.Close();
	__LOG(_L("Closed iPhy\n"));

	iSdpDB.DeleteRecord(iSrcHandle);
	iSdpDB.DeleteRecord(iSnkHandle);
	delete iStreamer;
	iStreamer = NULL;
	iGavdp.Close();
	iAutoStream = EFalse;
	
	iGavdpState = ENoClientOpen;

	}
	
void CAVTestApp::DisconnectSrc()
	{
	// Close RBTPhysicalLinkAdapter
	iPhy.Close();
	__LOG(_L("Closed iPhy\n"));

	delete iStreamer;
	iStreamer = NULL;
	iGavdp.Shutdown();
	iAutoStream = EFalse;
	
	iGavdpState = EIdle;
	Listen();
	}

void CAVTestApp::Connect()
	{
	__LOG(_L("Connecting...\n"));
	iGavdp.Connect(iDevAddr);
	}

void CAVTestApp::CreateBearers()
	{
	TInt ret = KErrNone;
	TSEID seid;
	
	if (iAutoStream)
		{
		seid = iCurrentSEID;
		}
	else
		{
		__PRINT(_L("\n-> Create bearers for remote SEID"));
		seid = TSEID(TTavsrcUtils::GetIntFromUser(iActiveConsole->Console()), EFalse);
		}

	ret = iGavdp.CreateBearerSockets(seid);
	__LOG(_L("Asking for Bearers..(sync_result) %d.\n"), ret);
	}

void CAVTestApp::CloseBearers()
	{
	__LOG(_L("Closing Bearer 0 (via Streamer dtor)...\n\r"));

	delete iStreamer;
	iStreamer = NULL;	

	__LOG(_L("Bearer closed (via Streamer dtor)...\n\r"));

	AllowLowPowerModes();
	}

void CAVTestApp::DiscoverSEPs()
	{
	__LOG(_L("Sending SEP Discovery...\n\r"));
	
	iGavdp.DiscoverRemoteSEPs();
	}
	
CAVTestApp::CAVTestApp()
	{
	}

CAVTestApp::~CAVTestApp()
	{
	StopSrc();
	delete iActiveConsole;
	delete iLogConsole;
	delete iMtUpdater;
	delete iTavsrcAbsoluteVolume;
	delete iStreamer;
	delete iRemConInterfaceSelector;
	delete iController;
	delete iPacketDropIoctl;
	delete iOperations;
	
#ifdef CHANNEL_BINDING_DEBUG_CHECK
	delete iRepReader;
	delete iRepWriter;
	delete iRecvReader;
	delete iRecvWriter;
#endif
	
	iFilename.Close();	
	iLocallyRegisteredSEPs.Close();
	iSockServ.Close();
	iSdpDB.Close();
	iSdp.Close();
	iSEPCapabilities.ResetAndDestroy();
	iShortlistedSEIDs.Close();
	}

CAVTestApp* CAVTestApp::NewL()
	{
	CAVTestApp* thisapp = new (ELeave) CAVTestApp;
	CleanupStack::PushL(thisapp);
	thisapp->ConstructL();
	CleanupStack::Pop();
	return thisapp;
	}

void CAVTestApp::ConstructL()
	{
	iLogConsole = CActiveConsole::NewL(*this,_L("Event Log"),KLogConsole);
	iActiveConsole = CActiveConsole::NewL(*this,_L(" Boom Box "),KMainConsole);
	
	User::LeaveIfError(iSockServ.Connect());
	}

void CAVTestApp::CreateRemConInterfacesL()
	{
	iRemConInterfaceSelector = CRemConInterfaceSelector::NewL();
	
	RArray<TRemConCoreApiOperationId> coreFeatures;
	for(TInt i = 0; i<0x76; i++)
		{
		coreFeatures.Append(static_cast<TRemConCoreApiOperationId>(i));
		}
	
	if(iRemConInterfaces & ECoreApiTarget)
		{
		iRemConTarget = CRemConCoreApiTarget::NewL(*iRemConInterfaceSelector, *this, coreFeatures);
		}
	coreFeatures.Close();
	
	if(iRemConInterfaces & EGroupNavigation)
		{
		iGroupNavigation = CRemConGroupNavigationApiTarget::NewL(*iRemConInterfaceSelector, *this, ETrue, ETrue);
		}
	
	if(iRemConInterfaces & EMediaInformation)
		{
		iMediaInformation = CRemConMediaInformationTarget::NewL(*iRemConInterfaceSelector, *this);
		}

	if(iRemConInterfaces & EBatteryStatus)
		{
		iBatteryStatus = CRemConBatteryApiTarget::NewL(*iRemConInterfaceSelector, *this);
		}

	if(iRemConInterfaces & (EAbsoluteVolumeTarget | EAbsoluteVolumeController))
		{
		iTavsrcAbsoluteVolume = CTavsrcAbsoluteVolume::NewL(*iRemConInterfaceSelector);
		}

	if(iRemConInterfaces & EVendorTrackInfoTarget)
		{
		iTrackInfo = CRemConTrackInfoTarget::NewL(*iRemConInterfaceSelector, *this);
		}
	
	if(iRemConInterfaces & (EPlayerInformation | ENowPlaying | EMediaBrowse ))
		{
		iMtUpdater = CTavsrcMtUpdater::NewL(*iRemConInterfaceSelector, iRemConInterfaces);
		}

	if(iRemConInterfaces & EVendorAbsoluteVolumeTarget)
		{
		iAbsoluteVolume = CRemConAbsVolTarget::NewL(*iRemConInterfaceSelector, *this);
		}

	iController = CTavsrcController::NewL(*iRemConInterfaceSelector, *iLogConsole);

	_LIT8(KTavsrcName, "Tavsrc");
	iRemConInterfaceSelector->OpenTargetL(ERemConAudioPlayer, ERemConNoSubType, KTavsrcName);
	
	iRemConInterfaceSelector->OpenControllerL();
	iOperations = CTavsrcOperations::NewL();
	}

void CAVTestApp::DisplayHelp()
	{
	__LOG(_L("\nUsage: tavsrc [-A <Remote BT Addr>] [-D <Display Mode>] [-P] [-H]\n"));
	__LOG(_L("\n-A <Remote BT Addr>: Optional argument to specify remote\n"));
	__LOG(_L("Bluetooth address. If not specified then you will be prompted\n"));
	__LOG(_L("using notifiers.\n"));
	__LOG(_L("\n-D <Display Mode>: Optional argument to specify the display mode\n"));
	__LOG(_L("to use. The possible values are a logical AND of the following:\n"));
	__LOG(_L("	0x00: Status and Command windows only\n"));
	__LOG(_L("	0x01: Streamer Info window\n"));
	__LOG(_L("	0x02: Progress Bar window\n"));
	__LOG(_L("	0x04: Playlist window\n"));
	__LOG(_L("	0x08: Chunky Icon window\n"));
	__LOG(_L("For example, for all the windows except the Progress Bar: -D d\n"));	
	__LOG(_L("Default value is 1 - Status, Command and Streamer Info windows\n"));
	__LOG(_L("\n-P: Preload SBC file before streaming.\n"));
	__LOG(_L("\n-H: Display this help information.\n"));
	__LOG(_L("\n-R <RemCon interfaces>: Optional argument to specify what interfaces to use:\n"));
	__LOG(_L("		ECoreApiTarget = 1 << 0\n"));
	__LOG(_L("		ECoreApiController = 1 << 1\n"));
	__LOG(_L("		EGroupNavigation = 1 << 2\n"));
	__LOG(_L("		EMediaInformation = 1 << 3\n"));
	__LOG(_L("		EBatteryStatus = 1 << 4\n"));
	__LOG(_L("		EAbsoluteVolumeTarget = 1 << 5\n"));
	__LOG(_L("		EAbsoluteVolumeController = 1 << 6\n"));
	__LOG(_L("		EPlayerInformation = 1 << 7\n"));
	__LOG(_L("		ENowPlaying = 1 << 8\n"));
	__LOG(_L("		EMediaBrowse = 1 << 9\n"));
	__LOG(_L("		EDatabaseAware = 1 << 10\n"));
	__LOG(_L("		ESeparateThreadForBrowse = 1 << 11\n"));
	__LOG(_L("		EVendorTrackInfoTarget = 1 << 12\n"));
	__LOG(_L("		EVendorAbsoluteVolumeTarget = 1 << 13\n"));
	}

void CAVTestApp::ParseCommandLineL()
	{
	CCommandLineArguments *cmdLine = CCommandLineArguments::NewL();
	CleanupStack::PushL(cmdLine);

	// set defaults
	iDisplayMode = 0x01;
	iPreloadFile = EFalse;
	
	TBool btAddrFound = EFalse;
	TBool remConInterfacesFound = EFalse;
	TBuf<20> arg;
	for (TInt argIndex = 1; argIndex < cmdLine->Count(); argIndex++)
		{
		arg = cmdLine->Arg(argIndex);
		arg.UpperCase();
		
		// look for help
		if (arg.FindF(_L("-H")) != KErrNotFound)
			{
			DisplayHelp();
			User::Leave(KErrArgument);
			}

		// look for preload file
		if (arg.FindF(_L("-P")) != KErrNotFound)
			{
			iPreloadFile = ETrue;
			continue;
			}

		// look for remote device address
		if (arg.FindF(_L("-A")) != KErrNotFound)
			{
			if (argIndex != cmdLine->Count() - 1)
				{
				btAddrFound = ETrue;
				iDevAddr.SetReadable(cmdLine->Arg(++argIndex));
				continue;
				}
			else
				{
				__LOG(_L("No address found for -A option\n"));
				DisplayHelp();
				User::Leave(KErrArgument);
				}
			}

		// look for display mode
		if (arg.FindF(_L("-D")) != KErrNotFound)
			{
			if(argIndex != cmdLine->Count() - 1)
				{
				TLex lex(cmdLine->Arg(++argIndex));
				lex.Val(iDisplayMode, EHex);
				continue;
				}
			else
				{
				__LOG(_L("No display mode found for -D option\n"));
				DisplayHelp();
				User::Leave(KErrArgument);
				}
			}
		
		// look for remcon interfaces
		if (arg.FindF(_L("-R")) != KErrNotFound)
			{
			if(argIndex != cmdLine->Count() - 1)
				{
				TLex lex(cmdLine->Arg(++argIndex));
				TInt err = lex.Val(iRemConInterfaces, EHex);
				if(!err)
					{
					remConInterfacesFound = ETrue;
					}
				else
					{
					__LOG(_L("Badly formatted interface bitmask\n"));
					}
				continue;
				}
			else
				{
				__LOG(_L("No remconinterfaces found for -R option\n"));
				DisplayHelp();
				User::Leave(KErrArgument);
				}
			}

		// if we got here it means that we have an unhandled argument
		__LOG(_L("Unrecognised argument\n"));
		DisplayHelp();
		User::Leave(KErrArgument);
		}
	CleanupStack::PopAndDestroy(); // cmdLine

	if (!btAddrFound)
		{
		__LOG(_L("No address found on command line - please enter an address\n"));
		TTavsrcUtils::GetDeviceAddressL(iDevAddr);
		}

	if(!remConInterfacesFound)
		{
		__LOG(_L("No interfaces found on command line, defaulting to all interfaces\n"));
		iRemConInterfaces = 0xFFFFFFFF;
		}
	}

void CAVTestApp::StartL()
	{
	// connect to sdp
	__LOG(_L("Connecting to ESOCK\n"));
	User::LeaveIfError(iSockServ.Connect());
	__LOG(_L("Connecting to SDP Server\n"));

	__LOG(_L("Connecting to SDP Server\n"));
	User::LeaveIfError(iSdp.Connect());
	__LOG(_L("Opening subsession on SDP Server\n"));

	__LOG(_L("Opening subsession on SDP Server\n"));
	User::LeaveIfError(iSdpDB.Open(iSdp));
	TBuf<512> cmdline;
	
	// ensure that we have something to stream
	RFs fs;
	CDir* files;
	User::LeaveIfError(fs.Connect());

	TInt err = fs.GetDir(KSBCFiles, KEntryAttNormal, ESortByName, files);

	fs.Close();

	if (err != KErrNone)
		{
		__LOG(_L("Error getting files %S: %d \n"), &KSBCFiles, err);		
		User::Leave(err);		
		}
		
	if (files->Count() > 0)
		{
		// store the filename for registering the SEP later
		iFilename.Create(KMaxFileName);	
		iFilename.Append(KSBCFileRoot);
		iFilename.Append(files->operator[](0).iName);
		delete files;
		}
	else
		{
		__LOG(_L("\nNo files matching %S found, at least one is required,\n"), &KSBCFiles);
		__LOG(_L("please add files and restart the application\n"));
		delete files;
		User::Leave(KErrNotFound);
		}

	TRAP(err, ParseCommandLineL());
	if (err != KErrNone)
		{
		iActiveConsole->Console().ClearScreen();
		iActiveConsole->Console().Printf(_L("\nProblem with Command Line arguments, see status window for"));
		iActiveConsole->Console().Printf(_L("\nmore information or press ESC to exit.\n"));
		iActiveConsole->RequestKey();
		}
	else
		{
		err = StartSrc();
		if(err == KErrNone)
			{
			TestMenu();
			}
		else
			{
			iActiveConsole->Console().ClearScreen();
			iActiveConsole->Console().Printf(_L("\nProblem starting source, see status window for"));
			iActiveConsole->Console().Printf(_L("\nmore information or press ESC to exit.\n"));
			iActiveConsole->RequestKey();
			}
		}
	CActiveScheduler::Start();
	}

void CAVTestApp::Stop()
	{
	CActiveScheduler::Stop();
	}
	
//remcon
 void CAVTestApp::MrccatoCommand(TRemConCoreApiOperationId aOperationId, 
		TRemConCoreApiButtonAction aButtonAct)
 	{
 	__LOG(_L("RemCon::Command received 0x%02x\t Button Act %d\n"), aOperationId, aButtonAct);
 	if((aOperationId == ERemConCoreApiStop) &&
 	   (aButtonAct == ERemConCoreApiButtonClick || aButtonAct == ERemConCoreApiButtonPress) )
 		{
	 	__LOG(_L("RemCon::Stop received\n"));
		iAutoStream = EFalse;
		if (iStreamer)
			{
			iStreamer->Stop();
			}
		iStreamState = EStopped;
		iOperations->EndOperation(KTavsrcOpStopStream, KErrNone);
 		}
 	else if (((aOperationId == ERemConCoreApiBackward) || (aOperationId == ERemConCoreApiRewind)) && iStreamer)
 		{
 		if (aButtonAct == ERemConCoreApiButtonClick)
 			{
			iStreamer->PrevTrack();
 			}
 		else if ((aButtonAct == ERemConCoreApiButtonPress) && (iStreamState == EStreaming))
 			{
			iStreamer->Backward();
			iStreamer->Faster();
 			}
 		else if ((aButtonAct == ERemConCoreApiButtonRelease) && (iStreamState == EStreaming))
 			{
			iStreamer->Forward();
			iStreamer->Slower();
 			}
 		}
 	else if (((aOperationId == ERemConCoreApiForward) || (aOperationId == ERemConCoreApiFastForward)) && iStreamer)
 		{
 		if (aButtonAct == ERemConCoreApiButtonClick)
 			{
			iStreamer->NextTrack();
 			}
 		else if ((aButtonAct == ERemConCoreApiButtonPress) && (iStreamState == EStreaming))
 			{
 			iStreamer->Faster();
 			}
 		else if ((aButtonAct == ERemConCoreApiButtonRelease) && (iStreamState == EStreaming))
 			{
			iStreamer->Slower();
 			}
 		}
 	else if ((aOperationId == ERemConCoreApiPause) && (aButtonAct != ERemConCoreApiButtonRelease))
 		{
 		TInt operation = KTavsrcOpStream;
 		
		if (iStreamState == EStreaming)
			{
			// this demo app doesn't suspend GAVDP, just the streamer
			if (iStreamer)
				{
				iStreamer->Suspend();
				}
			iStreamState = EPaused;
			operation = KTavsrcOpStopStream;
			}
		iOperations->EndOperation(operation, KErrNone);
 		}	

 	if (iStreamState == EStreaming)
 		{
 		PreventLowPowerModes();
 		}

 	TestMenu();
 	}
 	
 void CAVTestApp::MrccatoPlay(TRemConCoreApiPlaybackSpeed /*aSpeed*/, 
		TRemConCoreApiButtonAction aButtonAct)
 	{
 	__LOG(_L("RemCon::Play received\t Button Act %d\n"), aButtonAct);
 	if((aButtonAct == ERemConCoreApiButtonClick || aButtonAct == ERemConCoreApiButtonPress))
 	   	{
 	   	TInt operation = KTavsrcOpStream;
 	   	
   		// kick everything off
	   	if (iGavdpState == ESigConnected)
			{			
			// auto
			iAutoStream = ETrue;
			// kick off
			DiscoverSEPs();
			}
 		else if (iGavdpState == ESuspended)
 			{
			if (iStreamer)
				iStreamer->ReStream();
			iStreamState = EStreaming;
			}
		else if (iGavdpState == EOpen)
			{
			if (iStreamState == EPaused)
	 			{
				// this demo app doesn't unsuspend GAVDP, just the streamer
				if (iStreamer)
					iStreamer->ReStream();
				iStreamState = EStreaming;
				}
			else if (iStreamState == EStopped)
				{
				// this demo app doesn't unsuspend GAVDP, just the streamer
				if (iStreamer)
					iStreamer->Stream(CurrentSEIDIsSink()); //could be source or sink
				iStreamState = EStreaming;
				}
			else if (iStreamState == EClosed)
				{
				if (iStreamer)
					iStreamer->Stream(CurrentSEIDIsSink());
				iStreamState = EStreaming;
				}
			}
		iOperations->EndOperation(operation, KErrNone);

	 	if (iStreamState == EStreaming)
	 		{
	 		PreventLowPowerModes();
	 		}

	 	TestMenu();
 	   	}
 	}
 
void CAVTestApp::MediaCodecConfigurationRequired(TSBCCodecCapabilities& aConfig)
	{
	if (!iLocalReconfigure)
		{
		// set the reconfigure information
		iReconfigInfo = aConfig;
		iLocalReconfigure = ETrue;

		// start streaming automatically after the reconfigure
		iAutoStream = ETrue;
		
		SuspendStreams();
		}	
	}

void CAVTestApp::PrintCommandOption(TChar aOperation, TPtrC aDesc)
	{
	if (iOperations->IsOperationAllowed(aOperation))
		{		
		iActiveConsole->Console().Printf(_L("%c%s"), aOperation.IsUpper() ? aOperation.GetUpperCase() : aOperation.GetLowerCase(), aDesc.Ptr());
		}
	else
		{
		iActiveConsole->Console().Printf(_L(" %s"), aDesc.Ptr());	
		}
	}

// from MRemConMediaInformationTargetObserver
void CAVTestApp::MrcmitoGetCurrentlyPlayingMetadata( TMediaAttributeIter& aAttributeIter )
	{
	iActiveConsole->Console().Printf(_L("Got MrcpitoGetElementAttributes \n") );	

	// Make sure total of these defines is greater than KAVCMaxFrame (512 bytes)
	// to ensure that fragmentation is performed on this response
	_LIT8(KMediaTitle,     "Frederic Chopin (1810-1849): Ballade for Piano #1 in A flat major, Opus 23, CT 2 - Largo allegretto, Moderato con brio vivace, Presto con fuoco");
	_LIT8(KArtistName,     "Maurizio Pollini and the London Philharmonic Orchestra, conducted by Sir Colin Davis, recorded live at the Royal Albert Hall at the 1987 Proms season");
	_LIT8(KAlbumName,      "Frederic Chopin (1810-1849): Four Ballades, Two Preludes, the Valse in D-flat major - Op. 64 No. 1 and the Andante Spianato & Grand Polonaise Brillante Op. 22");
	_LIT8(KTrackNumber,    "10345");
	_LIT8(KNumberOfTracks, "6876436456");
	_LIT8(KGenre,          "Alternative super cool Classical Music with a lemon twist and a hint of progressive rock");
	_LIT8(KPlayingTime,    "150000");
	
	TBuf8<255> buffer;
	
	// for each element requested
	TMediaAttributeId id;
	while (aAttributeIter.Next(id))
		{
		switch(id)
			{
			case ETitleOfMedia:
				buffer.Copy(KMediaTitle);
				break;
			case ENameOfArtist:
				buffer.Copy(KArtistName);
				break;
			case ENameOfAlbum:
				buffer.Copy(KAlbumName);
				break;
			case ETrackNumber:
				buffer.Copy(KTrackNumber);
				break;
			case ENumberOfTracks:
				buffer.Copy(KNumberOfTracks);
				break;
			case EGenre:
				buffer.Copy(KGenre);
				break;
			case EPlayingTime:
				buffer.Copy(KPlayingTime);
				break;
			default:
				__DEBUGGER();
				break;
			}
		
		// return the element value
		iMediaInformation->AttributeValue( id, buffer );
		
		// convert the element value to unicode for display
		TBuf16<255> buf16;
		buf16.Copy(buffer);
		buf16.ZeroTerminate();
		iActiveConsole->Console().Printf(_L("Element:%d value:%s \n"),  id, buf16.Ptr() );	
		}

	// send response complete
	iMediaInformation->Completed();
		
	}


TInt CAVTestApp::MrcmtcGetElementAttributes(TUint64 /* aElement */, TUint32 aAttribute, HBufC8*& aOutValueOwnershipTransferred)
	{
//
//	__DEBUGGER(); //try this out
	iActiveConsole->Console().Printf(_L("Got GEA, attrib %d!\n"), aAttribute);	

	aOutValueOwnershipTransferred = HBufC8::New(20);
	if (aOutValueOwnershipTransferred)
		{
		aOutValueOwnershipTransferred->Des() = _L8("Hello");
		}
		
	return KErrNone;	
	}

void CAVTestApp::MrcncRegisterPlaybackStatusChangedNotification()
	{
	iActiveConsole->Console().Printf(_L("Queued notify: PlayStatusChanged!\n"));	
	}
	
void CAVTestApp::MrcncRegisterTrackChangedNotification()
	{
	iActiveConsole->Console().Printf(_L("Queued notify: TrackChanged!\n"));	
	}

void CAVTestApp::MrcncRegisterTrackReachedEndNotification()
	{
	iActiveConsole->Console().Printf(_L("Queued notify: TrackReachedEnd!\n"));	
	}

void CAVTestApp::MrcncRegisterTrackReachedStartNotification()
	{
	iActiveConsole->Console().Printf(_L("Queued notify: TrackReachedStart!\n"));	
	}

void CAVTestApp::MrcncRegisterPositionChangedNotification(TUint32 aInterval)
	{
	iActiveConsole->Console().Printf(_L("Queued notify: PositionChanged (0x%08x)!\n"), aInterval);	
	}

void CAVTestApp::MrcncRegisterBatteryStatusChangedNotification()
	{
	iActiveConsole->Console().Printf(_L("Queued notify: BatteryStatusChanged!\n"));	
	}

void CAVTestApp::MrcncRegisterSystemStatusChangedNotification()
	{
	iActiveConsole->Console().Printf(_L("Queued notify: SystemStatusChanged!\n"));	
	}

void CAVTestApp::MrcncRegisterPlayerApplicationStatusChangedNotification()
	{
	iActiveConsole->Console().Printf(_L("Queued notify: PlayerAppStatusChanged!\n"));	
	}
	
void CAVTestApp::MrcbstoBatteryStatus(TControllerBatteryStatus&  aBatteryStatus )
	{
	switch(aBatteryStatus)
		{
		case ENormal:
			iActiveConsole->Console().Printf(_L("Battery status: Normal \n"));	
			break;
		case EWarning:   
			iActiveConsole->Console().Printf(_L("Battery status: Warning \n"));	
			break;
		case ECritical:  
			iActiveConsole->Console().Printf(_L("Battery status: Critical \n"));	
			break;
		case EExternal:  
			iActiveConsole->Console().Printf(_L("Battery status: External \n"));	
			break;
		case EFullCharge:
			iActiveConsole->Console().Printf(_L("Battery status: FullCharge \n"));	
			break;
		default:
			iActiveConsole->Console().Printf(_L("Battery status: %d \n"), aBatteryStatus);	
			break;
		}
	}

void CAVTestApp::MrcgntoNextGroup(TRemConCoreApiButtonAction aButtonAct )
	{
	switch(aButtonAct)
		{
		case ERemConCoreApiButtonPress:
			iActiveConsole->Console().Printf(_L("Got NextGroup! Button Press \n"));	
			break;
		case ERemConCoreApiButtonRelease:
			iActiveConsole->Console().Printf(_L("Got NextGroup! Button Release \n"));	
			break;
		case ERemConCoreApiButtonClick:
			iActiveConsole->Console().Printf(_L("Got NextGroup! Button Click \n"));	
			break;
		default:
			iActiveConsole->Console().Printf(_L("Got NextGroup! Button ??? \n"));	
			break;
		}
		
	// for testing return an error	
	TRequestStatus status;
	TRequestStatus* ptrStatus = &status;
	iGroupNavigation->NextGroupResponse( ptrStatus, KErrNone);
	User::WaitForRequest(status);
	
	}
	
void CAVTestApp::MrcgntoPreviousGroup(TRemConCoreApiButtonAction  aButtonAct )
	{
	switch(aButtonAct)
		{
		case ERemConCoreApiButtonPress:
			iActiveConsole->Console().Printf(_L("Got PreviousGroup! Button Press \n"));	
			break;
		case ERemConCoreApiButtonRelease:
			iActiveConsole->Console().Printf(_L("Got PreviousGroup! Button Release \n"));	
			break;
		case ERemConCoreApiButtonClick:
			iActiveConsole->Console().Printf(_L("Got PreviousGroup! Button Click \n"));	
			break;
		default:
			iActiveConsole->Console().Printf(_L("Got PreviousGroup! Button ??? \n"));	
			break;
		}
		
	// return success	
	TRequestStatus status;
	TRequestStatus* ptrStatus = &status;
	iGroupNavigation->PreviousGroupResponse( ptrStatus, KErrNone );
	User::WaitForRequest(status);
	}

void CAVTestApp::MrcavtoGetAbsoluteVolume()
	{
	TRequestStatus status;
	iAbsoluteVolume->GetAbsoluteVolumeResponse(status, 1, 2, KErrNone);
	User::WaitForRequest(status);
	}

void CAVTestApp::MrcavtoSetAbsoluteVolume(TUint /*aAbsVol*/, TUint /*aMaxVol*/)
	{
	TRequestStatus status;
	iAbsoluteVolume->SetAbsoluteVolumeResponse(status, KErrNone);
	User::WaitForRequest(status);
	}

void CAVTestApp::MrctitoGetTrackName()
	{
	TRequestStatus status;
	_LIT(KTrackName, "trackname");
	iTrackInfo->GetTrackNameResponse(status, KTrackName, KErrNone);
	User::WaitForRequest(status);
	}

void CAVTestApp::MrctitoGetArtist()
	{
	TRequestStatus status;
	_LIT(KArtist, "artist");
	iTrackInfo->GetArtistResponse(status, KArtist, KErrNone);
	User::WaitForRequest(status);
	}

void CAVTestApp::MrctitoGetTrackDuration()
	{
	TRequestStatus status;
	_LIT(KTrackDuration, "0.2.56");
	TTime trackDuration(KTrackDuration);
	iTrackInfo->GetTrackDurationResponse(status, trackDuration, KErrNone);
	User::WaitForRequest(status);
	}

_LIT(KOn, "On");
_LIT(KOff, "Off");

void CAVTestApp::TestMenu()
	{
	iActiveConsole->Console().ClearScreen();

	iActiveConsole->Console().Printf(_L("Safe Mode: %S\n"), iOperations->SafeMode() ? &KOn() : &KOff());

	if (iGavdpState!=EIdle)
		{
		iActiveConsole->Console().Printf(_L("Connected \n\r"));	
		}
	else
		{
		iActiveConsole->Console().Printf(_L("          \n\r"));	
		}
	
	RProperty property;
	TBuf8<6> addr;

	TInt err = property.Get(KPropertyUidBluetoothCategory,
					KPropertyKeyBluetoothGetLocalDeviceAddress, addr);

	if ((err) || (addr.Length()!=6))
		{
		iActiveConsole->Console().Printf(_L("P&S: ERROR retrieving local address\n"));
		}
	else
		{
		TBTDevAddr localAddress(addr);
		TBuf<20> dispBuf;
		localAddress.GetReadable(dispBuf);
		TBuf<20> rBuf;
		iDevAddr.GetReadable(rBuf);
		iActiveConsole->Console().Printf(_L("Local address = 0x%S; Using Remote Addr = 0x%S\n"),&dispBuf,&rBuf);
		}
	
	iActiveConsole->Console().Printf(_L("\n"));

	switch (iGavdpState)
		{
		case ENoClientOpen:
			PrintCommandOption(KTavsrcOpStartSrc, _L(". Open GAVDP for Src\n"));
			break;
		case EIdle:
			PrintCommandOption(KTavsrcOpConnect, _L(". Connect\n"));
			PrintCommandOption(KTavsrcOpCancel, _L(". Cancel\n"));
			PrintCommandOption(KTavsrcOpRegisterSEP, _L(". Register SEP\n"));
			break;
		case ESigConnected:
		case EOpen:
		case ESuspended:
		case ERemoteReconfiguring:
			PrintCommandOption(KTavsrcOpAutoStream, _L(". Auto Stream\n"));
			PrintCommandOption(KTavsrcOpDiscoverSEPs, _L(". Discover remote SEPs             "));
			PrintCommandOption(KTavsrcOpCreateBearers, _L(". Create Bearers\n"));
			
			PrintCommandOption(KTavsrcOpGetCapabilities, _L(". Get Remote SEP Capabilites       "));
			PrintCommandOption(KTavsrcOpCloseBearers, _L(". Close Bearers\n"));
			
			PrintCommandOption(KTavsrcOpStartStreams, _L(". Start Stream                     "));
			PrintCommandOption(KTavsrcOpSuspendStreams, _L(". Suspend Stream\n"));

			PrintCommandOption(KTavsrcOpContentProtection, _L(". Content Protection               "));
			PrintCommandOption(KTavsrcOpAbort, _L(". Abort Stream\n"));

			PrintCommandOption(KTavsrcOpConfigureSEP, _L(". Select Remote SEP (configure the doofer)\n"));

			PrintCommandOption(KTavsrcOpStream, _L(". Start Streamer                   "));
			PrintCommandOption(KTavsrcOpStopStream, _L(". Stop Streamer\n\n"));

			PrintCommandOption(KTavsrcOpPacketDropIoctl, _L(". Send \"Notify Media Packet Dropped\" IOCTL\n\n"));

			PrintCommandOption(KTavsrcOpDisconnectSrc, _L(". Disconnect GAVDP - don't close\n"));
			PrintCommandOption(KTavsrcOpStopSrc, _L(". Close GAVDP\n"));			
			break;
		}

	iActiveConsole->Console().Printf(_L("\n"));
	
	PrintCommandOption(KTavsrcOpPlay, _L(". AVRCP Play                       "));
	PrintCommandOption(KTavsrcOpStop, _L(". AVRCP Stop\n"));

	iActiveConsole->Console().Printf(_L("Up. AVRCP Volume Up                 "));
	iActiveConsole->Console().Printf(_L("Down. AVRCP Volume Down\n"));

	iActiveConsole->Console().Printf(_L("Left. AVRCP Backwards               "));
	iActiveConsole->Console().Printf(_L("Right. AVRCP Forwards\n"));

	iActiveConsole->Console().Printf(_L("\n"));

	PrintCommandOption(KTavsrcOpToggleSafeMode, _L(".\tToggle Safe Mode\n"));
	iActiveConsole->Console().Printf(_L("Esc.\tStop\n"));
	
	if (!iActiveConsole->IsActive())
		{
		iActiveConsole->RequestKey();
		}
	}

void CAVTestApp::KeyPressed(TChar aKey)
	{
	TInt beginOperation = iOperations->BeginOperation(aKey);

	if (beginOperation == KErrNone)
		{
		switch (aKey)
			{
		case KTavsrcOpConnect:
			{
			Connect();
			break;
			}
		case KTavsrcOpCancel:
			{
			iOperations->EndOperation(KTavsrcOpCancel, KErrCancel);

			iGavdp.Cancel();
			__LOG(_L("Current Request Cancelled \n"));
			break;
			}
		case KTavsrcOpDiscoverSEPs:
			{
			DiscoverSEPs();
			break;
			}
		case KTavsrcOpCreateBearers:
			{
			CreateBearers();
			break;
			}
		case KTavsrcOpCloseBearers:
			{
			iOperations->EndOperation(KTavsrcOpCloseBearers, KErrNone);

			CloseBearers();
			break;
			}
		case KTavsrcOpContentProtection:
			{
			SendSecurityControl();
			break;
			}
		case KTavsrcOpGetCapabilities:
			{
			GetCapabilities();
			break;
			}
		case KTavsrcOpStartStreams:
			{
			StartStreams();
			break;
			}
		case KTavsrcOpSuspendStreams:
			{
			SuspendStreams();
			break;
			}
		case KTavsrcOpEchoStorm:
			{
			iOperations->EndOperation(KTavsrcOpEchoStorm, KErrNone);

			EchoStorm();
			break;
			}
		case KTavsrcOpAbort:
			{
			Abort();
			break;
			}
		case KTavsrcOpStream:
			{
			iOperations->EndOperation(KTavsrcOpStream, KErrNone);

			iStreamer->Stream(CurrentSEIDIsSink());
			iStreamState = EStreaming;
			}
			break;		
		case KTavsrcOpStreamFaster:
			{
			iOperations->EndOperation(KTavsrcOpStreamFaster, KErrNone);

			iStreamer->Faster();
			break;
			}
			
		case KTavsrcOpStreamSlower:
			{
			iOperations->EndOperation(KTavsrcOpStreamSlower, KErrNone);

			iStreamer->Slower();
			break;
			}
			
		case KTavsrcOpAutoStream:
			{
			// auto
			iAutoStream = ETrue;
			// kick off
			DiscoverSEPs();
			}
			break;

		case KTavsrcOpStopStream:
			{
			iOperations->EndOperation(KTavsrcOpStopStream, KErrNone);

			__LOG(_L("Stopping streaming... \n"));
			iStreamState = EPaused;
			iStreamer->Suspend();
			}
			break;
		case KTavsrcOpRegisterSEP:
			{
			iOperations->EndOperation(KTavsrcOpRegisterSEP, KErrNone);

			RegisterSEP();
			break;
			}
		case KTavsrcOpRegisterMultipleSEPs:
			{		
			iOperations->EndOperation(KTavsrcOpRegisterMultipleSEPs, KErrNone);

			for (TInt i=0; i<=40; i++)
				{
				TAvdtpSEPInfo info; iGavdp.RegisterSEP(info);
				}
			break;
			}
		case KTavsrcOpStartSrc:
			{
			iOperations->EndOperation(KTavsrcOpStartSrc, KErrNone);

			//reopen GAVDP
			StartSrc();
			break;
			}
				
		case KTavsrcOpStopSrc:
			{
			iOperations->EndOperation(KTavsrcOpStopSrc, KErrNone);

			StopSrc();
			break;
			}	
		case KTavsrcOpDisconnectSrc:
			{
			iOperations->EndOperation(KTavsrcOpDisconnectSrc, KErrNone);

			DisconnectSrc();
			break;
			}	
		case KTavsrcOpConfigureSEP:
			{
			TRAPD(err, ConfigureSEPL());
			if (err != KErrNone)
				{
				iOperations->EndOperation(KTavsrcOpConfigureSEP, err);
				}
			break;
			}
	
		case KTavsrcOpPacketDropIoctl:
			{
			iOperations->EndOperation(KTavsrcOpPacketDropIoctl, KErrNone);

			__LOG(_L("Sending packet drop IOCTL\n"));
			iPacketDropIoctl = CActivePacketDropIoctl::NewL(iLogConsole, iPendingSockets);//Qualified
			iPacketDropIoctl->Start();
			break;
			}
	
		case KTavsrcOpVolumeUp:
			{
			iOperations->EndOperation(KTavsrcOpVolumeUp, KErrNone);

			iController->Command(ERemConCoreApiVolumeUp);
			break;
			}
			
		case KTavsrcOpVolumeDown:
			{
			iOperations->EndOperation(KTavsrcOpVolumeDown, KErrNone);
			
			iController->Command(ERemConCoreApiVolumeDown);
			break;
			}
			
		case KTavsrcOpBackwards:
			{
			iOperations->EndOperation(KTavsrcOpBackwards, KErrNone);

			iController->Command(ERemConCoreApiBackward);
			break;
			}
			
		case KTavsrcOpForwards:
			{
			iOperations->EndOperation(KTavsrcOpForwards, KErrNone);

			iController->Command(ERemConCoreApiForward);
			break;
			}
			
		case KTavsrcOpPlay:
			{
			iOperations->EndOperation(KTavsrcOpPlay, KErrNone);

			iController->Command(ERemConCoreApiPlay);
			break;
			}
			
		case KTavsrcOpStop:
			{
			iOperations->EndOperation(KTavsrcOpStop, KErrNone);

			iController->Command(ERemConCoreApiStop);
			break;
			}
			
		case KTavsrcOpExit:
			{
			iOperations->EndOperation(KTavsrcOpExit, KErrNone);

			Stop();
			return;
			}

		case KTavsrcOpToggleSafeMode:
			{
			iOperations->EndOperation(KTavsrcOpToggleSafeMode, KErrNone);
			
			iOperations->SetSafeMode(!iOperations->SafeMode());
			break;
			}

		default:
			iActiveConsole->Console().Printf(_L("Unknown command\r\n"));
			}
		}
	else
		{
		__LOG(_L("Operation not allowed: %d\r\n"), beginOperation);		
		}
	
	TestMenu();

	}

TInt CAVTestApp::RegisterSEP()
	{
	TInt err;
	
	TAvdtpSEPInfo info;
	info.SetIsSink(TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(), _L("Sink ")));
	info.SetMediaType(TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(), _L("Audio ")) ?
									EAvdtpMediaTypeAudio :
									EAvdtpMediaTypeVideo);
	err = iGavdp.RegisterSEP(info);
		
	if (err==KErrNone)
		{
		iLocallyRegisteredSEPs.Append(info);
		}
	__LOG(_L("Registering SEP [SEID %d] - completed with error %d\n"), info.SEID().SEID(), err);

	if (err==KErrNone)
		{
		iCurrentLocalSEID = info.SEID();

		// add some caps, not to all though for testing!
		err = iGavdp.BeginConfiguringLocalSEP(info.SEID());
		
		__LOG(_L("Begin config Local SEP [SEID %d] - completed with error %d\n"), info.SEID().SEID(), err);
		
		TAvdtpMediaTransportCapabilities media;
		err = iGavdp.AddSEPCapability(media);
		__LOG(_L("Add configuration [Category %d] - completed with error %d\n"), media.Category(), err);
/*
		TAvdtpReportingCapabilities rep;
		err = iGavdp.AddSEPCapability(rep);
		__LOG(_L("Add configuration [Category %d] - completed with error %d\n"), rep.Category(), err);

		TAvdtpRecoveryCapabilities rec;
		rec.SetRecoveryType(ERFC2733Recovery);
		rec.SetMaxWindowSize(5);
		rec.SetMinWindowSize(1);
		err = iGavdp.AddSEPCapability(rec);
		__LOG(_L("Add configuration [Category %d] - completed with error %d\n"), rec.Category(), err);

		TAvdtpContentProtectionCapabilities cp;
		cp.SetContentProtectionType(0x1234);
		cp.SetContentProtectionData(_L8("A test content protection method"));
		err = iGavdp.AddSEPCapability(cp);
	*/	
		if(TTavsrcUtils::GetYNFromUser(iActiveConsole->Console(), _L("SBC ")))
			{
			TSBCCodecCapabilities sbc;
			sbc.SetSamplingFrequencies(E48kHz|E44100Hz|E32kHz|E16kHz);
			sbc.SetBlockLengths(EBlockLenFour | EBlockLenEight | EBlockLenTwelve | EBlockLenSixteen);
			sbc.SetMinBitpoolValue(2);
			sbc.SetMaxBitpoolValue(250);
			sbc.SetChannelModes(EJointStereo | EStereo | EMono | EDualChannel);
			sbc.SetSubbands(EFourSubbands|EEightSubbands);
			sbc.SetAllocationMethods(ELoudness | ESNR);
			
			// crazy stuff testing
			/*
			sbc.SetSamplingFrequencies(E16kHz);
			sbc.SetBlockLengths(EBlockLenTwelve);
			sbc.SetChannelModes(EJointStereo);
			sbc.SetSubbands(EFourSubbands);
			*/		
			err = iGavdp.AddSEPCapability(sbc);
			
			__LOG(_L("Add configuration [Category %d] - completed with error %d\n"), sbc.Category(), err);
			}
		else
			{
			TNonSBCCodecCapabilities codec(SymbianBluetoothAV::EAvdtpMediaTypeAudio, SymbianBluetoothAV::EAudioCodecMPEG24AAC);
			TBuf8<18> mpeg2aacData;
			mpeg2aacData.Append(0x80);		// MPEG2 AAC LC
			mpeg2aacData.Append(0x01);		// 44.1 kHz		
			mpeg2aacData.Append(0x80);		// 48.0 kHz, Channels 1 & 2
			mpeg2aacData.Append(0x80);		// VBR, unknown bitrate
			mpeg2aacData.Append(0x00);		// unknown bitrate
			mpeg2aacData.Append(0x00);		// unknown bitrate
	
			codec.SetCodecData(mpeg2aacData);
			
			err = iGavdp.AddSEPCapability(codec);
			
			__LOG(_L("Add configuration [Category %d] - completed with error %d\n"), codec.Category(), err);
			}

		iRegisteringLocalSEP = ETrue;
		iGavdp.CommitSEPConfiguration();
		
		__LOG(_L("Commit configuration [SEID %d]\n"), info.SEID().SEID());	
		}
	return err;
	}
