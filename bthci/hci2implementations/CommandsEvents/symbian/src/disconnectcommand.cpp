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
// This file was generated automatically from the template commandsource.tmpl
// on Thu, 29 May 2008 15:17:49 (time stamp)
// 
//

/**
 @file
 @internalComponent
*/

#include <bluetooth/hci/disconnectcommand.h>
#include <bluetooth/hci/event.h>
#include <bluetooth/hci/commandcompleteevent.h>
#include <bluetooth/hci/hciframe.h>
#include <bluetooth/hci/hciopcodes.h>
#include <bluetooth/hci/disconnectioncompleteevent.h>


#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_COMMANDSEVENTS_SYMBIAN);
#endif



// Factory methods

EXPORT_C CDisconnectCommand* CDisconnectCommand::NewL(THCIConnectionHandle aConnectionHandle, TUint8 aReason)
	{
	CDisconnectCommand* self = new (ELeave) CDisconnectCommand(aConnectionHandle, aReason);
	CleanupStack::PushL(self);
	self->CHCICommandBase::BaseConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CDisconnectCommand* CDisconnectCommand::NewL()
	{
	CDisconnectCommand* self = new (ELeave) CDisconnectCommand();
	CleanupStack::PushL(self);
	self->CHCICommandBase::BaseConstructL();
	CleanupStack::Pop(self);
	return self;
	}

// Constructors

CDisconnectCommand::CDisconnectCommand(THCIConnectionHandle aConnectionHandle, TUint8 aReason)
	: CHCICommandBase(KDisconnectOpcode)
	, iConnectionHandle(aConnectionHandle)
	, iReason(aReason)
	{
	SetExpectsCommandCompleteEvent(EFalse);
	}

CDisconnectCommand::CDisconnectCommand()
	: CHCICommandBase(KDisconnectOpcode)
	{
	SetExpectsCommandCompleteEvent(EFalse);
	}

// Destructor
CDisconnectCommand::~CDisconnectCommand()
	{
	
	}	

/*virtual*/ void CDisconnectCommand::Match(const THCIEventBase& aEvent, TBool& aMatchesCmd, TBool& aConcludesCmd, TBool& aContinueMatching) const
	{
	if (aEvent.EventCode() == EDisconnectionCompleteEvent)
		{
		TDisconnectionCompleteEvent& event = TDisconnectionCompleteEvent::Cast(aEvent);
		if (event.ConnectionHandle() == ConnectionHandle())
			{
			aMatchesCmd = ETrue;
			aConcludesCmd = ETrue;
			aContinueMatching = EFalse;
			}
		}
	// Command Status Event and default Command Complete Event matching
	// is implemented in the base class.  If we haven't matched already
	// then we should try the default matching.
	if (!aMatchesCmd)
		{
		CHCICommandBase::Match(aEvent, aMatchesCmd, aConcludesCmd, aContinueMatching);
		}
	}
	

// Override of pure virtual from CHCICommandBase. This method embodies the knowledge
// of how to format the specifics of this command into the HCTL command frame.
void CDisconnectCommand::Format(CHctlCommandFrame& aCommandFrame) const
	{
	aCommandFrame.PutConnectionHandle(iConnectionHandle);
	aCommandFrame.PutByte(iReason);
	}

// Assign new values to the parameters of this command
EXPORT_C void CDisconnectCommand::Reset(THCIConnectionHandle aConnectionHandle, TUint8 aReason)
	{
	iConnectionHandle = aConnectionHandle;
	iReason = aReason;
	}

// Accessor methods for the parameters of the command

EXPORT_C THCIConnectionHandle CDisconnectCommand::ConnectionHandle() const
	{
	return iConnectionHandle;
	}

EXPORT_C TUint8 CDisconnectCommand::Reason() const
	{
	return iReason;
	}



// Extension function.  Use this to retrieve any extension interfaces from CDisconnectCommand
// or any class from which it derives.
/*virtual*/ TInt CDisconnectCommand::Extension_(TUint aExtensionId, TAny*& aInterface, TAny* aData)
	{
	// To add an additional interface implementation specific for this class check the 
	// provided ID and return an appropriate interface.

	// If a specific interface implementation is not provided - forward the call to the base class.
	return CHCICommandBase::Extension_(aExtensionId, aInterface, aData);
	}

