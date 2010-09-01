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

#ifndef TAVSRC_H
#define TAVSRC_H

//only use define CHANNEL_BINDING_DEBUG_CHECK when checking channel binding
//#define CHANNEL_BINDING_DEBUG_CHECK

#define __PRINT iActiveConsole->Console().Printf
#define __LOG iLogConsole->Console().Printf /*RDebug::Print*/

#include "tavsrcConsole.h"
#include "tavsrcStreamerUser.h"
#include "tavsrcOperations.h"

#include <f32file.h>
#include <btsdp.h>
#include <rtp.h>
#include <gavdp.h>
#include <remconcoreapitargetobserver.h>
#include <remcongroupnavigationtarget.h>
#include <remcongroupnavigationtargetobserver.h>

#include <remconmediainformationtarget.h>
#include <remconmediainformationtargetobserver.h>

#include <remconbatterytarget.h>
#include <remconbatterytargetobserver.h>

#include <playerinformationtarget.h>
#include <playerinformationtargetobserver.h>

#include <remconabsvoltargetobserver.h>
#include <remcontrackinfotargetobserver.h>

enum TRemConInterfaces
	{
	ECoreApiTarget = 1 << 0,
	ECoreApiController = 1 << 1,
	EGroupNavigation = 1 << 2,
	EMediaInformation = 1 << 3,
	EBatteryStatus = 1 << 4,
	EAbsoluteVolumeTarget = 1 << 5,
	EAbsoluteVolumeController = 1 << 6,
	EPlayerInformation = 1 << 7,
	ENowPlaying = 1 << 8,
	EMediaBrowse = 1 << 9,
	EDatabaseAware = 1 << 10,
	ESeparateThreadForBrowse = 1 << 11,
	EVendorTrackInfoTarget = 1 << 12,
	EVendorAbsoluteVolumeTarget = 1 << 13,
	};


typedef TFixedArray<RSocket, 3> RSocketArray;

class CActivePacketDropIoctl;
class CActiveSockReader;
class CActiveSockWriter;
class CRemMetadataTransferTarget;
class CAVTestApp;
class CActiveStreamer;
class CTavsrcController;
class CRemConInterfaceSelector;	
class CRemConCoreApiTarget;
class CRemConTrackInfoTarget;
class CRemConAbsVolTarget;
class CTavsrcMtUpdater;
class CTavsrcAbsoluteVolume;
class CAVTestApp : public CBase, public MActiveConsoleNotify,
						public MGavdpUser, public MBluetoothPhysicalLinksNotifier,
						public MRemConCoreApiTargetObserver,
						public MActiveStreamerUser,
						public MRemConMediaInformationTargetObserver,
						public MRemConGroupNavigationTargetObserver,
						public MRemConBatteryTargetObserver,
						public MRemConTrackInfoTargetObserver,
						public MRemConAbsVolTargetObserver
	{
private:
	enum TTavsrcServiceCategory
		{
		ETavsrcServiceCategoryNull				=0x00,
		ETavsrcServiceCategoryMediaTransport 	=0x01,
		ETavsrcServiceCategoryReporting			=0x02,
		ETavsrcServiceCategoryRecovery			=0x04,
		ETavsrcServiceCategoryContentProtection	=0x08,
		ETavsrcServiceCategoryHeaderCompression	=0x10,
		ETavsrcServiceCategoryMultiplexing		=0x20,
		ETavsrcServiceCategoryMediaCodec		=0x40,
		};
public:
	static CAVTestApp* NewL();
	void EchoStorm();

	void StartL();
	void Stop();
	~CAVTestApp();
	void TestMenu();

	void KeyPressed(TChar aKey);

	TInt StartSrc();
	void StopSrc();
	void DisconnectSrc();

	// echo test
	virtual void HandleCreateConnectionCompleteL(TInt aErr);
	virtual void HandleDisconnectCompleteL(TInt aErr);
	virtual void HandleDisconnectAllCompleteL(TInt aErr);

private:
	void PreventLowPowerModes();
	void AllowLowPowerModes();
	void CreateBearers();
	void CloseBearers();
	void ConfigureSEPL();
	void Abort();
	void DiscoverSEPs();
	void StartStreams();
	void SuspendStreams();
	void GetCapabilities();
	void SendSecurityControl();
	TSBCCodecCapabilities InteractiveSBCMediaCodecConfig(TSBCCodecCapabilities& caps);
	
	void PrintCommandOption(TChar aOperation, TPtrC aDesc);
	
	void CreateStreamerL(RSocketArray aSockets);
	void CreateRemConInterfacesL();

	void Connect();
	TInt Listen();
	TInt RegisterSEP();
	
	CAVTestApp();
	void ConstructL();

	//return ETrue if current SEP is sink, EFalse otherwise
	TBool CurrentSEIDIsSink();

	void DisplayHelp();
	void ParseCommandLineL();
	
private:
	// GAVDP callbacks
	virtual void GAVDP_SEPDiscovered(const TAvdtpSEPInfo& aSEP);
	virtual void GAVDP_SEPDiscoveryComplete();
	virtual void GAVDP_SEPCapability(TAvdtpServiceCapability* aCapability);	
	virtual void GAVDP_SEPCapabilityComplete();
	virtual void GAVDP_AbortStreamConfirm();
	virtual void GAVDP_StartStreamsConfirm();
	virtual void GAVDP_SuspendStreamsConfirm();
	virtual void GAVDP_SecurityControlConfirm(const TDesC8& aResponseData);
	virtual void GAVDP_ConfigurationConfirm();	// configuration complete and SEP selected *AND* reconfigure confirm
	virtual void GAVDP_Error(TInt aError, const TDesC8& aErrorData);
	virtual void GAVDP_ConnectConfirm(const TBTDevAddr& aAddr);

	virtual void GAVDP_ConfigurationStartIndication(TSEID aLocalSEID, TSEID aRemoteSEID);	
	virtual TInt GAVDP_ConfigurationIndication(TAvdtpServiceCapability* aCapability);
	virtual TInt GAVDP_ConfigurationEndIndication();

	virtual TInt GAVDP_StartIndication(TSEID aSEID);
	virtual TInt GAVDP_SuspendIndication(TSEID aSEID);
	virtual TInt GAVDP_SecurityControlIndication(TSEID aSEID, TDes8& aSecurityData);
	virtual void GAVDP_AbortIndication(TSEID aSEID);
	virtual void GAVDP_ReleaseIndication(TSEID aSEID);
	virtual void GAVDP_BearerReady(RSocket aNewSocket, const TAvdtpSockAddr& aAddr);
	
	void PrettyPrint(TAvdtpServiceCapability& aCapability);
	
private:
	// RemConCoreApiTarget callbacks
	virtual void MrccatoCommand(TRemConCoreApiOperationId aOperationId, TRemConCoreApiButtonAction aButtonAct);
	virtual void MrccatoPlay(TRemConCoreApiPlaybackSpeed aSpeed, TRemConCoreApiButtonAction aButtonAct);

private:
	// from MActiveStreamerUser
	virtual void MediaCodecConfigurationRequired(TSBCCodecCapabilities& aConfig);

	virtual TInt MrcmtcGetElementAttributes(TUint64 aElement, TUint32 aAttribute, HBufC8*& aOutValueOwnershipTransferred);

	// from MRemConGroupNavigationTargetObserver	
	virtual void MrcgntoNextGroup(TRemConCoreApiButtonAction aButtonAct);
	virtual void MrcgntoPreviousGroup(TRemConCoreApiButtonAction aButtonAct);
	virtual void MrcbstoBatteryStatus(TControllerBatteryStatus& aBatteryStatus);

	// from MRemConMediaInformationTargetObserver	
	virtual void MrcmitoGetCurrentlyPlayingMetadata( TMediaAttributeIter& aAttributeIter );

	virtual void MrcncRegisterPlaybackStatusChangedNotification();
	virtual void MrcncRegisterTrackChangedNotification();
	virtual void MrcncRegisterTrackReachedEndNotification();
	virtual void MrcncRegisterTrackReachedStartNotification();
	virtual void MrcncRegisterPositionChangedNotification(TUint32 aInterval);
	virtual void MrcncRegisterBatteryStatusChangedNotification();
	virtual void MrcncRegisterSystemStatusChangedNotification();
	virtual void MrcncRegisterPlayerApplicationStatusChangedNotification();
	
	virtual void MrcavtoGetAbsoluteVolume();
	virtual void MrcavtoSetAbsoluteVolume(TUint /*aAbsVol*/, TUint /*aMaxVol*/);
	
	virtual void MrctitoGetTrackName();
	virtual void MrctitoGetArtist();
	virtual void MrctitoGetTrackDuration();

private:
	CActiveConsole* iActiveConsole;
	CActiveConsole* iLogConsole;
	CTavsrcMtUpdater*			iMtUpdater;
		
	RSocketServ iSockServ;
	
#ifdef CHANNEL_BINDING_DEBUG_CHECK
	CActiveSockReader* iRepReader;
	CActiveSockWriter* iRepWriter;
	CActiveSockReader* iRecvReader;
	CActiveSockWriter* iRecvWriter;
#endif
	
	CActivePacketDropIoctl* iPacketDropIoctl;

	RGavdp iGavdp;	
	
	TSEID iCurrentSEID; // Current Remote SEID
	TSEID iCurrentLocalSEID; // Current Local SEID

	RPointerArray<TAvdtpServiceCapability> iSEPCapabilities; // only held for one SEP at a time in this test code

	RArray<TSEID> iShortlistedSEIDs;	// for finding good remote SEP
	TBool iChosenSEP;
	
	RArray<TAvdtpSEPInfo> iLocallyRegisteredSEPs;
	TBool iRegisteringLocalSEP;
	
	CActiveStreamer* iStreamer;
	RSocketArray iPendingSockets;
	
	CRemConInterfaceSelector* iRemConInterfaceSelector;
	CRemConCoreApiTarget* iRemConTarget;
	CRemConGroupNavigationApiTarget* iGroupNavigation;
	CRemConMediaInformationTarget* iMediaInformation;
	CRemConBatteryApiTarget* iBatteryStatus;
	CRemConTrackInfoTarget* iTrackInfo;
	CRemConAbsVolTarget* iAbsoluteVolume;
	CTavsrcAbsoluteVolume* iTavsrcAbsoluteVolume;

	RSdp iSdp;
	RSdpDatabase iSdpDB;
	TSdpServRecordHandle iSrcHandle;
	TSdpServRecordHandle iSnkHandle;
	
	TBool iAutoStream;
	
	enum TGavdpState
		{
		ENoClientOpen,
		EIdle,
		ESigConnected,
		EOpen,
		ESuspended,
		ERemoteReconfiguring,
		};

	enum TStreamState
		{
		EClosed,
		EStreaming,
		EPaused,
		EStopped,
		};
		
	TGavdpState iGavdpState;
	TStreamState iStreamState;
	
	TSBCCodecCapabilities iReconfigInfo;
	TBool iLocalReconfigure;
	
	TUint iDisplayMode;
	TBool iPreloadFile;
	
	CTavsrcController* iController;
	TBTDevAddr iDevAddr;

	CTavsrcOperations* iOperations;
	
	RBuf iFilename;
	RBTPhysicalLinkAdapter iPhy;
		
	TUint iRemConInterfaces;
	};

#endif // TAVSRC_H
