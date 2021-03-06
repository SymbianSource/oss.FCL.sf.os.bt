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
// This file was generated automatically from the template completeeventheader.tmpl
// on Tue, 15 Jan 2008 17:26:22 (time stamp)
// 
//

/**
 @file
 @publishedPartner
 @released
*/

#ifndef READINQUIRYRESPONSETRANSMITPOWERLEVELCOMPLETEEVENT_H
#define READINQUIRYRESPONSETRANSMITPOWERLEVELCOMPLETEEVENT_H

#include <bluetooth/hci/commandcompleteevent.h>

/**
This class represents command completion event for the ReadInquiryResponseTransmitPowerLevel HCI command
*/

class TReadInquiryResponseTransmitPowerLevelCompleteEvent : public THCICommandCompleteEvent
	{
public:
	// Construct an event object to wrap existing event data received from the Controller
	IMPORT_C TReadInquiryResponseTransmitPowerLevelCompleteEvent(const TDesC8& aEventData);
	// Construct an event object to generate a faked event using the supplied parameters
	IMPORT_C TReadInquiryResponseTransmitPowerLevelCompleteEvent(THCIErrorCode aStatus, TUint8 aNumHCICommandPackets, TInt8 aTxPowerLevel, TDes8& aEventData);

	// The static Cast method is used to obtain a pointer to the derived class object
	IMPORT_C static TReadInquiryResponseTransmitPowerLevelCompleteEvent& Cast(const THCIEventBase& aEvent);

	// Accessor methods for the parameters of the event
	IMPORT_C TInt8 TxPowerLevel() const;
	
	};
	
#endif // READINQUIRYRESPONSETRANSMITPOWERLEVELCOMPLETEEVENT_H
