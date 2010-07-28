// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef BROADCOMHCTLH4_H
#define BROADCOMHCTLH4_H

#include <bluetooth/hci/hctluartbase.h>
#include <bluetooth/hciframelogger.h>
#include <bluetooth/hci/hcitypes.h>

#include <bluetooth/hci/hctlbcmconfiginterface.h>


class CHCTLBcmH4Receiver;
class CHCTLBcmH4Sender;
class CControllerManager;

// Define HCTL Packet Header Size
static const TInt KHctlHeaderSize = 1;
static const TInt KHctlPacketTypeOffset = 0;

// UART HCI Framing constants for command frames
static const TInt KHCTLCommandHeaderSize = 1;
static const TInt KHCTLCommandTrailerSize = 0;

// UART HCI Framing constants for ACL data frames
static const TInt KHCTLAclDataHeaderSize = 1;
static const TInt KHCTLAclDataTrailerSize = 0;

// UART HCI Framing constants for Synchronous data frames
static const TInt KHCTLSynchronousDataHeaderSize = 1;
static const TInt KHCTLSynchronousDataTrailerSize = 0;

_LIT(KIniFileName, "hctl_broadcom");

/**
This is the class that implements the UART specific HCTL.	
*/
NONSHARABLE_CLASS(CHCTLBcmH4) : public CHCTLUartBase,
								public MHctlBcmConfigInterface
	{
public:
	static CHCTLBcmH4* NewL();
	~CHCTLBcmH4();

	void DoConfigL();
	virtual void ProcessACLData(const TDesC8& aData);
	virtual void ProcessSynchronousData(const TDesC8& aData);
	virtual void ProcessEvent(const TDesC8& aEvent);
	
	MQdpPluginInterfaceFinder* QdpPluginInterfaceFinder();

	// From MHardResetInitiator
	virtual void MhriStartHardReset();

	// Called from the Controller Manager
	void HandlePowerOff();
	void HandlePowerOn();

	TBTPowerState CurrentPowerState() const;
	//CHRIS MODIF
	//void SetInitFlag(TBool flag);
	
public:
	// HCTL packet types as defined in Section 2, Part H: 4 of the
	// bluetooth specification.
	enum THctlPacketType 
	    {
		ECommandPacket			= 0x01,
		EACLDataPacket			= 0x02,
		ESynchronousDataPacket	= 0x03,
		EEventPacket			= 0x04,
	    };

private:
	CHCTLBcmH4();
	void ConstructL();
	TAny* Interface(TUid aUid);

	static void SetPacketIndicator(THctlPacketType aType, const TDesC8& aData);

	// From CHCTLUartBase
	virtual void PortOpenedL();
	
	// From MHCTLInterface
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

	// From MHCTLBcmConfigInterface
	virtual TInt MhciUpdateBaudRate(TUint32 aBaudRate);
	
	virtual void MhciSetInitPluginState(TInitState InitState);
private:
	CHCTLBcmH4Receiver* iReceiver;
	CHCTLBcmH4Sender* iSender;   
	MQdpPluginInterfaceFinder* iQdpPluginInterfaceFinder;
	TBTPowerState iCurrentPowerState;

	CControllerManager* iControllerMan;
	//CHRIS MODIF
	//TBool iInitFlag;
public:
	TInitState iInitpluginState;
	DECLARE_HCI_LOGGER
	};


#endif // BROADCOMHCTLH4_H

