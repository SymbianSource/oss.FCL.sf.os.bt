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

#ifndef HCTLUSBORIGINALACLOUT_H
#define HCTLUSBORIGINALACLOUT_H

#include <e32base.h>
#include <d32usbdi.h>
#include <d32usbtransfers.h>

class MHCTLChannelObserver;

NONSHARABLE_CLASS(CHCTLUsbOriginalAclOut)
	: public CActive
	{
private:
	static const TInt KEndpointNumber = 0x02;
	
public:
	static CHCTLUsbOriginalAclOut* NewL(RUsbInterface& aInterface);
	~CHCTLUsbOriginalAclOut();
	
	TInt Write(const TDesC8& aData);
	void SetChannelObserver(MHCTLChannelObserver& aObserver);
	
private: // from CActive
	virtual void RunL();
	virtual void DoCancel();
	
	CHCTLUsbOriginalAclOut(RUsbInterface& aInterface);
	void ConstructL();
	
private:
	MHCTLChannelObserver* iChannelObserver;
	RUsbInterface& iInterface;
	RUsbPipe iPipe;
	RUsbBulkTransferDescriptor iTransfer;
	};

#endif // HCTLUSBORIGINALACLOUT_H