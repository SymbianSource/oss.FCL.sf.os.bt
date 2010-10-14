// Copyright (c) 2007-2010 Nokia Corporation and/or its subsidiary(-ies).
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

/** 
@file
@internalComponent
*/

#ifndef HCTLUSBORIGINAL_H
#define HCTLUSBORIGINAL_H

#include <bluetooth/hci/hctlbase.h>
#include <bluetooth/hciframelogger.h>
#include <bluetooth/hci/hcitypes.h>
#include <bluetooth/hci/hctlinterface.h>
#include <bluetooth/hardresetinitiator.h>
#include <d32usbdi.h>
#include <fdchctloriginalcli.h>


class CHCTLUsbOriginalCommand;
class CHCTLUsbOriginalAclOut;
class CHCTLUsbOriginalAclIn;
class CHCTLUsbOriginalEvent;
class CControllerManager;
class CDeviceStateManager;
class CHCTLUsbOriginalServer;
class MHCTLChannelObserver;
class MHCTLDataObserver;
class MHCTLEventObserver;
class MControllerStateObserver;
class MQdpPluginInterfaceFinder;


/**USB Original HCI Framing constants for command frames */
static const TInt KHCTLCommandHeaderSize	= 0;
static const TInt KHCTLCommandTrailerSize	= 0;

/**USB Original HCI Framing constants for ACL data frames */
static const TInt KHCTLAclDataHeaderSize	= 0;
static const TInt KHCTLAclDataTrailerSize	= 0;

/**USB Original HCI Framing constants for Synchronous data frames */
static const TInt KHCTLSynchronousDataHeaderSize	= 0;
static const TInt KHCTLSynchronousDataTrailerSize	= 0;

/**
This is the class that implements the USB specific HCTL.
*/
NONSHARABLE_CLASS(CHCTLUsbOriginal)
	: public CHCTLBase
	, public MHCTLInterface
	, public MHardResetInitiator
	{
public:
	static CHCTLUsbOriginal* NewL();
	~CHCTLUsbOriginal();
	
	void ProcessACLData(const TDesC8& aData);
	void ProcessSynchronousData(const TDesC8& aData);
	void ProcessEvent(const TDesC8& aEvent);
	
	MQdpPluginInterfaceFinder* QdpPluginInterfaceFinder();

	// Called from the Controller Manager
	void HandlePowerOff();
	void HandlePowerOn();
	TBTPowerState CurrentPowerState() const;

	// Called from Server Session.
	void DeviceAttachedL(TUint32 aStandardIf, TUint32 aScoIf);
	void DeviceRemoved();
	
	TBool DevicePresent() const;

private:
	CHCTLUsbOriginal();
	void ConstructL();
	TAny* Interface(TUid aUid);

	static TInt AsyncCallBackSecondStage(TAny* aUsbOriginal);
	static void Rollback(TAny* aPtr);

	void HandleChannelStateChange();
	TInt SetPower(TBTPowerState aState);

private: // from MHCTLInterface
	virtual TInt MhiWriteCommand(const TDesC8& aData);
	virtual TInt MhiWriteAclData(const TDesC8& aData);
	virtual TInt MhiWriteSynchronousData(const TDesC8& aData);
	virtual void MhiGetAclDataTransportOverhead(TUint& aHeaderSize, TUint& aTrailerSize) const;
	virtual void MhiGetSynchronousDataTransportOverhead(TUint& aHeaderSize, TUint& aTrailerSize) const;
	virtual void MhiGetCommandTransportOverhead(TUint& aHeaderSize, TUint& aTrailerSize) const;
	virtual void MhiSetChannelObserver(MHCTLChannelObserver& aChannelObserver);
	virtual void MhiSetDataObserver(MHCTLDataObserver& aDataObserver);
	virtual void MhiSetEventObserver(MHCTLEventObserver& aEventObserver);
	virtual void MhiSetControllerStateObserver(MControllerStateObserver& aControllerStateObserver);
	virtual void MhiSetQdpPluginInterfaceFinder(MQdpPluginInterfaceFinder& aQdpPluginInterfaceFinder);
	
public: // from MHardResetInitiator
	virtual void MhriStartHardReset();
	
private:
	RUsbInterface iInterface;
	RUsbInterface iScoInterface;
	
	// Senders
	CHCTLUsbOriginalCommand* iControl;
	CHCTLUsbOriginalAclOut* iAclOut;
	// Receivers
	CHCTLUsbOriginalAclIn* iAclIn;
	CHCTLUsbOriginalEvent* iEvent;
	
	MQdpPluginInterfaceFinder* iQdpPluginInterfaceFinder;
	MHCTLChannelObserver* iChannelObserver;
	MHCTLEventObserver* iEventObserver;
	MHCTLDataObserver* iDataObserver;
	MControllerStateObserver* iControllerStateObserver;
	
	TBTPowerState iCurrentPowerState;
	
	CControllerManager* iControllerMan;
	CDeviceStateManager* iDeviceStateMan;
	CHCTLUsbOriginalServer* iServer;
	
	CAsyncCallBack* iSecondStageCallBack;
	
	RFdcHctlOriginal iFdc;
	
	DECLARE_HCI_LOGGER
	};


#endif // HCTLUSBORIGINAL_H

