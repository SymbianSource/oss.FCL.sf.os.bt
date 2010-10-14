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

#ifndef DEVICESTATEMANAGER_H
#define DEVICESTATEMANAGER_H

#include <e32base.h>

class RUsbInterface;

NONSHARABLE_CLASS(CInterfaceStateManager)
	: public CActive
	{
public:
	static CInterfaceStateManager* NewL(RUsbInterface& aInterface);
	~CInterfaceStateManager();
	
	void Suspend();
	void Resume();
	
private:
	CInterfaceStateManager(RUsbInterface& aInterface);
	
	void RequestSuspend();
	
private: // from CActive
	void DoCancel();
	void RunL();
	
private:
	RUsbInterface& iInterface;
	};


/**
This is the class to control the state of the USB Device.
*/
NONSHARABLE_CLASS(CDeviceStateManager) 
	: public CBase
	{
public:
	static CDeviceStateManager* NewL(RUsbInterface& aAclInterface, RUsbInterface& aScoInterface);
	~CDeviceStateManager();
	
	void Suspend();
	void Resume();
	
private:
	CDeviceStateManager();
	void ConstructL(RUsbInterface& aAclInterface, RUsbInterface& aScoInterface);
	
private:
	CInterfaceStateManager* iAclManager;
	CInterfaceStateManager* iScoManager;
	};

#endif // DEVICESTATEMANAGER_H
