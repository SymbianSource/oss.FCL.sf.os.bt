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

#include "devicestatemanager.h"

#include <d32usbdi.h>

#include <bluetooth/logger.h>

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_HCTL_USB_ORIGINAL);
#endif

CDeviceStateManager* CDeviceStateManager::NewL(RUsbInterface& aAclInterface, RUsbInterface& aScoInterface)
	{
	LOG_STATIC_FUNC
	CDeviceStateManager* self = new(ELeave) CDeviceStateManager;
	CleanupStack::PushL(self);
	self->ConstructL(aAclInterface, aScoInterface);
	CleanupStack::Pop(self);
	return self;
	}

CDeviceStateManager::~CDeviceStateManager()
	{
	LOG_FUNC
	delete iScoManager;
	delete iAclManager;
	}

void CDeviceStateManager::Suspend()
	{
	LOG_FUNC
	iAclManager->Suspend();
	iScoManager->Suspend();
	}

void CDeviceStateManager::Resume()
	{
	LOG_FUNC
	iAclManager->Resume();
	iScoManager->Resume();
	}

CDeviceStateManager::CDeviceStateManager()
	{
	LOG_FUNC
	}

void CDeviceStateManager::ConstructL(RUsbInterface& aAclInterface, RUsbInterface& aScoInterface)
	{
	LOG_FUNC
	iAclManager = CInterfaceStateManager::NewL(aAclInterface);
	iScoManager = CInterfaceStateManager::NewL(aScoInterface);
	}
	


CInterfaceStateManager* CInterfaceStateManager::NewL(RUsbInterface& aInterface)
	{
	LOG_STATIC_FUNC
	return new(ELeave) CInterfaceStateManager(aInterface);
	}

CInterfaceStateManager::~CInterfaceStateManager()
	{
	LOG_FUNC
	Cancel();
	}

CInterfaceStateManager::CInterfaceStateManager(RUsbInterface& aInterface)
	: CActive(EPriorityStandard)
	, iInterface(aInterface)
	{
	LOG_FUNC
	CActiveScheduler::Add(this);
	}

void CInterfaceStateManager::DoCancel()
	{
	LOG_FUNC
	iInterface.CancelPermitSuspend();
	}

void CInterfaceStateManager::RunL()
	{
	LOG_FUNC
	if(iStatus.Int() == KErrNone)
		{
		RequestSuspend();
		}
	// Ignore error - we'll try again on the next explict resume
	}

void CInterfaceStateManager::RequestSuspend()
	{
	iInterface.PermitSuspendAndWaitForResume(iStatus);
	SetActive();
	}

void CInterfaceStateManager::Suspend()
	{
	LOG_FUNC
	if(!IsActive()) // Only suspend if we aren't already suspended.
		{
		RequestSuspend();
		}
	}

void CInterfaceStateManager::Resume()
	{
	LOG_FUNC
	Cancel();
	}


