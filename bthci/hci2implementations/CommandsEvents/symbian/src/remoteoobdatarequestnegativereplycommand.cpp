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
// on Thu, 29 May 2008 15:17:55 (time stamp)
// 
//

/**
 @file
 @internalComponent
*/

#include <bluetooth/hci/remoteoobdatarequestnegativereplycommand.h>
#include <bluetooth/hci/event.h>
#include <bluetooth/hci/commandcompleteevent.h>
#include <bluetooth/hci/hciframe.h>
#include <bluetooth/hci/hciopcodes.h>


#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_COMMANDSEVENTS_SYMBIAN);
#endif



// Factory methods

EXPORT_C CRemoteOOBDataRequestNegativeReplyCommand* CRemoteOOBDataRequestNegativeReplyCommand::NewL(const TBTDevAddr& aBDADDR)
	{
	CRemoteOOBDataRequestNegativeReplyCommand* self = new (ELeave) CRemoteOOBDataRequestNegativeReplyCommand(aBDADDR);
	CleanupStack::PushL(self);
	self->CHCICommandBase::BaseConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C CRemoteOOBDataRequestNegativeReplyCommand* CRemoteOOBDataRequestNegativeReplyCommand::NewL()
	{
	CRemoteOOBDataRequestNegativeReplyCommand* self = new (ELeave) CRemoteOOBDataRequestNegativeReplyCommand();
	CleanupStack::PushL(self);
	self->CHCICommandBase::BaseConstructL();
	CleanupStack::Pop(self);
	return self;
	}

// Constructors

CRemoteOOBDataRequestNegativeReplyCommand::CRemoteOOBDataRequestNegativeReplyCommand(const TBTDevAddr& aBDADDR)
	: CHCICommandBase(KRemoteOOBDataRequestNegativeReplyOpcode)
	, iBDADDR(aBDADDR)
	{
	SetExpectsCommandStatusEvent(EFalse);
	}

CRemoteOOBDataRequestNegativeReplyCommand::CRemoteOOBDataRequestNegativeReplyCommand()
	: CHCICommandBase(KRemoteOOBDataRequestNegativeReplyOpcode)
	{
	SetExpectsCommandStatusEvent(EFalse);
	}

// Destructor
CRemoteOOBDataRequestNegativeReplyCommand::~CRemoteOOBDataRequestNegativeReplyCommand()
	{
	
	}	



// Override of pure virtual from CHCICommandBase. This method embodies the knowledge
// of how to format the specifics of this command into the HCTL command frame.
void CRemoteOOBDataRequestNegativeReplyCommand::Format(CHctlCommandFrame& aCommandFrame) const
	{
	aCommandFrame.PutDevAddr(iBDADDR);
	}

// Assign new values to the parameters of this command
EXPORT_C void CRemoteOOBDataRequestNegativeReplyCommand::Reset(const TBTDevAddr& aBDADDR)
	{
	iBDADDR = aBDADDR;
	}

// Accessor methods for the parameters of the command

EXPORT_C TBTDevAddr CRemoteOOBDataRequestNegativeReplyCommand::BDADDR() const
	{
	return iBDADDR;
	}



// Extension function.  Use this to retrieve any extension interfaces from CRemoteOOBDataRequestNegativeReplyCommand
// or any class from which it derives.
/*virtual*/ TInt CRemoteOOBDataRequestNegativeReplyCommand::Extension_(TUint aExtensionId, TAny*& aInterface, TAny* aData)
	{
	// To add an additional interface implementation specific for this class check the 
	// provided ID and return an appropriate interface.

	// If a specific interface implementation is not provided - forward the call to the base class.
	return CHCICommandBase::Extension_(aExtensionId, aInterface, aData);
	}

