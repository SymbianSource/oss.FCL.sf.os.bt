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
// 

/**
@file
@internalComponent
*/

#include "hctlusboriginalserver.h"

#include "hctlusboriginal.h"
#include "hctlusboriginalutils.h"
#include "hctlusboriginalpolicy.h"

#include <bluetooth/logger.h>

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_HCTL_USB_ORIGINAL);
#endif


CHCTLUsbOriginalServer::CHCTLUsbOriginalServer(CHCTLUsbOriginal& aUsbOriginal)
	: CPolicyServer(CActive::EPriorityStandard, KHCTLUsbOriginalPolicy)
	, iUsbOriginal(aUsbOriginal)
	{
	LOG_FUNC
	}

void CHCTLUsbOriginalServer::ConstructL()
	{
	LOG_FUNC
	StartL(KHCTLUsbOrginalSrvName);
	}

CHCTLUsbOriginalServer* CHCTLUsbOriginalServer::NewL(CHCTLUsbOriginal& aUsbOriginal)
	{
	LOG_STATIC_FUNC
	CHCTLUsbOriginalServer* self = new(ELeave) CHCTLUsbOriginalServer(aUsbOriginal);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

CHCTLUsbOriginalServer::~CHCTLUsbOriginalServer()
	{
	LOG_FUNC
	}

CSession2* CHCTLUsbOriginalServer::NewSessionL(const TVersion& aVersion, const RMessage2& /*aMessage*/) const
	{
	LOG_FUNC
	LOG3(_L8("\taVersion = (%d,%d,%d)"), aVersion.iMajor, aVersion.iMinor, aVersion.iBuild);
		
	// Version number check...
	TVersion v(KHCTLUsbOriginalSrvMajorVersionNumber,
			   KHCTLUsbOriginalSrvMinorVersionNumber,
			   KHCTLUsbOriginalSrvBuildNumber);
	
	if(!User::QueryVersionSupported(v, aVersion))
		{
		LEAVEIFERRORL(KErrNotSupported);
		}
	
	if(iSession)
		{
		LEAVEIFERRORL(KErrInUse);
		}
	
	iSession = new(ELeave) CHCTLUsbOriginalSession(iUsbOriginal);
	return iSession;
	}

void CHCTLUsbOriginalServer::DropSession(CHCTLUsbOriginalSession* aSession) const
	{
	__ASSERT_DEBUG(iSession == aSession, PANIC(KUsbOriginalPanic, EBadSessionPointer));
	iSession = NULL;
	}



CHCTLUsbOriginalSession::CHCTLUsbOriginalSession(CHCTLUsbOriginal& aUsbOriginal)
	: iUsbOriginal(aUsbOriginal)
	{
	LOG_FUNC
	}

CHCTLUsbOriginalSession::~CHCTLUsbOriginalSession()
	{
	LOG_FUNC
	static_cast<const CHCTLUsbOriginalServer*>(Server())->DropSession(this);
	}

void CHCTLUsbOriginalSession::ServiceL(const RMessage2& aMessage)
	{
	LOG_FUNC
	LOG1(_L("\taMessage.Function() = %d"), aMessage.Function());
	
	switch(aMessage.Function())
		{
	case EFunctionAttached:
			{
			if(iDeviceAttached)
				{
				PANIC_MSG(aMessage, KHCTLUsbOriginalServerPanicCat, EDeviceAlreadyAttached);
				return;
				}
			TUint32 aclToken = static_cast<TUint32>(aMessage.Int0());
			TUint32 scoToken = static_cast<TUint32>(aMessage.Int1());
			iUsbOriginal.DeviceAttachedL(aclToken, scoToken);
			iDeviceAttached = ETrue;
			}
		break;
	
	case EFunctionDetached:
			{
			if(!iDeviceAttached)
				{
				PANIC_MSG(aMessage, KHCTLUsbOriginalServerPanicCat, EDeviceNotAttached);
				return;
				}
			iUsbOriginal.DeviceRemoved();
			iDeviceAttached = EFalse;
			}
		break;
	
	default:
		PANIC_MSG(aMessage, KHCTLUsbOriginalServerPanicCat, EInvalidFunction);
		return;
		}
	aMessage.Complete(KErrNone);
	}

