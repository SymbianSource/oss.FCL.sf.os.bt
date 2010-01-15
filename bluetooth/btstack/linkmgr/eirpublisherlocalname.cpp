// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "eirpublisherlocalname.h"
#include "linkutil.h"
#include <bluetooth/eirpublisherbase.h>
#include <bluetooth/logger.h>

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_EIRMANAGER);
#endif


//**********************************
// CEirPublisherLocalName
//**********************************
/**
Provides functionality to publish Local Name to EIR.
**/
CEirPublisherLocalName* CEirPublisherLocalName::NewL()
	{
	LOG_STATIC_FUNC
	CEirPublisherLocalName* self = new (ELeave) CEirPublisherLocalName();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

CEirPublisherLocalName::CEirPublisherLocalName()
:	CEirPublisherBase(EEirTagName)
	{
	LOG_FUNC
	}

CEirPublisherLocalName::~CEirPublisherLocalName()
	{
	LOG_FUNC
	delete iPublishBuf;
	}
	
void CEirPublisherLocalName::ConstructL()
	{
	LOG_FUNC
	CEirPublisherBase::ConstructL();
	}
	
void CEirPublisherLocalName::UpdateName(const TDesC8& aName)
	{
	LOG_FUNC
	// Check aName isn't longer than 248 characters
	__ASSERT_DEBUG(aName.Length() <= 248, Panic(EEIRPublisherUpdateNameTooLong));
	iLocalName = aName;
	iPublisher->PublishData(aName.Size());
	}
	

// From MEirPublisherNotifier
void CEirPublisherLocalName::MepnSpaceAvailable(TUint aBytesAvailable)
	{
	LOG_FUNC
	// Delete memory from last time this was called
	delete iPublishBuf, iPublishBuf = NULL;
	
	TUint8 namelen = iLocalName.Length();
	TUint avail = aBytesAvailable;
	avail = avail < namelen ? avail : namelen;
		
	if (avail < namelen)
		{
		// Truncate the device name to space available
		iPublishBuf = iLocalName.Left(avail).Alloc();
		if(iPublishBuf)
			{
			iPublisher->SetData(*iPublishBuf, EEirDataPartial);
			}
		}
	else
		{
		// Zero-length device names will be published as "complete"
		// as defined in the specification, volume 3, part C, 8.1
		iPublishBuf = iLocalName.Alloc();
		if(iPublishBuf)
			{
			iPublisher->SetData(*iPublishBuf, EEirDataComplete);
			}
		}
	// Final case to handle if OOM occurs.
	if(!iPublishBuf)
		{
		iPublisher->SetData(KNullDesC8(), EEirDataPartial);
		}
	}
	
void CEirPublisherLocalName::MepnSetDataError(TInt /*aResult*/)
	{
	LOG_FUNC
	}
