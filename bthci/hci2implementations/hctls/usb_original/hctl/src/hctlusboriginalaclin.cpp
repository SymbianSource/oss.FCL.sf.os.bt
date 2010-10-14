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

#include "hctlusboriginalaclin.h"

#include "hctlusboriginal.h"
#include "hctlusboriginalutils.h"

// These files are included to get HCI specification defined constants.
#include <bluetooth/hci/hciframe.h>
#include <bluetooth/hci/event.h>

#include <bluetooth/logger.h>

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_HCTL_USB_ORIGINAL);
#endif


CHCTLUsbOriginalAclIn::CHCTLUsbOriginalAclIn(CHCTLUsbOriginal& aHCTLUsbOriginal, RUsbInterface& aInterface)
	: CActive(EPriorityStandard)
	, iHCTLUsbOriginal(aHCTLUsbOriginal)
	, iInterface(aInterface)
	, iTransfer(CHctlAclDataFrame::KHCTLMaxACLDataSize)
	{
	LOG_FUNC
	CActiveScheduler::Add(this);
	}

CHCTLUsbOriginalAclIn::~CHCTLUsbOriginalAclIn()
	{
	LOG_FUNC
	Cancel();
	iPipe.Close();
	iTransfer.Close();
	}

CHCTLUsbOriginalAclIn* CHCTLUsbOriginalAclIn::NewL(CHCTLUsbOriginal& aHCTLUsbOriginal, RUsbInterface& aInterface)
	{
	LOG_STATIC_FUNC
	CHCTLUsbOriginalAclIn* self = new(ELeave) CHCTLUsbOriginalAclIn(aHCTLUsbOriginal, aInterface);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

void CHCTLUsbOriginalAclIn::ConstructL()
	{
	LOG_FUNC
	LEAVEIFERRORL(iInterface.RegisterTransferDescriptor(iTransfer));
	LEAVEIFERRORL(iInterface.OpenPipeForEndpoint(iPipe, KEndpointNumber, EFalse));
	}

void CHCTLUsbOriginalAclIn::QueueRead()
	{
	LOG_FUNC
	
	__ASSERT_DEBUG(!IsActive(), PANIC(KUsbOriginalPanic, EReadAttemptWhenReadOutstanding));
	
	// I think the spec states that a single ACL packet is contained in a single USB
	// transaction.  As such we don't need to mess around working out how much to read
	// we just need to issue a transfer big enough to get all the data possible.
	// Of course it is unlikely that a controller would batch up that much data - it might be
	// better to just expect a 3 DH-5 payload size instead...
	
	iTransfer.SaveData(iTransfer.WritableBuffer().MaxLength());
	
	SetActive();
	iPipe.Transfer(iTransfer, iStatus);
	}

void CHCTLUsbOriginalAclIn::RunL()
	{
	LOG_FUNC
	// Only process the read if it has completed successfully.
	LEAVEIFERRORL(iStatus.Int());
	iHCTLUsbOriginal.ProcessACLData(iTransfer.Buffer());
	QueueRead();
	}

void CHCTLUsbOriginalAclIn::DoCancel()
	{
	LOG_FUNC
	iPipe.CancelAllTransfers();
	}

TInt CHCTLUsbOriginalAclIn::RunError(TInt aError)
	{
	LOG_FUNC
	LOG1(_L8("\taError = %d"), aError);
	// The HCTL can not recover from this.  Reset the controller and restart the host.
	// TODO we should try to handle the case where the device is unpluged.
	iHCTLUsbOriginal.MhriStartHardReset();
	return KErrNone;
	}

void CHCTLUsbOriginalAclIn::Start()
	{
	LOG_FUNC
	__ASSERT_DEBUG(!IsActive(), PANIC(KUsbOriginalPanic, EStartCalledWhenReadOutstanding));
	QueueRead();
	}
