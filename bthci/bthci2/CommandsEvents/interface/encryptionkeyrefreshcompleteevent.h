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
// on Thu, 29 May 2008 15:23:55 (time stamp)
// 
//

/**
 @file
 @publishedPartner
 @released
*/

#ifndef ENCRYPTIONKEYREFRESHCOMPLETEEVENT_H
#define ENCRYPTIONKEYREFRESHCOMPLETEEVENT_H

#include <bluetooth/hci/event.h>

/**
This class represents the EncryptionKeyRefreshComplete HCI Event
*/
class TEncryptionKeyRefreshCompleteEvent : public THCIEventBase
	{
public:
	// Construct an event object to wrap existing event data received from the Controller
	IMPORT_C TEncryptionKeyRefreshCompleteEvent(const TDesC8& aEventData);
	// Construct a faked event, storing the supplied event parameters into the supplied empty event data buffer.
	IMPORT_C TEncryptionKeyRefreshCompleteEvent(THCIErrorCode aStatus, THCIConnectionHandle aConnectionHandle, TDes8& aEventData);
	// Destructor
	IMPORT_C ~TEncryptionKeyRefreshCompleteEvent();

	// The static Cast method is used to obtain a pointer to the derived class object
	IMPORT_C static TEncryptionKeyRefreshCompleteEvent& Cast(const THCIEventBase& aEvent);

	// Accessor methods for the parameters of the event
	IMPORT_C THCIConnectionHandle ConnectionHandle() const;
	
	};
	
#endif // ENCRYPTIONKEYREFRESHCOMPLETEEVENT_H
