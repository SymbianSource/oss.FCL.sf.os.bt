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

#include "hctlusboriginalaclout.h"

#include <bluetooth/hci/hctlchannelobserver.h>
#include <bluetooth/hci/hciframe.h>

#include <bluetooth/logger.h>

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_HCTL_USB_ORIGINAL);
#endif

CHCTLUsbOriginalAclOut::CHCTLUsbOriginalAclOut(RUsbInterface& aInterface)
	: CActive(EPriorityStandard)
	, iInterface(aInterface)
	, iTransfer(CHctlAclDataFrame::KHCTLMaxACLDataSize)
	{
	LOG_FUNC
	CActiveScheduler::Add(this);
	}

void CHCTLUsbOriginalAclOut::ConstructL()
	{
	LOG_FUNC
	LEAVEIFERRORL(iInterface.RegisterTransferDescriptor(iTransfer));
	LEAVEIFERRORL(iInterface.OpenPipeForEndpoint(iPipe, KEndpointNumber, EFalse));
	}

CHCTLUsbOriginalAclOut::~CHCTLUsbOriginalAclOut()
	{
	LOG_FUNC
	Cancel();
	iPipe.Close();
	iTransfer.Close();
	}

CHCTLUsbOriginalAclOut* CHCTLUsbOriginalAclOut::NewL(RUsbInterface& aInterface)
	{
	LOG_STATIC_FUNC
	CHCTLUsbOriginalAclOut* self = new(ELeave) CHCTLUsbOriginalAclOut(aInterface);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

TInt CHCTLUsbOriginalAclOut::Write(const TDesC8& aData)
	{
	LOG_FUNC
	TInt rerr = KErrNone;
	//Check whether we finished with the previous write
	if(!IsActive())
		{
		// Currently we only support sending one transfer at a time, so close off the channel after a write
		iChannelObserver->MhcoChannelClosed(KHCITransportACLDataChannel);
		
		iTransfer.WritableBuffer() = aData;
		iTransfer.SaveData(aData.Length());
		
		SetActive();
		iPipe.Transfer(iTransfer, iStatus);
		}
	else
		{
		LOG(_L8("ERROR: Sender is already active!!\r\n"));
		rerr = KErrInUse;
		}

	return rerr;
	}

void CHCTLUsbOriginalAclOut::SetChannelObserver(MHCTLChannelObserver& aObserver)
	{
	LOG_FUNC
	iChannelObserver = &aObserver;
	}

void CHCTLUsbOriginalAclOut::RunL()
	{
	LOG_FUNC
	LOG1(_L8("\tiStatus = %d"), iStatus.Int());
	iChannelObserver->MhcoChannelOpen(KHCITransportACLDataChannel);
	}

void CHCTLUsbOriginalAclOut::DoCancel()
	{
	LOG_FUNC
	iPipe.CancelAllTransfers();
	}
