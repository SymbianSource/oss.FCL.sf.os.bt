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
// on Wed, 25 Jul 2007 17:00:37 (time stamp)
// 
//

/**
 @file
 @publishedPartner
 @released
*/

#ifndef READLOCALVERSIONINFOCOMPLETEEVENT_H
#define READLOCALVERSIONINFOCOMPLETEEVENT_H

#include <bluetooth/hci/commandcompleteevent.h>

/**
This class represents command completion event for the ReadLocalVersionInfo HCI command
*/

class TReadLocalVersionInfoCompleteEvent : public THCICommandCompleteEvent
	{
public:
	// Construct an event object to wrap existing event data received from the Controller
	IMPORT_C TReadLocalVersionInfoCompleteEvent(const TDesC8& aEventData);
	// Construct an event object to generate a faked event using the supplied parameters
	IMPORT_C TReadLocalVersionInfoCompleteEvent(THCIErrorCode aStatus, TUint8 aNumHCICommandPackets, TUint8 aVersion, TUint16 aRevision, TUint8 aLMPVersion, TUint16 aManufacturerName, TUint16 aLMPSubversion, TDes8& aEventData);

	// The static Cast method is used to obtain a pointer to the derived class object
	IMPORT_C static TReadLocalVersionInfoCompleteEvent& Cast(const THCIEventBase& aEvent);

	// Accessor methods for the parameters of the event
	IMPORT_C TUint8 Version() const;
	IMPORT_C TUint16 Revision() const;
	IMPORT_C TUint8 LMPVersion() const;
	IMPORT_C TUint16 ManufacturerName() const;
	IMPORT_C TUint16 LMPSubversion() const;
	
	};
	
#endif // READLOCALVERSIONINFOCOMPLETEEVENT_H
