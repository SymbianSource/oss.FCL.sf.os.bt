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

#ifndef HOSTCONTROLLERMANAGER_H
#define HOSTCONTROLLERMANAGER_H

#include <e32base.h>
#include <bluetooth/hci/hctlpowerinterface.h>

class MControllerStateObserver;
class CHCTLUsbOriginal;

/**
This is the class that implements a controller manager.
This is a class which provides a framework for managing the
power management and reset of the host controller.
*/
NONSHARABLE_CLASS(CControllerManager)
	: public CBase
	, public MHCTLPowerInterface
	{
public:
	static CControllerManager* NewL(CHCTLUsbOriginal& aHctl);
	~CControllerManager();
	
	void SetControllerStateObserver(MControllerStateObserver& aControllerStateObserver);
	void HardReset();
	
private:
	CControllerManager(CHCTLUsbOriginal& aHctl);
	
private: // from MHCTLPowerInterface
	virtual TInt MhpiGetPower(TBTPowerState& aState);
	virtual TInt MhpiSetPower(const TBTPowerState aState);
	
private:
	MControllerStateObserver* iControllerStateObserver;
	CHCTLUsbOriginal& iHctl;
	};

#endif // HOSTCONTROLLERMANAGER_H
