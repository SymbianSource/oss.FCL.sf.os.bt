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

#ifndef HCTLUSBORIGINALACLIN_H
#define HCTLUSBORIGINALACLIN_H

#include <e32base.h>
#include <bluetooth/hci/hcievents.h>
#include <d32usbdi.h>
#include <d32usbtransfers.h>

class CHCTLUsbOriginal;

NONSHARABLE_CLASS(CHCTLUsbOriginalAclIn)
	: public CActive
	{
private:
	static const TInt KEndpointNumber = 0x82;
	
public:
	static CHCTLUsbOriginalAclIn* NewL(CHCTLUsbOriginal& aHCTLUsbOriginal, RUsbInterface& aInterface);
	~CHCTLUsbOriginalAclIn();
	
	// Called to initiate the initial read on the port.
	void Start();
	
private:
	CHCTLUsbOriginalAclIn(CHCTLUsbOriginal& aHCTLUsbOriginal, RUsbInterface& aInterface);
	void ConstructL();
	
	// Helper methods
	void QueueRead();
	
private: // from CActive
	virtual void RunL();
	virtual void DoCancel();
	virtual TInt RunError(TInt aError);
	
private:
	CHCTLUsbOriginal& iHCTLUsbOriginal;
	RUsbInterface& iInterface;
	RUsbPipe iPipe;
	RUsbBulkTransferDescriptor iTransfer;
	};

#endif // HCTLUSBORIGINALACLIN_H

