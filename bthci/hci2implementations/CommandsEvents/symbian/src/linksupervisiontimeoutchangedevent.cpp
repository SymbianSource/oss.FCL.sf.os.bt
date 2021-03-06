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
// on Mon, 02 Jun 2008 17:32:30 (time stamp)
// 
//

/**
 @file
 @internalComponent
*/

#include <bluetooth/hci/linksupervisiontimeoutchangedevent.h>
#include <bluetooth/hci/hcierrors.h>
#include <bluetooth/hci/hciopcodes.h>
#include "symbiancommandseventsutils.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_COMMANDSEVENTS_SYMBIAN);
#endif

// Construct an event object to wrap existing event data received from the Controller
EXPORT_C TLinkSupervisionTimeoutChangedEvent::TLinkSupervisionTimeoutChangedEvent(const TDesC8& aEventData)
	: THCIEventBase(aEventData)
	{
	
	}

// Construct a faked event, storing the supplied event parameters into the supplied empty event data buffer.
EXPORT_C TLinkSupervisionTimeoutChangedEvent::TLinkSupervisionTimeoutChangedEvent(THCIConnectionHandle aConnectionHandle, TBasebandTime aLinkSupervisionTimeout, TDes8& aEventData)
	: THCIEventBase(ELinkSupervisionTimeoutChangedEvent, 4, aEventData)
	{
	
	PutConnectionHandle(aConnectionHandle, aEventData);
	PutTUint16(aLinkSupervisionTimeout, aEventData);
	iEventData.Set(aEventData);
	}

// Destructor
EXPORT_C TLinkSupervisionTimeoutChangedEvent::~TLinkSupervisionTimeoutChangedEvent()
	{
	}

// The static Cast method is used to obtain a pointer to the derived class object
EXPORT_C TLinkSupervisionTimeoutChangedEvent& TLinkSupervisionTimeoutChangedEvent::Cast(const THCIEventBase& aEvent)
	{
	__ASSERT_DEBUG(aEvent.EventCode() == ELinkSupervisionTimeoutChangedEvent, PANIC(KSymbianCommandsEventsPanicCat, EWrongEventCode));
	return *(reinterpret_cast<TLinkSupervisionTimeoutChangedEvent*>(&const_cast<THCIEventBase&>(aEvent)));
	}

// Accessor methods for the parameters of the event

EXPORT_C THCIConnectionHandle TLinkSupervisionTimeoutChangedEvent::ConnectionHandle() const
	{
	return AsConnectionHandle(2);
	}

EXPORT_C TBasebandTime TLinkSupervisionTimeoutChangedEvent::LinkSupervisionTimeout() const
	{
	return AsTUint16(4);
	}


