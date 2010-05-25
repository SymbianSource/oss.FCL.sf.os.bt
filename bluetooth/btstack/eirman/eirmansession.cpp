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

CEirManSession* CEirManSession::NewL(CEirManServer& aServer, MEirSessionNotifier& aParent, TBool aInternalSession)
	{
	LOG_STATIC_FUNC
	CEirManSession* self = new(ELeave) CEirManSession(aServer, aParent, aInternalSession);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

CEirManSession::CEirManSession(CEirManServer& aServer, MEirSessionNotifier& aParent, TBool aInternalSession)
	: iEirManServer(aServer)
	, iParent(aParent)
	, iEirTag(EEirTagRESERVED)
	, iInternalSession(aInternalSession)
	{
	LOG_FUNC
	}

void CEirManSession::ConstructL()
	{
	LOG_FUNC
	iEirManServer.AddSession(*this, iInternalSession);
	}

CEirManSession::~CEirManSession()
	{
	LOG_FUNC
	// deregister any registered tag that may be registered
	DeregisterTag();
	iLink.Deque();
	iEirManServer.DropSession(iInternalSession);
	}

void CEirManSession::RegisterTag(TEirTag aTag)
	{
	LOG_FUNC

	LOG1(_L("CEirManSession::RegisterTag tag = %d"), aTag);

	if(!(aTag >= EEirTagName && aTag < EEirTagRESERVED))
		{
		iParent.MesnRegisterComplete(KErrArgument);
		}
	else if(iEirTag != EEirTagRESERVED)
		{
		LOG1(_L("CEirManSession::RegisterTag ERROR, Tag in use: %d"), iEirTag);
		iParent.MesnRegisterComplete(KErrInUse);
		}
	else
		{
		// Register this tag for callbacks
		if(iEirManServer.EirFeatureState() == EEirFeatureReady)
			{
			// Eir is supported
			TInt result = iEirManServer.EirManager().RegisterTag(aTag, *this);
			if(result == KErrNone)
				{
				iEirTag = aTag;
				}
			iParent.MesnRegisterComplete(result);
			}
		else if(iEirManServer.EirFeatureState() == EEirFeaturePending)
			{
			// We don't know if eir is supported or not at this moment
			iRegisterPending = ETrue;
			iPendingEirTag = aTag;
			}
		else
			{
			// Eir is not supported
			iParent.MesnRegisterComplete(KErrNotSupported);
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
	if(aResult == KErrNone && iRegisterPending)
		{
		__ASSERT_DEBUG(iEirManServer.EirFeatureState() == EEirFeatureReady, EIR_SESSION_PANIC(EEirSessionEirFeatureNotSupported));
		TInt result = iEirManServer.EirManager().RegisterTag(iPendingEirTag, *this);
		if(result == KErrNone)
			{
			iEirTag = iPendingEirTag;
			}
		iRegisterPending = EFalse;
		iParent.MesnRegisterComplete(result);
		}
	else if(iRegisterPending)
		{
		iRegisterPending = EFalse;
		iParent.MesnRegisterComplete(aResult);
		}
	}


TInt CEirManSession::NewData(TInt aRequiredLength)
	{
	LOG_FUNC
	return iEirManServer.EirManager().NewData(iEirTag, aRequiredLength);
	}


TInt CEirManSession::SetData(const TDesC8& aData, TEirDataMode aMode)
	{
	LOG_FUNC
	return iEirManServer.EirManager().SetData(iEirTag, aData, aMode);
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

	iParent.MesnSpaceAvailable(aSpaceForTag);

	}

TEirTag CEirManSession::EirTag() const
	{
	return iEirTag;
	}

CEirManExternalSession* CEirManExternalSession::NewL(CEirManServer& aServer)
	{
	CEirManExternalSession* self = new(ELeave) CEirManExternalSession();
	CleanupStack::PushL(self);
	self->ConstructL(aServer);
	CleanupStack::Pop(self);
	return self;
	}

CEirManExternalSession::CEirManExternalSession()
	{
	
	}

void CEirManExternalSession::ConstructL(CEirManServer& aServer)
	{
	iSession = CEirManSession::NewL(aServer, *this, EFalse);
	}

CEirManExternalSession::~CEirManExternalSession()
	{
	if(!iDataAvailableListenerMessage.IsNull())
		{
		// complete any outstanding messages.
		iDataAvailableListenerMessage.Complete(KErrCancel);
		}
	delete iSession;
	}

void CEirManExternalSession::ServiceL(const RMessage2& aMessage)
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

void CEirManExternalSession::RegisterTag(const RMessage2& aMessage)
	{
	LOG_FUNC
	TEirTag tag = static_cast<TEirTag>(aMessage.Int0());
	LOG1(_L("CEirManSession::RegisterTag tag = %d"), tag);

	iRegisterMessage = aMessage;
	
	iSession->RegisterTag(tag);

	}

void CEirManExternalSession::MesnRegisterComplete(TInt aResult)
	{
	if (aResult == KErrArgument)
		{
		__ASSERT_ALWAYS(EFalse, iRegisterMessage.Panic(KEirManCliPncCat, EEirManPanicInvalidTag));
		}
	iRegisterMessage.Complete(aResult);
	}

TInt CEirManExternalSession::RegisterSpaceAvailableListener(const RMessage2& aMessage, TBool& aComplete)
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
	
TInt CEirManExternalSession::NewData(const RMessage2& aMessage)
	{
	LOG_FUNC
	__ASSERT_ALWAYS(iSession->EirTag() != EEirTagRESERVED, aMessage.Panic(KEirManCliPncCat, EEirManPanicInvalidTag));
	TInt requiredLength = static_cast<TInt>(aMessage.Int0());

	return iSession->NewData(requiredLength);
	}

TInt CEirManExternalSession::CancelSpaceAvailableListener()
	{
	LOG_FUNC

	if(!iDataAvailableListenerMessage.Handle())
		{
		return KErrNotFound;
		}

	iDataAvailableListenerMessage.Complete(KErrCancel);

	return KErrNone;
	}

TInt CEirManExternalSession::SetData(const RMessage2& aMessage)
	{
	LOG_FUNC
	__ASSERT_ALWAYS(iSession->EirTag() != EEirTagRESERVED, aMessage.Panic(KEirManCliPncCat, EEirManPanicInvalidTag));
	TEirDataMode eirDataMode = static_cast<TEirDataMode>(aMessage.Int1());
	LOG1(_L("EirDataMode: %d"),  eirDataMode);

	// No need to allocate memory with an expensive malloc() call (via HBuf8::NewL or whatever), 
	// since the EIR contents fit in the stack, and the EIR Manager will cache the data anyway
	TBuf8<KHCIExtendedInquiryResponseMaxLength> data;
	TInt err = aMessage.Read(0, data);
	
	if(err == KErrNone)
		{
		err = iSession->SetData(data, eirDataMode);
		}
	return err;
	}

void CEirManExternalSession::MesnSpaceAvailable(TUint aSpaceForTag)
	{
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

void CEirManExternalSession::CompleteSpaceAvailableRequest(TUint aBytesAvailable)
	{
	LOG_FUNC
	LOG1(_L("CEirManSession::CompleteSpaceAvailableRequest bytes: %d"), aBytesAvailable);
	// Offer the space to the client
	TPckgC<TUint32> pckg(aBytesAvailable);

	TInt err = iDataAvailableListenerMessage.Write(0, pckg);
	iDataAvailableListenerMessage.Complete(err);
	}

CEirManInternalSession* CEirManInternalSession::NewL(CEirManServer& aServer, MEirInternalSessionNotifier& aParent)
	{
	LOG_STATIC_FUNC
	CEirManInternalSession* self = new(ELeave) CEirManInternalSession(aParent);
	CleanupStack::PushL(self);
	self->ConstructL(aServer);
	CleanupStack::Pop(self);
	return self;
	}

CEirManInternalSession::CEirManInternalSession(MEirInternalSessionNotifier& aParent)
	: iParent(aParent)
	{
	
	}

void CEirManInternalSession::ConstructL(CEirManServer& aServer)
	{
	iSession = CEirManSession::NewL(aServer, *this, ETrue);
	iSetDataCb = new (ELeave) CAsyncCallBack(CActive::EPriorityHigh);
	TCallBack cb(&SetDataCb, this);
	iSetDataCb->Set(cb);
	}

CEirManInternalSession::~CEirManInternalSession()
	{
	delete iSetDataCb;
	delete iSession;
	}
	
void CEirManInternalSession::RegisterTag(TEirTag aTag)
	{
	iSession->RegisterTag(aTag);
	}

void CEirManInternalSession::SetData(const TDesC8& aData, TEirDataMode aMode)
	{
	delete iPendingData;
	iPendingData = NULL;
	iSetDataCb->Cancel();
	iPendingData = aData.Alloc();
	iPendingMode = aMode;
	if (iPendingData)
		{
		iSetDataCb->CallBack();
		}
	}

TInt CEirManInternalSession::SetDataCb(TAny* aThis)
	{
	static_cast<CEirManInternalSession*>(aThis)->DoSetData();
	return KErrNone;
	}

void CEirManInternalSession::DoSetData()
	{
	TInt err = iSession->SetData(*iPendingData, iPendingMode);
	if (err != KErrNone)
		{
		iParent.MeisnSetDataError(err);
		}
	}

TInt CEirManInternalSession::NewData(TInt aRequiredLength)
	{
	return iSession->NewData(aRequiredLength);
	}


void CEirManInternalSession::MesnRegisterComplete(TInt aResult)
	{
	iParent.MeisnRegisterComplete(aResult);
	}

void CEirManInternalSession::MesnSpaceAvailable(TUint aSpaceForTag)
	{
	iParent.MeisnSpaceAvailable(aSpaceForTag - KEirLengthTagLength);
	}
