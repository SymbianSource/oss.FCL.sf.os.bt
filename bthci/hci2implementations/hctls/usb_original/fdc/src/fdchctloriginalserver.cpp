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

#include "fdchctloriginalserver.h"

#include "fdchctloriginal.h"
#include "fdchctloriginalutils.h"
#include "fdchctloriginalpolicy.h"

#include <bluetooth/logger.h>

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "fdchctloriginal");
#endif


CFdcHctlOriginalServer::CFdcHctlOriginalServer(CFdcHctlOriginal& aHctlOriginal)
	: CPolicyServer(CActive::EPriorityStandard, KFdcHctlOriginalPolicy)
	, iHctlOriginal(aHctlOriginal)
	{
	LOG_FUNC
	}

void CFdcHctlOriginalServer::ConstructL()
	{
	LOG_FUNC
	StartL(KFdcHctlOrigSrvName);
	}

CFdcHctlOriginalServer* CFdcHctlOriginalServer::NewL(CFdcHctlOriginal& aHctlOriginal)
	{
	LOG_STATIC_FUNC
	CFdcHctlOriginalServer* self = new(ELeave) CFdcHctlOriginalServer(aHctlOriginal);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

CFdcHctlOriginalServer::~CFdcHctlOriginalServer()
	{
	LOG_FUNC
	}

CSession2* CFdcHctlOriginalServer::NewSessionL(const TVersion& aVersion, const RMessage2& /*aMessage*/) const
	{
	LOG_FUNC
	LOG3(_L8("\taVersion = (%d,%d,%d)"), aVersion.iMajor, aVersion.iMinor, aVersion.iBuild);
		
	// Version number check...
	TVersion v(KFdcHctlOrigSrvMajorVersionNumber,
			   KFdcHctlOrigSrvMinorVersionNumber,
			   KFdcHctlOrigSrvBuildNumber);
	
	if(!User::QueryVersionSupported(v, aVersion))
		{
		LEAVEIFERRORL(KErrNotSupported);
		}
	
	if(iSession)
		{
		LEAVEIFERRORL(KErrInUse);
		}
	
	iSession = new(ELeave) CFdcHctlOriginalSession(iHctlOriginal);
	return iSession;
	}

void CFdcHctlOriginalServer::DropSession(CFdcHctlOriginalSession* aSession) const
	{
	LOG_FUNC
	__ASSERT_DEBUG(iSession == aSession, PANIC(KFdcHctlOriginalPanic, EBadSessionPointer));
	iSession = NULL;
	}


CFdcHctlOriginalSession::CFdcHctlOriginalSession(CFdcHctlOriginal& aHctlOriginal)
	: iHctlOriginal(aHctlOriginal)
	{
	LOG_FUNC
	}

CFdcHctlOriginalSession::~CFdcHctlOriginalSession()
	{
	LOG_FUNC
	static_cast<const CFdcHctlOriginalServer*>(Server())->DropSession(this);
	}

void CFdcHctlOriginalSession::ServiceL(const RMessage2& aMessage)
	{
	LOG_FUNC
	switch(aMessage.Function())
		{
	case EFunctionRequestConnection:
		iHctlOriginal.RequestConnection();
		break;
	default:
		PANIC_MSG(aMessage, KFdcHctlOriginalSrvPanicCat, EFdcHctlInvalidFunction);
		return;
		}
	aMessage.Complete(KErrNone);
	}

