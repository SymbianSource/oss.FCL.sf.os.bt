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
// This file was generated automatically from the template eventheader.tmpl
// on Wed, 05 Dec 2007 14:54:30 (time stamp)
// 
//

/**
 @file
 @publishedPartner
 @released
*/

#ifndef IOCAPABILITYREQUESTEVENT_H
#define IOCAPABILITYREQUESTEVENT_H

#include <bluetooth/hci/event.h>

/**
This class represents the IOCapabilityRequest HCI Event
*/
class TIOCapabilityRequestEvent : public THCIEventBase
	{
public:
	// Construct an event object to wrap existing event data received from the Controller
	IMPORT_C TIOCapabilityRequestEvent(const TDesC8& aEventData);
	// Construct a faked event, storing the supplied event parameters into the supplied empty event data buffer.
	IMPORT_C TIOCapabilityRequestEvent(const TBTDevAddr& aBDADDR, TDes8& aEventData);
	// Destructor
	IMPORT_C ~TIOCapabilityRequestEvent();

	// The static Cast method is used to obtain a pointer to the derived class object
	IMPORT_C static TIOCapabilityRequestEvent& Cast(const THCIEventBase& aEvent);

	// Accessor methods for the parameters of the event
	IMPORT_C TBTDevAddr BDADDR() const;
	
	};
	
#endif // IOCAPABILITYREQUESTEVENT_H
