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
// This file was generated automatically from the template eventsource.tmpl
// on Wed, 25 Jul 2007 17:00:42 (time stamp)
// 
//

/**
 @file
 @internalComponent
*/

#include <bluetooth/hci/synchronousconnectionchangedevent.h>
#include <bluetooth/hci/hcierrors.h>
#include <bluetooth/hci/hciopcodes.h>
#include "symbiancommandseventsutils.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_COMMANDSEVENTS_SYMBIAN);
#endif

// Construct an event object to wrap existing event data received from the Controller
EXPORT_C TSynchronousConnectionChangedEvent::TSynchronousConnectionChangedEvent(const TDesC8& aEventData)
	: THCIEventBase(aEventData)
	{
	iErrorCode = THCIErrorCode(iEventData[2]); // Status field is in octet 3
	}

// Construct a faked event, storing the supplied event parameters into the supplied empty event data buffer.
EXPORT_C TSynchronousConnectionChangedEvent::TSynchronousConnectionChangedEvent(THCIErrorCode aStatus, THCIConnectionHandle aConnectionHandle, TUint8 aTransmissionInterval, TUint8 aRetransmissionWindow, TUint16 aRxPacketLength, TUint16 aTxPacketLength, TDes8& aEventData)
	: THCIEventBase(ESynchronousConnectionChangedEvent, 9, aEventData)
	{
	iErrorCode = aStatus;
	PutTUint8(aStatus, aEventData);
	PutConnectionHandle(aConnectionHandle, aEventData);
	PutTUint8(aTransmissionInterval, aEventData);
	PutTUint8(aRetransmissionWindow, aEventData);
	PutTUint16(aRxPacketLength, aEventData);
	PutTUint16(aTxPacketLength, aEventData);
	iEventData.Set(aEventData);
	}

// Destructor
EXPORT_C TSynchronousConnectionChangedEvent::~TSynchronousConnectionChangedEvent()
	{
	}

// The static Cast method is used to obtain a pointer to the derived class object
EXPORT_C TSynchronousConnectionChangedEvent& TSynchronousConnectionChangedEvent::Cast(const THCIEventBase& aEvent)
	{
	__ASSERT_DEBUG(aEvent.EventCode() == ESynchronousConnectionChangedEvent, PANIC(KSymbianCommandsEventsPanicCat, EWrongEventCode));
	return *(reinterpret_cast<TSynchronousConnectionChangedEvent*>(&const_cast<THCIEventBase&>(aEvent)));
	}

// Accessor methods for the parameters of the event

EXPORT_C THCIConnectionHandle TSynchronousConnectionChangedEvent::ConnectionHandle() const
	{
	return AsConnectionHandle(3);
	}

EXPORT_C TUint8 TSynchronousConnectionChangedEvent::TransmissionInterval() const
	{
	return AsTUint8(5);
	}

EXPORT_C TUint8 TSynchronousConnectionChangedEvent::RetransmissionWindow() const
	{
	return AsTUint8(6);
	}

EXPORT_C TUint16 TSynchronousConnectionChangedEvent::RxPacketLength() const
	{
	return AsTUint16(7);
	}

EXPORT_C TUint16 TSynchronousConnectionChangedEvent::TxPacketLength() const
	{
	return AsTUint16(9);
	}


