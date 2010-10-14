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

#include "hctlusboriginal.h"

#include "hctlusboriginalcommand.h"
#include "hctlusboriginalaclout.h"
#include "hctlusboriginalaclin.h"
#include "hctlusboriginalevent.h"
#include "controllermanager.h"
#include "devicestatemanager.h"
#include "hctlusboriginalserver.h"

#include "hctlusboriginalutils.h"

#include <bluetooth/hci/hctleventobserver.h>
#include <bluetooth/hci/hctldataobserver.h>
#include <bluetooth/hci/hctlchannelobserver.h>
#include <bluetooth/logger.h>


#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_HCTL_USB_ORIGINAL);
#endif

_LIT(KUsbdiLddFileName, "usbdi");

CHCTLUsbOriginal* CHCTLUsbOriginal::NewL()
	{
	LOG_STATIC_FUNC
	CHCTLUsbOriginal* self = new(ELeave) CHCTLUsbOriginal();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

CHCTLUsbOriginal::CHCTLUsbOriginal()
	{
	LOG_FUNC
	}

CHCTLUsbOriginal::~CHCTLUsbOriginal()
	{
	LOG_FUNC
	
	delete iControl;
	delete iAclOut;
	delete iAclIn;
	delete iEvent;
	
	iInterface.Close();
	iScoInterface.Close();
	
	iFdc.Close();
	
	delete iSecondStageCallBack;
	
	delete iServer;
	delete iControllerMan;
	HCI_LOG_UNLOAD(this);
	
	// User::FreeLogicalDevice(RUsbInterface::Name()); // TODO Maybe this affects others
	}

void CHCTLUsbOriginal::ConstructL()
	{
	LOG_FUNC
	
	LOG(_L("\tLoading USBDI LDD"));
	TInt err = User::LoadLogicalDevice(KUsbdiLddFileName);
	if(err != KErrNone && err != KErrAlreadyExists)
		{
		LEAVEIFERRORL(err);
		}
	
	HCI_LOG_LOADL(this, KHCILoggerDatalinkTypeH1); // no frame information - so use the H1 encoding.
	iControllerMan = CControllerManager::NewL(*this);
	iDeviceStateMan = CDeviceStateManager::NewL(iInterface, iScoInterface);
	iServer = CHCTLUsbOriginalServer::NewL(*this);
	
	// We need an asynchronous break from the synchronous creation path to complete the set-up.
	// This is due to the behaviour of the Bluetooth stack.
	TCallBack secondStageCallBack(AsyncCallBackSecondStage, this);
	iSecondStageCallBack = new(ELeave) CAsyncCallBack(secondStageCallBack, CActive::EPriorityStandard);
	iSecondStageCallBack->CallBack();
	}

TInt CHCTLUsbOriginal::AsyncCallBackSecondStage(TAny* aUsbOriginal)
	{
	LOG_STATIC_FUNC
	
	CHCTLUsbOriginal* usbOriginal = static_cast<CHCTLUsbOriginal*>(aUsbOriginal);
	static_cast<void>(usbOriginal->SetPower(EBTOff)); // Can't handle the error, it shouldn't affect operation.
	
	TInt err = usbOriginal->iFdc.Connect();
	LOG1(_L("\tFDC Connection result = %d"), err);
	if(err == KErrNone)
		{
		// Here we are passive on errors, a failure to connect indicates
		// that the FDC isn't loaded yet.  When it is it will attach to
		// us.
		usbOriginal->iFdc.RequestConnection(); // Causes the FDC to call back with tokens.
		}
	
	return EFalse;	// Don't call back any more.
	}



TAny* CHCTLUsbOriginal::Interface(TUid aUid)
	{
	LOG_FUNC
	
	TAny* ret = NULL;
	switch(aUid.iUid)
		{
		case KHCTLInterfaceUid:
			ret = reinterpret_cast<TAny*>(static_cast<MHCTLInterface*>(this));
			break;
		
		case KHCTLPowerInterfaceUid:
			ret = reinterpret_cast<TAny*>(static_cast<MHCTLPowerInterface*>(iControllerMan));
			break;
		
		case KHCHardResetUid:
			ret = reinterpret_cast<TAny*>(static_cast<MHardResetInitiator*>(this));   
			break;
		default:
			break;
		}
	
	return ret;
	}

//Implementation of pure virtuals from MHCTLInterface
TInt CHCTLUsbOriginal::MhiWriteAclData(const TDesC8& aData)
	{
	LOG_FUNC
	__ASSERT_DEBUG(iChannelObserver, PANIC(KUsbOriginalPanic, ENoChannelObserver));
	
	TInt rerr = KErrNotReady;
	// Send if the power is on.
	if(iCurrentPowerState == EBTOn && DevicePresent())
		{
		rerr = iAclOut->Write(aData);
		HCI_LOG_FRAME_IF_NO_ERROR(rerr, this, KHCILoggerHostToController | KHCILoggerACLDataFrame, aData);
		}
		
	return rerr;
	}

TInt CHCTLUsbOriginal::MhiWriteSynchronousData(const TDesC8& aData)
	{
	LOG_FUNC
	__ASSERT_DEBUG(iChannelObserver, PANIC(KUsbOriginalPanic, ENoChannelObserver));
	
	ASSERT(EFalse); // TODO Not supported yet
	
	TInt rerr = KErrNotReady;
	// Send if the power is on.
	if(iCurrentPowerState == EBTOn && DevicePresent())
		{
		// TODO insert code here :)
		HCI_LOG_FRAME_IF_NO_ERROR(rerr, this, KHCILoggerHostToController | KHCILoggerSynchronousDataFrame, aData);
		}
	
	return rerr;
	}

TInt CHCTLUsbOriginal::MhiWriteCommand(const TDesC8& aData)
	{
	LOG_FUNC
	__ASSERT_DEBUG(iChannelObserver, PANIC(KUsbOriginalPanic, ENoChannelObserver));
	
	TInt rerr = KErrNotReady;
	// Send if the power is on.
	if(iCurrentPowerState == EBTOn && DevicePresent())
		{
		rerr = iControl->Write(aData);
		HCI_LOG_FRAME_IF_NO_ERROR(rerr, this, KHCILoggerHostToController | KHCILoggerCommandOrEvent, aData);
		}
	
	return rerr;
	}

void CHCTLUsbOriginal::MhiSetQdpPluginInterfaceFinder(MQdpPluginInterfaceFinder& aQdpPluginInterfaceFinder)
	{
	iQdpPluginInterfaceFinder = &aQdpPluginInterfaceFinder;
	}

void CHCTLUsbOriginal::MhriStartHardReset()
	{
	LOG_FUNC
	iControllerMan->HardReset();
	}

void CHCTLUsbOriginal::MhiGetAclDataTransportOverhead(TUint& aHeaderSize, TUint& aTrailerSize) const
	{
	// Return the transport overhead for ACL data.
	aHeaderSize = KHCTLAclDataHeaderSize;
	aTrailerSize = KHCTLAclDataTrailerSize;
	}

void CHCTLUsbOriginal::MhiGetSynchronousDataTransportOverhead(TUint& aHeaderSize, TUint& aTrailerSize) const
	{
	// Return the transport overhead for Synchronous data.
	aHeaderSize = KHCTLSynchronousDataHeaderSize;
	aTrailerSize = KHCTLSynchronousDataTrailerSize;
	}

void CHCTLUsbOriginal::MhiGetCommandTransportOverhead(TUint& aHeaderSize, TUint& aTrailerSize) const
	{
	// Return the transport overhead for HCI commands data.
	aHeaderSize = KHCTLCommandHeaderSize;
	aTrailerSize = KHCTLCommandTrailerSize;
	}

/**
This function is used by the receiver for informing HCI that ACL data has been received
The receiver doesn't have reference to iDataObserver. So this is merely a wrapper for iDataObserver
*/
void CHCTLUsbOriginal::ProcessACLData(const TDesC8& aData)
	{
	HCI_LOG_FRAME(this, KHCILoggerControllerToHost | KHCILoggerACLDataFrame, aData);
	iDataObserver->MhdoProcessAclData(aData);
	}

/**
This function is used by the receiver for informing HCI that Synchronous data has been received
The receiver doesn't have reference to iDataObserver. So this is merely a wrapper for iDataObserver
*/
void CHCTLUsbOriginal::ProcessSynchronousData(const TDesC8& aData)
	{
	HCI_LOG_FRAME(this, KHCILoggerControllerToHost | KHCILoggerSynchronousDataFrame, aData);
	iDataObserver->MhdoProcessSynchronousData(aData);
	}

/**
This function is used by the receiver for informing HCI that event has been received
The receiver doesn't have reference to iEventObserver. So this is merely a wrapper for iDataObserver
*/	
void CHCTLUsbOriginal::ProcessEvent(const TDesC8& aEvent)
	{
	HCI_LOG_FRAME(this, KHCILoggerControllerToHost | KHCILoggerCommandOrEvent, aEvent);
	iEventObserver->MheoProcessEvent(aEvent);
	}

/**
QdpPluginInterfaceFinder getter.

@return returns iQdpPluginInterfaceFinder which could be NULL 
if it has not been given one.
*/
MQdpPluginInterfaceFinder* CHCTLUsbOriginal::QdpPluginInterfaceFinder()
	{
	return iQdpPluginInterfaceFinder;
	}

void CHCTLUsbOriginal::MhiSetDataObserver(MHCTLDataObserver& aDataObserver)
	{
	iDataObserver = &aDataObserver;
	}

void CHCTLUsbOriginal::MhiSetEventObserver(MHCTLEventObserver& aEventObserver)
	{
	iEventObserver = &aEventObserver;
	}

void CHCTLUsbOriginal::MhiSetChannelObserver(MHCTLChannelObserver& aChannelObserver)
	{
	iChannelObserver = &aChannelObserver;
	if(DevicePresent())
		{
		iControl->SetChannelObserver(aChannelObserver);
		iAclOut->SetChannelObserver(aChannelObserver);
		}
	HandleChannelStateChange();
	}

void CHCTLUsbOriginal::MhiSetControllerStateObserver(MControllerStateObserver& aControllerStateObserver)
	{
	iControllerStateObserver = &aControllerStateObserver;
	iControllerMan->SetControllerStateObserver(aControllerStateObserver);
	}

void CHCTLUsbOriginal::HandleChannelStateChange()
	{
	if(iCurrentPowerState == EBTOn && iChannelObserver && DevicePresent())
		{
		iDeviceStateMan->Resume();
		iChannelObserver->MhcoChannelOpen(KHCITransportAllChannels);
		iAclIn->Start();
		iEvent->Start();
		// The senders will be activated when the first frame needs to be sent.
		}
	else
		{
		if(iChannelObserver)
			{
			iChannelObserver->MhcoChannelClosed(KHCITransportAllChannels);
			}
		if(DevicePresent())
			{
			// abort communication with the hardware
			iControl->Cancel();
			iAclOut->Cancel();
			iAclIn->Cancel();
			iEvent->Cancel();
			
			// try to suspend the device
			iDeviceStateMan->Suspend();
			}
		}
	}

void CHCTLUsbOriginal::HandlePowerOff()
	{
	iCurrentPowerState = EBTOff;
	HandleChannelStateChange();
	}

void CHCTLUsbOriginal::HandlePowerOn()
	{
	iCurrentPowerState = EBTOn;
	HandleChannelStateChange();
	}

TBTPowerState CHCTLUsbOriginal::CurrentPowerState() const
	{
	return iCurrentPowerState;
	}

TInt CHCTLUsbOriginal::SetPower(TBTPowerState aState)
	{
	return static_cast<MHCTLPowerInterface*>(iControllerMan)->MhpiSetPower(aState);
	}

TBool CHCTLUsbOriginal::DevicePresent() const
	{
	if(iControl)
		{
		__ASSERT_DEBUG(iAclOut && iAclIn && iEvent, PANIC(KUsbOriginalPanic, EPartiallyOpenedInterface));
		return ETrue;
		}
	return EFalse;
	}

void CHCTLUsbOriginal::DeviceAttachedL(TUint32 aStandardIf, TUint32 aScoIf)
	{
	if(iCurrentPowerState == EBTOn)
		{
		// If the stack is already powered up then we cannot handle the attach.
		// Ideally we would inform the user to disconnect and connect the device again.
		LEAVEL(KErrAlreadyExists);
		}
	
	CleanupStack::PushL(TCleanupItem(CHCTLUsbOriginal::Rollback, this));
	
	// Open interface (there should be only 1 setting).
	LEAVEIFERRORL(iInterface.Open(aStandardIf));
	TInt aclAltIfCount = iInterface.GetAlternateInterfaceCount();
	if(aclAltIfCount != 1)
		{
		LEAVEIFERRORL(aclAltIfCount); // if it's an error then leave with that
		LOG1(_L8("ACL interface has %d alternate interfaces instead of 1"), aclAltIfCount);
		LEAVEL(KErrTotalLossOfPrecision);
		}
	
	// We don't do SCO yet, so just set alt. int. setting 0
	LEAVEIFERRORL(iScoInterface.Open(aScoIf));
	LEAVEIFERRORL(iScoInterface.SelectAlternateInterface(0));
	
	// Initialise pipe AOs
	iControl = CHCTLUsbOriginalCommand::NewL(iInterface);
	iAclOut = CHCTLUsbOriginalAclOut::NewL(iInterface);
	iAclIn = CHCTLUsbOriginalAclIn::NewL(*this, iInterface);
	iEvent = CHCTLUsbOriginalEvent::NewL(*this, iInterface);
	
	// Pipe AOs should have registered their trans. descriptors so initialise.
	LEAVEIFERRORL(iInterface.InitialiseTransferDescriptors());
	
	// Finally set-up channel observers, if needed
	if(iChannelObserver)
		{
		iControl->SetChannelObserver(*iChannelObserver);
		iAclOut->SetChannelObserver(*iChannelObserver);
		}
	
	// Ensure we tell everyone what has just happened.
	TInt err = SetPower(EBTOn);
	if(err != KErrNone && err != KErrAlreadyExists)
		{
		LEAVEIFERRORL(err);
		}
	
	CleanupStack::Pop(this);
	}

void CHCTLUsbOriginal::DeviceRemoved()
	{
	// Cleanup resources.
	delete iEvent,	iEvent = NULL;
	delete iAclIn,	iAclIn = NULL;
	delete iAclOut,	iAclOut = NULL;
	delete iControl,iControl = NULL;
	
	iScoInterface.Close();
	iInterface.Close();
	
	// Ensure we tell everyone what has just happened.
	static_cast<void>(SetPower(EBTOff));
	}

void CHCTLUsbOriginal::Rollback(TAny* aPtr)
	{
	CHCTLUsbOriginal* self = reinterpret_cast<CHCTLUsbOriginal*>(aPtr);
	self->DeviceRemoved();
	}
