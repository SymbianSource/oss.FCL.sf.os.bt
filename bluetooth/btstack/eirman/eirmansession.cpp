// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// EIR Manager server session.
// 
//

/**
 @file
 @internalComponent
*/

#include "eirmansession.h"
#include <bluetooth/logger.h>
#include <bluetooth/eirmanshared.h>
#include <bttypes.h>
#include <bluetooth/hci/hciconsts.h>
#include "eirmanpanics.h"
#include "eirmanserver.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_EIRMANAGER);
#endif

CEirManSession* CEirManSession::NewL(CEirManServer& aServer)
	{
	LOG_STATIC_FUNC
	CEirManSession* self = new(ELeave) CEirManSession(aServer);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

CEirManSession::CEirManSession(CEirManServer& aServer)
	: iEirManServer(aServer)
	, iEirTag(EEirTagRESERVED)
	{
	LOG_FUNC
	}

void CEirManSession::ConstructL()
	{
	LOG_FUNC
	iEirManServer.AddSession();
	}

CEirManSession::~CEirManSession()
	{
	LOG_FUNC
	// deregister any registered tag that may be registered
	DeregisterTag();
	if(!iDataAvailableListenerMessage.IsNull())
		{
		// complete any outstanding messages.
		iDataAvailableListenerMessage.Complete(KErrCancel);
		}
	iEirManServer.DropSession();
	}

void CEirManSession::ServiceL(const RMessage2& aMessage)
	{
	LOG_FUNC
	LOG1(_L("CEirManSession::ServiceL aMessage.Function() = %d"), aMessage.Function());
	TBool complete = ETrue;
	TInt ret = KErrNone;

	switch (aMessage.Function())
		{
	case EEirManRegisterTag:
		complete = EFalse; // always async.
		RegisterTag(aMessage);
		break;

	case EEirManSpaceAvailableNotification:
		ret = RegisterSpaceAvailableListener(aMessage, complete);
		break;

	case EEirManCancelSpaceAvailableNotification:
		ret = CancelSpaceAvailableListener();
		break;

	case EEirManSetData:
		ret = SetData(aMessage);
		break;
		
	case EEirManNewData:
		ret = NewData(aMessage);
		break;

	default:
		aMessage.Panic(KEirManCliPncCat, EEirManPanicInvalidIPC);
		break;
		}

	if (complete)
		{
		aMessage.Complete(ret);
		}
	}

void CEirManSession::RegisterTag(const RMessage2& aMessage)
	{
	LOG_FUNC

	TEirTag tag = static_cast<TEirTag>(aMessage.Int0());
	LOG1(_L("CEirManSession::RegisterTag tag = %d"), tag);

	if(!(tag >= EEirTagName && tag < EEirTagRESERVED))
		{
		__ASSERT_ALWAYS(EFalse, aMessage.Panic(KEirManCliPncCat, EEirManPanicInvalidTag));
		}
	else if(iEirTag != EEirTagRESERVED)
		{
		LOG1(_L("CEirManSession::RegisterTag ERROR, Tag in use: %d"), iEirTag);
		aMessage.Complete(KErrInUse);
		}
	else
		{
		// Register this tag for callbacks
		if(iEirManServer.EirFeatureState() == EEirFeatureReady)
			{
			// Eir is supported
			TInt result = iEirManServer.EirManager().RegisterTag(tag, *this);
			if(result == KErrNone)
				{
				iEirTag = tag;
				}
			aMessage.Complete(result);
			}
		else if(iEirManServer.EirFeatureState() == EEirFeaturePending)
			{
			// We don't know if eir is supported or not at this moment
			iPendingEirTag = tag;
			iRegisterMessage = aMessage;
			}
		else
			{
			// Eir is not supported
			aMessage.Complete(KErrNotSupported);
			}
		}
	}

void CEirManSession::DeregisterTag()
	{
	LOG_FUNC
	LOG1(_L("CEirManSession::DeregisterTag tag = %d"), iEirTag);
	
	if(iEirTag != EEirTagRESERVED)
		{
		// Deregister this tag for callbacks 
		iEirManServer.EirManager().DeregisterTag(iEirTag);
		iEirTag = EEirTagRESERVED;
		}
	}

// Eir Server has tried to register tag
void CEirManSession::NotifyEirFeatureState(TInt aResult)
	{
	LOG1(_L("Eir Server has been notified feature ready, result: %d"), aResult);
	if(aResult == KErrNone && !iRegisterMessage.IsNull())
		{
		__ASSERT_DEBUG(iEirManServer.EirFeatureState() == EEirFeatureReady, EIR_SESSION_PANIC(EEirSessionEirFeatureNotSupported));
		TInt result = iEirManServer.EirManager().RegisterTag(iPendingEirTag, *this);
		if(result == KErrNone)
			{
			iEirTag = iPendingEirTag;
			}
		iRegisterMessage.Complete(result);
		}
	else if(!iRegisterMessage.IsNull())
		{
		iRegisterMessage.Complete(aResult);
		}
	}

TInt CEirManSession::RegisterSpaceAvailableListener(const RMessage2& aMessage, TBool& aComplete)
	{
	LOG_FUNC

	if(iDataAvailableListenerMessage.Handle())
		{
		LOG(_L("CEirManSession:::RegisterSpaceAvailableListener ERROR IN USE"));
		return KErrInUse;
		}

	iDataAvailableListenerMessage = aMessage;

	aComplete = EFalse;

	if(iLastSpaceOffered != 0)
		{
		LOG(_L("cached space present, completing immediately"));
		CompleteSpaceAvailableRequest(iLastSpaceOffered);
		iLastSpaceOffered = 0;
		return KErrNone;
		}
	
	LOG(_L("waiting for callback..."));
	return KErrNone;
	}
	
TInt CEirManSession::NewData(const RMessage2& aMessage)
	{
	LOG_FUNC
	__ASSERT_ALWAYS(iEirTag != EEirTagRESERVED, aMessage.Panic(KEirManCliPncCat, EEirManPanicInvalidTag));
	TInt requiredLength = static_cast<TInt>(aMessage.Int0());

	return iEirManServer.EirManager().NewData(iEirTag, requiredLength);
	}

TInt CEirManSession::CancelSpaceAvailableListener()
	{
	LOG_FUNC

	if(!iDataAvailableListenerMessage.Handle())
		{
		return KErrNotFound;
		}

	iDataAvailableListenerMessage.Complete(KErrCancel);

	return KErrNone;
	}

TInt CEirManSession::SetData(const RMessage2& aMessage)
	{
	LOG_FUNC
	__ASSERT_ALWAYS(iEirTag != EEirTagRESERVED, aMessage.Panic(KEirManCliPncCat, EEirManPanicInvalidTag));
	TEirDataMode eirDataMode = static_cast<TEirDataMode>(aMessage.Int1());
	LOG2(_L("Tag: %d EirDataMode: %d"), iEirTag, eirDataMode);

	// No need to allocate memory with an expensive malloc() call (via HBuf8::NewL or whatever), 
	// since the EIR contents fit in the stack, and the EIR Manager will cache the data anyway
	TBuf8<KHCIExtendedInquiryResponseMaxLength> data;
	TInt err = aMessage.Read(0, data);
	
	if(err == KErrNone)
		{
		err = iEirManServer.EirManager().SetData(iEirTag, data, eirDataMode);
		}
	return err;
	}

// Callback from the EIR Manager
void CEirManSession::MemnEirBlockAvailable(TEirTag aTag, TUint aSpaceForTag)
	{
	LOG_FUNC
	LOG2(_L("CEirManSession::MemnEirBlockAvailable Tag: %d space: %d"), aTag, aSpaceForTag);

	// Silently discard callbacks not containing our EIR Tag
	if(aTag != iEirTag)
		{
		__ASSERT_DEBUG(EFalse, EIR_SESSION_PANIC(EEirSessionInvalidEirTag));
		LOG3(_L("CEirManSession::MemnEirBlockAvailable early return: aTag: %d iEirTag: %d space: %d"), aTag, iEirTag, aSpaceForTag);
		return;
		}

	if(iDataAvailableListenerMessage.Handle())
		{
		LOG(_L("Listener outstanding, completing request"));
		// Clear stored value
		iLastSpaceOffered = 0;
		CompleteSpaceAvailableRequest(aSpaceForTag);
		}
	else
		{
		LOG(_L("NO Listener outstanding, storing space"));
		// Store in case a client registers later. If this happens multiple times, only the last one will be remembered
		iLastSpaceOffered = aSpaceForTag;
		}
	}

void CEirManSession::CompleteSpaceAvailableRequest(TUint aBytesAvailable)
	{
	LOG_FUNC
	LOG1(_L("CEirManSession::CompleteSpaceAvailableRequest bytes: %d"), aBytesAvailable);
	// Offer the space to the client
	TPckg<TUint32> pckg(aBytesAvailable);

	TInt err = iDataAvailableListenerMessage.Write(0, pckg);
	iDataAvailableListenerMessage.Complete(err);
	}

