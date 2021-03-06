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

#include "controllermanager.h"

#include <bluetooth/hci/controllerstateobserver.h>
#include <bluetooth/hci/hctlchannelobserver.h>

#include "hctlusboriginal.h"
#include "hctlusboriginalutils.h"

#include <bluetooth/logger.h>

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_HCTL_USB_ORIGINAL);
#endif


CControllerManager* CControllerManager::NewL(CHCTLUsbOriginal& aHctl)
	{
	LOG_STATIC_FUNC
	CControllerManager* self = new(ELeave) CControllerManager(aHctl);
	return self;
	}

CControllerManager::CControllerManager(CHCTLUsbOriginal& aHctl)
	: iHctl(aHctl)
	{
	LOG_FUNC
	}

CControllerManager::~CControllerManager()
	{
	LOG_FUNC
	}

TInt CControllerManager::MhpiGetPower(TBTPowerState& aState)
	{
	LOG_FUNC
	
	__ASSERT_DEBUG(iHctl.CurrentPowerState() == EBTOff || iHctl.DevicePresent(),
		PANIC(KUsbOriginalPanic, EStackOnWhenNoDeviceConnected));
	
	// Return current state.
	aState = iHctl.CurrentPowerState();
	return KErrNone;
	}

TInt CControllerManager::MhpiSetPower(TBTPowerState aState)
	{
	LOG_FUNC
	
	TInt rerr = KErrAlreadyExists;
	// Check that the requested state differs from the current state.
	if(aState != iHctl.CurrentPowerState())
		{
		__ASSERT_ALWAYS(aState == EBTOff || aState == EBTOn,
			PANIC(KUsbOriginalPanic, EUnexpectedCtrlMgrPowerState));
		__ASSERT_ALWAYS(iControllerStateObserver,
			PANIC(KUsbOriginalPanic, EStateObserverNotAvailable));
		
		rerr = KErrNotReady;
		if(aState == EBTOff || iHctl.DevicePresent())
			{
			iControllerStateObserver->McsoProcessPowerChange(KErrNone,
															 MControllerStateObserver::EBTFatalChange,
															 aState);
			
			if(aState == EBTOff)
				{
				// Cancel any reads / writes.
				iHctl.HandlePowerOff();
				}
			else
				{
				// Re-start the sender and receiver.
				iHctl.HandlePowerOn();
				}
			
			rerr = KErrNone;
			}
		}
	
	return rerr;
	}

void CControllerManager::HardReset()
	{
	LOG_FUNC
	
	// Check if the power is currently switched off.  This takes priority over
	// hard reset.
	if(iHctl.CurrentPowerState() == EBTOn)
		{
		__ASSERT_ALWAYS(iControllerStateObserver, 
			PANIC(KUsbOriginalPanic, EStateObserverNotAvailable));
		__ASSERT_DEBUG(iHctl.DevicePresent(),
			PANIC(KUsbOriginalPanic, EStackOnWhenNoDeviceConnected));
		
		// Switch the power off then on to perform the reset.
		iControllerStateObserver->McsoProcessHardResetPhaseChange(KErrNone, MControllerStateObserver::EBTFatalChange, EBTResetStarted);
		
		iHctl.HandlePowerOff();
		// TODO should there be an asynchronous break here, or are we good?
		iHctl.HandlePowerOn();
		
		// Reset is complete.
		iControllerStateObserver->McsoProcessHardResetPhaseChange(KErrNone, MControllerStateObserver::EBTFatalChange, EBTResetComplete);
		}
	}

void CControllerManager::SetControllerStateObserver(MControllerStateObserver& aControllerStateObserver)
	{
	LOG_FUNC
	iControllerStateObserver = &aControllerStateObserver;
	}
