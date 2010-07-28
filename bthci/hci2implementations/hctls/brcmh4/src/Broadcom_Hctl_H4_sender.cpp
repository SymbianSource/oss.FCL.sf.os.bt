// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "Broadcom_Hctl_H4_sender.h"

#include <d32comm.h>
#include <bluetooth/hci/hctlchannelobserver.h>
#include <bluetooth/logger.h>


#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_HCTL_BCM_H4);
#endif

CHCTLBcmH4Sender::CHCTLBcmH4Sender(RBusDevComm& aPort)
  : CActive(EPriorityStandard), 
	iPort(aPort)
    {
	LOG_FUNC
	CActiveScheduler::Add(this);
    }


CHCTLBcmH4Sender::~CHCTLBcmH4Sender()
    {
	LOG_FUNC

	Cancel();
    }

CHCTLBcmH4Sender* CHCTLBcmH4Sender::NewL(RBusDevComm& aPort)
    {
	LOG_STATIC_FUNC

	CHCTLBcmH4Sender* self = new(ELeave)CHCTLBcmH4Sender(aPort);
    return self;
    }

TInt CHCTLBcmH4Sender::Write(const TDesC8& aData)
    {
	LOG_FUNC
	TInt rerr = KErrNone;
	//Check whether we finished with the previous write
    if(!IsActive())
        {
		// The UART can't write anymore data until this write is complete.
		// Close all channels.
		iChannelObserver->MhcoChannelClosed(KHCITransportAllChannels);	

	    SetActive();
	    iPort.Write(iStatus, aData);
        }
	else
		{
        LOG(_L8("ERROR: Sender is already active!!\r\n"));
        rerr = KErrInUse;
		}

    return rerr;
    }

void CHCTLBcmH4Sender::SetChannelObserver(MHCTLChannelObserver& aObserver)
 	{
	LOG_FUNC

 	iChannelObserver = &aObserver;
	
 	if(!IsActive())
 		{
 		// Asyncronously give the channel observer an initial kick
		SetActive();
		TRequestStatus* status = &iStatus;
		User::RequestComplete(status, KErrNone);
 		}
 	}

void CHCTLBcmH4Sender::RunL()
    {
	LOG_FUNC
	LOG1(_L8("\tiStatus = %d"), iStatus.Int());
	iChannelObserver->MhcoChannelOpen(KHCITransportAllChannels);
	}

void CHCTLBcmH4Sender::DoCancel()
	{
	LOG_FUNC

	iPort.WriteCancel();
	}
