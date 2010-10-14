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

#include "hctlusboriginalcommand.h"

#include <bluetooth/hci/hctlchannelobserver.h>
#include <bluetooth/logger.h>


#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_HCTL_USB_ORIGINAL);
#endif

CHCTLUsbOriginalCommand::CHCTLUsbOriginalCommand(RUsbInterface& aInterface)
	: CActive(EPriorityStandard)
	, iInterface(aInterface)
	{
	LOG_FUNC
	CActiveScheduler::Add(this);
	
	// Initialise Ep0 request details.
	iEp0Details.iRequestType	= 0x20;		// Class request
	iEp0Details.iRequest		= 0x00;		// All HCI commands are req 0x00.
	iEp0Details.iValue			= 0x0000;	// N/a for HCI
	iEp0Details.iIndex			= 0x0000;	// N/a for HCI
	iEp0Details.iFlags			= 0x04;		// Short transfer OK
	}


CHCTLUsbOriginalCommand::~CHCTLUsbOriginalCommand()
	{
	LOG_FUNC
	Cancel();
	}

CHCTLUsbOriginalCommand* CHCTLUsbOriginalCommand::NewL(RUsbInterface& aInterface)
	{
	LOG_STATIC_FUNC
	CHCTLUsbOriginalCommand* self = new(ELeave)CHCTLUsbOriginalCommand(aInterface);
	return self;
	}

TInt CHCTLUsbOriginalCommand::Write(const TDesC8& aData)
	{
	LOG_FUNC
	TInt rerr = KErrNone;
	//Check whether we finished with the previous write
	if(!IsActive())
		{
		// We want to send Ep0 commands in a serial fashion (we only use one active object).
		iChannelObserver->MhcoChannelClosed(KHCITransportCommandChannel);
		
		iRecvData.SetLength(0); // We don't receive any data via Ep0 (events come in on a separate pipe).
		
		SetActive();
		iInterface.Ep0Transfer(iEp0Details, aData, iRecvData, iStatus);
		}
	else
		{
		LOG(_L8("ERROR: Sender is already active!!\r\n"));
		rerr = KErrInUse;
		}
	
	return rerr;
	}

void CHCTLUsbOriginalCommand::SetChannelObserver(MHCTLChannelObserver& aObserver)
	{
	LOG_FUNC
	iChannelObserver = &aObserver;
	}

void CHCTLUsbOriginalCommand::RunL()
	{
	LOG_FUNC
	LOG1(_L8("\tiStatus = %d"), iStatus.Int());
	iChannelObserver->MhcoChannelOpen(KHCITransportCommandChannel);
	}

void CHCTLUsbOriginalCommand::DoCancel()
	{
	LOG_FUNC
	// TODO can't do this yet, there is no API available for this...
	}
