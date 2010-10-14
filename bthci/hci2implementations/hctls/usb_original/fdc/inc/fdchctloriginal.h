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

#ifndef FDCHCTLORIGINAL_H
#define FDCHCTLORIGINAL_H

#include <e32base.h>
#include <usbhost/internal/fdcplugin.h>
#include <usbhost/internal/fdcinterface.h>
#include <hctlusboriginalcli.h>

class CFdcHctlOriginalServer;

NONSHARABLE_CLASS(CFdcHctlOriginal)
	: public CFdcPlugin
	, public MFdcInterfaceV1
	{
public:
	static CFdcHctlOriginal* NewL(MFdcPluginObserver& aObserver);
	~CFdcHctlOriginal();
	
public:
	void RequestConnection();
	
private:
	CFdcHctlOriginal(MFdcPluginObserver& aObserver);
	void ConstructL();
	
private: // from CFdcPlugin
	virtual TAny* GetInterface(TUid aUid);
	
private: // from MFdcInterfaceV1
	virtual TInt Mfi1NewFunction(TUint aDeviceId,
		const TArray<TUint>& aInterfaces,
		const TUsbDeviceDescriptor& aDeviceDescriptor,
		const TUsbConfigurationDescriptor& aConfigurationDescriptor);
	virtual void Mfi1DeviceDetached(TUint aDeviceId);
	
private:
	CFdcHctlOriginalServer*	iServer;
	RHCTLUsbOriginal		iHctlSession;
	TBool					iReady;
	TUint32					iAclToken;
	TUint32					iScoToken;
	};

#endif // FDCHCTLORIGINAL_H
